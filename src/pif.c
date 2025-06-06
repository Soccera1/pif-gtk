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

  FILE *file = fopen(fileloc, "r");
  if (file == NULL) {
    if (errno != ENOENT) {
      handle_error("Failed to open file");
    }

    printf("File does not exist. Creating file.\n");
    file = fopen(fileloc, "w");
    if (file == NULL) {
      handle_error("Failed to create file");
    }

    fclose(file);

    struct stat st;
    if (stat(fileloc, &st) == -1) {
      handle_error("Failed to stat file");
    }

    // Check if file has correct permissions (readable/writable by owner only)
    if ((st.st_mode & 0600) != 0600) {
      printf("Warning: File has incorrect permissions. Setting to 600.\n");
      if (chmod(fileloc, 0600) == -1) {
        handle_error("Failed to set file permissions");
      }
    }
  } else {
    fclose(file);
  }

  while (1) {
    printf("Choose an option:\n1. Add song\n2. Remove song\n3. Modify frequency\n4. Add systemd service\n5. Show songs\n6. Exit\n");
    char option[100];
    if (fgets(option, sizeof(option), stdin) == NULL) {
      handle_error("Failed to read menu option");
    }
    // Remove newline if present
    option[strcspn(option, "\n")] = 0;
    
    // Handle numeric input
    int choice = 0;
    if (option[0] >= '1' && option[0] <= '6' && option[1] == '\0') {
      choice = option[0] - '0';
    }
    
    if (choice == 1 || strcmp(option, "Add song") == 0) {
      printf("Enter the song name: ");
      char song_name[100];
      if (fgets(song_name, sizeof(song_name), stdin) == NULL) {
        handle_error("Failed to read song name");
      }
      // Remove newline if present
      song_name[strcspn(song_name, "\n")] = 0;
      
      FILE *file = fopen(fileloc, "a");
      if (file == NULL) {
          handle_error("Failed to open file for appending");
      }
      
      fprintf(file, "%s\n", song_name);
      fclose(file);
    }
    else if (choice == 2 || strcmp(option, "Remove song") == 0) {
      printf("Enter the song name: ");
      char song_name[100];
      if (fgets(song_name, sizeof(song_name), stdin) == NULL) {
        handle_error("Failed to read song name");
      }
      // Remove newline if present
      song_name[strcspn(song_name, "\n")] = 0;
      
      // Read entire file into memory
      char *content = NULL;
      size_t content_size = 0;
      FILE *file = fopen(fileloc, "r");
      if (file == NULL) {
          handle_error("Failed to open file for reading");
      }
      
      char line[256];
      int found = 0;
      while (fgets(line, sizeof(line), file)) {
          line[strcspn(line, "\n")] = 0;
          if (strcmp(line, song_name) != 0) {
              // Append line to content
              size_t line_len = strlen(line);
              char *new_content = realloc(content, content_size + line_len + 2);
              if (new_content == NULL) {
                  free(content);
                  fclose(file);
                  handle_error("Memory allocation failed");
              }
              content = new_content;
              strcpy(content + content_size, line);
              content_size += line_len;
              strcpy(content + content_size, "\n");
              content_size += 1;
          } else {
              found = 1;
          }
      }
      fclose(file);
      
      // Write back to file
      file = fopen(fileloc, "w");
      if (file == NULL) {
          free(content);
          handle_error("Failed to open file for writing");
      }
      
      if (content != NULL) {
          fwrite(content, 1, content_size, file);
          free(content);
      }
      fclose(file);
      
      if (!found) {
          printf("Song '%s' not found in the file.\n", song_name);
      } else {
          printf("Song '%s' removed successfully.\n", song_name);
      }
    }
    else if (choice == 3 || strcmp(option, "Modify frequency") == 0) {
      printf("Enter the song name: ");
      char song_name[100];
      if (fgets(song_name, sizeof(song_name), stdin) == NULL) {
        handle_error("Failed to read song name");
      }
      // Remove newline if present
      song_name[strcspn(song_name, "\n")] = 0;
      printf("Enter the frequency (as an integer): ");
      int frequency;
      scanf("%d", &frequency);
      
      // Read entire file into memory
      char *content = NULL;
      size_t content_size = 0;
      FILE *file = fopen(fileloc, "r");
      if (file == NULL) {
          handle_error("Failed to open file for reading");
      }
      
      char line[256];
      int found = 0;
      while (fgets(line, sizeof(line), file)) {
          line[strcspn(line, "\n")] = 0;
          if (strcmp(line, song_name) == 0) {
              // Append modified line
              char modified_line[256];
              snprintf(modified_line, sizeof(modified_line), "%s %d", line, frequency);
              size_t line_len = strlen(modified_line);
              char *new_content = realloc(content, content_size + line_len + 2);
              if (new_content == NULL) {
                  free(content);
                  fclose(file);
                  handle_error("Memory allocation failed");
              }
              content = new_content;
              strcpy(content + content_size, modified_line);
              content_size += line_len;
              strcpy(content + content_size, "\n");
              content_size += 1;
              found = 1;
          } else {
              // Append original line
              size_t line_len = strlen(line);
              char *new_content = realloc(content, content_size + line_len + 2);
              if (new_content == NULL) {
                  free(content);
                  fclose(file);
                  handle_error("Memory allocation failed");
              }
              content = new_content;
              strcpy(content + content_size, line);
              content_size += line_len;
              strcpy(content + content_size, "\n");
              content_size += 1;
          }
      }
      fclose(file);
      
      // Write back to file
      file = fopen(fileloc, "w");
      if (file == NULL) {
          free(content);
          handle_error("Failed to open file for writing");
      }
      
      if (content != NULL) {
          fwrite(content, 1, content_size, file);
          free(content);
      }
      fclose(file);

      if (!found) {
          printf("Song '%s' not found in the file.\n", song_name);
      } else {
          printf("Frequency for '%s' modified successfully.\n", song_name);
      }
    }
    else if (choice == 4 || strcmp(option, "Add systemd service") == 0) {
      system("sudo ./install-pif-notify.sh");
    }
    else if (choice == 5 || strcmp(option, "Show songs") == 0) {
      FILE *file = fopen(fileloc, "r");
      if (file == NULL) {
        handle_error("Failed to open file for reading");
      }
      char line[256];
      printf("\nCurrent songs and practice frequencies:\n");
      printf("----------------------------------------\n");
      while (fgets(line, sizeof(line), file)) {
        printf("%s", line);
      }
      printf("----------------------------------------\n\n");
      fclose(file);
    }
    else if (choice == 6 || strcmp(option, "Exit") == 0) {
      break;
    }
    else {
      printf("\033[31m\nInvalid option. Please try again.\n\n\033[0m");
    }
  }
  return 0;
}