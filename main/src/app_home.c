#include "app_home.h"
#include "app_list.h"
#include "lvgl/lvgl.h"
#include <stdio.h>

#include "badgehub_client.h"

// --- STATIC VARIABLES ---
static lv_obj_t *list_container; // A container to hold the app list
static lv_obj_t *search_bar;     // The text area for search input
static lv_timer_t *search_timer; // A timer to delay searching while the user types

// --- FORWARD DECLARATIONS ---
static void search_timer_cb(lv_timer_t *timer);
static void search_bar_event_cb(lv_event_t *e);
static void refresh_app_list(void);

// --- IMPLEMENTATIONS ---

/**
 * @brief Creates the main home screen UI.
 */
void create_app_home_view(void) {
    lv_obj_clean(lv_screen_active());

    // Create a main container to hold the search bar and the list
    lv_obj_t *main_container = lv_obj_create(lv_screen_active());
    lv_obj_set_size(main_container, lv_pct(100), lv_pct(100));
    lv_obj_set_layout(main_container, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(main_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_center(main_container);

    // Create the search bar (a text area)
    search_bar = lv_textarea_create(main_container);
    lv_obj_set_width(search_bar, lv_pct(95));
    lv_textarea_set_one_line(search_bar, true);
    lv_textarea_set_placeholder_text(search_bar, "Search for apps...");
    lv_obj_add_event_cb(search_bar, search_bar_event_cb, LV_EVENT_ALL, NULL);

    // Create the container that will hold the list of cards
    list_container = lv_obj_create(main_container);
    lv_obj_set_width(list_container, lv_pct(100));
    lv_obj_set_flex_grow(list_container, 1); // Allow the list to fill remaining space
    lv_obj_set_flex_flow(list_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(list_container, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // Create the one-shot timer for handling search, but don't start it yet
    search_timer = lv_timer_create(search_timer_cb, 1000, NULL); // 1000ms delay
    lv_timer_set_repeat_count(search_timer, 1);
    lv_timer_pause(search_timer);

    // Perform the initial data load
    refresh_app_list();
}

/**
 * @brief Fetches projects based on the search bar and redraws the list.
 */
static void refresh_app_list(void) {
    const char *query = lv_textarea_get_text(search_bar);

    // Clear the old list before drawing the new one
    lv_obj_clean(list_container);

    // Show a loading indicator
    lv_obj_t *spinner = lv_spinner_create(list_container);
    lv_obj_center(spinner);
    lv_refr_now(NULL);

    // Fetch the data
    int project_count = 0;
    project_t *projects = get_applications(&project_count, query);

    lv_obj_del(spinner); // Remove the loading indicator

    // Pass the fetched data to the app_list module to render
    create_app_list_view(list_container, projects, project_count);

    // Clean up the data now that the list is rendered
    if (projects) {
        free_applications(projects, project_count);
    }
}

/**
 * @brief Callback for the search timer. Triggers the list refresh.
 */
static void search_timer_cb(lv_timer_t *timer) {
    printf("Search timer fired. Refreshing list...\n");
    refresh_app_list();
}

/**
 * @brief Event handler for the search bar. Restarts the search timer on every key press.
 */
static void search_bar_event_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_VALUE_CHANGED) {
        // Reset and resume the timer every time the user types
        lv_timer_reset(search_timer);
        lv_timer_resume(search_timer);
    }
}
