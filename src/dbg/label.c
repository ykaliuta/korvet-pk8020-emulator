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
#include "dbg.h"

#define FREEADDR 80000

_Label sLabel[MAXLABEL];

int TotalLabel=0;

/* Init label*/
void InitLabel(void)
{
 int i;
 TotalLabel=0;
 for(i=0;i<MAXLABEL;i++) sLabel[i].Addr = FREEADDR;
}


/* Fidn label by ADDR -> Return _Label PTR if found, else NULL*/
_Label *FindAddrLabel(word Addr)
{
 int i;
 for(i=0;i<TotalLabel;i++) if (sLabel[i].Addr == Addr) return &sLabel[i];
 return NULL;
}

/* Fidn label by Name -> Return _Label PTR if found, else NULL*/
_Label *FindNameLabel(char *Name)
{
 int i;//,l=_strlen(Name);
 char tmp[128];
 strcpy(tmp,Name);
 strupr(tmp);

 for(i=0;i<TotalLabel;i++) if (strcmp(sLabel[i].Name,tmp) == 0) return &sLabel[i];
 return NULL;
}

// Add New Label, if error return 0
int AddLabel(word Addr,char *Name)
{
 _Label *L;
 int i;

 if (isdigit(*Name)) return 0;

 if ((L=FindNameLabel(Name))) {return 0;}//L.Addr=Addr);return 1;} ; // Already Exist
 if ((L=FindAddrLabel(Addr))) {return 0;}//strcpy(L.Name,Name);return 1;} ; // Already Exist

 if (TotalLabel<MAXLABEL) // New Label
   {
    for (i=TotalLabel;i>0;i--)  // Insert Nel Label
      if (sLabel[i-1].Addr > Addr) sLabel[i]=sLabel[i-1];
      else {break;}

            sLabel[i].Addr=Addr;
    strncpy(sLabel[i].Name,Name,LABELLEN);
   strupr(sLabel[i].Name);

    TotalLabel++;
    return 1;
   }
 return 0;
}

// Delete Label, if Error return 0
int DeleteLabel(word Addr)
{
 _Label *L;
 int i;

 if (!(L=FindAddrLabel(Addr))) return 0;        // No Label
 for (i=0;i<TotalLabel;i++) if (sLabel[i].Addr == Addr) break;
 sLabel[i].Addr=FREEADDR;
 for (;i<TotalLabel;i++) sLabel[i]=sLabel[i+1];
 TotalLabel--;
 return 1;
}



