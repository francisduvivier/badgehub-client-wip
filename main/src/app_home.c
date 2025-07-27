#include "app_home.h"
#include "app_list.h"
#include "app_data_manager.h"
#include "lvgl/lvgl.h"
#include <stdio.h>

// --- STATIC UI VARIABLES ---
static lv_obj_t *list_container;
static lv_obj_t *search_bar;
static lv_obj_t *page_indicator_label;
static lv_timer_t *search_timer = NULL;

// --- FORWARD DECLARATIONS ---
static void search_timer_cb(lv_timer_t *timer);
static void search_bar_event_cb(lv_event_t *e);
static void search_bar_key_event_cb(lv_event_t *e);
static void home_view_delete_event_cb(lv_event_t *e);

// --- IMPLEMENTATIONS ---

lv_obj_t* get_search_bar(void) {
    return search_bar;
}

/**
 * @brief Implements the function to focus the search bar and start typing.
 */
void app_home_focus_search_and_start_typing(uint32_t key) {
    if (search_bar) {
        // Focus the search bar
        lv_group_focus_obj(search_bar);

        // Convert the key to a string
        char key_str[2] = {(char)key, '\0'};

        // Clear existing text and add the new character
        lv_textarea_set_text(search_bar, key_str);

        // Move the cursor to the end of the text
        lv_textarea_set_cursor_pos(search_bar, lv_textarea_get_cursor_pos(search_bar) + 1);
    }
}


void create_app_home_view(void) {
    lv_obj_clean(lv_screen_active());
    data_manager_init();

    lv_obj_t *main_container = lv_obj_create(lv_screen_active());
    lv_obj_set_size(main_container, lv_pct(100), lv_pct(100));
    lv_obj_set_layout(main_container, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(main_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_center(main_container);
    lv_obj_add_event_cb(main_container, home_view_delete_event_cb, LV_EVENT_DELETE, NULL);

    search_bar = lv_textarea_create(main_container);
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

    page_indicator_label = lv_label_create(main_container);
    lv_obj_set_width(page_indicator_label, lv_pct(95));
    lv_obj_set_style_text_align(page_indicator_label, LV_TEXT_ALIGN_CENTER, 0);

    data_manager_fetch_page(list_container, page_indicator_label, search_bar, 0, false);
}

static void search_bar_key_event_cb(lv_event_t *e) {
    uint32_t key = lv_indev_get_key(lv_indev_active());
    if (key == LV_KEY_DOWN && lv_obj_get_child_cnt(list_container) > 0) {
        lv_obj_t* first_card = lv_obj_get_child(list_container, 0);
        lv_group_focus_obj(first_card);
        lv_obj_scroll_to_view(first_card, LV_ANIM_ON);
    } else if (key == LV_KEY_UP) {
        data_manager_request_previous_page();
    }
}

static void search_timer_cb(lv_timer_t *timer) {
    printf("Search timer fired. Starting new search...\n");
    data_manager_start_new_search();
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
    data_manager_deinit();
    search_bar = NULL;
}
