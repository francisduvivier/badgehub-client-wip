#include "app_home.h"
#include "app_list.h"
#include "app_data_manager.h" // New include
#include "app_card.h"
#include "lvgl/lvgl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// --- STATIC STATE VARIABLES ---
static lv_obj_t *list_container;
static lv_obj_t *search_bar;
static lv_obj_t *page_indicator_label;
static lv_timer_t *search_timer = NULL;
static bool is_fetching = false;

// --- FORWARD DECLARATIONS ---
static void search_timer_cb(lv_timer_t *timer);
static void search_bar_event_cb(lv_event_t *e);
static void home_view_delete_event_cb(lv_event_t *e);
static void home_key_event_handler(lv_event_t *e);
static void redraw_page(bool focus_last);

// --- IMPLEMENTATIONS ---

lv_obj_t* get_search_bar(void) {
    return search_bar;
}

void create_app_home_view(void) {
    data_manager_init();
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
    lv_group_add_obj(lv_group_get_default(), search_bar);

    list_container = lv_obj_create(main_container);
    lv_obj_set_width(list_container, lv_pct(100));
    lv_obj_set_flex_grow(list_container, 1);
    lv_obj_set_flex_flow(list_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(list_container, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // The single key handler is now attached to the main container
    lv_obj_add_event_cb(main_container, home_key_event_handler, LV_EVENT_KEY, NULL);

    page_indicator_label = lv_label_create(main_container);
    lv_obj_set_width(page_indicator_label, lv_pct(95));
    lv_obj_set_style_text_align(page_indicator_label, LV_TEXT_ALIGN_CENTER, 0);

    data_manager_reset_and_load_first_page();
    redraw_page(false);
}

static void redraw_page(bool focus_last) {
    is_fetching = true;

    lv_obj_clean(list_container);
    create_app_list_view(list_container, data_manager_get_projects(), data_manager_get_project_count());

    lv_label_set_text_fmt(page_indicator_label, "Page %d", data_manager_get_current_page_num());

    int child_count = lv_obj_get_child_cnt(list_container);
    if (child_count > 0) {
        if (focus_last) {
            lv_obj_t* target = lv_obj_get_child(list_container, child_count - 1);
            lv_group_focus_obj(target);
            lv_obj_scroll_to_view(target, LV_ANIM_OFF);
        } else {
            lv_group_focus_obj(search_bar);
        }
    } else {
        lv_group_focus_obj(search_bar);
    }
    is_fetching = false;
}

static void home_key_event_handler(lv_event_t *e) {
    if (is_fetching) return;

    uint32_t key = lv_indev_get_key(lv_indev_active());
    lv_obj_t* focused = lv_group_get_focused(lv_group_get_default());

    if (focused == search_bar) {
        if (key == LV_KEY_DOWN && lv_obj_get_child_cnt(list_container) > 0) {
            lv_obj_t* first_card = lv_obj_get_child(list_container, 0);
            lv_group_focus_obj(first_card);
            lv_obj_scroll_to_view(first_card, LV_ANIM_ON);
        }
    } else if (focused && lv_obj_get_parent(focused) == list_container) {
        uint32_t idx = lv_obj_get_index(focused);
        if (key == LV_KEY_UP) {
            if (idx == 0) {
                if (data_manager_load_previous_page()) {
                    redraw_page(true);
                } else {
                    lv_group_focus_obj(search_bar);
                }
            } else {
                lv_group_focus_prev(lv_group_get_default());
                lv_obj_scroll_to_view(lv_group_get_focused(lv_group_get_default()), LV_ANIM_ON);
            }
        } else if (key == LV_KEY_DOWN) {
            if (idx == lv_obj_get_child_cnt(list_container) - 1) {
                if (data_manager_load_next_page()) {
                    redraw_page(false);
                }
            } else {
                lv_group_focus_next(lv_group_get_default());
                lv_obj_scroll_to_view(lv_group_get_focused(lv_group_get_default()), LV_ANIM_ON);
            }
        } else if (key >= ' ' && key < LV_KEY_DEL) {
            lv_group_focus_obj(search_bar);
            lv_textarea_add_char(search_bar, key);
        }
    }
}

static void search_timer_cb(lv_timer_t *timer) {
    data_manager_set_search_query(lv_textarea_get_text(search_bar));
    data_manager_reset_and_load_first_page();
    redraw_page(false);
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
