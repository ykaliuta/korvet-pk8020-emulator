/*
 *  (c) 2017 Yauheni Kaliuta <y.kaliuta@gmail.com>
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
 * This file contatin definitions for the implementation
 * of FIFO (queue) on a ringbuffer.
 *
 * The queue object is not transparent to make some
 * simple methods as static inline.
 */

#ifndef _HOST_QUEUE_H_
#define _HOST_QUEUE_H_

#include "host-threads.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

struct queue {
    int elem_size;
    int size;
    int head;
    int tail;
    uint8_t *buf;
    host_mutex_t lock;
    host_cond_t cond;
};

/*
 * @size is a maximum number of elements each of size @elem_size
 * in the queue. In fact, only @size-1 will be available.
 * It is recommended to use size as a power of 2,
 * because the compiler will optimize modulo operator.
 */
struct queue *queue_new(int size, int elem_size);
void queue_destroy(struct queue *q);
void *queue_push(struct queue *q, void *elm);
void *queue_pop(struct queue *q, void *elm);
void queue_wait(struct queue *q);

static inline void queue_lock(struct queue *q)
{
    int rc;

    rc = host_mutex_lock(&q->lock);
    if (rc != 0)
        abort();
}

static inline void queue_unlock(struct queue *q)
{
    int rc;

    rc = host_mutex_unlock(&q->lock);
    if (rc != 0)
        abort();
}

static inline bool queue_is_empty_locked(struct queue *q)
{
    return q->head == q->tail;
}

static inline bool queue_is_empty(struct queue *q)
{
    bool ret;

    queue_lock(q);
    ret = queue_is_empty_locked(q);
    queue_unlock(q);
    return ret;
}

static inline bool queue_is_full_locked(struct queue *q)
{
    int next;

    next = q->tail + 1;
    if (next >= q->size)
        next %= q->size;
    return next == q->head;
}

static inline bool queue_is_full(struct queue *q)
{
    bool ret;

    queue_lock(q);
    ret = queue_is_full_locked(q);
    queue_unlock(q);
    return ret;
}

#endif
