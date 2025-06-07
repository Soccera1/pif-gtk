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

// Rotation configuration
int songs_per_day = 3;  // Default value
int last_played = 0;    // Track last played song

void handle_error(const char *msg) {
  fprintf(stderr, "Error: %s: %s\n", msg, strerror(errno));
  exit(1);
}

char* get_word(const char* line, int word_num) {
  if (line == NULL || word_num < 0) {
    return NULL;
  }

  char* line_copy = strdup(line);
  if (line_copy == NULL) {
    return NULL;
  }

  char* word = strtok(line_copy, " \t");
  int current_word = 0;

  while (word != NULL && current_word < word_num) {
    word = strtok(NULL, " \t");
    current_word++;
  }

  char* result = NULL;
  if (word != NULL) {
    result = strdup(word);
  }

  free(line_copy);
  return result;
}

void ins_txt(const char* fileloc, int line_num, int word_num, const char* text) {
  FILE* file = fopen(fileloc, "r+");
  if (file == NULL) {
    handle_error("Failed to open file for reading and writing");
  }

  // Create a temporary file
  char temp_path[] = "/tmp/pif_temp_XXXXXX";
  int temp_fd = mkstemp(temp_path);
  if (temp_fd == -1) {
    fclose(file);
    handle_error("Failed to create temporary file");
  }
  FILE* temp_file = fdopen(temp_fd, "w");
  if (temp_file == NULL) {
    close(temp_fd);
    fclose(file);
    handle_error("Failed to open temporary file");
  }

  char line[1024];
  int current_line = 1;
  int inserted = 0;

  // Copy lines to temp file, inserting text at specified position
  while (fgets(line, sizeof(line), file)) {
    if (current_line == line_num) {
      char* word = get_word(line, word_num);
      if (word != NULL) {
        // Insert text at the specified word position
        char* word_start = strstr(line, word);
        if (word_start != NULL) {
          fwrite(line, 1, word_start - line, temp_file);
          fputs(text, temp_file);
          fputs(" ", temp_file);
          fputs(word_start, temp_file);
          inserted = 1;
        }
      }
      free(word);
    } else {
      fputs(line, temp_file);
    }
    current_line++;
  }

  fclose(file);
  fclose(temp_file);

  // Replace original file with temp file
  if (rename(temp_path, fileloc) != 0) {
    handle_error("Failed to replace original file");
  }

  if (!inserted) {
    printf("Warning: Could not insert text at specified position\n");
  }
}

void load_rotation_config(const char *configloc) {
    FILE *file = fopen(configloc, "r");
    if (file == NULL) {
        if (errno != ENOENT) {
            handle_error("Failed to open config file");
        }
        return;
    }

    char line[256];
    if (fgets(line, sizeof(line), file)) {
        sscanf(line, "songs_per_day=%d\n", &songs_per_day);
    }
    if (fgets(line, sizeof(line), file)) {
        sscanf(line, "last_played=%d\n", &last_played);
    }
    fclose(file);
}

void save_rotation_config(const char *configloc) {
    FILE *file = fopen(configloc, "w");
    if (file == NULL) {
        handle_error("Failed to open config file for writing");
        return;
    }

    fprintf(file, "songs_per_day=%d\n", songs_per_day);
    fprintf(file, "last_played=%d\n", last_played);
    fclose(file);
}

// Function to get songs for today's rotation
void get_todays_songs(const char *fileloc, char ***songs, int *num_songs) {
    FILE *file = fopen(fileloc, "r");
    if (file == NULL) {
        handle_error("Failed to open songs file");
    }

    // First pass: count total number of rotation songs
    int total_rotation_songs = 0;
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0;  // Remove newline
        char *freq = strrchr(line, ' ');
        if (freq != NULL && strcmp(freq + 1, "rot") == 0) {
            total_rotation_songs++;
        }
    }
    rewind(file);

    if (total_rotation_songs == 0) {
        *num_songs = 0;
        *songs = NULL;
        fclose(file);
        return;
    }

    // Calculate which songs to play today
    int start_idx = last_played % total_rotation_songs;
    *num_songs = (songs_per_day < total_rotation_songs) ? songs_per_day : total_rotation_songs;
    
    // Allocate memory for songs
    *songs = malloc(sizeof(char*) * (*num_songs));
    if (*songs == NULL) {
        fclose(file);
        handle_error("Memory allocation failed");
    }

    // Second pass: collect rotation songs
    int current_rotation_song = 0;
    int songs_collected = 0;
    while (fgets(line, sizeof(line), file) && songs_collected < *num_songs) {
        line[strcspn(line, "\n")] = 0;  // Remove newline
        char *freq = strrchr(line, ' ');
        
        if (freq != NULL && strcmp(freq + 1, "rot") == 0) {
            if (current_rotation_song >= start_idx) {
                // Extract song name (everything before the last space)
                *freq = '\0';
                (*songs)[songs_collected] = strdup(line);
                if ((*songs)[songs_collected] == NULL) {
                    for (int j = 0; j < songs_collected; j++) {
                        free((*songs)[j]);
                    }
                    free(*songs);
                    fclose(file);
                    handle_error("Memory allocation failed");
                }
                songs_collected++;
            }
            current_rotation_song++;
        }
    }

    // Update last_played
    last_played = (start_idx + *num_songs) % total_rotation_songs;
    fclose(file);
}

// Function to check if a song is due for practice based on frequency
int is_song_due(const char *song_name, const char *freq) {
    if (freq == NULL || *freq == '\0') {
        return 0;  // Ignore songs with no frequency
    }

    if (strcmp(freq, "rot") == 0) {
        return 0;  // Rotation songs are handled separately
    }

    // Check if frequency is a valid number
    char *endptr;
    long days = strtol(freq, &endptr, 10);
    if (*endptr != '\0' || days <= 0) {
        return 0;  // Invalid frequency
    }

    // Check last practice time
    char last_practice_file[512];
    snprintf(last_practice_file, sizeof(last_practice_file), "%s/.pif_last_practice_%s", getenv("HOME"), song_name);
    
    struct stat st;
    if (stat(last_practice_file, &st) == -1) {
        return 1;  // No last practice record, so it's due
    }

    time_t now = time(NULL);
    time_t last_practice = st.st_mtime;
    time_t days_since = (now - last_practice) / (24 * 3600);

    return days_since >= days;
}

int main(void) {
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

    // Load rotation config
    load_rotation_config(configloc);

    // Get today's rotation songs
    char **rotation_songs = NULL;
    int num_rotation_songs = 0;
    get_todays_songs(fileloc, &rotation_songs, &num_rotation_songs);

    // Save updated rotation config
    save_rotation_config(configloc);

    // Print today's rotation songs
    if (num_rotation_songs > 0) {
        printf("Today's rotation songs to practice:\n");
        for (int i = 0; i < num_rotation_songs; i++) {
            printf("%d. %s\n", i + 1, rotation_songs[i]);
            free(rotation_songs[i]);
        }
        free(rotation_songs);
    }

    // Check frequency-based songs
    FILE *file = fopen(fileloc, "r");
    if (file == NULL) {
        handle_error("Failed to open songs file");
    }

    char line[256];
    int has_frequency_songs = 0;
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0;  // Remove newline
        char *freq = strrchr(line, ' ');
        if (freq != NULL) {
            *freq = '\0';  // Split song name and frequency
            freq++;  // Move past the space
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

    return 0;
}