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
#include "libconfini/confini.h"
#include "verbose.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define INI_FORMAT INI_DEFAULT_FORMAT

struct listener_arg {
    struct config_entry *arr;
    int n;
    /* state data */
    /* current session */
    char *section;
    /* array of keys, found in the config to process defaults */
    bool *found;
};

static struct config_entry *config_entry_find(struct config_entry *arr,
					      int n,
					      char *sec,
					      char *name,
					      int *i)
{
    struct config_entry *e;

    while (--n >= 0) {
        e = &arr[n];
        if (ini_string_match_si(e->name, name, INI_FORMAT) &&
            ini_string_match_si(e->section, sec, INI_FORMAT)) {
            *i = n;
            return e;
        }
    }
    return NULL;
}

static void process_int(struct config_entry *e, int value)
{
    if (e->max_size < sizeof(int)) {
        pr_error("storage size %d too small\n", e->max_size);
        return;
    };

    *(int *)e->storage = value;
    pr_debug("int value %d\n", value);
}

static void process_string(struct config_entry *e, char *value)
{
    snprintf((char *)e->storage, e->max_size, "%s", value);
    pr_debug("string value: %s\n", (char *)e->storage);
}

static void process_key(IniDispatch *d,
			struct config_entry *arr,
			int n,
			char *section,
			bool *found_arr)
{
    struct config_entry *e;
    int i;
    int i_val;
    char *s_val;

    e = config_entry_find(arr, n, section, d->data, &i);
    if (e == NULL)
        return;
    pr_debug("Processing entry %s, idx %d\n", e->name, i);
    found_arr[i] = true;

    switch (e->type) {
    case CONFIG_TYPE_INT:
        i_val = ini_get_int(d->value);
        process_int(e, i_val);
        break;
    case CONFIG_TYPE_STRING:
        s_val = d->value;
        process_string(e, s_val);
        break;
    default:
        pr_error("Wrong entry type: %d\n", e->type);
    }
}

static int ini_listener(IniDispatch *d, void *arg)
{
    struct listener_arg *a = arg;

    if (d->type != INI_SECTION &&
        d->type != INI_KEY)
        return 0;

    switch (d->type) {
    case INI_SECTION:
        free(a->section);
        a->section = strdup(d->data);
        pr_debug("Section changed to %s\n", a->section);
        break;
    case INI_KEY:
        process_key(d, a->arr, a->n, a->section, a->found);
        break;
    }
    return 0;
}

static void process_defaults(struct listener_arg *arg)
{
    int n = arg->n;
    bool *found = arg->found;
    struct config_entry *arr = arg->arr;
    struct config_entry *e;
    int i_def;
    char *s_def;

    while (--n) {
        if (found[n])
            continue;

        e = &arr[n];
        pr_debug("Using default value for %s\n", e->name);

        switch (e->type) {
        case CONFIG_TYPE_INT:
            i_def = e->i_default;
            process_int(e, i_def);
            break;
        case CONFIG_TYPE_STRING:
            s_def = e->p_default;
            process_string(e, s_def);
            break;
        default:
            pr_error("Wrong entry type: %d\n", e->type);
        }
    }
}

int host_config_parse(char *fn, struct config_entry *arr, int n)
{
    int rc;
    struct listener_arg arg = {
        .arr = arr,
        .n = n,
    };
    bool *found;

    found = calloc(n, sizeof(*found));
    if (found == NULL) {
        pr_error("No memory\n");
        return -1;
    }

    arg.found = found;

    rc = load_ini_path(fn, INI_FORMAT,
                       NULL, ini_listener, &arg);
    if (rc != 0)
        pr_debug("config %s parsing error\n", fn);

    process_defaults(&arg);

    free(arg.section);
    free(found);
    return rc;
}
