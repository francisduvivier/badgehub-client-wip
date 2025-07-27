#ifndef APP_DATA_MANAGER_H
#define APP_DATA_MANAGER_H

#include "badgehub_client.h"
#include <stdbool.h>

void data_manager_init(void);
void data_manager_deinit(void);

// --- Data Access ---
project_t* data_manager_get_projects(void);
int data_manager_get_project_count(void);
int data_manager_get_current_page_num(void);

// --- Actions ---
void data_manager_set_search_query(const char* query);
bool data_manager_load_next_page(void);
bool data_manager_load_previous_page(void);
void data_manager_reset_and_load_first_page(void);

#endif // APP_DATA_MANAGER_H
