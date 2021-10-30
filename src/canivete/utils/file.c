#include "file.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

// posix
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <canivete/error.h>
#include <stb_ds.h>

// TODO this function, the way it is, cant differentiate empty files from errors. improve API
struct str file_read_to_str(const char* file_path)
{
    assert(file_path != NULL);

    FILE* file = fopen(file_path, "r");
    if (!file) {
        fprintf(stderr, "error: io: fopen failed: %s. file_path=\"%s\"\n", strerror(errno), file_path);
        return str_empty();
    }

    if (fseek(file, 0L, SEEK_END) != 0) {
        fprintf(stderr, "error: io: fseek failed: %s. file_path=\"%s\"\n", strerror(errno), file_path);
        goto err;
    }

    long file_size = ftell(file);
    if (file_size < 0) {
        fprintf(stderr, "error: io: ftell failed: %s. file_path=\"%s\"\n", strerror(errno), file_path);
        goto err;
    }

    size_t content_len = (size_t) file_size;
    size_t content_capacity = content_len + 1;

    char* content = calloc(1, content_capacity);
    if (content == NULL) {
        fprintf(stderr, "error: out of memory. file_path=\"%s\"\n", file_path);
        goto err;
    }

    if (fseek(file, 0, SEEK_SET) != 0) {
        fprintf(stderr, "error: io: fseek failed: %s. file_path=\"%s\"\n", strerror(errno), file_path);
        goto err_buf;
    }

    size_t bytes_read = fread(content, sizeof(char), content_len, file);
    if (bytes_read != content_len) {
        fprintf(stderr, "error: io: fread: not every byte from file could be read: %s. bytes_read=%zu file_path=\"%s\"\n", strerror(errno), bytes_read, file_path);
        goto err_buf;
    }    
    fclose(file);

    content[content_len] = '\0';

    return (struct str) {
        .cap = content_capacity,
        .len = content_len,
        .cstr = content,
    };

err_buf:
    free(content);
err:
    fclose(file);
    return str_empty();
}

int file_read_files_from_dir(const char* dir_path, bool (*select)(struct dirent* dir_entry), struct str** file_contents_arraylist)
{
    assert(file_contents_arraylist != NULL);

    DIR* dir = opendir(dir_path);
    if (!dir) {
        fprintf(stderr, "error: io: failed to open directory \"%s\": %s\n", dir_path, strerror(errno));
        goto err;
    }

    struct dirent* dir_entry = NULL;
    errno = 0;
    while ((dir_entry = readdir(dir)) != NULL) {
        if (!select(dir_entry)) {
            continue;
        }
        assert(sizeof(dir_entry->d_name) <= 256);
        const char* file_name = dir_entry->d_name;

        // d_name limit is 256 and dir_path will not be big enough to overflow this.
        char file_path[1024] = {0};
        const size_t file_path_cap = sizeof(file_path);
        strncat(file_path, dir_path, file_path_cap - 1);
        strncat(file_path, file_name, file_path_cap - 1);

        struct str file_content = file_read_to_str(file_path);
        if (str_is_empty(&file_content)) {
            goto err_free_arraylist;
        }

        arrput(*file_contents_arraylist, file_content);
        
        errno = 0;
    }
    if (errno != 0) {
        fprintf(stderr, "error: io: failed to read files from directory \"%s\": %s\n", dir_path, strerror(errno));
        goto err_close_dir;
    }

    closedir(dir);
    return CANIVETE_OK;

err_free_arraylist:
    if (*file_contents_arraylist) {
        arrfree(*file_contents_arraylist);
    }
err_close_dir:
    closedir(dir);
err:
    *file_contents_arraylist = NULL;
    return CANIVETE_ERR_IO;
}

bool select_regular_files(struct dirent* dir_entry)
{
    return dir_entry->d_type == DT_REG;
}

bool select_min_file_len(struct dirent* dir_entry, size_t min_file_len)
{
    const size_t file_name_len = strlen(dir_entry->d_name);
    return file_name_len >= min_file_len;
}

int ensure_directory(const char* dir_path)
{
    int rc = mkdir(dir_path, 0755);
    if (rc == 0 || errno == EEXIST) {
        return 0;
    }
    return errno;
}
