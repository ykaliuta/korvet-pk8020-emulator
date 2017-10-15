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

#include "verbose.h"

#include <allegro.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h> /* for mode definitions */
#include <ctype.h>

#include <sys/poll.h>
#include <unistd.h>
#include <signal.h>
#include <asm-generic/ioctls.h>
#include <termios.h>

#ifndef _KORVET_H
#define _KORVET_H

#ifdef __GNUC__
/*code for GNU C compiler */
#elif _MSC_VER
/*usually has the version number in _MSC_VER*/
/*code specific to MSVC compiler*/
#elif __BORLANDC__
/*code specific to borland compilers*/
#elif __MINGW32__
/*code specific to mingw compilers*/
#endif

#define RAMSIZE         (64*1024)

#define PLANESIZE 	16384


#define OK 		0
#define ERROR		1

#define CPU_CLK         2500000    // частота ЦПУ в герцах
#define ALL_TAKT        50000      // кол-во тактов в одном VBLANK
#define VBLANK_TAKT     8170       // кол-во тактов обрптного хода луча

#define KBD_QWERTY	0
#define KBD_JCUKEN	1
#define KBD_AUTO	2

#define SCR_EMULATOR	1
#define SCR_DBG		2

#define SOUNDFREQ (44100/2)
#define AUDIO_BUFFER_SIZE (SOUNDFREQ/50)

#define MAXBUF 50000 	//audio buffer size

#define LUT_BASE_COLOR 0x80

#ifndef _TYPEDEF_
typedef unsigned char  byte;
typedef unsigned short word;
typedef unsigned       dword;
typedef signed char    offset;
#define _TYPEDEF_
#endif

void CPU_Init(void);
int CPU_Exec1step (void);

void Memory_Init(void);
void Emulator_Write(int Addres,byte Value);
byte Emulator_Read(int Addres);
byte RD_BreakPoint(int Addr);
void WR_BreakPoint(int Addr,byte Value);

void FDC_Init(void);
void FDC_Reset(void);
void FDC_Write_DRVREG(byte Value);
void FDC_Write(int Addr, byte Value);
byte FDC_Read(int Addr);

void KBD_Init(void);
int  KEYBOARD_Read(int Addr,int InternalMode);

void GZU_Init(void);
void GZU_Write(int Addr,byte Value);
byte GZU_Read(int Addr);

int ACZU_Init(void);
void ACZU_Write(int Addr,byte Value);
byte ACZU_Read(int Addr);

void SCREEN_Init(void);
void SCREEN_SetGraphics(int ScrMode);
int SCREEN_SetText(void);
void SCREEN_ShowScreen(void);

void LUT_Init(void);
void LUT_Write(byte Value);
void LUT_Update(int BWFlag);
void PutLED_Lut(int x,int y,int i,int c);

void PIC_init(void);
byte PIC_Read(int Addr);
void PIC_Write(int Addr,byte Value);

void PIC_IntRequest(int IntNum);             // Запрос на прерывание.
int DoPIC(void);
int CheckPIC(void);
void PIC_IntReset(int IntNum);
void ShowPICdbg(void);

void PPI1_Write(int Addr, byte Value);
byte PPI1_Read(int Addr);

void PPI2_Write(int Addr, byte Value);
byte PPI2_Read(int Addr);

void PPI3_Write(int Addr, byte Value);
byte PPI3_Read(int Addr);

void PPI_Init(void);
void ShowPPIdbg(void);

void Serial_Init(void);
void RS232_Write(int Addr, byte Value);
byte RS232_Read(int Addr);
void AddSerialQueue(byte b);

void InitTMR(void);
void InitTimer(void);
void DestroyTimer(void);
void Timer_Write(int Addr, byte Value);
byte Timer_Read(int Addr);
void MakeSound(void);
int DoTimer(void);
void Timer50HzTick(void);
#ifdef TRACETIMER
int TimerTrace(const char *fmt, ...);
#else
#define TimerTrace(...)
#endif

void InitOSD(void);
void DestroyOSD(void);
void ResetOSD(void);
void update_osd(void);
void UpdateKBD_OSD(int Addr);

int InitPrinter(void);
void DestroyPrinter(void);
int GetPrinterStatus(void);
void SetPrinterStrobe(int Value);

void Init_Joystick(void);
int Read_Joystick(void);

void update_rus_lat(void);
void Debug_LUT(int Debug_Key);
void Write_Dump(void);
void MUTE_BUF(void);
void ReadConfig(void);
void parse_command_line(int argc,char **argv);
void check_missing_images(void);
void PrintDecor(void);

void CheckROM(void);
void CheckCCP(void);
void CheckComEXEC(void);

void dbg_INIT(void);
void doDBG(void);

void LAN_Init(void);
void LAN_Write(int Addr,byte Value);
byte LAN_Read(int Addr);

void GUI(void);
void AddPC(word pc);
void ChkMouse(void);
int GetFileAttr(char *t);
int ChDir(char *t);
byte FDC_Read_DRVREG(void);
byte ext_rom_api_read(void);

#endif

