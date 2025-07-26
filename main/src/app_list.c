#include "app_list.h"
#include "app_card.h" // We now include the card module
#include "badgehub_client/badgehub_client.h"
#include "lvgl/lvgl.h"
#include <stdio.h>

void create_app_list_view(void) {
    /* Create a scrollable container for the list of cards */
    lv_obj_t* list = lv_obj_create(lv_screen_active());
    lv_obj_set_size(list, lv_pct(100), lv_pct(100));
    lv_obj_center(list);
    lv_obj_set_flex_flow(list, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(list, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    /* Fetch the applications from the BadgeHub API */
    int project_count = 0;
    project_t* projects = get_applications(&project_count);

    if (projects != NULL && project_count > 0) {
        printf("Successfully fetched %d projects. Creating cards...\n", project_count);

        /* Loop through the projects and create a card for each one */
        for (int i = 0; i < project_count; i++) {
            create_app_card(list, &projects[i]);
        }

        /* We are done with the data, so free it */
        free_applications(projects, project_count);
    } else {
        /* If fetching fails, display an error message */
        printf("Failed to fetch projects.\n");
        lv_obj_t* label = lv_label_create(list);
        lv_label_set_text(label, "Error: Could not fetch projects from BadgeHub.");
        lv_obj_center(label); // Center the label within the list container
    }
}
