#include "app_home.h"
#include "app_list.h"
#include "badgehub_client.h"
#include "app_card.h" // Include the header with the type definition
#include "lvgl/lvgl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// --- STATIC STATE VARIABLES ---
static lv_obj_t *list_container;
static lv_obj_t *search_bar;
static lv_timer_t *search_timer = NULL;

static project_t *loaded_projects = NULL;
static int loaded_project_count = 0;
static int current_offset = 0;
static bool end_of_list_reached = false;
static bool is_fetching = false;

// --- FORWARD DECLARATIONS ---
static void search_timer_cb(lv_timer_t *timer);
static void search_bar_event_cb(lv_event_t *e);
static void home_view_delete_event_cb(lv_event_t *e);
static void reset_and_fetch_apps(void);

// --- IMPLEMENTATIONS ---

lv_obj_t* get_search_bar(void) {
    return search_bar;
}

void create_app_home_view(void) {
    lv_obj_clean(lv_screen_active());

    lv_obj_t *main_container = lv_obj_create(lv_screen_active());
    lv_obj_set_size(main_container, lv_pct(100), lv_pct(100));
    lv_obj_set_layout(main_container, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(main_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_center(main_container);
    lv_obj_add_event_cb(main_container, home_view_delete_event_cb, LV_EVENT_DELETE, NULL);

    search_bar = lv_textarea_create(main_container);
    lv_obj_set_width(search_bar, lv_pct(95));
    lv_textarea_set_one_line(search_bar, true);
    lv_textarea_set_placeholder_text(search_bar, "Search for apps...");
    lv_obj_add_event_cb(search_bar, search_bar_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    list_container = lv_obj_create(main_container);
    lv_obj_set_width(list_container, lv_pct(100));
    lv_obj_set_flex_grow(list_container, 1);
    lv_obj_set_flex_flow(list_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(list_container, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_group_add_obj(lv_group_get_default(), search_bar);

    reset_and_fetch_apps();
}

void app_home_fetch_more(void) {
    if (is_fetching || end_of_list_reached) return;
    is_fetching = true;

    char* focused_slug = NULL;
    lv_obj_t* focused_card = lv_group_get_focused(lv_group_get_default());
    if (focused_card && lv_obj_get_parent(focused_card) == list_container) {
        // This now compiles because card_user_data_t is a known type
        card_user_data_t* user_data = lv_obj_get_user_data(focused_card);
        if (user_data) {
            focused_slug = strdup(user_data->slug);
        }
    }

    const char *query = lv_textarea_get_text(search_bar);
    int limit = (current_offset == 0) ? 20 : 10;
    int new_count = 0;

    project_t *new_projects = get_applications(&new_count, query, limit, current_offset);

    if (new_projects && new_count > 0) {
        int old_count = loaded_project_count;
        loaded_project_count += new_count;
        loaded_projects = realloc(loaded_projects, loaded_project_count * sizeof(project_t));

        for (int i = 0; i < new_count; i++) {
            loaded_projects[old_count + i] = new_projects[i];
        }
        free(new_projects);

        current_offset += new_count;
        if (new_count < limit) {
            end_of_list_reached = true;
        }
    } else {
        end_of_list_reached = true;
        if (new_projects) free(new_projects);
    }

    lv_obj_clean(list_container);
    create_app_list_view(list_container, loaded_projects, loaded_project_count);

    if (focused_slug) {
        for (int i = 0; i < lv_obj_get_child_cnt(list_container); i++) {
            lv_obj_t* card = lv_obj_get_child(list_container, i);
            card_user_data_t* user_data = lv_obj_get_user_data(card);
            if (user_data && strcmp(user_data->slug, focused_slug) == 0) {
                lv_group_focus_obj(card);
                break;
            }
        }
        free(focused_slug);
    }

    is_fetching = false;
}

static void reset_and_fetch_apps(void) {
    if (loaded_projects) {
        free_applications(loaded_projects, loaded_project_count);
        loaded_projects = NULL;
    }
    loaded_project_count = 0;
    current_offset = 0;
    end_of_list_reached = false;
    is_fetching = false;

    lv_obj_clean(list_container);
    lv_obj_t *spinner = lv_spinner_create(list_container);
    lv_obj_center(spinner);
    lv_refr_now(NULL);

    app_home_fetch_more();
}

static void search_timer_cb(lv_timer_t *timer) {
    printf("Search timer fired. Starting new search...\n");
    reset_and_fetch_apps();
    search_timer = NULL;
}

static void search_bar_event_cb(lv_event_t *e) {
    if (search_timer) {
        lv_timer_del(search_timer);
    }
    search_timer = lv_timer_create(search_timer_cb, 1000, NULL);
    lv_timer_set_repeat_count(search_timer, 1);
}

static void home_view_delete_event_cb(lv_event_t *e) {
    if (search_timer) {
        lv_timer_del(search_timer);
        search_timer = NULL;
    }
    if (loaded_projects) {
        free_applications(loaded_projects, loaded_project_count);
        loaded_projects = NULL;
        loaded_project_count = 0;
    }
}
