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
#include <stdio.h>
#include "dbg.h"


#define x 29 
#define y 10 

byte SymFileName[1024]="";

void UpdateSymRW(int endfl) {
  byte buf[128];
  int tmp;

  tSetUpdate(0);

  tScreenPutString("______________________",C_Default,x+ 8,y+1);
  tScreenPutString(SymFileName,C_Default,x+ 8,y+1);

  tSetUpdate(1);
}

int SymDialog(int mode) { //1 - write

  char Title[2][16]={"Ä SYM  Read"," SYM  Write"};
  byte buf[128];
  int tmp;
  int i;
  FILE *F;

  tSetUpdate(0);

  tScreenPutString("ÚÄÄÄÄÄÄÄÄ            ÄÄÄÄÄÄÄÄÄÄ¿",C_ReadWrite,x,y+0);
  tScreenPutString("³ Name: ______________________ ³",C_ReadWrite,x,y+1);
  tScreenPutString("ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ",C_ReadWrite,x,y+2);

  tScreenPutString(Title[mode],C_ReadWrite,x+9,y+0);

  UpdateSymRW(mode);
  tSetUpdate(1);

  tmp=LineEdit(SymFileName,22,x+8,y+1); UpdateSymRW(mode);
  if (!tmp) return 0;
  if (strlen(SymFileName) == 0) return 0 ;

  return 1;
}

void WriteSYM(void) {
  int i;
  FILE *F;
  _Label *L;
  char BUF[1024];
  int Addr;

  if (!SymDialog(1)) return;

  if (strchr(SymFileName,'.') == NULL) {strcat(SymFileName,".sym");}

  F=fopen(SymFileName,"wt");
  for (Addr=0;Addr<0x10000;Addr++) {
    L=FindAddrLabel(Addr);
    if (L) {
      fprintf(F,"%04X %s\n",L->Addr,L->Name);
    }
  }
  fclose(F);
}

void ReadSYM(void) {
  int i;
  int b;
  FILE *F;
  char *tmp;
  char *tmp1;
  char buf[1024];
  int  Addr;
  char Label[128];

  if (!SymDialog(0)) return;

  F=fopen(SymFileName,"rt");
  if (F == NULL)  return;

  while(fgets(buf,512,F)) {

    tmp=buf;
    while(*tmp) {
     if (isxdigit(*(tmp+0)) && isxdigit(*(tmp+1)) && isxdigit(*(tmp+2)) && isxdigit(*(tmp+3)) && (*(tmp+4) == ' ')) {
       *(tmp+4)=0;
       sscanf(tmp,"%04x",&Addr);
       tmp+=5;
       while (*tmp && (*tmp == ' ')) tmp++;
       tmp1=Label;
       while (*tmp && !isspace(*tmp)) *tmp1++=*tmp++;
       *tmp1=0;
       strupr(Label);
//       printf("Addr = %04x -> [%s]\n",Addr,Label);
       DeleteLabel(Addr);
       AddLabel(Addr,Label);
     }
     if (*tmp) tmp++;
    }
  }

  fclose(F);
}
