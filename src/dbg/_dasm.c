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

extern struct CPUREG dbg_REG;
extern int           dbg_TRACE;   //если =1 то остановится ПЕРЕД выполнением комманды
extern word          dbg_HERE;    //адрес остановки при нажатии клавиши F4 (HERE)

/*
   int           Y;       // Y координата курсора в окне (0..YLine) относительно окна
   int           YLine;   // сколько строк в текущем окне (линий в поле)
   int           BaseY;   // строчка на экране
   int           BaseAddr;// Базовый адрес окна (т.е. адрес в левом окне)
   int           Cursor;  // номер поля Field
   int           MaxField;// сколько реально записей в след. поле.
   struct sFIELD Field[1+1+16+1+16+1]; // Label Addr 16hex stopfiled 16dmp (MAX variant)
*/
struct ZONE DASM_ZONE={ 0,20-6-1,2+3,0,
                        1,(1+1+3+1+1+1+1),
                       {{  0      ,15,zLABEL},
                        { 17      , 4,zADDR },
                        { 23+0x0*3, 2,zHEX  },{ 23+0x1*3, 2,zHEX},{ 23+0x2*3, 2,zHEX},
                        { 31      , 1,zSKIP },
                        { 35      ,28,zDASM },
                        { 53      , 1,zBRK  },
                       }
                      };

void Set_DASMAddr(word Addr) {
  DASM_ZONE.BaseAddr=Addr;
}
// Return len
void Update_DASM(void)
{
  int     len,i,j;
  char    BUF[1024];
  char    ASM[1024];
  char    Label[128]="";
  word    Addr;
  int     x,y,color;
  int     defcolor;
  byte    ch;
  _Label  *L;

  tFontSelect(PC_FONT);
  Addr=DASM_ZONE.BaseAddr;

  for (y=0;y<DASM_ZONE.YLine;y++) {

     Label[0]=0;
     L=FindAddrLabel(Addr);
     if (L != NULL) {strcpy(Label,L->Name);strcat(Label,":");}

     defcolor=(Addr == dbg_REG.PC)?C_PC:C_Default;

     tScreenPutString(" ",defcolor,23+2*3+2+3,DASM_ZONE.BaseY+y);

     sprintf(BUF,"%-16s ",Label);
     tScreenPutString(BUF,defcolor,0,DASM_ZONE.BaseY+y);

     color=GetBreakColor(Addr,mDASM);
     color=(color == C_Default)?defcolor:color;

     sprintf(BUF,"%04X",Addr);
     tScreenPutString(BUF,color,17,DASM_ZONE.BaseY+y);
     if (color == C_BPCPU) tScreenPutString(">>",color,35-3,DASM_ZONE.BaseY+y);
     else tScreenPutString("  ",color,35-3,DASM_ZONE.BaseY+y);

     sprintf(BUF,": ");
     tScreenPutString(BUF,defcolor,17+4,DASM_ZONE.BaseY+y);

     len=DASM(ASM,Addr);

     for (x=0;x<len;x++) {
       ch=Emulator_Read(Addr+x);
       color=GetBreakColor(Addr+x,mDUMP);
       color=(color == C_Default)?defcolor:color;

       sprintf(BUF,"%02x",ch);
       tScreenPutString(BUF,color,23+x*3,DASM_ZONE.BaseY+y);
       tScreenPutChar(' ',defcolor,23+x*3+2,DASM_ZONE.BaseY+y);
     }

     for (x=len;x<3;x++) {
       sprintf(BUF,"..");
       tScreenPutString(BUF,defcolor,23+x*3,DASM_ZONE.BaseY+y);
       tScreenPutChar(' ',defcolor,23+x*3+2,DASM_ZONE.BaseY+y);
     }

     tFontSelect(KORVET_FONT);
     sprintf(BUF,"%-28s",ASM);
     tScreenPutString(BUF,defcolor,35,DASM_ZONE.BaseY+y);
     tFontSelect(PC_FONT);

//     tScreenPutChar('|',C_Border,70,DASM_ZONE.BaseY+y);

     Addr+=len;
  }
  tScreenPutString("---------------------------------------------------------------",C_Border,0,4);
}
word FixWalker() {
    word Addr;
    char buf[128];
    int  len;
    int  i,x;

    Addr=DASM_ZONE.BaseAddr;
    for (i=0;i<DASM_ZONE.Y;i++) Addr+=DASM(buf,Addr);

    len=DASM(buf,Addr);

     for (x=0;x<len;x++) {
        DASM_ZONE.Field[2+x].Type=zHEX;
     }
     for (x=len;x<3;x++) {
        DASM_ZONE.Field[2+x].Type=zSKIP;
     }
     return Addr;
}

void NormPC(void) {
 int i,tmp=DASM_ZONE.BaseAddr;
 int FlagOnScreen=0;

 for (i=0;i<DASM_ZONE.YLine-1;i++) {
   if   (tmp == dbg_REG.PC) {FlagOnScreen=1;break;}
   else tmp+=GetCmdLen(tmp);
 }
 if (!FlagOnScreen) {
    DASM_ZONE.BaseAddr=dbg_REG.PC;
    DASM_ZONE.Y=0;
//    simulate_keypress(KEY_UP<<8);
//    simulate_keypress(KEY_DOWN<<8);
 }
}

int _DASM(int Key) {
  int i;
  int Key1,Shift=0;
  int len;

  word Addr;

  char BUF[128];

  struct ZONE  *zone=&DASM_ZONE;

  tSetUpdate(0);
//    Key=readkey();
  Addr=FixWalker();

    switch (Key>>8) {
       case KEY_LEFT : { if ( (Key&0xff) == KK_Ctrl ) {zone->BaseAddr-=1;Key=-1;}break;}
       case KEY_RIGHT: { if ( (Key&0xff) == KK_Ctrl ) {zone->BaseAddr+=1;Key=-1;}break;}

       case KEY_UP  : {
                       if (zone->Y == 0          ) { 
                              if      (GetCmdLen(zone->BaseAddr-3) == 3) i=3;
                              else if (GetCmdLen(zone->BaseAddr-2) == 2) i=2;
                              else if (GetCmdLen(zone->BaseAddr-1) == 1) i=1;
                              else i=1; // ???? no posible
                              zone->BaseAddr-=i;
                       } else zone->Y--;
                       break;
                      }
       case KEY_DOWN: {
                       if (zone->Y == zone->YLine-1) zone->BaseAddr+=DASM(BUF,zone->BaseAddr);
                       else zone->Y++;
                       break;
                      }
       case KEY_PGUP: {
                       for(i=0;i<zone->YLine-1;i++) simulate_keypress(KEY_UP<<8);
                       break;
                      }
       case KEY_PGDN: {
                       for(i=0;i<zone->YLine-1;i++) simulate_keypress(KEY_DOWN<<8);
                       break;
                      }
       case KEY_F2:   {
                       WR_BreakPoint(Addr,RD_BreakPoint(Addr)^bpCPU);
                       break;   
                      }
       case KEY_F4:   {
                       dbg_HERE=Addr;
                       Key=KEY_F9<<8; // emulate RUN
                       break;   
                      }
       case KEY_K:   { 
                       if ( (Key&0xff) == KEY_K ) {
                         int tmp=GetCmdLen(Addr);
                         for(i=0;i<tmp;i++) Emulator_Write(Addr+i,0);
                       }
                       break;   
                      }
       case KEY_N:   { 
                       if ( (Key&0xff) == KEY_N ) {
                         dbg_REG.PC=Addr;
                       }
                       break;   
                      }
    }

    Addr=FixWalker();
    Update_Screen();

    tSetUpdate(1);

    Key1=DBG_Walker(Key,zone,zone->BaseY+zone->Y);

    if (Key1>>8 == zEDIT) {
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
                                else {
                                  zone->BaseAddr=L->Addr;
                                  tmp=zone->Y;
                                  zone->Y=0;
                                  for(i=0;i<tmp;i++) simulate_keypress(KEY_UP<<8);
                                  for(i=0;i<tmp;i++) simulate_keypress(KEY_DOWN<<8);
                                }
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
                              zone->BaseAddr=tmp;
                              tmp=zone->Y;
                              zone->Y=0;
                              for(i=0;i<tmp;i++) simulate_keypress(KEY_UP<<8);
                              for(i=0;i<tmp;i++) simulate_keypress(KEY_DOWN<<8);
                           }
                           break;
          }             
          case zHEX   : {
                           int tmp;
                           if (KEY_ENTER != Key>>8) simulate_keypress(Key);
                           Addr+=zone->Cursor-2;
                           tmp = HEXEDIT(Emulator_Read(Addr),2,zone->Field[zone->Cursor].X,zone->BaseY+zone->Y);
                           if (tmp >= 0) {
                              Emulator_Write(Addr,tmp);
                              simulate_keypress(KEY_RIGHT<<8);
                           }
                           break;
          }
          case zASC   : {
                           Emulator_Write(Addr,Key&0xff);
                           if (zone->Field[zone->Cursor+1].Type == zBRK) {
                             zone->Field[zone->Cursor+1].X-=16;
                             simulate_keypress(KEY_DOWN<<8);
                           } else {
                             simulate_keypress(KEY_RIGHT<<8);
                           }
                           break;
          }             
          case zDASM  : {
                          int i,j;
                          char BUF[128];
                          char code[8];

                          DASM(BUF,Addr);
                          i=0;
                          while (!i) {
                            if (LineEdit(BUF,zone->Field[zone->Cursor].Len-1,zone->Field[zone->Cursor].X,zone->BaseY+zone->Y)) {
                              if ( (j=ASMStr(BUF,code)) > 0 ) {
                                while (j--)  Emulator_Write(Addr+j,code[j]);
                                i=1;
                                } else {/*ScreenVisualBell();*/}
                            } else i=1;
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
   return Key;
}

