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
#include <stdio.h>

#include "allegro.h"
#include "vg.h"

extern char Disks[4][1024];
extern int BW_Flag;
extern int LutUpdateFlag;
extern int AllScreenUpdateFlag;
extern int KeyboardLayout;
extern int KeyboadUpdateFlag;

int dofile_proc(int msg, DIALOG *d, int c,int Drive)
{
   int ret;
   int i;

   char message[1024];
   char path[1024];
   char ext[]="kdi;bin;mst;";

   /* call the parent object */
   ret = d_button_proc(msg, d, c);
  
   if (ret == D_CLOSE) {
     sprintf(message,"Select file for Drive '%c'",Drive+'A');
     strcpy(path,Disks[Drive]);
     i=file_select_ex(message,path,ext,sizeof(path)-1,0,0 );
     if (i) {
        i=VG.System;
        VG.System=Drive;
        VG.OperIO=0x0b;
        DskVG();

        strcpy(Disks[Drive],path);

        VG.OperIO=0x0a;
        DskVG();
        VG.System=i;
     }
     return D_REDRAW; 
   }

   return ret;
}

int fileA(int msg, DIALOG *d, int c) {
 return dofile_proc(msg,d,c,0);
}

int fileB(int msg, DIALOG *d, int c) {
 return dofile_proc(msg,d,c,1);
}

int fileC(int msg, DIALOG *d, int c) {
 return dofile_proc(msg,d,c,2);
}

int fileD(int msg, DIALOG *d, int c) {
 return dofile_proc(msg,d,c,3);
}



DIALOG dlg_korvet[] =
{
   /* (proc)         (x)  (y) (w)  (h) (fg) (bg) (key) (flags) (d1) (d2) (dp)           (dp2) (dp3) */
   { d_box_proc,     24,  12, 592+16, 72+8*7, 0,   0,   0,    0,      0,   0,   NULL,          NULL, NULL },
   { d_check_proc,   492, 94, 48,  12, 0,   0,   0,    0,      1,   0,   "mono",        NULL, NULL },

   { d_box_proc,     488, 36, 72+16,  40+14, 0,   0,   0,    0,      0,   0,   NULL,          NULL, NULL },
   { d_radio_proc,   492, 44, 64,  8,  0,   0,   0,    0,      0,   0,   "qwerty",      NULL, NULL },
   { d_radio_proc,   492, 60, 64,  8,  0,   0,   0,    0,      0,   0,   "jcuken",      NULL, NULL },
   { d_radio_proc,   492, 76, 64,  8,  0,   0,   0,    0,      0,   0,   " AUTO ",      NULL, NULL },

   { d_textbox_proc, 272, 16, 96,  32, 0,   0,   0,    0,      0,   0,   "Korvet MENU", NULL, NULL },
   { d_box_proc,     28,  36, 456, 40+8*4, 0,   0,   0,    0,      0,   0,   NULL,          NULL, NULL },
   { d_text_proc,    32,  40, 16,  16,  0,   0,   0,    0,      0,   0,   "A ",          NULL, NULL },
   { d_edit_proc,    48,  40, 408, 16,  0,   0,   0,    0,      4,   0,   Disks[0],      NULL, NULL },
   { fileA,          456, 40, 24,  16,  0,   0,   0,    D_EXIT, 0,   0,   "...",         NULL, NULL },
   { d_text_proc,    32,  48+8*1, 16,  16,  0,   0,   0,    0,      0,   0,   "B ",          NULL, NULL },
   { d_edit_proc,    48,  48+8*1, 408, 16,  0,   0,   0,    0,      4,   0,   Disks[1],      NULL, NULL },
   { fileB,          456, 48+8*1, 24,  16,  0,   0,   0,    D_EXIT, 0,   0,   "...",         NULL, NULL },
   { d_text_proc,    32,  56+8*2, 16,  16,  0,   0,   0,    0,      0,   0,   "C ",          NULL, NULL },
   { d_edit_proc,    48,  56+8*2, 408, 16,  0,   0,   0,    0,      4,   0,   Disks[2],      NULL, NULL },
   { fileC,          456, 56+8*2, 24,  16,  0,   0,   0,    D_EXIT, 0,   0,   "...",         NULL, NULL },
   { d_text_proc,    32,  64+8*3, 16,  16,  0,   0,   0,    0,      0,   0,   "D ",          NULL, NULL },
   { d_edit_proc,    48,  64+8*3, 408, 16,  0,   0,   0,    0,      4,   0,   Disks[3],      NULL, NULL },
   { fileD,          456, 64+8*3, 24,  16,  0,   0,   0,    D_EXIT, 0,   0,   "...",         NULL, NULL },
   { d_button_proc,  584, 60+8*3, 28,  16, 0,   0,   0,    D_EXIT, 0,   0,   "Ret",         NULL, NULL },
   { NULL,           0,   0,  0,   0,  0,   0,   0,    0,      0,   0,   NULL,          NULL, NULL }
};

void GUI(void)
{
   int ret;

   /* We set up colors to match screen color depth (in case it changed) */
   for (ret = 0; dlg_korvet[ret].proc; ret++) {
      dlg_korvet[ret].fg = makecol(0, 0, 0);
      dlg_korvet[ret].bg = makecol(255, 255, 255);
   }

   dlg_korvet[1].flags = (BW_Flag)?D_SELECTED:0;

   dlg_korvet[3].flags = (KeyboardLayout == KBD_QWERTY)?D_SELECTED:0;
   dlg_korvet[4].flags = (KeyboardLayout == KBD_JCUKEN)?D_SELECTED:0;
   dlg_korvet[5].flags = (KeyboardLayout == KBD_AUTO  )?D_SELECTED:0;

   /* do the dialog */
   ret = popup_dialog(dlg_korvet, -1);

   /* and report the results */
   BW_Flag=(dlg_korvet[1].flags == D_SELECTED)?1:0;

   if (dlg_korvet[3].flags == D_SELECTED) KeyboardLayout=KBD_QWERTY;
   if (dlg_korvet[4].flags == D_SELECTED) KeyboardLayout=KBD_JCUKEN;
   if (dlg_korvet[5].flags == D_SELECTED) KeyboardLayout=KBD_AUTO;
   KeyboadUpdateFlag=1;

//   KBD_Select(KeyboardLayout);

   LutUpdateFlag=1;
   AllScreenUpdateFlag=1;
}
