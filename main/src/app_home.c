#include "app_home.h"
#include "app_list.h"
#include "badgehub_client.h"
#include "app_card.h"
#include "lvgl/lvgl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// --- CONSTANTS ---
#define ITEMS_PER_PAGE 20

// --- STATIC STATE VARIABLES ---
static lv_obj_t *list_container;
static lv_obj_t *search_bar;
static lv_obj_t *page_indicator_label; // New label for page number
static lv_timer_t *search_timer = NULL;

static int current_offset = 0;
static bool is_fetching = false;
static bool end_of_list_reached = false;
static int total_pages = -1; // -1 means unknown

// --- FORWARD DECLARATIONS ---
static void search_timer_cb(lv_timer_t *timer);
static void search_bar_event_cb(lv_event_t *e);
static void search_bar_key_event_cb(lv_event_t *e);
static void home_view_delete_event_cb(lv_event_t *e);
static void fetch_and_display_page(int offset, bool focus_last);

// --- IMPLEMENTATIONS ---

lv_obj_t* get_search_bar(void) {
    return search_bar;
}

void create_app_home_view(void) {
    lv_obj_clean(lv_screen_active());

    lv_obj_t *main_container = lv_obj_create(lv_screen_active());
    lv_obj_set_size(main_container, lv_pct(100), lv_pct(100));
    lv_obj_set_layout(main_container, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(main_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_center(main_container);
    lv_obj_add_event_cb(main_container, home_view_delete_event_cb, LV_EVENT_DELETE, NULL);

    // Create the search bar but don't parent it yet.
    search_bar = lv_textarea_create(NULL); // Create without a parent
    lv_obj_set_width(search_bar, lv_pct(95));
    lv_textarea_set_one_line(search_bar, true);
    lv_textarea_set_placeholder_text(search_bar, "Search for apps...");
    lv_obj_add_event_cb(search_bar, search_bar_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(search_bar, search_bar_key_event_cb, LV_EVENT_KEY, NULL);
    lv_group_add_obj(lv_group_get_default(), search_bar);

    list_container = lv_obj_create(main_container);
    lv_obj_set_width(list_container, lv_pct(100));
    lv_obj_set_flex_grow(list_container, 1);
    lv_obj_set_flex_flow(list_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(list_container, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // Create the page indicator label
    page_indicator_label = lv_label_create(main_container);
    lv_obj_set_width(page_indicator_label, lv_pct(95));
    lv_obj_set_style_text_align(page_indicator_label, LV_TEXT_ALIGN_CENTER, 0);

    fetch_and_display_page(0, false);
}

void app_home_show_next_page(void) {
    if (is_fetching || end_of_list_reached) return;
    current_offset += ITEMS_PER_PAGE;
    fetch_and_display_page(current_offset, false);
}

void app_home_show_previous_page(void) {
    if (is_fetching || current_offset == 0) return;
    current_offset -= ITEMS_PER_PAGE;
    if (current_offset < 0) current_offset = 0;
    fetch_and_display_page(current_offset, true);
}

static void fetch_and_display_page(int offset, bool focus_last) {
    if (is_fetching) return;
    is_fetching = true;

    lv_obj_clean(list_container);
    lv_obj_t *spinner = lv_spinner_create(list_container);
    lv_obj_center(spinner);
    lv_refr_now(NULL);

    const char *query = lv_textarea_get_text(search_bar);
    int project_count = 0;
    project_t *projects = get_applications(&project_count, query, ITEMS_PER_PAGE, offset);

    lv_obj_clean(list_container);

    // Pass the search bar to the list view only if it's the first page
    create_app_list_view(list_container, projects, project_count, (offset == 0) ? search_bar : NULL);

    // Update page indicator
    int current_page = (offset / ITEMS_PER_PAGE) + 1;
    if (project_count < ITEMS_PER_PAGE) {
        end_of_list_reached = true;
        total_pages = current_page; // We now know the total number of pages
    } else {
        end_of_list_reached = false;
    }

    if (total_pages != -1) {
        lv_label_set_text_fmt(page_indicator_label, "Page %d / %d", current_page, total_pages);
    } else {
        lv_label_set_text_fmt(page_indicator_label, "Page %d / ?", current_page);
    }


    if (projects) {
        if (project_count > 0) {
            if (focus_last) {
                lv_group_focus_obj(lv_obj_get_child(list_container, lv_obj_get_child_cnt(list_container) - 1));
            } else {
                // If it's the first page, focus the search bar, otherwise focus the first card
                lv_group_focus_obj((offset == 0) ? search_bar : lv_obj_get_child(list_container, 0));
            }
        } else {
             lv_group_focus_obj(search_bar);
        }
        free_applications(projects, project_count);
    }

    is_fetching = false;
}

static void search_bar_key_event_cb(lv_event_t *e) {
    uint32_t key = lv_indev_get_key(lv_indev_active());
    if (key == LV_KEY_DOWN && lv_obj_get_child_cnt(list_container) > 1) { // >1 because search bar is child 0
        lv_obj_t* first_card = lv_obj_get_child(list_container, 1);
        lv_group_focus_obj(first_card);
        lv_obj_scroll_to_view(first_card, LV_ANIM_ON);
    }
}

static void search_timer_cb(lv_timer_t *timer) {
    printf("Search timer fired. Starting new search...\n");
    current_offset = 0;
    total_pages = -1; // Reset total pages on new search
    fetch_and_display_page(current_offset, false);
    search_timer = NULL;
}

static void search_bar_event_cb(lv_event_t *e) {
    if (search_timer) lv_timer_del(search_timer);
    search_timer = lv_timer_create(search_timer_cb, 1000, NULL);
    lv_timer_set_repeat_count(search_timer, 1);
}

static void home_view_delete_event_cb(lv_event_t *e) {
    if (search_timer) {
        lv_timer_del(search_timer);
        search_timer = NULL;
    }
    // Detach search bar from group to avoid use-after-free if it's deleted
    if (search_bar) {
        lv_group_remove_obj(search_bar);
        search_bar = NULL;
    }
}
