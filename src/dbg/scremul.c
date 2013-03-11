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

extern DATAFILE data[];

byte tScrBufChr[TSCRX*TSCRY];
byte tScrBufAtr[TSCRX*TSCRY];
byte tScrBufFnt[TSCRX*TSCRY];
byte tFont=PC_FONT;

byte tScrUpdateFlag[TSCRX*TSCRY];
int  DoUpdateFlag=1;                // if = 0 skip update

// Internal routine
void tDoUpdate(void) {
  int x,y;
  int addr=0;
  int SaveAttr=text_mode(0);
  byte chs[]=" ";

  if (!DoUpdateFlag) return;

  for (y=0;y<TSCRY;y++)
    for (x=0;x<TSCRX;x++) {
      if (tScrUpdateFlag[addr]) {
          font = data[tScrBufFnt[addr]].dat;

          *chs=(tScrBufChr[addr]==0)?' ':tScrBufChr[addr];
          text_mode(0x20+(tScrBufAtr[addr]>>4));
          textout(screen,font,chs,x*8,y*16,0x20+(tScrBufAtr[addr]&0x0f));
          tScrUpdateFlag[addr]=0;
      }
   addr++;
  }
  text_mode(SaveAttr);
}

// PUBLIC routines
void tFontSelect(int Fnt) { 

  if (Fnt == KORVET_FONT) {
    tFont=(scr_Second_Font)?KORVET_FONT1:KORVET_FONT0;
  } else {tFont=PC_FONT;}

  font = data[tFont].dat;
}

void tSetUpdate(int flag) {
  DoUpdateFlag=flag;
  tDoUpdate();
}

void tShowAll(void) {
  int x,y;

  for (y=0;y<TSCRY*TSCRX;y++) tScrUpdateFlag[y]=1;

  for (y=0;y<16+1;y++) 
    for (x=64-1;x<TSCRX;x++) tScrUpdateFlag[y*TSCRX+x]=0;
}

void tScrInit(void) {
 int addr=0;

 while (addr < TSCRX*TSCRY) {
   tScrBufChr[addr]=0;
   tScrBufAtr[addr]=0;
   tScrBufFnt[addr]=PC_FONT;
   tScrUpdateFlag[addr++]=0;
 }
 set_uformat(U_ASCII);
 fixup_datafile(data);
 tFontSelect(PC_FONT);
 tSetUpdate(1);
}

void  tScreenPutChar(int ch, int attr, int col, int row) {
  int update=0;
  int addr=row*TSCRX+col;
  
  if (tScrBufChr[addr] != ch) {tScrUpdateFlag[addr]=1;update=1;}
  tScrBufChr[addr] = ch;
  if (tScrBufAtr[addr] != attr) {tScrUpdateFlag[addr]=1;update=1;}
  tScrBufAtr[addr] = attr;
  if (tScrBufFnt[addr] != tFont) {tScrUpdateFlag[addr]=1;update=1;}
  tScrBufFnt[addr++] = tFont;
  
  if (update) tDoUpdate();
}

void  tScreenPutString(byte *str, int attr, int col, int row) {
  int update=0;
  int addr=row*TSCRX+col;
  byte ch;

  while (*str) {
    ch=*str++;
    if (tScrBufChr[addr] != ch) {tScrUpdateFlag[addr]=1;update=1;}
    tScrBufChr[addr] = ch;
    if (tScrBufAtr[addr] != attr) {tScrUpdateFlag[addr]=1;update=1;}
    tScrBufAtr[addr] = attr;
    if (tScrBufFnt[addr] != tFont) {tScrUpdateFlag[addr]=1;update=1;}
    tScrBufFnt[addr++] = tFont;
  }
  
  if (update) tDoUpdate();
}

void tScreenClear(void) {
 int addr=0;

 while (addr < TSCRX*TSCRY) {
   tScrBufChr[addr]=0;
   tScrBufAtr[addr]=0;
   tScrBufFnt[addr]=PC_FONT;
   tScrUpdateFlag[addr++]=1;
 }
 tDoUpdate();
}

void  tScreenUpdateLine(void *buf, int row) {
  int addr=row*TSCRX;
  int update=0;
//  int i=TSCRX;
  int i=80;
  byte ch;
  while (i--) {

    tScrUpdateFlag[addr]=1;

    ch=*(byte *)buf++;
    if (tScrBufChr[addr] != ch) {tScrUpdateFlag[addr]=1;update=1;}
    tScrBufChr[addr] = ch;
    
    ch=*(byte *)buf++;
    if (tScrBufAtr[addr] != ch) {tScrUpdateFlag[addr]=1;update=1;}
    tScrBufAtr[addr] = ch;

    if (tScrBufFnt[addr] != tFont) {tScrUpdateFlag[addr]=1;update=1;}
    tScrBufFnt[addr++] = tFont;
  }
  if (update) tDoUpdate();
}

void  tSetNewAttr(int attr,int len, int col, int row) {
  int addr=row*TSCRX+col;
  int update=0;
  int i=len;

  while (i--) {
    if (tScrBufAtr[addr] != attr) {tScrUpdateFlag[addr]=1;update=1;}
    tScrBufAtr[addr++] = attr;
  }
  if (update) tDoUpdate();
}


void DBG_Pallete_Active(void) {
   PALLETE pallete;
   int i;
   int bright;
   int c;

   int StartColor=0x20;

   for (i=0;i<16;i++) {
      bright=(i&0x8)?21:0;
      pallete[StartColor+i].r=((i&0x4)?42:0)+bright;
      pallete[StartColor+i].g=((i&0x2)?42:0)+bright;
      pallete[StartColor+i].b=((i&0x1)?42:0)+bright;

   }
   set_palette_range(pallete,StartColor,StartColor+16,1);
}

void DBG_Pallete_Pasive(void) {
   PALLETE pallete;
   int i;
   int bright;
   int c;

   int StartColor=0x20;

   for (i=0;i<16;i++) {
      bright=(i&0x8)?21:0;
      pallete[StartColor+i].r=(((i&0x4)?42:0)+bright)/3;
      pallete[StartColor+i].g=(((i&0x2)?42:0)+bright)/3;
      pallete[StartColor+i].b=(((i&0x1)?42:0)+bright)/3;

   }
   set_palette_range(pallete,StartColor,StartColor+16,1);
}


// 2003-11-23: теперь фонт в дампе зависит от шрифта эмулятора scr_Second_Font
