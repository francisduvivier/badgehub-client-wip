#include "app_data_manager.h"
#include <stdlib.h>
#include <string.h>

#define ITEMS_PER_PAGE 20

// --- Static State ---
static project_t* s_projects = NULL;
static int s_project_count = 0;
static int s_current_offset = 0;
static char* s_search_query = NULL;
static bool s_end_of_list_reached = false;

void data_manager_init(void) {
    // This function is here for future expansion if needed.
}

void data_manager_deinit(void) {
    if (s_projects) {
        free_applications(s_projects, s_project_count);
        s_projects = NULL;
    }
    if (s_search_query) {
        free(s_search_query);
        s_search_query = NULL;
    }
}

project_t* data_manager_get_projects(void) {
    return s_projects;
}

int data_manager_get_project_count(void) {
    return s_project_count;
}

int data_manager_get_current_page_num(void) {
    return (s_current_offset / ITEMS_PER_PAGE) + 1;
}

void data_manager_set_search_query(const char* query) {
    if (s_search_query) {
        free(s_search_query);
        s_search_query = NULL;
    }
    if (query && strlen(query) > 0) {
        s_search_query = strdup(query);
    }
}

static bool load_page_internal(int offset) {
    if (s_projects) {
        free_applications(s_projects, s_project_count);
        s_projects = NULL;
        s_project_count = 0;
    }

    s_projects = get_applications(&s_project_count, s_search_query, ITEMS_PER_PAGE, offset);

    if (s_projects) {
        s_current_offset = offset;
        s_end_of_list_reached = (s_project_count < ITEMS_PER_PAGE);
        return true;
    }
    return false;
}

bool data_manager_load_next_page(void) {
    if (s_end_of_list_reached) return false;
    return load_page_internal(s_current_offset + ITEMS_PER_PAGE);
}

bool data_manager_load_previous_page(void) {
    if (s_current_offset == 0) return false;
    int new_offset = s_current_offset - ITEMS_PER_PAGE;
    if (new_offset < 0) new_offset = 0;
    return load_page_internal(new_offset);
}

void data_manager_reset_and_load_first_page(void) {
    s_end_of_list_reached = false;
    load_page_internal(0);
}
