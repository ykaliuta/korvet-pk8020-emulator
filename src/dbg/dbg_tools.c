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

// на вход кнопка, указатель зоны и Y
//   отображаем эту зону на экране, и перемещаем при необходимости
//  на выходе 0 если перехватили кнопку 
//  или zEDIT | ZoneType если нажатая клавиша входит в область редактирования зоны


byte FieldCHR[20][255]={
                   [zLABEL] {KEY_ENTER,KEY_DEL},
                   [zADDR ] {KEY_ENTER,KEY_0,KEY_1,KEY_2,KEY_3,KEY_4,KEY_5,KEY_6,KEY_7,KEY_8,KEY_9,KEY_A,KEY_B,KEY_C,KEY_D,KEY_E,KEY_F},
                   [zHEX  ] {KEY_ENTER,KEY_0,KEY_1,KEY_2,KEY_3,KEY_4,KEY_5,KEY_6,KEY_7,KEY_8,KEY_9,KEY_A,KEY_B,KEY_C,KEY_D,KEY_E,KEY_F},
                   [zASC  ] {KEY_ENTER,KEY_0,KEY_1,KEY_2,KEY_3,KEY_4,KEY_5,KEY_6,KEY_7,KEY_8,KEY_9,KEY_Q,KEY_W,KEY_E,KEY_R,KEY_T,KEY_Y,KEY_U,KEY_I,KEY_O,KEY_P,KEY_A,KEY_S,KEY_D,KEY_G,KEY_F,KEY_G,KEY_H,KEY_J,KEY_K,KEY_L,KEY_Z,KEY_X,KEY_C,KEY_V,KEY_B,KEY_N,KEY_M,KEY_TILDE,KEY_MINUS,KEY_EQUALS,KEY_OPENBRACE,KEY_CLOSEBRACE,KEY_COLON,KEY_QUOTE,KEY_BACKSLASH,KEY_BACKSLASH2,KEY_COMMA,KEY_STOP,KEY_SLASH,KEY_SPACE},
                   [zDASM ] {KEY_ENTER},
                   [zFLAG ] {KEY_SPACE,KEY_ENTER},
                   [zRAF  ] {KEY_ENTER,KEY_0,KEY_1,KEY_2,KEY_3,KEY_4,KEY_5,KEY_6,KEY_7,KEY_8,KEY_9,KEY_A,KEY_B,KEY_C,KEY_D,KEY_E,KEY_F},
                   [zRBC  ] {KEY_SPACE,KEY_ENTER,KEY_0,KEY_1,KEY_2,KEY_3,KEY_4,KEY_5,KEY_6,KEY_7,KEY_8,KEY_9,KEY_A,KEY_B,KEY_C,KEY_D,KEY_E,KEY_F},
                   [zRDE  ] {KEY_SPACE,KEY_ENTER,KEY_0,KEY_1,KEY_2,KEY_3,KEY_4,KEY_5,KEY_6,KEY_7,KEY_8,KEY_9,KEY_A,KEY_B,KEY_C,KEY_D,KEY_E,KEY_F},
                   [zRHL  ] {KEY_SPACE,KEY_ENTER,KEY_0,KEY_1,KEY_2,KEY_3,KEY_4,KEY_5,KEY_6,KEY_7,KEY_8,KEY_9,KEY_A,KEY_B,KEY_C,KEY_D,KEY_E,KEY_F},
                   [zRSP  ] {KEY_ENTER,KEY_0,KEY_1,KEY_2,KEY_3,KEY_4,KEY_5,KEY_6,KEY_7,KEY_8,KEY_9,KEY_A,KEY_B,KEY_C,KEY_D,KEY_E,KEY_F},
                   [zRPC  ] {KEY_ENTER,KEY_0,KEY_1,KEY_2,KEY_3,KEY_4,KEY_5,KEY_6,KEY_7,KEY_8,KEY_9,KEY_A,KEY_B,KEY_C,KEY_D,KEY_E,KEY_F},
                   [zSTACK] {KEY_ENTER},
                   [zBRK  ] {0xff},
};

int DBG_Walker(int Key,struct ZONE *Z,int y) {

  char BUF[64];

  int x;
  int RetKey = Key;
  char *CC;

  while (Z->Field[Z->Cursor].Type == zSKIP) Z->Cursor--;

  switch (Key>>8) {
     case KEY_LEFT : {
                        if (Z->Cursor != 0) Z->Cursor--;
                        if (Z->Field[Z->Cursor].Type == zBRK) Z->Cursor++;
                        while (Z->Field[Z->Cursor].Type == zSKIP) Z->Cursor--;
                        RetKey=0;
                        break;
                     }
     case KEY_RIGHT: {
                        if (Z->Cursor != Z->MaxField) Z->Cursor++;
                        if (Z->Field[Z->Cursor].Type == zBRK) Z->Cursor--;
                        while (Z->Field[Z->Cursor].Type == zSKIP) Z->Cursor++;
                        RetKey=0;
                        break;
                     }
     default:        {
                        if ((CC=strchr(FieldCHR[Z->Field[Z->Cursor].Type],Key>>8 )) != NULL) {
                           RetKey=(zEDIT<<8) | Z->Field[Z->Cursor].Type ;
                        }
                        break;
                     }
  }

  x=Z->Cursor;
  tSetNewAttr(C_Cursor,Z->Field[x].Len,Z->Field[x].X,y);

//  sprintf(BUF,"%04x",RetKey);
//  tScreenPutString(BUF,C_Default,6,33);

  return RetKey;
}

byte GetVisibleChar(byte Chr) {
  if (Chr<0x20) return '.';
  return Chr;
}

int GetBreakColor(word Addr,int Type) {
    int Flags=RD_BreakPoint(Addr);
    int Color=C_Default;

    switch (Type) {
      case mDUMP: {
//                  if (Flags & (bpCPU)   ) Color=C_BPCPU;
                  if (Flags & (bpRDMEM) ) Color=C_BPRD;
                  if (Flags & (bpWRMEM) ) Color=C_BPWR;
                  if ((Flags & (bpRDMEM | bpWRMEM)) == (bpRDMEM | bpWRMEM) ) Color=C_BPRW;
                  break;
                 };
      case mDASM: {
                  if (Flags & (bpCPU)   ) Color=C_BPCPU;
                  break;
                 };
    };
    return Color;
}

int HEXEDIT(int Value,int Len,int x,int y) {
// if ESC pressed return -Value else new value
// if Left on first digit or Right on last return OK

  int pos=0;
  char buf[8];
  int work;
  int flEdit=1;
  int Key,Key1;
  int mask;
  int Ok=1;

  
  work=(4 == Len)?Value&0xffff:Value&0xff; 

  while (flEdit) {
     sprintf(buf,(4 == Len)?"%04X":"%02X",work);
     tScreenPutString(buf,C_Edit,x,y);
     tSetNewAttr(C_EditCursor,1,x+pos,y);

     Key=readkey();
     Key1=Key>>8;
//     if ((Key1>=KEY_0 && Key1<=KEY_9)) { mask=(0xF000>>4*pos)^0xffff; work=(work & mask) | (Key1-KEY_0+0x00)<<4*(Len-1-pos);Key=KEY_RIGHT<<8;}
//     if ((Key1>=KEY_A && Key1<=KEY_F)) { mask=(0xF000>>4*pos)^0xffff; work=(work & mask) | (Key1-KEY_A+0x0A)<<4*(Len-1-pos);Key=KEY_RIGHT<<8;}
     if ((Key1>=KEY_0 && Key1<=KEY_9)) { mask=(((4 == Len)?0xF000:0xF0)>>4*pos)^0xffff; work=(work & mask) | (Key1-KEY_0+0x00)<<4*(Len-1-pos);Key=KEY_RIGHT<<8;}
     if ((Key1>=KEY_A && Key1<=KEY_F)) { mask=(((4 == Len)?0xF000:0xF0)>>4*pos)^0xffff; work=(work & mask) | (Key1-KEY_A+0x0A)<<4*(Len-1-pos);Key=KEY_RIGHT<<8;}

     switch (Key>>8) {
        case KEY_LEFT : {if (pos)       pos--;else {flEdit=0;Ok=0;};break;}
        case KEY_RIGHT: {if (pos<Len-1) pos++;else flEdit=0;break;}
        case KEY_ENTER: {flEdit=0;break;}
        case KEY_ESC  : {flEdit=0;Ok=0;break;}
     }
  }
  if (Ok) return work;
  return -32000;
}

int LineEdit(char *src,int maxlen,int x,int y) {
  int  pos=0;
  int  Len;
  byte buf[128];
  byte tmps[128];
  int  work;
  int  flEdit=1;
  int  Key,Key1;
  int  Ok=1;
  char *tmp;
  int  i;

  strcpy(buf,src);

  Len=strlen(src);

  while (flEdit) {

     strcpy(tmps,buf);
     strcat(tmps,"_______________________________");
     tmps[maxlen]='\0';
     tScreenPutString(tmps,C_Edit,x,y);
     tSetNewAttr(C_EditCursor,1,x+pos,y);

     Key=readkey();
     Key1=Key&0xff;


     if (isprint(Key1)) {
        if (Len < maxlen) {
          for (i=maxlen;i>pos;i--) {buf[i]=buf[i-1];}
          buf[pos]=Key1;
          Key=KEY_RIGHT<<8;
          Len++;
        }
     }

     switch (Key>>8) {
        case KEY_BACKSPACE  : 
        case KEY_DEL  : {
                         if (Len) {
                           if ( KEY_BACKSPACE == (Key>>8) ) {
                             if (pos) pos--;
                             else break;
                           };

                           for (i=pos;i<maxlen;i++) buf[i]=buf[i+1];
                           buf[maxlen+1]=0;
                           buf[maxlen]=' ';
                           Len--;
                           break;
                         }
                        }
        case KEY_LEFT : {if (pos)       pos--;break;}
        case KEY_RIGHT: {if (pos<Len)   pos++;break;}
        case KEY_ENTER: {flEdit=0;break;}
        case KEY_ESC  : {flEdit=0;Ok=0;break;}
     }



  }
  if (Ok) {
//    tmp=buf+strlen(buf);
//    while (*tmp == ' ') *tmp='\0';
    strcpy(src,buf);
    return 1;
  }
  return 0;
}
