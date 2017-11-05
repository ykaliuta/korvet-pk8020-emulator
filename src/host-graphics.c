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

#include <allegro.h>

struct host_g_image {
    BITMAP *bmp;
};

#define DEPTH_8bit 8

struct host_g_image *host_g_image_new(int x, int y)
{
    struct host_g_image *img;
    BITMAP *bmp;

    img = malloc(sizeof(*img));
    if (img == NULL)
        return NULL;

    bmp = create_bitmap_ex(DEPTH_8bit, x, y);
    if (bmp == NULL) {
        free(img);
        return NULL;
    }

    img->bmp = bmp;
    return img;
}

void host_g_image_destroy(struct host_g_image *img)
{
    if (img == NULL)
        return;
    destroy_bitmap(img->bmp);
    free(img);
}

uint8_t *host_g_image_get_line_ptr(struct host_g_image *img,
                                          int y)
{
    return (uint8_t *)bmp_write_line(img->bmp, y);
}

void host_g_image_finish(struct host_g_image *img)
{
    bmp_unwrite_line(img->bmp);
}

void host_g_image_to_screen(struct host_g_image *img,
                                   int x_src, int y_src,
                                   int x_dst, int y_dst,
                                   int w, int h)
{
    blit(img->bmp, screen, x_src, y_src, x_dst, y_dst, w, h);
}

void host_g_set_mode(int x, int y)
{
    set_gfx_mode(GFX_AUTODETECT_WINDOWED, x, y, 0, 0);
}

void host_g_init(void)
{
    set_color_depth(DEPTH_8bit);
}
