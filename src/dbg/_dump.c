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
/*
   int           Y;       // Y координата курсора в окне (0..YLine) относительно окна
   int           YLine;   // сколько строк в текущем окне (линий в поле)
   int           BaseY;   // строчка на экране
   int           BaseAddr;// Базовый адрес окна (т.е. адрес в левом окне)
   int           Cursor;  // номер поля Field
   int           MaxField;// сколько реально записей в след. поле.
   struct sFIELD Field[1+1+16+1+16+1]; // Label Addr 16hex stopfiled 16dmp (MAX variant)
*/

struct ZONE DUMP_ZONE={ 0,10,19,0,
                        1,(1+1+16+1+16+1),
                       {{  0      ,15,zLABEL},
                        { 17      , 4,zADDR },
                        { 23+0x0*3, 2,zHEX  },{ 23+0x1*3, 2,zHEX},{ 23+0x2*3, 2,zHEX},{ 23+0x3*3, 2,zHEX},	
                        { 23+0x4*3, 2,zHEX  },{ 23+0x5*3, 2,zHEX},{ 23+0x6*3, 2,zHEX},{ 23+0x7*3, 2,zHEX},	
                        { 23+0x8*3, 2,zHEX  },{ 23+0x9*3, 2,zHEX},{ 23+0xa*3, 2,zHEX},{ 23+0xb*3, 2,zHEX},	
                        { 23+0xc*3, 2,zHEX  },{ 23+0xd*3, 2,zHEX},{ 23+0xe*3, 2,zHEX},{ 23+0xf*3, 2,zHEX},
                        { 70      , 1,zBRK  },
                        { 71+0x0*1, 1,zASC  },{ 71+0x1*1, 1,zASC},{ 71+0x2*1, 1,zASC},{ 71+0x3*1, 1,zASC},	
                        { 71+0x4*1, 1,zASC  },{ 71+0x5*1, 1,zASC},{ 71+0x6*1, 1,zASC},{ 71+0x7*1, 1,zASC},	
                        { 71+0x8*1, 1,zASC  },{ 71+0x9*1, 1,zASC},{ 71+0xa*1, 1,zASC},{ 71+0xb*1, 1,zASC},	
                        { 71+0xc*1, 1,zASC  },{ 71+0xd*1, 1,zASC},{ 71+0xe*1, 1,zASC},{ 71+0xf*1, 1,zASC},
                        { 70+16+1 , 1,zBRK  },
                       }
                      };

void Update_DUMP(void) {
  int x,y,i;
  byte BUF[1024];
  byte ch,color;
  int Key;
  word Addr;

  char Label[128]="";
  _Label *L;

  tFontSelect(PC_FONT);
  Addr=DUMP_ZONE.BaseAddr;

  for (y=0;y<DUMP_ZONE.YLine;y++) {

     Label[0]=0;
     L=FindAddrLabel(Addr);
     if (L != NULL) {strcpy(Label,L->Name);strcat(Label,":");}

     sprintf(BUF,"%-16s %04X: ",Label,Addr);
     tScreenPutString(BUF,C_Default,0,DUMP_ZONE.BaseY+y);
     for (x=0;x<0x10;x++) {
       ch=Emulator_Read(Addr);
       color=GetBreakColor(Addr,mDUMP);

       sprintf(BUF,"%02x",ch);
       tScreenPutString(BUF,color,23+x*3,DUMP_ZONE.BaseY+y);
       tScreenPutChar(' ',C_Default,23+x*3+2,DUMP_ZONE.BaseY+y);

       tFontSelect(KORVET_FONT);
       tScreenPutChar(GetVisibleChar(ch),color,23+16*3+x,DUMP_ZONE.BaseY+y);
       tFontSelect(PC_FONT);

       Addr++;
     }
     tScreenPutChar('¦',C_Border,70,DUMP_ZONE.BaseY+y);
     tScreenPutChar('¦',C_Border,70+16+1,DUMP_ZONE.BaseY+y);
  }
                                                                                           
  tScreenPutString("----------------------------------------------------------------------¦",C_Border,0,DUMP_ZONE.BaseY-1);
  tScreenPutChar('¦',C_Border,70+16+1,DUMP_ZONE.BaseY-1);
//  tScreenPutString("----------------------------------------------------------------------+----------------+",C_Border,0,DUMP_ZONE.BaseY-1);
  tScreenPutString("----------------------------------------------------------------------+-----------------",C_Border,0,DUMP_ZONE.BaseY+DUMP_ZONE.YLine);
}       

int _DUMP(int Key){
  int i;
  int Key1,Shift=0;

  word Addr;


  char BUF[128];

  struct ZONE    *zone=&DUMP_ZONE;
/*
  tSetUpdate(0);
  Update_Screen();
  DBG_Walker(Key,zone,zone->BaseY+zone->Y);
  tSetUpdate(1);

  while (1) {
*/
    tSetUpdate(0);

    Addr=zone->BaseAddr+(zone->Y*16);
    if (zone->Field[zone->Cursor].Type == zHEX) Addr+=zone->Cursor-2;
    if (zone->Field[zone->Cursor].Type == zASC) Addr+=zone->Cursor-19;

//    Key=readkey();
//    tSetUpdate(0);

    switch (Key>>8) {
       case KEY_UP  : {
                       if (zone->Y == 0            ) zone->BaseAddr-=0x10;
                       else zone->Y--;
                       break;
                      }
       case KEY_DOWN: {
                       if (zone->Y == zone->YLine-1) zone->BaseAddr+=0x10;
                       else zone->Y++;
                       break;
                      }
       case KEY_LEFT : { if ( (Key&0xff) == KK_Ctrl ) {zone->BaseAddr-=1;Key=-1;}break;}
       case KEY_RIGHT: { if ( (Key&0xff) == KK_Ctrl ) {zone->BaseAddr+=1;Key=-1;}break;}

       case KEY_PGUP: {
                       for(i=0;i<zone->YLine-1;i++) simulate_keypress(KEY_UP<<8);
                       break;
                      }
       case KEY_PGDN: {
                       for(i=0;i<zone->YLine-1;i++) simulate_keypress(KEY_DOWN<<8);
                       break;
                      }
       case KEY_TAB:  {
                       if (zone->Field[zone->Cursor].Type == zHEX) zone->Cursor+=16+1;   
                       else if (zone->Field[zone->Cursor].Type == zASC) zone->Cursor-=16+1;   
                       break;   
                      }
       case KEY_F2:   {
                       switch (Key&0xff) {
                         case KK_Ctrl : {i=bpRDMEM;break;}
                         case KK_Shift: {i=bpWRMEM;break;}
                         default: i=bpRDMEM|bpWRMEM;
                       }
                       WR_BreakPoint(Addr,RD_BreakPoint(Addr)^i);
                       break;   
                      }
    }

    Update_Screen();

    tSetUpdate(1);
    Key1=DBG_Walker(Key,zone,zone->BaseY+zone->Y);

    Addr=zone->BaseAddr+(zone->Y*16);
    if (zone->Field[zone->Cursor].Type == zHEX) Addr+=zone->Cursor-2;
    if (zone->Field[zone->Cursor].Type == zASC) Addr+=zone->Cursor-19;

    sprintf(BUF,"%04x",Addr);
    tScreenPutString(BUF,C_Default,0,zone->BaseY-1);

    i=RD_BreakPoint(Addr);
    if (i & bpRDMEM) tScreenPutString("[C_Read]",C_BPRD,10,zone->BaseY-1);
    if (i & bpWRMEM) tScreenPutString("[S_Write]",C_BPRW,19,zone->BaseY-1);

    if (Key1>>8 == zEDIT) {
        tSetUpdate(1);
        switch (Key1&0xff) {
          case zLABEL : {
                           int tmp;
                           _Label *L;
                           char BUF[128]="";
                           char BUF1[128]="";

                           L=FindAddrLabel(Addr);

                           if ( (KEY_DEL == Key>>8) && (L != NULL) ) {
                               DeleteLabel(Addr);
                               break;
                           }

                           if (L != NULL) strcpy(BUF,L->Name);
 
                           tmp = LineEdit(BUF,LABELLEN,zone->Field[zone->Cursor].X,zone->BaseY+zone->Y);

                           if (tmp > 0) {
                              if (L == NULL) { // label with empty label field
                                L=FindNameLabel(BUF);
                                if (L == NULL) AddLabel(Addr,BUF);
                                else  zone->BaseAddr=L->Addr-(zone->Y)*0x10;
                              } else {          // edit old label 
                                DeleteLabel(Addr);
                                AddLabel(Addr,BUF);
                              }
                           }
                           break;
          }
          case zADDR  : {
                           int tmp;
                           if (KEY_ENTER != Key>>8) simulate_keypress(Key);
                           tmp = HEXEDIT(Addr,4,zone->Field[zone->Cursor].X,zone->BaseY+zone->Y);
                           if (tmp >= 0) {
                              zone->BaseAddr=tmp-(zone->Y)*0x10;
                           }
                           break;
          }             
          case zHEX   : {
                           int tmp;
                           if (KEY_ENTER != Key>>8) simulate_keypress(Key);
                           tmp = HEXEDIT(Emulator_Read(Addr),2,zone->Field[zone->Cursor].X,zone->BaseY+zone->Y);
                           if (tmp >= 0) {
                              Emulator_Write(Addr,tmp);
                              if (zone->Field[zone->Cursor+1].Type == zBRK) {
                                for(i=0;i<15;i++) simulate_keypress(KEY_LEFT<<8);
                                simulate_keypress(KEY_DOWN<<8);
                              } else {
                                simulate_keypress(KEY_RIGHT<<8);
                              }
//                              simulate_keypress(KEY_ENTER<<8);
                           }
                           break;
          }
          case zASC   : {
                           Emulator_Write(Addr,Key&0xff);
                           if (zone->Field[zone->Cursor+1].Type == zBRK) {
                             zone->Cursor-=15;
//                             for(i=0;i<15;i++) simulate_keypress(KEY_LEFT<<8);
                             simulate_keypress(KEY_DOWN<<8);
                           } else {
                             simulate_keypress(KEY_RIGHT<<8);
                           }
                           break;
          }             
       }
       Key=0;
       tSetUpdate(0);
       Update_Screen();
       DBG_Walker(0,zone,zone->BaseY+zone->Y);
       tSetUpdate(1);
    }
//
//    if ((Key>>8) == KEY_ESC) {
//      tSetUpdate(1);
//      Update_Screen();
//      break;
//    }
//  }
    return Key;
}
