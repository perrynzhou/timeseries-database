
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include "cstring.h"
#include "util.h"
void string_init(struct string *str)
{
    str->len = 0;
    str->data = NULL;
}

void string_deinit(struct string *str)
{
    assert((str->len == 0 && str->data == NULL) ||
           (str->len != 0 && str->data != NULL));

    if (str->data != NULL)
    {
        free(str->data);
        string_init(str);
    }
}

bool string_empty(const struct string *str)
{
    assert((str->len == 0 && str->data == NULL) ||
           (str->len != 0 && str->data != NULL));
    return str->len == 0 ? true : false;
}

int string_duplicate(struct string *dst, const struct string *src)
{
    assert(dst->len == 0 && dst->data == NULL);
    assert(src->len != 0 && src->data != NULL);

    dst->data = (uint8_t *)strndup(src->data, src->len + 1);
    if (dst->data == NULL)
    {
        return -1;
    }

    dst->len = src->len;
    dst->data[dst->len] = '\0';

    return 0;
}

int string_copy(struct string *dst, const uint8_t *src, uint32_t srclen)
{
    assert(dst->len == 0 && dst->data == NULL);
    assert(src != NULL && srclen != 0);

    dst->data = strndup(src, srclen + 1);
    if (dst->data == NULL)
    {
        return -1;
    }

    dst->len = srclen;
    dst->data[dst->len] = '\0';

    return 0;
}

int string_compare(const struct string *s1, const struct string *s2)
{
    if (s1->len != s2->len)
    {
        return s1->len > s2->len ? 1 : -1;
    }

    return strncmp((char *)s1->data, (char *)s2->data, s1->len);
}