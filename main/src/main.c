/**
 * @file main
 *
 */

/*********************
 * INCLUDES
 *********************/
#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE /* needed for usleep() */
#endif

#include <stdlib.h>
#include <stdio.h>
#ifdef _MSC_VER
#include <Windows.h>
#else
#include <unistd.h>
#include <pthread.h>
#endif
#include "lvgl/lvgl.h"
#include <SDL.h>
#include "badgehub_client.h" // Include our new client header

/*********************
 * DEFINES
 *********************/

/**********************
 * TYPEDEFS
 **********************/

/**********************
 * STATIC PROTOTYPES
 **********************/
static lv_display_t *hal_init(int32_t w, int32_t h);

/**********************
 * STATIC VARIABLES
 **********************/

/**********************
 * MACROS
 **********************/

/**********************
 * GLOBAL FUNCTIONS
 **********************/

#if LV_USE_OS == LV_OS_FREERTOS
extern void freertos_main(void);
#endif

int main(int argc, char **argv)
{
    (void)argc; /*Unused*/
    (void)argv; /*Unused*/

    /*Initialize LVGL*/
    lv_init();

    /*Initialize the HAL (display, input devices, tick) for LVGL*/
    hal_init(720, 720);

    /* Create a label that will be updated with the project count */
    lv_obj_t *label = lv_label_create(lv_screen_active());
    lv_label_set_text(label, "Fetching data...");
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0); // Use lv_obj_align for better control

    /* Force a redraw to show the "Fetching data..." message immediately */
    lv_refr_now(NULL);

    /* Fetch the applications from the BadgeHub API */
    int project_count = 0;
    project_t *projects = get_applications(&project_count);

    /* Prepare the text to display */
    char display_text[128];
    if (projects != NULL) {
        printf("Successfully fetched %d projects.\n", project_count);
        snprintf(display_text, sizeof(display_text), "BadgeHub Projects: %d", project_count);
        // We are done with the data, so free it
        free_applications(projects, project_count);
    } else {
        printf("Failed to fetch projects.\n");
        snprintf(display_text, sizeof(display_text), "Error: Failed to fetch projects.");
    }

    /* Update the label with the result */
    lv_label_set_text(label, display_text);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);


#if LV_USE_OS == LV_OS_NONE
    while (1)
    {
        /* Periodically call the lv_task handler.
         * It could be done in a timer interrupt or an OS task too.*/
        lv_timer_handler();
#ifdef _MSC_VER
        Sleep(5);
#else
        usleep(5 * 1000);
#endif
    }
#elif LV_USE_OS == LV_OS_FREERTOS
    /* Run FreeRTOS and create lvgl task */
    freertos_main();
#endif

    return 0;
}

/**********************
 * STATIC FUNCTIONS
 **********************/

/**
 * Initialize the Hardware Abstraction Layer (HAL) for the LVGL graphics
 * library
 */
static lv_display_t *hal_init(int32_t w, int32_t h)
{
    lv_group_set_default(lv_group_create());

    lv_display_t *disp = lv_sdl_window_create(w, h);

    lv_indev_t *mouse = lv_sdl_mouse_create();
    lv_indev_set_group(mouse, lv_group_get_default());
    lv_indev_set_display(mouse, disp);
    lv_display_set_default(disp);

    // LV_IMAGE_DECLARE(mouse_cursor_icon); /*Declare the image file.*/
    // lv_obj_t *cursor_obj;
    // cursor_obj = lv_image_create(lv_screen_active());  /*Create an image object for the cursor */
    // lv_image_set_src(cursor_obj, &mouse_cursor_icon);  /*Set the image source*/
    // lv_indev_set_cursor(mouse, cursor_obj);            /*Connect the image object to the driver*/

    lv_indev_t *mousewheel = lv_sdl_mousewheel_create();
    lv_indev_set_display(mousewheel, disp);
    lv_indev_set_group(mousewheel, lv_group_get_default());

    lv_indev_t *kb = lv_sdl_keyboard_create();
    lv_indev_set_display(kb, disp);
    lv_indev_set_group(kb, lv_group_get_default());

    return disp;
}
