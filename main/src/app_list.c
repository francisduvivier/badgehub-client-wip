#include "app_list.h"
#include "badgehub_client.h"
#include "lvgl/lvgl.h"
#include <stdio.h>

/**
 * @brief Creates the main application view.
 *
 * This function is responsible for creating the initial user interface
 * that fetches and displays the project count from BadgeHub.
 */
void create_app_list_view(void) {
    /* Create a label that will be updated with the project count */
    lv_obj_t *label = lv_label_create(lv_screen_active());
    lv_label_set_text(label, "Fetching data...");
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

    /* Force a redraw to show the "Fetching data..." message immediately */
    lv_refr_now(NULL);

    /* Fetch the applications from the BadgeHub API */
    int project_count = 0;
    project_t *projects = get_applications(&project_count);

    /* Prepare the text to display */
    char display_text[128];
    if (projects != NULL) {
        printf("Successfully fetched %d projects.\n", project_count);
        snprintf(display_text, sizeof(display_text), "BadgeHub Projects: %d", project_count);
        /* We are done with the data, so free it */
        free_applications(projects, project_count);
    } else {
        printf("Failed to fetch projects.\n");
        snprintf(display_text, sizeof(display_text), "Error: Failed to fetch projects.");
    }

    /* Update the label with the result */
    lv_label_set_text(label, display_text);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
}
