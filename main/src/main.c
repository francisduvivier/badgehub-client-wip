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
#include <string.h>
#ifdef _MSC_VER
#include <Windows.h>
#else
#include <unistd.h>
#include <pthread.h>
#endif
#include "lvgl/lvgl.h"
#include <SDL.h>
#include <curl/curl.h>
#include "cjson/cJSON.h"

/*********************
 * DEFINES
 *********************/
#define BADGEHUB_API_URL "https://badgehub.p1m.nl/api/v3/projects"

/**********************
 * TYPEDEFS
 **********************/
// Struct to hold the response from curl
struct MemoryStruct
{
    char *memory;
    size_t size;
};

/**********************
 * STATIC PROTOTYPES
 **********************/
static lv_display_t *hal_init(int32_t w, int32_t h);
static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp);

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
    lv_obj_center(label);

    CURL *curl_handle;
    CURLcode res;

    struct MemoryStruct chunk;
    chunk.memory = malloc(1); /* will be grown as needed by the realloc above */
    chunk.size = 0;           /* no data at this point */

    curl_global_init(CURL_GLOBAL_ALL);
    curl_handle = curl_easy_init();

    if (curl_handle)
    {
        curl_easy_setopt(curl_handle, CURLOPT_URL, BADGEHUB_API_URL);
        curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
        curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

        res = curl_easy_perform(curl_handle);

        char display_text[128];

        if (res != CURLE_OK)
        {
            snprintf(display_text, sizeof(display_text), "HTTP request failed:\n%s", curl_easy_strerror(res));
        }
        else
        {
            cJSON *root = cJSON_Parse(chunk.memory);
            if (root == NULL)
            {
                 snprintf(display_text, sizeof(display_text), "Error: Failed to parse JSON.");
            }
            else
            {
                if(cJSON_IsArray(root)) {
                    int item_count = cJSON_GetArraySize(root);
                    snprintf(display_text, sizeof(display_text), "BadgeHub Projects: %d", item_count);
                } else {
                    snprintf(display_text, sizeof(display_text), "Error: JSON is not an array.");
                }
                cJSON_Delete(root);
            }
        }

        lv_label_set_text(label, display_text);
        lv_obj_center(label);


        curl_easy_cleanup(curl_handle);
        free(chunk.memory);
    }

    curl_global_cleanup();


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
 * libcurl callback to write received data into a memory buffer
 */
static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if (ptr == NULL)
    {
        /* out of memory! */
        printf("not enough memory (realloc returned NULL)\n");
        return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}


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
