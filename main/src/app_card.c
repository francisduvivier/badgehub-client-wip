#include "app_card.h"
#include "app_detail.h" // Include the detail view header
#include "lvgl/lvgl.h"
#include <string.h> // For strdup
#include <stdlib.h> // For free

// Event handler for when a card is clicked
static void card_click_event_handler(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        // Get the slug from the user data we attached to the card
        char* slug = (char*)lv_event_get_user_data(e);
        if (slug) {
            // Create the detail view, passing the slug
            create_app_detail_view(slug);
        }
    }
}

// Event handler for when a card is deleted
static void card_delete_event_handler(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_DELETE) {
        // Get the slug from the user data
        char* slug = (char*)lv_event_get_user_data(e);
        if (slug) {
            // Free the memory we allocated for the slug copy
            free(slug);
        }
    }
}


void create_app_card(lv_obj_t* parent, const project_t* project) {
    lv_obj_t* card = lv_obj_create(parent);
    lv_obj_set_size(card, lv_pct(95), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    lv_obj_add_flag(card, LV_OBJ_FLAG_CLICKABLE);

    // IMPORTANT: The original project data will be freed. We must make a copy
    // of the slug to use it safely in the event handler later.
    char* slug_copy = strdup(project->slug);

    // Add the click and delete event handlers
    lv_obj_add_event_cb(card, card_click_event_handler, LV_EVENT_CLICKED, (void*)slug_copy);
    lv_obj_add_event_cb(card, card_delete_event_handler, LV_EVENT_DELETE, (void*)slug_copy);

    static lv_style_t style_title;
    lv_style_init(&style_title);
    lv_style_set_text_font(&style_title, lv_font_get_default());

    lv_obj_t* title_label = lv_label_create(card);
    lv_label_set_text(title_label, project->name);
    lv_obj_add_style(title_label, &style_title, 0);
    lv_label_set_long_mode(title_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(title_label, lv_pct(100));

    lv_obj_t* desc_label = lv_label_create(card);
    lv_label_set_text(desc_label, project->description);
    lv_label_set_long_mode(desc_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(desc_label, lv_pct(100));
    lv_obj_set_style_margin_top(desc_label, 5, 0);
}
