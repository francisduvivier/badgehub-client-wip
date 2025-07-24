#include "app_detail.h"
#include "app_list.h" // Needed for the back button
#include "badgehub_client.h"
#include "lvgl/lvgl.h"
#include <stdio.h>
#include <string.h> // Required for strncpy

static void back_button_event_handler(lv_event_t * e) {
    // When the back button is clicked, recreate the app list view
    create_app_list_view();
}

void create_app_detail_view(const char* slug) {
    // --- FIX ---
    // Make a local copy of the slug *before* cleaning the screen.
    // Cleaning the screen will free the memory of the original 'slug' pointer
    // via the card's delete event handler.
    char local_slug[256];
    if (slug) {
        strncpy(local_slug, slug, sizeof(local_slug) - 1);
        local_slug[sizeof(local_slug) - 1] = '\0'; // Ensure null-termination
    } else {
        // If slug is somehow null, prevent a crash.
        local_slug[0] = '\0';
    }

    // Clear the screen before building the new view
    lv_obj_clean(lv_screen_active());

    // Create a container for the detail page
    lv_obj_t* container = lv_obj_create(lv_screen_active());
    lv_obj_set_size(container, lv_pct(100), lv_pct(100));
    lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(container, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);

    // Add a "Back" button
    lv_obj_t* btn_back = lv_btn_create(container);
    lv_obj_add_event_cb(btn_back, back_button_event_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_t* label_back = lv_label_create(btn_back);
    lv_label_set_text(label_back, "Back to List");
    lv_obj_set_style_margin_bottom(btn_back, 10, 0);


    // Show a loading message while we fetch data
    lv_obj_t* loading_label = lv_label_create(container);
    lv_label_set_text(loading_label, "Loading details...");
    lv_obj_align(loading_label, LV_ALIGN_CENTER, 0, 0);
    lv_refr_now(NULL); // Force redraw

    // Fetch the project details using our safe, local copy of the slug
    project_detail_t* details = get_project_details(local_slug);

    // We don't need the loading label anymore
    lv_obj_del(loading_label);

    if (details) {
        // Create labels to display the details
        lv_obj_t* title_label = lv_label_create(container);
        lv_label_set_text_fmt(title_label, "Name: %s", details->name);
        lv_obj_set_width(title_label, lv_pct(95));
        lv_label_set_long_mode(title_label, LV_LABEL_LONG_WRAP);

        lv_obj_t* desc_label = lv_label_create(container);
        lv_label_set_text_fmt(desc_label, "Description: %s", details->description);
        lv_obj_set_width(desc_label, lv_pct(95));
        lv_label_set_long_mode(desc_label, LV_LABEL_LONG_WRAP);
        lv_obj_set_style_margin_top(desc_label, 10, 0);

        lv_obj_t* date_label = lv_label_create(container);
        lv_label_set_text_fmt(date_label, "Published: %s", details->published_at);
        lv_obj_set_width(date_label, lv_pct(95));
        lv_label_set_long_mode(date_label, LV_LABEL_LONG_WRAP);
        lv_obj_set_style_margin_top(date_label, 10, 0);

        // Clean up the allocated memory for the details
        free_project_details(details);
    } else {
        // Show an error message if fetching failed
        lv_obj_t* error_label = lv_label_create(container);
        lv_label_set_text(error_label, "Failed to load project details.");
    }
}
