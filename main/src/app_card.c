#include "app_card.h"
#include "app_detail.h"
#include "app_home.h"
#include "badgehub_client.h"
#include "lvgl/lvgl.h"
#include <string.h>
#include <stdlib.h>

static void card_click_event_handler(lv_event_t * e);
static void card_delete_event_handler(lv_event_t * e);
static void card_key_event_handler(lv_event_t * e);

void create_app_card(lv_obj_t* parent, const project_t* project) {
    static lv_style_t style_focused;
    lv_style_init(&style_focused);
    lv_style_set_border_color(&style_focused, lv_palette_main(LV_PALETTE_BLUE));
    lv_style_set_border_width(&style_focused, 2);

    lv_obj_t* card = lv_obj_create(parent);
    lv_obj_set_size(card, lv_pct(95), 80);
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(card, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_add_flag(card, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_style(card, &style_focused, LV_STATE_FOCUSED);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* icon_img = lv_image_create(card);
    lv_image_set_src(icon_img, LV_SYMBOL_IMAGE);
    lv_obj_set_size(icon_img, 64, 64);

    lv_obj_t* text_container = lv_obj_create(card);
    lv_obj_remove_style_all(text_container);
    lv_obj_set_flex_grow(text_container, 1);
    lv_obj_set_height(text_container, lv_pct(100));
    lv_obj_set_flex_flow(text_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_left(text_container, 10, 0);

    card_user_data_t* user_data = calloc(1, sizeof(card_user_data_t));
    if (user_data) {
        user_data->slug = strdup(project->slug);
        user_data->revision = project->revision;
        if (project->icon_url) {
            user_data->icon_url = strdup(project->icon_url);
        }
    }

    lv_obj_set_user_data(card, user_data);
    lv_obj_add_event_cb(card, card_click_event_handler, LV_EVENT_CLICKED, user_data);
    lv_obj_add_event_cb(card, card_delete_event_handler, LV_EVENT_DELETE, user_data);
    lv_obj_add_event_cb(card, card_key_event_handler, LV_EVENT_KEY, NULL);

    lv_group_add_obj(lv_group_get_default(), card);

    static lv_style_t style_title;
    lv_style_init(&style_title);
    lv_style_set_text_font(&style_title, lv_font_get_default());

    lv_obj_t* title_label = lv_label_create(text_container);
    lv_label_set_text(title_label, project->name);
    lv_obj_add_style(title_label, &style_title, 0);
    lv_label_set_long_mode(title_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(title_label, lv_pct(100));

    lv_obj_t* desc_label = lv_label_create(text_container);
    lv_label_set_text(desc_label, project->description);
    lv_label_set_long_mode(desc_label, LV_LABEL_LONG_DOT);
    lv_obj_set_width(desc_label, lv_pct(100));
}

void app_card_load_icon(lv_obj_t* card) {
    if (!card) return;
    card_user_data_t* user_data = lv_obj_get_user_data(card);
    if (!user_data || !user_data->icon_url || user_data->icon_data) {
        return; // No URL or icon already loaded
    }

    size_t icon_size = 0;
    uint8_t* icon_data = download_icon_to_memory(user_data->icon_url, &icon_size);

    if (icon_data) {
        user_data->icon_data = icon_data;
        user_data->icon_dsc.data = icon_data;
        user_data->icon_dsc.data_size = icon_size;
        user_data->icon_dsc.header.cf = LV_COLOR_FORMAT_RAW;

        lv_obj_t* icon_img = lv_obj_get_child(card, 0);
        lv_image_set_src(icon_img, &user_data->icon_dsc);
    }
}

static void card_delete_event_handler(lv_event_t * e) {
    card_user_data_t* user_data = (card_user_data_t*)lv_event_get_user_data(e);
    if (user_data) {
        free(user_data->slug);
        free(user_data->icon_url);
        if (user_data->icon_data) {
            free(user_data->icon_data);
        }
        free(user_data);
    }
}

// ... (rest of app_card.c is unchanged)
static void card_click_event_handler(lv_event_t * e) {
    card_user_data_t* user_data = (card_user_data_t*)lv_event_get_user_data(e);
    if (user_data) {
        create_app_detail_view(user_data->slug, user_data->revision);
    }
}
static void card_key_event_handler(lv_event_t * e) {
    uint32_t key = lv_indev_get_key(lv_indev_active());
    lv_obj_t * card = lv_event_get_target(e);
    lv_obj_t * parent = lv_obj_get_parent(card);
    uint32_t current_index = lv_obj_get_index(card);
    uint32_t child_count = lv_obj_get_child_cnt(parent);
    lv_obj_t* new_focus_target = NULL;
    if (key == LV_KEY_UP) {
        if (current_index == 0) {
            app_home_show_previous_page();
            lv_event_stop_processing(e);
            return;
        } else {
            new_focus_target = lv_obj_get_child(parent, current_index - 1);
        }
    } else if (key == LV_KEY_DOWN) {
        if (current_index == child_count - 1) {
            app_home_show_next_page();
            lv_event_stop_processing(e);
            return;
        } else {
            new_focus_target = lv_obj_get_child(parent, current_index + 1);
        }
    } else if (key >= ' ' && key < LV_KEY_DEL) {
        app_home_focus_search_and_start_typing(key);
        return;
    }else if (key == LV_KEY_ESC || key == LV_KEY_BACKSPACE || key == LV_KEY_DEL) {
        app_home_focus_search_and_start_typing(0);
    }
    if (new_focus_target) {
        lv_group_focus_obj(new_focus_target);
        lv_obj_scroll_to_view(new_focus_target, LV_ANIM_ON);
    }
}
