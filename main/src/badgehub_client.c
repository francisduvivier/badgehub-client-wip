#include "badgehub_client.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "cjson/cJSON.h"

#define INSTALLATION_DIR "installation_dir"

// --- FORWARD DECLARATION ---
static uint8_t* download_icon_to_memory(const char* icon_url, size_t* data_size);

project_t *get_applications(int *project_count, const char* search_query, int limit, int offset) {
    *project_count = 0;
    CURL *curl_handle;
    CURLcode res;
    struct MemoryStruct chunk = { .memory = malloc(1), .size = 0 };
    project_t *projects = NULL;
    char url[512];
    char base_url[256];
    snprintf(base_url, sizeof(base_url), "%s/projects", "https://badgehub.p1m.nl/api/v3");
    curl_global_init(CURL_GLOBAL_ALL);
    curl_handle = curl_easy_init();
    if (!curl_handle) { free(chunk.memory); return NULL; }
    char *escaped_query = NULL;
    if (search_query && strlen(search_query) > 0) {
        escaped_query = curl_easy_escape(curl_handle, search_query, 0);
        snprintf(url, sizeof(url), "%s?search=%s&limit=%d&offset=%d", base_url, escaped_query, limit, offset);
        curl_free(escaped_query);
    } else {
        snprintf(url, sizeof(url), "%s?limit=%d&offset=%d", base_url, limit, offset);
    }
    curl_easy_setopt(curl_handle, CURLOPT_URL, url);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "lvgl-badgehub-client/1.0");
    res = curl_easy_perform(curl_handle);
    if (res == CURLE_OK) {
        cJSON *root = cJSON_Parse(chunk.memory);
        if (cJSON_IsArray(root)) {
            *project_count = cJSON_GetArraySize(root);
            projects = calloc(*project_count, sizeof(project_t)); // Use calloc to zero-initialize
            if (projects) {
                cJSON *proj_json = NULL;
                int i = 0;
                cJSON_ArrayForEach(proj_json, root) {
                    projects[i].name = get_json_string(proj_json, "name");
                    projects[i].slug = get_json_string(proj_json, "slug");
                    projects[i].description = get_json_string(proj_json, "description");
                    cJSON *icon_map = cJSON_GetObjectItemCaseSensitive(proj_json, "icon_map");
                    cJSON *icon_64_obj = cJSON_GetObjectItemCaseSensitive(icon_map, "64x64");

                    char* icon_url = get_json_string(icon_64_obj, "url");
                    if (icon_url) {
                        size_t icon_size = 0;
                        projects[i].icon_data = download_icon_to_memory(icon_url, &icon_size);
                        if (projects[i].icon_data) {
                            projects[i].icon_dsc.data = projects[i].icon_data;
                            projects[i].icon_dsc.data_size = icon_size;
                            projects[i].icon_dsc.header.cf = LV_COLOR_FORMAT_RAW; // LVGL knows it's a PNG
                        }
                        free(icon_url);
                    }

                    cJSON *revision_item = cJSON_GetObjectItemCaseSensitive(proj_json, "revision");
                    projects[i].revision = cJSON_IsNumber(revision_item) ? revision_item->valueint : 0;
                    i++;
                }
            }
        }
        cJSON_Delete(root);
    }
    curl_easy_cleanup(curl_handle);
    free(chunk.memory);
    curl_global_cleanup();
    return projects;
}

static uint8_t* download_icon_to_memory(const char* icon_url, size_t* data_size) {
    if (!icon_url || strlen(icon_url) == 0) return NULL;

    CURL *curl_handle;
    CURLcode res;
    struct MemoryStruct chunk = { .memory = malloc(1), .size = 0 };
    long response_code = 0;

    curl_handle = curl_easy_init();
    if (curl_handle) {
        curl_easy_setopt(curl_handle, CURLOPT_URL, icon_url);
        curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
        res = curl_easy_perform(curl_handle);
        if (res == CURLE_OK) {
            curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &response_code);
            if (response_code == 200) {
                *data_size = chunk.size;
                curl_easy_cleanup(curl_handle);
                return (uint8_t*)chunk.memory; // Return the downloaded data
            }
        }
        // Cleanup on failure
        free(chunk.memory);
        curl_easy_cleanup(curl_handle);
    }
    return NULL;
}

void free_applications(project_t *projects, int count) {
    if (!projects) return;
    for (int i = 0; i < count; i++) {
        free(projects[i].name);
        free(projects[i].slug);
        free(projects[i].description);
        if (projects[i].icon_data) {
            free(projects[i].icon_data);
        }
    }
    free(projects);
}

// ... (rest of badgehub_client.c is unchanged)
project_detail_t *get_project_details(const char *slug, int revision) {
    CURL *curl_handle;
    CURLcode res;
    struct MemoryStruct chunk = { .memory = malloc(1), .size = 0 };
    project_detail_t *details = NULL;
    char url[256];
    if (!slug) return NULL;
    snprintf(url, sizeof(url), "%s/projects/%s/rev%d", "https://badgehub.p1m.nl/api/v3", slug, revision);
    if (chunk.memory == NULL) return NULL;
    curl_global_init(CURL_GLOBAL_ALL);
    curl_handle = curl_easy_init();
    if (!curl_handle) { free(chunk.memory); return NULL; }
    curl_easy_setopt(curl_handle, CURLOPT_URL, url);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "lvgl-badgehub-client/1.0");
    res = curl_easy_perform(curl_handle);
    if (res == CURLE_OK) {
        cJSON *root = cJSON_Parse(chunk.memory);
        if (root) {
            details = calloc(1, sizeof(project_detail_t));
            if (details) {
                details->slug = strdup(slug);
                details->revision = revision;
                cJSON *version_obj = cJSON_GetObjectItemCaseSensitive(root, "version");
                if (version_obj) {
                    cJSON *metadata_obj = cJSON_GetObjectItemCaseSensitive(version_obj, "app_metadata");
                    details->name = get_json_string(metadata_obj, "name");
                    details->description = get_json_string(metadata_obj, "description");
                    details->author = get_json_string(metadata_obj, "author");
                    details->version = get_json_string(metadata_obj, "version");
                    details->published_at = get_json_string(version_obj, "published_at");
                    cJSON *files_array = cJSON_GetObjectItemCaseSensitive(version_obj, "files");
                    if (cJSON_IsArray(files_array)) {
                        details->file_count = cJSON_GetArraySize(files_array);
                        details->files = malloc(details->file_count * sizeof(project_file_t));
                        if (details->files) {
                            cJSON *file_json = NULL;
                            int i = 0;
                            cJSON_ArrayForEach(file_json, files_array) {
                                details->files[i].full_path = get_json_string(file_json, "full_path");
                                details->files[i].sha256 = get_json_string(file_json, "sha256");
                                details->files[i].url = get_json_string(file_json, "url");
                                i++;
                            }
                        }
                    }
                }
            }
            cJSON_Delete(root);
        }
    }
    curl_easy_cleanup(curl_handle);
    free(chunk.memory);
    curl_global_cleanup();
    return details;
}
bool download_project_file(const project_file_t* file_info, const char* project_slug) {
    if (!file_info || !file_info->url || !project_slug) return false;
    CURL *curl_handle;
    CURLcode res;
    FILE *fp;
    char local_path[512];
    long response_code = 0;
    bool success = false;
    snprintf(local_path, sizeof(local_path), "%s/%s/%s", INSTALLATION_DIR, project_slug, file_info->full_path);
    ensure_dir_exists(local_path);
    curl_handle = curl_easy_init();
    if (curl_handle) {
        fp = fopen(local_path, "wb");
        if (fp) {
            curl_easy_setopt(curl_handle, CURLOPT_URL, file_info->url);
            curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteFileCallback);
            curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, fp);
            res = curl_easy_perform(curl_handle);
            fclose(fp);
            if (res == CURLE_OK) {
                curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &response_code);
                if (response_code == 200) {
                    success = true;
                } else {
                    fprintf(stderr, "Download failed for %s: HTTP status %ld\n", file_info->url, response_code);
                    remove(local_path);
                }
            } else {
                fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            }
        } else {
            fprintf(stderr, "Failed to open file for writing: %s\n", local_path);
        }
        curl_easy_cleanup(curl_handle);
    }
    return success;
}
void free_project_details(project_detail_t *details) {
    if (!details) return;
    free(details->name);
    free(details->description);
    free(details->published_at);
    free(details->author);
    free(details->version);
    free(details->slug);
    if (details->files) {
        for (int i = 0; i < details->file_count; i++) {
            free(details->files[i].full_path);
            free(details->files[i].sha256);
            free(details->files[i].url);
        }
        free(details->files);
    }
    free(details);
}
