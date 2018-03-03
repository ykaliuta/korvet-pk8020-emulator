/*
 *  (c) 2018 Yauheni Kaliuta <y.kaliuta@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 */

/*
 * This file contatin the implementation of dynamic array.
 */

#include "darray.h"
#include <string.h>

struct darray *darray_new(unsigned size, unsigned elem_size)
{
    struct darray *a;
    uint8_t *p;

    a = malloc(sizeof(*a));
    if (a == NULL)
        return NULL;

    p = malloc(size * elem_size);
    if (p == NULL) {
        free(a);
        return NULL;
    }

    a->buf = p;
    a->elem_size = elem_size;
    a->size = size;
    a->size_increment = size;
    a->head = -1;

    host_mutex_init(&a->lock);

    return a;
}

void darray_destroy(struct darray *a)
{
    free(a->buf);
    host_mutex_destroy(&a->lock);
    free(a);
}

static bool darray_is_full_locked(struct darray *a)
{
    return a->head == a->size - 1;
}

/* a->lock should be taken */
static int darray_increase(struct darray *a)
{
    void *p;
    unsigned new_size;

    new_size = a->size + a->size_increment;
    p = realloc(a->buf, new_size);
    if (p == NULL)
        return -1;

    a->buf = p;
    a->size = new_size;
    return 0;
}

void *darray_push(struct darray *a, void *elm)
{
    void *ret = NULL;

    darray_lock(a);
    if (darray_is_full_locked(a))
        if (darray_increase(a) != 0)
            goto out;

    a->head++;

    memcpy(a->buf + a->head * a->elem_size,
           elm,
           a->elem_size);
    ret = elm;
out:
    darray_unlock(a);
    return ret;
}

void *darray_read(struct darray *a, int idx, void *elm)
{
    /* the check is a bit racy */
    if (idx < 0 || idx > a->head)
        return NULL;

    darray_lock(a);

    if (darray_is_empty_locked(a)) {
        darray_unlock(a);
        return NULL;
    }

    memcpy(elm,
           a->buf + idx * a->elem_size,
           a->elem_size);

    darray_unlock(a);
    return elm;
}
