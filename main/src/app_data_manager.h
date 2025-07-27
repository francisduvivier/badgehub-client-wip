#ifndef APP_DATA_MANAGER_H
#define APP_DATA_MANAGER_H

#include "lvgl/lvgl.h"

/**
 * @brief Initializes the data manager state.
 */
void data_manager_init(void);

/**
 * @brief Deinitializes the data manager, freeing all cached data.
 */
void data_manager_deinit(void);

/**
 * @brief Fetches a page of applications and updates the UI.
 *
 * @param list_container The LVGL object where the list cards will be created.
 * @param page_indicator The label to update with the current page number.
 * @param search_bar The search bar object, used for getting the query and managing focus.
 * @param offset The data offset to fetch from the API.
 * @param focus_last True if the last item on the new page should be focused.
 */
void data_manager_fetch_page(lv_obj_t* list_container, lv_obj_t* page_indicator, lv_obj_t* search_bar, int offset, bool focus_last);

/**
 * @brief Triggers a fetch for the next page of data.
 */
void data_manager_request_next_page(void);

/**
 * @brief Triggers a fetch for the previous page of data.
 */
void data_manager_request_previous_page(void);

/**
 * @brief Starts a new search, resetting the pagination.
 */
void data_manager_start_new_search(void);

#endif // APP_DATA_MANAGER_H
