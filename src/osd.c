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
#include "korvet.h"
#include "vg.h"


extern int AllScreenUpdateFlag;

int OSD_LUT_Flag=0;
int OSD_FPS_Flag=0;
int OSD_FDD_Flag=0;
int OSD_KBD_Flag=0;

extern int SCREEN_OFFX;
extern int SCREEN_OFFY;
extern int SCREEN_XMAX;
extern int SCREEN_YMAX;
extern int SCREEN_OSDY;

// extern int OSD_LUT_Flag;
// extern int OSD_FPS_Flag;
// extern int OSD_FDD_Flag;

extern int InUseFDD[4];

extern int FPS_Scr;
extern int FPS_LED;

int InUseKBD=0;                 // in use flag
int InUseKBD_rows[16];          // Frames to show OSD KBD
int InUseKBD_single_rows[16];   // Frames to show OSD KBD single row

int CurrentKbdOSD[16];          // current row color - speed optimizarion
int CurrentKbdOSD_single[16];


void PrintDecor(void);

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

void update_osd(void);

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

  d0=i%   10;i-=d0;
  d1=i%  100;i-=d1;d1/=  10;
  d2=i% 1000;i-=d2;d2/= 100;
  d3=i%10000;i-=d3;d3/=1000;

  rectfill(screen,x,y,x+4*(7+2)-1,y+10,makecol8(200,0,0) );

  if (d3 > 15) d3=15; // check if value >10000

  if (d3)       masked_blit(DIGITS[d3],screen,0,0,x+2+1+7*0,y+2,7,7);
  if (d3 || d2) masked_blit(DIGITS[d2],screen,0,0,x+2+1+7*1,y+2,7,7);
                masked_blit(DIGITS[d1],screen,0,0,x+2+1+7*2,y+2,7,7);
                masked_blit(DIGITS[d0],screen,0,0,x+2+1+7*3,y+2,7,7);

}

void UpdateKBD_OSD(int Addr) {
  int shift=0x0001;
  int i=0;
  int offset=0;

  if (Addr & 0x0100) offset=8;

  Addr &= 0xff;

  if (Addr == 0x01) {InUseKBD_single_rows[offset+0]=6;}
  if (Addr == 0x02) {InUseKBD_single_rows[offset+1]=6;}
  if (Addr == 0x04) {InUseKBD_single_rows[offset+2]=6;}
  if (Addr == 0x08) {InUseKBD_single_rows[offset+3]=6;}
  if (Addr == 0x10) {InUseKBD_single_rows[offset+4]=6;}
  if (Addr == 0x20) {InUseKBD_single_rows[offset+5]=6;}
  if (Addr == 0x40) {InUseKBD_single_rows[offset+6]=6;}
  if (Addr == 0x80) {InUseKBD_single_rows[offset+7]=6;}

  for (i=0;i<8;i++) {
    // printf("ADD: %d : %d : %04x : %02x : %02x\n",i,offset,Addr,shift,Addr & shift);
    if (Addr & shift) {
      InUseKBD_rows[offset+i]=6;
      InUseKBD=1;
    }
    shift=shift << 1;
  }
}

void PutLED_KBD(int x0,int y0) {

  int x=x0;
  int y=y0;
  int c;
  int i;
  int showFlag=0;
  int xmatrix_color=makecol8(20,80,180);
  int single_row_color=makecol8(20,120,20);

  for (i=0;i<8;i++) {
    if (InUseKBD_rows[i]) {
      InUseKBD_rows[i]--;
      c=InUseKBD_rows[i] ? xmatrix_color : 0 ;
      showFlag=1;
      if (CurrentKbdOSD[i] != c) {
        CurrentKbdOSD[i] = c;
        hline(screen,x,y+i*3  ,x+8,c);
        hline(screen,x,y+i*3+1,x+8,c);
      }
    }

    if (InUseKBD_rows[i+8]) {
      InUseKBD_rows[i+8]--;
      showFlag=1;
      c=InUseKBD_rows[i+8] ? xmatrix_color : 0 ;
      if (CurrentKbdOSD[i+8] != c) {
        CurrentKbdOSD[i+8] = c;
        hline(screen,x+10,y+i*3  ,x+8+10,c);
        hline(screen,x+10,y+i*3+1,x+8+10,c);
      }
    }

    if (InUseKBD_single_rows[i]) {
      InUseKBD_single_rows[i]--;
      c=InUseKBD_single_rows[i] ? single_row_color : 0 ;
      showFlag=1;
      if (CurrentKbdOSD_single[i] != c) {
        CurrentKbdOSD_single[i] = c;
        hline(screen,x-20,y+i*3  ,x-20+8,c);
        hline(screen,x-20,y+i*3+1,x-20+8,c);
      }
    }

    if (InUseKBD_single_rows[i+8]) {
      InUseKBD_single_rows[i+8]--;
      showFlag=1;
      c=InUseKBD_single_rows[i+8] ? single_row_color : 0 ;
      if (CurrentKbdOSD_single[i+8] != c) {
        CurrentKbdOSD_single[i+8] = c;
        hline(screen,x-20+10,y+i*3  ,x-20+8+10,c);
        hline(screen,x-20+10,y+i*3+1,x-20+8+10,c);
      }
    }

  }

  if (showFlag == 0) {
    InUseKBD=0;
    // printf("KBD:OFF\n");
  }
}

void ResetOSD(void) {
  int i;
  //

  InUseKBD=1;
  for (i=0;i<16;i++) {
    CurrentKbdOSD[i]=1;         //not 0
    InUseKBD_rows[i]=1;         //update on frame
    CurrentKbdOSD_single[i]=1;  //not 0
    InUseKBD_single_rows[i]=1;  //update on frame
  }

  //
  InUseFDD[0]=0;
  InUseFDD[1]=0;
  InUseFDD[2]=0;
  InUseFDD[3]=0;
  //
  update_osd();

  InUseKBD=0;
  for (i=0;i<16;i++) {
    CurrentKbdOSD[i]=1; //not 0
    InUseKBD_rows[i]=0;
    CurrentKbdOSD_single[i]=1; //not 0
    InUseKBD_single_rows[i]=0;
  }

  PrintDecor();

  int x=SCREEN_OFFX+512-80-8-8-4-1;
  int y=SCREEN_OSDY;

  hline(screen,x-20,y+4*3-1  ,x+8+10,makecol8(120,20,20));

}

void update_osd(void) {
    // выводим OnScreen LED
    // ТОЛЬКО если есть необходимость обновить индикаторы,
    // иначе будут мигать, да и FPS падает ;-)
    // FPS
    if (OSD_FPS_Flag && (FPS_Scr != FPS_LED)) {
        PutLED_FPS(SCREEN_OFFX,SCREEN_OSDY  ,FPS_Scr);
        FPS_LED=FPS_Scr;
    };
    // Floppy Disk TRACK
    if (OSD_FDD_Flag && InUseFDD[0]) {
        InUseFDD[0]--;
        PutLED_FDD(SCREEN_OFFX+512-80,SCREEN_OSDY,VG.TrackReal[0],InUseFDD[0]);
    }
    if (OSD_FDD_Flag && InUseFDD[1]) {
        InUseFDD[1]--;
        PutLED_FDD(SCREEN_OFFX+512-60,SCREEN_OSDY,VG.TrackReal[1],InUseFDD[1]);
    }
    if (OSD_FDD_Flag && InUseFDD[2]) {
        InUseFDD[2]--;
        PutLED_FDD(SCREEN_OFFX+512-40,SCREEN_OSDY,VG.TrackReal[2],InUseFDD[2]);
    }
    if (OSD_FDD_Flag && InUseFDD[3]) {
        InUseFDD[3]--;
        PutLED_FDD(SCREEN_OFFX+512-20,SCREEN_OSDY,VG.TrackReal[3],InUseFDD[3]);
    }

    // if (JoystickUseFlag) {JoystickUseFlag--;textprintf(screen,font,SCREEN_OFFX+512,SCREEN_OSDY,255,"%s",(JoystickUseFlag==0)?"      ":"JOY:3B");}

    if (getpixel(screen,SCREEN_OFFX-2,SCREEN_OFFY-2) != 255) {
        PrintDecor();
        AllScreenUpdateFlag=1;
    }

    if (OSD_KBD_Flag && InUseKBD) {
       PutLED_KBD(SCREEN_OFFX+512-80-8-8-4-1,SCREEN_OSDY);
    }
}

