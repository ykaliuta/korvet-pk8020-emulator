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
 * This file contatin the implementation of the host events
 * related abstractions. Examples: input events (keyboard,
 * mouse), timer.
 *
 * For now I'm trying to keep all events related code in this one
 * file because if we switch to the event based library, it
 * becomes easier to forward the events.
 */

/* for asprintf */
#define _GNU_SOURCE

#include "host-events.h"
#include "host.h"
#include "queue.h"
#include "verbose.h"

#include <string.h>

#define MAX_EVENTS 512

/* Globals */
static struct queue *event_queue;

void host_event_wait(struct host_event *ev)
{
    queue_wait(event_queue);
    queue_pop(event_queue, ev);
}

bool host_event_pop(struct host_event *ev)
{
    ev = queue_pop(event_queue, ev);
    return ev != NULL;
}

void host_event_push(struct host_event *ev)
{
    while (queue_is_full(event_queue))
        pr_once("Event queue got stuck\n");
    queue_push(event_queue, ev);
}

void host_event_init_common(struct host_event *ev,
				   enum event_type type)
{
    /* May add timestamp later */
    memset(ev, 0, sizeof(*ev));
    ev->type = type;
}

char *host_event_to_str(struct host_event *ev)
{
    char *str;

    asprintf(&str, "%d", ev->type);

    return str;
}

int host_events_init(void)
{
    /* global */
    event_queue = queue_new(MAX_EVENTS, sizeof(struct host_event));
    if (event_queue == NULL)
        return -1;

    return 0;
}

void host_events_shutdown(void)
{
    queue_destroy(event_queue);
    event_queue = NULL;
}
