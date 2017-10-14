#include "korvet.h"
#include "host.h"
#include "vg.h"
#include <assert.h>
#include <getopt.h>

#include "ext_rom.h"
#include "lan.h"

#ifdef DBG
#include "dbg/dbg.h"
#endif

extern byte LUT[16];

extern int turboBOOT;        // boost turbo in frames, (50*10) - 10 virtual second

extern int OSD_LUT_Flag;
extern int OSD_FPS_Flag;
extern int OSD_FDD_Flag;
extern int OSD_KBD_Flag;

extern int InUseKBD;                 // kbd osd flag
extern int InUseKBD_rows[16];
extern int KBD_LED;                  // RusEngFlag

extern int SCREEN_OFFX;
extern int SCREEN_OFFY;
extern int SCREEN_XMAX;
extern int SCREEN_YMAX;
extern int SCREEN_OSDY;

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
extern byte GZU[4][PLANESIZE*(3+1)]; // 3 слоя ГЗУ (4 страницы) + слой АЦЗУ
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

void Write_Dump(void) {

    FILE *F_DMP;
    word reg;
    char BUF[1024];

    F_DMP=fopen("DMP.RAM","wb");
    fwrite(RAM,0x10000-1,1,F_DMP);
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
    struct config_entry arr[] = {
	{
            .type = CONFIG_TYPE_STRING,
            .name = "DriveA",
            .section = section,
            .storage = Disks[0],
            .max_size = sizeof(Disks[0]),
            .p_default = "disk/disk.kdi",
	},
	{
            .type = CONFIG_TYPE_STRING,
            .name = "DriveB",
            .section = section,
            .storage = Disks[1],
            .max_size = sizeof(Disks[1]),
            .p_default = "disk/disk1.kdi",
	},
	{
            .type = CONFIG_TYPE_STRING,
            .name = "DriveC",
            .section = section,
            .storage = Disks[2],
            .max_size = sizeof(Disks[2]),
            .p_default = "disk/disk2.kdi",
	},
	{
            .type = CONFIG_TYPE_STRING,
            .name = "DriveD",
            .section = section,
            .storage = Disks[3],
            .max_size = sizeof(Disks[3]),
            .p_default = "disk/disk3.kdi",
	},
        {
            .type = CONFIG_TYPE_STRING,
            .name = "FONT",
            .section = section,
            .storage = FontFileName,
            .max_size = sizeof(FontFileName),
            .p_default = "data/korvet2.fnt",
	},
        {
            .type = CONFIG_TYPE_STRING,
            .name = "ROM",
            .section = section,
            .storage = RomFileName,
            .max_size = sizeof(RomFileName),
            .p_default = "data/rom.rom",
	},
        {
            .type = CONFIG_TYPE_STRING,
            .name = "MAPPER",
            .section = section,
            .storage = MapperFileName,
            .max_size = sizeof(MapperFileName),
            .p_default = "data/mapper.mem",
	},
	{
            .type = CONFIG_TYPE_INT,
            .name = "GZU_Pages",
            .section = section,
            .storage = &scr_GZU_Size_Mask,
            .max_size = sizeof(int),
            .i_default = 4,
	},
	{
            .type = CONFIG_TYPE_INT,
            .name = "OSD_LUT",
            .section = section,
            .storage = &OSD_LUT_Flag,
            .max_size = sizeof(int),
            .i_default = 0,
	},
        {
            .type = CONFIG_TYPE_INT,
            .name = "OSD_FPS",
            .section = section,
            .storage = &OSD_FPS_Flag,
            .max_size = sizeof(int),
            .i_default = 0,
	},
        {
            .type = CONFIG_TYPE_INT,
            .name = "OSD_FDD",
            .section = section,
            .storage = &OSD_FDD_Flag,
            .max_size = sizeof(int),
            .i_default = 0,
	},
        {
            .type = CONFIG_TYPE_INT,
            .name = "OSD_KBD",
            .section = section,
            .storage = &OSD_KBD_Flag,
            .max_size = sizeof(int),
            .i_default = 0,
	},
        {
            .type = CONFIG_TYPE_INT,
            .name = "SCALE_WINDOW",
            .section = section,
            .storage = &FlagScreenScale,
            .max_size = sizeof(int),
            .i_default = 0,
	},
        {
            .type = CONFIG_TYPE_INT,
            .name = "KEYBOARD_MODE",
            .section = section,
            .storage = &KeyboardLayout,
            .max_size = sizeof(int),
            .i_default = KBD_AUTO,
	},
#ifdef LAN_SUPPORT
        {
            .type = CONFIG_TYPE_INT,
            .name = "ADDR",
            .section = "lan",
            .storage = &LAN_Addr,
            .max_size = sizeof(int),
            .i_default = 0,
	},
        {
            .type = CONFIG_TYPE_STRING,
            .name = "DEVICE",
            .section = "lan",
            .storage = LAN_ttdev,
            .max_size = sizeof(LAN_ttdev),
            .p_default = "",
	},
#endif
    };

    host_config_parse("./korvet.cfg", arr, ARRAY_SIZE(arr));

    scr_GZU_Size_Mask = (scr_GZU_Size_Mask == 1) ? 0 : 0x0f;

    #ifdef LAN_SUPPORT
    LAN_Addr = (~LAN_Addr) & 0xf;
    #endif
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

    printf("\nGeneral\n");
    printf("\t-T TurboBOOT, execute first 10 emulator seconds on maximum speed\n");
    printf("\t-t <secons>, custom turbo time\n");

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
        "hHa:A:b:B:c:C:d:D:x:X:r:R:j:J:m:M:f:F:e:E:zZTt:"
    #ifdef LAN_SUPPORT
        "n:N:l:L:q:Q:"
    #endif
        )) != -1) {
        switch (i) {
        case 'h':
        case 'H':
            help();
            exit(-1);
            break;
        case 'a':
        case 'A':
            strcpy(Disks[0],optarg);
            break;
        case 'b':
        case 'B':
            strcpy(Disks[1],optarg);
            break;
        case 'c':
        case 'C':
            strcpy(Disks[2],optarg);
            break;
        case 'd':
        case 'D':
            strcpy(Disks[3],optarg);
            break;
        case 'z':
        case 'Z':
            floppy_disabled=1;
            break;
        case 'r':
        case 'R':
            strcpy(RomFileName,optarg);
            break;
        case 'f':
        case 'F':
            strcpy(FontFileName,optarg);
            break;
        case 'e':
        case 'E':
            strcpy(ext_rom_emu_folder,optarg);
            printf("%c\n",ext_rom_emu_folder[strlen(ext_rom_emu_folder)-1]);
            if (ext_rom_emu_folder[strlen(ext_rom_emu_folder)-1] != '/') {
                strcat(ext_rom_emu_folder,"/");
            }
            strcpy(ext_rom_file_name,"stage1.rom");
            ext_rom_mode=1;
            break;
        case 'x':
        case 'X':
            strcpy(ext_rom_file_name,optarg);
            ext_rom_mode=1;
            break;
        case 'j':
        case 'J':
            if (optarg[0]>='0' && optarg[0]<='9') {
                JoystickNumber=optarg[0]-'0';
                JoystickEnabled=1;
            } else {
                printf(" Invalid joystick number '%s'\n",optarg );
            }
            break;
        case 'm':
        case 'M':
            i=optarg[0]-'0';
            if (i==0 || i==1 || i==2) {
                MouseType=i;
            } else {
                printf(" Invalid mouse type (0 - disable, 1 - MS , 2 - MouseSystem) '%s'\n",optarg );
            }
            break;
        #ifdef LAN_SUPPORT
            case 'l':
            case 'L':
                strcpy(LAN_ttdev,optarg);
                break;
            case 'n':
            case 'N':
                sscanf(optarg,"%i",&LAN_Addr);
                LAN_Addr=(~LAN_Addr)&0xf;
                break;
            case 'q':
            case 'Q':
                strcpy(LAN_logfile,optarg);
                // if (strcmp(LAN_logfile,"-") == 0) {
                //     LAN_logfile[0]=0;   // Если указан ключ "-l-" то отказываемся от лог-файла
                // }
                break;
        #endif
        case 'T':
            turboBOOT=50*10; // force turbo for first 10 seconds (50 frame in seconds)
            break;
        case 't':
            sscanf(optarg,"%i",&turboBOOT);
            turboBOOT=50*turboBOOT; // force turbo for first <ARG> seconds (50 frame in seconds)
            break;
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


void update_rus_lat(void) {
  // if LAT<->RUS rebuild KeboardLayout table (auto qwerty<->jcuken)
  if ((RAM[0xf72e] ^ (KEYBOARD_Read(0x80,1)&2)) != KBD_LED) {
      KBD_LED=(RAM[0xf72e] ^ (KEYBOARD_Read(0x80,1)&2));
      KeyboadUpdateFlag=1;
  }
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
