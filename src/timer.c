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
#include "host.h"
#include "korvet.h"
#include "darray.h"
#include <stdarg.h>
#include <stdio.h>

//extern screen,font;
#ifdef TRACETIMER
extern int Takt;
static FILE *F_TIMER;
#endif

// -------------------------------------------------------------------------
// Переменная из модуля Звука (Флаг разрешения звука)
extern int SoundEnable;
extern int Takt;
int PrevTakt;
int MuteFlag=0;

// -------------------------------------------------------------------------
// buffer for КР580ВИ53 (i8253) cnannel 0 (sound) samples per Takt
// consider it private, use _OUT functions for access
static struct darray *tout; /* timer output */

// buffer for Allegro play_audio_stream
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

/*
 * Subroutines to access timer output buffer.  ADD_OUT in theory
 * can be used with any channel, it is used by the timer chip
 * emulator, so it takes the channel as its argument.
 * Others are used in sound related code only, so no point to
 * carry the channel there.
 */

static void ADD_OUT(int CH, int Value)
{
    uint8_t v;
    void *rc;

    /* Only channel 0 supported, used for sound generation */
    if (CH != 0)
        return;

    if (MuteFlag)
        return;

    v = (uint8_t)(Value & SoundEnable);
    rc = darray_push(tout, &v);
    if (rc == NULL) {
        pr_error("Timer buffer is full!\n");
        abort();
    }
}

static bool GET_OUT(int idx, int *Value)
{
    uint8_t v;
    void *rc;

    rc = darray_read(tout, idx, &v);
    if (rc == NULL) {
        pr_debug("Run out of timer buffer");
        return false;
    }
    *Value = v;
    return true;
}

static inline void DRAIN_OUT(void)
{
    darray_reset(tout);
}

static unsigned LENGTH_OUT(void)
{
    return darray_count(tout);
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

    // trace timer init, planed to use for autograb mouse when pors are settings for use
    // if ( (Counter == 1) && (i8253[Counter].CountRegister == 13)) {
    //   printf("Default port init\n");
    // } else if ( (Counter == 1) && (i8253[Counter].CountRegister == 0x68)) {
    //   printf("Mouse init on port\n");
    // } else {
    //   printf("WriteCntr: %d : %d\n",Counter, i8253[Counter].CountRegister);
    // }
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
    PrevTakt=0;

    darray_reset(tout);
    memset(SOUNDBUF, 0, sizeof(SOUNDBUF));
}

int DoTimer(void){
    DoTMR(0,(Takt-PrevTakt)*20/25);

// printf("DoTimer: %06d - %06d\n",Takt,PrevTakt);
    PrevTakt=Takt;
    return 0;
}

byte Timer_Read(int Addr)
{
    static unsigned short ret;

    TimerTrace("R: %08d %04x\n",Takt,Addr);
    return ret++;
}


void Timer_Write(int Addr, byte Value)
{
    TimerTrace("W: %08d %04x=%02x\n",Takt,Addr,Value);
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


#define MAX_ERROR_SAMPLES   8
#define DAMPING_LEVEL       3

void MakeSound()
{
    int nticks, tickval, sample, tick, error;
    int SampleIntegralAmplitude, TicksPerSample, SampleMaxAmplitude;
    int TickWindowBegin, TickWindowEnd;

    nticks = LENGTH_OUT();
    if (MuteFlag || nticks == 0) {
        memset(SOUNDBUF, 0, sizeof(SOUNDBUF));
        return;
    }
    TicksPerSample = nticks / AUDIO_BUFFER_SIZE;
    SampleMaxAmplitude = TicksPerSample;
    error = (40000 - nticks) / (40000 / AUDIO_BUFFER_SIZE);
    if (error > MAX_ERROR_SAMPLES)
        pr_error("%d audio samples over/underrun, distortion possible. Please report.\n",
                 error);
    for (sample = 0; sample < AUDIO_BUFFER_SIZE; sample++) {
        SampleIntegralAmplitude = 0;
        TickWindowBegin = sample * nticks / AUDIO_BUFFER_SIZE;
        TickWindowEnd = (sample+1) * nticks / AUDIO_BUFFER_SIZE;
        for (tick = TickWindowBegin; tick < TickWindowEnd; tick++) {
            if (GET_OUT(tick, &tickval) && tickval)
                SampleIntegralAmplitude++;
        }
        /* try to make smooth wave instead of original 0/1 trigger implementation */
        SOUNDBUF[sample] = SampleIntegralAmplitude * 128 / SampleMaxAmplitude;
        /* avoid uneven sampling artifacts */
        if (SOUNDBUF[sample] >= (128 - DAMPING_LEVEL))
            SOUNDBUF[sample] = 128;
        else if (SOUNDBUF[sample] <= DAMPING_LEVEL)
            SOUNDBUF[sample] = 0;
    }
    DRAIN_OUT();
}

void Timer50HzTick(void)
{
    DoTimer();
    PrevTakt -= ALL_TAKT;
}

void InitTimer(void)
{
#ifdef TRACETIMER
    F_TIMER=fopen("_timer.log","wb");
    setlinebuf(F_TIMER);
#endif
    tout = darray_new(MAXBUF, sizeof(uint8_t));
    if (tout == NULL)
        abort();
}

void DestroyTimer(void)
{
    darray_destroy(tout);
#ifdef TRACETIMER
    fclose(F_TIMER);
#endif
}

#ifdef TRACETIMER
int TimerTrace(const char *fmt, ...)
{
    va_list ap;
    int ret;

    va_start(ap, fmt);
    ret = vfprintf(F_TIMER, fmt, ap);
    va_end(ap);

    return ret;
}
#endif
// ---------------------------------------------------------
