#ifndef CANIVETE_UTILS_FILE_H
#define CANIVETE_UTILS_FILE_H

#include <stdbool.h>
#include <dirent.h>
#include <canivete/str.h>

struct str file_read_to_str(const char* file_path);
int file_read_files_from_dir(const char* dir_path, bool (*select)(struct dirent* dir_entry), struct str** file_contents_arraylist);

bool select_regular_files(struct dirent* dir_entry);
bool select_min_file_len(struct dirent* dir_entry, size_t min_file_len);

int ensure_directory(const char* dir_path);

#endif
