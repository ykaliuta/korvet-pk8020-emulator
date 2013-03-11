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

/*
RIFF	c	4	RIFF
????	u32	1	Len
WAVE	c	4	DataID
fmt 	c	4	RIFF
16	u32	1	Len
1	w	1	wFormatTag
1	u16	1	wChanels
22050	u32	1	SamplesPerSec
22050	u32	1	AvgSamplesPerSec
1	u16	1	wBlockAllign	
8	u16	1	wBitsPerSample; 
data	c	4	RIFF
????	u32	1	Len
	b	l	Data
*/

struct WAVE_HEADER {
       char             RIFF[4];
       unsigned int     LEN;
       char             WAVE[4];
       char             FMT[4];
       unsigned int     Len16;
       short int        w1;
       short int        Chanel;
       unsigned int     SPS;
       unsigned int     ASPS;
       short int        BA;
       short int        BPS;
       char             DATA[4];
       unsigned int     LEN2;
} __attribute__ ((packed));

#define WAVBUFLEN (1024*10)

struct WAVE_HEADER WAV_Hdr={"RIFF",0,"WAVE","fmt ",16,1,1,22050,22050,1,8,"data",0};

FILE *FileWAV;
unsigned char WAV_Buf[WAVBUFLEN];
static int WAV_Ptr;
static unsigned int WAV_Len;

int OpenWAV(char *FileName) {
    FileWAV=fopen(FileName,"wb");
    fwrite(&WAV_Hdr,sizeof(WAV_Hdr),1,FileWAV);
    if (!FileWAV) return -1;
    WAV_Len=0;
    WAV_Ptr=0;
    return 0;
}

void CloseWAV(void) {
    if (!FileWAV) return;
    if (WAV_Ptr) {
      fwrite((void *)WAV_Buf,WAV_Ptr,1,FileWAV);
      WAV_Len+=WAV_Ptr;
    }
    if (WAV_Len) {
      fseek(FileWAV,0,SEEK_SET);
      WAV_Hdr.LEN=WAV_Len+118;
      WAV_Hdr.LEN2=WAV_Len;
      fwrite(&WAV_Hdr,sizeof(WAV_Hdr),1,FileWAV);
    }
    fclose(FileWAV);
}

void AddWAV(unsigned char *BUF,int Len) {
    if (WAV_Ptr+Len > WAVBUFLEN) {
      fwrite(WAV_Buf,WAV_Ptr,1,FileWAV);
      WAV_Ptr=0;
    }
    memcpy((void *)WAV_Buf+WAV_Ptr,(void *)BUF,Len);
    WAV_Ptr+=Len;
    WAV_Len+=Len;
}


