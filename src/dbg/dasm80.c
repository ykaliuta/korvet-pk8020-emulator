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
#include <string.h>
//#include "d_label.h"
#include "dbg.h"
#include "d8080.h"
//#include "../i8080.h"

extern struct CPUREG dbg_REG;

#define F_CARRY                 0x01
#define F_NADD                  0x02
#define F_PARITY                0x04
#define F_OVERFLOW              F_PARITY
#define F_HCARRY                0x10
#define F_ZERO                  0x40
#define F_NEG                   0x80


#define check_flag(idx, flag, value) 	\
	case idx:                       \
		cond = (dbg_REG.AF&flag)==value;	\
		break

#define check_flags()				  \
	switch(cond_dir) {			  \
		check_flag(0, F_ZERO  , 0);	  \
		check_flag(1, F_ZERO  , F_ZERO);  \
		check_flag(2, F_CARRY , 0);	  \
		check_flag(3, F_CARRY , F_CARRY); \
		check_flag(4, F_PARITY, 0);	  \
		check_flag(5, F_PARITY, F_PARITY);\
		check_flag(6, F_NEG   , 0);	  \
		check_flag(7, F_NEG   , F_NEG);	  \
	}


/** DAsm() ****************************************************/
/** This function will disassemble a single command and      **/
/** return the number of bytes disassembled.                 **/
/**************************************************************/
int DASM(char *S,word A)
{
  char H[128];
  word B;
  word W;
  int  C;
  int  CC;
_Label *L;
  int  cond;
  int  cond_dir;


  B=A;

  C=Emulator_Read(B++);
  strcpy(S,Mnemo80[C]);
  strcat(S,Oper80 [C]);

  switch (Toper80[C]) {
    case 1: {
          C=Emulator_Read(B++);
          sprintf(H,"%02x  ;'%c'",C,GetVisibleChar(C));
          strcat(S,H);
          break;
    }
    case 2: {
         W=Emulator_Read(B++)+256*Emulator_Read(B++);
         if (L=FindAddrLabel(W)) sprintf(H,"%s:%04X",L->Name,W);  //ESL
         else sprintf(H,"%04X",W);                       //ESL
//         sprintf(H,"%04x",W);                       //ESL
         strcat(S,H);
         break;
   }
    case 3: {
          sprintf(H,"%02x",C);
          strcat(S,H);
          break;
    }
  }
//  strcat(S,"                       ");
//  S[28]=0;
  if (dbg_REG.PC==A) {

    cond_dir=(C>>3)&3;
    check_flags();

    CC= C & 0xe7;

    if ( (CC==0xC0) || (CC==0xC2) || (CC==0xC4) ) {
      strcat(S, cond ? (CC==0xC0 ? "  ;*" : (W<dbg_REG.PC ? "  ;\x18" : "  ;\x19" ) ): "  ;");
    } else if ( (C==0xC3) || (C==0xCD) ) {
      strcat(S, W<dbg_REG.PC ? "  ;\x18" : "  ;\x19" );
    }

  }

  B-=A;
  return (B);
}

int GetCmdLen(word Addr) {
  char BUF[256];
  return DASM(BUF,Addr);
}
