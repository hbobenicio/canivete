#include "str.h"

#include <stdlib.h>
#include <string.h>

struct str str_empty(void)
{
    return (struct str) {
        .cap = 0,
        .len = 0,
        .cstr = NULL,
    };
}

void str_free(struct str* s)
{
    free(s->cstr);
    s->cstr = NULL;
    s->cap = s->len = 0;
}

bool str_is_empty(struct str* s)
{
    if (s == NULL)
        return true;
    return s->cstr == NULL;
}

struct str_borrow str_borrow_from_cstr(const char* cstr)
{
    return (struct str_borrow) {
        .cstr = cstr,
        .len = strlen(cstr),
    };
}

struct str_view str_view_empty()
{
    struct str_view sv = {0};
    return sv;
}

struct str_view str_view_from_cstr(const char *cstr)
{
    return str_view_from_sized_cstr(cstr, strlen(cstr));
}

struct str_view str_view_from_sized_cstr(const char* cstr, size_t str_len)
{
    return (struct str_view) {
        .ptr = cstr,
        .len = str_len,
    };
}

size_t str_view_fwrite(struct str_view sv, FILE* stream)
{
    if (sv.ptr != NULL && sv.len > 0) {
        return fwrite(sv.ptr, sizeof(char), sv.len, stream);
    }
    return 0;
}

int str_view_cmp(struct str_view a, struct str_view b)
{
    return memcmp(a.ptr, b.ptr, (a.len < b.len) ? a.len : b.len);
}

int str_view_cmp_cstr(struct str_view a, const char* b)
{
    return str_view_cmp_sized_cstr(a, b, strlen(b));
}

int str_view_cmp_sized_cstr(struct str_view a, const char* b, size_t b_len)
{
    return str_view_cmp(a, str_view_from_sized_cstr(b, b_len));
}
