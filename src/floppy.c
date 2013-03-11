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

extern screen,font;

#include "vg.h"
int InUseFDD[4];

struct DatVG VG;
char   Disks[4][1024]
                    ={"disk/disk.kdi",
                      "disk/disk2.kdi",
                      "disk/disk3.kdi",
                      "disk/disk4.kdi",
                     };

//           0,1,2,3,4,5,6,7,8,9,a,b,c,d,e,f
  int drv[]={0,0,1,1,2,1,2,1,3,0,1,0,1,0,1,0};

char   DskPth[512];
char   StPth[512];

int DRVREG;

//#include "vg.c"

int ChDir(char *t)
{ return 0;
}

int GetFileAttr(char *t)
{ return 0;
}

/*
00h (DC_READSTATUS)   - чтение состояния ВГ (вызов при чтении из порта 18h).
01h (DC_WRITECOMMAND) - запись команды ВГ (вызов при записи в порт 18h).
02h (DC_READTRACK)    - чтение дорожки (вызов при чтении из порта 19h).
03h (DC_WRITETRACK)   - запись дорожки (вызов при записи в порт 19h).
04h (DC_READSECTOR)   - чтение сектора (вызов при чтении из порта 1Ah).
05h (DC_WRITESECTOR)  - запись сектора (вызов при записи в порт 1Ah).
06h (DC_READDATA)     - чтение данных (вызов при чтении из порта 1Bh).
07h (DC_WRITEDATA)    - запись данных (вызов при записи в порт 1Bh).
08h (DC_READDRQINTRQ) - чтение сигналов Intrq и Drq 
                        (вызов при чтении из порта 39h).
09h (DC_WRITESYSTEM)  - запись системного регистра контроллера дисководов 
                        (вызов при записи в порт 39h).
0Ah (DC_INIT)         - инициализация диска (вызов при включении эмуляции и 
                        смене диска). При вызове этой функции текущая 
                        директория - директория с файлами образа диска.
0Bh (DC_UNINIT)       - снятие диска (вызов при выключении эмуляции и смене диска).
0Ch (DC_RESET)        - Reset контроллера дисковода.
*/



byte FDC_Read_DRVREG(void) {
//  printf("%04x: DRV_R: %02x\n",Z80_GetPC(),DRVREG);
  return DRVREG;
}


void FDC_Write_DRVREG(byte Value) {

  DRVREG=Value;
  VG.DataIO=(Value & 0xf0) | drv[Value & 0x0f];
  VG.OperIO=9;
  DskVG();
//  printf("%04x: DRV_W: %02x\n",Z80_GetPC(),Value);
}

void FDC_Write(int Addr, byte Value) {
  VG.DataIO=Value;
  switch (Addr & 0x03) {
    case 0:    VG.OperIO=1;break;
    case 1:    VG.OperIO=3;break;
    case 2:    VG.OperIO=5;break;
    case 3:    VG.OperIO=7;break;
  }
//  textprintf(screen,font,400,410,15,"FDC_W: %04x=%02x",Addr,Value);
//  printf("%04x: FDC_W: %04x=%02x\n",Z80_GetPC(),Addr,Value);
  InUseFDD[drv[DRVREG&0x0f]]=50*1.5;
  DskVG();
}

byte FDC_Read(int Addr) {
  switch (Addr & 0x03) {
    case 0:    VG.OperIO=0;break;
    case 1:    VG.OperIO=2;break;
    case 2:    VG.OperIO=4;break;
    case 3:    VG.OperIO=6;break;
  }
  DskVG();
//  textprintf(screen,font,400,400,15,"FDC_R: %04x=%02x",Addr,VG.DataIO);
//  printf("%04x: FDC_R: %04x=%02x\n",Z80_GetPC(),Addr,VG.DataIO);
  return VG.DataIO;
}


void FDC_Init(void) {
  VG.OperIO=0x0a;
  DskVG();
  InUseFDD[0]=InUseFDD[1]=InUseFDD[2]=InUseFDD[3]=0;
}

void FDC_Reset(void) {
  VG.OperIO=0x0c;
  DskVG();
  VG.OperIO=0x0a;
  DskVG();
}


/*
void FDC_Read(int Addr, byte Value)
{
  textprintf(screen,font,400,400,15,"FDC_W: %04x=%02x",Addr,Value);
  return ;
}

byte FDC_Write(int Addr)
{
  textprintf(screen,font,400,410,15,"FDC_R: %04x",Addr);
  return 0;
}
*/

#ifdef nDBG
void ShowFDCdbg(void)
{

 int i=0;

 int x=800;
 int y=490+100;

 textprintf(screen,font,x+28,y-20,0x20+0x0f,"FDC");

 textprintf(screen,font,x,y+16*i++,0x20+0x07,"DataIO: %02x",VG.DataIO);
 textprintf(screen,font,x,y+16*i++,0x20+0x07,"OperIO: %02x",VG.OperIO);
 textprintf(screen,font,x,y+16*i++,0x20+0x07,"R_Stat: %02x",VG.RegStatus);
 textprintf(screen,font,x,y+16*i++,0x20+0x07,"R_CMD : %02x",VG.RegCom);
 textprintf(screen,font,x,y+16*i++,0x20+0x07,"R_TRK : %02x",VG.RegTrack);
 textprintf(screen,font,x,y+16*i++,0x20+0x07,"R_Sec : %02x",VG.RegSect);
 textprintf(screen,font,x,y+16*i++,0x20+0x07,"R_Data: %02x",VG.RegData);
 textprintf(screen,font,x,y+16*i++,0x20+0x07,"System: %02x",VG.System);
 textprintf(screen,font,x,y+16*i++,0x20+0x07,"StpDir: %d"  ,VG.StepDirect);
 textprintf(screen,font,x,y+16*i++,0x20+0x07,"a:%02x b:%02x"  ,VG.TrackReal[0],VG.TrackReal[1]);
 textprintf(screen,font,x,y+16*i++,0x20+0x07,"c:%02x d:%02x"  ,VG.TrackReal[2],VG.TrackReal[3]);

 rect(screen,x-3,y-3,x+10*8+3,y+i*16+1,0x20+0xf);
}
/*
struct DatVG {
	byte        DataIO;       // Значение, которое передается через порт
	byte        OperIO;       // Операция ВГ
	byte        RegStatus;    // Регистр состояния ВГ
	byte        RegCom;       // Регистр команд
	signed char RegTrack;     // Регистр дорожки
	byte        RegSect;      // Регистр сектора
	byte        RegData;      // Регистр данных
	byte        System;       // Системный регистр контроллера дисковода
	signed char StepDirect;   // Направление шага: -01 - назад; 01 - вперед
	byte        TrackReal[4]; // Положение головок дисководов
	byte        reserved[3];
};
*/
#endif
