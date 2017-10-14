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
 * Support of different levels of verbose output
 */

#ifndef _PR_VERBOSE_H__
#define _PR_VERBOSE_H__

#include <stdio.h>

extern int verbose;

#ifndef pr_fmt
#define pr_fmt(fmt) fmt
#endif

#define v_print(fmt, level, ...)	do {	\
	if (verbose >= level)			\
            printf(fmt, ## __VA_ARGS__);	\
    } while (0)

#define pr_info(fmt, ...) v_print(pr_fmt(fmt), 0, ## __VA_ARGS__)
#define pr_verbose(fmt, ...) v_print(pr_fmt(fmt), 1, ## __VA_ARGS__)
#define pr_debug(fmt, ...) v_print(pr_fmt(fmt), 2, ## __VA_ARGS__)
#define pr_vdebug(fmt, ...) v_print(pr_fmt(fmt), 3, ## __VA_ARGS__)
#define pr_error(fmt, ...) fprintf(stderr,                             \
				   "ERROR: " pr_fmt(fmt), ##__VA_ARGS__)

#define pr_once(fmt, ...) do {			\
	static bool printed;			\
	if (! printed)				\
            pr_info(fmt, ## __VA_ARGS__);	\
    } while(0);

static inline void verbose_level_inc(void)
{
    verbose++;
}

#endif
