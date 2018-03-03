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
 * This file contatin definitions for the implementation of a
 * dynamic array. It allows random read access, automatically
 * increases own size, and maintains data head pointer to allow
 * simply to add data to the end.
 *
 * The object is not transparent to make some simple methods as
 * static inline.
 */

#ifndef _HOST_DARRAY_H_
#define _HOST_DARRAY_H_

#include "host-threads.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

struct darray {
    unsigned elem_size;
    unsigned size;
    unsigned size_increment;
    int head; /* one below new storage position to recognize empty */
    uint8_t *buf;
    host_mutex_t lock;
};

struct darray *darray_new(unsigned size, unsigned elem_size);
void darray_destroy(struct darray *a);
void *darray_push(struct darray *a, void *elm);
void *darray_read(struct darray *a, int idx, void *elm);

static inline void darray_lock(struct darray *a)
{
    int rc;

    rc = host_mutex_lock(&a->lock);
    if (rc != 0)
        abort();
}

static inline void darray_unlock(struct darray *a)
{
    int rc;

    rc = host_mutex_unlock(&a->lock);
    if (rc != 0)
        abort();
}

static inline bool darray_is_empty_locked(struct darray *a)
{
    return a->head < 0;
}

static inline bool darray_is_empty(struct darray *a)
{
    bool ret;

    darray_lock(a);
    ret = darray_is_empty_locked(a);
    darray_unlock(a);
    return ret;
}

static inline unsigned darray_count(struct darray *a)
{
    return a->head + 1;
}

static inline void darray_reset(struct darray *a)
{
    a->head = -1;
}

#endif
