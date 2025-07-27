#ifndef APP_CARD_H
#define APP_CARD_H

#include "badgehub_client.h"
#include "lvgl/lvgl.h"

typedef struct {
    char* slug;
    int revision;
    char* icon_url; // Store the URL for on-demand loading
    uint8_t* icon_data;
    lv_image_dsc_t icon_dsc;
} card_user_data_t;

void create_app_card(lv_obj_t* parent, const project_t* project);

/**
 * @brief Triggers the download and display of the icon for a specific card.
 * @param card A pointer to the card object.
 */
void app_card_load_icon(lv_obj_t* card);

#endif // APP_CARD_H
