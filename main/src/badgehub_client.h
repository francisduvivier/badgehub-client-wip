#ifndef BADGEHUB_CLIENT_H
#define BADGEHUB_CLIENT_H

#include <stdbool.h>

/**
 * @brief Represents a single project from the BadgeHub API.
 * * Note that all string fields are dynamically allocated and must be freed.
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
 * @brief Fetches the list of projects from the BadgeHub API.
 *
 * This function performs an HTTP GET request, parses the resulting JSON,
 * and returns a dynamically allocated array of project_t structs.
 *
 * @param project_count A pointer to an integer that will be populated with the
 * number of projects in the returned array.
 * @return A pointer to a dynamically allocated array of project_t structs on success,
 * or NULL on failure. The caller is responsible for freeing the memory
 * allocated for the array and its contents by calling free_applications().
 */
project_t *get_applications(int *project_count);

/**
 * @brief Frees the memory allocated for an array of projects.
 *
 * This function safely iterates through the project array, freeing each
 * dynamically allocated string field and then the array itself.
 *
 * @param projects The array of projects to free.
 * @param count The number of projects in the array.
 */
void free_applications(project_t *projects, int count);

#endif // BADGEHUB_CLIENT_H
