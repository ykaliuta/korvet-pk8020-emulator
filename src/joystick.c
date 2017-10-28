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
// (c) 2004 Sergey Erokhin
// 2004-11-04 first release

#include "host.h"
#include <allegro.h>
#include <stdio.h>

int JoystickEnabled;
int JoystickNumber;

static bitmap_t buttons;
static bitmap_t axis;

enum joystick_port_bits {
    J_BUTTON = 4,
    J_UP = 0,
    J_DOWN = 1,
    J_LEFT = 2,
    J_RIGHT = 3,
};

void Joystick_Update(bitmap_t b, bitmap_t a)
{
    buttons = b;
    axis = a;
}

int Joystick_Read(void)
{
   int port = 0;

   if (buttons != 0)
       BIT_SET(port, J_BUTTON);

   if (BIT_TEST(axis, HOST_JOYSTICK_UP))
       BIT_SET(port, J_UP);

   if (BIT_TEST(axis, HOST_JOYSTICK_DOWN))
       BIT_SET(port, J_DOWN);

   if (BIT_TEST(axis, HOST_JOYSTICK_LEFT))
       BIT_SET(port, J_LEFT);

   if (BIT_TEST(axis, HOST_JOYSTICK_RIGHT))
       BIT_SET(port, J_RIGHT);

   return port;
}
