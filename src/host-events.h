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

#ifndef _HOST_EVENTS_H_
#define _HOST_EVENTS_H_

#include "host.h"
#include <stdbool.h>

enum event_type {
    HOST_NOTHING = 0,
    HOST_KEY_DOWN = 1,
    HOST_KEY_UP,
    HOST_MOUSE,
};

#define HOST_KEY_MAX (HOST_KEY_UP + 1)

/* bit positions and constants, so keep 0 free */
enum key_mods {
    HOST_MOD_NONE = 0,
    HOST_MOD_SHIFT = 1,
    HOST_MOD_ALT = 2,
    HOST_MOD_CTRL = 3,
    HOST_MOD_MAX,
};

enum mouse_buttons {
    HOST_MOUSE_LEFT = 0,
    HOST_MOUSE_RIGHT,
    HOST_MOUSE_MIDDLE,
};

struct host_event {
    enum event_type type;
    union {
        struct key_event {
            bitmap_t mods;
            int code;
        } key;
        struct mouse_event {
            bitmap_t buttons;
            int dx;
            int dy;
        } mouse;
    };
};

int host_events_init(void);
void host_events_shutdown(void);
void host_event_wait(struct host_event *ev);
bool host_event_pop(struct host_event *ev);
void host_event_push(struct host_event *ev);
char *host_event_to_str(struct host_event *ev);
void host_events_pause(void);
void host_events_resume(void);
void host_events_flush(void);
int host_event_kbd_to_ascii(struct host_event *ev);
void host_event_kbd_simulate(int code);

#endif
