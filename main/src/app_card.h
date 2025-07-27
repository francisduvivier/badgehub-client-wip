#ifndef APP_CARD_H
#define APP_CARD_H

#include "badgehub_client.h"
#include "lvgl/lvgl.h"

/**
 * @brief The user data struct attached to each card object to store its unique info.
 */
typedef struct {
    char* slug;
    int revision;
} card_user_data_t;

/**
 * @brief Creates a single application card widget.
 *
 * @param parent The parent LVGL object to which the card will be added.
 * @param project A pointer to the project data to display on the card.
 */
void create_app_card(lv_obj_t* parent, const project_t* project);

#endif // APP_CARD_H
