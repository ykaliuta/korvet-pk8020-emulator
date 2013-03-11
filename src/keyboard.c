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
#include <allegro.h>

#define noKEYBOARD_DEBUG

int KeyboardLayout;               // mode
int KeyboadUpdateFlag=0;          // =1 if need rebuld keyboard layout

int KeyAlias[256];
#define MAXKEY 115

extern int KBD_LED;

int AliasTab[]={
//                 Alias      => Key
                 KEY_DOWN      , KEY_2_PAD,
                 KEY_LEFT      , KEY_4_PAD,
                 KEY_RIGHT     , KEY_6_PAD,
                 KEY_UP        , KEY_8_PAD,
                 KEY_ENTER_PAD , KEY_ENTER,
               };
#define MAXALIAS 5

/*
int KBD0[8][12]={
{0		,KEY_H,KEY_P,KEY_X	,KEY_0	,KEY_8		,KEY_ENTER	,KEY_LSHIFT	,KEY_0_PAD,KEY_8_PAD	,KEY_F1},
{KEY_A		,KEY_I,KEY_Q,KEY_Y	,KEY_1	,KEY_9		,KEY_PGUP	,KEY_ALT	,KEY_1_PAD,KEY_9_PAD	,KEY_F2},
{KEY_B		,KEY_J,KEY_R,KEY_Z	,KEY_2	,KEY_COLON	,KEY_HOME	,KEY_ALTGR	,KEY_2_PAD,0		,KEY_F3},
{KEY_C		,KEY_K,KEY_S,KEY_OPENBRACE,KEY_3,KEY_TILDE	,KEY_INSERT	,KEY_ESC	,KEY_3_PAD,0		,KEY_F4},
{KEY_D		,KEY_L,KEY_T,KEY_TILDE	,KEY_4	,KEY_COMMA	,KEY_DEL	,KEY_RCONTROL	,KEY_4_PAD,0		,KEY_F5},
{KEY_E		,KEY_M,KEY_U,KEY_CLOSEBRACE,KEY_5,KEY_MINUS	,KEY_BACKSPACE	,KEY_LCONTROL	,KEY_5_PAD,0		,},
{KEY_F		,KEY_N,KEY_V,KEY_COLON	,KEY_6	,KEY_STOP	,KEY_TAB	,KEY_CAPSLOCK	,KEY_6_PAD,KEY_DEL_PAD	,},
{KEY_G		,KEY_O,KEY_W,KEY_BACKSLASH,KEY_7,KEY_SLASH	,KEY_SPACE	,KEY_RSHIFT	,KEY_7_PAD,0		,},
};
*/
/*
 		         D0  D1  D2  D3  D4  D5  D6  D7
		       |   |   |   |   |   |   |   |   |
    ŒÒÌÓ‚ÌÓÂ ÔÓÎÂ:     |   |   |   |   |   |   |   |   |

			01  02  04  08  10  20  40  80

      KB00    01H        ﬁ@  ¿a  ¡b  ÷c  ƒd  ≈e  ‘f  √g
      KB01    02H        ’h  »i  …j   k  Àl  Ãm  Õn  Œo
      KB02    04H        œp  ﬂq  –r  —s  “t  ”u  ∆v  ¬w
      KB03    08H        ‹x  €y  «z  ÿ{  ›|  Ÿ}  ◊`  ˙_
      KB04    10H        0   1   2   3   4   5   6   7
      KB05    20H        8   9   *:  +;  <,  =-  >.  /?
      KB06    40H       ¬   s“– —“œ  »«  ¬«  ¬ÿ “¿¡ œ–Œ¡≈À
      			    CLS STO DEL INS  BS TAB SPACE
      KB07    80H       –√À ¿À‘ √–‘ œ–‘ —≈À ”œ– ‘ — –√œ
			ShL ALF GRP ESC SEL CTR LOCK ShL
    ƒÓÔÓÎÌËÚÂÎ¸ÌÓÂ ÔÓÎÂ:
      KB08   101H        0   1   2   3   4   5   6   7
      KB09   102H        8   9                   .
      KB10   104H        F1  F2  F3  F4  F5  F6  F7  F8
      KB11   108H        F9  F10 */


// QWERTY Keyboard MAP
int KBD0[8][12]={
//     01       02     04     08             10     20         40              80            101        102         104
/*01*/{KEY_PGDN,KEY_H ,KEY_P ,KEY_X 	    ,KEY_0 ,KEY_8     ,KEY_ENTER    ,KEY_LSHIFT	  ,KEY_0_PAD ,KEY_8_PAD	 ,KEY_F1},
/*02*/{KEY_A   ,KEY_I ,KEY_Q ,KEY_Y	    ,KEY_1 ,KEY_9     ,KEY_HOME	    ,KEY_ALT	  ,KEY_1_PAD ,KEY_9_PAD	 ,KEY_F2},
/*04*/{KEY_B   ,KEY_J ,KEY_R ,KEY_Z	    ,KEY_2 ,KEY_COLON ,KEY_PGUP	    ,KEY_ALTGR	  ,KEY_2_PAD ,254        ,KEY_F3},
/*08*/{KEY_C   ,KEY_K ,KEY_S ,KEY_OPENBRACE ,KEY_3 ,KEY_TILDE ,KEY_INSERT   ,KEY_ESC	  ,KEY_3_PAD ,254 	 ,KEY_F4},
/*10*/{KEY_D   ,KEY_L ,KEY_T ,KEY_BACKSLASH ,KEY_4 ,KEY_COMMA ,KEY_DEL	    ,KEY_RCONTROL ,KEY_4_PAD ,254	 ,KEY_F5},
/*20*/{KEY_E   ,KEY_M ,KEY_U ,KEY_CLOSEBRACE,KEY_5 ,KEY_MINUS ,KEY_BACKSPACE,KEY_LCONTROL ,KEY_5_PAD ,254	 ,254},
/*40*/{KEY_F   ,KEY_N ,KEY_V ,KEY_END	    ,KEY_6 ,KEY_STOP  ,KEY_TAB	    ,KEY_CAPSLOCK ,KEY_6_PAD ,KEY_DEL_PAD,254},
/*80*/{KEY_G   ,KEY_O ,KEY_W ,KEY_EQUALS    ,KEY_7 ,KEY_SLASH ,KEY_SPACE    ,KEY_RSHIFT	  ,KEY_7_PAD ,254	 ,254},
};

// …÷” ≈Õ Keyboard MAP
int KBD1[8][12]={
//     01        02            04         08             10     20         40             80              101       102           104
/*01*/{KEY_STOP ,KEY_OPENBRACE,KEY_G     ,KEY_M         ,KEY_0 ,KEY_8	 ,KEY_ENTER	,KEY_LSHIFT	,KEY_0_PAD,KEY_8_PAD	,KEY_F1},
/*02*/{KEY_F	,KEY_B        ,KEY_Z     ,KEY_S         ,KEY_1 ,KEY_9	 ,KEY_PGUP	,KEY_ALT	,KEY_1_PAD,KEY_9_PAD	,KEY_F2},
/*04*/{KEY_COMMA,KEY_Q        ,KEY_H     ,KEY_P         ,KEY_2 ,0         ,KEY_HOME	,KEY_ALTGR	,KEY_2_PAD,0		,KEY_F3},
/*08*/{KEY_W	,KEY_R        ,KEY_C     ,KEY_I         ,KEY_3 ,KEY_TILDE ,KEY_INSERT	,KEY_ESC	,KEY_3_PAD,0		,KEY_F4},
/*10*/{KEY_L	,KEY_K        ,KEY_N     ,KEY_QUOTE     ,KEY_4 ,0	 ,KEY_DEL	,KEY_RCONTROL	,KEY_4_PAD,0		,KEY_F5},
/*20*/{KEY_T	,KEY_V        ,KEY_E     ,KEY_O         ,KEY_5 ,KEY_MINUS ,KEY_BACKSPACE	,KEY_LCONTROL	,KEY_5_PAD,0		,},
/*40*/{KEY_A	,KEY_Y        ,KEY_COLON ,KEY_X         ,KEY_6 ,0	 ,KEY_TAB	,KEY_CAPSLOCK	,KEY_6_PAD,KEY_DEL_PAD	,},
/*80*/{KEY_U	,KEY_J        ,KEY_D     ,KEY_CLOSEBRACE,KEY_7 ,KEY_SLASH ,KEY_SPACE	,KEY_RSHIFT	,KEY_7_PAD,0		,},
};

int KBD[8][12];

void KBD_Select() {
  int x,y;
  int WorkLayout=KeyboardLayout;

  if (WorkLayout == KBD_AUTO) {
     if (KBD_LED == 2) {WorkLayout=KBD_QWERTY;}
     else {WorkLayout=KBD_JCUKEN;}
  }

  for (x=0;x<8;x++)
   for (y=0;y<12;y++) 
     KBD[x][y]=(WorkLayout == KBD_QWERTY)?KBD0[x][y]:KBD1[x][y];

  KeyboadUpdateFlag=0;
}

void KBD_Init(void) {
  KeyboadUpdateFlag=1;
//  KeyboardLayout=KBD_AUTO;
  KBD_Select();
}

int ChkMattrixLine(int N)
{
  int Value=0;
  Value|=KeyAlias[KBD[0][N]]?0x01:0;
  Value|=KeyAlias[KBD[1][N]]?0x02:0;
  Value|=KeyAlias[KBD[2][N]]?0x04:0;
  Value|=KeyAlias[KBD[3][N]]?0x08:0;
  Value|=KeyAlias[KBD[4][N]]?0x10:0;
  Value|=KeyAlias[KBD[5][N]]?0x20:0;
  Value|=KeyAlias[KBD[6][N]]?0x40:0;
  Value|=KeyAlias[KBD[7][N]]?0x80:0;
  return Value;
}



int KEYBOARD_Read(int Addr) {
  int Value=0;
  int Line=0;
  int i;

  if (KeyboadUpdateFlag) {KBD_Select();}

  for (i=0;i<MAXKEY;i++)      {KeyAlias[i]=key[i];}
  for (i=0;i<MAXALIAS*2;i+=2) {if (KeyAlias[AliasTab[i]]) KeyAlias[AliasTab[i+1]]=1;}

  if (Addr&0x100) Line=8;
  
  if (Addr&0x0001) Value|=ChkMattrixLine(Line);Line++;
  if (Addr&0x0002) Value|=ChkMattrixLine(Line);Line++;
  if (Addr&0x0004) Value|=ChkMattrixLine(Line);Line++;
  if (Addr&0x0008) Value|=ChkMattrixLine(Line);Line++;
  if (Addr&0x0010) Value|=ChkMattrixLine(Line);Line++;
  if (Addr&0x0020) Value|=ChkMattrixLine(Line);Line++;
  if (Addr&0x0040) Value|=ChkMattrixLine(Line);Line++;
  if (Addr&0x0080) Value|=ChkMattrixLine(Line);Line++;

#ifdef KEYBOARD_DEBUG
  textprintf(screen,font,450,50,255,"KBD_R: %04x:%02x",Addr,Value);
#endif

  return Value;
}

char *PrintKBD(int Addr,int y) {

 static char BUFFER[512];
 int Mask=0x80;
 int i=14;
 char *ptr=BUFFER+6;
 int Value=KEYBOARD_Read(Addr);

 char KBD_TXT[13][32]={
                       "ﬁ@¿a¡b÷cƒd≈e‘f√g",
                       "’h»i…j kÀlÃmÕnŒo",
                       "œpﬂq–r—s“t”u∆v¬w",
                       "‹x€y«zÿ{›|Ÿ}◊`˙_",
                       " 0 1 2 3 4 5 6 7",
                       " 8 9 *:+;<,=->./",
                       " C S D I B T S  ",
                       " L A G E S C L R",
                       " 0 1 2 3 4 5 6 7",
                       " 8 9         .  ",
                       " 1 2 3 4 5 6 7 8",
                       " 8 9            ",
                       "11111111111111111",
     };


 sprintf(BUFFER,"%04x: ",Addr);

 while (Mask) {
   if ((Value & Mask) == 0) {*ptr++=KBD_TXT[y][i+1];}
   else *ptr++='_';
   Mask>>=1;
   i-=2;
 }

 *ptr='\0';
 return BUFFER;
}

#ifdef nDBG
void ShowKBDdbg(void) {

 int i=0;
 int x=850;
 int y=550;

 textprintf(screen,font,x,y+16*i++,0x20+0x07,"%s",PrintKBD(0xf801,0));
 textprintf(screen,font,x,y+16*i++,0x20+0x07,"%s",PrintKBD(0xf802,1));
 textprintf(screen,font,x,y+16*i++,0x20+0x07,"%s",PrintKBD(0xf804,2));
 textprintf(screen,font,x,y+16*i++,0x20+0x07,"%s",PrintKBD(0xf808,3));
 textprintf(screen,font,x,y+16*i++,0x20+0x07,"%s",PrintKBD(0xf810,4));
 textprintf(screen,font,x,y+16*i++,0x20+0x07,"%s",PrintKBD(0xf820,5));
 textprintf(screen,font,x,y+16*i++,0x20+0x07,"%s",PrintKBD(0xf840,6));
 textprintf(screen,font,x,y+16*i++,0x20+0x07,"%s",PrintKBD(0xf880,7));
 i++;
 textprintf(screen,font,x,y+16*i++,0x20+0x07,"%s",PrintKBD(0xf901,8));
 textprintf(screen,font,x,y+16*i++,0x20+0x07,"%s",PrintKBD(0xf902,9));
 textprintf(screen,font,x,y+16*i++,0x20+0x07,"%s",PrintKBD(0xf904,10));
 textprintf(screen,font,x,y+16*i++,0x20+0x07,"%s",PrintKBD(0xf908,11));
 i++;
// textprintf(screen,font,x,y+16*i++,0x20+0x07,"%s",PrintKBD(0xf8ff,12));
// textprintf(screen,font,x,y+16*i++,0x20+0x07,"%s",PrintKBD(0xf9ff,12));
}
#endif
