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
// Game TOOLS
#include "dbg.h"
#include <allegro.h>

extern byte    RAM[];           // RAM эмулятора

int GT_Pass=0;

#define GT_ZERO 0

#define GT_BYTE 1
#define GT_WORD 2

#define GT_GT   1
#define GT_LT   2
#define GT_EQ   3
#define GT_NE   4

#define GT_NE0  5
#define GT_EQ0  10
#define GT_EQ1  11
#define GT_EQ2  12
#define GT_EQ3  13
#define GT_EQ4  14
#define GT_EQ5  15
#define GT_EQ6  16
#define GT_EQ7  17
#define GT_EQ8  18
#define GT_EQ9  19


int GT_Mode=0;

int GT_Y=30;
int SearchMode=GT_ZERO;

struct _gt_cell {
    word Addr;
    word Value;
};

struct _gt_cell GT_Ram[0x10000];
struct _gt_cell GT_TMP[0x10000];
int GT_Used=0;


void UpdateGameTool(void) {
  byte BUF[1024];
  int i;
  int x,y;
  char mode[2+1][5]={[1] "BYTE",[2] "WORD"};

  tScreenPutString("GameTools : ",C_Default,0,GT_Y+0);

  if (!GT_Pass)   strcpy(BUF,"not initialized                                                  ");
  else            sprintf(BUF,"mode [%s],pass [%2d],values [%5d]              ",mode[GT_Mode],GT_Pass,GT_Used);

  tScreenPutString(BUF,C_Default,12,GT_Y+0);

  tScreenPutString("                                                                                        ",C_Default,0,GT_Y+1);

  for (y=GT_Y+2;y<GT_Y+7;y++) 
   tScreenPutString("                                                                                       ",C_Default,0,y);

  if ( (GT_Used>0) && (GT_Used <= 15) ) {
    x=0;y=GT_Y+2;
    for (i=0;i<GT_Used;i++) {
//                                           01) aabb:   ff (255)
//                                           01) aabb: ffff (65535)
     if (GT_Mode == GT_BYTE)  sprintf(BUF,"%02d) %04x: %02x [%3d]",i+1,GT_Ram[i].Addr,GT_Ram[i].Value,GT_Ram[i].Value);
     else                     sprintf(BUF,"%02d) %04x: %04x [%6d]",i+1,GT_Ram[i].Addr,GT_Ram[i].Value,GT_Ram[i].Value);
     tScreenPutString(BUF,C_Default,x,y);
     y++;
     if (y == GT_Y+7) {y=GT_Y+2;x+=25;}
    }
  } else {
     if (GT_Used>15) 
       tScreenPutString(" too many values ... ",C_Default,45,GT_Y+4);
  }

  tScreenPutString("----------------------------------------------------------------------------------------",C_Border,0,GT_Y+7);

  sprintf(BUF,"SM:%2d ",SearchMode);

  tScreenPutString(BUF,C_Border,0,GT_Y+7);
}


void GameTools(void) {
  int Key;
  int Exit=0;
  int i,j,tmp;
  int cmp;
  char BUF[64];

  if (GT_Used == 0) {
     GT_Mode=GT_ZERO;
     GT_Pass=0;
  }

  UpdateGameTool();

  if (GT_Pass == 0) {
     tScreenPutString("  B) new BYTE mode, W) new WORD mode, ESC - exit                ",C_Edit,0,GT_Y+1);
     GT_Mode=GT_ZERO;
     while (!Exit) {
       Key=readkey();
       switch (Key>>8) {
          case KEY_B   : {Exit=1;GT_Mode=GT_BYTE;GT_Pass=0;break;}
          case KEY_W   : {Exit=1;GT_Mode=GT_WORD;GT_Pass=0;break;}
          case KEY_ESC : {Exit=1;break;}
       }
     }
     
  } else {
     tScreenPutString("1) < 2) > 3) = 4) != 9) !=0 0) ==0 C+(0-9)==(0..9), B/W) new Byte/Word, C) Clr ESC) Exit",C_Edit,0,GT_Y+1);
     while (!Exit) {
       Key=readkey();

       switch (Key>>8) {
          case KEY_C   : {Exit=1;GT_Mode=GT_ZERO;GT_Pass=0;GT_Used=0;break;}
          case KEY_B   : {Exit=1;GT_Mode=GT_BYTE;GT_Pass=0;break;}
          case KEY_W   : {Exit=1;GT_Mode=GT_WORD;GT_Pass=0;break;}

          case KEY_0 ... KEY_9   : {
                          if ( KK_Ctrl == (Key & 0xff)  ) {
                                  SearchMode=GT_EQ0+( (Key>>8)-KEY_0 );
                                  Exit=1;
                          } else {
                                 if (Key>>8 == KEY_1) {SearchMode=GT_LT ;Exit=1;}
                            else if (Key>>8 == KEY_2) {SearchMode=GT_GT ;Exit=1;}
                            else if (Key>>8 == KEY_3) {SearchMode=GT_EQ ;Exit=1;}
                            else if (Key>>8 == KEY_4) {SearchMode=GT_NE ;Exit=1;}
                            else if (Key>>8 == KEY_0) {SearchMode=GT_EQ0;Exit=1;}
                            else if (Key>>8 == KEY_9) {SearchMode=GT_NE0;Exit=1;}
                          }

                          break;
                         }

          case KEY_ESC : {SearchMode=GT_ZERO;Exit=1;break;}
       }
     }
  }

  if ( (GT_Mode != 0) && (GT_Pass == 0) ) {
     for (i=0;i<=0xffff;i++) {
        GT_Ram[i].Addr=i;
        if (GT_Mode == GT_BYTE) GT_Ram[i].Value=RAM[i];
        else                    GT_Ram[i].Value=RAM[i]|(RAM[(i+1)&0xffff]<<8) ;
     }
     GT_Used=0xffff;
     GT_Pass=1;
  } else {
     if (GT_Mode) GT_Pass++;
     if (GT_Pass && !SearchMode) GT_Pass--; // if press ESC don't inc counter
  }

  if (SearchMode && (GT_Pass>1)) {

    j=0;
    for (i=0;i<GT_Used;i++) {
      if (GT_Mode == GT_BYTE) tmp=RAM[GT_Ram[i].Addr];
      else                    tmp=RAM[GT_Ram[i].Addr]|(RAM[(GT_Ram[i].Addr+1)&0xffff]<<8) ;

      cmp=0;
      switch (SearchMode) {
         case GT_GT: {cmp=(tmp >GT_Ram[i].Value);break;}
         case GT_LT: {cmp=(tmp <GT_Ram[i].Value);break;}
         case GT_EQ: {cmp=(tmp =GT_Ram[i].Value);break;}
         case GT_NE: {cmp=(tmp!=GT_Ram[i].Value);break;}

         case GT_NE0:{cmp=(tmp!=0);break;}

         case GT_EQ0 ... GT_EQ9 : {
       sprintf(BUF,"3Key:%04x ",SearchMode-GT_EQ0);
       tScreenPutString(BUF,C_Border,13,GT_Y+9);
              cmp=(tmp==(SearchMode-GT_EQ0));
              break;
         }

      }

      if (cmp) {
         GT_Ram[j].Addr =GT_Ram[i].Addr;
         GT_Ram[j].Value=tmp;
         j++;
      }
    }
    GT_Used=j;
  }
  UpdateGameTool();
}
