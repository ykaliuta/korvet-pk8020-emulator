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
#include "korvet.h"
#include  <allegro.h>

#define MAXQ232 30*3

byte Q232[MAXQ232];
int  Q232Len=0;

void AddSerialQueue(byte B) {
  if (Q232Len == MAXQ232) return;
  Q232[Q232Len++]=B;
  PIC_IntRequest(1);
//  textprintf(screen,font,50,50,255," %3d A",Q232Len);
}

byte GetSerialQueue(void) {
  byte ret=Q232[0];
  int i;
  if (Q232Len) {
   Q232Len--;
   Q232[0]=0;
   for(i=1;i<MAXQ232;i++) Q232[i-1]=Q232[i];
//   textprintf(screen,font,50,50,255," %3d R",Q232Len);
   PIC_IntRequest(1);
  }
  return ret;
}

void  InitSerialQueue(void) {
  Q232Len=0;
}


void RS232_Write(int Addr, byte Value){return;}
byte RS232_Read(int Addr) {
 if (Addr & 1) return 0;       // Ctrl  
 else return GetSerialQueue(); // Data
}

void LAN_Write(int Addr,byte Value){return;}
byte LAN_Read(int Addr){
// fixfor KORNET work
  return 0x4;
}

void Serial_Init(void) {
  InitSerialQueue();
}
