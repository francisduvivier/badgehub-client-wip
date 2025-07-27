#ifndef APP_HOME_H
#define APP_HOME_H

#include "lvgl/lvgl.h"

void create_app_home_view(void);
lv_obj_t* get_search_bar(void);

/**
 * @brief Called by a list item after navigation to check if data needs updating.
 * This function contains the logic for fetching more items or pruning old ones.
 */
void app_home_handle_list_nav(void);

#endif // APP_HOME_H
