#ifndef APP_DETAIL_H
#define APP_DETAIL_H

/**
 * @brief Creates the application detail view for a given project slug and revision.
 *
 * This function clears the screen, fetches detailed project information,
 * and displays it. It also provides a "Back" button to return to the list.
 *
 * @param slug The unique slug of the project to display.
 * @param revision The specific revision of the project to display.
 */
void create_app_detail_view(const char* slug, int revision);

#endif // APP_DETAIL_H
