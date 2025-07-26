#ifndef APP_LIST_H
#define APP_LIST_H

#include "badgehub_client.h"
#include "lvgl/lvgl.h"

/**
 * @brief Creates and populates the application list view.
 *
 * This function takes an array of project data and renders it as a list of
 * cards on the given parent object. It does not fetch data itself.
 *
 * @param parent The parent LVGL object to which the list will be added.
 * @param projects A pointer to the array of project data.
 * @param project_count The number of projects in the array.
 */
void create_app_list_view(lv_obj_t* parent, project_t* projects, int project_count);

#endif // APP_LIST_H
