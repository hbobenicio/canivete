#ifndef CANIVETE_STR_H
#define CANIVETE_STR_H

#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>

/**
 * Heap-allocated byte array which represents an owned null-terminated c-string.
 */
struct str {
    /// The pointer to the heap-allocated buffer that contains a null-terminated c-string
    char* cstr;
    /// String's length
    size_t len;
    /// String's capacity
    size_t cap;
};

/**
 * Borrowed string from a null-terminated cstr.
 */
struct str_borrow {
    const char* cstr;
    size_t len;
};

/**
 * Borrowed string view.
 */
struct str_view {
    /// The start of the string
    const char* ptr;
    /// String's length
    size_t len;
};

#define STR_VIEW(cstr) { .ptr = cstr, .len = (sizeof(cstr) / sizeof(cstr[0])) - 1 }
#define STR_BORROW_VIEW(s) { .cstr = s, .len = (sizeof(s) / sizeof(s[0])) - 1 }

struct str str_empty(void);
void str_free(struct str* s);
bool str_is_empty(struct str* s);

struct str_borrow str_borrow_from_cstr(const char* cstr);

struct str_view str_view_empty();
struct str_view str_view_from_cstr(const char* str);
struct str_view str_view_from_sized_cstr(const char* str, size_t str_len);
size_t str_view_fwrite(struct str_view sv, FILE* stream);
int str_view_cmp(struct str_view a, struct str_view b);
int str_view_cmp_cstr(struct str_view a, const char* b);
int str_view_cmp_sized_cstr(struct str_view a, const char* b, size_t b_len);

#endif
