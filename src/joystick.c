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
#include <stdio.h>

int JoystickEnabled=0;

int JoystickNumber=1;

int JoystickFlag=0;

int JoystickUseFlag=0; // for main.c how many tick show usejoystik indicator

char *flags(int flags) {
   static char f[1024]="";
   sprintf(f,"(%04x :: %s %s %s %s %s %s %s)", 
      flags,
      flags & JOYFLAG_DIGITAL ? "DIGITAL" : "x",
      flags & JOYFLAG_ANALOGUE ? "ANALOGUE" : "x",
      flags & JOYFLAG_CALIB_DIGITAL ? "CALIB_DIGITAL" : "x",
      flags & JOYFLAG_CALIB_ANALOGUE ? "CALIB_ANALOGUE" : "x",
      flags & JOYFLAG_DIGITAL ? "DIGITAL" : "x",
      flags & JOYFLAG_SIGNED ? "SIGNED" : "x",
      flags & JOYFLAG_UNSIGNED ? "UNSIGNED" : "x"
   );
   return f;
}

void show_joystick_info(void) {
  int i,j,k;
  printf("num_joysticks: %d\n", num_joysticks);

   for (i=0;i<num_joysticks;i++) {
      printf("\tjoystick: %2d\n", i);
      printf("\t\tflags: %s\n", flags(joy[i].flags) );
      printf("\t\tnum_buttons: %2d\n", joy[i].num_buttons);
      for(j=0;j<joy[i].num_buttons;j++){
         printf("\t\t\tbutton %2d: bool: %2d : name %s\n", j,joy[i].button[j].b,joy[i].button[j].name);
      }
      
      printf("\t\tnum_sticks: %d\n", joy[i].num_sticks);
      for(j=0;j<joy[i].num_sticks;j++){
         printf("\t\t\tstick %2d: flags: %s : num_axis %2d : name: %2s\n", j,flags(joy[i].stick[j].flags),joy[i].stick[j].num_axis,joy[i].stick[j].name);
         for(k=0;k<joy[i].stick[j].num_axis;k++){
            printf("\t\t\t\t axis: %2d : analog pos %5d: d1: %4d : d2 %4d : name: %s\n", 
               j,
               joy[i].stick[j].axis[k].pos,
               joy[i].stick[j].axis[k].d1,
               joy[i].stick[j].axis[k].d2,
               joy[i].stick[j].axis[k].name
            );
         }             
      }
   }

}

int Init_Joystick(void) {
 
   if (JoystickEnabled) {    
      JoystickFlag=1;

      if (install_joystick(JOY_TYPE_AUTODETECT) != 0) {
   	  JoystickFlag=0;
      }

      /* make sure that we really do have a joystick */
      if (!num_joysticks) {
   	  JoystickFlag=0;
      }

      if (joy[JoystickNumber].num_buttons < 1) {
         printf("Joystick support - selected joystick has no buttons, failed\n");
         JoystickFlag=0;
      }

      if (joy[JoystickNumber].flags & JOYFLAG_CALIBRATE) {
         if (calibrate_joystick(JoystickNumber) == 0) {
   	     JoystickFlag=0;
         }
      }

      printf("Joystick support %d - %s \n",JoystickNumber, JoystickFlag ? "enabled" : "not suppored");

      if (JoystickFlag == 0) {
         show_joystick_info();
         exit(-1);
      }
   } 
   // else {
   //    printf("Joystick support to enabled (-j for enable)\n");
   // }
}

// Deflector Digital Joystick

int Read_Joystick(void) {
   int Port=0;
   int c;
//   JoystickUseFlag=50*0.5;

   if (!JoystickEnabled) return 0;

   if (!JoystickFlag) return 0;//^0xff;

   poll_joystick();     /* we HAVE to do this to read the joystick */

   for (c=0; c<joy[JoystickNumber].num_buttons; c++) {
     if (joy[JoystickNumber].button[c].b) Port=0x10;
   }

   if (joy[JoystickNumber].stick[0].axis[1].d1) Port|=1; // Up
   if (joy[JoystickNumber].stick[0].axis[1].d2) Port|=2; // Down
   if (joy[JoystickNumber].stick[0].axis[0].d1) Port|=4; // Left
   if (joy[JoystickNumber].stick[0].axis[0].d2) Port|=8; // Right

   //strange joystick in KWASTI AC3 (Cadet Academy)
   // if (joy[JoystickNumber].stick[0].axis[1].d1) Port|=2; // Up
   // if (joy[JoystickNumber].stick[0].axis[1].d2) Port|=1; // Down
   // if (joy[JoystickNumber].stick[0].axis[0].d1) Port|=8; // Left
   // if (joy[JoystickNumber].stick[0].axis[0].d2) Port|=4; // Right
   // Port^=0x10;

   return Port^0xff;
}
