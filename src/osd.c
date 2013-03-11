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
// (c) 2003      Korvet Team.
// portition copyright WinUAE team, 
// special thanks for FONT & OSD Ideas

#include <allegro.h>


int OSD_LUT_Flag=0;
int OSD_FPS_Flag=0;
int OSD_FDD_Flag=0;

BITMAP *DIGITS[0x10];

#define TD_NUM_WIDTH 7
#define TD_NUM_HEIGHT 7

#define TD_TOTAL_HEIGHT (TD_PADY * 2 + TD_NUM_HEIGHT)

#define NUMBERS_NUM 16

static char *numbers = { /* ugly */
/*|      |      |      |      |      |      |      |      |      |      |      |      |      |      |      |      */
 "+++++++--++++-+++++++++++++++++-++++++++++++++++++++++++++++++++++++++-+++++-++++++-+++++++++++++-++++++++++++++"
 "+xxxxx+--+xx+-+xxxxx++xxxxx++x+-+x++xxxxx++xxxxx++xxxxx++xxxxx++xxxxx+++xxx+++xxxx+++xxxxx++xxxx+++xxxxx++xxxxx+"
 "+x+++x+--++x+-+++++x++++++x++x+++x++x++++++x++++++++++x++x+++x++x+++x++x+++x++x+++x++x++++++x+++x++x++++++x+++++"
 "+x+-+x+---+x+-+xxxxx++xxxxx++xxxxx++xxxxx++xxxxx+--++x+-+xxxxx++xxxxx++xxxxx++xxxx+-+x+----+x+-+x++xxxx+-+xxxx+-"
 "+x+++x+---+x+-+x++++++++++x++++++x++++++x++x+++x+--+x+--+x+++x++++++x++x+++x++x+++x++x++++++x+++x++x++++++x++++-"
 "+xxxxx+---+x+-+xxxxx++xxxxx+----+x++xxxxx++xxxxx+--+x+--+xxxxx++xxxxx++x+-+x++xxxx+-+xxxxx++xxxx+++xxxxx++x+----"
 "+++++++---+++-++++++++++++++----+++++++++++++++++--+++--+++++++++++++++++-+++++++++-+++++++++++++-++++++++++----"
};

static void write_tdnumber (BITMAP *bmp,int x, int y, int num)
{
    int i,j,c;
    char *numptr;
    
    for (i = 0; i < TD_NUM_HEIGHT; i++) {
      numptr = numbers + num * TD_NUM_WIDTH + NUMBERS_NUM * TD_NUM_WIDTH * i;
      for (j = 0; j < TD_NUM_WIDTH; j++) {
        switch (*numptr++) {
           case 'x' : c=0xff;break;
           case '+' : c=0xfe;break;
           case '-' : c=0x00;break;
        } 
        putpixel (bmp, x+j ,y+i , c);
      }
    }
}

void InitOSD(void) {
   int i;
   for (i=0;i<0x10;i++) {
     DIGITS[i] = create_bitmap(7,7);
     clear_bitmap  (DIGITS[i]);
     write_tdnumber(DIGITS[i], 0, 0, i);
  }
}

void DestroyOSD(void) {
   int i;
   for (i=0;i<0x10;i++) {
     destroy_bitmap(DIGITS[i]);
  }
}

void PutLED_FDD(int x,int y,int i,int OnFlag) {
  int d1,d2,c;

  d1=i/10;
  d2=i%10;
  c=OnFlag?makecol8(0,244,0):makecol8(0,70,0);

  rectfill(screen,x,y,x+18,y+10,c);
  masked_blit(DIGITS[d1],screen,0,0,x+2    ,y+2,7,7);
  masked_blit(DIGITS[d2],screen,0,0,x+2+1+7,y+2,7,7);

}

void PutLED_Lut(int x,int y,int i,int c) {
  int d1,d2;

  d1=i>>4;
  d2=i&0x0f;

  rectfill(screen,x,y,x+18,y+10,c);
  masked_blit(DIGITS[d1],screen,0,0,x+2    ,y+2,7,7);
  masked_blit(DIGITS[d2],screen,0,0,x+2+1+7,y+2,7,7);
  hline(screen,x,y+12,x+18,d2);
  hline(screen,x,y+13,x+18,d2);

  putpixel(screen,x- 1,y+14,255);
  putpixel(screen,x+19,y+14,255);
  putpixel(screen,x- 1,y+19,255);
  putpixel(screen,x+19,y+19,255);

  if (d2 & 0x01) hline(screen,x,y+15,x+18,makecol8(0,0,255));
  if (d2 & 0x02) hline(screen,x,y+16,x+18,makecol8(0,255,0));
  if (d2 & 0x04) hline(screen,x,y+17,x+18,makecol8(255,0,0));
  if (d2 & 0x08) hline(screen,x,y+18,x+18,makecol8(100,100,100));
}

void PutLED_FPS(int x,int y,int i) {
  int d0,d1,d2,d3;

//  textprintf(screen,font,x,y+16,255,"%4d",i);
//  return;

  d0=i%   10;i-=d0;
  d1=i%  100;i-=d1;d1/=  10;
  d2=i% 1000;i-=d2;d2/= 100;
  d3=i%10000;i-=d3;d3/=1000;

//  textprintf(screen,font,x,y+16+10,255,"%d%d%d%d",d3,d2,d1,d0);

  rectfill(screen,x,y,x+4*(7+2)-1,y+10,makecol8(200,0,0) );

  if (d3 > 15) d3=15; // check if value >10000

  if (d3)       masked_blit(DIGITS[d3],screen,0,0,x+2+1+7*0,y+2,7,7);
  if (d3 || d2) masked_blit(DIGITS[d2],screen,0,0,x+2+1+7*1,y+2,7,7);
                masked_blit(DIGITS[d1],screen,0,0,x+2+1+7*2,y+2,7,7);
                masked_blit(DIGITS[d0],screen,0,0,x+2+1+7*3,y+2,7,7);

}
