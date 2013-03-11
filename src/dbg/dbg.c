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
#include <allegro.h>
#include "dbg.h"
#include "../korvet.h"

extern int AllScreenUpdateFlag;

extern byte BreakPoint[0xffff];

struct CPUREG dbg_REG;
struct CPUREG dbg_prevREG;

#define MAXDBG 3
int (*_dbg[MAXDBG])(int Key)={_REGS,_DASM,_DUMP};
int  dbgMODE =1;    //текущий режим отладчика.
 
int  dbg_TRACE=0;   //если =1 то остановится ПЕРЕД выполнением комманды
word dbg_HERE =0xffff;   //адрес остановки при нажатии клавиши F4 (HERE)
int  InDBG    =0;
int  dbg_Cause=0;
int  dbg_CauseAddr=0;

// =1 if CALL      0 1 2 3 4 5 6 7 8 9 a b c d e f
byte  TCall[256]= {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 00
                   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 10
                   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 20
                   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 30
                   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 40
                   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 50
                   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 60
                   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 70
                   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 80
                   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 90
                   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // a0
                   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // b0
                   0,0,0,0,1,0,0,1,0,0,0,0,1,1,0,1, // c0
                   0,0,0,0,1,0,0,1,0,0,0,0,1,0,0,1, // d0
                   0,0,0,0,1,0,0,1,0,0,0,0,1,0,0,1, // e0
                   0,0,0,0,1,0,0,1,0,0,0,0,1,0,0,1, // f0
                  };


extern int Takt;

extern int scr_Page_Show; // Страница для отображения             ViReg:000000xx
int Saved_scr_Page_Show;
int DBG_scr_Page_Show=-1;

extern byte LUT[];
extern int BW_Flag;
byte DBG_LUT[16];
byte Save_LUT[16];
int Flag_DBG_LUT=0;
int Flag_DBG_LUT_Mode=0;   // 0 - all color, 1 - ACZU only

void dbg_INIT(void) {
 int i;
 for (i=0;i<0x10000;i++) BreakPoint[i]=0;
 tScrInit();
 InitLabel();
}

void dbg_GetREG(void) {
   dbg_prevREG=dbg_REG;

   dbg_REG.AF =CPU_GetAF();
   dbg_REG.BC =CPU_GetBC();
   dbg_REG.DE =CPU_GetDE();
   dbg_REG.HL =CPU_GetHL();
   dbg_REG.SP =CPU_GetSP();
   dbg_REG.PC =CPU_GetPC();
   dbg_REG.Int=CPU_GetI();
}

void dbg_SetREG(void) {

   CPU_SetAF(dbg_REG.AF);
   CPU_SetBC(dbg_REG.BC);
   CPU_SetDE(dbg_REG.DE);
   CPU_SetHL(dbg_REG.HL);
   CPU_SetSP(dbg_REG.SP);
   CPU_SetPC(dbg_REG.PC);

}

void SetDBGLut(int Flag) {
  int i;
//  byte LUT_ACZU[16]={0x0,0x9,0xa,0xb,0xc,0xd,0xe,0xf,0xF,0xF,0xF,0xF,0xF,0xF,0xF,0xF};
  byte LUT_ACZU[16]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xF,0xF,0xF,0xF,0xF,0xF,0xF,0xF};
  if (Flag) {
      if (Flag_DBG_LUT_Mode) for (i=0;i<16;i++) LUT[i]=LUT_ACZU[i];
      else                   for (i=0;i<16;i++) LUT[i]=i;  
  } else {
    for (i=0;i<16;i++) {LUT[i]=Save_LUT[i];};
  }
  AllScreenUpdateFlag=1;
  LUT_Update(BW_Flag);
}

void UpdateHARDWARE(void){
 char BUF[1024];

 sprintf(BUF,"TState: %5d ",Takt);tScreenPutString(BUF,C_Default,90,18);
// sprintf(BUF,"SysReg: %02x",SYSREG<<2);tScreenPutString(BUF,CText,62,11);
/*
 if (DBG_scr_Page_Show != -1) {
    sprintf(BUF,"SHOW_Page:%d -> %d",DBG_scr_Page_Show,Saved_scr_Page_Show);
    tScreenPutString(BUF,CBreak1,62,13);
 } else {
    sprintf(BUF,"                  ");
    tScreenPutString(BUF,CText,62,13);
 };
*/
 if (Flag_DBG_LUT) {
    sprintf(BUF,"F5: LUT:DBG %s",(Flag_DBG_LUT_Mode)?"aczu":"all ");
    tScreenPutString(BUF,C_High,71,18);
 } else {
    sprintf(BUF,"F5: LUT:real    ");
    tScreenPutString(BUF,C_Default,71,18);
 };

 ShowPICdbg();
// ShowTMRdbg();
 ShowPPIdbg();
// ShowKBDdbg();
// ShowFDCdbg();
}

void Update_Screen(void) {
  Update_REGS();
  Update_DASM();
  Update_DUMP();
  Update_HISTORY();
  UpdateHARDWARE();
  UpdateCOMNAME();
  AllScreenUpdateFlag=1;
  SCREEN_ShowScreen();
  UpdateGameTool();
}

void doDBG(void) {
  int Key;
  int Exit=0;
  int i;

  dbg_TRACE=0;
  dbg_HERE =0xffff;
  InDBG    =1;

  Saved_scr_Page_Show=scr_Page_Show;
  if (DBG_scr_Page_Show != -1) scr_Page_Show=DBG_scr_Page_Show;

  for (i=0;i<16;i++) {Save_LUT[i]=LUT[i];}
//  SetDBGLut(Flag_DBG_LUT);

  SCREEN_SetGraphics(SCR_DBG);
  DBG_Pallete_Active();
  dbg_GetREG();
  while (keypressed()) readkey();

  NormPC();
  Update_Screen();
  _dbg[dbgMODE](-1);

  while (!Exit) {

    AddKorvetLabel();

    SetDBGLut(Flag_DBG_LUT);

    Key=readkey();

    switch (Key) {

       case (KEY_UP  <<8)+KK_Ctrl : {if (dbgMODE != 0 )       dbgMODE--;Key=-1;break;}
       case (KEY_DOWN<<8)+KK_Ctrl : {if (dbgMODE != MAXDBG-1) dbgMODE++;Key=-1;break;}

       case (KEY_L   <<8)+KEY_L   : {tShowAll();Update_Screen();        Key=-1;break;}
       case (KEY_F5  <<8)         : {Flag_DBG_LUT^=1;                   Key=-1;break;}
       case (KEY_F5  <<8)+KK_Ctrl : {Flag_DBG_LUT_Mode^=1;              Key=-1;break;}

       case (KEY_W   <<8)+KK_Alt  : {WriteMEM();Update_Screen();        Key=-1;break;}
       case (KEY_R   <<8)+KK_Alt  : {ReadMEM();Update_Screen();         Key=-1;break;}

       case (KEY_Y   <<8)+KK_Alt  : {WriteSYM();Update_Screen();        Key=-1;break;}
       case (KEY_Y   <<8)+KEY_Y   : {ReadSYM();Update_Screen();         Key=-1;break;}

       case (KEY_Z   <<8)+KEY_Z   : {GameTools();Update_Screen();       Key=-1;break;}
    }

    Key=_dbg[dbgMODE](Key);

    switch (Key) {

       case (KEY_F7  <<8)         : {dbg_TRACE=1;Exit=1;break;}
       case (KEY_F8  <<8)         : {if (TCall[Emulator_Read(dbg_REG.PC)]) dbg_HERE=dbg_REG.PC+GetCmdLen(dbg_REG.PC);
                                     else dbg_TRACE=1;
                                     Exit=1;break;
                                    }
       case (KEY_F9  <<8)         : {Exit=1;break;}
    }
//    if (Key>>8 == KEY_ESC) Exit=1;
  }

  while (keypressed()) readkey();

  while(key[KEY_F4]);
  while(key[KEY_F7]);
  while(key[KEY_F8]);
  while(key[KEY_F9]);

  while(key[Key]);

  scr_Page_Show=Saved_scr_Page_Show;
  AllScreenUpdateFlag=1;

  SetDBGLut(0);

  InDBG    =0;
  dbg_SetREG();
  DBG_Pallete_Pasive();
}

