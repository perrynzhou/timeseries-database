
#include "array.h"
#include <stdlib.h>
#include <assert.h>
struct array *array_create(uint32_t n, size_t size)
{
    struct array *a;

    assert(n != 0 && size != 0);

    a = malloc(sizeof(*a));
    if (a == NULL)
    {
        return NULL;
    }

    a->elem = malloc(n * size);
    if (a->elem == NULL)
    {
        free(a);
        return NULL;
    }

    a->nelem = 0;
    a->size = size;
    a->nalloc = n;

    return a;
}

void array_destroy(struct array *a)
{
    array_deinit(a);
    free(a);
}

int
array_init(struct array *a, uint32_t n, size_t size)
{
    assert(n != 0 && size != 0);

    a->elem = malloc(n * size);
    if (a->elem == NULL)
    {
        return -3;
    }

    a->nelem = 0;
    a->size = size;
    a->nalloc = n;

    return 0;
}

void array_deinit(struct array *a)
{
    assert(a->nelem == 0);

    if (a->elem != NULL)
    {
        free(a->elem);
    }
}

uint32_t
array_idx(struct array *a, void *elem)
{
    uint8_t *p, *q;
    uint32_t off, idx;

    assert(elem >= a->elem);

    p = a->elem;
    q = elem;
    off = (uint32_t)(q - p);

    assert(off % (uint32_t)a->size == 0);

    idx = off / (uint32_t)a->size;

    return idx;
}

void *
array_push(struct array *a)
{
    void *elem, *new;
    size_t size;

    if (a->nelem == a->nalloc)
    {

        /* the array is full; allocate new array */
        size = a->size * a->nalloc;
        new = realloc(a->elem, 2 * size);
        if (new == NULL)
        {
            return NULL;
        }

        a->elem = new;
        a->nalloc *= 2;
    }

    elem = (uint8_t *)a->elem + a->size * a->nelem;
    a->nelem++;

    return elem;
}

void *
array_pop(struct array *a)
{
    void *elem;

    assert(a->nelem != 0);

    a->nelem--;
    elem = (uint8_t *)a->elem + a->size * a->nelem;

    return elem;
}

void *
array_get(struct array *a, uint32_t idx)
{
    void *elem;

    assert(a->nelem != 0);
    assert(idx < a->nelem);

    elem = (uint8_t *)a->elem + (a->size * idx);

    return elem;
}

void *
array_top(struct array *a)
{
    assert(a->nelem != 0);

    return array_get(a, a->nelem - 1);
}

void array_swap(struct array *a, struct array *b)
{
    struct array tmp;

    tmp = *a;
    *a = *b;
    *b = tmp;
}

/*
 * Sort nelem elements of the array in ascending order based on the
 * compare comparator.
 */
void array_sort(struct array *a, array_compare_t compare)
{
    assert(a->nelem != 0);

    qsort(a->elem, a->nelem, a->size, compare);
}

/*
 * Calls the func once for each element in the array as long as func returns
 * success. On failure short-circuits and returns the error status.
 */
int
array_each(struct array *a, array_each_t func, void *data)
{
    uint32_t i, nelem;

    assert(array_n(a) != 0);
    assert(func != NULL);

    for (i = 0, nelem = array_n(a); i < nelem; i++)
    {
        void *elem = array_get(a, i);
        int status;

        status = func(elem, data);
        if (status != 0)
        {
            return status;
        }
    }

    return 0;
}

inline void
array_null(struct array *a)
{
    a->nelem = 0;
    a->elem = NULL;
    a->size = 0;
    a->nalloc = 0;
}

inline void
array_set(struct array *a, void *elem, size_t size, uint32_t nalloc)
{
    a->nelem = 0;
    a->elem = elem;
    a->size = size;
    a->nalloc = nalloc;
}

inline uint32_t
array_n(const struct array *a)
{
    return a->nelem;
}