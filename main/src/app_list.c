#include "app_list.h"
#include "app_card.h"
#include <stdio.h>

void create_app_list_view(lv_obj_t* parent, project_t* projects, int project_count) {
    if (projects != NULL && project_count > 0) {
        // Loop through the provided projects and create a card for each one
        for (int i = 0; i < project_count; i++) {
            create_app_card(parent, &projects[i]);
        }
    } else {
        // If there's no data, display a message
        lv_obj_t* label = lv_label_create(parent);
        lv_label_set_text(label, "No applications found.");
        lv_obj_center(label);
    }
}
