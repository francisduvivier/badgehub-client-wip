#ifndef BADGEHUB_CLIENT_H
#define BADGEHUB_CLIENT_H

#include <stdbool.h>

// Represents a project summary from the main project list.
typedef struct {
    char *name;
    char *slug;
    char *description;
    char *project_url;
    char *icon_url;
    int revision; // The latest revision number of the project
} project_t;

// Represents a single file within a project.
typedef struct {
    char *full_path;
    char *sha256;
} project_file_t;

// Represents the detailed information for a single project.
typedef struct {
    char *name;
    char *description;
    char *published_at;
    char *author;
    char *version;
    char *slug;
    int revision; // The revision number for this specific detail view
    project_file_t *files;
    int file_count;
} project_detail_t;


// Fetches the list of projects from the BadgeHub API.
project_t *get_applications(int *project_count);

// Frees the memory allocated for an array of project summaries.
void free_applications(project_t *projects, int count);

// Fetches the detailed information for a single project by its slug and revision.
project_detail_t *get_project_details(const char *slug, int revision);

// Frees the memory allocated for a project_detail_t struct.
void free_project_details(project_detail_t *details);

// Downloads a single project file for a specific revision.
bool download_project_file(const char* slug, int revision, const project_file_t* file_info);


#endif // BADGEHUB_CLIENT_H
