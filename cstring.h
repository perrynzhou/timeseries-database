#ifndef _CSTRING_H_
#define _CSTRING_H_

#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>
struct string
{
    uint32_t len;  /* string length */
    uint8_t *data; /* string data */
};

#define string(_str)                        \
    {                                       \
        sizeof(_str) - 1, (uint8_t *)(_str) \
    }
#define null_string \
    {               \
        0, NULL     \
    }

#define string_set_text(_str, _text)                 \
    do                                               \
    {                                                \
        (_str)->len = (uint32_t)(sizeof(_text) - 1); \
        (_str)->data = (uint8_t *)(_text);           \
    } while (0);

#define string_set_raw(_str, _raw)              \
    do                                          \
    {                                           \
        (_str)->len = (uint32_t)(strlen(_raw)); \
        (_str)->data = (uint8_t *)(_raw);       \
    } while (0);
void string_init(struct string *str);
void string_deinit(struct string *str);
bool string_empty(const struct string *str);
int string_duplicate(struct string *dst, const struct string *src);
int string_copy(struct string *dst, const uint8_t *src, uint32_t srclen);
int string_compare(const struct string *s1, const struct string *s2);
#endif
