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

//extern screen,font;
#ifdef TRACETIMER
extern int Takt;
extern F_TIMER;
#endif


// -------------------------------------------------------------------------
// Переменная из модуля Звука (Флаг разрешения звука)
extern int SoundEnable;
extern int Takt;
int PrevTakt;
int MuteFlag=0;

// -------------------------------------------------------------------------
#define MAXBUF 50000
byte TIMERBUF[MAXBUF*2];
int  BytePtr=0;

byte SOUNDBUF[MAXBUF*2];

// -------------------------------------------------------------------------
struct _TIMER {
	word	CountingElement;
	word	CountRegister;
	word	OutputLatch;
	int 	Mode;
	int	BCDFlag;
	int	RWMode;

	int	LatchCounter; 	// Определяет какой байт сейчас читается если
				// залочено.
	int	WriteCounter;	// Определяет какой байт сейчас пишется 
	int	ReadCounter;	// Определяет какой байт сейчас читается
	
	int 	OUT;	
	int	StartFlag;	// 1 если счетчик считает
        int	NextTickLoad;	// 1 если на след. такте надо загрузить счетчик
};

struct _TIMER i8253[4];
//

// OUT_ routines

inline void ADD_OUT(int CH,int Value) {
  if (!MuteFlag) {
    TIMERBUF[BytePtr]=Value&SoundEnable;
  }
  if (BytePtr < MAXBUF) BytePtr++;
}

// MODE 0: INTERRUPT ON TERMINAL COUNT ----------------------------------------

void mode0_setmode(int Counter) {
  i8253[Counter].OUT=0;
  i8253[Counter].NextTickLoad=1;
  i8253[Counter].StartFlag=1;
};

void mode0_WriteNewValue(int Counter,int RWMode) {
  if (RWMode == 3) { // 2 byte load
    i8253[Counter].OUT=0;
  } else {
    i8253[Counter].NextTickLoad=1;
    i8253[Counter].StartFlag   =1;
  }
  // ПОКА НЕ РЕАЛИЗОВАНО !!!!!!!!!!!!!!!!!!!!!
  // когда сечетчик считает, загрузка нового LSB перед тем как счетчик
  // дойдет до 0 и пеерскочит на FFFF, останавливает счетчик.
  // однако, если LSB загружен ПОСЛЕ того как счетчик перескоил в FFFF
  // так что MSB теперь присутсвует в счетчике, тогда счетчик не останавливается.
}

void mode0_do(int Counter,int Takt) {

   if (i8253[Counter].NextTickLoad) {
      i8253[Counter].CountingElement=i8253[Counter].CountRegister;
      i8253[Counter].NextTickLoad=0;
      Takt--;
   }
 
   while (Takt--) {
     if (i8253[Counter].StartFlag) {
        if ( i8253[Counter].CountingElement-- ) {
            ADD_OUT(Counter,i8253[Counter].OUT);
            i8253[Counter].OUT=0;
        } else {
            i8253[Counter].StartFlag=0;
            i8253[Counter].OUT=1;
        };
     } else {
            ADD_OUT(Counter,i8253[Counter].OUT); //???? may be 1 ???
     }
   } 
}

// MODE 1: HARDWARE RETRIGGRTABLE ONE-SHOT ------------------------------------
void mode1_setmode(int Counter) {
  i8253[Counter].OUT=1;
  i8253[Counter].StartFlag=0;
};
void mode1_WriteNewValue(int Counter,int RWMode) {}
void mode1_do(int Counter,int Takt) {
   while (Takt--) {
     ADD_OUT(Counter,i8253[Counter].OUT); //???? may be 1 ???
   } 
}

// MODE 2: RATE GENERATOR -----------------------------------------------------
void mode2_setmode(int Counter) {
  i8253[Counter].OUT=1;
  i8253[Counter].NextTickLoad=1;
  i8253[Counter].StartFlag=0;
};
void mode2_WriteNewValue(int Counter,int RWMode) {
  if (RWMode == 3) { // 2 byte load
  } else {
    i8253[Counter].NextTickLoad=1;
    i8253[Counter].StartFlag=1;
  }
}

void mode2_do(int Counter,int Takt) {

   if (i8253[Counter].NextTickLoad) {
      i8253[Counter].CountingElement=i8253[Counter].CountRegister;
      i8253[Counter].NextTickLoad=0;
   }

   while (Takt--) {
     if (i8253[Counter].StartFlag) {
       if (1 == i8253[Counter].CountingElement-- ) {
         ADD_OUT(Counter,0);
         i8253[Counter].CountingElement=i8253[Counter].CountRegister;
       }
     } else {
       ADD_OUT(Counter,1);
     }
   } 
}

// MODE 3: SQUARE WAVE MODE ---------------------------------------------------
void mode3_setmode(int Counter) {
  i8253[Counter].OUT=1;
  i8253[Counter].NextTickLoad=1;
  i8253[Counter].StartFlag=0;
};
void mode3_WriteNewValue(int Counter,int RWMode) {
  if (RWMode == 3) { // 2 byte load
  } else {
    i8253[Counter].NextTickLoad=1;
    i8253[Counter].StartFlag=1;
  }
}
void mode3_do(int Counter,int Takt) {

   if (i8253[Counter].NextTickLoad) {
      i8253[Counter].CountingElement=i8253[Counter].CountRegister&0xfffe;
      i8253[Counter].NextTickLoad=0;
   }

   while (Takt--) {
     if (i8253[Counter].StartFlag) {
       if (i8253[Counter].CountRegister & 1) { // ODD count


         i8253[Counter].CountingElement-=2;

         if (1 == i8253[Counter].CountingElement) {
           i8253[Counter].CountingElement = i8253[Counter].CountRegister&0xfffe;
           i8253[Counter].OUT = 0;
         } 

         if (!i8253[Counter].CountingElement) {
           if (i8253[Counter].OUT == 1) i8253[Counter].CountingElement=3;
           else {
              i8253[Counter].CountingElement = i8253[Counter].CountRegister&0xfffe;
              i8253[Counter].OUT = 1;
           }
         } 
       } else {                                // EVEN count  
         i8253[Counter].CountingElement-=2;
         if (!i8253[Counter].CountingElement) {
           i8253[Counter].CountingElement = i8253[Counter].CountRegister;
           i8253[Counter].OUT ^= 1;
         }
       }
       ADD_OUT(Counter,i8253[Counter].OUT);
     } else {
       ADD_OUT(Counter,1);
     }
   } 
}

// MODE 4: SOFTWARE TRIGGERED STROBE ------------------------------------------
void mode4_setmode(int Counter) {
  i8253[Counter].OUT=1;
  i8253[Counter].NextTickLoad=1;
  i8253[Counter].StartFlag=0;
};
void mode4_WriteNewValue(int Counter,int RWMode) {
  if (RWMode == 3) { // 2 byte load
  } else {
    i8253[Counter].NextTickLoad=1;
    i8253[Counter].StartFlag=1;
  }
}
void mode4_do(int Counter,int Takt) {

   if (i8253[Counter].NextTickLoad) {
      i8253[Counter].CountingElement=i8253[Counter].CountRegister;
      i8253[Counter].NextTickLoad=0;
   }

   while (Takt--) {
     if (i8253[Counter].StartFlag) {
       if (0 == i8253[Counter].CountingElement-- ) {
         ADD_OUT(Counter,0);
       }
     } else {
       ADD_OUT(Counter,1);
     }
   } 
}

// MODE 5: HARDWARE TRIGGERED STROBE (RETRIGGERABLE) --------------------------
void mode5_setmode(int Counter) {
  i8253[Counter].OUT=1;
};
void mode5_WriteNewValue(int Counter,int RWMode) {}
void mode5_do(int Counter,int Takt) {
   while (Takt--) {
     ADD_OUT(Counter,i8253[Counter].OUT); //???? may be 1 ???
   } 
}

void WriteControlWord(int Value) {

  int Counter=(Value&0xc0)>>6;
  int RWMode =(Value&0x30)>>4;
  int Mode   =(Value&0x0e)>>1;
  int BCDFlag=(Value&0x01);
 
  if (0 == RWMode) { // Counter Latch command
    // ???? counter = 3 ??? 5253?, 8254 - Read Back CMD ....
    if (0 == i8253[Counter].RWMode) {// latch command ignored if already latched.
      i8253[Counter].OutputLatch  = i8253[Counter].CountingElement;
      i8253[Counter].LatchCounter = i8253[Counter].RWMode;
    }; 
  } else { // Set Chanel MODE

    if (Mode & 0x02) Mode &= 0x03; // mode 6,7 -> mode 2,3

    i8253[Counter].Mode           = Mode;
    i8253[Counter].RWMode         = RWMode;
    i8253[Counter].BCDFlag        = BCDFlag;
//    i8253[Counter].CountRegister  = 0;        // 
    i8253[Counter].WriteCounter   = RWMode;   // Prepare for Write;

    switch(i8253[Counter].Mode) {
      case 0: mode0_setmode(Counter);break;
      case 1: mode1_setmode(Counter);break;
      case 2: mode2_setmode(Counter);break;
      case 3: mode3_setmode(Counter);break;
      case 4: mode4_setmode(Counter);break;
      case 5: mode5_setmode(Counter);break;
    }

  }
}

void WriteCounter(int Counter,int Value) {

   switch(i8253[Counter].Mode) {
     case 0: mode0_WriteNewValue(Counter,i8253[Counter].WriteCounter);break;
     case 1: mode1_WriteNewValue(Counter,i8253[Counter].WriteCounter);break;
     case 2: mode2_WriteNewValue(Counter,i8253[Counter].WriteCounter);break;
     case 3: mode3_WriteNewValue(Counter,i8253[Counter].WriteCounter);break;
     case 4: mode4_WriteNewValue(Counter,i8253[Counter].WriteCounter);break;
     case 5: mode5_WriteNewValue(Counter,i8253[Counter].WriteCounter);break;
   }
   // 0 1 - least significat byte
   // 1 0 - most  significat byte
   // 1 1 - least significat byte, most significat byte

   if (i8253[Counter].WriteCounter & 0x01) { // 01, 11 - Least significat
     i8253[Counter].CountRegister  = (i8253[Counter].CountRegister & 0xff00) | Value;
     i8253[Counter].WriteCounter  ^= 0x01;
   } else if (i8253[Counter].WriteCounter & 0x02) { // 10, 11 - Most significat
     i8253[Counter].CountRegister  = (i8253[Counter].CountRegister & 0x00ff) | (Value<<8);
     i8253[Counter].WriteCounter  ^= 0x02;
   } 

   if (0 == i8253[Counter].WriteCounter) { 
     i8253[Counter].WriteCounter=i8253[Counter].RWMode;
   }

}

// ---------------------------------------------------------
void DoTMR  (int Counter,int N)         // Выполнить N тактов таймера
{
   switch (i8253[Counter].Mode) {
    case 0:mode0_do(Counter,N);break;
    case 1:mode1_do(Counter,N);break;
    case 2:mode2_do(Counter,N);break;
    case 3:mode3_do(Counter,N);break;
    case 4:mode4_do(Counter,N);break;
    case 5:mode5_do(Counter,N);break;
  }
}


void Init_TimerCntr(int Counter) {
  i8253[Counter].CountingElement=0;
  i8253[Counter].CountRegister=0;
  i8253[Counter].OutputLatch=0;
  i8253[Counter].Mode=0;
  i8253[Counter].BCDFlag=0;
  i8253[Counter].RWMode=0;
  i8253[Counter].LatchCounter=0;
  i8253[Counter].WriteCounter=0;
  i8253[Counter].ReadCounter=0;
  i8253[Counter].OUT=0;
  i8253[Counter].StartFlag=0;
  i8253[Counter].NextTickLoad=0;
}


void InitTMR(void)                      // Инициализация при старте
{
  Init_TimerCntr(0);
  Init_TimerCntr(1);
  Init_TimerCntr(2);
  BytePtr=0;
  PrevTakt=0;
}

inline int DoTimer(void){
 DoTMR(0,(Takt-PrevTakt)*20/25);

// printf("DoTimer: %06d - %06d\n",Takt,PrevTakt);
 PrevTakt=Takt;
 return 0;
}

byte Timer_Read(int Addr)
{
 static unsigned short ret=0;
#ifdef TRACETIMER
 fprintf(F_TIMER,"R: %08d %04x\n",Takt,Addr);
#endif
 //DoTimer();
 return ret++;
}


void Timer_Write(int Addr, byte Value)
{
#ifdef TRACETIMER
 fprintf(F_TIMER,"W: %08d %04x=%02x\n",Takt,Addr,Value);
#endif
//  if (Addr&3 == 0) DoTimer();
//  if (Addr&3 == 3) DoTimer();
  DoTimer();
  switch (Addr & 3) {
    case 0: 
    case 1:
    case 2: WriteCounter(Addr&3,Value);break;
    case 3: WriteControlWord(Value);break;
  }
}


void MakeSound(void) {

  int  TempValue=0;
  int  i,j;
  int  ByteInByte=40000/AUDIO_BUFFER_SIZE;
  int  outptr=0;
  int  d7=0;
  DoTimer();
  PrevTakt-=ALL_TAKT;

  if (MuteFlag) {
    for (i=0;i<AUDIO_BUFFER_SIZE;i++) SOUNDBUF[i]=0;
  } else {
    for (i=0;i<AUDIO_BUFFER_SIZE;i++) {
      for (j=0;j<ByteInByte;j++) if (TIMERBUF[outptr++]) TempValue++;

      d7+=7028;if (d7 >=10000) {if (TIMERBUF[outptr++]) TempValue++;d7-=10000;}

      SOUNDBUF[i]=(TempValue>ByteInByte/2)?255:0;
      TempValue=0;
    }
  }
/*
  for (i=0;i<AUDIO_BUFFER_SIZE;i++) {
    TempValue=0;
    for (j=0;j<ByteInByte;j++) if (TIMERBUF[outptr++]) TempValue++;
    SOUNDBUF[i]=(TempValue>ByteInByte/2)?255:0;
  }
*/
  BytePtr=0;
}

// ---------------------------------------------------------

