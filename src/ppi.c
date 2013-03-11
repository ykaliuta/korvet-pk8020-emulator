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

#define noPPI_DEBUG

//extern screen,font;
#ifdef DBG
 #include <allegro.h>
#endif

#ifdef TRACETIMER
extern int Takt;
extern F_TIMER;
#endif

extern int SYSREG;
extern int NCREG;

// --------------------------------------------------------------------------
// переменные из модуля экрана, собиракм и разбираем
// прямо при записи, чтении.
extern int scr_Page_Acces;       // Страница для достуап к видеопамяти   ViReg:xx000000
extern int scr_Page_Show;        // Страница для отображения             ViReg:000000xx
// АЦЗУ
extern int scr_Attr_Write;       // Атрибут для записи в АЦЗУ	         ViReg:00xx0000
extern int scr_Wide_Mode;        // Флаг режима 32 символа в строке      ViReg:0000x000
extern int scr_Second_Font;      // Флаг выбора второго знакогенератора  ViReg:00000x00

extern int scr_Attr_Read;        // Значени Инверсии при чтении 	 VisST:0000x000

extern int AllScreenUpdateFlag;  // Флаг необходимости обновть весь экран
extern int scr_GZU_Size_Mask;	 // маска размера ГЗУ, =0x0f - 4*48, =0 - 1x 48k
// -------------------------------------------------------------------------
// Переменная из модуля
extern int DRVREG;               // см. в модуле контролера дисковода

// -------------------------------------------------------------------------
// Переменная из модуля Звука (Флаг разрешения звука)
extern int SoundEnable;

// -------------------------------------------------------------------------
// Переменная из модуля Синхронизация.
extern int VBLANK;
extern int Takt;

extern int k_mousex;
extern int k_mousey;
extern int k_mouseb;

// -------------------------------------------------------------------------
// Переменная из модуля Принтера.
//extern int LSTSTS; // Признак готовности принтера PPI1, port A, Bit 00000x00
//extern int LSTCTL; // Строб записи

//
int PPI1_A,PPI1_B,PPI1_C,PPI1_RUS;
int PPI2_A,PPI2_B,PPI2_C,PPI2_RUS;
int PPI3_A,PPI3_B,PPI3_C,PPI3_RUS;

// Все управляющие сигналы в собственных переменных,
// Assemble
//             - Собираем значение байта из отдельных переменных.
// Disassemble
//             - Разбираем байт на отедльные переменные.
//

byte Assemble_PPI1A(void)
{
   int Value=0;

   Value|=(LANADDR           &0x0f)<<4; // xxxx0000
   Value|=(scr_Attr_Read     &0x01)<<3; // 0000x000
   Value|=(GetPrinterStatus()&0x01)<<2; // 00000x00
   Value|=((Takt<VBLANK_TAKT)?1:0 )<<1; // 000000x0
//   Value|=(TapeIN  &0x01)<<0;        // 0000000x
//  textprintf(screen,font,500,60,15,"R_38: %02x",Value);
   return Value;
}

void DisassembleVIREG_PPI1C(byte Value)
{
   static unsigned char cntr=0;

   scr_Page_Acces =(Value&0xc0)>>6;    // ViReg:xx000000
   scr_Attr_Write =(Value&0x30)>>4;    // ViReg:00xx0000
   scr_Wide_Mode  =(Value&0x08)>>3;    // ViReg:0000x000
   scr_Second_Font=(Value&0x04)>>2;    // ViReg:00000x00
   scr_Page_Show  =(Value&0x03)>>0;    // ViReg:000000xx

   // Обновить весь экран если установлен режим влияет на весь
   // экран. (ширина символов, знакогенератор, стр. отображения графики)
   if ( (PPI1_C&0x0f) != (Value&0x0f)) AllScreenUpdateFlag=1;

   scr_Page_Acces &= scr_GZU_Size_Mask;
   scr_Page_Show  &= scr_GZU_Size_Mask;

//   textprintf(screen,font,0,60,255,"W:Vi %3d",cntr++);
#ifndef EGA
//   textprintf(screen,font,0,60,255,"A:%d S:%d",scr_Page_Acces,scr_Page_Show);
#endif

}

byte AssembleVIREG_PPI1C(void)
{
   int Value=0;
   Value|=(scr_Page_Acces &0x03)<<6;     // ViReg:xx000000
   Value|=(scr_Attr_Write &0x03)<<4;     // ViReg:00xx0000
   Value|=(scr_Wide_Mode  &0x01)<<3;     // ViReg:0000x000
   Value|=(scr_Second_Font&0x01)<<2;     // ViReg:00000x00
   Value|=(scr_Page_Show  &0x03)<<0;     // ViReg:000000xx
   return Value;
}

void Disassemble_PPI2C(int Value)
{
   DoTimer();
//   XS1:32          =(Value&0x80)>>7;
//  Reset for Analog Joystick =(Value&0x40)>>6;
// ~SE               =(Value&0x20)>>5;

// ~ACK              =(Value&0x10)>>4;
//   SetPrinterStrobe((Value&0x10)>>4);
   SetPrinterStrobe((Value&0x20)>>4);

   SoundEnable       =(Value&0x08)>>3;
// Cassete Motor ON  =(Value&0x04)>>2;
// CasseteOut level  =(Value&0x03)>>0;

#ifdef TRACETIMER
 fprintf(F_TIMER,"S: %08d %d\n",Takt,SoundEnable);
#endif
}

int Assemble_PPI2C(void)
{
    int Value=0;
//   XS1:32          =(Value&0x80)>>7;
//  Reset for Analog Joystick =(Value&0x40)>>6;
// ~SE               =(Value&0x20)>>5;
// ~ACK              =(Value&0x10)>>4;
   Value             =(SoundEnable&0x01)<<3;
// Cassete Motor ON  =(Value&0x04)>>2;
// CasseteOut level  =(Value&0x03)>>0;
   return Value;
}

void PPI1_Write(int Addr, byte Value) {
    int bit,oldc;

    switch (Addr & 0x03) {
      case 0: {PPI1_A=Value;break;}
      case 1: {FDC_Write_DRVREG(Value);break;}
      case 2: {
           DisassembleVIREG_PPI1C(Value);
           PPI1_C=Value;
           break;
      }
      case 3: {
           if (Value & 0x80) PPI1_RUS=Value;
           else {
             bit=1 << ( (Value & 0x0e) >> 1);
             oldc=PPI1_C;
             if (Value &1) oldc|=bit;
             else          oldc&=bit^0xff;
             DisassembleVIREG_PPI1C(oldc);
             PPI1_C=oldc;
           }
           break;
      }
    }
}

byte PPI1_Read(int Addr){
    int Value;
    switch (Addr & 0x03) {
      case 0: {Value=Assemble_PPI1A();break;}
      case 1: {Value=FDC_Read_DRVREG();break;}
      case 2: {Value=AssembleVIREG_PPI1C();break;}
      case 3: {return 0xff;break;}
    }
    return Value;
}

void PPI2_Write(int Addr, byte Value) {
    int bit;
    switch (Addr & 0x03) {
      case 0: {PPI2_A=Value;break;}
      case 1: {PPI2_B=Value;break;}
      case 2: {PPI2_C=Value;Disassemble_PPI2C(PPI2_C);break;}
      case 3: {
           if (Value & 0x80) PPI2_RUS=Value;
           else {
             bit=1 << ( (Value & 0x0e) >> 1);
             if (Value &1) PPI2_C|=bit;
             else          PPI2_C&=bit^0xff;
           }
           Disassemble_PPI2C(PPI2_C);
           break;
      }
    }
}

byte PPI2_Read(int Addr){
    int Value;
    switch (Addr & 0x03) {
      case 0: {Value=PPI2_A;break;}
      case 1: {Value=PPI2_B;break;}
      case 2: {Value=PPI2_C=Assemble_PPI2C();break;}
      case 3: {return 0xff;break;}
    }
    return Value;
}

void PPI3_Write(int Addr, byte Value) {
    int bit;
    switch (Addr & 0x03) {
      case 0: {PPI3_A=Value;break;}
      case 1: {PPI3_B=Value;break;}
      case 2: {PPI3_C=Value;break;}
      case 3: {
           if (Value & 0x80) PPI3_RUS=Value;
           else {
             bit=1 << ( (Value & 0x0e) >> 1);
             if (Value &1) PPI3_C|=bit;
             else          PPI3_C&=bit^0xff;
           }
           break;
      }
    }

}

byte PPI3_Read(int Addr){
    int Value;
    switch (Addr & 0x03) {
      case 0: {Value=PPI3_A;break;}
      case 1: {
/*
// k_mouse part
           if (PPI3_C & 0x10) {
              Value = (8-k_mousey) | (k_mouseb^0x30);
              k_mousey=0;
           } else {
              Value = (k_mousex+8) | (k_mouseb^0x30);
              k_mousex=0;
           }
           break;
*/
// deflector Joystick
             Value=Read_Joystick();break;

//           Value=PPI3_B;break;
      }
      case 2: {Value=PPI3_C;break;}
      case 3: {return 0xff;break;}
    }
    return Value;
}

void PPI_Init(void)
{
 PPI1_C=AssembleVIREG_PPI1C();
 PPI2_C=Assemble_PPI2C();
}

#ifdef DBG
void ShowPPIdbg(void) {

 int i=0;
 int x=10;
 int y=657;
 char nc_rd[8][4] ={
                    "___",
                    "__1",
                    "_2_",
                    "_21",
                    "3__",
                    "3_1",
                    "32_",
                    "321",
                   };

 char tncreg[512];

 char tsysreg[32][40]={          
                       "0-37FF|3C00|    |3B00|3A00|3800",
                       "0-1FFF|    |    |    |    |    ",
                       "0-3FFF|    |    |    |    |    ",
                       "      |    |    |    |    |    ",
                       "0-1FFF|FC00|    |FB00|FA00|F800",
                       "0-1FFF|FC00|    |FB00|FA00|F800",
                       "0-3FFF|FC00|    |FB00|FA00|F800",
                       "      |FC00|    |FB00|FA00|F800",
                       "0-37FF|3C00|C000|3B00|3A00|3800",
                       "0-1FFF|    |C000|    |    |    ",
                       "0-3FFF|    |C000|    |    |    ",
                       "      |    |C000|    |    |    ",
                       "0-1FFF|    |4000|FE00|FF00|    ",
                       "0-1FFF|    |4000|FE00|FF00|    ",
                       "0-3FFF|    |4000|FE00|FF00|    ",
                       "      |    |4000|FE00|FF00|    ",
                       "0-5FFF|FC00|    |FB00|FA00|F800",
                       "0-1FFF|FC00|    |FB00|FA00|F800",
                       "0-3FFF|FC00|    |FB80|FA00|F800",
                       "      |FC00|    |FB00|FA00|F800",
                       "0-5FFF|    |    |FE00|FF00|    ",
                       "0-1FFF|    |    |FE00|FF00|    ",
                       "0-3FFF|    |    |FE00|FF00|    ",
                       "      |    |    |FE00|FF00|    ",
                       "0-5FFF|    |C000|    |BF00|    ",
                       "0-1FFF|    |C000|    |BF00|    ",
                       "0-3FFF|    |C000|    |BF00|    ",
                       "      |    |C000|    |BF00|    ",
                       "0-5FFF|    |C000|    |    |    ",
                       "0-1FFF|    |C000|    |    |    ",
                       "0-3FFF|    |C000|    |    |    ",
                       "      |    |C000|    |    |    ",
                      };


// rect(screen,x-3,y-3,x+15*8+3,y+16*8+3,0x20+0xf);
// textprintf(screen,font,x+48,y-20,0x20+0x0f,"PIC");

 if (NCREG & 0x80) { // Color MODE
   sprintf(tncreg,"COLOR: Read:%d (%s) Write:%d (%s)            ",
                 (NCREG>>4)&0x7,nc_rd[(NCREG>>4)&0x7],
                 (NCREG>>1)&0x7,nc_rd[(NCREG>>1)&0x7]
          );

 } else {
   sprintf(tncreg,"PLANE: Val: %d Read:%s Write:%s "
                 ,NCREG&1,nc_rd[(NCREG>>4)&0x7],nc_rd[((~NCREG)>>1)&0x7]);
 }

 textprintf(screen,font,x,y+16*i++,0x20+0x07,"           | ROM  |ACZU|GZU |RG  |PB  |KEY ");

 textprintf(screen,font,x,y+16*i++,0x20+0x07,"SysReg: %02x >%s",SYSREG<<2,tsysreg[SYSREG]);
 textprintf(screen,font,x,y+16*i++,0x20+0x07,"NCREG : %02x >%s",NCREG,tncreg);
 textprintf(screen,font,x,y+16*i++,0x20+0x07,"ViREG : %02x >GZU PAGE (Acess :%d Show:%d)",
                                             AssembleVIREG_PPI1C(),
                                             scr_Page_Acces,
                                             scr_Page_Show
           );
 textprintf(screen,font,x,y+16*i++,0x20+0x07,"            ACZU     (Attr:%d Wide:%d FONT2:%d)",
                                             scr_Attr_Write,
                                             scr_Wide_Mode,
                                             scr_Second_Font
           );
 textprintf(screen,font,x,y+16*i++,0x20+0x07,"VBlank: %d",VBLANK);
}

#endif
