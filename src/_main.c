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
#include "vg.h"
#include <assert.h>
#include "lan.h"

#ifdef DBG
#include "dbg/dbg.h"
#endif

int VBLANK=0;
int SoundEnable=0;
int Takt;

int BW_Flag=0;

int main_loop_run_flag=1;
int FPS=0;
int FPS_Scr=0;
int FPS_LED=0;
int Counter50hz=0;		// 50 hz synchronization counter
extern int KBD_LED;                  // RusEngFlag

extern byte SOUNDBUF[];
extern byte RAM[65535];
extern int KeyboadUpdateFlag;   // =1 if need rebuld keyboard layout
extern int KeyboardLayout;
extern int LutUpdateFlag;
extern int FlagScreenScale;
extern int MuteFlag;
extern int AllScreenUpdateFlag;  // Флаг необходимости обновть весь экран

AUDIOSTREAM *stream;

#ifdef TRACETIMER
FILE *F_TIMER;
#endif

#ifdef DBG
extern word dbg_HERE;  // Адрес останова при пошаговом выполнении
extern int  dbg_TRACE;  // Флаг необходимости вызова отладчика
#endif

extern const char AboutMSG[];

// timer routine for measuring
void Timer_1S()
{
    FPS_Scr=FPS;
    FPS=0;
}
END_OF_FUNCTION(Timer_1S);

// timer routine for measuring
void Timer_50hz()
{
    Counter50hz++;
}
END_OF_FUNCTION(Timer_1S);

void Reset(void) {
    printf("Reset\n");
    GZU_Init();
    ACZU_Init();
    Memory_Init();
    FDC_Init();
    FDC_Reset();
    PIC_init();
    CPU_Init();
    KBD_Init();
    FDC_Reset();
    PPI_Init();
    Serial_Init();
    InitTMR();
    ResetOSD();
}

void trace_bios(word pc) {
    static unsigned int dma;
    word bc=CPU_GetBC();
    word HL;
    int i,j;
    switch (pc) {
        //DEFAULT cpm addrs
        case 0xda18: { printf("TRACE_BIOS: HOME\n"); break; }
        case 0xda1B: { printf("TRACE_BIOS: sel_DSK dsk=%d\n",bc & 0xff); break; }
        case 0xda1E: { printf("TRACE_BIOS: set_TRK trk=%d\n",bc & 0xff); break; }
        case 0xda21: { printf("TRACE_BIOS: set_SEC sec=%d\n",bc & 0xff); break; }
        case 0xda24: { printf("TRACE_BIOS: set_DMA dma=0x%04x\n",bc); break; }
        case 0xda27: { printf("TRACE_BIOS: READ\n"); break; }
        case 0xda2A: { printf("TRACE_BIOS: WRITE type=%d\n",bc & 0xff); break; }

        // microdos, DISKIO point
        case 0xE46E:  // IO 870430
        case 0xEAA3:  // IO 880630
        case 0xEAA5:  // IO 900105
        {
            HL=CPU_GetHL();
            dma=RAM[HL+6] | (RAM[HL+7] << 8);
            printf("DSK_IO: drv=%02x chword=%02x func=%02x numb=%02x trk=%02x sec=%02x abuf=%04x htrk=%02x hsec=%02x\n",
                RAM[HL+0],
                RAM[HL+1],
                RAM[HL+2],
                RAM[HL+3],
                RAM[HL+4],
                RAM[HL+5],
                dma,
                RAM[HL+8],
                RAM[HL+9]
            );
            break;
        }

        case 0xeabe:  //dma buf after IO
        {
            printf("DMA BUF:\n");
            for (i=0;i<8;i++) {
                printf("%04X: ",dma+i*16);
                for (j=0;j<16;j++) {
                    byte v=RAM[dma+i*16+j];
                    printf("%02X ",v);
                }
                printf("\n");
            }
        }
    }
}


int main_loop(void) {
    unsigned char *p;

    Takt=0;
    while (!key[KEY_F12]) {

        if (key[KEY_F7]) {
            Debug_LUT(KEY_F7);
        }

        if (key[KEY_F8]) {
            if ((key_shifts & KB_ALT_FLAG)) {
                Write_Dump();
                while ((key_shifts & KB_ALT_FLAG));
            } else {
                FlagScreenScale^=1;
                SCREEN_SetGraphics(SCR_EMULATOR);
                update_osd();
            }
            while (key[KEY_F8]);
        }

        if (key[KEY_F9]) {

            if (!(key_shifts & KB_ALT_FLAG)) {
    #ifdef DBG
                while (key[KEY_F9]);
                dbg_TRACE=1;
    #endif
            } else {
                while(key[KEY_ALT]);
                while(key[KEY_F9]);
                GUI();
                while(key[KEY_ESC]);
            }
        }

        if (key[KEY_F10]) {
            BW_Flag^=1;
            LutUpdateFlag=1;
            while (key[KEY_F10]);
        }

        if (key[KEY_F11]) {
            Takt=0;
            Reset();
            while(key[KEY_F11]);
        }

    #ifdef DBG
        word  __PC=CPU_GetPC();

        if (__PC == 0x0000) CheckROM();
        else if (__PC == 0xc75c) CheckCCP();
        else if (__PC == 0xcade) CheckComEXEC();

        // show special bios events like disk access
        // trace_bios(__PC);

        if (__PC == dbg_HERE) dbg_TRACE=1;

        if (RD_BreakPoint(__PC)&bpCPU) {
            dbg_TRACE=1;/*TraceCause=4;CauseAddr=CPU_GetPC()*/;
        }
        if (dbg_TRACE) doDBG();

        AddPC(__PC);

    #endif

        Takt+=CPU_Exec1step();

    #ifdef LAN_SUPPORT
        LAN_poll();  
    #endif

        if (Takt>=ALL_TAKT) {
    #ifdef SOUND
    //       MakeSound(); // timer
            if (!key[KEY_F6]) {
                // были в MUTE перед этим
                if (MuteFlag==1) {
                    AllScreenUpdateFlag=1;
                }

                MuteFlag=0;
                MakeSound(); // timer

                while (!(p = get_audio_stream_buffer(stream)))  rest(0);
                memcpy(p,SOUNDBUF,AUDIO_BUFFER_SIZE);
    #ifdef WAV
                AddWAV(p,AUDIO_BUFFER_SIZE);
    #endif
                free_audio_stream_buffer(stream);
                while (!Counter50hz) rest(0);
            } else {
                if (MuteFlag == 0) {
                    // mute sund
                    MUTE_BUF();
                    MUTE_BUF();
                    MUTE_BUF();
                    MUTE_BUF();
                    MUTE_BUF();
                    MUTE_BUF();
                }
                MuteFlag=1;
                MakeSound(); // timer
            }
    #endif
            Counter50hz=0;

            PIC_IntRequest(4);

            ChkMouse();

    #ifdef TRACETIMER
            fprintf(F_TIMER,"V: %08d\n",Takt);
    #endif
            if (LutUpdateFlag) LUT_Update(BW_Flag);
            SCREEN_ShowScreen();

            FPS++;

            Takt-=ALL_TAKT;

            update_osd();

            // update_rus_lat();
        }
    }
}

int do_hw_inits(void) {
    #ifdef DBG
    dbg_INIT();
    #endif

    SCREEN_Init();

    set_uformat(U_ASCII);

    install_keyboard();
    install_timer();
    install_mouse();
    enable_hardware_cursor();
    show_os_cursor(MOUSE_CURSOR_QUESTION);


    LOCK_FUNCTION(Timer_50hz);
    LOCK_VARIABLE(Counter50hz);

    LOCK_FUNCTION(Timer_1S);
    LOCK_VARIABLE(FPS);
    LOCK_VARIABLE(FPS_Scr);

    LOCK_VARIABLE(ShowedLines_Scr);
    LOCK_VARIABLE(ShowedLines);
    LOCK_VARIABLE(ShowedLinesTotal);
    LOCK_VARIABLE(ShowedLinesCnt);


    #ifdef SOUND
    // install a digital sound driver
    if (install_sound(DIGI_AUTODETECT, MIDI_NONE, NULL) != 0) {
        allegro_message("Error initialising sound system\n%s\n", allegro_error);
        return 1;
    }
    printf("Audio driver: %s\n", digi_driver->name);

    // create an audio stream
    stream = play_audio_stream(AUDIO_BUFFER_SIZE, 8, FALSE, SOUNDFREQ, 255, 128);
    if (!stream) {
        set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
        allegro_message("Error creating audio stream!\n");
        return 1;
    }
    #endif

    install_int_ex(Timer_1S, SECS_TO_TIMER(1));
    install_int_ex(Timer_50hz,BPS_TO_TIMER(50));
}

int main(int argc,char **argv) {

    int i,j;

    //LUT_BASE_COLOR = 0x80
    assert(LUT_BASE_COLOR == 0x80); //very important for SCREEN_ShowScreen

    printf("%s\n",AboutMSG);

    allegro_init();

    ReadConfig(); // Read KORVET.CFG file and set default values ...

    // parse command line option -A filename -B filename
    parse_command_line(argc,argv);

    check_missing_images();


    InitOSD();
    InitPrinter();
    Init_Joystick();

    #ifdef LAN_SUPPORT  
    LAN_Init();
    #endif

    #ifdef TRACETIMER
    F_TIMER=fopen("_timer.log","wb");
    #endif

    #ifdef WAV
    OpenWAV("korvet.wav");
    #endif

    i=do_hw_inits();
    if (i != 0) return i;
    
    fflush(stdout);
    SCREEN_SetGraphics(SCR_EMULATOR);
    clear_to_color(screen, 254);
    PrintDecor();

    LUT_Init();
    LUT_Update(BW_Flag);

    Reset();

    SCREEN_ShowScreen();

    main_loop();

    #ifdef WAV
    CloseWAV();
    #endif

    DestroyOSD();
    DestroyPrinter();
    SCREEN_SetText();

    printf("\n\n\n%s\n",AboutMSG);

    #ifdef TRACETIMER
    fclose(F_TIMER);
    #endif

    #ifdef LAN_SUPPORT
    if (LAN_Addr == 0x0f) { // remove ptx file only when RMU
        unlink(LAN_PTX_FILE);
    }
    #endif


    return 0;
}
END_OF_MAIN();

