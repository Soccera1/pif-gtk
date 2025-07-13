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

#ifndef PIF_H
#define PIF_H

#include <stdio.h>

// Rotation configuration
extern int songs_per_day;
extern int last_played;

void handle_error(const char *msg);
char* get_word(const char* line, int word_num);
void ins_txt(const char* fileloc, int line_num, int word_num, const char* text);
void load_rotation_config(const char *configloc);
void save_rotation_config(const char *configloc);
void get_todays_songs(const char *fileloc, char ***songs, int *num_songs);
int is_song_due(const char *song_name, const char *freq);
void print_html_output(char **rotation_songs, int num_rotation_songs, const char *fileloc);

#endif // PIF_H