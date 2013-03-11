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
#include "dbg.h"

#define HISTORYLEN 19

word dbg_HISTORY[HISTORYLEN+1];
int dbg_HISTORYPTR=0;

void Update_HISTORY(void) {
  int x=90,y=19;
  int i,j;
  byte BUF[128];
  byte ASM[128];

  j=dbg_HISTORYPTR-1;
  for (i=HISTORYLEN-1;i>=0;i--) {
     if (j<0) j=HISTORYLEN-1;
     DASM(ASM,dbg_HISTORY[j]);
     sprintf(BUF,"%04x : %-28s",dbg_HISTORY[j],ASM); 
     tScreenPutString(BUF,C_Default,x,y+i);
     j--;
  }
}       

void AddPC(word PC) {
  dbg_HISTORY[dbg_HISTORYPTR]=PC;
  dbg_HISTORYPTR++;
  if (dbg_HISTORYPTR == HISTORYLEN) dbg_HISTORYPTR=0;
}
