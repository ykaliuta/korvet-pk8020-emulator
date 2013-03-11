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
#include <allegro.h>

int JoystickFlag=0;

int JoystickUseFlag=0; // for main.c how many tick show usejoystik indicator

int Init_Joystick(void) {

   JoystickFlag=1;

   if (install_joystick(JOY_TYPE_AUTODETECT) != 0) {
	JoystickFlag=0;
   }

   /* make sure that we really do have a joystick */
   if (!num_joysticks) {
	JoystickFlag=0;
   }

   if (joy[0].flags & JOYFLAG_CALIBRATE) {
      if (calibrate_joystick(0) == 0) {
	JoystickFlag=0;
      }
   }
}


// Deflector Digital Joystick

int Read_Joystick(void) {
   int Port=0;
   int c;
   JoystickUseFlag=50*0.5;

   if (!JoystickFlag) return 0^0xff;

   poll_joystick();     /* we HAVE to do this to read the joystick */

   for (c=0; c<joy[0].num_buttons; c++) {
     if (joy[0].button[c].b) Port=0x10;
   }

   if (joy[0].stick[0].axis[1].d1) Port|=1; // Up
   if (joy[0].stick[0].axis[1].d2) Port|=2; // Down
   if (joy[0].stick[0].axis[0].d1) Port|=4; // Left
   if (joy[0].stick[0].axis[0].d2) Port|=8; // Right
   return Port^0xff;
}
