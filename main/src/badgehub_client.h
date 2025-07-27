#ifndef BADGEHUB_CLIENT_H
#define BADGEHUB_CLIENT_H

#include <stdbool.h>
#include "lvgl/lvgl.h" // Needed for lv_image_dsc_t

// Represents a project summary from the main project list.
typedef struct {
    char *name;
    char *slug;
    char *description;
    int revision;

    // --- NEW: In-memory icon data ---
    lv_image_dsc_t icon_dsc; // LVGL descriptor pointing to the raw PNG data
    uint8_t* icon_data;      // The raw PNG data buffer that needs to be freed
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

// The separate download_icon function is no longer needed.

#endif // BADGEHUB_CLIENT_H
