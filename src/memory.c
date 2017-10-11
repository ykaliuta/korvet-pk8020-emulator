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

#include "korvet.h"

#ifdef DBG
 #include "dbg/dbg.h"
#endif

//#include "i8080.h"


#ifdef DBG
extern word dbg_HERE;  // Адрес останова при пошаговом выполнении
extern int  dbg_TRACE;  // Флаг необходимости вызова отладчика
extern int  dbg_Cause;
extern int  dbg_CauseAddr;
#endif

char RomFileName[1024]="data/rom.rom";
char MapperFileName[1024]="data/mapper.mem";
#define noDEBUG_MEMORY

#define ROMSIZE         (24*1024)

// Определения для маппера (как в файле обозначены типы памяти)
#define M_RAM            0
#define M_ROM0           1      // Реально ROM0 0000-1fff
#define M_ROM1           2      // Реально ROM1 2000-3fff
#define M_ROM2           3      // Реально ROM2 4000-5fff
#define M_KEYBOARD       4
#define M_PORTBASE       5      // Порты перифрийных БИС
#define M_REGBASE        6      // Системные регистры
#define M_ACZU           7
#define M_GZU            8


extern int DBG_Trace ;
extern int TraceCause;
extern int CauseAddr;
extern int InDBG;

extern unsigned int NCREG;

int     SYSREG;                 // Значение регистра конфигурации памяти
                                // Прямое значение (0-31)
                                // Перед записью в эту переменную (>>2)&0x1f

byte    MemMapper[32][256];     // Таблица типов памяти при доступе

byte    RAM[RAMSIZE];           // RAM эмулятора
byte    ROM[ROMSIZE];           // ROM эмулятора

byte    BreakPoint[RAMSIZE];    // DBG:: Флаги ловушек

// ================================================================================= RAM part

// --------------------------------------------------------------------------------- SYSREG Part
// Прочитать из файла таблички SYSREGa
int Mapper_Init(void) {
 int i;
 int retflag=OK;
 FILE *MFILE;
 if ((MFILE=fopen(MapperFileName,"rb")) == NULL) retflag=ERROR;
 for (i=0;i<32;i++) {
   if (fread(MemMapper[i],1,256,MFILE) != 256) retflag=ERROR;
 }
 fclose(MFILE);
 SYSREG=0;
 if (retflag == ERROR) {
  printf("ERROR: can't read memorymap file '%s'\n",MapperFileName);
 }
 return retflag;
}
// ================================================================================= SYSREG part

// --------------------------------------------------------------------------------- ROM part
// Прочитать из файла Образ ПЗУ
int ROM_Init(char *RomFileName) {
 int i;
 int retflag=OK;
 printf("ROM file : %s\n",RomFileName);
 FILE *MFILE;
 if ((MFILE=fopen(RomFileName,"rb")) == NULL) retflag=ERROR;
 if ((i=fread(ROM,1,ROMSIZE,MFILE)) != ROMSIZE) {  retflag=ERROR; }
 if (retflag == ERROR) {
  printf("WARNING: read rom file '%s' : %d bytes long\n",RomFileName,i);
 }
 fclose(MFILE);
 return retflag;
}
// ================================================================================= ROM part

// --------------------------------------------------------------------------------- RAM part
// Обнулить память
int RAM_Init(void) {
 int i;
 for (i=0;i<RAMSIZE;i++) RAM[i]=0;
 return OK;
}
// ================================================================================= RAM part

// --------------------------------------------------------------------------------- Emulator_Write
// Функция Записи в память эмулятора
void Emulator_Write(int Addres,byte Value)
{
  int RamType;
#ifdef DBG
  if (!InDBG && (BreakPoint[Addres] & bpWRMEM) )  {dbg_TRACE=1;dbg_Cause=3;dbg_CauseAddr=Addres;} // SetTrace
#endif


  // запись в память выполняется 80% времени
  if ( 0 == (RamType=MemMapper[SYSREG][(Addres&0xff00)>>8])) {
    RAM[Addres&0xffff]=Value;
  } else {
  switch (RamType) {
    case  M_RAM     :
    case  M_ROM0    :               // При записи в область ROM пишем в RAM
    case  M_ROM1    :               // При записи в область ROM пишем в RAM
    case  M_ROM2    :               // При записи в область ROM пишем в RAM
    case  M_KEYBOARD:               // При записи в область KEY пишем в RAM
                      {RAM[Addres&0xffff]=Value;break;}
    case  M_REGBASE : {
           // Регистры адресуются по отедльными битами шины адреса
           // соотвествуюший бит должен быть равен 0
           if ((Addres & (1<<2)) == 0) { LUT_Write(Value);       }; /*A2, xxFB*/
           if ((Addres & (1<<6)) == 0) { NCREG =Value;           }; /*A6, xxBF*/
           if ((Addres & (1<<7)) == 0) { SYSREG=(Value>>2)&0x1f; }; /*A7, xx7F*/
           break;
         }
    case M_PORTBASE : {
           // Регистры БИС адресуются дешифратором на вход которого поданы
           // адресные шины А3,A4,A5 - 8 устройств
           switch (Addres & 0x38) {
             case 0<<3:	Timer_Write(Addres,Value);break; //xx00..03
             case 1<<3:	PPI3_Write (Addres,Value);break; //xx08..0B
             case 2<<3:	RS232_Write(Addres,Value);break; //xx10..11
             case 3<<3:	FDC_Write  (Addres,Value);break; //xx18..1B
             case 4<<3:	LAN_Write  (Addres,Value);break; //xx20..21
             case 5<<3:	PIC_Write  (Addres,Value);break; //xx28..29
             case 6<<3:	PPI2_Write (Addres,Value);break; //xx30..33
             case 7<<3:	PPI1_Write (Addres,Value);break; //xx39..3B
           }
           break;
         }
    case  M_GZU     : {GZU_Write (Addres,Value);break;}
    case  M_ACZU    : {ACZU_Write(Addres,Value);break;}
  }
  }
}

// ================================================================================= Emulator_Write

// --------------------------------------------------------------------------------- Emulator_Read
// Функция Записи в память эмулятора
byte Emulator_Read(int Addres)
{
  byte Value;
  int RamType;

#ifdef DBG
  if (!InDBG && (BreakPoint[Addres] & bpRDMEM) ) {dbg_TRACE=1;dbg_Cause=2;dbg_CauseAddr=Addres;} // SetTrace
#endif
  // чтение из памяти - 85% времени
  if ( 0 == (RamType=MemMapper[SYSREG][(Addres&0xff00)>>8])) {
    Value=RAM[Addres&0xffff];
  } else {
  switch (RamType) {
    case  M_RAM     : {Value=RAM[Addres&0xffff];break;}
    case  M_ROM0    :
    case  M_ROM1    :
    case  M_ROM2    : {Value=ROM[Addres&0xffff];break;}
    case  M_KEYBOARD: {Value=KEYBOARD_Read(Addres,0);break;}
    case  M_REGBASE : {
           // при чтении по адресам Регистров читается содержимое памяти  ???
           // по соответствующим адресам.
          Value=RAM[Addres&0xffff];
          break;
         }
    case M_PORTBASE : {
           // Регистры БИС адресуются дешифратором на вход которого поданы
           // адресные шины А3,A4,A5 - 8 устройств
           switch (Addres & 0x38) {
             case 0<<3:	Value=Timer_Read(Addres);break;	//xx00..03
             case 1<<3:	Value=PPI3_Read (Addres);break; //xx08..0B
             case 2<<3:	Value=RS232_Read(Addres);break; //xx10..11
             case 3<<3:	Value=FDC_Read  (Addres);break; //xx18..1B
             case 4<<3:	Value=LAN_Read  (Addres);break; //xx20..21
             case 5<<3:	Value=PIC_Read  (Addres);break; //xx28..29
             case 6<<3:	Value=PPI2_Read (Addres);break; //xx30..33
             case 7<<3:	Value=PPI1_Read (Addres);break; //xx38..3B
           }
           break;
         }
    case  M_GZU     : {Value=GZU_Read(Addres);break;}
    case  M_ACZU    : {Value=ACZU_Read(Addres);break;}
  }
  }
  return Value;
}
// ================================================================================= Emulator_Read


void Memory_Init(void)
{
 Mapper_Init();
 RAM_Init();
 ROM_Init(RomFileName);
}

byte RD_BreakPoint(int Addr)           { return BreakPoint[Addr&0xffff];};
byte WR_BreakPoint(int Addr,byte Value){ BreakPoint[Addr&0xffff]=Value;};

