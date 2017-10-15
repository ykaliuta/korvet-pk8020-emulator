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
 * This file contatin definitions for the host abstraction layer
 */

#ifndef _HOST_HOST_H_
#define _HOST_HOST_H_

typedef unsigned int bitmap_t;

#include "host-config.h"
#include "host-events.h"

#define BIT(nr) (1UL << (nr))
#define BIT_TEST(val, nr) (((val) & BIT(nr)) != 0)
#define BIT_CLEAR(val, nr) ((val) &= ~(BIT(nr)))
#define BIT_SET(val, nr) ((val) |= BIT(nr))
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

int host_init(void);
void host_shutdown(void);

#endif
