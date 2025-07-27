#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>  // For FILE
#include <stddef.h> // For size_t
#include "cjson/cJSON.h" // For cJSON

// Struct to help curl write data into a dynamically growing memory buffer
struct MemoryStruct {
    char *memory;
    size_t size;
};

// libcurl callback function to write received data into our MemoryStruct
size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp);

// libcurl callback to write downloaded file data to a FILE* handle
size_t WriteFileCallback(void *ptr, size_t size, size_t nmemb, FILE *stream);

// Helper function to safely extract a string from a cJSON object.
char* get_json_string(cJSON *json, const char *key);

/**
 * @brief Ensures the full directory path for a file exists, creating it if necessary.
 *
 * @param path The full path to the file (e.g., "dir1/dir2/file.txt").
 */
void ensure_dir_exists(const char *path);

#endif // UTILS_H
