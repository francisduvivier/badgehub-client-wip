#include "app_list.h"
#include "app_card.h"
#include <stdio.h>

void create_app_list_view(lv_obj_t* parent, project_t* projects, int project_count, lv_obj_t* search_bar_to_add) {
    // If a search bar is provided, add it as the first item in the list
    if (search_bar_to_add) {
        lv_obj_set_parent(search_bar_to_add, parent);
        lv_obj_move_to_index(search_bar_to_add, 0);
    }

    if (projects != NULL && project_count > 0) {
        for (int i = 0; i < project_count; i++) {
            create_app_card(parent, &projects[i]);
        }
    } else if (!search_bar_to_add) { // Only show "not found" if there's no search bar
        lv_obj_t* label = lv_label_create(parent);
        lv_label_set_text(label, "No applications found.");
        lv_obj_center(label);
    }
}
