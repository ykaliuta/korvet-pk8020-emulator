//===================================================================
//   Проект КОРВЕТ-Extrom, микропрограмма внешнего контроллера
// Предназначен для эмуляции дисков CP/M через боковой разъем ППИ 3
//===================================================================
//  Автор - forth32 (Alexader Stepanov),  forth32@mail.ru
//
//  Процессор - Atmega32, 8 MHz, внутренний Rc-генератор
//
//  Подключение периферии к портам процессора:
//
// Pin  сигнал AVR   Описание
//=======================================================================
//  1       PB0    Светодиод индикации обращения к диску (и просто для отладки), анодом к +5V, катодом сюда через резистор
//  5       -SS    Сигнал выбора SPI-устройства, подключается ко входу SS карты
//  6       MOSI   Выход данных SPI, ко входу данных карты (DI)
//  7       MISO   Вход данных SPI, к выходу данных карты (DO)
//  8       MSCK   Сигнал тактирования SPI, ко входу синхронизации карты (SC)
//  9       RESET  Кнопка сброса, обязательно подтянуть к +5v!
// 14       RxD    Приемник последовательного порта  
// 15       ТxD    Передатчик последовательного порта  
// 16       Int0   Вход сигнала control для определения презапуска корвета
// 17       Int1   Вход сигнала -OBF (PC7 BB55) - запрос  от корвета на передачу байта
// 18       PD4    Выход сигнала -ACK (PC6 BB55) - подтверждение приема байта от корвета, длина не менее 300 нс
// 19       PD5    Вход сигнала IBF (PC5 ВВ55) - подтверждение передачи байта в корвет
// 20       PD6    Выход сигнала -STB (PC4 BB55) - запрос на передачу байта в корвет
// 21       PD7    Адрес A0 эмулируемого ПЗУ, подключается у порту PB0 ВВ55
// 33-40 PA0..7    Порт ввода данных PA0-PA7 ВВ55
//
//
#define F_CPU 8000000UL  // 8 MHz

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include <avr/pgmspace.h>

#include "fs.h"

// Параметры кольцевых буферов накопления UART
#define USART_RX_BUFFER_SIZE 16     // размер буфера приемника
#define USART_TX_BUFFER_SIZE 128     // размер буфера передатчика
#define USART_RX_BUFFER_MASK ( USART_RX_BUFFER_SIZE - 1 )  // битовая маска маркеров приемника
#define USART_TX_BUFFER_MASK ( USART_TX_BUFFER_SIZE - 1 )  // битовая маска маркеров передатчика

// Распределение кольцевых буферов в памяти
static unsigned char USART_RxBuf[USART_RX_BUFFER_SIZE]; // буфер приемника
static volatile unsigned char USART_RxHead;				// маркер начала
static volatile unsigned char USART_RxTail;				// маркер конца
static unsigned char USART_TxBuf[USART_TX_BUFFER_SIZE]; // буфер передатчика
static volatile unsigned char USART_TxHead;				// маркер начала
static volatile unsigned char USART_TxTail;				// маркер конца

static unsigned char fb[256];     // буфер для страницы загрузчика, а также логических блоков CP/M

// процедуры обслуживания консольного псевдофайлового потокоа вывода
static int fputchar(char c, FILE *stream);
static FILE mystdout = FDEV_SETUP_STREAM(fputchar, 0, _FDEV_SETUP_WRITE);

unsigned char kbhit( void );
void readstr();


static volatile unsigned char boot_flag;   // Флаг начальной загрузки (фаза 1): 0 - корвет еще не загружен, 1 - процесс начальной загрузки окончен

static volatile char lspt[2];   // Число логических секторов на дорожку для каждого из образов A и B
static volatile char Aname[16]; // имя файла образа A
static unsigned char bios_flag=0;  // флаг действующей подмены образа A на образ BIOS
static unsigned char enable_bios_flag=1;  // флаг разрешения такой подмены
static unsigned char roflag[2];		  // флаг "только чтение" каждого диска: 0-чтение/запись  1-только чтение


//------------------- Процедуры обслуживания последовательного порта ---------------------------

//***********************************************
//* Обработчик прерывания от приемника USART    *
//***********************************************
ISR (USART_RXC_vect) {

unsigned char data;
unsigned char i;

data = UDR; // принятый байт данных
// вычисление маркера начала буфера
i = ( USART_RxHead + 1 ) & USART_RX_BUFFER_MASK;
USART_RxHead = i;
USART_RxBuf[i] = data; // сохраняем байт байт в буфере
}

//*************************************************
//* Обработчик прерывания от передатчика USART    *
//*************************************************
ISR (USART_UDRE_vect) {

unsigned char i;

if ( USART_TxHead != USART_TxTail ) {
// если еще не все данные переданы
  i = (USART_TxTail + 1) & USART_TX_BUFFER_MASK; // вычисляем маркер конца буфера
  USART_TxTail = i;
  UDR = USART_TxBuf[i];  // выводим очередной байт из буфера в передатчик
}
else  {
// все данные уже переданы
  UCSRB &= ~(1<<UDRIE); // запрещаем прерывания от передатчика
}
}

//**************************************************
//* Ожидание полного освобождения буфера USART     *
//**************************************************
void pflush() {
 while (USART_TxHead != USART_TxTail);
} 

//******************************************
//* Передача байта для посылки в USART     *
//******************************************

void putch(char data) {

unsigned char i;


i = ( USART_TxHead + 1 ) & USART_TX_BUFFER_MASK; // вычисляем новый маркер начала
while ( i == USART_TxTail );  // Ждем освобождения места в буфере
USART_TxBuf[i] = data;  // Сохраняем данные в буфере
USART_TxHead = i;       // новый индекс начала
UCSRB |= (1<<UDRIE);          // открываем прерывания от передатчика
}

//******************************************
//* Получение очередного принятого байта   *
//******************************************

char getch(void) {

unsigned char i;

while ( USART_RxHead == USART_RxTail );  // Ждем появления байта в буфере
i = ( USART_RxTail + 1 ) & USART_RX_BUFFER_MASK; // вычисляем новый маркер конца
USART_RxTail = i;
putch(USART_RxBuf[i]);   // эхо-печать введенного байта
return USART_RxBuf[i];   // возвращаем байт из буфера
}

//**********************************
//*  Печать байта в HEX            *
//**********************************
void printhex(unsigned char c) {

unsigned char r;

r=((c>>4)&0xf)+'0';
if (r>'9') r+=7;
putch(r);		// старший байт

r=(c&0xf)+'0';
if (r >'9') r+=7;
putch(r);		// младший байт
}

//**********************************
//* Вывод байта в файловый поток   *
//**********************************
static int fputchar(char c, FILE *stream) {

if (c == '\n') fputchar('\r', stream); // заменяем <cr> на <cr><lf>
putch(c);
return 0;
}

//***************************************
//* Печать строки фиксированной длины   *
//***************************************
void printstrl(unsigned char* str,unsigned  char len) {

char i;
for(i=0;i<len;i++) fputchar(str[i],stdout);
}

//****************************************************************
//* Проверка наличия непрочитанных байт в буфере приемника USART *
//****************************************************************

unsigned char kbhit( void )  {
	return ( USART_RxHead != USART_RxTail );
}


//--------  Процедуры работы с SPI и SD-картой ---------------


//*************************************
//* Отправка и прием байта через SPI  *
//*************************************
unsigned char spi_send(unsigned char c) {

SPDR = c;        // отправляем байт
while((SPSR & (1<<SPIF)) == 0);  // ждем ответа
return SPDR;   // выходной байт
}

#define spi_receive() spi_send(0xff)    // макрос приема байта из SPI

//*************************************
//* Отправка команды SD-карте         *
//*************************************
unsigned char send_sd_cmd(char cmd, unsigned long adr) { 

unsigned char res;
unsigned int pass;

spi_send (cmd|0x40);	// код команды

spi_send ((adr>>24)&0xff);	// адрес
spi_send ((adr>>16)&0xff);
spi_send ((adr>>8)&0xff);
spi_send (adr&0xff);
 
spi_send (0x95);   // CRC

// Ждем ответа R1
for(pass=0;pass<0xffff;pass++) {
  res=spi_receive();
  if ((res&0x80)==0) break;
}
if (pass != 0xffff)return res;
else return 0xaa;
}

//**************************************************
//*   Чтение ответа на команду SD-карте            *
//**************************************************
unsigned char read_sd_data(unsigned char* buf,unsigned int len,unsigned int offset) {

unsigned char res;
unsigned int i;
unsigned int ln=0;

for(i=0;i<0xffff;i++)  {	
  //Ждем начала пакета данных
  res=spi_receive();   // должен быть маркер FE
  if (res==0xfe) break;
}
 
if (i==0xffff) { 
  printf_P(PSTR("\n Таймаут данных - %02x  %i"),res,len);
  return 1;
}

// Цикл побайтового приема всего сектора
for (i=0;i<512;i++) {
 res=spi_receive();
 if ((i<offset)||(i>offset+len)) continue;  // вырезаем из всего потока байтов нужный кусок
 buf[ln++]=res;
} 
spi_receive();	// CRC-H
spi_receive();  // CRC-L
return 0;
}

//**************************************************
//*   Чтение блока с SD-карты                      *
//**************************************************

unsigned char read_block (unsigned char* buf, unsigned long adr, unsigned int len,unsigned int offset) { 

unsigned char res;

//printf_P(PSTR("\n - read: %lx - %i"),adr,len);
res=send_sd_cmd(17,adr);	// CMD17 - чтение одного блока

if (res!=0x00) return res;	//ошибка
spi_send (0xff);
return read_sd_data(buf,len,offset);
}

//**************************************************
//*   Запись блока на SD-карту                     *
//**************************************************
unsigned char write_block (unsigned char* buf, unsigned long adr) { 

unsigned char res;
unsigned int i;
 
res=send_sd_cmd(24,adr);	// CMD24 - запись одного блока
if (res!=00) return res;	// ошибка
spi_send (0xff);
 
spi_send (0xfe);	// Токен начала данных
for (i=0;i<512;i++)  spi_send(buf[i]); // Блок данных

spi_send (0xff);	// CRC
spi_send (0xff);

res=spi_receive();
if ((res&0x05)==0) return 0xaa;	// Ошибка - блок данных отвергнут картой
 
i=0;
for(i=0;i<0xffff;i++) {  //Ждем окончания busy состояния
  res=spi_receive();
  if (res==0xff) break;
}
if (i==0xffff) return 0xaa;  // ошибка - таймаут записи
return 0;
}


//---------- Обработчики внешних прерываний -------------------
//

//**************************************
//* Чтение байта из EEPROM
//**************************************
unsigned char EEread(int adr) {

EEAR=adr;
EECR|=(1<<EERE);   // строб чтения
return EEDR;  // выбираем байт и возвращаем его 
}  


//**************************************
//* Int0 - от сигнала control          *
//**************************************

//   CONTROL - сигнал, вырабатываемый ОПТС 
//  Положительный фронт - начало последовательности загрузки
//  Отрицательный фронт - запрос на отключение от шины, если не отработать - будет ошибка шины ОПТС
//
ISR (INT0_vect) {

unsigned int adr;
unsigned char passcount;

if (bit_is_clear(PIND,2)) {
 // Сигнал Control ушел в 0 - немедленно отключаемся от шины !
 DDRA=0;   // порт - на ввод
 PORTA=0; // отключаем все подтяжки
 boot_flag=0; // сбрасываем флаг загрузки
 DDRD=_BV(1);     // TxD на вывод, остальные линиии порта на ввод во избежании конфликтов с адресом на канале C
 return;
}
  
// Сигнал control стал 1 - начинаем процесс начальной загрузки
DDRA=0xff;   // шину - на вывода
//sei();       // разрешаем новое прерывание для перезапуска системы
// Фиксированная последовательность адресов 4 5 6 7
PORTA=EEread(4);
while(bit_is_clear(PIND,7));
PORTA=EEread(5);
while(bit_is_set(PIND,7));
PORTA=EEread(6);
while(bit_is_clear(PIND,7));
PORTA=EEread(7);
while(bit_is_set(PIND,7));
// Далее выдаем по кольцу данные с адресов 00-FF
adr=0;
passcount=0;  // счетчик полных проходов
// цикл полной передачи данных за 3 прохода
while(passcount<3) {
  PORTA=EEread(adr++);	          // четные байты
  while(bit_is_clear(PIND,7));
  PORTA=EEread(adr++);            // нечетные байты
  while(bit_is_set(PIND,7));
  if (adr >= 0x100) { 
    adr=0;         // заворачиваем адрес
    passcount++;   // счетчик проходов++
  }  
}  
// 3 прохода отработали - настраивает порты в основной режим работы

DDRD=0x52;              // рабочий режим - TxD,-STB, -ACK - на вывод
PORTD=0x50;		// -STB=1    -ACK=1
boot_flag=1;  // взводим флаг загрузки
}


//*************************************************
//* HEX-дамп области памяти                       *
//*
//*  offset добавляется к адресу при печати
//*************************************************
/*
void dump(char* sbuf, int len) {
int i,j;
unsigned char ch;

fputchar('\n',stdout);
for (i=0;i<len;i+=16) {
  printhex(i>>8); printhex(i&0xff); printstrl(": ",2); 
  for (j=0;j<16;j++){
   if ((i+j) < len) { printhex(sbuf[i+j]&0xff); putch(' ');}
   else printf_P(PSTR("   "));
  }
  printf_P(PSTR(" *"));
  for (j=0;j<16;j++) {
   if ((i+j) < len) {
    ch=sbuf[i+j];
    if ((ch < 0x20)||(ch > 0x7e)) putch('.');
    else putch(ch);
   } 
   // заполнение пробелами для неполных строк 
   else putch(' ');
  }
  printf_P(PSTR("*\n"));
 }
}
*/


//******************************************************************
//*  Пересылка байта в корвет через двунаправленный порт А ВВ55
//******************************************************************
void iop_send_byte(unsigned char c) {

while(bit_is_set(PIND,5));   // ждем IBF=0 - освобождения буфера ввода
PORTA=c;   // выводимый байт
PORTD &= 0xbf;   //pd6 (-STB) = 0
while(bit_is_clear(PIND,5));   // ждем IBF=1 - подтверждение приема
PORTD |= 0x40;   //pd6 (-STB) = 1 - снимаем строб
}

//******************************************************************
//*  Прием байта из корвета через двунаправленный порт А ВВ55
//******************************************************************
unsigned char iop_get_byte() {

unsigned char c;
while(bit_is_set(PIND,3));   // ждем OBF=0 - готовность данных в буфере вывода
DDRA=0;		             // порт на ввод!
PORTD &= 0xef;   //pd4 (-ACK) = 0
asm volatile("nop");
asm volatile("nop");
asm volatile("nop");
c=PINA;                  // забираем вводимый байт
PORTD |= 0x10;  	 //pd4 (-ACK) = 1 - снимаем строб
while(bit_is_clear(PIND,3));   // ждем OBF=1 - снятие готовности данных
DDRA=0xFF;		 // порт на вывод
return c;
}


//***********************************************************
//*   Заливка в корвет загрузчика 2 фазы    
//* 
//*  Берется из файла LOADER.BIN или одного из ROMn.BIN
//***********************************************************
void send_loader() {

unsigned char loader_size;  // размер загрузчика в блоках по 256 байт
unsigned int res,blk,i,sts;
unsigned char* sbuf=getsbufptr();
unsigned char nfile;  // # файла для загрузки


printf_P(PSTR("\nЗагрузка фазы 2\n"));

// Принимаем от загрузчика номер файла
nfile=iop_get_byte();
// Формируем имя файла
if (nfile == 8) strcpy(sbuf, "LOADER.BIN");
else {
 strcpy(sbuf,"ROMX.BIN");
 sbuf[3]='0'+nfile;
} 
printf_P(PSTR("\nLoader file: %s"), sbuf);

res=fs_open();
if (res != 0) { printf_P(PSTR("\n - нет файла!")); return; }
fs_getfilesize();  
loader_size=fs_tmp>>8;   // вычисляем размер файла в 256-байтовых блоках

fs_lseek(6,0);    // +6 - старший байт адреса загрузки
fs_read(fb, 1, &res);


iop_send_byte(fb[0]);         // отсылаем загрузчику адрес размещения файла в памяти
printf_P(PSTR("\nLoader base: %x"), fb[0]);
iop_send_byte(loader_size);   // и число загружаемых блоков
printf_P(PSTR("\nLoader size: %x"), loader_size);

// Пересылка загрузчику всех блоков файла
fputchar('\n',stdout);
fs_lseek(0,0);    // встаем на начало файла
for(blk=0;blk<loader_size;blk++) {
 sts=fs_read(fb, 256, &res);
 if (sts != 0) { printf_P(PSTR("\n read error %i"),lastError); break;}
 if (res == 0) break;  // конец файла
 for(i=0;i<256;i++) iop_send_byte(fb[i]); // отсылаем очередные 256 байт
 printf_P(PSTR("\r Block # %i"),blk);
} 
printf_P(PSTR("\nЗагрузка фазы 2 окончена"));
}

//######################################### Далее идут процедуры работы с образами дисков и программным API ############################

//**********************************************************************************************
//*   Монтирование образа KDI
//*   С образа считывается и разбирается информационный сектор для получения логического SPT
//*
//*  dsk - номер диска, 0 или 1
//*  filename - имя файла, не более 8.3, заканчивается 0.
//*  prefix=0 - монтирование из корневого каталога
//*         1 - из каталога DISK
//**********************************************************************************************
void mount_disk(char dsk,char* filename, char prefix){

unsigned char* sbuf=getsbufptr();  // буфер сектора VinxFS
unsigned int res;

sbuf[0]=0;
if (prefix == 1) strcpy(sbuf,"DISK/");
strcat(sbuf,filename);
if (dsk == 1) fs_swap();	// диск В - переключаем на файл 1
if (fs_open() != 0) { printf_P(PSTR("\nДиск %c: - нет файла %s"),'A'+dsk,sbuf); return; }
fs_read(fb, 32, &res);  // читаем инфосектор файла
lspt[dsk]=fb[16];       // достаем логический SPT
printf_P(PSTR("\n Disk %i: SPT=%i\n"),dsk,lspt[dsk]);
if (dsk == 1) fs_swap();	// возвращаемся обратно на файл 0
//dump(fb,32);
pflush();
}



//**********************************************************************************************
//* Int1 - от сигнала -OBF
//*
//*  В этом прерывании обрабатываются запросы, поступающие от корвета
//*
//*  Все запросы начинаются 1-байтовым кодом команды, за которым следует пакет параметров из 4 байт. 
//*--------------------------------------------------------
//*  
//*  Команды обмена секторами данных:
//*   
//*  CMD:    DS    1    // код команды: 01-чтение  02-запись
//*  DRV:    DS    1    // номер устройства, 0-A,  1-B
//*  TRK:    DS    1    // номер дорожки
//*  SEC     DS    1    // номер сектора (размер сектора всегда 128 байт
//*  CSUM:   DS    1    // Контрольная сумма предыдущих 4 байт, равна их беззнаковой 8-битной сумме минус 1:  CSUM=(CMD+DRV+TRK+SEC)&0xFF-1
//*
//*  Команды измерителя скорости. Все поля пакета, кроме кода команды, не имеют значения.
//*
//*   F0 - прием 8000h байта мусора
//*   F1 - передача 8000h байтов мусора
//* 
//*  Комадны управления работой контроллера
//*
//*   00 - пустая операция, всегда возвращает ответ ОК (1)
//*   A0 - включить/выключить подстановку системных дорожек. DRV всегда 0, TRK=0 - выключить, 1 - включить
//*   
//**********************************************************************************************
ISR (INT1_vect) {

unsigned char cmd,drv,trk,sec,csum,rcsum;  
unsigned long offset;   // смещение до сектора от начала образа файла  
unsigned int i,res;  
GICR&=~_BV(INT1);         // Запрещаем повторный вход в прерывание
sei();  
if (bit_is_clear(PIND,2)) return;   // нажата кнопка RESET, Control ушел в 0, ничего не делаем.
PORTB&=~_BV(0); 	            // светодиод включить
// получаем код команды
cmd=iop_get_byte();  
// получаем параметры команды
drv=iop_get_byte();  
trk=iop_get_byte();
sec=iop_get_byte();
csum=iop_get_byte();

//printf_P(PSTR("\ncmd %x, drv %x, trk %x, sec %x"),cmd,drv,trk,sec);
//pflush();

// Проверка контрольной суммы
rcsum=cmd+drv+trk+sec-1;
if (rcsum != csum) {
 printf("\nCS ERR: r:%x c:%x",rcsum,csum);
  goto nocmd;
}  
//while((UCSRA&_BV(UDRE)) == 0); // ждем освобождения передатчика USART

// Подмена образа диска на образ BIOS - только для образа 0 и только если разрешено
if ((drv == 0) && (enable_bios_flag == 1)) {
    if ((trk<2) && (bios_flag == 0)) { mount_disk(0,"SYSTEM.BIN",0); bios_flag=1; }  // переход на подставную систему
    if ((trk>=2) && (bios_flag == 1)) { mount_disk(0,Aname,1); bios_flag=0; }        // переход на реальный диск A
}
if (drv == 1) fs_swap();	// диск B - подставляем второй файл

// Разбор и исполнение команд
switch (cmd) {
     // команда NOP
  case 0:
    UDR='0';  // выводим знак записи
    iop_send_byte(1);   // ответ ОК
    break;

  case 1:
  case 2:
     // ------ чтение/запись сектора -----
     // вычисляем смещение до сектора
     offset=(trk*lspt[drv]+sec);  // смещение в блоках
     fs_lseek(offset*128,0);      // при позиционировании передаем смещение в секторах
     if (cmd == 1) {
       // операция чтения
       UDR='R';  // выводим знак чтения
       if (fs_read(fb,128,&res) == 0)  iop_send_byte(1);   // ответ ОК
       else  iop_send_byte(0);   // ответ ERROR
       for(i=0;i<128;i++) iop_send_byte(fb[i]); // отправляем блок в корвет
       GIFR|=_BV(INTF1);	// сбрасываем ложное ждущее прерывание от обмена данными
     }
     else {
       UDR='W';  // выводим знак записи
       // проверяем защиту записи
       if (roflag[drv] == 1) {
           iop_send_byte(0);   // ответ Error
           break;
       }    
       iop_send_byte(1);   // ответ ОК
       for(i=0;i<128;i++) fb[i]=iop_get_byte(); // получаем блок из корвета
       GIFR|=_BV(INTF1);	// сбрасываем ложное ждущее прерывание от обмена данными
       if ((drv != 0) || (bios_flag == 0) || (enable_bios_flag == 0))  // запись в подставной файл запрещена
           fs_write(fb,128); // на всякий случай запись лучше выключить при отладке - иначе можно испортить KDI при ошибках позиционирования
     }
     break;

   case 0xa0:
     // --------- включение-отключение подстановки системных дорожек
      if (drv == 0) { enable_bios_flag=trk; iop_send_byte(1); } // для диска А выполняем операцию и отвечаем ОК
      else        iop_send_byte(0);   // иначе ответ Error
      UDR='S';
      break;
     
   case 0xf1:
    // ------------ измеритель скорости - прием 32к мусора
      UDR='D';  // выводим знак теста приема
      iop_send_byte(1);   // ответ ОК
      for(i=0;i<0x8000;i++) iop_get_byte(); // получаем мусор из корвета и отбрасываем
      GIFR|=_BV(INTF1);	// сбрасываем ложное ждущее прерывание от обмена данными
      break;
   
   case 0xf0:
    // ------------ измеритель скорости - передача 32к мусора
      UDR='U';  // выводим знак теста передачи
      iop_send_byte(1);   // ответ ОК
      for(i=0;i<0x8000;i++) iop_send_byte(0xeb); // отсылаем мусор в корвет
      GIFR|=_BV(INTF1);	// сбрасываем ложное ждущее прерывание от обмена данными
      break;
   
   
   default:
    printf_P(PSTR("\n? %x "),cmd); // неопределенная команда
    break;
}
if (drv == 1) fs_swap();	// возвращаем на место файл 1
// Сюда идет обход при неправильной контрольеой сумме пакета
nocmd:

PORTB|=_BV(0); 	        // светодиод отключить
GICR|=_BV(INT1);        // Разрешаем Int1 от -OBF ВВ55
}

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//###########   Головная программа       ###########################

void main(void) {

unsigned int i;
unsigned int res;
unsigned char st;
unsigned long adr;
unsigned char infobuf[16];
unsigned char* sbuf;

//----- настройка собаки
wdt_disable();					// собака пока не нужна

// ---------- прерывания

MCUCR= _BV(ISC00)|_BV(ISC11);	// INT0 - Прерывание по обоим фронтам   Int1 - только по отрицательному фронту
GICR=_BV(INT0);                 // Разрешаем Int0

//------ Настройка портов GPIO

DDRA=0;          // порт данных пока на ввод
PORTA=0;	 // отключаем подтяжки
DDRD=_BV(1);     // TxD на вывод
DDRB=_BV(0)|_BV(4)|_BV(5)|_BV(7);  // индикатор, SS, MOSI, SCK - на вывод
PORTB=_BV(0)|_BV(4); 	            // SS=1, светодиод отключить

//--- настройка последовательного порта

//Скорость обмена UART - 1000000, с учетом удвоителя U2X

UBRRH=0;
UBRRL=0;
UCSRA=(1<< U2X);
// Включаем приемник, передатчик и открываем прерывание от них
UCSRB = (1 << RXCIE)|(1<<UDRIE)|(1 << RXEN)|(1 << TXEN );
// Формат кадра - 8N1
UCSRC = (1<<URSEL)|(1<<UCSZ1)|(1<<UCSZ0);

// Инициализируем маркеры буферов
USART_RxTail=0;
USART_RxHead=0;
USART_TxTail=0;
USART_TxHead=0;

// Назначаем поток вывода
stdout = &mystdout;


//---------- Настройка контроллера SPI --------------------


SPCR = (1<<SPE)|(1<<MSTR); // разрешаем SPI, режим master, максимальная скорость
//SPSR = (1 << SPI2X);		   // удваиваем скорость обмена - глюкает с картой, поэтому пока не включаем

//-----------------------------------------------------------------------------
// Открываем прерывания. Настройка закончена, входим в рабочий режим
//-----------------------------------------------------------------------------

boot_flag=0;  // корвет пока не загружен

sei();


// Инициализация SD-карты

_delay_ms(10);  // задержка на запуск карты

printf_P(PSTR("\n   *** Extrom - SD ***\n"));

PORTB|=0x10;   // Поднимаем сигнал SS
for(i=0;i<9;i++) spi_send(0xff);  // 88 перепадов SCK - для ввода карты в SPI-режим
PORTB&=0xef;   // Опускаем сигнал SS. Навсегда - карта всегда выбрана

// Отсылаем команду CMD0 и проверяем готовность карты
st=send_sd_cmd (0,0);
//if (st!=0x01) printf_P(PSTR("\n Ошибка CMD0 - %02x"),st);
spi_send (0xff); // межкомандный промежуток

// Попытка отослать CMD1
i=0;
for(i=0;i<0xffff;i++) {
  st=send_sd_cmd (1,0);	   // CMD1
  spi_send (0xff);         // межкомандный промежуток
  if (st == 0) break;
}

if (i==0xffff) printf_P(PSTR("\n Ошибка CMD1 - %02x"),st);

// Идентифицируем карту

res=send_sd_cmd(10,0);   // команда чтения CID
if (res != 0) printf_P(PSTR("\n Ошибка идентификации - %2x"),i);
read_sd_data(infobuf,16,0);      // блок CID
printf_P(PSTR("\n CARD MID=%2x  OID=%2x"),(unsigned int)infobuf[0],*((unsigned int*)&infobuf[1]));
printf_P(PSTR("\n Product - "));
printstrl(infobuf+3,5);
printf_P(PSTR("\n rev %i  serail %i"),(unsigned int)infobuf[8],*((unsigned int*)&infobuf[9]));

sbuf=getsbufptr();  // получаем указатель на буфер сектора

res=fs_init(); // монтируем файловую систеу на карте
if (res != 0) printf_P(PSTR("\n fs_init error %i"),i);

sbuf[0]=0;
res==fs_opendir();
if (res != 0) printf_P(PSTR("\n opendir error: %02x"),res);

printf_P(PSTR("\n -- Каталог диска --"));
for(i=0;i<20;i++) {
  res=fs_readdir();
  if(FS_DIRENTRY[0] == 0) break;  // конец каталога
  if (res != 0) printf_P(PSTR("\n readdir #%i error: %04x - %04x"),i,res,lastError);
  else {
   FS_DIRENTRY[DIR_Attr] = 0;
   printf_P(PSTR("\n%i: %s"), i,FS_DIRENTRY);
  } 
}
printf_P(PSTR("\n\nTotal %i files\n"), i);

//------------ Запуск и перезапуск загрузки 2 фазы --------------------------
reboot:

PORTB|=_BV(0); 	        // светодиод отключить

while(boot_flag == 0){};  // ждем окончания начальной загрузки корвета
DDRD=0x52;              // рабочий режим - TxD,-STB, -ACK - на вывод
PORTD=0x50;		// -STB=1    -ACK=1

// заливаем и запускаем загрузчик 2 ступени
send_loader();
// снимаем флаги Readonly
roflag[0]=0;
roflag[1]=0;

// монтируем оба образа
strcpy(Aname,"DISKA.KDI");	// сохраняем имя диска А на будущее
mount_disk(0,Aname,1);
mount_disk(1,"DISKB.KDI",1);

// Далее идет бесконечный цикл ожидания перезагрузки корвета. Вся полезная работа будет идти в прерывании int1 
GICR|=_BV(INT1);         // Разрешаем Int1 от -OBF ВВ55
while (boot_flag == 1);
goto reboot;		 // сработало int0 - уходим в перезагрузку
/*
*/
for(;;) {}
}


