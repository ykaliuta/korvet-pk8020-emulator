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

#include "host.h"
#include <allegro.h>
#include <stdbool.h>

#define MAX_TIMERS 2

struct host_timer {
    bool busy;
    void (*cb)(void *);
    void *ctx;
};

static struct host_timer timers[MAX_TIMERS];

static struct host_timer *host_timer_find_free(void)
{
    int i;

    for (i = 0; i < ARRAY_SIZE(timers); i++)
        if (!timers[i].busy)
            return &timers[i];
    return NULL;
}

struct host_timer *host_timer_start_bps(int bps,
                                        void (*cb)(void *),
                                        void *ctx)
{
    struct host_timer *timer;

    timer = host_timer_find_free();
    if (timer == NULL)
        return NULL;

    timer->busy = true;
    timer->cb = cb;
    timer->ctx = ctx;

    install_param_int_ex(cb, ctx, BPS_TO_TIMER(bps));

    return timer;
}

void host_timer_stop(struct host_timer *timer)
{
    remove_param_int(timer->cb, timer->ctx);
    timer->busy = false;
}

