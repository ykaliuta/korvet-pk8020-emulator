#include "korvet.h"
#include "vg.h"
#include <assert.h>
#include <getopt.h>

#include "ext_rom.h"
#include "lan.h"

#ifdef DBG
#include "dbg/dbg.h"
#endif

extern byte LUT[16];

extern int SCREEN_OFFX;
extern int SCREEN_OFFY;
extern int SCREEN_XMAX;
extern int SCREEN_YMAX;
extern int SCREEN_OSDY;

extern int OSD_LUT_Flag;
extern int OSD_FPS_Flag;
extern int OSD_FDD_Flag;

extern int FPS_Scr;
extern int FPS_LED;

extern char FontFileName[1024];
extern char RomFileName[1024];
extern char MapperFileName[1024];

extern int BW_Flag;

extern int scr_Wide_Mode;
extern int scr_Second_Font;
extern int scr_Page_Show;

extern int FlagScreenScale;
extern int Current_Scr_Mode;

extern byte RAM[65535];
extern byte ACZU[1024*2];          // 1К памяти АЦЗУ
extern byte GZU[4][PLANESIZE*3]; // 3 слоя ГЗУ (4 страницы)
extern PALLETE pallete;

extern int AllScreenUpdateFlag;
extern int LutUpdateFlag;

extern int scr_GZU_Size_Mask;    // маска размера ГЗУ, =0x0f - 4*48, =0 - 1x 48k

extern int KeyboadUpdateFlag;   // =1 if need rebuld keyboard layout
extern int KeyboardLayout;

extern int InUseFDD[4];
extern int floppy_disabled;

extern byte SOUNDBUF[];

const char AboutMSG[]="Korvet Emulator by Sergey Erokhin & Korvet Team|pk8020@narod.ru|2012-05-30|V1.?.1";

extern AUDIOSTREAM *stream;

int BMP_NUM=0;

extern int JoystickEnabled;
extern int JoystickNumber;

extern int MouseType; //1 - MSMouse, 2-MouseSystem


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

    #ifdef LAN_SUPPORT
    // Секция [lan]    
    LAN_Addr              =get_config_hex("lan","ADDR",0);  
    LAN_Addr=(~LAN_Addr)&0xf;
    strcpy(LAN_ttdev     ,get_config_string("lan","DEVICE",""));
    //strcpy(LAN_logfile   ,get_config_string("lan","LOG",""));
    #endif
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

void help(void) {
    printf("\nAvailable keys\n\n");

    printf("Disk Images\n");
    printf("\t-a <KDI.FILE> KDI disk image mounted in drive A\n");
    printf("\t-b <KDI.FILE> KDI disk image mounted in drive B\n");
    printf("\t-c <KDI.FILE> KDI disk image mounted in drive C\n");
    printf("\t-d <KDI.FILE> KDI disk image mounted in drive D\n");
    printf("\t-z disable floppy disk controller emulation\n");

    printf("\nROM\n");
    printf("\t-r <ROM.FILE> Path to MAIN ROM file\n");
    printf("\t-f <FONTROM.FILE> Path to FONT ROM file\n");
    printf("\t-x <ROM.FILE> ROM attached to EXT connector (and turn on ext rom support)\n");
    printf("\t\t\tdisable joystick support\n");
    printf("\t-E <foldername> - turn on ExtROMExtender, point to folder that emulate SD card\n");


    printf("\nMouse and Joystick\n");
    printf("\t-m <mouse type> select attached mouse type\n");
    printf("\t\t0 - turn support off\n");
    printf("\t\t1 - emualte Microsoft mouse (default) \n");
    printf("\t\t2 - emualte MouseSystem mouse\n");

    printf("\t-j <joystick num> emulate physical joystick <joystick num> to korvet joystick (attached to EXT port)\n");
    printf("\t\t\ttry -j 9 to show all available joysticks in system\n");

    #ifdef LAN_SUPPORT
    printf("\tLAN support\n");
    printf("\t-l - emulate corver network\n");
    printf("\t\t-l <path to port> /dev/ttyS0 - use real ttyS0 (aka COM1: ) for connect to real PK8010\n");
    printf("\t\t-l RMP - run as RMP\n");
    printf("\t\t-l RMU - run as RMU (and attach to the emulator started with -l RMP)\n");
    // printf("\t\tor long way ...\n");
    // printf("\t\t-l /dev/ptmx  - attach to virtual pesudoterminal (emulate network), see pseudeterminal id in output\n");
    // printf("\t\t-l /dev/pts/2 - on rmu (see /dev/pttmx output)\n");
    printf("\t\tsample\n");
    // printf("\t\t\tRMP ./kdbg -n0 -s/dev/ptmx\n");
    // printf("\t\t\tRMU ./kdbg -n2 -s/dev/pts/2\n");
    printf("\t\t-l /dev/ttyS0 - use real ttyS0 (aka COM1: ) for connect to real PK8010\n");
    printf("\n");
    printf("\t-n - (0..15) RMU network address\n");
    printf("\t-q - file name for logging network traffic\n");
    #endif    
}

void parse_command_line(int argc,char **argv) {
    int i;
    // parse command line option -A filename -B filename
    while ((i=getopt(argc, argv, 
        "hHa:A:b:B:c:C:d:D:x:X:r:R:j:J:m:M:f:F:e:E:zZ"
    #ifdef LAN_SUPPORT
        "n:N:l:L:q:Q:"
    #endif
        )) != -1) {
        switch (tolower(i)) {
        case 'h':
            help();
            exit(-1);
            break;
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
        case 'z':
            floppy_disabled=1;
            break;
        case 'r':
            strcpy(RomFileName,optarg);
            break;
        case 'f':
            strcpy(FontFileName,optarg);
            break;
        case 'e':
            strcpy(ext_rom_emu_folder,optarg);
            printf("%c\n",ext_rom_emu_folder[strlen(ext_rom_emu_folder)-1]);
            if (ext_rom_emu_folder[strlen(ext_rom_emu_folder)-1] != '/') {
                strcat(ext_rom_emu_folder,"/");
            }
            strcpy(ext_rom_file_name,"stage1.rom");
            ext_rom_mode=1;
            break;
        case 'x': 
            strcpy(ext_rom_file_name,optarg);
            ext_rom_mode=1;
            break;
        case 'j':
            if (optarg[0]>='0' && optarg[0]<='9') {
                JoystickNumber=optarg[0]-'0';
                JoystickEnabled=1;
            } else {
                printf(" Invalid joystick number '%s'\n",optarg );
            }
            break;
        case 'm':
            i=optarg[0]-'0';
            if (i==0 || i==1 || i==2) {
                MouseType=i;
            } else {
                printf(" Invalid mouse type (0 - disable, 1 - MS , 2 - MouseSystem) '%s'\n",optarg );
            }
            break;
        #ifdef LAN_SUPPORT
        case 'l': 
            strcpy(LAN_ttdev,optarg);
            break;
        case 'n': 
            sscanf(optarg,"%i",&LAN_Addr);
            LAN_Addr=(~LAN_Addr)&0xf;
            break;
        case 'q': 
            strcpy(LAN_logfile,optarg); 
            // if (strcmp(LAN_logfile,"-") == 0) {
            //     LAN_logfile[0]=0;   // Если указан ключ "-l-" то отказываемся от лог-файла
            // }
            break;
        #endif
        default:
            printf("aborted, use -h for help\n"); 
            exit(-1);   
       }         
    }
}

void check_missing_images(void) {
    int     i;
    int     errors_count=0;
    int     file;

    for (i=0; i<4; i++) {
        file=open(Disks[i],O_RDONLY);
        if (file<0) {
            printf("Warning: Can't open drive '%c' file: %s\n",'A'+i,Disks[i]);
            errors_count++;
        }
        close(file);
    }

    if (errors_count) {
        printf("Press Enter to continue\n");
        getchar();
    }
}

void update_osd(void) {
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

    if (getpixel(screen,SCREEN_OFFX-2,SCREEN_OFFY-2) != 255) {
        PrintDecor();
        AllScreenUpdateFlag=1;
    }

}