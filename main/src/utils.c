#include "utils.h"
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

// libcurl callback function to write received data into our MemoryStruct
size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
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
size_t WriteFileCallback(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    return fwrite(ptr, size, nmemb, stream);
}

// Helper function to safely extract a string from a cJSON object.
char* get_json_string(cJSON *json, const char *key) {
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
void ensure_dir_exists(const char *path) {
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
