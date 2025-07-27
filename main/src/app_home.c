#include "app_home.h"
#include "app_list.h"
#include "badgehub_client.h"
#include "app_card.h"
#include "lvgl/lvgl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// --- CONSTANTS ---
#define ITEMS_PER_PAGE 7

// --- STATIC STATE VARIABLES ---
static lv_obj_t *list_container;
static lv_obj_t *search_bar;
static lv_obj_t *page_indicator_label;
static lv_timer_t *search_timer = NULL;
static lv_timer_t *icon_loader_timer = NULL; // Timer to orchestrate icon downloads
static int icon_loader_index = 0; // Which icon to download next

static project_t *s_current_page_projects = NULL;
static int s_current_page_project_count = 0;

static int current_offset = 0;
static bool is_fetching = false;
static bool end_of_list_reached = false;
static int total_pages = -1;

// --- FORWARD DECLARATIONS ---
static void search_timer_cb(lv_timer_t *timer);
static void search_bar_event_cb(lv_event_t *e);
static void search_bar_key_event_cb(lv_event_t *e);
static void home_view_delete_event_cb(lv_event_t *e);
static void fetch_and_display_page(int offset, bool focus_last);
static void icon_loader_timer_cb(lv_timer_t *timer);

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

    search_bar = lv_textarea_create(main_container);
    lv_obj_set_width(search_bar, lv_pct(95));
    lv_textarea_set_one_line(search_bar, true);
    lv_textarea_set_placeholder_text(search_bar, "Start typing to search");
    lv_obj_add_event_cb(search_bar, search_bar_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(search_bar, search_bar_key_event_cb, LV_EVENT_KEY, NULL);
    lv_group_add_obj(lv_group_get_default(), search_bar);

    list_container = lv_obj_create(main_container);
    lv_obj_set_width(list_container, lv_pct(100));
    lv_obj_set_flex_grow(list_container, 1);
    lv_obj_set_flex_flow(list_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(list_container, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

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

void app_home_focus_search_and_start_typing(uint32_t key) {
    if (!search_bar) return;
    current_offset = 0;
    total_pages = -1;
    lv_group_focus_obj(search_bar);
    lv_textarea_add_char(search_bar, key);
    search_bar_event_cb(NULL);
}

static void fetch_and_display_page(int offset, bool focus_last) {
    if (is_fetching) return;
    is_fetching = true;

    if (icon_loader_timer) {
        lv_timer_del(icon_loader_timer);
        icon_loader_timer = NULL;
    }

    lv_obj_clean(list_container);
    lv_obj_t *spinner = lv_spinner_create(list_container);
    lv_obj_center(spinner);
    lv_refr_now(NULL);

    if (s_current_page_projects) {
        free_applications(s_current_page_projects, s_current_page_project_count);
        s_current_page_projects = NULL;
    }

    const char *query = lv_textarea_get_text(search_bar);
    s_current_page_projects = get_applications(&s_current_page_project_count, query, ITEMS_PER_PAGE, offset);

    lv_obj_clean(list_container);
    create_app_list_view(list_container, s_current_page_projects, s_current_page_project_count);

    int current_page = (offset / ITEMS_PER_PAGE) + 1;
    if (s_current_page_project_count < ITEMS_PER_PAGE) {
        end_of_list_reached = true;
        total_pages = current_page;
    } else {
        end_of_list_reached = false;
    }

    if (total_pages != -1) {
        lv_label_set_text_fmt(page_indicator_label, "Page %d / %d", current_page, total_pages);
    } else {
        lv_label_set_text_fmt(page_indicator_label, "Page %d / ?", current_page);
    }

    if (s_current_page_projects && s_current_page_project_count > 0) {
        lv_obj_t* target_to_focus = NULL;
        if (focus_last) {
            target_to_focus = lv_obj_get_child(list_container, s_current_page_project_count - 1);
            lv_obj_scroll_to_view(target_to_focus, LV_ANIM_OFF);
        } else {
            target_to_focus = search_bar;
        }
        lv_group_focus_obj(target_to_focus);

        // --- NEW: Start the sequential icon loader ---
        icon_loader_index = 0;
        icon_loader_timer = lv_timer_create(icon_loader_timer_cb, 10, NULL);
    } else {
         lv_group_focus_obj(search_bar);
    }

    is_fetching = false;
}

static void icon_loader_timer_cb(lv_timer_t *timer) {
    if (is_fetching) return; // Don't load icons while fetching a new page

    if (icon_loader_index >= s_current_page_project_count) {
        lv_timer_del(icon_loader_timer); // All icons loaded for this page
        icon_loader_timer = NULL;
        return;
    }

    lv_obj_t* card = lv_obj_get_child(list_container, icon_loader_index);
    if (card) {
        app_card_load_icon(card); // Tell the card to download its icon
    }

    icon_loader_index++;
}

static void search_bar_key_event_cb(lv_event_t *e) {
    uint32_t key = lv_indev_get_key(lv_indev_active());
    if (key == LV_KEY_DOWN && lv_obj_get_child_cnt(list_container) > 0) {
        lv_obj_t* first_card = lv_obj_get_child(list_container, 0);
        lv_group_focus_obj(first_card);
        lv_obj_scroll_to_view(first_card, LV_ANIM_ON);
    }
}

static void search_timer_cb(lv_timer_t *timer) {
    printf("Search timer fired. Starting new search...\n");
    current_offset = 0;
    total_pages = -1;
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
    if (icon_loader_timer) {
        lv_timer_del(icon_loader_timer);
        icon_loader_timer = NULL;
    }
    if (s_current_page_projects) {
        free_applications(s_current_page_projects, s_current_page_project_count);
        s_current_page_projects = NULL;
    }
    search_bar = NULL;
}
