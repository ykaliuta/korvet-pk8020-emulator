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

#include <allegro.h>
#include <string.h>

#define MAX_EVENTS (512 - 1)

/* Globals */
static struct queue *event_queue;
static bitmap_t current_mods;
static bool paused;
static struct host_thread *joy_thread;

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
    char *str;

    if (paused)
        return;

    while (queue_is_full(event_queue))
        pr_once("Event queue got stuck\n");

    str = host_event_to_str(ev);
    pr_vdebug("pushing event: %s\n", str);
    free(str);

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
    const char *keyname;

    switch (ev->type) {
    case HOST_KEY_UP:
    case HOST_KEY_DOWN:
        keyname = scancode_to_name(ev->key.code);
        asprintf(&str, "%d, mods 0x%08x, key %s",
                 ev->type, ev->key.mods, keyname);
        break;
    case HOST_MOUSE:
        asprintf(&str, "Mouse: %x, %d, %d\n",
                 ev->mouse.buttons, ev->mouse.dx, ev->mouse.dy);
        break;
    default:
        asprintf(&str, "%d", ev->type);
    }

    return str;
}

static void host_event_kbd_init(struct host_event *ev,
				enum event_type type,
				bitmap_t mods,
				int scancode)
{
    host_event_init_common(ev, type);

    ev->key.mods = mods;
    ev->key.code = scancode;
}

static enum key_mods host_scancode_to_mod(int scancode)
{
    switch(scancode) {
    case KEY_ALT:
    case KEY_ALTGR:
        return HOST_MOD_ALT;
    case KEY_LSHIFT:
    case KEY_RSHIFT:
        return HOST_MOD_SHIFT;
    case KEY_LCONTROL:
    case KEY_RCONTROL:
        return HOST_MOD_CTRL;
    }
    return 0;
}

static void key_callback(int event)
{
    bool is_down = !BIT_TEST(event, 7);
    int scancode = BIT_CLEAR(event, 7);
    bool is_mod;
    enum key_mods mod;
    struct host_event ev;
    enum event_type ev_type;

    pr_vdebug("key callback for scancode %d\n", scancode);

    mod = host_scancode_to_mod(scancode);
    is_mod = (mod != HOST_MOD_NONE);

    if (is_mod) {
        if (is_down)
            BIT_SET(current_mods, mod);
        else
            BIT_CLEAR(current_mods, mod);
        return;
    }

    ev_type = is_down ? HOST_KEY_DOWN : HOST_KEY_UP;

    host_event_kbd_init(&ev, ev_type, current_mods, scancode);
    host_event_push(&ev);
}

int host_event_kbd_to_ascii(struct host_event *ev)
{
    int a = 0;
    /*
     * For some reason does work mostly for the letters,
     * so use own table for the symbols, needed in the debugger
     */
    int table[KEY_MAX] = {
        [KEY_COMMA] = ',',
        [KEY_STOP] = '.',
        [KEY_COLON] = ';',
    };

    a = scancode_to_ascii(ev->key.code);
    if (a != 0)
        return a;

    return table[ev->key.code];
}

/*
 * It takes the allregro4 readkey() type scancode,
 * so we will shift it to get the real scancode
 */
void host_event_kbd_simulate(int code)
{
    struct host_event ev;
    int scancode = (code >> 8) & 0xFF;

    host_event_kbd_init(&ev, HOST_KEY_DOWN, 0, scancode);
    host_event_push(&ev);
    host_event_kbd_init(&ev, HOST_KEY_UP, 0, scancode);
    host_event_push(&ev);
}

static void host_event_mouse_init(struct host_event *ev,
                                  enum event_type type,
                                  bitmap_t buttons,
                                  int dx, int dy)
{
    host_event_init_common(ev, type);

    ev->mouse.buttons = buttons;
    ev->mouse.dx = dx;
    ev->mouse.dy = dy;
}

enum allegro_buttons {
    A_LEFT = 0,
    A_RIGHT = 1,
    A_MIDDLE = 2,
};

static void m_callback(int flags)
{
    struct host_event ev;
    bitmap_t buttons = 0;
    int dx;
    int dy;
    int b_state = mouse_b;

    get_mouse_mickeys(&dx, &dy);

    if (b_state & BIT(A_LEFT))
        BIT_SET(buttons, HOST_MOUSE_LEFT);

    if (b_state & BIT(A_RIGHT))
        BIT_SET(buttons, HOST_MOUSE_RIGHT);

    if (b_state & BIT(A_MIDDLE))
        BIT_SET(buttons, HOST_MOUSE_MIDDLE);

    host_event_mouse_init(&ev, HOST_MOUSE, buttons, dx, dy);
    host_event_push(&ev);
}

void host_events_pause(void)
{
    paused = true;
}

void host_events_resume(void)
{
    paused = false;
}

void host_events_flush(void)
{
    struct host_event ev;

    while (host_event_pop(&ev))
        ;
}

static void host_event_joystick_init(struct host_event *ev,
                                     bitmap_t buttons,
                                     bitmap_t axis)
{
    host_event_init_common(ev, HOST_JOYSTICK);

    ev->joystick.buttons = buttons;
    ev->joystick.axis = axis;
}

static void *host_joystick_thread(void *arg)
{
    useconds_t time_to_sleep_us = 20 * 1000; /* 20 ms, 50Hz */
    struct host_event ev;
    bitmap_t old_buttons = 0;
    bitmap_t old_axis = 0;
    bitmap_t buttons;
    bitmap_t axis;
    int num_buttons;
    intptr_t n = (intptr_t)arg;
    int i;

    num_buttons = joy[n].num_buttons;
    if (num_buttons > (sizeof(buttons) * 8))
        num_buttons = sizeof(buttons) * 8;


    for (;;) {
        buttons = 0;
        axis = 0;

        poll_joystick();

        for (i = 0; i < num_buttons; i++)
            if (joy[n].button[i].b)
                BIT_SET(buttons, i);

        if (joy[n].stick[0].axis[1].d1)
            BIT_SET(axis, HOST_JOYSTICK_UP);

        if (joy[n].stick[0].axis[1].d2)
            BIT_SET(axis, HOST_JOYSTICK_DOWN);

        if (joy[n].stick[0].axis[0].d1)
            BIT_SET(axis, HOST_JOYSTICK_LEFT);

        if (joy[n].stick[0].axis[0].d2)
            BIT_SET(axis, HOST_JOYSTICK_RIGHT);

        if (buttons != old_buttons || axis != old_axis) {
            host_event_joystick_init(&ev, buttons, axis);
            host_event_push(&ev);

            old_buttons = buttons;
            old_axis = axis;
        }

        usleep(time_to_sleep_us);
    }
    return NULL;
}

/* take flags and show info almost as is */
static char *flags(int flags)
{
    static char f[1024]="";
    sprintf(f,"(%04x :: %s %s %s %s %s %s %s)",
            flags,
            flags & JOYFLAG_DIGITAL ? "DIGITAL" : "x",
            flags & JOYFLAG_ANALOGUE ? "ANALOGUE" : "x",
            flags & JOYFLAG_CALIB_DIGITAL ? "CALIB_DIGITAL" : "x",
            flags & JOYFLAG_CALIB_ANALOGUE ? "CALIB_ANALOGUE" : "x",
            flags & JOYFLAG_DIGITAL ? "DIGITAL" : "x",
            flags & JOYFLAG_SIGNED ? "SIGNED" : "x",
            flags & JOYFLAG_UNSIGNED ? "UNSIGNED" : "x"
        );
    return f;
}

static void host_joystick_show_info(void)
{
    int i,j,k;
    pr_info("num_joysticks: %d\n", num_joysticks);

    for (i=0;i<num_joysticks;i++) {
        pr_info("\tjoystick: %2d\n", i);
        pr_info("\t\tflags: %s\n", flags(joy[i].flags) );
        pr_info("\t\tnum_buttons: %2d\n", joy[i].num_buttons);
        for(j=0;j<joy[i].num_buttons;j++){
            pr_info("\t\t\tbutton %2d: bool: %2d : name %s\n", j,joy[i].button[j].b,joy[i].button[j].name);
        }

        pr_info("\t\tnum_sticks: %d\n", joy[i].num_sticks);
        for(j=0;j<joy[i].num_sticks;j++){
            pr_info("\t\t\tstick %2d: flags: %s : num_axis %2d : name: %2s\n", j,flags(joy[i].stick[j].flags),joy[i].stick[j].num_axis,joy[i].stick[j].name);
            for(k=0;k<joy[i].stick[j].num_axis;k++){
                pr_info("\t\t\t\t axis: %2d : analog pos %5d: d1: %4d : d2 %4d : name: %s\n",
                       j,
                       joy[i].stick[j].axis[k].pos,
                       joy[i].stick[j].axis[k].d1,
                       joy[i].stick[j].axis[k].d2,
                       joy[i].stick[j].axis[k].name
                    );
            }
        }
    }

}

int host_joystick_init(int n)
{
    struct host_thread *t;

    if (install_joystick(JOY_TYPE_AUTODETECT) != 0) {
        pr_error("Could not init joystick: %s\n", allegro_error);
        goto err;
    }

    if (n >= num_joysticks) {
        pr_error("Cannot use joystick %d, max: %d\n", (int)n, num_joysticks);
        goto err;
    }

    if (joy[n].num_buttons < 1) {
        pr_error("Selected joystick has no buttons\n");
        goto err;
    }

    if (joy[n].flags & JOYFLAG_CALIBRATE) {
        if (calibrate_joystick(n) == 0) {
            pr_error("Could not calibrate joystick: %s\n", allegro_error);
            goto err;
        }
    }

    t = host_thread_create(host_joystick_thread, (void *)(intptr_t)n);
    if (t == NULL)
        return -1;

    joy_thread = t;
    return 0;

err:
    host_joystick_show_info();
    return -1;
}

void host_joystick_shutdown(void)
{
    host_thread_cancel(joy_thread);
    joy_thread = NULL;
}

int host_events_init(void)
{
    /* global */
    event_queue = queue_new(MAX_EVENTS, sizeof(struct host_event));
    if (event_queue == NULL)
        return -1;

    keyboard_lowlevel_callback = key_callback;
    mouse_callback = m_callback;

    return 0;
}

void host_events_shutdown(void)
{
    mouse_callback = NULL;
    keyboard_lowlevel_callback = NULL;
    queue_destroy(event_queue);
    event_queue = NULL;
}
