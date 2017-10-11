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
#include "dbg.h"

extern int SYSREG;

int RGBASE[32]={
		0x3A00,		-10000,		-10000,		-10000,
		0xFA00,		0xFA00,		0xFA00,		0xFA00,
		0x3A00,		-10000,		-10000,		-10000,
		0xFF00,		0xFF00,		0xFF00,		0xFF00,
		0xFA00,		0xFA00,		0xFA00,		0xFA00,
		0xFF00,		0xFF00,		0xFF00,		0xFF00,
		0xBF00,		0xBF00,		0xBF00,		0xBF00,
};

int PBASE[32]={
		0x3B00,		-10000,		-10000,		-10000,
		0xFB00,		0xFB00,		0xFB00,		0xFB00,
		0x3B00,		-10000,		-10000,		-10000,
		0xFE00,		0xFE00,		0xFE00,		0xFE00,
		0xFB00,		0xFB00,		0xFB80,		0xFB00,
		0xFE00,		0xFE00,		0xFE00,		0xFE00,
		-10000,		-10000,		-10000,		-10000,
};




void AddSysLabel(word Addr,char *Name) {
 _Label *L;
 if ((L=FindNameLabel(Name))) DeleteLabel(L->Addr);
 if (Addr>0) AddLabel(Addr,Name);
}

void AddKorvetLabel(void) {
int rgbase=RGBASE[SYSREG];
int pbase =PBASE[SYSREG];

AddSysLabel(rgbase+0x7f,"SYSREG");
AddSysLabel(rgbase+0xbf,"NCREG");
AddSysLabel(rgbase+0xfb,"LUT");

AddSysLabel(pbase+0x00,"TMR_0");
AddSysLabel(pbase+0x01,"TMR_1");
AddSysLabel(pbase+0x02,"TMR_2");
AddSysLabel(pbase+0x03,"TMR_RUS");
AddSysLabel(pbase+0x08,"PPI3A");
AddSysLabel(pbase+0x09,"PPI3B");
AddSysLabel(pbase+0x0a,"PPI3C");
AddSysLabel(pbase+0x0b,"PPI3RUS");
AddSysLabel(pbase+0x10,"RS232_DATA");
AddSysLabel(pbase+0x11,"RS232_CTL");
AddSysLabel(pbase+0x18,"FDC_CMD");
AddSysLabel(pbase+0x19,"FDC_TRK");
AddSysLabel(pbase+0x1a,"FDC_SEC");
AddSysLabel(pbase+0x1b,"FDC_DATA");
AddSysLabel(pbase+0x20,"LAN_DATA");
AddSysLabel(pbase+0x21,"LAN_CTRL");
AddSysLabel(pbase+0x28,"PIC_RUS");
AddSysLabel(pbase+0x29,"PIC_MASK");
AddSysLabel(pbase+0x30,"PPI2A_LSTDATA");
AddSysLabel(pbase+0x31,"PPI2B_");
AddSysLabel(pbase+0x32,"PPI2C_");
AddSysLabel(pbase+0x33,"PPI2_RUS");
AddSysLabel(pbase+0x38,"PPI1A_");
AddSysLabel(pbase+0x39,"P1B_DRVREG");
AddSysLabel(pbase+0x3a,"P1C_VIREG");
AddSysLabel(pbase+0x3b,"P1_RUS");

AddSysLabel(0x43,"RLOADTST");
AddSysLabel(0x46,"RCONST");
AddSysLabel(0x49,"RCONIN");
AddSysLabel(0x4c,"RCONOUT");

AddSysLabel(0xf700,"RMUNUM");
AddSysLabel(0xf701,"FDDFLAG");
AddSysLabel(0xf703,"SysCOPY");
AddSysLabel(0xf704,"ColCOPY");
AddSysLabel(0xf705,"CursorAddr");
AddSysLabel(0xf707,"CsrFLAG");
AddSysLabel(0xf708,"Cursor");
AddSysLabel(0xf709,"CsrAttr");
AddSysLabel(0xf70a,"BufChar");
AddSysLabel(0xf70b,"BufAttr");
AddSysLabel(0xf70c,"AutoFlag");
AddSysLabel(0xf70d,"ESC_Flag");
AddSysLabel(0xf70e,"FNUM");
AddSysLabel(0xf70f,"ADRTAB");
AddSysLabel(0xf711,"ESCTAB");
AddSysLabel(0xf713,"MIN_ESC");
AddSysLabel(0xf714,"LIM_ESC");
AddSysLabel(0xf715,"BELL_Div");
AddSysLabel(0xf717,"BELL_Del");
AddSysLabel(0xf719,"SYMNBUF");
AddSysLabel(0xf71a,"SetFlag");
AddSysLabel(0xf720,"LongVal");
AddSysLabel(0xf721,"AutoVal");
AddSysLabel(0xf723,"GetPNT");
AddSysLabel(0xf725,"PutPNT");
AddSysLabel(0xf729,"ConTAB");
AddSysLabel(0xf72b,"FunTAB");
AddSysLabel(0xf72d,"F_Shift");
AddSysLabel(0xf72e,"F_Alft");
AddSysLabel(0xf72f,"F_Graph");
AddSysLabel(0xf730,"F_Sel");
AddSysLabel(0xf731,"SndFlag");
AddSysLabel(0xf76e,"LutFlag");
AddSysLabel(0xf76f,"AdrLUT");
AddSysLabel(0xf771,"TabLUT");
AddSysLabel(0xf781,"PrnTYPE");
AddSysLabel(0xf782,"PrnTAB");
AddSysLabel(0xf7c8,"_DOINT0");
AddSysLabel(0xf7ca,"_DOiNT1");
AddSysLabel(0xf7cc,"_DOINT2");
AddSysLabel(0xf7ce,"_DOINT3");
AddSysLabel(0xf7d0,"_DOINT4");
AddSysLabel(0xf7d2,"_DOINT5");
AddSysLabel(0xf7d4,"_DOINT6");
AddSysLabel(0xf7d6,"_DOINT7");
AddSysLabel(0xf7d8,"_DOAUX0");
AddSysLabel(0xf7da,"_DOAUX1");
AddSysLabel(0xf7dc,"_DOAUX2");
AddSysLabel(0xf7de,"_DOAUX3");
}
