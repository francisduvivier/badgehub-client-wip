#include "app_data_manager.h"
#include "badgehub_client.h"
#include "app_list.h"
#include "app_card.h" // For card_user_data_t

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// --- CONSTANTS ---
#define ITEMS_PER_PAGE 20

// --- STATIC STATE VARIABLES ---
static int current_offset = 0;
static bool is_fetching = false;
static bool end_of_list_reached = false;
static int total_pages = -1;

// --- UI Pointers (set by the fetch function) ---
static lv_obj_t *current_list_container = NULL;
static lv_obj_t *current_page_indicator = NULL;
static lv_obj_t *current_search_bar = NULL;

void data_manager_init(void) {
    current_offset = 0;
    is_fetching = false;
    end_of_list_reached = false;
    total_pages = -1;
}

void data_manager_deinit(void) {
    // Nothing to do here currently, as memory is managed per-page.
}

void data_manager_request_next_page(void) {
    if (is_fetching || end_of_list_reached) return;
    current_offset += ITEMS_PER_PAGE;
    data_manager_fetch_page(current_list_container, current_page_indicator, current_search_bar, current_offset, false);
}

void data_manager_request_previous_page(void) {
    if (is_fetching || current_offset == 0) return;
    current_offset -= ITEMS_PER_PAGE;
    if (current_offset < 0) current_offset = 0;
    data_manager_fetch_page(current_list_container, current_page_indicator, current_search_bar, current_offset, true);
}

void data_manager_start_new_search(void) {
    current_offset = 0;
    total_pages = -1; // Reset total pages on a new search
    data_manager_fetch_page(current_list_container, current_page_indicator, current_search_bar, current_offset, false);
}

void data_manager_fetch_page(lv_obj_t* list_container, lv_obj_t* page_indicator, lv_obj_t* search_bar, int offset, bool focus_last) {
    if (is_fetching) return;
    is_fetching = true;

    // Store references to the UI elements we need to update
    current_list_container = list_container;
    current_page_indicator = page_indicator;
    current_search_bar = search_bar;
    current_offset = offset;

    // Show/hide search bar based on page
    if (offset == 0) {
        lv_obj_clear_flag(search_bar, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(search_bar, LV_OBJ_FLAG_HIDDEN);
    }

    lv_obj_clean(list_container);
    lv_obj_t *spinner = lv_spinner_create(list_container);
    lv_obj_center(spinner);
    lv_refr_now(NULL);

    const char *query = lv_textarea_get_text(search_bar);
    int project_count = 0;
    project_t *projects = get_applications(&project_count, query, ITEMS_PER_PAGE, offset);

    lv_obj_clean(list_container);
    create_app_list_view(list_container, projects, project_count);

    int current_page = (offset / ITEMS_PER_PAGE) + 1;
    if (project_count < ITEMS_PER_PAGE) {
        end_of_list_reached = true;
        total_pages = current_page;
    } else {
        end_of_list_reached = false;
    }

    if (total_pages != -1) {
        lv_label_set_text_fmt(page_indicator, "Page %d / %d", current_page, total_pages);
    } else {
        lv_label_set_text_fmt(page_indicator, "Page %d / ?", current_page);
    }

    if (projects) {
        if (project_count > 0) {
            lv_obj_t* target_to_focus = NULL;
            if (focus_last) {
                target_to_focus = lv_obj_get_child(list_container, project_count - 1);
                lv_obj_scroll_to_view(target_to_focus, LV_ANIM_OFF);
            } else {
                target_to_focus = (offset == 0) ? search_bar : lv_obj_get_child(list_container, 0);
            }
            lv_group_focus_obj(target_to_focus);
        } else {
             lv_group_focus_obj(search_bar);
        }
        free_applications(projects, project_count);
    }

    is_fetching = false;
}
