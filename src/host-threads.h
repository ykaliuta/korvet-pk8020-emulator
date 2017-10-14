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
 * This is a wrapper for pthreads library.
 * Will be used as an abstraction layer for multiplatform software.
 *
 * The implementation is not hidden to keep the wrappers inline.
 */

#ifndef _HOST_THREADS_H_
#define _HOST_THREADS_H_

#include <pthread.h>

typedef pthread_mutex_t host_mutex_t;
typedef pthread_cond_t host_cond_t;

#define HOST_MUTEX_INITIALIZER PHREAD_MUTEX_INITIALIZER
#define HOST_COND_INITIALIZER PHREAD_COND_INITIALIZER

static inline int host_mutex_lock(host_mutex_t *m)
{
    return pthread_mutex_lock(m);
}

static inline int host_mutex_unlock(host_mutex_t *m)
{
    return pthread_mutex_unlock(m);
}

static inline int host_mutex_init(host_mutex_t *m)
{
    pthread_mutexattr_t *no_attr = NULL;
    return pthread_mutex_init(m, no_attr);
}

static inline int host_mutex_destroy(host_mutex_t *m)
{
    return pthread_mutex_destroy(m);
}

static inline int host_cond_init(host_cond_t *c)
{
    pthread_condattr_t *no_attr = NULL;
    return pthread_cond_init(c, no_attr);
}

static inline int host_cond_destroy(host_cond_t *c)
{
    return pthread_cond_destroy(c);
}

static inline int host_cond_signal(host_cond_t *c)
{
    return pthread_cond_signal(c);
}

static inline int host_cond_broadcast(host_cond_t *c)
{
    return pthread_cond_broadcast(c);
}

static inline int host_cond_wait(host_cond_t *c, host_mutex_t *m)
{
    return pthread_cond_wait(c, m);
}

static inline int host_cond_timedwait(host_cond_t *c,
				      host_mutex_t *m,
				      const struct timespec *abstime)
{
    return pthread_cond_timedwait(c, m, abstime);
}

#endif
