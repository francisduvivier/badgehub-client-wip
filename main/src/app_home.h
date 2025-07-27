#ifndef APP_HOME_H
#define APP_HOME_H

#include "lvgl/lvgl.h"

/**
 * @brief Creates the main home screen of the application.
 */
void create_app_home_view(void);

/**
 * @brief Gets a pointer to the search bar widget.
 * @return A pointer to the search bar lv_obj_t, or NULL if not created.
 */
lv_obj_t* get_search_bar(void);

/**
 * @brief Triggers a fetch for the next page of applications.
 * This is called by the UI when the user scrolls near the end of the list.
 */
void app_home_fetch_more(void);

#endif // APP_HOME_H
