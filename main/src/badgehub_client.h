#ifndef BADGEHUB_CLIENT_H
#define BADGEHUB_CLIENT_H

#include <stdbool.h>

/**
 * @brief Represents a project summary from the main project list.
 */
typedef struct {
    int id;
    char *name;
    char *slug;
    char *description;
    char *project_url;
    char *icon_url;
    bool public;
} project_t;

/**
 * @brief Represents the detailed information for a single project.
 * Note that all string fields are dynamically allocated and must be freed.
 */
typedef struct {
    char *name;
    char *description;
    char *published_at;
    char *author;
    char *version;
} project_detail_t;


/**
 * @brief Fetches the list of projects from the BadgeHub API.
 * @param project_count A pointer to an integer that will be populated with the number of projects.
 * @return A pointer to a dynamically allocated array of project_t structs, or NULL on failure.
 */
project_t *get_applications(int *project_count);

/**
 * @brief Frees the memory allocated for an array of project summaries.
 */
void free_applications(project_t *projects, int count);

/**
 * @brief Fetches the detailed information for a single project by its slug.
 * @param slug The unique slug of the project to fetch.
 * @return A pointer to a dynamically allocated project_detail_t struct, or NULL on failure.
 */
project_detail_t *get_project_details(const char *slug);

/**
 * @brief Frees the memory allocated for a project_detail_t struct.
 */
void free_project_details(project_detail_t *details);

#endif // BADGEHUB_CLIENT_H
