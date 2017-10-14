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

#ifndef _HOST_CONFIG_H_
#define _HOST_CONFIG_H_

enum config_types {
    CONFIG_TYPE_INT = 1,
    CONFIG_TYPE_STRING = 2,
};

struct config_entry {
    enum config_types type;
    char *name;
    char *section;
    void *storage;
    int max_size;
    union {
        void *p_default;
        int i_default;
    };
};

int host_config_parse(char *filename,
		      struct config_entry *entries,
		      int n_entries);

#endif
