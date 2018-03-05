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
 * The host wrapper of thread manipulations.
 * Threads are not supposed to be destroyed,
 * only canceld all together at the end
 */

#include "host.h"
#include "verbose.h"
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>

struct host_thread {
    struct host_thread *next;
    pthread_t tid;
    bool alive;
};

struct host_thread *threads;

static void host_thread_add(struct host_thread *t)
{
    t->next = threads;
    threads = t;
}

struct thread_wrapper_arg {
    void *(*f)(void *);
    void *a;
};

static void *thread_func_wrapper(void *arg)
{
    struct thread_wrapper_arg *a = arg;
    void *ret;

    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    ret = (*a->f)(a->a);

    free(arg);
    return ret;
}

struct host_thread *host_thread_create(void *(*f)(void *), void *a)
{
    struct host_thread *t;
    pthread_attr_t *no_attr = NULL;
    int rc;
    struct thread_wrapper_arg *wa;

    t = malloc(sizeof(*t));
    if (t == NULL)
        return NULL;
    t->next = NULL;

    wa = malloc(sizeof(*wa));
    if (wa == NULL) {
        free(t);
        return NULL;
    }

    wa->f = f;
    wa->a = a;

    rc = pthread_create(&t->tid, no_attr, thread_func_wrapper, wa);
    if (rc != 0) {
        pr_error("Could not create thread for function %p\n", f);
        free(t);
        free(wa);
        return NULL;
    }

    t->alive = true;
    host_thread_add(t);

    return t;
}

void host_thread_cancel(struct host_thread *t)
{
    void **no_retval = NULL;

    if (t == NULL)
        return;

    pthread_cancel(t->tid);
    pthread_join(t->tid, no_retval);
    t->alive = false;
}

void host_threads_shutdown(void)
{
    struct host_thread *cur, *next;
    void **no_retval = NULL;

    for (cur = threads; cur != NULL; cur = cur->next) {
        if (cur->alive)
            pthread_cancel(cur->tid);
    }

    for (cur = threads; cur != NULL; cur = next) {
        next = cur->next;
        if (cur->alive)
            pthread_join(cur->tid, no_retval);
        free(cur);
    }
    threads = NULL;
}
