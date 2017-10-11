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
// Korvet ETALON emulator project
// (c) 2000-2003 Sergey Erokhin aka ESL
// (c) 2003      Korvet Team.


#include "korvet.h"

#include "PlaneMask.h"      // Arrays for Plane MASK optimization

#define noSLOWBITMAPCONVERT

#define LUT_BASE_COLOR 0x80


int (*SetGraphics)(void);
int (*SetText)(void);
void (*ShowScreen)(void);

#define noDEBUG_SCREEN

#define PLANESIZE 	16384
#define FONTSIZE        (16*256*2)

// GZU defines

#define COLORMD 0x80	  // 10000000
#define WRMSK   0x0E	  // 00001110
#define RDMSK   0x70    // 01110000
#define WBIT    0x01    // 00000001
#define WSEL0   0xfd    // 11111101
#define WSEL1   0xfb    // 11111011
#define WSEL2   0xf7    // 11110111
#define RSEL0   0x10    // 00010000
#define RSEL1   0x20    // 00100000
#define RSEL2   0x40    // 01000000


// EXTERNAL !!!!!!!!!!!!!!! --------------------------------------- EXT VAR
unsigned int NCREG=0;
// ================================================================ EXT VAR

// Переменные
// Биты продставленны в виде байтов, разбока и сборка для чтения
// и записи в порты делаем только при необходимости

// Vireg : xx3A  - запись
// VistST: xx38  - чтение

// ГЗУ
int scr_Page_Acces;     // Страница для достуап к видеопамяти  ViReg:xx000000
int scr_Page_Show;      // Страница для отображения            ViReg:000000xx
int scr_GZU_Size_Mask=0x0f;  // маска размера ГЗУ, =0x0f - 4*48, =0 - 1x 48k

// АЦЗУ
int scr_Attr_Write;   	// Атрибут для записи в АЦЗУ	         ViReg:00xx0000
int scr_Wide_Mode;	    // Флаг режима 32 символа в строке     ViReg:0000x000
int scr_Second_Font;	  // Флаг выбора второго знакогенератора ViReg:00000x00

int scr_Attr_Read;	    // Значени Инверсии при чтении 	       VisST:0000x000

// Масивы памяти

byte ACZU[1024*2];	      // 1К памяти АЦЗУ плюс атрибуты (INV)
int  ACZU_Update_Flag;
int  ACZU_TouchFlag[1024];// флаг изменения байта, для оптимизации

byte KFONT[FONTSIZE];   // Знакогенератор для режима 64 символа в строке
byte KFONT2[FONTSIZE*2];// Знакогенератор для режима 32 символа в строке
char FontFileName[1024]="data/korvet2.fnt";

byte GZU[4][PLANESIZE*(3+1)]; // 3 слоя ГЗУ (4 страницы) + слой АЦЗУ

// Таблица LUT
byte LUT[16]= {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
int  LutUpdateFlag;
PALLETE pallete;
extern int OSD_LUT_Flag;

int LineUpdateFlag[256];// Таблица флагов необходимости обновления строки
int AllScreenUpdateFlag;// Флаг необходимости обновть весь экран

// ------------------------------------------------------------------
int SCREEN_OFFX=0;
int SCREEN_OFFY=0;
int SCREEN_XMAX=0;
int SCREEN_YMAX=0;
int SCREEN_OSDY=0;

int Current_Scr_Mode=0;
int FlagScreenScale=0;

int BWMode=1;

// ================================================================== Variables

// ------------------------------------------------------------------ ACZU
// работа с текстовым экраном АЦЗУ

// Прочитать из файла образ шрифта
// сгенерировать образ для широкого знакогенератора
int ACZU_InitFont(char *FileName) {
    FILE *F=fopen(FileName,"rb");
    int i,j;
    byte *p;
    byte wmask,m;

    if (F == NULL) return ERROR;
    if (fread(KFONT,1,FONTSIZE,F) != FONTSIZE) {fclose(F); return -2;}
    fclose(F);

    // convert font to WIDE
    p=KFONT2;
    for (i=0; i<FONTSIZE; i++) {
        wmask=0xc0;
        m=0x80;
        *p=0;

        for (j=0; j<4; j++) {
            if (KFONT[i]&m) *p|=wmask;
            m>>=1;
            wmask>>=2;
        }
        p++;
        wmask=0xc0;
        *p=0;
        for (j=0; j<4; j++) {
            if (KFONT[i]&m) *p|=wmask;
            m>>=1;
            wmask>>=2;
        }
        p++;
    }

    return OK;
}

// Сформировать образ битовых плоскостей в Frame_ACZU
// используя знакогенератор и в зависимости от установленых флагов режимов
// fixed by Eduard Kalinovsky

void ACZU_MakeFrameBuffer(void) {
    byte *p;
    byte *src=ACZU;
    byte *dst=GZU[scr_Page_Acces]+3;
    byte *chrdst;
    byte *fnt;
    byte *chr;
    int  scrlen;
    int  *touch=ACZU_TouchFlag;

    if (!scr_Wide_Mode) { // NORMAL FONT
        fnt=(scr_Second_Font)*16*256+KFONT;
        for (scrlen=1024; scrlen--; ) {
            if (*touch || AllScreenUpdateFlag) {
                chr=fnt+*(src)*16;
                chrdst=dst;
                byte i;
                for (i=16; i--; ) {
                    *chrdst=(*chr++)^(*(src+1));
                    chrdst+=64*4;
                }
            }
            src+=2;
            dst+=4;
            *touch++=0;
            if (!(scrlen & 0x3f)) dst+=15*64*4;
        }
    }
    else { // WIDE FONT
        fnt=(scr_Second_Font)*16*256*2+KFONT2;
        for (scrlen=512; scrlen--; ) {
            if (*touch || AllScreenUpdateFlag) {
                chr=fnt+*(src)*32;
                chrdst=dst;
                byte i;
                for (i=16; i--; ) {
                    *chrdst=(*chr++)^(*(src+1));
                    chrdst+=4;
                    *chrdst=(*chr++)^(*(src+1));
                    chrdst+=64*4-4;
                }
            }
            src+=4;
            dst+=8;
            *touch=0;
            touch+=2;
            if (!(scrlen & 0x1f)) dst+=15*64*4;
        }
    }
}

void ACZU_Write(int Addr,byte Value) {
    int i,y;
    byte *addr;
    ACZU_TouchFlag[Addr&0x3ff]=1;
    addr=&ACZU[(Addr&0x3ff)<<1];
    *addr=Value;
    addr++;
    ACZU_Update_Flag=1;
    switch (scr_Attr_Write) {
    case 0:
        break;            		// ?
    case 1:
        *addr=0xff;
        break; 	// ATRSET
    case 2:
        *addr=0x00;
        break; 	// ATRRES
    case 3:
        *addr=scr_Attr_Read;
        break;  // ATRFRE
    }
    y=((Addr&0x3f0)>>6)<<4;
    for (i=15; i>=0; i--) LineUpdateFlag[y+i]=1;
}

byte ACZU_Read(int Addr) {
    byte *adr=&ACZU[(Addr&0x3ff)<<1];
    if (scr_Attr_Write == 3) scr_Attr_Read=*(adr+1);
    return *adr;
}

int ACZU_Init(void) {
    int i;

    // clear memory
    for (i=0; i<1024*2; i++) ACZU[i]=0;
    for (i=0; i<1024; i++) ACZU_TouchFlag[i]=1;
    for (i=0; i<256; i++) LineUpdateFlag[i]=1;
    AllScreenUpdateFlag=ACZU_Update_Flag=1;

    // init vars
    scr_Attr_Write =2;
    scr_Wide_Mode  =0;
    scr_Second_Font=0;
    scr_Attr_Read  =0;

    // read font
    if (ACZU_InitFont(FontFileName) != OK) {
        printf("Can't open font file : %s\n",FontFileName);
        return ERROR;
    }
    return OK;
}
// ================================================================== ACZU

// ------------------------------------------------------------------ GZU

void GZU_Write(int Addr,byte Value) {
    byte *GZU_Ptr=GZU[scr_Page_Acces]+(Addr&0x3fff)*4;
    byte mask;

    int a0,a1,a2,c0,c1,c2;

    a0=*(GZU_Ptr+0);
    a1=*(GZU_Ptr+1);
    a2=*(GZU_Ptr+2);

    LineUpdateFlag[(Addr&0x3fff)>>6]=1;

    if (NCREG & COLORMD) { //Color mode
        *(GZU_Ptr+0)=(*(GZU_Ptr+0) & ~Value) | ((NCREG & 2)?Value:0);
        *(GZU_Ptr+1)=(*(GZU_Ptr+1) & ~Value) | ((NCREG & 4)?Value:0);
        *(GZU_Ptr+2)=(*(GZU_Ptr+2) & ~Value) | ((NCREG & 8)?Value:0);
    } else { //Plane mode
        // WBIT !!!!!!!!!!!!!!!!!!!!!!!
        mask=(NCREG&WBIT)?Value:0;

        if (!(NCREG & ~WSEL0)) {
            *(GZU_Ptr+0)=(*(GZU_Ptr+0) & ~Value) | mask;
        }
        if (!(NCREG & ~WSEL1)) {
            *(GZU_Ptr+1)=(*(GZU_Ptr+1) & ~Value) | mask;
        }
        if (!(NCREG & ~WSEL2)) {
            *(GZU_Ptr+2)=(*(GZU_Ptr+2) & ~Value) | mask;
        }
    }

    c0=*(GZU_Ptr+0);
    c1=*(GZU_Ptr+1);
    c2=*(GZU_Ptr+2);
}

byte GZU_Read(int Addr) {
    unsigned char *GZU_Ptr=GZU[scr_Page_Acces]+(Addr&0x3fff)*4;
    unsigned char Value=0;
    unsigned char C0;
    unsigned char C1;
    unsigned char C2;

    C0=*GZU_Ptr++;
    C1=*GZU_Ptr++;
    C2=*GZU_Ptr++;

    if (NCREG & COLORMD) { //Color mode
        if (!(NCREG & RSEL0)) C0^=0xff;
        if (!(NCREG & RSEL1)) C1^=0xff;
        if (!(NCREG & RSEL2)) C2^=0xff;
        Value=(C0&C1&C2)^0xff;
    } else { //Plane mode
        if (NCREG & RSEL0) Value|=C0;
        if (NCREG & RSEL1) Value|=C1;
        if (NCREG & RSEL2) Value|=C2;
    }

    return Value;
}

int GZU_Init(void) {
    int i,j;
    // clear memory
    for (i=0; i<4; i++)
        for (j=0; j<16384*4; j++) GZU[i][j]=0;

    // init vars
    scr_Page_Acces=0;
    scr_Page_Show=0;
    AllScreenUpdateFlag=1;
}

// ================================================================== GZU

// ------------------------------------------------------------------ LUT
// Используя таблицу LUT сформировать палитру для VGA
// возможно чернобелую

void LUT_Update(int BWFlag) {
    int i;
    int bright;
    int c;

    //      I    R    G     B
    //[:p, [3.0, 6.2, 12.0, 24.0]]
    //      0x88 0x44 0x22  0x11
    // /4 (00..255 -> 00.63)

    //esl calulated shifted to 8
    int BW_Array_ESL[]= {0,8,10,13,16,19,22,25,29,33,37,42,46,51,56,60};
    //ddp photo bw
    int BW_Array_DDP[]= {4/4, 20/4, 34/4, 50/4, 62/4, 78/4, 93/4, 108/4, 123/4, 140/4, 155/4, 171/4, 184/4, 199/4, 215/4, 231/4};

    for (i=0; i<16; i++) {
        // COLOR PALETTE
        bright=(LUT[i]&0x8)?21:0;
        pallete[LUT_BASE_COLOR+i].r=((LUT[i]&0x4)?42:0)+bright;
        pallete[LUT_BASE_COLOR+i].g=((LUT[i]&0x2)?42:0)+bright;
        pallete[LUT_BASE_COLOR+i].b=((LUT[i]&0x1)?42:0)+bright;

        if (BWFlag) {// BLACK & WHILE PALETTE

            c=BW_Array_ESL[LUT[i]];

            pallete[LUT_BASE_COLOR+i].r=c;
            pallete[LUT_BASE_COLOR+i].g=c; // if r=0 & b=0 -- cool black green pallete
            pallete[LUT_BASE_COLOR+i].b=c;
        }
    }
    set_palette_range(pallete,LUT_BASE_COLOR,LUT_BASE_COLOR+16,0); //last paramerer - vblakn sync flag
    LutUpdateFlag=0;
}

void LUT_Write(byte Value) {
    LUT[Value&0x0F]=Value>>4;

    // show LUT value on screen
    //  textprintf(screen,font,100+(Value&0x0F)*20,40,LUT_BASE_COLOR+(Value&0x0f),"%02x",Value);
    if (OSD_LUT_Flag) PutLED_Lut(SCREEN_OFFX+70+(Value&0x0F)*20,SCREEN_OSDY,Value,LUT_BASE_COLOR+(Value&0x0f));
    LutUpdateFlag=1;
}

void LUT_Init(void) {
    LutUpdateFlag=1;
    return ;
}

// ================================================================== LUT
// ---------------------------------------------------------------------- SCREEN
int SCREEN_SetGraphics(int ScrMode) {
    int tmp;
    static int oldWindowed=0;
    int response=0;
    int WindowSizeX=0;
    int WindowSizeY=0;


    if (oldWindowed != FlagScreenScale) Current_Scr_Mode=-1;
    if (ScrMode != Current_Scr_Mode) {

        set_gfx_mode(GFX_TEXT,0,0,0,0);
        set_color_depth(8);

        if (ScrMode == SCR_DBG) {

            SCREEN_OFFX=(1024-512-4);
            SCREEN_OFFY=(2+5);

            SCREEN_XMAX=512;
            SCREEN_YMAX=256;

            response=set_gfx_mode(GFX_AUTODETECT_WINDOWED, 1024, 768, 0, 0);
            FlagScreenScale=0;

        } else {

            if (FlagScreenScale) {
                WindowSizeX=1024+4;
                WindowSizeY=512+4+70;
                SCREEN_XMAX=512*2;
                SCREEN_YMAX=256*2;
            } else {
                WindowSizeX=512+4;
                WindowSizeY=256+4+70;
                SCREEN_XMAX=512;
                SCREEN_YMAX=256;
            }

            response=set_gfx_mode(GFX_AUTODETECT_WINDOWED, WindowSizeX, WindowSizeY, 0, 0);

            SCREEN_OFFX=2;//((WindowSizeX-512)/2);
            SCREEN_OFFY=2;//((WindowSizeY-256-70)/2); // 70 - fullscreen shift for bottom text

        }

        SCREEN_OSDY=SCREEN_OFFY+SCREEN_YMAX+2;

        AllScreenUpdateFlag=1;
        LutUpdateFlag=1;
    }
    Current_Scr_Mode=ScrMode;
    oldWindowed = FlagScreenScale;
}

int SCREEN_SetText(void) {
    return set_gfx_mode(GFX_TEXT,0,0,0,0);
}

// ------------------------------------------------------------------ MAIN
// Продцедура вывода экрана Корвета на реальный экран ПК
//void ShowSCREEN(void) {

void SCREEN_ShowScreen(void) {
    int 		x,y,i,j;
    int updated;
    int lines_updated=0;

    byte 		     c1,c2,c3,c4;
    byte 		     VGA_Frame[512*256];
    byte            *GZU_Ptr;            // Указатель на область ГЗУ
    unsigned int  	*d_VGA_Ptr;          // Указатель на область буфера виртуального экрана

    int y_min=500;
    int y_max=0;
    int y_lines_to_scr=0;

    int blit2x_flag=(Current_Scr_Mode != SCR_DBG) && FlagScreenScale;

    static BITMAP  *BITMAP_KORVET = NULL;
    static BITMAP  *BITMAP_KORVET2x = NULL;
    static unsigned int *BITMAP_PTR;
    static unsigned int *BITMAP_PTR2X;

    if (!BITMAP_KORVET) {
        BITMAP_KORVET=create_bitmap_ex(8,512,256);
        BITMAP_KORVET2x=create_bitmap_ex(8,512*2,256*2);
    }

    if (ACZU_Update_Flag || AllScreenUpdateFlag) {
        ACZU_MakeFrameBuffer();
        ACZU_Update_Flag=0;
    }

    GZU_Ptr   =GZU[scr_Page_Show];

    acquire_screen();

    for (y=0; y<256; y++) {
        //  Ввыводим на экран только строки которые нужно обновить
        if (LineUpdateFlag[y] || AllScreenUpdateFlag) {
            LineUpdateFlag[y]=0;
            lines_updated+=1;

            if (y<y_min) y_min=y;
            if (y>y_max) y_max=y;

            BITMAP_PTR=(unsigned int*) bmp_write_line(BITMAP_KORVET,y);

            d_VGA_Ptr = BITMAP_PTR;
            for (x=0; x<64; x++) {
                c1=*GZU_Ptr++;
                c2=*GZU_Ptr++;
                c3=*GZU_Ptr++;
                c4=*GZU_Ptr++;

                //LUT_BASE_COLOR = 0x80
                *d_VGA_Ptr++=PlaneMask01[c1][0]|PlaneMask02[c2][0]|
                             PlaneMask04[c3][0]|PlaneMask08[c4][0]|0x80808080;
                *d_VGA_Ptr++=PlaneMask01[c1][1]|PlaneMask02[c2][1]|
                             PlaneMask04[c3][1]|PlaneMask08[c4][1]|0x80808080;
            } // for (in line loop)

            bmp_unwrite_line(BITMAP_KORVET);

            //update 2x bitmat if required
            if (blit2x_flag) {
                // scele line to 2x
                byte c;
                byte *src =(unsigned char*) BITMAP_PTR;
                byte *dst =(unsigned char*) bmp_write_line(BITMAP_KORVET2x,y*2);
                byte *dst2=(unsigned char*) bmp_write_line(BITMAP_KORVET2x,y*2+1);

                for (x=0;x<512;x++) {
                    c=*src++;
                    *dst++=c;
                    *dst++=c;
                    *dst2++=c;
                    *dst2++=c;
                }
            }
        }   // if Update
        else
        {   // Skip Line
            GZU_Ptr	    +=4*64;
            d_VGA_Ptr	+=2*64;
        };
    } // scr update loop

    if (lines_updated>0) {
      y_lines_to_scr=y_max-y_min+1;

      // void blit(BITMAP *source, BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height);
      if (blit2x_flag) {
          blit(BITMAP_KORVET2x, screen,
                0,y_min*2,
                SCREEN_OFFX, (y_min*2)+SCREEN_OFFY,
                512*2,y_lines_to_scr*2);
      } else {
          blit(BITMAP_KORVET, screen,
                0,y_min,
                SCREEN_OFFX, y_min+SCREEN_OFFY,
                512,y_lines_to_scr);
      }
    }

    AllScreenUpdateFlag=0;
    release_screen();
}

void SCREEN_Init(void) {
    // SetGraphics=SCREEN_SetGraphics;
    // SetText=SCREEN_SetText;
    // ShowScreen=SCREEN_ShowScreen;
}

// ====================================================================== SCREEN


