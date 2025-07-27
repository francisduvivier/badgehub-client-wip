#ifndef APP_CARD_H
#define APP_CARD_H

#include "badgehub_client.h"
#include "lvgl/lvgl.h"

// The user data struct now needs a place to store the downloaded icon data
typedef struct {
    char* slug;
    int revision;
    uint8_t* icon_data; // Pointer to the raw PNG data
    lv_image_dsc_t icon_dsc; // LVGL descriptor for the icon
} card_user_data_t;

void create_app_card(lv_obj_t* parent, const project_t* project);

#endif // APP_CARD_H
