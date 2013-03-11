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
struct ZONE REG_ZONE[4]={
   [0] {                0,4,0,0,
                        0,(1+8),
                        {{ 10      , 4,zRAF  },
                         { 16+0    , 1,zFLAG  },{ 16+1, 1,zFLAG},{ 16+2, 1,zFLAG},{ 16+3, 1,zFLAG},
                         { 16+4    , 1,zFLAG  },{ 16+5, 1,zFLAG},{ 16+6, 1,zFLAG},{ 16+7, 1,zFLAG},
                         { 31      , 1,zBRK  },
                         { 31      , 1,zSKIP },
                        }
       },
   [1] {                0,4,1,0,
                        0,(1),
                       {{ 10      , 4,zRBC  },
                        { 31      , 1,zBRK },
                        { 31      , 1,zSKIP },{31,1,zSKIP},{31,1,zSKIP},{31,1,zSKIP},{31,1,zSKIP},
                        { 31      , 1,zSKIP },{31,1,zSKIP},{31,1,zSKIP},{31,1,zSKIP},{31,1,zSKIP},
                       }
                       
       },
   [2] {                0,4,2,0,
                        0,(1+1+4),
                        {{ 10      , 4,zRDE  },
                         { 32      , 4,zRSP  },
                         { 38      , 4,zSTACK },{43,4,zSTACK},{48,4,zSTACK},{53,4,zSTACK},
                         { 31      , 1,zBRK },
                         { 31      , 1,zSKIP },{31,1,zSKIP},{31,1,zSKIP},{31,1,zSKIP},{31,1,zSKIP},
                         { 31      , 1,zSKIP },{31,1,zSKIP},{31,1,zSKIP},{31,1,zSKIP},{31,1,zSKIP},
                        }
                       
       },
   [3] {                0,4,3,0,
                        0,(1+1),
                        {{ 10      , 4,zRHL  },
                         { 32      , 4,zRPC  },
                         { 31      , 1,zBRK },
                         { 31      , 1,zSKIP },{31,1,zSKIP},{31,1,zSKIP},{31,1,zSKIP},{31,1,zSKIP},
                         { 31      , 1,zSKIP },{31,1,zSKIP},{31,1,zSKIP},{31,1,zSKIP},{31,1,zSKIP},
                        }
       },
}

;

extern struct CPUREG dbg_REG;
extern struct CPUREG dbg_prevREG;

byte BUF[128];

byte *GetRegPtr(word Reg,int Type) {
  static char BUF[128]="";
  int i;
  
  switch (Type) {
    case 0: sprintf(BUF,"%02x %02x %02x %02x",Emulator_Read(Reg+0),Emulator_Read(Reg+1),Emulator_Read(Reg+2),Emulator_Read(Reg+3));
            break;
    case 1: BUF[9]=BUF[0]='\"';for (i=0;i<8;i++) BUF[i+1]=GetVisibleChar(Emulator_Read(Reg+i));BUF[10]=0;
            break;
    case 2: sprintf(BUF,"'%c%c'",GetVisibleChar(Reg>>8),GetVisibleChar(Reg&0xff));
            break;
  }
  strcat(BUF,"               ");
  BUF[12]=0;
  return BUF;
}

#define getcmpcolor(reg)  (((dbg_REG.reg) == (dbg_prevREG.reg))?C_Default:C_NEQ)
void Update_REGS(void){
 word a0,a1,a2,a3,a4,a5;
 int i;
 char Flag[]="sz.a.p.c";
 byte     fl=dbg_REG.AF & 0xff;
 byte prevfl=dbg_prevREG.AF&0xff;;

                  //0123456789012345678901234567890123456789012345678901234567890123456789
                  //0         1         2         3         4         5         6
//tScreenPutString("     AF = 00   '11111111'  Int DI                              ",C_Default,0,0);
//tScreenPutString("     BC = 0000 \"abcdefgh\"             -3   -2   1    0    +1   ",C_Default,0,1);
//tScreenPutString("     DE = 0000 01 02 03 04 SP = 1111: 1234 2233 4444 5555 6666 ",C_Default,0,2);
//tScreenPutString("     HL = 0000 'ZZ'        PC = 2222 00 00 00 00               ",C_Default,0,3);

 if (fl & 0x80) Flag[0]='S';
 if (fl & 0x40) Flag[1]='Z';
 if (fl & 0x20) Flag[2]='5';
 if (fl & 0x10) Flag[3]='A';
 if (fl & 0x08) Flag[4]='3';
 if (fl & 0x04) Flag[5]='P';
 if (fl & 0x02) Flag[6]='2';
 if (fl & 0x01) Flag[7]='C';

 tFontSelect(KORVET_FONT);
 tScreenPutString("     AF = ",C_Default,0,0);tScreenPutString("  Int:",C_Default,25,0);
 //sprintf(BUF,"%02x   [%s]",dbg_REG.AF>>8,Flag);tScreenPutString(BUF,C_Default,10,0);

 sprintf(BUF,"%02x [%s]",fl,Flag);tScreenPutString(BUF,C_Default,10+2,0);

 if (fl != prevfl) {
   sprintf(BUF,"%02x",fl);tScreenPutString(BUF,C_NEQ,10+2,0);
   for (i=0;i<8;i++) {
     if ( (fl&(0x80>>i)) != (prevfl&(0x80>>i)) ) {
       sprintf(BUF,"%c",Flag[i]);tScreenPutString(BUF,C_NEQ,10+2+4+i,0);
     }
   }
 }

// sprintf(BUF,"%02x [%s]",dbg_REG.AF&0xff,Flag);tScreenPutString(BUF,getcmpcolor(AF&0xff),10+2,0);
 sprintf(BUF,"%02x",(dbg_REG.AF>>8),Flag);tScreenPutString(BUF,getcmpcolor(AF>>8),10,0);

 sprintf(BUF,"%s",dbg_REG.Int?"Enable ":"Disable");tScreenPutString(BUF,getcmpcolor(Int),31,0);

 tScreenPutString("     BC = ",C_Default,0,1);tScreenPutString("          0    -1   -2   -3",C_Default,28,1);
 sprintf(BUF,"%04x ",dbg_REG.BC);tScreenPutString(BUF,getcmpcolor(BC),10,1);
 tScreenPutString(GetRegPtr(dbg_REG.BC,REG_ZONE[1].BaseAddr),C_Shadow,15,1);
 
 tScreenPutString("     DE = ",C_Default,0,2);tScreenPutString(" SP = ",C_Default,26,2);
 sprintf(BUF,"%04x ",dbg_REG.DE);tScreenPutString(BUF,getcmpcolor(DE),10,2);
 tScreenPutString(GetRegPtr(dbg_REG.DE,REG_ZONE[2].BaseAddr),C_Shadow,15,2);

 sprintf(BUF,"%04x",dbg_REG.SP);tScreenPutString(BUF,getcmpcolor(SP),32,2);
 a0=(Emulator_Read(dbg_REG.SP+1)<<8)+Emulator_Read(dbg_REG.SP+0);
 a1=(Emulator_Read(dbg_REG.SP+3)<<8)+Emulator_Read(dbg_REG.SP+2);
 a2=(Emulator_Read(dbg_REG.SP+5)<<8)+Emulator_Read(dbg_REG.SP+4);
 a3=(Emulator_Read(dbg_REG.SP+7)<<8)+Emulator_Read(dbg_REG.SP+6);
// a3=(Emulator_Read(dbg_REG.SP-7)<<8)+Emulator_Read(dbg_REG.SP-8);
 sprintf(BUF,": %04x %04x %04x %04x",a0,a1,a2,a3);tScreenPutString(BUF,C_Default,36,2);

 
 tScreenPutString("     HL = ",C_Default,0,3);tScreenPutString(" PC = ",C_Default,26,3);
 sprintf(BUF,"%04x ",dbg_REG.HL);tScreenPutString(BUF,getcmpcolor(HL),10,3);
 sprintf(BUF,"%04x ",dbg_REG.PC);tScreenPutString(BUF,C_Default,32,3);
 tScreenPutString(GetRegPtr(dbg_REG.HL,REG_ZONE[3].BaseAddr),C_Shadow,15,3);
 tFontSelect(PC_FONT);
}

int _REGS(int Key){
  int i;
  int Key1,Shift=0;
  int YY=0;

  char BUF[128];

  YY=REG_ZONE[0].Y;

  struct ZONE    *zone=&REG_ZONE[YY];

  tSetUpdate(0);
//  Update_Screen();
//  DBG_Walker(0,zone,zone->BaseY+zone->Y);

//    Key=readkey();

    switch (Key>>8) {
       case KEY_UP  : {
                       if (YY != 0            ) YY--;
                       Key=-1;
                       break;
                      }
       case KEY_DOWN: {
                       if (YY != zone->YLine-1) YY++;
                       Key=-1;
                       break;
                      }

    }

    REG_ZONE[0].Y=YY;
    zone=&REG_ZONE[YY];

    Update_Screen();

    tSetUpdate(1);
    Key1=DBG_Walker(Key,zone,zone->BaseY+zone->Y);

    if (Key1>>8 == zEDIT) {
        switch (Key1&0xff) {
          case zFLAG  : {
                           dbg_REG.AF^=1<<(8-zone->Cursor);
                           break;
          }
          case zRAF   : 
          case zRBC   : 
          case zRDE   : 
          case zRHL   : 
          case zRSP   : 
          case zRPC   : {
                           int tmp;
                           word *ptr;

                           if (KEY_SPACE == Key>>8) {
                             zone->BaseAddr++;
                             if (zone->BaseAddr == 3) zone->BaseAddr=0;
                             break;
                           };

                           switch (Key1&0xff) {
                            case zRAF: ptr=&dbg_REG.AF;break;
                            case zRBC: ptr=&dbg_REG.BC;break;
                            case zRDE: ptr=&dbg_REG.DE;break;
                            case zRHL: ptr=&dbg_REG.HL;break;
                            case zRSP: ptr=&dbg_REG.SP;break;
                            case zRPC: ptr=&dbg_REG.PC;break;
                           }
                           if (KEY_ENTER != Key>>8) simulate_keypress(Key);
                           tmp = HEXEDIT(*ptr,4,zone->Field[zone->Cursor].X,zone->BaseY+zone->Y);
                           if (tmp >= 0) { *ptr=tmp;}
                           break;
          }
          case zSTACK : {
                           int tmp,tmp1;
                           tmp=(zone->Cursor-2)*2;
                           tmp1=(Emulator_Read(dbg_REG.SP+tmp+1)<<8)+Emulator_Read(dbg_REG.SP+tmp+0);
                           Set_DASMAddr(tmp1);
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
