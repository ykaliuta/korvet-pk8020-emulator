BASE:	EQU	9000H		; База для размещения этого кода
PORTA:	EQU     0FB08H		; Порт A ВВ55 #3
PORTC:	EQU     0FB0AH		; Порт C ВВ55 #3
TRAM:	EQU	0FC00H		; Начало видеопамяти
SYSREG: EQU	0FA7FH		; регистр карты памяти
PCHDRV:	EQU	4Ch		; подпрограмма печати байта через драйвер консоли ОПТС

INT_PROC: EQU 	0xf7f0+1


	ORG   	BASE

; Стандартный заголовок внешнего ПЗУ корвета
	JP	START	        ; +0
	DB      0		; +3
	DW	START           ; +4 - адрес запуска 
	DB	BASE>>8         ; +6 - старшие 8 бит адреса загрузки (младшие всегда 00)
	DB	(FTOP-BASE)>>8  ; +7 - количество загружаемых 256-байтовых блоков

hello:
	db 	0x1f,'rom5 - RX/TX raw Speed Test for EXTPORT api',0x0d,0x0a
	db 	'bytes in one int | timer tick required to process 32k',0x0d,0x0a
	db 	0
crlf:
	db 	0x0d,0x0a,0
spcdiv:
	db 	'|'
spc:
	db 	0x20,0

; Далее идет непосредственный программный код	
START:
	DI
	LD	SP,0F700h	; рабочий стек, размещаем в области F-макросов драйвера клавиатуры

	ld 	hl,hello
	call 	putmsg

	call 	hw_init

s_lp:
	ld 	a,'R'
	call 	PUTCH
	ld 	a,':'
	call 	PUTCH

	call 	read_test

;  	ld 	hl,spcdiv
;  	call 	putmsg
	ld 	a,0x80+1+4+16
	call 	PUTCH

	ld 	a,'W'
	call 	PUTCH
	ld 	a,':'
	call 	PUTCH

	call 	write_test

	ld 	hl,crlf
	call 	putmsg

	jp 	s_lp


read_test:
;
 	call 	test_rx_byes_in_one_frame

 	ld 	hl,spcdiv
 	call 	putmsg

 	call 	test_timer_tick_when_rx_8000_bytes


	ret


write_test:
;
 	call 	test_tx_byes_in_one_frame

 	ld 	hl,spcdiv
 	call 	putmsg

 	call 	test_timer_tick_when_tx_8000_bytes

;  	ld 	hl,spcdiv
;  	call 	putmsg

	ret


mOneTick:
	db 	'bytes in one frame',0x0d,0x0a,0

test_rx_byes_in_one_frame:

	ld 	hl,testint_store_counter
	ld 	(INT_PROC),hl

	ld 	a,0xF0 	;test, send 0x8000h to korvet
	ld 	hl,test_rx_byte
	call 	do_test_bytes_in_one_frame

	ld 	a,0xF0 	;test, send 0x8000h to korvet
	ld 	hl,test_rx_byte2
	call 	do_test_bytes_in_one_frame

	ld 	a,0xF0 	;test, send 0x8000h to korvet
	ld 	hl,test_rx_fastbyte_8000
	call 	do_test_bytes_in_one_frame

	ret

test_tx_byes_in_one_frame:

	ld 	hl,testint_store_counter
	ld 	(INT_PROC),hl

	ld 	a,0xF1 	;test, send 0x8000h to korvet
	ld 	hl,test_tx_byte
	call 	do_test_bytes_in_one_frame

	ld 	a,0xF1 	;test, send 0x8000h to korvet
	ld 	hl,test_tx_byte2
	call 	do_test_bytes_in_one_frame

	ld 	a,0xF1 	;test, send 0x8000h to korvet
	ld 	hl,test_tx_fastbyte_8000
	call 	do_test_bytes_in_one_frame

	ret




do_test_bytes_in_one_frame:
	ld 	(toneframe_call_code+1),hl
; 	ld 	a,0xF0 	;test, send 0x8000h to korvet
	call 	SendSMD_A

	ld 	de,0

	ei
	halt

	ei
	halt

	ei

toneframe_call_code:
	call 	0

	ld 	hl,(counter)
	ld 	de,0x8000
	ld 	a,e
	sub 	l
	ld 	l,a
	ld 	a,d
	sbc 	a,h
	ld 	h,a
	ld 	(counter),hl

	ld 	hl,counter
	call 	HEXW

	ld 	hl,spc
	call 	putmsg
	ret

	
test_timer_tick_when_rx_8000_bytes:

	ld 	hl,testint_inc_time_counter
	ld 	(INT_PROC),hl

	ld 	a,0xF0 	;test, send 0x8000h to korvet
	ld 	hl,test_rx_byte
	call 	do_test_8000

	ld 	a,0xF0 	;test, send 0x8000h to korvet
	ld 	hl,test_rx_byte2
	call 	do_test_8000

	ld 	a,0xF0 	;test, send 0x8000h to korvet
	ld 	hl,test_rx_fastbyte_8000
	call 	do_test_8000

	ret

test_timer_tick_when_tx_8000_bytes:

	ld 	hl,testint_inc_time_counter
	ld 	(INT_PROC),hl

	ld 	a,0xF1 	;test, send 0x8000h to korvet
	ld 	hl,test_tx_byte
	call 	do_test_8000

	ld 	a,0xF1 	;test, send 0x8000h to korvet
	ld 	hl,test_tx_byte2
	call 	do_test_8000

	ld 	a,0xF1 	;test, send 0x8000h to korvet
	ld 	hl,test_tx_fastbyte_8000
	call 	do_test_8000

	ret



mRX8000:
	db 	'full frames when rx 0x8000',0x0d,0x0a,0

;a - F0 - rx/F1-tx
;hl - call proc
do_test_8000:
	ld 	(t8000_call_code+1),hl
; 	ld 	a,0xF0 	;test, send 0x8000h to korvet
	call 	SendSMD_A

	ei
	halt

	ei
	halt

	xor 	a
	ld 	(int_counter),a

	ei
t8000_call_code:
	call 	0
	di

	ld 	a,(int_counter)
	call	HEX2

	ld 	hl,spc
	call 	putmsg
	ret


int_counter:
	db 	0

counter:
	dw 	0

int_flag:
	db 	0

testint_store_counter:
	push 	af

	ex 	de,hl
	ld 	(counter),hl
	ex 	de,hl

	ld	a,0x20
	ld 	(0xfb28),a

	pop 	af
	ret	

testint_inc_time_counter:
	push 	af

	ld	a,0x20
	ld 	(0xfb28),a

	ld 	a,(int_counter)
	inc 	a
	ld 	(int_counter),a

	pop 	af
	ei
	ret	



;------------------------
;RX test routines


test_rx_byte:

	ld 	hl,0
	ld 	de,0x8000
rx_byte_lp:
	call 	GETBYTE
	ld 	(hl),a
	inc 	hl
	dec 	e
	jp 	nz,rx_byte_lp
	dec 	d
	jp 	nz,rx_byte_lp

	ret

test_rx_byte2:

	ld 	hl,0
	ld 	de,0x8000
rx_byte2_lp:
	call 	GETBYTE2
	ld 	(hl),a
	inc 	hl
	dec 	e
	jp 	nz,rx_byte2_lp
	dec 	d
	jp 	nz,rx_byte2_lp

	ret


test_rx_fastbyte_8000:

	ld 	de,0x8000
	ld 	bc,0
	LD	HL,PORTC

test_rx_fast:
tst_rx_wait_lp:
	LD	A,(HL)		; 7 слово состояния ВВ55 - берется из порта С
	AND	0x20		; 4 выделяем сигнал IBF
	JP	Z,tst_rx_wait_lp		;10 IBF=0 - данных еще нет
	ld 	a,(PORTA)	;13 

	ld 	(bc),a
	inc 	bc

	dec 	e
	jp 	nz,test_rx_fast
	dec 	d
	jp 	nz,test_rx_fast

	ret

;------------------------
;TX test routines


test_tx_byte:

	ld 	hl,0
	ld 	de,0x8000
tx_byte_lp:
	call 	PUTBYTE
	ld 	(hl),a
	inc 	hl
	dec 	e
	jp 	nz,tx_byte_lp
	dec 	d
	jp 	nz,tx_byte_lp

	ret

test_tx_byte2:

	ld 	hl,0
	ld 	de,0x8000
tx_byte2_lp:
	call 	PUTBYTE2
	ld 	(hl),a
	inc 	hl
	dec 	e
	jp 	nz,tx_byte2_lp
	dec 	d
	jp 	nz,tx_byte2_lp

	ret


test_tx_fastbyte_8000:

	ld 	de,0x8000
	ld 	bc,0
	LD	HL,PORTC

test_tx_fast:
tst_tx_wait_lp:
	LD	A,(HL)		; 7 слово состояния ВВ55 - берется из порта С
	AND	0x80		; 4 выделяем сигнал IBF
	JP	Z,tst_tx_wait_lp		;10 IBF=0 - данных еще нет
	ld 	a,(bc)
	inc 	bc
	ld 	(PORTA),a	;13 

	dec 	e
	jp 	nz,test_tx_fast
	dec 	d
	jp 	nz,test_tx_fast

	ret

;------------------------


;STDLIB

;****************************************
;*  Прием байта из порта А по стробу    *
;****************************************
GETBYTE:
	PUSH	HL
	LD	HL,PORTC
WG_GB1:
	LD	A,(HL)		; слово состояния ВВ55 - берется из порта С
	AND	20h		; выделяем сигнал IBF
	JP	Z,WG_GB1		; IBF=0 - данных еще нет
	DEC	L
	DEC	L
	LD	A,(HL)		; данные поступили - выбираем их из порта А

	POP	HL
	RET


GETBYTE2:
	PUSH	HL
	LD	HL,PORTC
WG_GB2:
	LD	A,(HL)		; слово состояния ВВ55 - берется из порта С
	AND	20h		; выделяем сигнал IBF
	JP	Z,WG_GB2		; IBF=0 - данных еще нет
	ld 	a,(PORTA)	;13 

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

PUTBYTE2:
	PUSH	HL
	PUSH	BC
	LD 	C,A
	ld 	B,0x80
	LD	HL,PORTC
WP2:
	LD	A,(HL)		; слово состояния ВВ55 - берется из порта С
	AND	B		; выделяем сигнал -OBF
	JP	Z,WP2		; -OBF=0 - в передатчике сидит незабранный байт
	LD 	A,C
	LD	(PORTA),A		; отправляем данные в порт данных
	pop 	BC
	POP	HL
	RET

;****************************************
;*      Печать полубайта в HEX          *
;****************************************

HEX1:
	AND	0Fh		; младший полубайт 
; 	ADD	a,'0'		; в ASCII
; 	CP	3Ah		; 0-9?
; 	JP	C,H1D		; да
; 	ADD	a,7		; коррекция до A-F
	add 	a,90h
	daa
	adc 	a,40h
	daa
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
	
		
;****************************************
;*     Отправка командного пакета       *
;****************************************
SendSMD_A:
	ld 	(CMD),a
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
	ADD 	A,B		; добавляем к КС
	LD	B,A
	LD	A,(HL)		; очередной байт пакета 
	CALL	PUTBYTE		; - в порт
	INC	HL
	DEC	C
	JP	NZ,SCL
	LD	A,B
	DEC 	A		; КС-1
	CALL	PUTBYTE     ; контрольная сумма
	CALL	GETBYTE	; ответ контроллера
	POP	BC
	POP	HL
	RET

;*******************************************
;*      Прием сектора                      *
;* HL - адрес размещения сектора в памяти  *
;*  Размер сектора - 256 байт              *
;*******************************************
GETSEC:
	PUSH	BC
	LD	C,128		; счетчик байтов сектора, всего 128 байт
GSL:
	CALL	GETBYTE
	LD	(HL),A		; принимаем и размещаем очередной байт
	INC	HL
	DEC	C		; принимаем остальные байты
	JP	NZ,GSL
	POP	BC
	RET

	
;****************************************
;  Печать строки на текстовый экран     *
;****************************************

putmsg_x:
	pop 	hl
	call 	putmsg
	push 	hl
	ret

putmsg:
	LD	A,(HL)
	INC	HL
	OR	A		; 0 - конец строки
	RET	Z
	CALL	PUTCH		; выводим байт через вызов ОПТС
	JP	putmsg
	
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

CMD:	ds 	4

hw_init:
	ld 	a,0xf6
	ld 	(0xfb28),a
	ld 	a,0xf7
	ld 	(0xfb29),a
	ld 	a,0xef
	ld 	(0xfb29),a

	ld 	a,0xc3
	ld 	(0xf7f0),a
	ret


; Округляем размер всей секции до ближайших 256 байт вверх	
        DS      ((($>>8)+1)<<8)-$
FTOP:	EQU	$			; верхний адрес памяти, занимаемый загружаемым блоком
;--------------------------------------------------------------------------
; Конец загружаемой области
; Далее идет область рабочих данных, не входящая в тело файла загрузчика
;--------------------------------------------------------------------------

; Буфер сектора
SECBUF:	EQU	$
	ORG	$+128	; буфер 128 байт, DS использовать нельзя, чтобы не порождать объектный код
	
	END
    