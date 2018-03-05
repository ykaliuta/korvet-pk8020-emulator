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
 * This file contatin the implementation
 * of FIFO (queue) on a ringbuffer.
 */

#include "queue.h"
#include <string.h>

struct queue *queue_new(unsigned _size, unsigned elem_size)
{
    struct queue *q;
    uint8_t *p;
    /*
     * cannot fill the last element, since then head and tail
     * pointers will be the same again like in the empty case
     */
    unsigned size = _size + 1; /* one dummy element for the full case */

    q = malloc(sizeof(*q));
    if (q == NULL)
        return NULL;

    p = malloc(size * elem_size);
    if (p == NULL) {
        free(q);
        return NULL;
    }

    q->buf = p;
    q->elem_size = elem_size;
    q->size = size;
    q->head = 0;
    q->tail = 0;

    host_mutex_init(&q->push_lock);
    host_mutex_init(&q->pop_lock);
    host_cond_init(&q->cond);

    return q;
}

void queue_destroy(struct queue *q)
{
    free(q->buf);
    host_cond_destroy(&q->cond);
    host_mutex_destroy(&q->pop_lock);
    host_mutex_destroy(&q->push_lock);
    free(q);
}

void *queue_push_locked(struct queue *q, void *elm)
{
    void *ret = NULL;

    if (queue_is_full_locked(q))
        return NULL;

    memcpy(q->buf + q->tail * q->elem_size,
           elm,
           q->elem_size);
    q->tail = _queue_inc_ptr(q, q->tail);
    ret = elm;
    host_cond_broadcast(&q->cond);

    return ret;
}

void *queue_pop_locked(struct queue *q, void *elm)
{
    void *ret = NULL;
    void *p;

    if (queue_is_empty_locked(q))
        return NULL;

    p = q->buf + q->head * q->elem_size;
    memcpy(elm, p, q->elem_size);
    q->head = _queue_inc_ptr(q, q->head);
    ret = elm;

    return ret;
}

void queue_wait(struct queue *q)
{
    queue_lock_pop(q);
    while (queue_is_empty_locked(q))
        host_cond_wait(&q->cond, &q->pop_lock);
    queue_unlock_pop(q);
}
