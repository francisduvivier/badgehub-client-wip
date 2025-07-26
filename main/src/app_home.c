#include "app_home.h"
#include "app_list.h"
#include "badgehub_client.h"
#include "lvgl/lvgl.h"
#include <stdio.h>

// --- STATIC VARIABLES ---
static lv_obj_t *list_container;
static lv_obj_t *search_bar;
static lv_timer_t *search_timer = NULL; // Initialize to NULL to be safe

// --- FORWARD DECLARATIONS ---
static void search_timer_cb(lv_timer_t *timer);
static void search_bar_event_cb(lv_event_t *e);
static void refresh_app_list(void);
static void home_view_delete_event_cb(lv_event_t *e);

// --- IMPLEMENTATIONS ---

void create_app_home_view(void) {
    lv_obj_clean(lv_screen_active());

    lv_obj_t *main_container = lv_obj_create(lv_screen_active());
    lv_obj_set_size(main_container, lv_pct(100), lv_pct(100));
    lv_obj_set_layout(main_container, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(main_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_center(main_container);
    // Add a delete event to clean up the timer if we navigate away from this screen
    lv_obj_add_event_cb(main_container, home_view_delete_event_cb, LV_EVENT_DELETE, NULL);

    search_bar = lv_textarea_create(main_container);
    lv_obj_set_width(search_bar, lv_pct(95));
    lv_textarea_set_one_line(search_bar, true);
    lv_textarea_set_placeholder_text(search_bar, "Search for apps...");
    // Only trigger on value changed to be more efficient
    lv_obj_add_event_cb(search_bar, search_bar_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    list_container = lv_obj_create(main_container);
    lv_obj_set_width(list_container, lv_pct(100));
    lv_obj_set_flex_grow(list_container, 1);
    lv_obj_set_flex_flow(list_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(list_container, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // No need to create the timer here, it will be created on the first keypress

    refresh_app_list();
}

static void refresh_app_list(void) {
    const char *query = lv_textarea_get_text(search_bar);
    lv_obj_clean(list_container);

    // CORRECTED: Use the correct function signature for lv_spinner_create
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

// Callback for the search timer.
static void search_timer_cb(lv_timer_t *timer) {
    printf("Search timer fired. Refreshing list...\n");
    refresh_app_list();
    // After firing, LVGL deletes the timer, so we MUST set our pointer to NULL
    search_timer = NULL;
}

// Event handler for the search bar.
static void search_bar_event_cb(lv_event_t *e) {
    // --- FIX ---
    // This is the robust way to handle a "debounce" timer.
    // Always delete the old timer if it exists.
    if (search_timer) {
        lv_timer_del(search_timer);
    }
    // Then create a new one.
    search_timer = lv_timer_create(search_timer_cb, 1000, NULL); // 1000ms delay
    lv_timer_set_repeat_count(search_timer, 1); // Make it a one-shot timer
}

// This is a safety net. If the home screen is deleted (e.g., when navigating
// to the detail page), we must ensure the timer is also deleted.
static void home_view_delete_event_cb(lv_event_t *e) {
    if (search_timer) {
        lv_timer_del(search_timer);
        search_timer = NULL;
    }
}
