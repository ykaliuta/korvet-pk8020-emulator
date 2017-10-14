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

struct queue *queue_new(int size, int elem_size)
{
    struct queue *q;
    uint8_t *p;

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

    host_mutex_init(&q->lock);
    host_cond_init(&q->cond);
	
    return q;
}

void queue_destroy(struct queue *q)
{
    free(q->buf);
    host_cond_destroy(&q->cond);
    host_mutex_destroy(&q->lock);
    free(q);
}

void *queue_push(struct queue *q, void *elm)
{
    int next;
    void *ret = NULL;

    queue_lock(q);
    if (queue_is_full_locked(q))
        goto out;
	
    next = q->tail + 1;
    if (next >= q->size)
        next %= q->size;
    memcpy(q->buf + q->tail * q->elem_size,
           elm,
           q->elem_size);
    q->tail = next;
    ret = elm;
    host_cond_broadcast(&q->cond);
out:
    queue_unlock(q);
    return ret;
}
	    
void *queue_pop(struct queue *q, void *elm)
{
    int next;
    void *ret = NULL;
    void *p;

    queue_lock(q);
    if (queue_is_empty_locked(q))
        goto out;
	
    next = q->head + 1;
    if (next >= q->size)
        next %= q->size;
    p = q->buf + q->head * q->elem_size;
    memcpy(elm, p, q->elem_size);
    q->head = next;
    ret = elm;
out:
    queue_unlock(q);
    return ret;
}

void queue_wait(struct queue *q)
{
    queue_lock(q);
    while (queue_is_empty_locked(q))
        host_cond_wait(&q->cond, &q->lock);
    queue_unlock(q);
}
