;**********************************************************************************************
;*  Загрузчик 2 фазы дискового эмулятора Korvet-Extrom
;*  Работает только в паре с контроллером Extrom через боковой разъем корвета
;*
;*  Загружает в память системные дорожки диска А и передает управление на код инициализации BIOS
;*   Сборка с помощью кросс-ассемблера z80asm, контрольная сумма рассчитывается утилитой ercs
;*
;*   Программа записывается на карту SD контроллера в файл LOADER.BIN
;**********************************************************************************************
BASE	EQU	2000H		; База для размещения этого кода
PORTA	EQU     0FB08H		; Порт A ВВ55 #3
PORTC	EQU     0FB0AH		; Порт C ВВ55 #3
TRAM	EQU	0FC00H		; Начало видеопамяти

SYSREG14  EQU	0FA7FH		; регистр карты памяти
SYSREG5C  EQU	0FF7FH		; регистр карты памяти
; SYSREG6C  EQU	0BF7FH
SYSREG3C  EQU	0FF7FH

PCHDRV	EQU	4Ch		; подпрограмма печати байта через драйвер консоли ОПТС
CONIN	EQU	49h		; подпрограмма ввода символа через драйвер консоли ОПТС


; started in config 14
; 	14  NDOS   0-1FFF  2000-F7FF                   F800  FA00  FC00  FB00

	ORG   	BASE

; Стандартный заголовок внешнего ПЗУ корвета
	JP	START	        ; +0
	DB      0		; +3
	DW	START           ; +4 - адрес запуска 
	DB	BASE>>8         ; +6 - старшие 8 бит адреса загрузки (младшие всегда 00)
	DB	(FTOP-BASE)>>8  ; +7 - количество загружаемых 256-байтовых блоков


; Далее идет непосредственный программный код	
START:
	DI
	LD	SP,0F700h	; рабочий стек, размещаем в области F-макросов драйвера клавиатуры

	call 	initLUT

; Печать промпта 	
	LD	HL,HMSG
	CALL	PSTR

	call 	hw_test
; 	halt


	xor 	a
	ld 	(flag_microdos),a

	ld 	a,(0xf880)
	and 	0x21 		;ctrl+shift
	cp 	0x21

	jp 	z,force_default_bios
	xor 	a		;turn off system substitution
	jp 	restart_disable

force_default_bios:
	ld 	hl,msgFORCEDDEFAULTBIOS
	call 	PSTR
; 	jp 	set_subst

force_subst:
; 	ld 	hl,msgFORCEDDEFAULTBIOS
; 	call 	PSTR

	ld 	hl,msgSUBST
	call 	PSTR

	ld 	a,(flag_microdos)

	ld 	hl,msgMICRODOS
	cp 	1
	jp 	z,.print
	ld 	hl,msgCPM
.print:
	call 	PSTR

	ld 	hl,msgASK_SUBST
	call 	PSTR

WaitKey:
	ld 	a,7
	call 	PUTCH
	call 	GETCH
	and 	0x5F

	cp 	0x0d 	;CR
	jp 	nz,.printKEY

	ld 	a,'Y'

.printKEY:
	ld 	(tmpKEY),a

	cp 	'Y'
	jp 	z,set_subst
	cp 	'D'
	jp 	z,set_subst

	cp 	a,'C'
	jp 	nz,.chkM

	ld 	a,0
	ld 	(flag_microdos),a
	jp 	set_subst
.chkM:
	cp 	'M'
	jp 	nz,WaitKey
	ld 	a,1	
	ld 	(flag_microdos),a


set_subst:
	ld 	a,(flag_microdos)
	inc 	a

restart_disable:
	ld 	(TRK),A
	ld 	a,0xA0 		;system substitution
	ld 	(CMD),a
	xor 	a
	ld 	(DRV),a
	call 	SENDCMD

	ld 	a,(tmpKEY)
	or 	a
	jp 	z,.noPrintKey
	call 	PUTCH
	ld 	hl,HCRLF
	call	PSTR 	

.noPrintKey:


	ld 	a,0
	ld 	(flag_unsupported),a
	ld 	(flag_microdos),a
	ld 	hl,0
	ld 	(msg_warning),hl

	DI
	LD	SP,0F700h	; рабочий стек, размещаем в области F-макросов драйвера клавиатуры


; Загрузка инфосектора

	ld 	a,0x1 		;read
	ld 	(CMD),a
	xor 	a
	ld 	(TRK),A
	ld 	(SEC),A


	CALL	SENDCMD		; отсылаем команду загрузки сектора, параметры уже прописаны в командном блоке
	LD	HL,SECBUF
	CALL	GETSEC		; забираем 256 байт сектора 0

; Разбор инфосектора
;
; Вынимаем идущие подряд переменные LOADR, RUNADR, COUNT
;

;TODO: add crc check!!!!!!!!!!!!!!!

	LD	HL,SECBUF
	LD	DE,LOADR	; начало блока переменных в памяти
	LD	C,5		; всего 5 байтов
IL:
	LD	A,(HL)		; копируем блок переменных
	LD	(DE),A
	INC	HL
	INC	DE
	DEC	C
	JP	NZ,IL

; Получаем индекс размера сектора
	LD	HL,SECBUF+10	;  +10 - коэффициент блокирования секторов
	LD	A,(HL)
	LD	(DE),A
	INC	DE
	LD	HL,SECBUF+16	;  +16 - LSPT
	LD	A,(HL)
	LD	(DE),A

	call 	CheckBootParams
	jp 	nz,force_subst
	
	ld 	a,(0xf880)
	and 	0x1
	ld 	(mute_flag),a
	jp 	z,skip_msg_1

	call 	ShowBootParams

skip_msg_1:
		
; Вычисление числа полных дорожек для загрузки	
	
	LD	HL,SCOUNT	
	LD	E,(HL)		; число загружаемых физических секторов
	LD	D,0		; -> DE
	INC	HL
	LD	C,(HL)		; индекс размера сектора
;
; Индекс размера - очень удобная величина, определяющая коэффициент блокирования секторов, то есть число логических секторов в физическом
; Размер логического сектора CP/M - всегда 128 байт.
;
;  Физ.сектор   SSIZE    коэффициент блокирования
;----------------------------------------------------
;     128         0                1
;     256         1                2
;     512         2                4
;     1024        3                8 
;
;
; Далее выполняется операция DE << C, что приводит к умножению числа физических секторов
; на коэффициент блокирования - получаем число загружаемых логических секторов
	ex 	de,hl
	INC	C		; Коррекция SSIZE, чтобы начинался с 1
SSL1:	
	DEC	C		; счетчик сдвига--
	JP	Z,SSL2		; сдвиг окончен
; 	XOR	A		; CF=0
; 	LD	A,E
; 	RLA			; сдвигаем младший байт
; 	LD	E,A
; 	LD	A,D
; 	RLA			; сдвигаем старший байт
; 	LD	D,A
	add 	hl,hl
	JP	SSL1
;
;  Выводим на экран число логических секторов
;
SSL2:
	ex 	de,hl
	ld 	a,(mute_flag)
	or 	a
	jp 	z,skip_msg_2

	LD	HL,HMLS
	CALL	PSTR
	LD	A,D
	CALL	HEX2
	LD	A,E
	CALL	HEX2

	LD	HL,HCRLF
	CALL	PSTR

	;de - sectors to load

skip_msg_2:

	;DE - sectors to load

	ld 	c,e 		; C - sectors to load (up to 32k)
	LD	HL,(LOADR)	; HL - указатель позиции загрузки в памяти

	xor 	a
	ld 	(TRK),a

	inc 	c

ld_loop_sec0:
	xor 	a
	ld 	(SEC),a

ld_loop:

	dec 	c
	ld 	a,c
	or 	a
	jp 	z,load_done

	ld 	a,(mute_flag)
	or 	a
	jp 	z,skip_msg_star

	LD	A,'*'		; прогресс-бар загружаемых секторов
	CALL	PUTCH
skip_msg_star:
	

	CALL	SENDCMD		; отправляем команду
	CALL	GETSEC		; получаем и размещаем сектор

	PUSH	HL
	LD	HL,SEC
	INC	(HL)		; номер сектора ++

	ld 	a,(LSPT)
	cp 	(hl)
	POP	HL

	jp 	nz,ld_loop

	PUSH	HL
	LD	HL,TRK
	INC	(HL)		; номер сектора ++
	POP	HL
	jp 	ld_loop_sec0

load_done:
; 	halt

; Вся системная область диска загружена - можно запускать ОС
;	
; 	LD	A,'@'		; Признак окончания загрузки - на экран
; 	CALL	PUTCH

	call 	cpm_bios_patcher

; 	LD	HL,SYSREG14	
; 	LD	(HL),1Ch	; Включаем карту памяти 1С, для CP/M
	
	;start with sysreg=14

	LD	HL,(RUNADR)
	JP	(HL)		; уходим по адресу запуска
	
CheckBootParams:

	LD	HL,LOADR
	call 	FetchDEfromHL

	ld 	a,d
	cp 	0x22 		; load addr shiould be >0x2200
	jp 	c,.BadBootParam

	LD	HL,RUNADR
	call 	FetchDEfromHL
	ld 	a,d
	cp 	0x22 		; run addr shiould be >0x2200
	jp 	c,.BadBootParam

	LD	HL,SCOUNT
	LD	A,(HL)
	or 	a
	jp 	z,.BadBootParam
	cp 	48 		;sectors to load < 48
	jp 	nc,.BadBootParam

	xor 	a
	ret

.BadBootParam:
	call 	ShowBootParams
	ld 	hl,msgBadBootParam
	CALL	PSTR
	xor 	a
	inc 	a
	ret

msgBadBootParam:
	db 	0x0d,0x0a
	db 	'Incorrect boot parameters detected, fallback to default'
	db 	0x0d,0x0a,0

ShowBootParams:
	; Вывод параметров загрузки на экран
	LD	HL,HMBASE
	CALL	PSTR
	LD	HL,LOADR
	CALL	HEXW
	LD	HL,HMRUN
	CALL	PSTR
	LD	HL,RUNADR
	CALL	HEXW
	LD	HL,HMPS
	CALL	PSTR
	LD	HL,SCOUNT
	LD	A,(HL)
	CALL	HEX2
	LD	HL,HMSS
	CALL	PSTR
	LD	HL,SSIZE
	LD	A,(HL)
	CALL	HEX2
	LD	HL,HMSP
	CALL	PSTR
	LD	HL,LSPT
	LD	A,(HL)
	CALL	HEX2
	ret

initLUT:
	ld      hl, 0xFAFB
	ld      a, 0F8h 

.iniLUT:                  
	ld      (hl), a         ; F8,F9,FA,FB,FC,FD,FE,FF
	inc     a
	jp      nz, .iniLUT      ; F8,F9,FA,FB,FC,FD,FE,FF
	
	ld      bc, 1108h       ; A=0
	
.iniLUTGRP:                  
	ld      (hl), a         ; 00,11,22,33,44,55,66,77
	add     a, b
	dec     c
	jp      nz, .iniLUTGRP   ; 00,11,22,33,44,55,66,77

	ret


;****************************************
;*  Прием байта из порта А по стробу    *
;****************************************
GETBYTE:
	PUSH	HL
	LD	HL,PORTC
WG:
	LD	A,(HL)		; слово состояния ВВ55 - берется из порта С
	AND	20h		; выделяем сигнал IBF
	JP	Z,WG		; IBF=0 - данных еще нет
	DEC	L
	DEC	L
	LD	A,(HL)		; данные поступили - выбираем их из порта А
	POP	HL
	RET

;****************************************
;*  Отправка байта в порт A             *
;****************************************
PUTBYTE:
	PUSH	HL
	PUSH	AF
	LD	HL,PORTC
WP:
	LD	A,(HL)		; слово состояния ВВ55 - берется из порта С
	AND	80h		; выделяем сигнал -OBF
	JP	Z,WP		; -OBF=0 - в передатчике сидит незабранный байт
	DEC	L
	DEC	L
	POP	AF
	LD	(HL),A		; отправляем данные в порт данных
	POP	HL
	RET

;****************************************
;*      Печать полубайта в HEX          *
;****************************************

HEX1:
	AND	0Fh		; младший полубайт 
	ADD	'0'		; в ASCII
	CP	3Ah		; 0-9?
	JP	C,H1D		; да
	ADD	7		; коррекция до A-F
H1D:	CALL	PUTCH		; Печать байта
	RET

;****************************************
;*      Печать байта в HEX              *
;****************************************

HEX2:	PUSH	AF
	RRCA
	RRCA			; сдвигаем старший полубайт в младший
	RRCA
	RRCA
	CALL	HEX1		; печатаем его
	POP	AF
	AND	0Fh		
	CALL	HEX1		; печатаем младший полубайт
	RET

HEX4:
	push 	AF
	ld 	a,h
	call 	HEX2
	ld 	a,l
	call 	HEX2
	pop 	AF
	ret
;****************************************
;*  Печать слова (2 байта) из (HL)      *
;****************************************
HEXW:
	INC	HL
	LD	A,(HL)		; старший байт
	CALL	HEX2
	DEC	HL		; младший байт
	LD	A,(HL)
	CALL	HEX2
	RET

FetchDEfromHL:
	LD	E,(HL)
	INC	HL
	LD	D,(HL)		; старший байт
	DEC	HL		; младший байт
	RET
	
		
;****************************************
;*     Отправка командного пакета       *
;****************************************
SENDCMD:
	PUSH	HL
	PUSH	BC
	LD	HL,CMD
	LD	C,4		; пакет - 4 байта
	LD	B,0		; заготовка контрольной суммы
SCL:
	LD	A,(HL)		; очередной байт пакета 
	ADD 	B		; добавляем к КС
	LD	B,A
	LD	A,(HL)		; очередной байт пакета 
	CALL	PUTBYTE		; - в порт
	INC	HL
	DEC	C
	JP	NZ,SCL
	LD	A,B
	DEC 	A		; КС-1
	CALL	PUTBYTE     ; контрольная сумма
	CALL	GETBYTE	    ; ответ контроллера
	POP	BC
	POP	HL
	RET
	
	
;*******************************************
;*      Прием сектора                      *
;* HL - адрес размещения сектора в памяти  *
;*  Размер сектора - 128 байт              *
;*******************************************
GETSEC:
	PUSH	BC
	PUSH	DE
	LD	C,128		; счетчик байтов сектора, всего 128 байт
	EX	DE,HL		; адрес буфера -> DE
	LD	HL,PORTC
GSL:
	LD	A,(HL)		; слово состояния ВВ55 - берется из порта С
	AND	20h		; выделяем сигнал IBF
	JP	Z,GSL		; IBF=0 - данных еще нет
	LD	A,(PORTA)		; данные поступили - выбираем их из порта А
	LD	(DE),A		; принимаем и размещаем очередной байт
	INC	DE		; указатель буфера ++
	DEC	C		; принимаем остальные байты
	JP	NZ,GSL
	EX	DE,HL		; адрес буфера -> HL
	POP	DE
	POP	BC
	RET
	
	
	
;****************************************
;  Печать строки на текстовый экран     *
;****************************************
PSTR:
	LD	A,(HL)
	INC	HL
	OR	A		; 0 - конец строки
	RET	Z
	CALL	PUTCH		; выводим байт через вызов ОПТС
	JP	PSTR
	
;****************************************
;   Вывод байта на экран через ОПТС
;****************************************
PUTCH:
	PUSH	HL
	PUSH	BC
	PUSH	DE
	LD	C,A
	CALL	PCHDRV
	POP	DE
	POP	BC
	POP	HL
	RET

;A - chr
GETCH:
	EI
	PUSH	HL
	PUSH	BC
	PUSH	DE
	CALL	CONIN
	POP	DE
	POP	BC
	POP	HL
	DI
	RET


; Командный пакет	
CMD:	DB	1		; Команда чтения
DRV:	DB	0
TRK:	DB	0
SEC:	DB	0

; Параметры загрузки из инфосектора
LOADR:	DS	2	; адрес загрузки
RUNADR:	DS	2	; адрес запуска
SCOUNT:	DS	1	; число загружаемых физических секторов
SSIZE:	DS	1	; коэффициент размера физического сектора
LSPT:	DS	1	; число логических секторов на дорожку

; Текстовые строки для вывода на экран
HMSG:   DB	1Fh ; очистка экрана
	DB	"--EXTROM Stage2 boot version 1.0 by Forh32 & ESL 2014--",0dh,0ah,0	; текстовое сообщение на экран
HMBASE: DB	"BASE:",0
HMRUN:  DB	" START:",0
HMPS:   DB	" PSECTORS:",0
HMSS:   DB	" SSIZE:",0
HMSP:   DB	" LSPT:",0
HMLS:   DB	" LSECTORS:",0
; HMTR:   DB	" TRACKS:",0
HCRLF:	DB	0dh,0ah,0
mute_flag:
	db 	0
tmpKEY:
	db 	0
msgCPM:
 	db 	'CP/M',0
msgMICRODOS:
	db 	'MICRODOS',0
msgSUBST:
 	db 	'Load default ',0 
msgASK_SUBST:
	db 	' ?(Enter/Y-yes, C-CP/M, M-microdos) ?',0	

msgFORCEDDEFAULTBIOS:
	db 	0dh,0ah,'!! Ctrl+Shift - pressed, forced to use BIOS SUBSTITUTION !!',0dh,0ah,0dh,0ah,0

	include 	"hw_test.asm"
	include 	"generator/V0/extrom-patcher.asm"


; Округляем размер всей секции до ближайших 256 байт вверх	
        DS      ((($>>8)+1)<<8)-$,0
FTOP	EQU	$			; верхний адрес памяти, занимаемый загружаемым блоком
;--------------------------------------------------------------------------
; Конец загружаемой области
; Далее идет область рабочих данных, не входящая в тело файла загрузчика
;--------------------------------------------------------------------------

; Буфер сектора
SECBUF	EQU	$
	ORG	$+128	; буфер 128 байт, DS использовать нельзя, чтобы не порождать объектный код

loader	END 	
    