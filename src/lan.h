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

extern unsigned int LAN_Addr; // Адрес РМУ в сети 00-0f
extern char 		LAN_ttdev[1000];     // последовательный порт сети
extern char 		LAN_logfile[1000];  // Имя файла для лога

#define LAN_MODE_RMP 1
#define LAN_MODE_RMU 2
#define LAN_PTX_FILE ".lan_ptx_path.tmp"

void LAN_Write(int Addr,byte Value);
byte LAN_Read(int Addr);
void LAN_Init(void);      // открытие и настройка последовательного порта
void LAN_poll(void);      // формирование прерывание при наличии байта
void LAN_destroy(void);

