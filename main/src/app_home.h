#ifndef APP_HOME_H
#define APP_HOME_H

#include "lvgl/lvgl.h"

void create_app_home_view(void);
lv_obj_t* get_search_bar(void);

/**
 * @brief Triggers a fetch and redraw for the next page of applications.
 */
void app_home_show_next_page(void);

/**
 * @brief Triggers a fetch and redraw for the previous page of applications.
 */
void app_home_show_previous_page(void);

#endif // APP_HOME_H
