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

#ifndef _HOST_GRAPHICS_H_
#define _HOST_GRAPHICS_H_

struct host_g_image;

void host_g_init(void);
struct host_g_image *host_g_image_new(int x, int y);
void host_g_image_destroy(struct host_g_image *img);
uint8_t *host_g_image_get_line_ptr(struct host_g_image *img, int y);
void host_g_image_finish(struct host_g_image *img);
void host_g_image_to_screen(struct host_g_image *img,
                            int x_src, int y_src,
                            int x_dst, int y_dst,
                            int w, int h);
void host_g_set_mode(int x, int y);

#endif
