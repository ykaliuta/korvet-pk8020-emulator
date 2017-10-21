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
 * The file contains sound abstraction on top on the library
 * (allegro, SDL ...)
 */

#include "host.h"
#include "verbose.h"
#include <allegro.h>
#include <unistd.h>

#define BITS_8 8
#define MONO 0
#define VOLUME_255 255
#define PAN_128 128

static AUDIOSTREAM *stream;
static unsigned n_samples;
static unsigned freq;
static host_sound_cb_t cb;
static void *cb_ctx;
static struct host_thread *thread;

static void *host_sound_thread(void *arg)
{
    void *buf;
    /* 1 sec == 1000 * 1000 usec. Better multiply before division */
    useconds_t time_to_sleep = n_samples * 1000 * 1000 / freq;

    for (;;) {
        buf = get_audio_stream_buffer(stream);
        while (buf == NULL) {
            usleep(time_to_sleep);
            buf = get_audio_stream_buffer(stream);
        }

        cb(buf, n_samples, cb_ctx);
        free_audio_stream_buffer(stream);
    }
    return NULL;
}

int host_sound_start(unsigned _n_samples, unsigned _freq,
                     host_sound_cb_t _cb, void *ctx)
{
    struct host_thread *t;

    stream = play_audio_stream(_n_samples,
                               BITS_8,
                               MONO,
                               _freq,
                               VOLUME_255,
                               PAN_128);
    if (!stream) {
        pr_error("Error creating audio stream: %s\n", allegro_error);
        goto err;
    }

    /* Needed before starting the thread */
    n_samples = _n_samples;
    freq = _freq;
    cb = _cb;
    cb_ctx = ctx;

    t = host_thread_create(host_sound_thread, NULL);
    if (t == NULL)
        goto err_stream;

    thread = t;

    return 0;

err_stream:
    stop_audio_stream(stream);
    stream = NULL;
err:
    return -1;
}

void host_sound_stop(void)
{
    if (stream == NULL)
        return;

    host_thread_cancel(thread);
    stop_audio_stream(stream);
    stream = NULL;
}

int host_sound_init(void)
{
    if (install_sound(DIGI_AUTODETECT, MIDI_NONE, NULL) != 0) {
        pr_error("Error initialising sound system\n%s\n",
                 allegro_error);
        return -1;
    }
    pr_verbose("Audio driver: %s\n", digi_driver->name);
    return 0;
}

void host_sound_shutdown(void)
{
    host_sound_stop();
    remove_sound();
}
