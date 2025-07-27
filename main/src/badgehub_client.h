#ifndef BADGEHUB_CLIENT_H
#define BADGEHUB_CLIENT_H

#include <stdbool.h>
#include "lvgl/lvgl.h"

// Represents a project summary from the main project list.
typedef struct {
    char *name;
    char *slug;
    char *description;
    char *project_url;
    char *icon_url; // We now store the URL, not the downloaded data.
    int revision;
} project_t;

// Represents a single file within a project.
typedef struct {
    char *full_path;
    char *sha256;
    char *url;
} project_file_t;

// Represents the detailed information for a single project.
typedef struct {
    char *name;
    char *description;
    char *published_at;
    char *author;
    char *version;
    char *slug;
    int revision;
    project_file_t *files;
    int file_count;
} project_detail_t;


project_t *get_applications(int *project_count, const char* search_query, int limit, int offset);
void free_applications(project_t *projects, int count);
project_detail_t *get_project_details(const char *slug, int revision);
void free_project_details(project_detail_t *details);
bool download_project_file(const project_file_t* file_info, const char* project_slug);

/**
 * @brief Downloads an icon from a URL into a memory buffer.
 *
 * @param icon_url The direct URL of the icon to download.
 * @param data_size A pointer that will be populated with the size of the downloaded data.
 * @return A dynamically allocated buffer with the raw PNG data, or NULL on failure. The caller must free this buffer.
 */
uint8_t* download_icon_to_memory(const char* icon_url, size_t* data_size);

#endif // BADGEHUB_CLIENT_H
