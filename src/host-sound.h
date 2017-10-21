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

#ifndef _HOST_SOUND_H_
#define _HOST_SOUND_H_

#include <stdint.h>

/* SDL-style audio callback, but len is in samples! */
typedef void (*host_sound_cb_t)(uint8_t *buf, unsigned len, void *ctx);

#ifdef SOUND
int host_sound_init(void);
void host_sound_shutdown(void);
int host_sound_start(unsigned _n_samples, unsigned _freq,
                     host_sound_cb_t _cb, void *ctx);
void host_sound_stop(void);
#else
static inline int host_sound_init(void) { return 0; };
static inline void host_sound_shutdown(void) {};
static inline int host_sound_start(unsigned _n_samples, unsigned _freq,
                                   host_sound_cb_t _cb, void *ctx)
{
    return 0;
};
static inline void host_sound_stop(void) {};
#endif


#endif
