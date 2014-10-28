MD_rPORTA	EQU	0FE08H		; Порт Канала A интерфейса расширения
MD_rPORTC	EQU	0FE0AH		; Порт Канала С интерфейса расширения
MD_rPCWR	EQU	0FE0BH		; Порт управляющих команд

MD_RRBUFF 	EQU 	0

resident_md:

; 	.phase 	0xFD80
	.phase 	0xFA00
resident_md_addr 	EQU 	$


;мы тут в конфигуации 0x5C

;Код Cfg    ПЗУ     ОЗУ        ГЗУ   ОЗУ2       Клав  Рег2  А/Ц   Рег1
;ID  Name   ROM     RAM        GZU   Ram2       KBD   REGS  ACZU  DEV
;______________________________________________________________________
;1C  ODOSA          0000-F7FF                   F800  FA00  FC00  FB00
;5C  DOSA           0000-FDFF                         FF00        FE00



;PATCH --------------------------------------------------------------------------

;do_dskIO

; RAM:EAFD FE 04                    cp      4 		
; RAM:EAFF CA 36 EB                 jp      z, jREAD 		<--- MY write
; RAM:EB02 FE 06                    cp      6
; RAM:EB04 CA A0 EB                 jp      z, jWRITE 		<--- MY write


; SELDSK:
; RAM:EC87 CD 14 EE                 call    SEL_DSK_SEEK0       <--- 00 00 00
; RAM:EC8A CD 1E EE                 call    ReadToRDBUF 	<--- MY read info into F04E. nz if err 

; RAM:EC3A          Flush?:                                 ; CODE XREF: do_DSKIO+6Ap
; RAM:EC3A                                                  ; do_DSKIO+9Ap ...
; RAM:EC3A 21 FD EE                 ld      hl, flagWriteReuired?
; RAM:EC3D 7E                       ld      a, (hl)
; RAM:EC3E B7                       or      a
; RAM:EC3F C8                       ret     z  			<--- C9, newer flush
; RAM:EC42 21 0A EF                 ld      hl, WrBufDiskInfo   <--- or C9 HERE


;DISKINFO -----------------------------------------------------------------------

; RAM:EB36          jREAD:                                  ; CODE XREF: do_DSKIO+40j
; RAM:EB36 2A F8 EE                 ld      hl, (DSC_IO_HL) ; DSCDESCR
; RAM:EB39 46                       ld      b, (hl)         ; Drive
; RAM:EB3A 23                       inc     hl

; RAM:EAF1 7E                       ld      a, (hl) 	    ;operation 3-reset?, 4-read,6-write
; RAM:EAF2 23                       inc     hl

; RAM:EB3C 23                       inc     hl 		    ; ?chword?
; RAM:EB3D 23                       inc     hl              ; ?NumB?

; RAM:EB3E 4E                       ld      c, (hl)         ; track
; RAM:EB3F 23                       inc     hl

; RAM:EB40 7E                       ld      a, (hl)         ; sector
; RAM:EB3F 23                       inc     hl

; RAM:EAF6 5E                       ld      e, (hl)
; RAM:EAF7 23                       inc     hl
; RAM:EAF8 56                       ld      d, (hl)
; RAM:EAF9 EB                       ex      de, hl
; RAM:EAFA 22 FA EE                 ld      (_DMA??), hl
;DISKINFO -----------------------------------------------------------------------
md_res_DRIVEA:
	db 	0
md_res_DRIVEB:
	db 	0

md_res_DRIVE_TAB:
	dw 	md_res_DRIVEA 	;A
	dw 	md_res_DRIVEB 	;B
	dw 	0 	;C
	dw 	0 	;D
	dw 	0 	;E
	dw 	0 	;F

md_res_seldsk:
	ld 	a,c
	cp 	0xfe
md_old_seld_dsk:
	jp 	nz,$	
	ld 	hl,md_res_DRIVE_TAB
	ret

md_fetch_params:

	push 	af
	push 	bc
	push 	de
	push 	hl

MD_PARAM2:
	ld      hl, (0xEEF8) ; DSCDESCR
	ld      a, (hl)         ; Drive
	inc     hl
	ld 	(MD_EXR_DRV),a

; 	ld      a, (hl)		;operation 3-reset?, 4-read,6-write
	inc     hl

	inc     hl		; ?chword?
	inc     hl              ; ?NumB?

	ld      a, (hl)         ; track
	inc     hl
	ld 	(MD_EXR_TRK),a

	ld      a, (hl)         ; sector
	DEC	A
	inc     hl
	ld 	(MD_EXR_SEC),a

	ld      e, (hl)
	inc     hl
	ld      d, (hl)
	ex      de, hl
	ld      (MD_DMA), hl

	pop 	hl
	pop 	de
	pop 	bc
	pop 	af

	ret

MD_READ:
	call 	md_fetch_params
	
	LD	A,1
	LD	(MD_EXR_CMD),A	; 1 - команда чтения

	CALL	MD_EXR_SENDCMD	; отсылаем команду
	DEC	A		; ответ, 0 - ошибка, 1 - ОК
	RET	NZ		; 0 - ошибка
	LD	HL,(MD_DMA)	; адрес буфера для приема данных
	CALL	MD_EXR_GETSEC	; принимаем данные
	XOR	A		; завершение всегда успешно
	RET

MD_WRITE:
	call	md_fetch_params

	LD	A,2		; 2 - команда записи
	LD	(MD_EXR_CMD),A

	CALL	MD_EXR_SENDCMD
	DEC	A		; ответ, 0 - ошибка, 1 - ОК
	RET	NZ		; 0 - ошибка
	LD	HL,(MD_DMA)
	CALL	MD_EXR_PUTSEC
	XOR	A		; запись успешна
	RET

MD_READ_INFOSECTOR:
	CALL	MD_EXR_READINFO
	DEC	A		; ответ, 0 - ошибка, 1 - ОК
	ret


;==================  Драйвер ExtROM-API ====================================
	
	
;****************************************
;*  Прием байта из порта А по стробу    *
;****************************************
MD_EXR_GETBYTE:
	PUSH	HL
	LD	HL,MD_rPORTC
MD_pWG:
	LD	A,(HL)		; слово состояния ВВ55 - берется из порта С
	AND	20h		; выделяем сигнал IBF
	JP	Z,MD_pWG		; IBF=0 - данных еще нет
	LD	A,(MD_rPORTA)		; данные поступили - выбираем их из порта А
	POP	HL
	RET

;****************************************
;*  Отправка байта в порт A             *
;****************************************
MD_EXR_PUTBYTE:
	PUSH	HL
	PUSH	AF
	LD	HL,MD_rPORTC
MD_pWP:
	LD	A,(HL)		; слово состояния ВВ55 - берется из порта С
	AND	80h		; выделяем сигнал -OBF
	JP	Z,MD_pWP		; -OBF=0 - в передатчике сидит незабранный байт
	POP	AF
	LD	(MD_rPORTA),A		; данные поступили - выбираем их из порта А
	POP	HL
	RET

;*******************************************
;*      Прием сектора                      *
;* HL - адрес размещения сектора в памяти  *
;*  Размер сектора - 128 байт              *
;*******************************************
MD_EXR_GETSEC:
	di
	PUSH	BC
	PUSH	DE
	LD	C,128		; счетчик байтов сектора, всего 128 байт
	EX	DE,HL		; адрес буфера -> DE
	LD	HL,MD_rPORTC
MD_pGSL:
	LD	A,(HL)		; слово состояния ВВ55 - берется из порта С
	AND	20h		; выделяем сигнал IBF
	JP	Z,MD_pGSL		; IBF=0 - данных еще нет
	LD	A,(MD_rPORTA)		; данные поступили - выбираем их из порта А
	LD	(DE),A		; принимаем и размещаем очередной байт
	INC	DE		; указатель буфера ++
	DEC	C		; принимаем остальные байты
	JP	NZ,MD_pGSL
	POP	DE
	POP	BC
	ei 		
	RET

;*******************************************
;*      Передача сектора                   *
;* HL - адрес размещения сектора в памяти  *
;*  Размер сектора - 128 байт              *
;*******************************************
MD_EXR_PUTSEC:
	di
	PUSH	BC
	PUSH	DE
	LD	C,128		; счетчик байтов сектора, всего 128 байт
	EX	DE,HL		; адрес буфера -> DE
	LD	HL,MD_rPORTC
MD_pPSL:
	LD	A,(HL)		; слово состояния ВВ55 - берется из порта С
	AND	80h		; выделяем сигнал -OBF
	JP	Z,MD_pPSL		; -OBF=0 - в передатчике сидит незабранный байт
	LD	A,(DE)		; принимаем и размещаем очередной байт
	LD	(MD_rPORTA),A		; данные поступили - выбираем их из порта А
	INC	DE
	DEC	C		; принимаем остальные байты
	JP	NZ,MD_pPSL
	POP	DE
	POP	BC
	ei 		
	RET

;****************************************
;*     Отправка командного пакета       *
;****************************************
MD_EXR_SENDCMD:

	di
	PUSH	HL
	PUSH	BC
	LD	A,0Ch
	LD	(MD_rPCWR),A	; переключаем порт в режим 2
	LD	HL,MD_EXR_CMD
	LD	C,4		; пакет - 4 байта
	LD	B,0		; заготовка контрольной суммы
MD_pSCL:
	LD	A,(HL)		; очередной байт пакета 
	ADD 	B		; добавляем к КС
	LD	B,A
	LD	A,(HL)		; очередной байт пакета 
	CALL	MD_EXR_PUTBYTE		; - в порт
	INC	HL
	DEC	C
	JP	NZ,MD_pSCL
	LD	A,B
	DEC 	A		; КС-1
	CALL	MD_EXR_PUTBYTE     ; контрольная сумма
	CALL	MD_EXR_GETBYTE	; ответ контроллера
	POP	BC
	POP	HL
	ei 		
	RET

;*******************************************
;*  Чтение информационного сектора
;*******************************************
MD_EXR_READINFO:
	LD	HL,MD_EXR_CMD	; командный пакет
	LD	(HL),1		; команда чтения
	INC	HL
MD_DRV2:
	LD	A,(0xEF06)	; вписываем # устройства
	LD	(HL),A
	INC	HL
	LD	(HL),0		; обнуляем, поскольку читаем сектор 0 дорожки 0 
	INC	HL
	LD	(HL),0		
	CALL	MD_EXR_SENDCMD	; отправляем команду
	OR	A
	RET 	Z		; ошибка
MD_RDBUF2:
	LD	HL,MD_RRBUFF	; принимаем данные
	CALL	MD_EXR_GETSEC
	LD	A,1
	RET


; Командный пакет интерфейса Extrom-API
;===============================================
MD_EXR_CMD:	DB	0	; Команда чтения(0)-записи(1)
MD_EXR_DRV:	DB	0	; Устройство - A(0), B(1)	
MD_EXR_TRK:	DB	0	; логическая дорожка
MD_EXR_SEC:	DB	0	; логический сектор (128b)
MD_DMA: 	DW 	0 	; адресс куда/откуда

	.dephase
resident_md_len 	equ $-resident_md


