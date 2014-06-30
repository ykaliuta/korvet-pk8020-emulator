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

/* 
 * This code writen by Alexander Stepanov  forth32 <at> mail.ru
 * http://zx-pk.ru/showthread.php?t=23456
 * http://zx-pk.ru/showthread.php?t=23458
 * and bit modified by Sergey Erokhin
 * jun 2014
 */ 

#include <allegro.h>
#include <stdio.h>
#include <sys/poll.h>
#include <stdlib.h>
#include <termios.h>

#include <time.h>

#include "korvet.h"
#include "lan.h"

unsigned int 	LAN_Addr=0^0x0f;

// увеличивается время поиска РМУ в сети и ожидания при потере пакетов

#ifndef LAN_SUPPORT
void LAN_Write(int Addr,byte Value) {
    return;
}
byte LAN_Read(int Addr) {
// fixfor KORNET work
    return 0x85;
}
void LAN_Init(void) {};
void LAN_poll(void) {};
#else

char        LAN_ttdev[1000];     // последовательный порт сети
char        LAN_logfile[1000]="";  // Имя файла для лога 


//*******************************************************************
//*   Определение кольцевого буфера для приема потока данных из сети
//*******************************************************************
#define lqsize 132000    // Размер буфера, чтобы обработчик не захлебывался при быстром поступлении данных
char lanbuf[lqsize];     // сам буфер
int lbip,lbop;           // указатели позиции записи и чтения в буфере

unsigned int lanfd; // Файловый дескриптор последовательного порта
FILE* netlog=0;     // Поток для записи лог-файла

struct termios sioparm; // описатель параметров последовательного порта
int i3pending;         // Признак ждущего прерывания от сетевого ВВ51

struct pollfd pf;   // структура для опроса состояния очередей канала

sigset_t ss;            // маска системных сигналов


//**************************************************
//*  Запись байтов в кольцевой буфер ввода
//**************************************************
void lput(char c) {

    lanbuf[lbip++]=c;
    if (lbip == lqsize) lbip=0;   // по достижении конца заворачиваем к началу буфера
    if (lbip == lbop) {
        printf("\n - переполнение буфера ввода\n"); // переполнение - дальнейшая работа бессмысленна, из-за потери данных
        exit(0);
    }
}

//**************************************************
//*  Чтение байтов из кольцевого буфера ввода
//**************************************************
char lget() {

    char c;
    if (lbip == lbop) return 0;   // буфер пуст  - читать нечего
    c=lanbuf[lbop++];
    if (lbop == lqsize) lbop=0;   // заворачиваем в начало
    return c;
}

//******************************************************
//*  Проверка наличия данных в кольцевом буфере ввода  *
//*
//*  Возвращает 0, если данные есть, или !=0, если нет
//******************************************************
int lq_empty() {
    return (lbip == lbop);
}


//**************************************************
//*  Проверка наличия принимаемый байтов           *
//*
//*  0 - данных в буфере нет,  !=0 - есть
//**************************************************
unsigned int LAN_incoming_available() {

    pf.events=POLLIN;
    poll(&pf,1,0);
    return (pf.revents&POLLIN);
}

//**********************************************************
//*  Проверка наличия данных очереди отправляемых байтов   *
//*
//*  0 - данные есть   !=0 - очередь пуста
//**********************************************************
unsigned int LAN_wepmty() {

    pf.events=POLLOUT;
    poll(&pf,1,0);           // При некоторых условиях этот poll может конфликтовать с обработчиком сигнала SIGIO. Пока вроде все работает
    //ppoll(&pf,1,0,&ss);    // Так правильнее, но иногда получаем мертвый вис внутри этого вызова. Почему - пока х.з.
    return (pf.revents&POLLOUT);
}

//*********************************************************
//*  Эмуляция записи в регистры сетевого ВВ51             *
//*
//*  Addr - адрес порта, используется только младший бит
//*********************************************************
void LAN_Write(int Addr,byte Value) {

    unsigned char c=Value;
    int res=0;

    // запись в регистр управления не поддерживается -
    // настройки сетевого контроллера однозначны, 19200-8-O-1

    if (lanfd<=0) return;  // открытого сетевого порта нет

    if ((Addr&1) == 0) {
        // запрещаем обработку сигналов на время записи байта
        sigprocmask(SIG_BLOCK, &ss,0);
        // ждем опустошения буфера передатчика
        while (LAN_wepmty()==0);
        // отправляем байт в сеть
        while (res<=0) {
            res=write(lanfd,&c,1);
            if (res <=0) {
                printf("\n- ошибка записи в последовательный канал ");
                perror("errno: ");
                fflush(stdout);
            }
        }
        // чистим выходную очередь
        tcdrain(lanfd);
        // возобновляем обработку сигналов
        sigprocmask(SIG_UNBLOCK, &ss,0);
        // записываем протокол
        if (netlog != 0) {
            fputc(c,netlog);
            fputc(0,netlog);     // код 0 - данные для записи
        }
        // usleep(10000000);
        //usleep(1000);
    }
}


//********************************************************
//*  Эмуляция чтения регистров сетевого ВВ51             *
//*
//*  Addr - адрес порта, используется только младший бит
//********************************************************
byte LAN_Read(int Addr) {

    unsigned char c=0,nl=0;
    unsigned int ql;  // размер выходной очереди
    byte res=0x80;    // DSR=1 как в реальном корвете

    if (lanfd<=0) return 0;  // открытого сетевого порта нет
    if ((Addr&1) == 0) {

    // *** регистр данных ***
        // запрещаем обработку сигнала SIGIO
        sigprocmask(SIG_BLOCK, &ss,0);
        i3pending=0;    // сбрасываем запрос на прерывание, как в реальном ВВ51 при чтении байта
        if (!lq_empty()) {
            //данные есть
            c=lget();
            if (netlog != 0) {
                fputc(c,netlog);
                fputc(1,netlog);   // код 1 - прочитанные данные
            }
        }
        else {
            // буфер пуст
            c=0xff;
            if (netlog != 0) {
                fputc(0xff,netlog);
                fputc(2,netlog);  // код 2 - чтение при пустом буфере
            }
        }
        sigprocmask(SIG_UNBLOCK, &ss,0);
        return c;
    }
    else {

        // *** регистр состояния ***
        ql=LAN_wepmty();               // читаем размер очереди передатчика
        if (LAN_wepmty()) res|=5;	// очередь пуста -    TxEmpty=1  TxRDY=1
        // иначе         -    TxEmpty=0  TxRDY=0
        if (lq_empty()) return res;   // данных нет -  RxRdy=0
        else 		  return res|2; // данные есть - RxRdy=1
    }
}


//******************************************************************
//* Формирование прерывания при наличиии входящего по сети байта
//******************************************************************
void LAN_poll() {

    if ((lanfd<=0)||lq_empty() || (i3pending==1)) return;  // открытого сетевого порта нет,
    // или нет принятых данных, или еще не обработали старое прерывание
    PIC_IntRequest(3);    // формируем запрос прерывания
    i3pending=1;          // флаг необработанного прерывания
}

//***********************************************
//*  Обработчик сигнала ввода-вывода SIGIO
//***********************************************
void sio_handler(int sig) {

    char c;

    while (LAN_incoming_available() != 0) {  // пока во входной очереди есть байты
        read(lanfd,&c,1);      // читаем очередной байт
        lput(c);               // и в кольцевой буфер его
    }

}

char *get_rmp_ptx_path(void) {
    char *ret=NULL;
    static char buf[1024];
    FILE *f=fopen(LAN_PTX_FILE,"r");
    if (f) {
        fscanf(f,"RMP PTX: %s",buf);
        ret=buf;
        fclose(f);
    }
    return ret;
}



//***********************************************
//* Инициализация сетевого адаптера ВВ51        *
//***********************************************

void LAN_Init(void) {

    char npts[100];      // строка для приема имени псевдотерминала
    int err;
    struct sigaction iosig;  // описатель обработчика сигнала

    lanfd=0; 	// пока нет открытого порта;
    netlog=0;       // лог пока не открыт

    if (strlen(LAN_ttdev) == 0) return;   // сетевой адаптер не описан в конфиге - пропускаем

    if ( (0==strcmp("rmp",LAN_ttdev)) || (0==strcmp("RMP",LAN_ttdev)) ) {
            printf("LAN RMP mode initalized\n");
            strcpy(LAN_ttdev,"/dev/ptmx");
            LAN_Addr=0 ^ 0x0f;
    } else if ((0==strcmp("rmu",LAN_ttdev)) || (0==strcmp("RMU",LAN_ttdev)) ) {
        if (get_rmp_ptx_path() != NULL) {
            strncpy(LAN_ttdev,get_rmp_ptx_path(),1000);

            if (LAN_Addr == 0x0f) {
                LAN_Addr = 2 ^ 0x0f;
            }

            printf("LAN RMU mode initalized (RMU NUM: %-2d, server ptx=%s)\n",LAN_Addr^0x0f,LAN_ttdev);
        } else {
            printf("WARNING: LAN RMU mode ignored, can't find %s\nrun -s rmp before\n",LAN_PTX_FILE);
            exit(-1);
        }
    }


    //lanfd = open(LAN_ttdev, O_RDWR | O_NOCTTY | O_SYNC);  // для синхронной заиси - надежнее, но с тормозпми
    lanfd = open(LAN_ttdev, O_RDWR | O_NOCTTY);             // для асинхронной записи - менее надежно, но летает пулей

    if (lanfd == -1) {
        printf("\nПоследовательный порт %s не открывается\n", LAN_ttdev);
        lanfd=0;   // флаг отсутствия сети
        return;
    }

    printf("\nПоследовательный порт %s открыт", LAN_ttdev);

    // проверяем на псевдотерминал
    if (grantpt(lanfd) == 0) {
        unlockpt(lanfd);         // разблокируем подчиненный терминал
        ptsname_r(lanfd,npts,100); // получаем его имя
        if (npts != 0) {
            printf("\nПодчиненный псевдотерминал - %s",npts);
            FILE *f=fopen(LAN_PTX_FILE,"wt");
            if (f != NULL) {
                fprintf(f,"RMP PTX: %s",npts);
                fclose(f);
            } else {
                printf("can't create %s, -s RMU can't work !\n",LAN_PTX_FILE);
            }
           
        } else {
            printf("\nНевозможно определить имя подчиненного терминала");
        }
        fflush(stdout);
    }

    // Настройка физических параметров последовательного порта
    bzero(&sioparm, sizeof(sioparm));
    sioparm.c_cflag = B19200 | CS8 | CLOCAL | CREAD | PARENB | PARODD;  // 19200-8-О-1, аппаратные линии управления потоком игнорируются
    sioparm.c_iflag = 0;  // INPCK;
    sioparm.c_oflag = 0;
    sioparm.c_lflag = 0;
    sioparm.c_cc[VTIME]=5;  // таймаут последовательного порта - 0.5с. Можно попробовать его выкинуть, но тогда возможны мертвые зависания в ожидании данных
    sioparm.c_cc[VMIN]=0;
    tcsetattr(lanfd, TCSANOW, &sioparm);

    // Открытие файла протокола
    //
    //  Формат файла протокола: двухбайтовые записи
    //    Младший байт - принятый или переданный байт
    //    Старший байт - флаг напрвления:
    //       0 - переданный байт     1 - принятый   2 - чтение при пустом входном буфере
    //
    // Открываем лог-файл, если он определен в конфигурации
    if (strlen(LAN_logfile) != 0) netlog=fopen(LAN_logfile,"wb");
    if (netlog < 0) {
        printf("\nОшибка открытия файла %s",LAN_logfile);
        netlog=0;   // в случае ошибки от
    }
    if (netlog >0) printf("\nОткрыт файл сетевого протокола %s",LAN_logfile);


    // настройка обработчика сигнала SIGIO
    memset(&iosig,0,sizeof(iosig));  // чистим от мусора
    iosig.sa_handler=sio_handler;   // адрес обработчика
    
    // формирование маски сигнала SIGIO
    sigemptyset(&ss);
    sigaddset(&ss, SIGIO);         // Список блокирующихся сигналов
    iosig.sa_mask = ss;
    err=sigaction(SIGIO, &iosig ,0);  // устанавливаем обработчик
    if (err != 0) {
        printf("\nОшибка установки обработчика сигнала SIGIO");
        exit(0);
    }
    fcntl(lanfd, F_SETOWN ,getpid());   // Разрешаем генерацию сигнала SIGIO
    fcntl(lanfd, F_SETFL, FASYNC);	    // включаем асинхронный режим

    i3pending=0;   // ждущих прерываний нет
    // инициализация кольцевого буфера
    lbip=0;
    lbop=0;
    // инициализация структуры запроса ppoll()
    pf.fd=lanfd;

    printf("\nАдрес эмулятора в сети - %1x\n",(~LAN_Addr)&0xf);
    fflush(stdout);

}

#endif

/*
Стартуем первый эмулятор.

Код:

./kdbg -n0 -s/dev/ptmx -l server.log -a disk/unsort18.kdi -b disk/MIKRDOS3.KDI
Последовательный порт /dev/ptmx открыт
Подчиненный псевдотерминал - /dev/pts/2
Открыт файл сетевого протокола server.log
Адрес эмулятора в сети - 0
Driver: ALSA

Это у нас будет РМП. В качестве последовательного устройства указываем мультиплексор виртуальных терминалов /dev/ptmx. Будет сформирован лог server.log.
При старте эмулятор указал, что сформирован подчиненный последовательный порт /dev/pts/2. Эта информация нам нужна для запуска второй копии эмулятора.

Теперь стартуем второй эмулятор

Код:

./kdbg -n2 -s/dev/pts/2 -l client.log
Открыт файл сетевого протокола client.log
Адрес эмулятора в сети - 2
Driver: ALSA

Это будет РМУ с адресом 2, сетевой лог сохраняется в client.log. В качестве последовательного порта указываем имя подчиненного виртуального терминала, выданное нам первой копией эмулятора.

Все, сеть из 2 корветов готова. Можно начинать развлекаться. Я потестировал сеть в программах STS и YP - работает вроде без сбоев в обе стороны.

*/