#include "app_home.h"
#include "app_list.h"
#include "badgehub_client.h"
#include "app_card.h"
#include "lvgl/lvgl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// --- CONSTANTS ---
#define INITIAL_LOAD_LIMIT 10
#define SCROLL_FETCH_LIMIT 5
#define PRUNE_THRESHOLD 10
#define PRUNE_AMOUNT 5

// --- STATIC STATE VARIABLES ---
static lv_obj_t *list_container;
static lv_obj_t *search_bar;
static lv_timer_t *search_timer = NULL;

static project_t *loaded_projects = NULL;
static int loaded_project_count = 0;
static int list_start_offset = 0;
static bool end_of_list_reached = false;
static bool is_fetching = false;

// --- FORWARD DECLARATIONS ---
static void search_timer_cb(lv_timer_t *timer);
static void search_bar_event_cb(lv_event_t *e);
static void search_bar_key_event_cb(lv_event_t *e); // For navigating out of search
static void home_view_delete_event_cb(lv_event_t *e);
static void reset_and_fetch_initial(void);
static void fetch_more_apps(bool fetch_at_end);
static void redraw_list_and_preserve_focus(void);

// --- IMPLEMENTATIONS ---

lv_obj_t* get_search_bar(void) {
    return search_bar;
}

/**
 * @brief Public function to trigger fetching the next page of applications.
 */
void app_home_fetch_more(void) {
    // This is the public-facing function called from the outside (e.g., app_card).
    // It always fetches new items to append to the end of the current list.
    fetch_more_apps(true);
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
    lv_obj_add_event_cb(search_bar, search_bar_key_event_cb, LV_EVENT_KEY, NULL); // Add key handler

    list_container = lv_obj_create(main_container);
    lv_obj_set_width(list_container, lv_pct(100));
    lv_obj_set_flex_grow(list_container, 1);
    lv_obj_set_flex_flow(list_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(list_container, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    // The list container itself no longer needs a key handler.

    lv_group_add_obj(lv_group_get_default(), search_bar);

    reset_and_fetch_initial();
}

static void search_bar_key_event_cb(lv_event_t *e) {
    uint32_t key = lv_indev_get_key(lv_indev_active());
    if (key == LV_KEY_DOWN && lv_obj_get_child_cnt(list_container) > 0) {
        lv_obj_t* first_card = lv_obj_get_child(list_container, 0);
        lv_group_focus_obj(first_card);
        lv_obj_scroll_to_view(first_card, LV_ANIM_ON);
    }
}

static void fetch_more_apps(bool fetch_at_end) {
    if (is_fetching || (fetch_at_end && end_of_list_reached)) return;
    is_fetching = true;
    const char *query = lv_textarea_get_text(search_bar);
    int limit = fetch_at_end ? SCROLL_FETCH_LIMIT : INITIAL_LOAD_LIMIT;
    int offset = fetch_at_end ? list_start_offset + loaded_project_count : 0;
    int new_count = 0;
    project_t *new_projects = get_applications(&new_count, query, limit, offset);
    if (new_projects && new_count > 0) {
        int old_count = loaded_project_count;
        loaded_project_count += new_count;
        loaded_projects = realloc(loaded_projects, loaded_project_count * sizeof(project_t));
        memcpy(&loaded_projects[old_count], new_projects, new_count * sizeof(project_t));
        free(new_projects);
        if (new_count < limit) {
            end_of_list_reached = true;
        }
    } else {
        end_of_list_reached = true;
        if (new_projects) free(new_projects);
    }
    redraw_list_and_preserve_focus();
    is_fetching = false;
}
static void reset_and_fetch_initial(void) {
    if (loaded_projects) {
        free_applications(loaded_projects, loaded_project_count);
        loaded_projects = NULL;
    }
    loaded_project_count = 0;
    list_start_offset = 0;
    end_of_list_reached = false;
    is_fetching = false;
    lv_obj_clean(list_container);
    lv_obj_t *spinner = lv_spinner_create(list_container);
    lv_obj_center(spinner);
    lv_refr_now(NULL);
    fetch_more_apps(false);
}
static void redraw_list_and_preserve_focus(void) {
    char* focused_slug = NULL;
    lv_obj_t* focused_card = lv_group_get_focused(lv_group_get_default());
    if (focused_card && lv_obj_get_parent(focused_card) == list_container) {
        card_user_data_t* user_data = lv_obj_get_user_data(focused_card);
        if (user_data) focused_slug = strdup(user_data->slug);
    }
    lv_obj_clean(list_container);
    create_app_list_view(list_container, loaded_projects, loaded_project_count);
    if (focused_slug) {
        for (int i = 0; i < loaded_project_count; i++) {
            if (strcmp(loaded_projects[i].slug, focused_slug) == 0) {
                lv_group_focus_obj(lv_obj_get_child(list_container, i));
                break;
            }
        }
        free(focused_slug);
    }
}
static void search_timer_cb(lv_timer_t *timer) {
    printf("Search timer fired. Starting new search...\n");
    reset_and_fetch_initial();
    search_timer = NULL;
}
static void search_bar_event_cb(lv_event_t *e) {
    if (search_timer) lv_timer_del(search_timer);
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
    }
}
