#include "badgehub_client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "cjson/cJSON.h"

#define BADGEHUB_API_URL "https://badgehub.p1m.nl/api/v3/projects"

// A struct to help curl write data into a dynamically growing memory buffer
struct MemoryStruct {
    char *memory;
    size_t size;
};

// libcurl callback function to write received data into our MemoryStruct
static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if (!ptr) {
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

// Helper function to safely extract a string from a cJSON object.
// The returned string is dynamically allocated and must be freed by the caller.
static char* get_json_string(cJSON *json, const char *key) {
    cJSON *item = cJSON_GetObjectItemCaseSensitive(json, key);
    if (cJSON_IsString(item) && (item->valuestring != NULL)) {
        char *str = malloc(strlen(item->valuestring) + 1);
        if (str) {
            strcpy(str, item->valuestring);
        }
        return str;
    }
    // Return an empty string if the key is not found or not a string
    char *empty_str = malloc(1);
    if(empty_str) *empty_str = '\0';
    return empty_str;
}

// Implementation of the get_applications function
project_t *get_applications(int *project_count) {
    *project_count = 0;
    CURL *curl_handle;
    CURLcode res;
    struct MemoryStruct chunk = { .memory = malloc(1), .size = 0 };
    project_t *projects = NULL;

    if (chunk.memory == NULL) {
        fprintf(stderr, "Failed to allocate memory\n");
        return NULL;
    }

    curl_global_init(CURL_GLOBAL_ALL);
    curl_handle = curl_easy_init();

    if (!curl_handle) {
        free(chunk.memory);
        return NULL;
    }

    curl_easy_setopt(curl_handle, CURLOPT_URL, BADGEHUB_API_URL);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "lvgl-badgehub-client/1.0");

    res = curl_easy_perform(curl_handle);

    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    } else {
        cJSON *root = cJSON_Parse(chunk.memory);
        if (root == NULL) {
            const char *error_ptr = cJSON_GetErrorPtr();
            if (error_ptr != NULL) {
                fprintf(stderr, "Error before: %s\n", error_ptr);
            }
        } else if (cJSON_IsArray(root)) {
            *project_count = cJSON_GetArraySize(root);
            projects = malloc(*project_count * sizeof(project_t));
            if (projects) {
                cJSON *proj_json = NULL;
                int i = 0;
                cJSON_ArrayForEach(proj_json, root) {
                    cJSON *id = cJSON_GetObjectItemCaseSensitive(proj_json, "id");
                    cJSON *is_public = cJSON_GetObjectItemCaseSensitive(proj_json, "public");

                    projects[i].id = cJSON_IsNumber(id) ? id->valueint : -1;
                    projects[i].public = cJSON_IsBool(is_public) ? (bool)is_public->valueint : false;
                    projects[i].name = get_json_string(proj_json, "name");
                    projects[i].slug = get_json_string(proj_json, "slug");
                    projects[i].description = get_json_string(proj_json, "description");
                    projects[i].project_url = get_json_string(proj_json, "project_url");
                    projects[i].icon_url = get_json_string(proj_json, "icon_url");
                    i++;
                }
            }
            cJSON_Delete(root);
        }
    }

    curl_easy_cleanup(curl_handle);
    free(chunk.memory);
    curl_global_cleanup();

    return projects;
}

// Implementation of the free_applications function
void free_applications(project_t *projects, int count) {
    if (!projects) return;
    for (int i = 0; i < count; i++) {
        free(projects[i].name);
        free(projects[i].slug);
        free(projects[i].description);
        free(projects[i].project_url);
        free(projects[i].icon_url);
    }
    free(projects);
}
