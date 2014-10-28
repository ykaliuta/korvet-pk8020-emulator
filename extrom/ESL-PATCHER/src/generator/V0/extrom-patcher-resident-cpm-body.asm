rPORTA	EQU	0FB08H		; Порт Канала A интерфейса расширения
rPORTC	EQU	0FB0AH		; Порт Канала С интерфейса расширения
rPCWR	EQU	0FB0BH		; Порт управляющих команд

_DSK 	EQU 	0xFFFF
_OPER 	EQU 	0xFFFF
_TRK 	EQU 	0xFFFF
_SEC 	EQU 	0xFFFF
_DMA 	EQU 	0xFFFF

_RRBUFF	EQU 	0xEE00

cpm_resident_body 	macro	preffix

;keyhook support removed 2014-10-22
;больше нет нужды в ней, т.к. есть диск F
; preffix&kbd_hook_noCtrl_03;
; 	ld 	(preffix&save_a),a
; 	cp 	0x03
; 	jp 	preffix&kbd_hook_noCtrl_chk

; preffix&kbd_hook_noCtrl_33:
; 	ld 	(preffix&save_a),a
; 	cp 	0x33

; preffix&kbd_hook_noCtrl_chk:
; 	jp 	nz,preffix&bios_hook

; 	ld 	a,(0xf880)
; 	and 	00100001b
; 	cp 	00100001b

; 	jp 	nz,preffix&bios_hook

; 	jp 	preffix&do_key_action

; preffix&kbd_hook_2133:
; 	ld 	(preffix&save_a),a
; 	ld 	a,c
; 	cp 	0x33 	;stop
; 	jp 	nz,preffix&bios_hook

; 	ld 	a,b
; 	cp 	0x21 	;ctrl+stop
; 	jp 	nz,preffix&bios_hook

; preffix&do_key_action:
; 	push 	hl
; 	push 	de
; 	push 	bc
; 	ld 	hl,preffix&msg_hook
; 	ld 	de,0xfc00
; preffix&.kmlp:	
; 	ld 	a,(hl)
; 	or 	a
; 	jp 	z,preffix&.kmlp_ext
; 	ld 	(de),a
; 	inc 	hl
; 	inc 	de
; 	jp 	preffix&.kmlp

; preffix&.kmlp_ext:
; 	pop 	bc
; 	pop 	de
; 	pop 	hl

; 	ld 	bc,0 	;simulate no key
; 	ld 	a,0
; 	jp 	preffix&old_hook
; ; 	halt


; preffix&bios_hook:
; 	ld 	a,(preffix&save_a)

; preffix&old_hook:
; 	jp 	$
; preffix&save_a:
; 	db 	0	
; preffix&msg_hook:
; 	db 	'CTRL+SHIFT+STOP pressed',0

;ptrs to drive drvreg bytes 
preffix&res_DRIVE_TAB:
	dw 	0 	;A
	dw 	0 	;B
	dw 	0 	;C
	dw 	0 	;D
	dw 	0 	;E
	dw 	0 	;F


preffix&dph_F:
	dw	0	;_XLT:           
	dw	0	;_1:             
	dw	0	;_2:             
	dw	0	;_3:             
	dw	0xFC00	;DirBuf: value store here via patcher 0xfc00 - debug
	dw	preffix&dpb_F	;DPB:   
	dw	0	;CSV: - fixed drive   
	dw	preffix&alw_F	;ALV:   


;  ФИЗИЧЕСКИЕ ПАРАМЕТРЫ ДИСКА
;--------------------------------------
      DB 50H	;контрольная сумма

      DB 080H ;константа скорости движения головки.
;	; 3 - 30 мс на шаг, 2 - 20 мс, 1 - 12 мс,
;	; 0 - 6 мс и 80H - 3 мс.
      DB 05H  ; число физических секторов на дорожке
      DB 01H  ; информация о сторонах диска
;	;( 00 - односторонный диск
;	;  01 - двухсторонний диск
      DB 03	; 512 bytes/sector
;	;( 00 - 128 байт/сектор
;	;  01 - 256 байт/сектор
;	;  02 - 512 байт/сектор
;	;  03 - 1024 байт/сектор
      DB 0	; номер текущей дорожки.
      DB 0	; позиционный номер устройства (00000100) - для записи в регистр выбора
;---------------------------------------------------
      DB 0	;INFO	; флаг успешного считывания

preffix&dpb_F:
; kdi params
      DW 40	;128 байтовых секторов/трек ; SPT
      DB 4	; размер блока миним.кбайт  ; BSH
;	; фактор сдвига блока распределения данных
;	; определяется размером блока данных. Блок 
;	; данных - минимально возможная частица файла.
;	; Размер блока - 1, 2, 4, 8 или 16 Кбайт. Если
;	; блок имеет большой размер, в каждом файле 
;	; теряется значительное число неиспользуемых
;	; секторов. При уменьшении размера блока
;	; увеличивается размер директории, описывающий
;	; расположение частей файла. BSH = log2[число
;	; логических секторов в блоке].
      DB 15	; число логических сек/блок  ; BLM
;	; маска блока распределения данных.
;	; BLM = (число логических секторов в блоке)-1.
      DB 0	; extent mask                ; EXM
;	; = (BLM+1)*128/1024 - 1 - [DSM/256]. EXM - 
;	; вспомогательная величина для определения
;	; номера extent'а.
;	; extent - часть файла, описываемая одним 
;	; входом в директорию.
      DW 394	; обьем диска в блоках-1     ; DSM
      DW 127	; входов в директорий-1      ; DRM
      DB 192	; определяет, какие блоки    ; AL0
;	; зарезервированы
      DB 0	; alloc 1                    ; AL1
;	; под директорию. Каждый  бит AL0,AL1, 
;	; начиная со старшего бита AL0 и кончая 
;	; младшим битом AL1, значением 1 резервирует
;	; один блок данных для директории. Нужно
;	; резервировать необходимое число блоков
;	; для хранения входов в директорию: 32*DRM/BLS
      DW 0	; размер вектора контроля директории. ; CKS
;	; CKS=(DRM+1)/4. Если диск не сменяем, CKS=0.
      DW 2	; число системных дорожек    ; OFS


; preffix&alw_F: 	ds 	50
preffix&alw_F: 	ds 	50


preffix&res_seldsk:
	ld 	a,c
	cp 	0xfe
preffix&old_seld_dsk:
	jp 	nz,$	
	ld 	hl,preffix&res_DRIVE_TAB
	ret


;RAM:DA27 C3 EB DC                 jp      j_READ

;RAM:DCEB          j_READ:                                 ; CODE XREF: RAM:_READj
;RAM:DCEB                                                  ; RAM:DB84p
;RAM:DCEB
;RAM:DCEB          ; FUNCTION CHUNK AT RAM:DDC4 SIZE 0000007B BYTES
;RAM:DCEB
;RAM:DCEB 3E 04                    ld      a, 4
;RAM:DCED 32 66 E0                 ld      (_OPERATION), a
;RAM:DCF0 3A 64 E0                 ld      a, (_DSK)
;RAM:DCF3 FE 04                    cp      4
;RAM:DCF5 CA C4 DD                 jp      z, loc_DDC4

;сюда попадает из биоса JP READ
;а если не наше то прыгаем на старый read
preffix&res_READ:
; 	ld	 A,4	;Режим чтения
; 	ld  	(_OPER),a
preffix&r_p_DSK1:
	ld 	a,(_DSK) 	;_DSK
	cp 	4
	jp 	z,preffix&_old_read

	cp 	5 		;disk F mapped to EMU drive E
	jp 	nz,preffix&.r_no_drv_dec
	dec 	a

preffix&.r_no_drv_dec:
	LD	(preffix&EXR_DRV),A	; # устройства

	push 	hl
preffix&r_p_SELDSKDRIVER:
	ld 	hl,(0)	;EXR_SELDSK_DRIVE
	ld 	a,(hl)
	pop 	hl
	or 	a
	jp 	nz,preffix&_old_read

preffix&r_p_TRK1:
	LD	A,(_TRK)	
	LD	(preffix&EXR_TRK),A	; дорожка
preffix&r_p_SEC1:
	LD	A,(_SEC)	; сектор
	DEC	A		; у нас номер сектора начинается с 0
	LD	(preffix&EXR_SEC),A

	LD	A,1
	LD	(preffix&EXR_CMD),A	; 1 - команда чтения

	CALL	preffix&EXR_SENDCMD	; отсылаем команду
	DEC	A		; ответ, 0 - ошибка, 1 - ОК
	RET	NZ		; 0 - ошибка
preffix&r_p_DMA1:
	LD	HL,(_DMA)	; адрес буфера для приема данных
	CALL	preffix&EXR_GETSEC	; принимаем данные
	XOR	A		; завершение всегда успешно
	RET
;

preffix&_old_read:
	jp 	$

preffix&res_WRITE:
; 	ld	 A,6	;Режим чтения
; 	ld  	(_OPER),a

preffix&r_p_DSK2:
	ld 	a,(_DSK) 	;_DSK
	cp 	4
	jp 	z,preffix&_old_write
	cp 	5 		;disk F mapped to EMU drive E
	jp 	nz,preffix&.w_no_drv_dec
	dec 	a
preffix&.w_no_drv_dec:
	LD	(preffix&EXR_DRV),A


	push 	hl
preffix&r_p_SELDSKDRIVEW:
	ld 	hl,(0)	;EXR_SELDSK_DRIVE
	ld 	a,(hl)
	pop 	hl
	or 	a
	jp 	nz,preffix&_old_write

preffix&r_p_TRK2:
	LD	A,(_TRK)
	LD	(preffix&EXR_TRK),A
preffix&r_p_SEC2:
	LD	A,(_SEC)
	DEC	A
	LD	(preffix&EXR_SEC),A

	LD	A,2		; 2 - команда записи
	LD	(preffix&EXR_CMD),A

	CALL	preffix&EXR_SENDCMD
	DEC	A		; ответ, 0 - ошибка, 1 - ОК
	RET	NZ		; 0 - ошибка
preffix&r_p_DMA2:
	LD	HL,(_DMA)
	CALL	preffix&EXR_PUTSEC
	XOR	A		; запись успешна
	RET

preffix&_old_write:
	jp 	$


preffix&res_GETINFO:

	LD 	A,(HL)	; A= маска выбора привода
	or 	a
	jp 	nz,preffix&_old_getinfo 	;=0 if emulated

;
;  Чтение с эмулируемых дисков A или B
;
	push 	hl
	CALL	preffix&EXR_READINFO
	DEC	A		; ответ, 0 - ошибка, 1 - ОК
; 	JP	NZ,0xE6ff	;IERR		; 0 - ошибка
preffix&_old_getinfo_chkdo:
	JP	$ 		;IERR jnz xxx, CHKDO
; ;
preffix&_old_getinfo:
	jp 	$ 		;GETINFO


;==================  Драйвер ExtROM-API ====================================
	
	
;****************************************
;*  Прием байта из порта А по стробу    *
;****************************************
preffix&EXR_GETBYTE:
	PUSH	HL
	LD	HL,rPORTC
preffix&pWG:
	LD	A,(HL)		; слово состояния ВВ55 - берется из порта С
	AND	20h		; выделяем сигнал IBF
	JP	Z,preffix&pWG		; IBF=0 - данных еще нет
	LD	A,(rPORTA)		; данные поступили - выбираем их из порта А
	POP	HL
	RET

;****************************************
;*  Отправка байта в порт A             *
;****************************************
preffix&EXR_PUTBYTE:
	PUSH	HL
	PUSH	AF
	LD	HL,rPORTC
preffix&pWP:
	LD	A,(HL)		; слово состояния ВВ55 - берется из порта С
	AND	80h		; выделяем сигнал -OBF
	JP	Z,preffix&pWP		; -OBF=0 - в передатчике сидит незабранный байт
	POP	AF
	LD	(rPORTA),A		; данные поступили - выбираем их из порта А
	POP	HL
	RET

;*******************************************
;*      Прием сектора                      *
;* HL - адрес размещения сектора в памяти  *
;*  Размер сектора - 128 байт              *
;*******************************************
preffix&EXR_GETSEC:
	di
	PUSH	BC
	PUSH	DE
	LD	C,128		; счетчик байтов сектора, всего 128 байт
	EX	DE,HL		; адрес буфера -> DE
	LD	HL,rPORTC
preffix&pGSL:
	LD	A,(HL)		; слово состояния ВВ55 - берется из порта С
	AND	20h		; выделяем сигнал IBF
	JP	Z,preffix&pGSL		; IBF=0 - данных еще нет
	LD	A,(rPORTA)		; данные поступили - выбираем их из порта А
	LD	(DE),A		; принимаем и размещаем очередной байт
	INC	DE		; указатель буфера ++
	DEC	C		; принимаем остальные байты
	JP	NZ,preffix&pGSL
	POP	DE
	POP	BC
; 	ei - int should be still disabled 
	RET

;*******************************************
;*      Передача сектора                   *
;* HL - адрес размещения сектора в памяти  *
;*  Размер сектора - 128 байт              *
;*******************************************
preffix&EXR_PUTSEC:
	di
	PUSH	BC
	PUSH	DE
	LD	C,128		; счетчик байтов сектора, всего 128 байт
	EX	DE,HL		; адрес буфера -> DE
	LD	HL,rPORTC
preffix&pPSL:
	LD	A,(HL)		; слово состояния ВВ55 - берется из порта С
	AND	80h		; выделяем сигнал -OBF
	JP	Z,preffix&pPSL		; -OBF=0 - в передатчике сидит незабранный байт
	LD	A,(DE)		; принимаем и размещаем очередной байт
	LD	(rPORTA),A		; данные поступили - выбираем их из порта А
	INC	DE
	DEC	C		; принимаем остальные байты
	JP	NZ,preffix&pPSL
	POP	DE
	POP	BC
; 	ei - int should be still disabled 
	RET

;****************************************
;*     Отправка командного пакета       *
;****************************************
preffix&EXR_SENDCMD:

	di
	PUSH	HL
	PUSH	BC
	LD	A,0Ch
	LD	(rPCWR),A	; переключаем порт в режим 2
	LD	HL,preffix&EXR_CMD
	LD	C,4		; пакет - 4 байта
	LD	B,0		; заготовка контрольной суммы
preffix&pSCL:
	LD	A,(HL)		; очередной байт пакета 
	ADD 	B		; добавляем к КС
	LD	B,A
	LD	A,(HL)		; очередной байт пакета 
	CALL	preffix&EXR_PUTBYTE		; - в порт
	INC	HL
	DEC	C
	JP	NZ,preffix&pSCL
	LD	A,B
	DEC 	A		; КС-1
	CALL	preffix&EXR_PUTBYTE     ; контрольная сумма
	CALL	preffix&EXR_GETBYTE	; ответ контроллера
	POP	BC
	POP	HL
; 	ei - int should be still disabled 
	RET

;*******************************************
;*  Чтение информационного сектора
;*******************************************
preffix&EXR_READINFO:
	LD	HL,preffix&EXR_CMD	; командный пакет
	LD	(HL),1		; команда чтения
	INC	HL
preffix&r_p_DSK3:
	LD	A,(_DSK)	; вписываем # устройства
	LD	(HL),A
	INC	HL
	LD	(HL),0		; обнуляем, поскольку читаем сектор 0 дорожки 0 
	INC	HL
	LD	(HL),0		
	CALL	preffix&EXR_SENDCMD	; отправляем команду
	OR	A
	RET 	Z		; ошибка
	LD	HL,_RRBUFF	; принимаем данные
	CALL	preffix&EXR_GETSEC
	LD	A,1
	RET


; Командный пакет интерфейса Extrom-API
;===============================================
preffix&EXR_CMD:	DB	0	; Команда чтения(0)-записи(1)
preffix&EXR_DRV:	DB	0	; Устройство - A(0), B(1)	
preffix&EXR_TRK:	DB	0	; логическая дорожка
preffix&EXR_SEC:	DB	0	; логический сектор (128b)

	endm