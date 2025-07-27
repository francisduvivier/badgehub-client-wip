#include "app_list.h"
#include "app_card.h"
#include <stdio.h>

void create_app_list_view(lv_obj_t* parent, project_t* projects, int project_count) {
    // This function is now purely for rendering cards.
    if (projects != NULL && project_count > 0) {
        for (int i = 0; i < project_count; i++) {
            create_app_card(parent, &projects[i]);
        }
    } else {
        lv_obj_t* label = lv_label_create(parent);
        lv_label_set_text(label, "No applications found.");
        lv_obj_center(label);
    }
}
