/*
 * AUTHOR: Sergey Erokhin                 esl@pisem.net,pk8020@gmail.com
 * &Korvet Team                                              2000...2005
 * ETALON Korvet Emulator                         http://pk8020.narod.ru
 * ---------------------------------------------------------------------
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
 *
 */
#include "host.h"
#include "korvet.h"
#include <allegro.h>

int MouseType = 1; //0 - disabled, 1 - MSMouse, 2-MouseSystem

/* static int k_mousex = 0; */
/* static int k_mousey = 0; */
/* static int k_mouseb = 0; */

/*
               1st byte        2nd byte         3rd byte
          ┌─┬─┬─┬─┬─┬─┬─┬─┐┌─┬─┬─┬─┬─┬─┬─┬─┐┌─┬─┬─┬─┬─┬─┬─┬─┐
          │-│1│?│?│Y│Y│X│X││-│0│X│X│X│X│X│X││-│0│Y│Y│Y│Y│Y│Y│
          └─┴─┴─┴─┴─┴─┴─┴─┘└─┴─┴─┴─┴─┴─┴─┴─┘└─┴─┴─┴─┴─┴─┴─┴─┘
               │ │ └┬┘ └┬┘      └────┬────┘      └────┬────┘
               │ │  │   │            │                │
               │ │  └───┼────────────┼────────┐       │
               │ │      └────┐       │        │       │
               │ │          ┌┴┐ ┌────┴────┐  ┌┴┐ ┌────┴────┐
               │ │         ┌─┬─┬─┬─┬─┬─┬─┬─┐┌─┬─┬─┬─┬─┬─┬─┬─┐
               │ │         │ │ │ │ │ │ │ │ ││ │ │ │ │ │ │ │ │
 Left Button ──┘ │         └─┴─┴─┴─┴─┴─┴─┴─┘└─┴─┴─┴─┴─┴─┴─┴─┘
Right Button ────┘            X increment      Y increment
*/

enum ms_buttons {
    MS_LEFT = 5,
    MS_RIGHT = 4,
};

static void ChkMouse_Microsoft(bitmap_t buttons, int dx, int dy)
{
    uint8_t b1 = 0x40; /* see above, bit 6 always set */
    uint8_t b2;
    uint8_t b3;

    if (BIT_TEST(buttons, HOST_MOUSE_LEFT))
        BIT_SET(b1, MS_LEFT);

    if (BIT_TEST(buttons, HOST_MOUSE_RIGHT))
        BIT_SET(b1, MS_RIGHT);

    b1 |= ((dy & 0xc0) >> 4) | ((dx & 0xc0) >> 6);
    b2 = dx & 0x3f;
    b3 = dy & 0x3f;

    AddSerialQueue(b1);
    AddSerialQueue(b2);
    AddSerialQueue(b3);
/*
// Korvet mouse? (Paralel)
if (mickeyx) {k_mousex+=(mickeyx>0)?1:-1;}
if (mickeyy) {k_mousey+=(mickeyy>0)?1:-1;}

btn  =(mouse_b & 2)?0x10:0;
btn |=(mouse_b & 1)?0x20:0;
k_mouseb = (k_mouseb != btn)?btn:k_mouseb;
*/
}

// specila verson for Kwasi
// use 5 byte mouse sequence (Mouse System)

/*
      Mouse systems mouse


        Serial data parameters:

1200bps, 8 databits, 1 stop-bit


        The data is sent in 5 byte packets in following format:

        D7      D6      D5      D4      D3      D2      D1      D0

1.      1       0       0       0       0       LB      CB      RB
2.      X7      X6      X5      X4      X3      X2      X1      X0
3.      Y7      Y6      Y5      Y4      Y3      Y4      Y1      Y0
4.      X7'     X6'     X5'     X4'     X3'     X2'     X1'     X0'
5.      Y7'     Y6'     Y5'     Y4'     Y3'     Y4'     Y1'     Y0'

LB is left button state (0=pressed, 1=released)
CB is center button state (0=pressed, 1=released)
RB is right button state (0=pressed, 1=released)
X7-X0 movement in X direction since last packet in signed byte
      format (-128..+127), positive direction right
Y7-Y0 movement in Y direction since last packet in signed byte
      format (-128..+127), positive direction up
X7'-X0' movement in X direction since sending of X7-X0 packet in signed byte
      format (-128..+127), positive direction right
Y7'-Y0' movement in Y direction since sending of Y7-Y0 in signed byte
      format (-128..+127), positive direction up

The last two bytes in the packet (bytes 4 and 5) contains information
about movement data changes which have occured after data butes 2 and 3
have been sent.

*/

enum mss_buttons {
    MSS_LEFT = 2,
    MSS_RIGHT = 0,
    MSS_MIDDLE = 1,
};

static void ChkMouse_MouseSystem(bitmap_t buttons, int dx, int dy)
{
    uint8_t b1 = 0x87; /* bit 7 always set, first three 1 if not pressed */
    uint8_t b2;
    uint8_t b3;

    if (BIT_TEST(buttons, HOST_MOUSE_LEFT))
        BIT_CLEAR(b1, MSS_LEFT);

    if (BIT_TEST(buttons, HOST_MOUSE_RIGHT))
        BIT_CLEAR(b1, MSS_RIGHT);

    if (BIT_TEST(buttons, HOST_MOUSE_MIDDLE))
        BIT_CLEAR(b1, MSS_MIDDLE);

    b2 = dx;
    b3 = (256 - dy) & 0xff;

    AddSerialQueue(b1);
    AddSerialQueue(b2);
    AddSerialQueue(b3);
    AddSerialQueue(0);
    AddSerialQueue(0);
}

/*
TODO: додбавить автодетект мыши
если проинитились таймер и ви51

  ви53
  *(char *) 0xfb01 = 0x68;
  *(char *) 0xfb01 = 0;

  вв51
  *(char *) 0xfb11 = 0x35;
*/

void MouseUpdate(bitmap_t buttons, int dx, int dy)
{
    if (MouseType == 1) ChkMouse_Microsoft(buttons, dx, dy);
    if (MouseType == 2) ChkMouse_MouseSystem(buttons, dx, dy);
}
