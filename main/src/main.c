#include <stdlib.h>
#include <stdio.h>
#ifdef _MSC_VER
#include <Windows.h>
#else
#include <unistd.h>
#endif
#include "lvgl/lvgl.h"
#include <SDL.h>
#include "app_home.h"

static lv_display_t *hal_init(int32_t w, int32_t h);

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    lv_init();
    hal_init(720, 720);

    // Create the main application UI using the new home screen
    create_app_home_view();

    while (1)
    {
        lv_timer_handler();
#ifdef _MSC_VER
        Sleep(5);
#else
        usleep(5 * 1000);
#endif
    }

    return 0;
}

static lv_display_t *hal_init(int32_t w, int32_t h)
{
    lv_group_set_default(lv_group_create());
    lv_display_t *disp = lv_sdl_window_create(w, h);
    lv_indev_t *mouse = lv_sdl_mouse_create();
    lv_indev_set_group(mouse, lv_group_get_default());
    lv_indev_set_display(mouse, disp);
    lv_display_set_default(disp);
    lv_indev_t *mousewheel = lv_sdl_mousewheel_create();
    lv_indev_set_display(mousewheel, disp);
    lv_indev_set_group(mousewheel, lv_group_get_default());
    lv_indev_t *kb = lv_sdl_keyboard_create();
    lv_indev_set_display(kb, disp);
    lv_indev_set_group(kb, lv_group_get_default());
    return disp;
}
