/*
 * AUTHOR: Sergey Erokhin                 esl@pisem.net,pk8020@gmail.com
 * (c) Alexander Demin                                       2000...2005
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
//#include "config.h"
#include "i8080.h"
#include "i8080dis.h"
//#include "pkemul.h"
#include "mem.h"
#include "korvet.h"

#if defined(DEBUG)
#include "debug.h"
#endif

extern int CPU_DBG[256];
extern int TotalCPU;


byte Emulator_Read(int Addres);


struct i8080 Cpu;
int Tstates;


int catch_invalid;
int IntREQ;
int IntDelay;


uns32 work32;
uns16 work16;
uns8 work8, work8_2;
int idx;
uns8 carry, add, sub;

int parityTbl[] = {
	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 
	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 
	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 
	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 
	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 
	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 
	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 
	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 
	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 
	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 
	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 
	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 
	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 
	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 
	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 
};

int halfCarryTbl[]    = { 0, 0, 1, 0, 1, 0, 1, 1 };
int subHalfCarryTbl[] = { 0, 1, 1, 1, 0, 0, 0, 1 };

int overflowTbl[]     = { 0, 1, 0, 0, 0, 0, 1, 0 };
int subOverflowTbl[]  = { 0, 0, 0, 1, 1, 0, 0, 0 };

void invalid_opcode()
{
	if (catch_invalid) {
		(PC)--;
#if defined(DEBUG)
		debug_window();
#endif
	}
}

void I8080_Init(void)
{
	Tstates = 0;
	C_FLAG = 0;
	S_FLAG = 0;
	Z_FLAG = 0;
	H_FLAG = 0;
	P_FLAG = 0;
	UN3_FLAG = 0;
	UN5_FLAG = 0;

	I8080_Reset();
}

void I8080_Reset(void)
{
	PC	= 0x0000;
	LAST_PC = 0x0000;
}

void I8080_StoreFlags(void)
{
	if (S_FLAG) F |= F_NEG;      else F &= ~F_NEG;
	if (Z_FLAG) F |= F_ZERO;     else F &= ~F_ZERO;
	if (H_FLAG) F |= F_HCARRY;   else F &= ~F_HCARRY;
	if (P_FLAG) F |= F_OVERFLOW; else F &= ~F_OVERFLOW;
	if (N_FLAG) F |= F_NADD;     else F &= ~F_NADD;
	if (C_FLAG) F |= F_CARRY;    else F &= ~F_CARRY;
}

void I8080_RetrieveFlags(void)
{
	S_FLAG = F & F_NEG      ? 1 : 0;
	Z_FLAG = F & F_ZERO     ? 1 : 0;
	H_FLAG = F & F_HCARRY   ? 1 : 0;
	P_FLAG = F & F_OVERFLOW ? 1 : 0;
	N_FLAG = F & F_NADD     ? 1 : 0;
	C_FLAG = F & F_CARRY    ? 1 : 0;
}


void CPU_Init(void) {I8080_Init();}
word CPU_GetPC(void) {return PC;}
word CPU_GetSP(void) {return SP;}
word CPU_GetHL(void) {return HL;}
word CPU_GetDE(void) {return DE;}
word CPU_GetBC(void) {return BC;}
word CPU_GetAF(void) {return AF;}
word CPU_GetI(void)  {return IFF;}

void CPU_SetPC(word Val) {PC=Val;}
void CPU_SetSP(word Val) {SP=Val;}
void CPU_SetHL(word Val) {HL=Val;}
void CPU_SetDE(word Val) {DE=Val;}
void CPU_SetBC(word Val) {BC=Val;}
void CPU_SetAF(word Val) {AF=Val;}



int RD_WORD(int addr ) { return Emulator_Read(addr)|Emulator_Read(addr+1)<<8;}
void WR_WORD(int addr, unsigned int word ) {Emulator_Write(addr,word&0xff);Emulator_Write(addr+1,(word&0xff00)>>8);};

byte RD_BYTE( int addr ) {
         return Emulator_Read(addr);};
void WR_BYTE(int addr, unsigned char ch ){Emulator_Write(addr,ch);};

unsigned char in_byte(byte port ){return Emulator_Read(port|port<<8);};
void out_byte(unsigned char port ){};


void DoINT()
{
  int a;
  IFF=0;
  IntREQ=0;
  if (Emulator_Read(PC) == 0x76) PC++;    // if Halt
  a=DoPIC();
  if (a<0) return;                      // Такого быть не должно !!!
  PUSH(PC);
  PC=a;
  Tstates+=11;
}


int CPU_Exec1step (void)
{
 unsigned char op;
 int i,j;

  Tstates = 0;
  op=RD_BYTE((int)PC++);
//  CPU_DBG[op]++;
//  TotalCPU++;
//  execute(op);
#include "i8080inc.c"

#ifdef zzSOUND
  j=Tstates;
  while (j--) DoTMR();
#endif

  if (IFF && IntREQ && !IntDelay) {
    DoINT();
  }

  IntREQ=CheckPIC();

  if (IntDelay) IntDelay=0;
  return Tstates;
}
