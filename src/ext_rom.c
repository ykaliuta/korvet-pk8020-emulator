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

// Поддержка загрузки с внешнего ROM

int ext_rom_mode=0;
FILE* extrom_file;       		
char ext_rom_file_name[1024]="not set";	// имя файла с образом ROM

void init_extrom(void) {
  if (ext_rom_mode) {
    printf("\nExtROM mode with file: %s\n",ext_rom_file_name);

    extrom_file=fopen(ext_rom_file_name,"rb");
    if (extrom_file == 0) {
      printf("\n - Ошибка открытия файла образа ROM - %s\n",ext_rom_file_name);
      printf("ExtRom Boot - режим отключен");

      ext_rom_mode=0;
    }
  }
}

byte ext_rom_read(unsigned char PPI3_B, unsigned char PPI3_C) {
	unsigned char value=0;
	unsigned int ext_rom_addr=(PPI3_C<<8)+PPI3_B;
	if (ext_rom_mode) {
		fseek(extrom_file,ext_rom_addr,SEEK_SET);
		value=fgetc(extrom_file);
		// printf("EXTROM: %04x=%02x\n",ext_rom_addr,value);
	}
	return value;
}
