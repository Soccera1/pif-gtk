/*
 * This file is part of pif.
 *
 * pif is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * pif is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with pif.  If not, see <https://www.gnu.org/licenses/>.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include "pif.h"

#ifndef PIF_GTK
int main(int argc, char *argv[]) {
    char* homedir;
    uid_t uid = getuid();

    struct passwd *info = getpwuid(uid);
    if (info == NULL) {
        handle_error("Failed to get user information");
    }

    homedir = info->pw_dir;
    if (homedir == NULL) {
        handle_error("Home directory is NULL");
    }

    char fileloc[267];
    size_t len = snprintf(fileloc, sizeof(fileloc), "%s/.pif", homedir);
    if (len >= sizeof(fileloc)) {
        handle_error("Path too long");
    }

    char configloc[267];
    len = snprintf(configloc, sizeof(configloc), "%s/.pif-config", homedir);
    if (len >= sizeof(configloc)) {
        handle_error("Path too long");
    }

    load_rotation_config(configloc);

    char **rotation_songs = NULL;
    int num_rotation_songs = 0;
    get_todays_songs(fileloc, &rotation_songs, &num_rotation_songs);

    save_rotation_config(configloc);

    if (argc > 1 && strcmp(argv[1], "html") == 0) {
        print_html_output(rotation_songs, num_rotation_songs, fileloc);
    } else {
        if (num_rotation_songs > 0) {
            printf("Today's rotation songs to practice:\n");
            for (int i = 0; i < num_rotation_songs; i++) {
                printf("%d. %s\n", i + 1, rotation_songs[i]);
            }
        }

        FILE *file = fopen(fileloc, "r");
        if (file == NULL) {
            handle_error("Failed to open songs file");
        }

        char line[256];
        int has_frequency_songs = 0;
        while (fgets(line, sizeof(line), file)) {
            line[strcspn(line, "\n")] = 0;
            char *freq = strrchr(line, ' ');
            if (freq != NULL) {
                *freq = '\0';
                freq++;
                if (is_song_due(line, freq)) {
                    if (!has_frequency_songs) {
                        printf("\nSongs due for practice based on frequency:\n");
                        has_frequency_songs = 1;
                    }
                    printf("- %s (every %s days)\n", line, freq);
                } 
            }
        }
        fclose(file);
    }

    for (int i = 0; i < num_rotation_songs; i++) {
        free(rotation_songs[i]);
    }
    free(rotation_songs);

    return 0;
}
#endif