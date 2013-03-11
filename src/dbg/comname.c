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
// Korvet ETALON emulator project
// (c) 2000-2003 Sergey Erokhin aka ESL
//
// Debuger: 
//  отобразить имя текущего com файла, для этого ставим брякпоинт (в _main.c) на адрес 
//  в CCP в том месте где пускаем файл, и считываем из FCB имя файла.
//
//  2003-11-05 - v 1.0 

#include <stdio.h>
#include "dbg.h"

extern int SYSREG;
//                          filename.ext
byte COM_NAME[8+3+1+1+2]="a:????????.???";


void UpdateCOMNAME(void) {
  byte BUF[128];

  sprintf(BUF,"now:>%s<    ",COM_NAME);
  tScreenPutString(BUF,C_Border,110-5,18);
}

void CheckComEXEC(void) {
  byte pattern[7]={0xca,0x6b,0xcb,0x21,0x00,0x01,0xe5};
  int i,j;
  byte b;

  if (SYSREG != 0x1c>>2) return ;

  for (i=0;i<7;i++) {
    if ( pattern[i] != Emulator_Read(0xcadb+i) ) return; // wrong check pattern;
  }

  j=0;
  for (i=0;i< 8;i++) if ((b=Emulator_Read(0xcbce + i)) != ' ') COM_NAME[j++]=b;
  COM_NAME[j++]='.';
  for (i=8;i<11;i++) if ((b=Emulator_Read(0xcbce + i)) != ' ') COM_NAME[j++]=b;
  COM_NAME[j]=0;
}

void CheckCCP(void) {
  byte pattern[7]={0x31,0xab,0xcb,0xc5,0x79,0x1f,0x1f};
  int i;

  if (SYSREG != 0x1c>>2) return ;

  for (i=0;i<7;i++) {
    if ( pattern[i] != Emulator_Read(0xc75c+i) ) return; // wrong check pattern;
  }
  strcpy(COM_NAME,"--- CCP ---");
}

void CheckROM(void) {
  int i;

  if (SYSREG != 0) return ;

  if ( Emulator_Read(0) != 0xf3 ) return; // wrong check pattern;

  strcpy(COM_NAME,"-- Reset --");
}

