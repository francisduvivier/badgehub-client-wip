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
    bool public;
} project_t;

// Represents a single file within a project.
typedef struct {
    char *full_path; // The relative path of the file to be downloaded
    char *sha256;    // The hash of the file for verification (not used in this example)
} project_file_t;

// Represents the detailed information for a single project.
typedef struct {
    char *name;
    char *description;
    char *published_at;
    char *author;
    char *version;
    char *slug; // Keep a copy of the slug for download URLs
    project_file_t *files; // A dynamic array of files
    int file_count;        // The number of files in the array
} project_detail_t;


// Fetches the list of projects from the BadgeHub API.
project_t *get_applications(int *project_count);

// Frees the memory allocated for an array of project summaries.
void free_applications(project_t *projects, int count);

// Fetches the detailed information for a single project by its slug.
project_detail_t *get_project_details(const char *slug);

// Frees the memory allocated for a project_detail_t struct.
void free_project_details(project_detail_t *details);

/**
 * @brief Downloads a single project file.
 *
 * Constructs the download URL and saves the file to the specified local path.
 *
 * @param slug The slug of the project.
 * @param file_info A pointer to the file information containing the path.
 * @return True on success, false on failure.
 */
bool download_project_file(const char* slug, const project_file_t* file_info);


#endif // BADGEHUB_CLIENT_H
