#include "app_home.h"
#include "app_list.h"
#include "badgehub_client.h"
#include "lvgl/lvgl.h"
#include <stdio.h>

// --- STATIC VARIABLES ---
static lv_obj_t *list_container;
static lv_obj_t *search_bar;
static lv_timer_t *search_timer = NULL;

// --- FORWARD DECLARATIONS ---
static void search_timer_cb(lv_timer_t *timer);
static void search_bar_text_event_cb(lv_event_t *e);
static void search_bar_key_event_cb(lv_event_t *e); // New handler for key presses
static void refresh_app_list(void);
static void home_view_delete_event_cb(lv_event_t *e);

// --- IMPLEMENTATIONS ---

// Getter function for other modules to access the search bar
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
    lv_textarea_set_placeholder_text(search_bar, "Search for apps...");
    lv_obj_add_event_cb(search_bar, search_bar_text_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(search_bar, search_bar_key_event_cb, LV_EVENT_KEY, NULL); // Add key handler

    list_container = lv_obj_create(main_container);
    lv_obj_set_width(list_container, lv_pct(100));
    lv_obj_set_flex_grow(list_container, 1);
    lv_obj_set_flex_flow(list_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(list_container, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // Add the search bar to the default input group to make it focusable
    lv_group_t * g = lv_group_get_default();
    lv_group_add_obj(g, search_bar);

    refresh_app_list();
}

static void refresh_app_list(void) {
    const char *query = lv_textarea_get_text(search_bar);
    lv_obj_clean(list_container);

    lv_obj_t *spinner = lv_spinner_create(list_container);
    lv_obj_center(spinner);
    lv_refr_now(NULL);

    int project_count = 0;
    project_t *projects = get_applications(&project_count, query);

    lv_obj_del(spinner);
    create_app_list_view(list_container, projects, project_count);

    if (projects) {
        free_applications(projects, project_count);
    }
}

static void search_timer_cb(lv_timer_t *timer) {
    printf("Search timer fired. Refreshing list...\n");
    refresh_app_list();
    search_timer = NULL;
}

static void search_bar_text_event_cb(lv_event_t *e) {
    if (search_timer) {
        lv_timer_del(search_timer);
    }
    search_timer = lv_timer_create(search_timer_cb, 1000, NULL);
    lv_timer_set_repeat_count(search_timer, 1);
}

// New event handler to move focus from the search bar to the list
static void search_bar_key_event_cb(lv_event_t *e) {
    uint32_t key = lv_indev_get_key(lv_indev_active());
    if (key == LV_KEY_DOWN) {
        // If there are items in the list, focus the first one
        if (lv_obj_get_child_cnt(list_container) > 0) {
            lv_obj_t * first_card = lv_obj_get_child(list_container, 0);
            lv_group_focus_obj(first_card);
        }
    }
}

static void home_view_delete_event_cb(lv_event_t *e) {
    if (search_timer) {
        lv_timer_del(search_timer);
        search_timer = NULL;
    }
}
