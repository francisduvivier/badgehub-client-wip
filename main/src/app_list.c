#include "app_list.h"
#include "app_card.h"
#include "app_home.h" // Needed to get the search bar
#include <stdio.h>

// --- FORWARD DECLARATION ---
static void list_key_event_handler(lv_event_t *e);

// --- IMPLEMENTATION ---

void create_app_list_view(lv_obj_t* parent, project_t* projects, int project_count) {
    // Add the key event handler to the parent list container
    lv_obj_add_event_cb(parent, list_key_event_handler, LV_EVENT_KEY, NULL);

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

/**
 * @brief Handles key events for the entire list, managing focus navigation.
 */
static void list_key_event_handler(lv_event_t *e) {
    uint32_t key = lv_indev_get_key(lv_indev_active());
    lv_group_t *g = lv_group_get_default();
    lv_obj_t *focused = lv_group_get_focused(g);
    lv_obj_t *list = lv_event_get_target(e);

    // Ensure the focused object is actually a card within this list
    if (!focused || lv_obj_get_parent(focused) != list) {
        return;
    }

    if (key == LV_KEY_UP) {
        // If the focused card is the first one, jump to the search bar
        if (lv_obj_get_index(focused) == 0) {
            lv_group_focus_obj(get_search_bar());
        } else {
            // This is more reliable than lv_group_focus_prev() and behaves like Shift+Tab.
            lv_group_send_data(g, LV_KEY_PREV);
        }
    } else if (key == LV_KEY_DOWN) {
        // If the focused card is the last one, jump to the search bar
        if (lv_obj_get_index(focused) == lv_obj_get_child_cnt(list) - 1) {
            lv_group_focus_obj(get_search_bar());
        } else {
            // This makes the DOWN key behave exactly like the TAB key for focus changes.
            lv_group_send_data(g, LV_KEY_NEXT);
        }
    }
}
