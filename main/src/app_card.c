#include "app_card.h"
#include "app_detail.h"
#include "app_home.h" // Still needed for the back-navigation from the detail page
#include "lvgl/lvgl.h"
#include <string.h>
#include <stdlib.h>

typedef struct {
    char* slug;
    int revision;
} card_user_data_t;

// --- FORWARD DECLARATIONS ---
static void card_click_event_handler(lv_event_t * e);
static void card_delete_event_handler(lv_event_t * e);
// The key event handler has been removed from this file.

// --- IMPLEMENTATIONS ---

void create_app_card(lv_obj_t* parent, const project_t* project) {
    // --- NEW: Add styles for focused state ---
    static lv_style_t style_focused;
    lv_style_init(&style_focused);
    lv_style_set_border_color(&style_focused, lv_palette_main(LV_PALETTE_BLUE));
    lv_style_set_border_width(&style_focused, 2);

    lv_obj_t* card = lv_obj_create(parent);
    lv_obj_set_size(card, lv_pct(95), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    lv_obj_add_flag(card, LV_OBJ_FLAG_CLICKABLE);
    // Apply the focused style when the card is in the focused state
    lv_obj_add_style(card, &style_focused, LV_STATE_FOCUSED);

    card_user_data_t* user_data = malloc(sizeof(card_user_data_t));
    if (user_data) {
        user_data->slug = strdup(project->slug);
        user_data->revision = project->revision;
    }

    lv_obj_add_event_cb(card, card_click_event_handler, LV_EVENT_CLICKED, user_data);
    lv_obj_add_event_cb(card, card_delete_event_handler, LV_EVENT_DELETE, user_data);
    // The key event handler is no longer added here.

    // Add the card to the default group to make it focusable
    lv_group_add_obj(lv_group_get_default(), card);

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


static void card_click_event_handler(lv_event_t * e) {
    card_user_data_t* user_data = (card_user_data_t*)lv_event_get_user_data(e);
    if (user_data) {
        create_app_detail_view(user_data->slug, user_data->revision);
    }
}

static void card_delete_event_handler(lv_event_t * e) {
    card_user_data_t* user_data = (card_user_data_t*)lv_event_get_user_data(e);
    if (user_data) {
        free(user_data->slug);
        free(user_data);
    }
}
