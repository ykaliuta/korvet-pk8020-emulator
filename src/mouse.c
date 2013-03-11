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
#include <allegro.h>

int mickeyx = 0;
int mickeyy = 0;
int btn=0;

int mickeyx2 = 0;
int mickeyy2 = 0;
int btn2=0;

int k_mousex = 0;
int k_mousey = 0;
int k_mouseb = 0;


/*
               1st byte        2nd byte         3rd byte
          ÚÄÂÄÂÄÂÄÂÄÂÄÂÄÂÄ¿ÚÄÂÄÂÄÂÄÂÄÂÄÂÄÂÄ¿ÚÄÂÄÂÄÂÄÂÄÂÄÂÄÂÄ¿
          ³-³1³?³?³Y³Y³X³X³³-³0³X³X³X³X³X³X³³-³0³Y³Y³Y³Y³Y³Y³
          ÀÄÁÄÁÄÁÄÁÄÁÄÁÄÁÄÙÀÄÁÄÁÄÁÄÁÄÁÄÁÄÁÄÙÀÄÁÄÁÄÁÄÁÄÁÄÁÄÁÄÙ
               ³ ³ ÀÂÙ ÀÂÙ      ÀÄÄÄÄÂÄÄÄÄÙ      ÀÄÄÄÄÂÄÄÄÄÙ
               ³ ³  ³   ³            ³                ³
               ³ ³  ÀÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄ¿       ³
               ³ ³      ÀÄÄÄÄ¿       ³        ³       ³
               ³ ³          ÚÁ¿ ÚÄÄÄÄÁÄÄÄÄ¿  ÚÁ¿ ÚÄÄÄÄÁÄÄÄÄ¿
               ³ ³         ÚÄÂÄÂÄÂÄÂÄÂÄÂÄÂÄ¿ÚÄÂÄÂÄÂÄÂÄÂÄÂÄÂÄ¿
               ³ ³         ³ ³ ³ ³ ³ ³ ³ ³ ³³ ³ ³ ³ ³ ³ ³ ³ ³
 Left Button ÄÄÙ ³         ÀÄÁÄÁÄÁÄÁÄÁÄÁÄÁÄÙÀÄÁÄÁÄÁÄÁÄÁÄÁÄÁÄÙ
Right Button ÄÄÄÄÙ            X increment      Y increment
*/

void ChkMouse(void) {
  static unsigned char b1,b2,b3;

  get_mouse_mickeys(&mickeyx, &mickeyy);
  btn  =(mouse_b & 2)?0x20:0;
  btn |=(mouse_b & 1)?0x10:0;

  if ( (mickeyx != mickeyx2) || (mickeyy != mickeyy2) || (btn != btn2) ) {
    b2=mickeyx;
    b3=mickeyy;

    b1=0x40 | btn | ((b3&0xc0)>>4) | ((b2&0xc0)>>6);
    b2 &= 0x3f;
    b3 &= 0x3f;

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

  mickeyx2=mickeyx;
  mickeyy2=mickeyy;
  btn2    =btn; 
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

void ChkMouse_MouseSystem(void) {
  static unsigned char b1,b2,b3;

  get_mouse_mickeys(&mickeyx, &mickeyy);

  btn  = 0x80;
  btn |=(mouse_b & 2)?0:0x1;
  btn |=(mouse_b & 1)?0:0x4;
  btn |=(mouse_b & 4)?0:0x2;

  if ( (mickeyx != mickeyx2) || (mickeyy != mickeyy2) || (btn != btn2) ) {
    b2=mickeyx;
    b3=(256-mickeyy)&0xff;

    AddSerialQueue(btn);
    AddSerialQueue(b2);
    AddSerialQueue(b3);
    AddSerialQueue(0);
    AddSerialQueue(0);
  }

  mickeyx2=mickeyx;
  mickeyy2=mickeyy;
  btn2    =btn; 
}
