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

#define pr_fmt(fmt) __FILE__ ": " fmt

#include "host-config.h"
#include "host.h"
#include "verbose.h"

#include <allegro.h>
#include <stdlib.h>

static void process_int(struct config_entry *e)
{
    int val;

    if (e->max_size < sizeof(int)) {
        pr_error("storage size %d too small\n", e->max_size);
        return;
    };

    val = get_config_int(e->section, e->name, e->i_default);
    *(int *)e->storage = val;

    pr_debug("int value %d\n", val);
}

static void process_string(struct config_entry *e)
{
    const char *str;

    str = get_config_string(e->section, e->name, e->p_default);
    snprintf((char *)e->storage, e->max_size, "%s", str);

    pr_debug("string value: %s\n", (char *)e->storage);
}

static void process_key(struct config_entry *e)
{
    pr_debug("processing entry: %s\n", e->name);
    switch (e->type) {
    case CONFIG_TYPE_STRING:
        process_string(e);
        break;
    case CONFIG_TYPE_INT:
        process_int(e);
        break;
    default:
        pr_error("wrong config type %d\n", e->type);
    }
}

int host_config_parse(char *fn, struct config_entry *arr, int n)
{
    int i;

    set_config_file(fn);

    for (i = 0; i < n; i++)
        process_key(&arr[i]);

    return 0;
}
