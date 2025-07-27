#ifndef APP_HOME_H
#define APP_HOME_H

#include "lvgl/lvgl.h"

void create_app_home_view(void);
lv_obj_t* get_search_bar(void);

void app_home_show_next_page(void);
void app_home_show_previous_page(void);

/**
 * @brief Focuses the search bar and initiates a search with the given key.
 * This is called when a user types a letter while focused on the list.
 * @param key The character key that was pressed.
 */
void app_home_focus_search_and_start_typing(uint32_t key);

#endif // APP_HOME_H
