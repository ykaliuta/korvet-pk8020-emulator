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
// Korvet ETALON emulator project
// (c) 2000-2003 Sergey Erokhin aka ESL
// (c) 2003      Korvet Team.

#include <stdio.h>
#include <conio.h>
#include <allegro.h>
#include "korvet.h"

/*
; АДАПТЕР ПЕЧАТАЮЩЕГО УСТРОЙСТВА

; В состав этого узла входят регистр данных, регистр
; управления и регистр состояния, реализованные в виде
; портов параллельного адаптера КР580ВВ55А, причем
; регистр управления обеспечивает непосредственное
; управление битами (сброс и установку). Передача байта
; данных осуществляется при наличии признака готовности
; принтера в регистре состояния путем записи кода
; символа в регистр данных с последующей установкой и
; сбросом бита стробирующего сигнала в регистре управления.
; В драйверы печатающего устройства в некоторых случаях
; необходимо вводить инвертирование передаваемого
; байта и байта состояния (условное ассемблирование
; в зависимости от значения параметра LSTINV).

LSTDAT EQU 30H  ; Регистр данных принтера
LSTSTS EQU 38H  ; Регистр состояния
LSTCTL EQU 33H  ; Регистр управления
LSTINV SET   1  ; Признак инверсии
                ; данных и состояния
LSTRDY EQU   4  ; Маска признака готовности
                ; принтера
STBSET EQU  0BH ; Установка строба данных
STBRST EQU  0AH ; Сброс строба данных

LSTINT EQU 6    ; уровень прерывания по готовности
                ; принтера

        Адаптер  печатающего устройства (ПУ) осуществляет вывод
данных на печать и  реализует  необходимое  подмножество  цепей
параллельного    интерфейса   типа   ИРПР-М(CENTRONICS).    Это
подмножество включает восемь однонаправленных  цепей  данных  с
высоким   активным   уровнем   напряжения   и   цепи   сигналов
стробирования "SE", выбора ПУ "ACK" и сигнала "BUSYP".
        Адаптер  содержит регистр данных, реализованный на базе
порта  "A"  микросхемы   D16   (13.52),   регистр   управления,
реализованный на базе порта "C" микросхемы D16 (разряды 4 и 5),
регистр  состояния,  реализованный на базе порта "A" микросхемы
D17 (13.35) (разряд 2).  Микропроцессор  осуществляет  передачу
данных  в ПУ, предварительно анализируя состояние линии "BUSYP"
через   регистр    состояния,    адресуя    его    по    линиям
"CSIOP1","А0","А1".  При наличии признака готовности ПУ (низкий
уровень  напряжения на линии "BUSYP") микропроцессор записывает
байт  данных  в  регистр  данных,   адресуя   его   по   линиям
"CSIOP2","А0","А1",     устанавливает    и    сбрасывает    бит
стробирующего сигнала.  Предусмотрена возможность  обслуживания
ПУ  по  прерыванию,  для  чего  сигнал  "BUSYP" по линии "IRQ6"
передается в контроллер прерывания.  Микропроцессор  программно
решает, когда обслужить этот запрос.


PPI2
..30    -       LSTDATA
..31    -       empty
..32    -       (CASOUT, LSTCTL, SOUNDC)
..33    -       RUS

PPI1
..38    -       (LANADDR, VISTS, LSTSTS, VBLANK, CASIN)
*/

extern int PPI2_A;
char FileNamePrinter[1024]="printer.txt";
FILE *PrinterFILE;

#define MAXPRINTBUF 1024
unsigned char PrinterBuffer[MAXPRINTBUF];
int PrintBufCntr=0;

//int LSTSTS; // Признак готовности принтера PPI1, port A, Bit 00000x00
//int LSTCTL; // Строб записи

int PrinterStatus=0;
int StatusDelay=0;

void FlushPrinterBuf(void) {
    static int iter=0;
    textprintf(screen,font,20,20,255,"%d",iter++);
    if (PrintBufCntr) fwrite(PrinterBuffer,1,PrintBufCntr,PrinterFILE);
    PrintBufCntr=0;
}

void AddToPrinter(unsigned char C) {
    if (PrintBufCntr == MAXPRINTBUF) FlushPrinterBuf();
    PrinterBuffer[PrintBufCntr++]=C;
//    textprintf(screen,font,20,40,255,"%c ",C);
}

int InitPrinter(void) {
     PrinterFILE=fopen(FileNamePrinter,"ab");
     PrintBufCntr=0;
     return 1;
}

void DestroyPrinter(void) {
    FlushPrinterBuf();
    fclose(PrinterFILE);
}

int GetPrinterStatus(void) {
  int ret;

  // =1 - ready
  if (StatusDelay) {StatusDelay--;ret=0;}
  else {ret=1;}
  return ret;
}

int SetPrinterStrobe(int Value) {
  static int PrevStrobe=0;

  if ( Value && (PrevStrobe == 0) ) {
    AddToPrinter(PPI2_A^0xff);
    StatusDelay=5; // Delay 5 port read
  }
  PrevStrobe=Value;
}

