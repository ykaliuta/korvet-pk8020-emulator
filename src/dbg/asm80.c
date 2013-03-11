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
#include <stdlib.h>
#include <string.h>
#include "dbg.h"

#define ERR_Inavlid_Mnemo  -1
#define ERR_InvalidOperand -2

typedef struct
{
 char Name[4];
 int  Op1;
 int  Op2;
 int  Code;
 int  Hash;
} MNEMO;

MNEMO A_Tab[6]={{"CI"	,1,0,0xCE,0x18},
		{"DC"	,2,0,0x88,0x10},
		{"DD"	,2,0,0x80,0x12},
		{"DI"	,1,0,0xC6,0x1C},
		{"NA"	,2,0,0xA0,0x34},
		{"NI"	,1,0,0xE6,0x44},
		};
MNEMO C_Tab[13]={{"ALL"	,3,0,0xCD,0x42},
		{"C"	,3,0,0xDC,0x04},
		{"M"	,3,0,0xFC,0x18},
		{"MA"	,0,0,0x2F,0x30},
		{"MC"	,0,0,0x3F,0x34},
		{"MP"	,2,0,0xB8,0x4E},
		{"NC"	,3,0,0xD4,0x38},
		{"NZ"	,3,0,0xC4,0x66},
		{"P"	,3,0,0xF4,0x1E},
		{"PE"	,3,0,0xEC,0x44},
		{"PI"	,1,0,0xFE,0x4C},
		{"PO"	,3,0,0xE4,0x58},
		{"Z"	,3,0,0xCC,0x32},
		};
MNEMO D_Tab[7]={{"AA"	,0,0,0x27,0x00},
		{"AD"	,4,0,0x09,0x06},
		{"CR"	,2+0x10,0,0x05,0x2A},
		{"CX"	,4,0,0x0B,0x36},
		{"I"	,0,0,0xF3,0x10},
		{"B"	,7,0,0x00,0x02},
		{"W"	,8,0,0x00,0x2C},
		};
MNEMO E_Tab[1]={{"I"	,0,0,0xFB,0x10},
		};
MNEMO H_Tab[1]={{"LT"	,0,0,0x76,0x52},
		};
MNEMO I_Tab[3]={{"N"	,1,0,0xDB,0x1A},
		{"NR"	,2+0x10,0,0x04,0x56},
		{"NX"	,4,0,0x03,0x62},
		};
MNEMO J_Tab[9]={{"MP"	,3,0,0xC3,0x4E},
		{"C"	,3,0,0xDA,0x04},
		{"M"	,3,0,0xFA,0x18},
		{"NC"	,3,0,0xD2,0x38},
		{"NZ"	,3,0,0xC2,0x66},
		{"P"	,3,0,0xF2,0x1E},
		{"PE"	,3,0,0xEA,0x44},
		{"PO"	,3,0,0xE2,0x58},
		{"Z"	,3,0,0xCA,0x32},
		};
MNEMO L_Tab[4]={{"DA"	,3,0,0x3A,0x0C},
		{"DAX"	,5+0x10,0,0x0A,0x46},
		{"HLD"	,3,0,0x2A,0x6A},
		{"XI"	,4,3,0x01,0x6C},
		};
MNEMO M_Tab[2]={{"OV"	,2+0x10,2,0x40,0x62},
		{"VI"	,2+0x10,1,0x06,0x64},
		};
MNEMO N_Tab[1]={{"OP"	,0,0,0x00,0x56},
		};
MNEMO O_Tab[3]={{"RA"	,2,0,0xB0,0x44},
		{"RI"	,1,0,0xF6,0x54},
		{"UT"	,1,0,0xD3,0x76},
		};
MNEMO P_Tab[3]={{"CHL"	,0,0,0xE9,0x42},
		{"OP"	,5,0,0xC1,0x56},
		{"USH"	,5,0,0xC5,0xF6},
		};
MNEMO R_Tab[14]={{"AL"	,0,0,0x17,0x16},
		{"AR"	,0,0,0x1F,0x22},
		{"ET"	,0,0,0xC9,0x36},
		{"C"	,0,0,0xD8,0x04},
		{"M"	,0,0,0xF8,0x18},
		{"NC"	,0,0,0xD0,0x38},
		{"NZ"	,0,0,0xC0,0x66},
		{"P"	,0,0,0xF0,0x1E},
		{"PE"	,0,0,0xE8,0x44},
		{"PO"	,0,0,0xE0,0x58},
		{"Z"	,0,0,0xC8,0x32},
		{"LC"	,0,0,0x07,0x30},
		{"RC"	,0,0,0x0F,0x48},
		{"ST"	,6,0,0xC7,0x6E},
		};
MNEMO S_Tab[9]={{"BB"	,2,0,0x98,0x06},
		{"BI"	,1,0,0xDE,0x14},
		{"HLD"	,3,0,0x22,0x6A},
		{"PHL"	,0,0,0xF9,0xAA},
		{"TA"	,3,0,0x32,0x4C},
		{"TAX"	,5+0x10,0,0x02,0xC6},
		{"TC"	,0,0,0x37,0x50},
		{"UB"	,2,0,0x90,0x52},
		{"UI"	,1,0,0xD6,0x60},
		};
MNEMO X_Tab[4]={{"CHG"	,0,0,0xEB,0x38},
		{"RA"	,2,0,0xA8,0x44},
		{"RI"	,1,0,0xEE,0x54},
		{"THL"	,0,0,0xE3,0xCA},
		};
MNEMO *Bukwa1[26]={A_Tab,NULL,C_Tab,D_Tab,E_Tab,NULL,NULL,H_Tab,I_Tab,J_Tab,NULL,L_Tab,M_Tab,N_Tab,O_Tab,P_Tab,NULL,R_Tab,S_Tab,NULL,NULL,NULL,NULL,X_Tab,NULL,NULL,};
int BSize[26]={6,0,13,7,1,0,0,1,3,9,0,4,2,1,3,3,0,14,9,0,0,0,0,4,0,0,};
int    ASMStr   (char *Sstr,byte *Sdst);
MNEMO *FindMnemo(char *src);
char  *GetValue (char *s,int *Value,int *ERR);
char  *GetWord  (char *s,char *d);
char  *SkipSpc  (char *s);


/************************* 29.01.99 *************************
**                   Найти мнемонику                        **
* Ret: =NULL если нет такой, иначе *на MNEMO                 *
**************************************************************/

MNEMO *FindMnemo(char *src)
{
 MNEMO *MN=Bukwa1[*src-'A'];
 MNEMO *tmp=NULL;
 int   Len =BSize[*src-'A'];
 int   Hash=0;
 int   i;

 src++;                         //Skip first letter

 for (i=0;i<strlen(src);i++) Hash=(Hash+(src[i]-'A'))<<1; //Calc mnemonic HASH

 for (i=0;i<Len;i++) if ( MN[i].Hash == Hash) {tmp=&MN[i];break;}

 return tmp;
}

char *SkipSpc(char *s)
{
 while (*s && isspace(*s)) s++;
 return s;
}

char *GetWord(char *s,char *d)
{
 while (*s && !isspace(*s)) *d++=*s++;
 *d++='\0';
 return s;
}

char *GetValue(char *s,int *Value,int *ERR)
{
 char *SR,*tmp;
 int  V=0;
 int  Hex;
 int  TryLabel=0;
 char Lbl[128];
_Label *L;

 SR=SkipSpc(s);

 if (*s == '\0') {*ERR=ERR_InvalidOperand;return s;};

 if (!isxdigit(*SR)) TryLabel=1;

 while (*SR && isxdigit(*SR) ) {
   Hex=*SR | 0x20;
   Hex=(Hex > '9')?Hex-'a'+10:Hex-'0';
   V=(V<<0x4) | Hex;
   SR++;
 }

 if ((*SR != '\0' && *SR != ' ') || TryLabel) {
   tmp=Lbl;
   SR=SkipSpc(s);

   while ( *SR && ( *SR>='0' && *SR <= '_' ) ) *tmp++=*SR++;
   *tmp++='\0';

   if (!(L=FindNameLabel(Lbl)))  {*ERR=ERR_InvalidOperand;return 0;}
   V=L->Addr;
 }

 *Value=V;
 return SR;
}

/************************* 29.01.99 *************************
**          Ассемблировать строку из src в dst              **
* Ret: >=0 - длина комманды (0-служебная) <0 - Ошибка        *
**************************************************************/
int ASMStr(char *Sstr,byte *Sdst)
{

 char  IntBuf[128];
 char  *src=IntBuf;
 byte  *dst=Sdst;

 MNEMO *MN;

 char  tR1[]="BCDEHLMA";

 char  tmp[100];
 char  *ptmp;
 int   op1=0,op2=0,mask1=0,mask2=0,len=0;
 int   i;
 int   ERR=0;

 strcpy(src,Sstr);

 ptmp=strchr(src,';'); if (ptmp) *ptmp='\0'; // comment
 ptmp=strchr(src,':'); if (ptmp) *ptmp='\0'; // Label:0000

 strupr(src);
 src=SkipSpc(src);
// if (*src == '\0') return -1;

 src=GetWord(src,tmp);
 src=SkipSpc(src);

 if ( (MN=FindMnemo(tmp)) == NULL ) {
//   printf(" ! Error: invalid mnemonic %s\n",tmp);
   return ERR_Inavlid_Mnemo;
 }

// printf(" op1=%d, op2=%d, code=%x\n",MN->Op1,MN->Op2,MN->Code);

 switch ((MN->Op1) & 0x0f) {
   case  0:    { break;}                                 // NOP
   case  1:    {                                         // #xx
                 src=GetValue(src,&op1,&ERR);
                 if (op1>0xff) return ERR_InvalidOperand;
                 if (ERR) return ERR;
                 len++;
                 break;
   }
   case  2:    {                                        // r = BCDEHLMA
                 if (!(ptmp=strchr(tR1,*src)) ) return ERR_InvalidOperand;
                 mask1=ptmp-tR1;
                 src++;
                 break;
   }
   case  3:    {                                       // #xxxx
                 src=GetValue(src,&op1,&ERR);
                 if (ERR) return ERR;
                 op2=(op1&0xff00)>>8;
                 op1&=0xff;
                 len+=2;
                 break;
   }
   case  4:    {                                       // rp= BC DE HL SP
                switch (*src++) {
                  case 'B': { mask1=0x00; break;}
                  case 'D': { mask1=0x10; break;}
                  case 'H': { mask1=0x20; break;}
                  case 'S': { if ((*src++) == 'P') mask1=0x30; else return ERR_InvalidOperand; break;}
                  default : return ERR_InvalidOperand;
                }
                break;
   }
   case  5:    {                                       // rp= BC DE HL PSW
                switch (*src++) {
                  case 'B': { mask1=0x00; break;}
                  case 'D': { mask1=0x10; break;}
                  case 'H': { mask1=0x20; break;}
                  case 'P': { if ( (*src == 'S') && (*(src+1) == 'W') ) {src+=2;mask1=0x30;} else return ERR_InvalidOperand; break;}
                  default : return ERR_InvalidOperand;
                }
                src++;
                break;
   }
   case  6:    {                                         // RST #
                 src=GetValue(src,&mask1,&ERR);
                 if (mask1>0x7) return ERR_InvalidOperand;
                 if (ERR) return ERR;
                 mask1<<=3;
                 break;
   }
   case  7:    {                                         // db
                 src=GetValue(src,&op1,&ERR);
                 if (op1>0xff) return ERR_InvalidOperand;
                 MN->Code=op1;
                 op1=0;
                 break;
   }
   case  8:    {                                        // dw
                 src=GetValue(src,&op1,&ERR);
                 if (ERR) return ERR;
                 MN->Code=op1&0xff;
                 op1=(op1&0xff00)>>8;
                 len++;
                 break;
   }
 }
 if  ((MN->Op1) == 0x12) mask1<<=3;
 if  ((MN->Op1) == 0x15) mask1&=0x10;

 src=SkipSpc(src);

 if ( MN->Op2 ) {
   if (*src != ',') return ERR_InvalidOperand;
   src++;
   src=SkipSpc(src);
 } else if (*src != '\0') return ERR_InvalidOperand;

 switch ( MN->Op2 ) {
   case  1:    {                                         // #xx
                 src=GetValue(src,&op1,&ERR);
                 if (op2>0xff) return ERR_InvalidOperand;
                 if (ERR) return ERR;
                 len++;
                 break;
   }
   case  2:    {                                        // r = BCDEHLMA
                 if (!(ptmp=strchr(tR1,*src)) ) return ERR_InvalidOperand;
                 mask2=ptmp-tR1;
                 src++;
                 break;
   }
   case  3:    {                                       // #xxxx
                 src=GetValue(src,&op1,&ERR);
                 if (ERR) return ERR;
                 op2=(op1&0xff00)>>8;
                 op1&=0xff;
                 len+=2;
                 break;
   }
 }

 src=SkipSpc(src);
 if (*src != '\0') return ERR_InvalidOperand;

 *dst++=MN->Code | mask1 | mask2;
 if (len--) *dst++=op1;
 if (len==1) *dst++=op2;

 return (dst-Sdst);
}


