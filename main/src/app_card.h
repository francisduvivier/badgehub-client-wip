#ifndef APP_CARD_H
#define APP_CARD_H

#include "badgehub_client/badgehub_client.h"
#include "lvgl/lvgl.h"

/**
 * @brief Creates a single application card widget.
 *
 * This function creates a styled container with labels for a project's
 * title and description, and adds it to the specified parent object.
 *
 * @param parent The parent LVGL object to which the card will be added.
 * @param project A pointer to the project data to display on the card.
 */
void create_app_card(lv_obj_t* parent, const project_t* project);

#endif // APP_CARD_H
