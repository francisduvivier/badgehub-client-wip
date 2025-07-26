#include "badgehub_client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "cjson/cJSON.h"
#include <sys/stat.h> // For mkdir
#include <errno.h>    // For errno

#define BADGEHUB_API_BASE_URL "https://badgehub.p1m.nl/api/v3"
#define INSTALLATION_DIR "installation_dir"

// --- URL Templates ---
#define PROJECTS_URL_TEMPLATE "%s/projects"
// CORRECTED: Restored "rev" to the URL template as per your instruction
#define PROJECT_DETAIL_URL_TEMPLATE "%s/projects/%s/rev%d"
#define PROJECT_FILE_URL_TEMPLATE "%s/projects/%s/rev%d/files/%s"


// --- FORWARD DECLARATIONS ---
static void ensure_dir_exists(const char *path);

// Struct to help curl write data into a dynamically growing memory buffer
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
        printf("not enough memory (realloc returned NULL)\n");
        return 0;
    }
    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;
    return realsize;
}

// libcurl callback to write downloaded file data to a FILE* handle
static size_t WriteFileCallback(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    return fwrite(ptr, size, nmemb, stream);
}


// Helper function to safely extract a string from a cJSON object.
static char* get_json_string(cJSON *json, const char *key) {
    if (!json) return NULL;
    cJSON *item = cJSON_GetObjectItemCaseSensitive(json, key);
    if (cJSON_IsString(item) && (item->valuestring != NULL)) {
        char *str = malloc(strlen(item->valuestring) + 1);
        if (str) {
            strcpy(str, item->valuestring);
        }
        return str;
    }
    char *empty_str = malloc(1);
    if(empty_str) *empty_str = '\0';
    return empty_str;
}

/**
 * @brief Ensures the full directory path for a file exists, creating it if necessary.
 *
 * @param path The full path to the file (e.g., "dir1/dir2/file.txt").
 */
static void ensure_dir_exists(const char *path) {
    char *path_copy = strdup(path);
    if (!path_copy) return;

    // Iterate through the path and create directories level by level
    for (char *p = path_copy; *p; p++) {
        if (*p == '/') {
            *p = '\0'; // Temporarily terminate the string
            // Create directory, ignore error if it already exists
            if (mkdir(path_copy, 0755) != 0 && errno != EEXIST) {
                 fprintf(stderr, "Failed to create directory %s\n", path_copy);
            }
            *p = '/'; // Restore the slash
        }
    }
    free(path_copy);
}

project_t *get_applications(int *project_count, const char* search_query) {
    *project_count = 0;
    CURL *curl_handle;
    CURLcode res;
    struct MemoryStruct chunk = { .memory = malloc(1), .size = 0 };
    project_t *projects = NULL;
    char url[512];

    // Build the base URL
    snprintf(url, sizeof(url), PROJECTS_URL_TEMPLATE, BADGEHUB_API_BASE_URL);

    curl_global_init(CURL_GLOBAL_ALL);
    curl_handle = curl_easy_init();
    if (!curl_handle) { free(chunk.memory); return NULL; }

    // If there's a search query, append it
    if (search_query && strlen(search_query) > 0) {
        char *escaped_query = curl_easy_escape(curl_handle, search_query, 0);
        if (escaped_query) {
            snprintf(url, sizeof(url), "%s/projects?q=%s", BADGEHUB_API_BASE_URL, escaped_query);
            curl_free(escaped_query);
        }
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
            projects = malloc(*project_count * sizeof(project_t));
            if (projects) {
                cJSON *proj_json = NULL;
                int i = 0;
                cJSON_ArrayForEach(proj_json, root) {
                    projects[i].name = get_json_string(proj_json, "name");
                    projects[i].slug = get_json_string(proj_json, "slug");
                    projects[i].description = get_json_string(proj_json, "description");
                    projects[i].project_url = get_json_string(proj_json, "project_url");
                    projects[i].icon_url = get_json_string(proj_json, "icon_url");
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

project_detail_t *get_project_details(const char *slug, int revision) {
    CURL *curl_handle;
    CURLcode res;
    struct MemoryStruct chunk = { .memory = malloc(1), .size = 0 };
    project_detail_t *details = NULL;
    char url[256];
    if (!slug) return NULL;
    snprintf(url, sizeof(url), PROJECT_DETAIL_URL_TEMPLATE, BADGEHUB_API_BASE_URL, slug, revision);
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

bool download_project_file(const char* slug, int revision, const project_file_t* file_info) {
    CURL *curl_handle;
    CURLcode res;
    FILE *fp;
    char url[512];
    char local_path[512];
    long response_code = 0;
    bool success = false;
    snprintf(url, sizeof(url), PROJECT_FILE_URL_TEMPLATE, BADGEHUB_API_BASE_URL, slug, revision, file_info->full_path);
    snprintf(local_path, sizeof(local_path), "%s/%s/%s", INSTALLATION_DIR, slug, file_info->full_path);
    ensure_dir_exists(local_path);
    curl_handle = curl_easy_init();
    if (curl_handle) {
        fp = fopen(local_path, "wb");
        if (fp) {
            curl_easy_setopt(curl_handle, CURLOPT_URL, url);
            curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteFileCallback);
            curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, fp);
            res = curl_easy_perform(curl_handle);
            fclose(fp);
            if (res == CURLE_OK) {
                curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &response_code);
                if (response_code == 200) {
                    success = true;
                } else {
                    fprintf(stderr, "Download failed for %s: HTTP status %ld\n", url, response_code);
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
        }
        free(details->files);
    }
    free(details);
}

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
