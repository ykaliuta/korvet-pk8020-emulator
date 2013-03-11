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
#ifndef __DBG__

#define __DBG__

#ifndef _TYPEDEF_
typedef unsigned char  byte;
typedef unsigned short word;
typedef unsigned       dword;
typedef signed char    offset;
#define _TYPEDEF_
#endif

#define KORVET_FONT0                     0        /* FONT */
#define KORVET_FONT1                     1        /* FONT */
#define PC_FONT                          2        /* FONT */
#define KORVET_FONT                      0        /* FONT */

#define TSCRX (1024/8) // 128
#define TSCRY (768/16) //  48

#define mDASM 1
#define mDUMP 2

#define bpCPU   1
#define bpRDMEM 2
#define bpWRMEM 4

#define C_BPCPU      0xc7
#define C_BPRW       0xa7
#define C_BPRD       0x97
#define C_BPWR       0xb7
#define C_Default    0x07
#define C_PC         0xe0
#define C_Border     0x02
#define C_Cursor     0x2f
#define C_Edit       0x0f
#define C_EditCursor 0x2F
#define C_Shadow     0x08
#define C_High       0x4f
#define C_ReadWrite  0x0a
#define C_NEQ        0x0b
 
#define zLABEL	1
#define zADDR	2
#define	zHEX	3
#define	zASC	4
#define zDASM   5
#define zBRK    6
#define zSKIP   7
#define zINT    8
#define zFLAG   9
#define zRAF    10
#define zRBC    11
#define zRDE    12
#define zRHL    13
#define zRSP    14
#define zRPC    15
#define zSTACK  16

#define zEDIT   0xff

#define KK_Shift 1
#define KK_Ctrl  2
#define KK_Alt   0

void tScrInit(void);
void tSetUpdate(int i);
void tFontSelect(int Fnt);
void tShowAll(void);
void tScreenPutChar(int ch, int attr, int col, int row);
void tScreenPutString(byte *str, int attr, int col, int row);
void tScreenClear(void);
void tScreenUpdateLine(void *buf, int row);
void tScreenRetrieve(byte *SCR);
void tScreenUpdate(byte *SCR);
void tFontSelect(int Fnt);
void tSetNewAttr(int attr,int len, int col, int row);
void DBG_Pallete_Active(void);
void DBG_Pallete_Pasive(void);

struct sFIELD {
   int X;
   int Len;
   int Type;
};

struct ZONE {
   int           Y;       // Y координата курсора в окне (0..YLine) относительно окна
   int           YLine;   // сколько строк в текущем окне (линий в поле)
   int           BaseY;   // строчка на экране

   int           BaseAddr;// Базовый адрес окна (т.е. адрес в левом окне)

   int           Cursor;  // номер поля Field
   int           MaxField;// сколько реально записей в след. поле.
   struct sFIELD Field[1+1+16+1+16+1]; // Label Addr 16hex stopfiled 16dmp (MAX variant)
};

struct CPUREG {
  word AF;
  word BC;
  word DE;
  word HL;
  word SP;
  word PC;
  int  Int;
};


int _REGS(int Key);
int _DASM(int Key);
int _DUMP(int Key);

word CPU_GetPC(void);
word CPU_GetSP(void); 
word CPU_GetHL(void); 
word CPU_GetDE(void); 
word CPU_GetBC(void); 
word CPU_GetAF(void); 
word CPU_GetI(void);  

void CPU_SetPC(word Val);
void CPU_SetSP(word Val);
void CPU_SetHL(word Val);
void CPU_SetDE(word Val);
void CPU_SetBC(word Val);
void CPU_SetAF(word Val);

#define MAXLABEL 1024
#define LABELLEN 16

typedef struct
{
 char Name[LABELLEN];
 unsigned int  Addr;
} _Label;

int AddLabel(word Addr,char *Name);
int DeleteLabel(word Addr);
_Label *FindAddrLabel(word Addr);
_Label *FindNameLabel(char *Name);

void WriteSYM(void);
void ReadSYM(void);

extern int scr_Second_Font;	// Флаг выбора второго знакогенератора ViReg:00000x00

#endif
