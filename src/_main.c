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

#ifdef DBG
#include "dbg/dbg.h"
#endif

const char AboutMSG[]="Korvet Emulator by Sergey Erokhin & Korvet Team|pk8020@narod.ru|2012-05-30|V1.?.1";

// GetOpt external variables
extern char *optarg;
extern int optind, opterr, optopt;
extern byte SOUNDBUF[];


// for DUMP

#define PLANESIZE 	16384

extern byte ACZU[1024*2];          // 1К памяти АЦЗУ
//extern byte ACZU_INV[1024];      // 1К флагов атрибута АЦЗУ
extern byte GZU[4][PLANESIZE*3]; // 3 слоя ГЗУ (4 страницы)
extern byte LUT[16];
extern PALLETE pallete;
extern int scr_GZU_Size_Mask;    // маска размера ГЗУ, =0x0f - 4*48, =0 - 1x 48k
// end ForDUMP

extern byte RAM[65535];
extern int SYSREG;
extern int AllScreenUpdateFlag;
extern int LutUpdateFlag;
extern int scr_Wide_Mode;
extern int scr_Second_Font;
extern int scr_Page_Show;
extern byte LUT[];

extern int SCREEN_OFFX;
extern int SCREEN_OFFY;
extern int SCREEN_XMAX;
extern int SCREEN_YMAX;
extern int SCREEN_OSDY;

extern int FlagScreenScale;
extern int Current_Scr_Mode;

extern byte OUTBUF[];
extern int  BytePTR;


extern char Disks[4][1024];
extern int  InUseFDD[4];

extern char FontFileName[1024];
extern char RomFileName[1024];
extern char MapperFileName[1024];

extern int OSD_LUT_Flag;
extern int OSD_FPS_Flag;
extern int OSD_FDD_Flag;

int VBLANK=0;
int SoundEnable=0;
int Takt;
extern int IntREQ;
extern int IntREQ;
extern int IntDelay;
unsigned char op;

extern int MuteFlag;

extern int JoystickFlag;
extern int JoystickUseFlag;

int BW_Flag=0;

int main_loop_run_flag=1;
int FPS=0;
int FPS_Scr=0;
int FPS_LED=0;
int Counter50hz=0;		// 50 hz synchronization counter
int KBD_LED=2;                  // RusEngFlag
extern int KeyboadUpdateFlag;   // =1 if need rebuld keyboard layout
extern int KeyboardLayout;
PALLETE pallete;

int BMP_NUM=0;
AUDIOSTREAM *stream;

#ifdef TRACETIMER
FILE *F_TIMER;
#endif


#ifdef DBG
extern word dbg_HERE;  // Адрес останова при пошаговом выполнении
extern int  dbg_TRACE;  // Флаг необходимости вызова отладчика
#endif

//
int Treasure=0;

int CPU_DBG[256];
int TotalCPU;

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
    GZU_Init();
    ACZU_Init();
    Memory_Init();
    PIC_init();
    CPU_Init();
    KBD_Init();
// FDC_Init();
    FDC_Reset();
    Serial_Init();
    InitTMR();
}

void Debug_LUT(int Debug_Key) {
    byte SaveLut[16];
    byte i;
    for (i=0; i<16; i++) {
        SaveLut[i]=LUT[i];
        LUT[i]=i;
    }
    LUT_Update(BW_Flag);
    while (key[Debug_Key]);
    for (i=0; i<16; i++) {
        LUT[i]=SaveLut[i];
    }
    LUT_Update(BW_Flag);
}

void Write_BMP(char * FileName,int page) {
    int kx=1;
    int saved_page=scr_Page_Show;
    if (FlagScreenScale) kx=2;
    BITMAP *bmp=create_bitmap(512*kx,256*kx);
    clear_bitmap(bmp);

    scr_Page_Show=page;
    AllScreenUpdateFlag=1;
    SCREEN_ShowScreen();

    blit(screen,bmp,SCREEN_OFFX,SCREEN_OFFY,0,0,512*kx,256*kx);
    save_bmp(FileName,bmp,pallete);

    scr_Page_Show=saved_page;
    AllScreenUpdateFlag=1;
    SCREEN_ShowScreen();
}


void dump_gzu(int page) {
    char fname[512];
    int i;
    byte B1[3][PLANESIZE];
    FILE *F_DMP;

    sprintf(fname,"DMP.LGZU%d",page);

    F_DMP=fopen(fname,"wb");
    for (i=0; i<PLANESIZE; i++) {
        B1[0][i]=GZU[page][i*4+0];
        B1[1][i]=GZU[page][i*4+1];
        B1[2][i]=GZU[page][i*4+2];
        //+3 - aczu
    }
    fwrite(B1,PLANESIZE*3,1,F_DMP);
    fclose(F_DMP);
}

void Write_Dump(void)
{

    FILE *F_DMP;

    int i,j;
    word reg;
    char BUF[1024];

    F_DMP=fopen("DMP.RAM","wb");
    fwrite(RAM,0x10000,1,F_DMP);
    fclose(F_DMP);

    F_DMP=fopen("DMP.ACZU","wb");
    fwrite(ACZU,1024*2,1,F_DMP);
    fclose(F_DMP);

    dump_gzu(0);
    dump_gzu(1);
    dump_gzu(2);
    dump_gzu(3);

    F_DMP=fopen("DMP.LUT","wb");
    fwrite(LUT,sizeof(LUT),1,F_DMP);
    fclose(F_DMP);

    F_DMP=fopen("DMP.CPU","wb");
    reg=CPU_GetPC();
    fwrite(&reg,sizeof(reg),1,F_DMP);
    reg=CPU_GetSP();
    fwrite(&reg,sizeof(reg),1,F_DMP);
    reg=CPU_GetAF();
    fwrite(&reg,sizeof(reg),1,F_DMP);
    reg=CPU_GetHL();
    fwrite(&reg,sizeof(reg),1,F_DMP);
    reg=CPU_GetDE();
    fwrite(&reg,sizeof(reg),1,F_DMP);
    reg=CPU_GetBC();
    fwrite(&reg,sizeof(reg),1,F_DMP);
    fclose(F_DMP);

    sprintf(BUF,"DMPSC%03d_0.bmp",BMP_NUM);
    Write_BMP(BUF,0);
    BMP_NUM++;

}

void ReadConfig(void) {
    char section[]="korvet";
    set_config_file("./korvet.cfg");
    strcpy(Disks[0]      ,get_config_string(section,"DriveA","disk/disk.kdi"));
    strcpy(Disks[1]      ,get_config_string(section,"DriveB","disk/disk1.kdi"));
    strcpy(Disks[2]      ,get_config_string(section,"DriveC","disk/disk2.kdi"));
    strcpy(Disks[3]      ,get_config_string(section,"DriveD","disk/disk3.kdi"));

    strcpy(FontFileName  ,get_config_string(section,"FONT","data/korvet2.fnt"));
    strcpy(RomFileName   ,get_config_string(section,"ROM","data/rom.rom"));
    strcpy(MapperFileName,get_config_string(section,"MAPPER","data/mapper.mem"));

    scr_GZU_Size_Mask    =get_config_hex(section,"GZU_Pages",4);
    scr_GZU_Size_Mask    =(scr_GZU_Size_Mask == 1) ? 0:0x0f;

    OSD_LUT_Flag         =get_config_hex(section,"OSD_LUT",0);
    OSD_FPS_Flag         =get_config_hex(section,"OSD_FPS",0);
    OSD_FDD_Flag         =get_config_hex(section,"OSD_FDD",0);

    FlagScreenScale      =get_config_hex(section,"SCALE_WINDOW",0);

    KeyboardLayout       =get_config_hex(section,"KEYBOARD_MODE",KBD_AUTO);
}

void PrintDecor() {

    rect(screen,SCREEN_OFFX-2,SCREEN_OFFY-2,SCREEN_OFFX+SCREEN_XMAX+1,SCREEN_OFFY+SCREEN_YMAX+1,255);

    //init InUseFlag for screen update
    InUseFDD[0]=InUseFDD[1]=InUseFDD[2]=InUseFDD[3]=1;

    if (Current_Scr_Mode != SCR_DBG) {
        textprintf_ex(screen,font,15,SCREEN_H-33,255,0,"Alt-F9-MENU, F11-Reset, F12-Quit, F8-Zoom, F6-Turbo, F10-Mono, F9-dbg");
        rect(screen,0,SCREEN_H-40,SCREEN_W,SCREEN_H-40,255);
        textprintf_ex(screen,font,0,SCREEN_H-16,255,0,AboutMSG);
    }
}

void MUTE_BUF(void) {
    int i;
    unsigned char *p;

    for(i=0; i<AUDIO_BUFFER_SIZE; i++) {
        SOUNDBUF[i]=0;
    }

    while (!(p = get_audio_stream_buffer(stream))) rest(0);
    memcpy(p,SOUNDBUF,AUDIO_BUFFER_SIZE);
    free_audio_stream_buffer(stream);
}


int main(int argc,char **argv) {

    int i,j;

    FILE *FTMP;

    int skip;

    unsigned char *p;
    int TickCntr;
    int outptr;
    int TempValue;
    int SndCnt=1;
    int SndAvg=1;

    char BUFFER[2048];

    //LUT_BASE_COLOR = 0x80
    assert(LUT_BASE_COLOR == 0x80); //very important for SCREEN_ShowScreen

    allegro_init();

    ReadConfig(); // Read KORVET.CFG file and set default values ...

    InitOSD();
    InitPrinter();
    Init_Joystick();

    // parse command line option -A filename -B filename
    while ((i=getopt(argc, argv, "a:A:b:B:c:C:d:D:")) != -1) {
        switch (tolower(i)) {
        case 'a':
            strcpy(Disks[0],optarg);
            break;
        case 'b':
            strcpy(Disks[1],optarg);
            break;
        case 'c':
            strcpy(Disks[2],optarg);
            break;
        case 'd':
            strcpy(Disks[3],optarg);
            break;
        }
    }

    j=0;
    for (i=0; i<4; i++) {
        TempValue=open(Disks[i],O_RDONLY);
        if (TempValue<0) {
            printf("Warning: Can't open drive '%c' file: %s\n",'A'+i,Disks[i]);
            j++;
        }
        close(TempValue);
    }

    if (j) {
        printf("Press Enter to continue\n");
        getchar();
    }

#ifdef TRACETIMER
    F_TIMER=fopen("_timer.log","wb");
#endif

#ifdef WAV
    OpenWAV("korvet.wav");
#endif


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


#ifdef SOUND
//    install a digital sound driver
    if (install_sound(DIGI_AUTODETECT, MIDI_NONE, NULL) != 0) {
        allegro_message("Error initialising sound system\n%s\n", allegro_error);
        return 1;
    }
    printf("Audio driver: %s\n", digi_driver->name);

//    create an audio stream
    stream = play_audio_stream(AUDIO_BUFFER_SIZE, 8, FALSE, SOUNDFREQ, 255, 128);
    if (!stream) {
        set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
        allegro_message("Error creating audio stream!\n");
        return 1;
    }
#endif
    fflush(stdout);
    SCREEN_SetGraphics(SCR_EMULATOR);

    LOCK_FUNCTION(Timer_50hz);
    LOCK_VARIABLE(Counter50hz);

    LOCK_FUNCTION(Timer_1S);
    LOCK_VARIABLE(FPS);
    LOCK_VARIABLE(FPS_Scr);

    LOCK_VARIABLE(ShowedLines_Scr);
    LOCK_VARIABLE(ShowedLines);
    LOCK_VARIABLE(ShowedLinesTotal);
    LOCK_VARIABLE(ShowedLinesCnt);

    install_int_ex(Timer_1S, SECS_TO_TIMER(1));
    install_int_ex(Timer_50hz,BPS_TO_TIMER(50));

// Screen texts
//                                   ________________________________________________________________________________
    clear_to_color(screen, 254);
    PrintDecor();
// end screen texts

//
    Reset();
    LUT_Init();
    LUT_Update(BW_Flag);

    SCREEN_ShowScreen();

    Takt=0;
    while (!key[KEY_F12]) {

        if (key[KEY_F11]) {
            Takt=0;
            Reset();
            while(key[KEY_F11]);
        }

        if (key[KEY_F10]) {
            BW_Flag^=1;
            LutUpdateFlag=1;
            while (key[KEY_F10]);
        }
        if (key[KEY_F8]) {
            if ((key_shifts & KB_ALT_FLAG)) {
                Write_Dump();
                while ((key_shifts & KB_ALT_FLAG));
            } else {
                FlagScreenScale^=1;
                SCREEN_SetGraphics(SCR_EMULATOR);
            }
            while (key[KEY_F8]);
        }
        if (key[KEY_F7]) {
            Debug_LUT(KEY_F7);
        }

#ifdef DBG
        word  __PC=CPU_GetPC();

        if (__PC == 0x0000) CheckROM();
        else if (__PC == 0xc75c) CheckCCP();
        else if (__PC == 0xcade) CheckComEXEC();

        if (__PC == dbg_HERE) dbg_TRACE=1;

        if (RD_BreakPoint(__PC)&bpCPU) {
            dbg_TRACE=1;/*TraceCause=4;CauseAddr=CPU_GetPC()*/;
        }
        if (dbg_TRACE) doDBG();

        AddPC(__PC);

#endif

        Takt+=CPU_Exec1step();

        if (Takt>=ALL_TAKT) {
#ifdef SOUND
//       MakeSound(); // timer
            if (!key[KEY_F6]) {
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
//         free_audio_stream_buffer(stream);
            }
#endif
            Counter50hz=0;

            PIC_IntRequest(4);
//       DoCH(2);

            ChkMouse();
//        ChkMouse_MouseSystem();

//       IntREQ=CheckPIC();



#ifndef SOUND
//       else if (!key[KEY_F6]) vsync();
//       if (!key[KEY_F6]) {
//            while (!Counter50hz) yield_timeslice();
//            Counter50hz=0;
//       };
#endif


#ifdef TRACETIMER
            fprintf(F_TIMER,"V: %08d\n",Takt);
#endif
            if (LutUpdateFlag) LUT_Update(BW_Flag);
            SCREEN_ShowScreen();


            FPS++;

//       textprintf(screen,font,0,60,255,"fps: %d SL:%d slavg:%d cnt50:%d %%%3d            ",FPS_Scr,ShowedLines_Scr/((FPS_Scr)?FPS_Scr:1),ShowedLinesTotal/((FPS_Scr)?FPS_Scr:1)/ShowedLinesCnt,Counter50hz,FPS_Scr*100/50);
//       textprintf(screen,font,0,0,255,"fps: %d SL:%d slavg:%d cnt50:%d  ",FPS_Scr,ShowedLines_Scr/((FPS_Scr)?FPS_Scr:1),ShowedLinesTotal/((FPS_Scr)?FPS_Scr:1)/ShowedLinesCnt,Counter50hz);
            Takt-=ALL_TAKT;

            if (getpixel(screen,SCREEN_OFFX-2,SCREEN_OFFY-2) != 255) {
                PrintDecor();
                AllScreenUpdateFlag=1;
            }
            // выводим OnScreen LED
            // ТОЛЬКО если есть необходимость обновить индикаторы,
            // иначе будут мигать, да и FPS падает ;-)
            // FPS
            if (OSD_FPS_Flag && (FPS_Scr != FPS_LED)) {
                PutLED_FPS(SCREEN_OFFX,SCREEN_OSDY  ,FPS_Scr);
                FPS_LED=FPS_Scr;
            };
            // Floppy Disk TRACK
            if (OSD_FDD_Flag && InUseFDD[0]) {
                InUseFDD[0]--;
                PutLED_FDD(SCREEN_OFFX+512-80,SCREEN_OSDY,VG.TrackReal[0],InUseFDD[0]);
            }
            if (OSD_FDD_Flag && InUseFDD[1]) {
                InUseFDD[1]--;
                PutLED_FDD(SCREEN_OFFX+512-60,SCREEN_OSDY,VG.TrackReal[1],InUseFDD[1]);
            }
            if (OSD_FDD_Flag && InUseFDD[2]) {
                InUseFDD[2]--;
                PutLED_FDD(SCREEN_OFFX+512-40,SCREEN_OSDY,VG.TrackReal[2],InUseFDD[2]);
            }
            if (OSD_FDD_Flag && InUseFDD[3]) {
                InUseFDD[3]--;
                PutLED_FDD(SCREEN_OFFX+512-20,SCREEN_OSDY,VG.TrackReal[3],InUseFDD[3]);
            }

            // if (JoystickUseFlag) {JoystickUseFlag--;textprintf(screen,font,SCREEN_OFFX+512,SCREEN_OSDY,255,"%s",(JoystickUseFlag==0)?"      ":"JOY:3B");}


            // if LAT<->RUS rebuild KeboardLayout table (auto qwerty<->jcuken)
            if ((RAM[0xf72e] ^ (KEYBOARD_Read(0x80)&2)) != KBD_LED) {
                KBD_LED=(RAM[0xf72e] ^ (KEYBOARD_Read(0x80)&2));
                KeyboadUpdateFlag=1;
            }

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
    }

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

    return 0;
}
END_OF_MAIN();

