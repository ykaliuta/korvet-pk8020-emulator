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
#ifndef _KORVET_H
#define _KORVET_H

#define DBG
#define noEGA
#define SOUND
#define noWAV
#define noTRACETIMER

#define RAMSIZE         (64*1024)


#define OK 		0
#define ERROR		1
#define LANADDR         0x0f       // Адрес РМУ в сети ^0x0f

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

#ifndef _TYPEDEF_
typedef unsigned char  byte;
typedef unsigned short word;
typedef unsigned       dword;
typedef signed char    offset;
#define _TYPEDEF_
#endif


void Emulator_Write(int Addres,byte Value);
byte Emulator_Read(int Addres);

void FDC_Write_DRVREG(byte Value);
void FDC_Write(int Addr, byte Value);
byte FDC_Read(int Addr);

int  KEYBOARD_Read(int Addr);

void GZU_Write(int Addr,byte Value);
byte GZU_Read(int Addr);

void ACZU_Write(int Addr,byte Value);
byte ACZU_Read(int Addr);

void LUT_Write(byte Value);

byte PIC_Read(int Addr);
void PIC_Write(int Addr,byte Value);

void PPI1_Write(int Addr, byte Value);
byte PPI1_Read(int Addr);

void PPI2_Write(int Addr, byte Value);
byte PPI2_Read(int Addr);

void PPI3_Write(int Addr, byte Value);
byte PPI3_Read(int Addr);

void RS232_Write(int Addr, byte Value);
byte RS232_Read(int Addr);

void LAN_Write(int Addr,byte Value);
byte LAN_Read(int Addr);

void Timer_Write(int Addr, byte Value);
byte Timer_Read(int Addr);

#endif
