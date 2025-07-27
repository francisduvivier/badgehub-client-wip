#include "app_detail.h"
#include "app_home.h"
#include "badgehub_client.h"
#include "lvgl/lvgl.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>

#define INSTALLATION_DIR "installation_dir"

// ... (forward declarations and other functions are unchanged)
static void back_button_event_handler(lv_event_t * e);
static void install_button_event_handler(lv_event_t * e);
static void detail_view_delete_event_handler(lv_event_t * e);
static void detail_key_event_handler(lv_event_t * e);
typedef struct { lv_obj_t *btn_back; lv_obj_t *btn_install; } detail_nav_t;

void create_app_detail_view(const char* slug, int revision) {
    // ... (this function is unchanged)
    char local_slug[256];
    if (slug) { strncpy(local_slug, slug, sizeof(local_slug) - 1); local_slug[sizeof(local_slug) - 1] = '\0'; } else { local_slug[0] = '\0'; }
    int local_revision = revision;
    lv_obj_clean(lv_screen_active());
    lv_obj_t* container = lv_obj_create(lv_screen_active());
    lv_obj_set_size(container, lv_pct(100), lv_pct(100));
    lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(container, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
    lv_obj_t* btn_back = lv_btn_create(container);
    lv_obj_add_event_cb(btn_back, back_button_event_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_t* label_back = lv_label_create(btn_back);
    lv_label_set_text(label_back, "Back to List");
    lv_obj_set_style_margin_bottom(btn_back, 10, 0);
    lv_obj_t* loading_label = lv_label_create(container);
    lv_label_set_text(loading_label, "Loading details...");
    lv_obj_align(loading_label, LV_ALIGN_CENTER, 0, 0);
    lv_refr_now(NULL);
    project_detail_t* details = get_project_details(local_slug, local_revision);
    lv_obj_del(loading_label);
    if (details) {
        lv_obj_add_event_cb(container, detail_view_delete_event_handler, LV_EVENT_DELETE, details);
        lv_obj_t* title_label = lv_label_create(container);
        lv_label_set_text_fmt(title_label, "Name: %s (rev %d)", details->name, details->revision);
        lv_obj_set_width(title_label, lv_pct(95));
        lv_label_set_long_mode(title_label, LV_LABEL_LONG_WRAP);
        lv_obj_t* desc_label = lv_label_create(container);
        lv_label_set_text_fmt(desc_label, "Description: %s", details->description);
        lv_obj_set_width(desc_label, lv_pct(95));
        lv_label_set_long_mode(desc_label, LV_LABEL_LONG_WRAP);
        lv_obj_set_style_margin_top(desc_label, 10, 0);
        lv_obj_t* date_label = lv_label_create(container);
        lv_label_set_text_fmt(date_label, "Published: %s", details->published_at);
        lv_obj_set_width(date_label, lv_pct(95));
        lv_label_set_long_mode(date_label, LV_LABEL_LONG_WRAP);
        lv_obj_set_style_margin_top(date_label, 10, 0);
        lv_obj_t* btn_install = lv_btn_create(container);
        lv_obj_set_style_margin_top(btn_install, 20, 0);
        lv_obj_add_event_cb(btn_install, install_button_event_handler, LV_EVENT_CLICKED, details);
        lv_obj_t* label_install = lv_label_create(btn_install);
        lv_label_set_text(label_install, "Install");
        lv_obj_t* status_label = lv_label_create(container);
        lv_label_set_text(status_label, "");
        lv_obj_set_width(status_label, lv_pct(95));
        lv_label_set_long_mode(status_label, LV_LABEL_LONG_WRAP);
        lv_obj_set_style_margin_top(status_label, 10, 0);
        lv_obj_set_user_data(btn_install, status_label);
        lv_group_t * g = lv_group_get_default();
        lv_group_add_obj(g, btn_back);
        lv_group_add_obj(g, btn_install);
        detail_nav_t *nav_data = malloc(sizeof(detail_nav_t));
        if (nav_data) {
            nav_data->btn_back = btn_back;
            nav_data->btn_install = btn_install;
            lv_obj_add_event_cb(btn_back, detail_key_event_handler, LV_EVENT_KEY, nav_data);
            lv_obj_add_event_cb(btn_install, detail_key_event_handler, LV_EVENT_KEY, nav_data);
        }
        lv_obj_add_event_cb(container, detail_view_delete_event_handler, LV_EVENT_DELETE, nav_data);
    } else {
        lv_obj_t* error_label = lv_label_create(container);
        lv_label_set_text(error_label, "Failed to load project details.");
    }
}

static void install_button_event_handler(lv_event_t * e) {
    lv_obj_t* btn = lv_event_get_target(e);
    project_detail_t* details = (project_detail_t*)lv_event_get_user_data(e);
    lv_obj_t* status_label = (lv_obj_t*)lv_obj_get_user_data(btn);
    if (!details || !status_label) return;
    lv_obj_add_state(btn, LV_STATE_DISABLED);
    lv_label_set_text(status_label, "Starting installation...");
    lv_refr_now(NULL);
    char project_dir_path[512];
    snprintf(project_dir_path, sizeof(project_dir_path), "%s/%s", INSTALLATION_DIR, details->slug);
    if (mkdir(INSTALLATION_DIR, 0755) != 0 && errno != EEXIST) {
        lv_label_set_text_fmt(status_label, "Error: Could not create directory '%s'", INSTALLATION_DIR);
        lv_obj_clear_state(btn, LV_STATE_DISABLED);
        return;
    }
    if (mkdir(project_dir_path, 0755) != 0 && errno != EEXIST) {
        lv_label_set_text_fmt(status_label, "Error: Could not create directory '%s'", project_dir_path);
        lv_obj_clear_state(btn, LV_STATE_DISABLED);
        return;
    }
    bool all_successful = true;
    for (int i = 0; i < details->file_count; i++) {
        lv_label_set_text_fmt(status_label, "Downloading (%d/%d): %s",
                              i + 1, details->file_count, details->files[i].full_path);
        lv_refr_now(NULL);
        // --- CHANGED: Call the simplified download function ---
        if (!download_project_file(&details->files[i], details->slug)) {
            lv_label_set_text_fmt(status_label, "Error: Failed to download %s", details->files[i].full_path);
            all_successful = false;
            break;
        }
    }
    if (all_successful) {
        lv_label_set_text(status_label, "Installation complete!");
    } else {
        lv_obj_clear_state(btn, LV_STATE_DISABLED);
    }
}

// ... (rest of app_detail.c is unchanged)
static void back_button_event_handler(lv_event_t * e) { create_app_home_view(); }
static void detail_view_delete_event_handler(lv_event_t * e) { void* user_data = lv_event_get_user_data(e); if (user_data) { free(user_data); } }
static void detail_key_event_handler(lv_event_t * e) {
    uint32_t key = lv_indev_get_key(lv_indev_active());
    lv_obj_t * focused_btn = lv_event_get_target(e);
    detail_nav_t * nav_data = (detail_nav_t*)lv_event_get_user_data(e);
    if (key == LV_KEY_UP) {
        if (focused_btn == nav_data->btn_back) { lv_group_focus_obj(nav_data->btn_install); } else { lv_group_focus_obj(nav_data->btn_back); }
    } else if (key == LV_KEY_DOWN) {
        if (focused_btn == nav_data->btn_install) { lv_group_focus_obj(nav_data->btn_back); } else { lv_group_focus_obj(nav_data->btn_install); }
    }
}
