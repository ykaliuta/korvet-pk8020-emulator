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
#include "ext_rom.h"
#include <dirent.h>

// Поддержка загрузки с внешнего ROM

int   ext_rom_mode=0;
FILE* extrom_file;
char  ext_rom_file_name[1024]="not set";    // имя файла с образом ROM
int   ext_rom_addr_changed=0;               // =1 while EXT ROM BOOT (rom loader only write in PPI3B,PPI3C)
char  ext_rom_emu_folder[1024]="";          // папка которая прикидывается SDCARD эмулятора
char  drive_filename[5][14];                // имена файлов для монтирования образа
char  drive_foldername[5][14];              // имена каталогов, хранящих файлы
char  drive_status[5];                      // состояние образа - 0 - не смонтирован, 1 - смонтирован
char  drive_roflag[5];                      // разрешение записи: 0-чтение/запись   1-только чтение
char  diskfolder[14];                       // имя каталога по умолчанию, из которго берутся образы
FILE* mount_cfg;                            // файл списка монтирования
char  control_flag=1;                       // Флаг анализа сигнала Control: 0-игнорируется, 1-учитвается при файловых операциях
int  extrom_debug=1;

unsigned char e_cmd=0;
unsigned char e_trk=0;
unsigned char e_sec=0;
unsigned char e_drv=0;
unsigned char e_crc=0;

int   e_substitute_system_track=1;        //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

// Имена файлов, подставляемых для загрузки системных дорожек
char fname_subst[64]="SYSTEMn.BIN";
char* sysfiles[3]= {"SYSTEM.BIN","MICRODOS.BIN",fname_subst};
char substitute_number=0;

#define EXT_BUF_SIZE (64*1024)
byte in_buffer[EXT_BUF_SIZE];
int in_buffer_size=0;

byte out_buffer[EXT_BUF_SIZE];
int out_buffer_size=0;

int emu_stage=EMU_STAGE1;

FILE *f_emu=NULL;

char tmp_path[1024];

byte static_buf128[128];

void ext_rom_api_write(byte Value);
void add_to_out_buf(byte value);
void add_block_to_out_buf(int size,byte *block);
byte ext_rom_api_read(void);

void init_extrom(void) {
    if (ext_rom_mode) {
        if (strlen(ext_rom_emu_folder)>0) {
            printf("ExtROM_EMU_SD folder   : %s\n",ext_rom_emu_folder);
        }
        printf("ExtROM                 : %s\n",ext_rom_file_name);

        sprintf(tmp_path,"%s%s",ext_rom_emu_folder,ext_rom_file_name);
        extrom_file=fopen(tmp_path,"rb");
        if (extrom_file == 0) {
            printf("WARNING: Ошибка открытия файла образа ROM - %s\n",tmp_path);
            printf("ExtRom Boot - режим отключен\n");

            ext_rom_mode=0;
        }
        emu_stage=EMU_STAGE1;
    }

    // Читаем и разбираем файл конфигурации
    strcpy(tmp_path,ext_rom_emu_folder);
    strcat(tmp_path,"MOUNT.CFG");
    mount_cfg=fopen(tmp_path,"r");

    if (mount_cfg == 0) {   // конфиг отсутствует на диске
        mount_cfg=fopen(tmp_path,"w");
        strcpy(drive_filename[0],"DISKA.KDI");
        strcpy(drive_filename[1],"DISKB.KDI");
        strcpy(drive_filename[2],"DISKC.KDI");
        strcpy(drive_filename[3],"DISKD.KDI");

        strcpy(drive_foldername[0],"DISK");
        strcpy(drive_foldername[1],"DISK");
        strcpy(drive_foldername[2],"DISK");
        strcpy(drive_foldername[3],"DISK");

        strcpy(diskfolder,"DISK");

        fwrite(drive_foldername[0],1,14,mount_cfg);
        fwrite(drive_filename[0],1,14,mount_cfg);

        fwrite(drive_foldername[1],1,14,mount_cfg);
        fwrite(drive_filename[1],1,14,mount_cfg);

        fwrite(drive_foldername[2],1,14,mount_cfg);
        fwrite(drive_filename[2],1,14,mount_cfg);

        fwrite(drive_foldername[3],1,14,mount_cfg);
        fwrite(drive_filename[3],1,14,mount_cfg);

        fwrite(diskfolder,1,14,mount_cfg);

        fclose (mount_cfg);
    } else {    // конфиг есть - читаем и разбираем
        fread(drive_foldername[0],1,14,mount_cfg);
        fread(drive_filename[0],1,14,mount_cfg);

        fread(drive_foldername[1],1,14,mount_cfg);
        fread(drive_filename[1],1,14,mount_cfg);

        fread(drive_foldername[2],1,14,mount_cfg);
        fread(drive_filename[2],1,14,mount_cfg);

        fread(drive_foldername[3],1,14,mount_cfg);
        fread(drive_filename[3],1,14,mount_cfg);

        fread(diskfolder,1,14,mount_cfg);
    }

// Диск 4 (E) - Mount-диск
    strcpy(drive_filename[4],"EXRTOOLS.KDI");
    strcpy(drive_foldername[4],"");

    drive_status[0]=1;
    drive_status[1]=1;  // образ смонтирован - поднимаем флаги тодбко для A и B
    drive_status[2]=1;
    drive_status[3]=1;
    drive_status[4]=1;
    drive_roflag[0]=0;
    drive_roflag[1]=0;  // снимаем защиту от записи
    drive_roflag[2]=0;
    drive_roflag[3]=0;
    drive_roflag[4]=1;  // диск Е по умолчанию защищен!
}

byte ext_rom_read(unsigned char PPI3_B, unsigned char PPI3_C) {
    unsigned char value=0;
    unsigned int ext_rom_addr=(PPI3_C<<8)+PPI3_B;
    if (ext_rom_mode) {
        if (ext_rom_addr_changed) {
            fseek(extrom_file,ext_rom_addr,SEEK_SET);
            value=fgetc(extrom_file);
            // printf("EXTROM: %04x=%02x\n",ext_rom_addr,value);
        }
    }
    return value;
}
//************************
//*  Чтение образа диска
//************************

void emu_read_image_128(void) {
    //CMD,DRV,TRK,SEC
    int offset;
    unsigned short lspt;
    // substitute 2 track for cpm & co, and 3 for microdos
    int track_to_substitute= (substitute_number == 1) ? 3 : 2 ;

    // printf("READ CMD: %02x, DRV:%02x, TRK: %03d, SEC: %02d OFFSET:%08x\n",e_cmd,e_drv,e_trk,e_sec,offset);
    strcpy(tmp_path,ext_rom_emu_folder);  // имя каталога с образами
    if ((e_substitute_system_track == 1) && (e_trk<track_to_substitute) && (e_drv == 0)) strcat(tmp_path,sysfiles[substitute_number]); // для режима подстановки образа системы
    else  {
        strcat(tmp_path,drive_foldername[e_drv]);
        strcat(tmp_path,"/");                                     // каталог на карте с образами
        strcat(tmp_path,drive_filename[e_drv]);                       // для доступа к самому диску A
    }
//  sprintf(tmp_path,"%sdisk_%c.kdi",ext_rom_emu_folder,'a'+e_drv);
    f_emu=fopen(tmp_path,"rb");
    if (drive_status[e_drv]==0) {   // диск не смонтирован
        add_to_out_buf(EMU_API_FAIL);   // даем отлуп в интерфейс
        printf(" - диск не смонтирован\n");
        return;                         // обрываем операцию
    }
    if (f_emu == 0) {
        add_to_out_buf(EMU_API_FAIL);
        printf("ERROR: can't open %s\n",tmp_path);
        drive_status[e_drv]=0;   // снимаем флаг смонтированного диска
    } else {
        add_to_out_buf(EMU_API_OK);
        fseek(f_emu,16,SEEK_SET);  // поле LSPT инфосектора
        fread(&lspt,1,2,f_emu);    // читаем SPT

        offset=(e_trk*lspt+e_sec)*128; // полное смещение до сектора

        fseek(f_emu,offset,SEEK_SET);
        fread(static_buf128,1,128,f_emu);
        fclose(f_emu);
    }
}

//************************
//*  Запись образа диска
//************************

void emu_write128(void) {
    //CMD,DRV,TRK,SEC
    int offset;
    unsigned short lspt;

    // fseek(f_emu,(trk*40+sec)*128,SEEK_SET);
    // printf("WRITE CMD: %02x, DRV:%02x, TRK: %03d, SEC: %02d OFFSET:%08x\n",e_cmd,e_drv,e_trk,e_sec,offset);

    strcpy(tmp_path,ext_rom_emu_folder);      // имя каталога с образами
    strcat(tmp_path,drive_foldername[e_drv]);
    strcat(tmp_path,"/");                     // каталог на карте с образами
    strcat(tmp_path,drive_filename[e_drv]);
    f_emu=fopen(tmp_path,"r+b");
    if (f_emu == 0) {
        printf("ERROR: can't open for write %s\n",tmp_path);
        drive_status[e_drv]=0;         // снимаем флаг смонтированного диска
    }
    else {
        fseek(f_emu,16,SEEK_SET);  // поле LSPT инфосектора
        fread(&lspt,1,2,f_emu);    // читаем SPT
        offset=(e_trk*lspt+e_sec)*128; // полное смещение до сектора
        fseek(f_emu,offset,SEEK_SET);
        fwrite(in_buffer,1,128,f_emu);
        fclose(f_emu);
    }
}


//****************************************************
//*  Получение имени файла KDI из интерфейса
//*  и монтирование его к диcку 0-4
//****************************************************
void emu_getfilename() {

    strncpy(drive_filename[e_drv],in_buffer,14);       // имя файла
    strncpy(drive_foldername[e_drv],diskfolder,14);    // имя файла
    drive_filename[e_drv][13]=0;
    drive_status[e_drv]=1;		                       // образ смонтирован - поднимаем флаг
    printf(" + mount %c: %s\n",e_drv+'A',drive_filename[e_drv]);
    if (e_trk != 0) drive_roflag[e_drv]=1;             // защита от записи
    if (e_sec != 0) {
        // Сохраняем в конфиг
        strcpy(tmp_path,ext_rom_emu_folder);
        strcat(tmp_path,"MOUNT.CFG");
        mount_cfg=fopen(tmp_path,"r+");
        fseek(mount_cfg,28*e_drv,SEEK_SET);
        fwrite(diskfolder,1,14,mount_cfg);             // каталог берем по умолчанию
        fwrite(drive_filename[e_drv],1,14,mount_cfg);  // имя файла - из параметров команды
        fclose(mount_cfg);
    }
    // проверяем наличие файла
    strcpy(tmp_path,ext_rom_emu_folder);
    strcat(tmp_path,diskfolder);
    strcat(tmp_path,"/");                              // каталог на карте с образами
    strcat(tmp_path,drive_filename[e_drv]);
    f_emu=fopen(tmp_path,"r");
    if (f_emu == 0) {
        printf(" - файл не найден\n");
        drive_status[e_drv]=0;	                       // опускае флаг готовности диска
    }
    else fclose(f_emu);
}


//****************************************
//*  Установка каталога с образами KDI   *
//****************************************
void emu_setfolder() {

    DIR* nfolder;

    strcpy(tmp_path,ext_rom_emu_folder);
    strcat(tmp_path,"/");
    strncat(tmp_path,in_buffer,14); // имя файла
    nfolder=opendir(tmp_path);
    if (nfolder == 0) {
        printf("Нет каталога %s\n",tmp_path);
        return;
    }
    closedir(nfolder);
    strncpy(diskfolder,in_buffer,14); // имя файла
    printf("Новый каталог: %s\n",diskfolder);
    if (e_sec != 0) {
        // сохраняем в конфиг
        strcpy(tmp_path,ext_rom_emu_folder);
        strcat(tmp_path,"MOUNT.CFG");
        mount_cfg=fopen(tmp_path,"r+");
        fseek(mount_cfg,112,SEEK_SET);
        fwrite(diskfolder,1,14,mount_cfg);
        fclose(mount_cfg);
    }
}

//****************************************************
//*  Создание пустого образа KDI
//*  и монтирование его к диcку A или B.
//****************************************************
void emu_createkdi() {

    int i;
    unsigned char fb[256];
    const char infosector[]= {
        0x80, 0xc3, 0x00, 0xda, 0x0a, 0x00, 0x00, 0x01,
        0x01, 0x01, 0x03, 0x01, 0x05, 0x00, 0x50, 0x00,
        0x28, 0x00, 0x04, 0x0f, 0x00, 0x8a, 0x01, 0x7f,
        0x00, 0xc0, 0x00, 0x20, 0x00, 0x02, 0x00, 0x10
    };

    strncpy(drive_filename[e_drv],in_buffer,14); // имя файла
    drive_filename[e_drv][13]=0;
    printf(" + create %c: %s\n",e_drv+'A',drive_filename[e_drv]);
    strcpy(tmp_path,ext_rom_emu_folder);  // имя каталога с образами
    strcat(tmp_path,diskfolder);
    strcat(tmp_path,"/");                                     // каталог на карте с образами
    strcat(tmp_path,drive_filename[e_drv]);
    f_emu=fopen(tmp_path,"w");
    // Начинаем создание буфера 0 - с ифосктором
    memcpy(fb,infosector,32);
    memset(fb+32,0xe5,256-32);
    // записываем буфер 0
    fwrite(fb,1,256,f_emu);
    // создаем буфер-заполнитель
    // Формируем буфер-заполнитель
    memset(fb,0xe5,32);
    // Записываем заполнитель во все секторы KDI
    for(i=0; i<0xc7f; i++) fwrite(fb,1,256,f_emu);

    fclose(f_emu);
    drive_status[e_drv]=1;	// образ смонтирован - поднимаем флаг
}

//****************************************************
//*  Передача списка файлов
//****************************************************
void emu_send_dir() {

    DIR* ddisk;
    struct dirent* d;

    strcpy(tmp_path,ext_rom_emu_folder);    // имя каталога с образами
    strcat(tmp_path,diskfolder);
    strcat(tmp_path,"/");                   // каталог на карте с образами
    // открываем каталог для чтения
    ddisk=opendir(tmp_path);
    printf("Список файлов каталога %s:\n",diskfolder);
    while ((d=readdir(ddisk)) != 0) {
        // Читаем и передаем элементы каталога
        if (d->d_name[0] != '.')  {         // скрытые файлы пропускаем
            add_block_to_out_buf(14,d->d_name);
            printf("* %s",d->d_name);
        }
    }
    add_to_out_buf(0);
    closedir(ddisk);
}

//****************************************************
//*  Передача списка каталогов
//****************************************************
void emu_send_listdir() {

    DIR* ddisk;
    struct dirent* d;


    strcpy(tmp_path,ext_rom_emu_folder);  // имя каталога с образами
    strcat(tmp_path,"/");                 // каталог на карте с образами
    // открываем каталог для чтения
    ddisk=opendir(tmp_path);
    printf("Список каталогов карты:\n");
    while ((d=readdir(ddisk)) != 0) {
        // Читаем и передаем элементы каталога
        if ((d->d_name[0] != '.') && (d->d_type == DT_DIR))  {   // скрытые файлы пропускаем, выводим только каталоги
            add_block_to_out_buf(14,d->d_name);
            printf("* %s",d->d_name);
        }
    }
    add_to_out_buf(0);
    closedir(ddisk);
}

//=========================================================================================
void emu_stage1(void) {

    sprintf(tmp_path,"%s%s",ext_rom_emu_folder,"stage2.rom");
    if ( (in_buffer[0]>=1) && (in_buffer[0]<=7) ) {
        sprintf(tmp_path,"%srom%c.rom",ext_rom_emu_folder,'0'+in_buffer[0]);
    }

    printf("requested file: %s\n",tmp_path);

    f_emu=fopen(tmp_path,"rb");
    if (f_emu == 0) {
        printf("ERROR: Stage1 loader request '%d' file, but file '%s' can't be opened\n",in_buffer[0],tmp_path);
    } else {
        in_buffer_size=0;
        out_buffer_size=fread(out_buffer+2,1,EXT_BUF_SIZE,f_emu);
        fclose(f_emu);

        out_buffer[0]=out_buffer[2+6];    // отсылаем загрузчику адрес размещения файла в памяти
        out_buffer[1]=out_buffer_size>>8; // отсылаем загрузчику адрес размещения файла в памяти
        out_buffer_size+=2;               // два первых байта
        emu_stage=EMU_STAGE2_WAITCMD;
    }
}

void emu_api_cmd(void) {
    int i;
    switch (in_buffer[0]) {

    case 0: { // ping
        if (extrom_debug) {printf("CMD: PING\n");}

        add_to_out_buf(EMU_API_OK);
        in_buffer_size=0;
        break;
    }

    case 1: { // read sector
        if (extrom_debug) {printf("CMD: READ sector\n");}
        emu_read_image_128();
        if (drive_status[e_drv] == 1) add_block_to_out_buf(128,static_buf128);
        in_buffer_size=0;
        break;
    }

    case 2: { // write sector
        if (extrom_debug) {printf("CMD: WRITE sector\n");}
        if ((drive_status[e_drv]==0) || (drive_roflag[e_drv] == 1)) {   // диск не смонтирован или запрещена запись
            add_to_out_buf(EMU_API_FAIL);                               // даем отлуп в интерфейс
            printf(" - запись недоступна или диск не смонтирова\n");
        }
        else   {
            add_to_out_buf(EMU_API_OK);
            emu_stage=EMU_STAGE2_WRITE128;
        }
        in_buffer_size=0;
        break;
    }

    case 0x80: { // получить имя файла образа
        if (extrom_debug) {printf("CMD: GET image file name\n");}
        add_to_out_buf(EMU_API_OK);
        add_to_out_buf(drive_roflag[e_drv]);
        add_block_to_out_buf(14,drive_foldername[e_drv]);
        add_block_to_out_buf(14,drive_filename[e_drv]);
        in_buffer_size=0;
        break;
    }

    case 0x81: { // Установить имя файла образа
        if (extrom_debug) {printf("CMD: SET image file name\n");}
        add_to_out_buf(EMU_API_OK);
        emu_stage=EMU_STAGE2_GETFILENAME;
        in_buffer_size=0;
        break;
    }

    case 0x82: { // получить состояние смонтированного образа
        if (extrom_debug) {printf("CMD: GET mount state\n");}
        add_to_out_buf(drive_status[e_drv]);
        in_buffer_size=0;
        break;
    }

    case 0x83: { // Создание образа KDI
        if (extrom_debug) {printf("CMD: CREATE KDI\n");}
        add_to_out_buf(EMU_API_OK);
        emu_stage=EMU_STAGE2_CREATE_KDI;
        in_buffer_size=0;
        break;
    }

    case 0x84: { // Получение списка файлов
        if (extrom_debug) {printf("CMD: GET FLIST\n");}
        add_to_out_buf(EMU_API_OK);
        emu_send_dir();
        in_buffer_size=0;
        break;
    }

    case 0x85: { // Получение имени каталога с образами
        if (extrom_debug) {printf("CMD: GET FOLDER NAME\n");}
        add_to_out_buf(EMU_API_OK);
        add_block_to_out_buf(14,diskfolder);
        in_buffer_size=0;
        break;
    }

    case 0x86: { // Установка имени каталога с образами sec=0 верменно,  1 - постоянно
        if (extrom_debug) {printf("CMD: SET FOLDER NAME\n");}
        add_to_out_buf(EMU_API_OK);
        emu_stage=EMU_STAGE2_GETFOLDER;
        in_buffer_size=0;
        break;
    }

    case 0x87: { // Вывод списка каталогов, имеющихся на карте
        if (extrom_debug) {printf("CMD: GET FOLDER LIST\n");}
        add_to_out_buf(EMU_API_OK);
        emu_send_listdir();
        in_buffer_size=0;
        break;
    }

    case 0x88: { // Снятие защиты записи с диска Е
        if (extrom_debug) {printf("CMD: UNLOCK DRIVE E \n");}
        add_to_out_buf(EMU_API_OK);
        drive_roflag[4]=0;
        in_buffer_size=0;
        break;
    }

    case 0xA0: {  // --------- включение-отключение подстановки системных дорожек
        if (extrom_debug) {printf("CMD: substitution \n");}
        if (e_drv == 0) {
            add_to_out_buf(EMU_API_OK);
            if (e_trk == 0) e_substitute_system_track=0;
            else {
                e_substitute_system_track=1;
                sysfiles[2][6]='0'+e_trk;    // впиывваем # в SYSTEMn.BIN
                substitute_number=e_trk-1; // индекс в массиве подставляемых имен файлов
                if (substitute_number>2) substitute_number=2;  // для параметров 3 и более подставляем SYSTEMn.BIN
            }
        }
        else  add_to_out_buf(EMU_API_FAIL);   // иначе ответ Error
        in_buffer_size=0;
        break;
    }

    case 0xA1: { // установка реакции на Control
        if (extrom_debug) {printf("CMD: Control sense\n");}
        add_to_out_buf(EMU_API_OK);
        control_flag=e_trk;
        in_buffer_size=0;
        break;
    }

    case 0xf0: { // SPEED TEST - out 0x8000 bytes
        if (extrom_debug) {printf("CMD: SpeedTest OUT \n");}
        add_to_out_buf(EMU_API_OK);
        printf("SDEMU: SPEDD TEST 8000 to korvet\n");
        for (i=0; i<0x8000; i++) {
            add_to_out_buf(0);
        }
        // out_buffer_size=0x8000;
        in_buffer_size=0;
        break;
    }

    case 0xf1: { // SPEED TEST - in 0x8000 bytes
        if (extrom_debug) {printf("CMD: SpeedTest IN \n");}
        add_to_out_buf(EMU_API_OK);
        printf("SDEMU: SPEDD TEST 8000 from korvet\n");
        emu_stage=EMU_STAGE2_WRSPPEDTEST;
        in_buffer_size=0;
        break;
    }

    default: {
        printf("SDEMU: unsupported CMD '%02x'\n",in_buffer[0]);
        break;
    }
    }
}

void parse_write(void) {
    unsigned char crc;
    int result_status=0;

    // char fname[1024]="extrom/stage2.rom";
    switch (emu_stage) {

    case EMU_STAGE1: { // Stage1 loader send GET_IMAGE_X where X-0..8 (8 by default)
        emu_stage1();
        break;
    }

    case EMU_STAGE2_WAITCMD: {
        if (in_buffer_size == 5) {
            e_cmd=in_buffer[0];
            e_drv=in_buffer[1];
            e_trk=in_buffer[2];
            e_sec=in_buffer[3];
            e_crc=in_buffer[4];
            crc=e_cmd+e_drv+e_trk+e_sec-1;

            if (extrom_debug) {
                printf("CMD: %02x, DRV:%02x, TRK: %03d, SEC: %02d CRC:%02x (%02x)%s\n",e_cmd,e_drv,e_trk,e_sec,e_crc,crc, e_crc==crc ? "" : " - ERROR !!!");
            }

            if (crc == e_crc) {
                emu_api_cmd();
            } else {
                add_to_out_buf(EMU_API_FAIL);
            }
        }
        break;
    }
    case EMU_STAGE2_WRITE128: {
        if (in_buffer_size == 128) {
            emu_write128();
            in_buffer_size=0;
            emu_stage=EMU_STAGE2_WAITCMD;
        }
        break;
    }

    case EMU_STAGE2_GETFILENAME: {
        if (in_buffer_size == 14) {
            emu_getfilename();
            in_buffer_size=0;
            emu_stage=EMU_STAGE2_WAITCMD;
        }
        break;
    }

    case EMU_STAGE2_CREATE_KDI: {
        if (in_buffer_size == 14) {
            emu_createkdi();
            in_buffer_size=0;
            emu_stage=EMU_STAGE2_WAITCMD;
        }
        break;
    }

    case EMU_STAGE2_GETFOLDER: {
        if (in_buffer_size == 14) {
            emu_setfolder();
            in_buffer_size=0;
            emu_stage=EMU_STAGE2_WAITCMD;
        }
        break;
    }

    case EMU_STAGE2_WRSPPEDTEST: {
        if (in_buffer_size == 0x8000) {
            in_buffer_size=0;
            emu_stage=EMU_STAGE2_WAITCMD;
        }
        break;
    }
    default: {
        printf("ERROR: unsupported stage '%d' size:%d\n",emu_stage,in_buffer_size);
    }
    }
    return;
}

void ext_rom_api_write(byte Value) {
    if (in_buffer_size == EXT_BUF_SIZE) {
        printf("WARNING: in_buffer FULL, ignore\n");
    } else {
        // printf("DBG: in_buffer: %02d=%02x\n",in_buffer_size,Value);
        in_buffer[in_buffer_size++]=Value;
    }
    parse_write();
}

void add_to_out_buf(byte Value) {
    if (out_buffer_size == EXT_BUF_SIZE) {
        printf("WARNING: out_buffer FULL, ignore\n");
    } else {
        // printf("DBG: in_buffer: %02d=%02x\n",out_buffer_size,Value);
        out_buffer[out_buffer_size++]=Value;
    }
}

void add_block_to_out_buf(int size,byte *block) {
    int i;
    for (i=0; i<size; i++) {
        add_to_out_buf(block[i]);
    }
}

byte ext_rom_api_read(void) {
    byte Value;
    if (out_buffer_size == 0) {
        printf("WARNING: out_buffer_size Read from empty buffer, ignore\n");
    } else {
        Value=out_buffer[0];
        out_buffer_size--;
        if (out_buffer_size>0) {
            memmove(out_buffer,out_buffer+1,out_buffer_size);
        }
    }
    return Value;
}
