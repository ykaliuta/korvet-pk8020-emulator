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

#ifndef _EXTROM_H
#define _EXTROM_H

#define EXT_ROM_EMU_MODE        (0xc0)

#define EMU_STAGE1		        1
#define EMU_STAGE2_WAITCMD   	2
#define EMU_STAGE2_WRITE128  	3
#define EMU_STAGE2_WRSPPEDTEST  4
#define EMU_STAGE2_GETFILENAME  5
#define EMU_STAGE2_CREATE_KDI   6
#define EMU_STAGE2_GETFOLDER    7

#define EMU_API_FAIL            0
#define EMU_API_OK              1


extern int ext_rom_mode;
extern FILE* extrom_file;
extern char ext_rom_file_name[];	// имя файла с образом ROM
extern char ext_rom_emu_folder[];  	// папка которая прикидывается SDCARD эмулятора

int   ext_rom_addr_changed; 		// =1 while EXT ROM BOOT (rom loader only write in PPI3B,PPI3C)

void init_extrom(void);
byte ext_rom_read(unsigned char PPI3_B, unsigned char PPI3_C);

void ext_rom_api_write(byte Value);

extern char  control_flag;           // Флаг анализа сигнала Control: 0-игнорируется, 1-учитвается при файловых операциях

#endif