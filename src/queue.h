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
    unsigned elem_size;
    unsigned size;
    unsigned head;
    unsigned tail;
    uint8_t *buf;
    host_mutex_t push_lock;
    host_mutex_t pop_lock;
    host_cond_t cond;
    uint64_t pushes;
    uint64_t pops;
    uint64_t max_count;
};

static inline unsigned _queue_inc_ptr(struct queue *q, unsigned ptr)
{
    unsigned next;

    next = ptr + 1;
    if (next >= q->size)
        next %= q->size;
    return next;
}

/*
 * @size is a maximum number of elements each of size @elem_size
 * in the queue.
 * It is recommended to use (@size + 1) as a power of 2,
 * because the compiler will optimize modulo operator.
 */
struct queue *queue_new(unsigned size, unsigned elem_size);
void queue_destroy(struct queue *q);
void queue_wait(struct queue *q);
void *queue_push_locked(struct queue *q, void *elm);
void *queue_pop_locked(struct queue *q, void *elm);

static inline void _queue_lock(host_mutex_t *lock)
{
    int rc;

    rc = host_mutex_lock(lock);
    if (rc != 0)
        abort();
}

static inline void _queue_unlock(host_mutex_t *lock)
{
    int rc;

    rc = host_mutex_unlock(lock);
    if (rc != 0)
        abort();
}

static inline void queue_lock_push(struct queue *q)
{
    _queue_lock(&q->push_lock);
}

static inline void queue_unlock_push(struct queue *q)
{
    _queue_unlock(&q->push_lock);
}

static inline void queue_lock_pop(struct queue *q)
{
    _queue_lock(&q->pop_lock);
}

static inline void queue_unlock_pop(struct queue *q)
{
    _queue_unlock(&q->pop_lock);
}

static inline void *queue_push(struct queue *q, void *elm)
{
    void *ret;

    queue_lock_push(q);
    ret = queue_push_locked(q, elm);
    queue_unlock_push(q);

    return ret;
}

static inline void *queue_pop(struct queue *q, void *elm)
{
    void *ret;

    queue_lock_pop(q);
    ret = queue_pop_locked(q, elm);
    queue_unlock_pop(q);

    return ret;
}

static inline bool queue_is_empty_locked(struct queue *q)
{
    return q->head == q->tail;
}

static inline bool queue_is_empty(struct queue *q)
{
    bool ret;

    queue_lock_pop(q);
    ret = queue_is_empty_locked(q);
    queue_unlock_pop(q);
    return ret;
}

static inline bool queue_is_full_locked(struct queue *q)
{
    return _queue_inc_ptr(q, q->tail) == q->head;
}

static inline bool queue_is_full(struct queue *q)
{
    bool ret;

    queue_lock_push(q);
    ret = queue_is_full_locked(q);
    queue_unlock_push(q);
    return ret;
}

static inline unsigned queue_count(struct queue *q)
{
    return (q->tail + q->size - q->head) % q->size;
}

static inline void queue_reset(struct queue *q)
{
    q->tail = q->head = 0;
}

#endif
