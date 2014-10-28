_HW_ROM_OPTS1		equ 	1
_HW_ROM_OPTS2		equ 	2

_HW_ROM_MODEL_OPTS11 	equ 	1
_HW_ROM_MODEL_OPTS20 	equ 	2
_HW_ROM_MODEL_KVANT8	equ 	3

_HW_GZU_SIZE_48K 	equ 	1
_HW_GZU_SIZE_192K 	equ 	2

;0 - undefined
;1 - OPTS 1.x
;2 - OPTS 2.x
HW_ROM_OPTS_CLASS:
	db 	0 	


;0 - undefined
;1 - ОПТС 1.1
;2 - ОПТС 2.0
;3 - КОНТУР
;4 - КВАНТ-8 - Терминал
;5 - КВАНТ-8 - Тигрис
;6 - CPM.ROM
HW_ROM_MODEL:
	db 	0

;0 - undefined
;1 - 48k
;2 - 192k
HW_GZU_SIZE:	db 	0


HW_FDC_PRESENT:	db 	0
HW_FDD_DRIVE_A:	db 	0		
HW_FDD_DRIVE_B:	db 	0		
HW_FDD_DRIVE_C:	db 	0		
HW_FDD_DRIVE_D:	db 	0		

msg_tab8_rom_model:
	;        01234567
	db 	'??ROM?? '
	db 	'OPTS 1.1'
	db 	'OPTS 2.0'
	db 	'Kvant8  '
;нет поддержки загрузки EXTROM
; 	db 	'Kontur  ' 	
; 	db 	'Kvant8-T'
; 	db 	'CPM     '

msg_tab8_gzu_size:
	;        01234567
	db 	'???k ',0,0,0
	db 	'48k ',0,0,0,0
	db 	'192k ',0,0,0

msg_tab16_fdc:
	;        01234  5     6   789012  3     4  5
	db 	'10 - ',01bh,'6','no FDC',01bh,'7',0
	;        0123456789012345
	db 	'20 with FDC',0

msg_rom_model:
	db 	'ROM: ',0
msg_gzu_size:
	db 	' | GZU: ',0
msg_fdc:
	db 	' | PK80',0

; CP/M-80  
; v. 2.2..SHCOMP BIOS Ver. 1.0 1991

putstr16:
	ld 	c,16
	ld 	l,a
	ld 	h,0
	add 	hl,hl 	
	jp 	putx8
; a - n
;DE - base addr
putstr8:
	ld 	c,8
	ld 	l,a
	ld 	h,0
putx8:
	add 	hl,hl 	
	add 	hl,hl 	
	add 	hl,hl 	
	add 	hl,de

putstr_:
	ld 	a,c
	or 	a
	ret 	z
	dec 	c
	LD	A,(HL)
	or 	a
	ret 	z
; 	jp 	nz,.putNoSpc
; 	ld 	a,'_'
; 	dec 	hl
.putNoSpc:
	INC	HL
	CALL	PUTCH		
	JP	putstr_

show_hardware_info:
	ld 	hl,msg_rom_model
	call 	PSTR

	ld 	a,(HW_ROM_MODEL)
	ld 	de,msg_tab8_rom_model
	call 	putstr8

	ld 	hl,msg_fdc
	call 	PSTR

	ld 	a,(HW_FDC_PRESENT)
	ld 	de,msg_tab16_fdc
	call 	putstr16

; 	ld 	a,'/'
; 	call 	PUTCH

; 	ld 	a,(HW_ROM_OPTS_CLASS)
; 	call 	HEX2

	ld 	hl,msg_gzu_size
	call 	PSTR

	ld 	a,(HW_GZU_SIZE)
	ld 	de,msg_tab8_gzu_size
	call 	putstr8

	LD	HL,HCRLF
	CALL	PSTR

	ret


hw_test:
	call 	hw_check_rom
	call 	hw_check_gzu_size
	call 	hw_check_floppy
	call 	show_hardware_info

; 	halt
	ret

hw_check_floppy:
	ld 	a,1
	ld 	de,HW_FDC_PRESENT
	ld 	(de),a

; Проверка наличия дисководов
	LD	HL,0FB19H 	; регистр дорожки
	LD	(HL),5		; записываем образец
	LD	A,(HL)
	CP	5		; проверяем
	RET 	Z		; совпал - FDC у нас есть
	ld 	a,0
	ld 	(de),a
	ret
; В машине нет FDC
; 	XOR	A	
; 	LD	(CPMT02-2),A	; Очищаем маски дисков C и D
; 	LD	(CPMT03-2),A	; Они становятся эмулируемыми
; 	RET	ret 

NCREG 	equ 	0xFE3A
VIREG 	equ 	0xFFBF

hw_check_gzu_size:
; 	проверка ГЗУ, не убивая рамдиск в 1+ страницах
; 	включаем 0 страницу на доступ
; 	в последние 3 байта пишем значения
; 	включаем 3 страницу на доступ
; 	проверяем содержимое этих байт
; 	если совпало то 48к (т.к. переключение не проихошло)
; 	иначе счетаем что 192к


;3C  DOSG1          0000-3FFF  4000  8000-FDFF        FF00        FE00
	ld 	a,0x3C
	ld 	(SYSREG14),a

        ld      hl, 07FFFh 	;last gzu byte

	; vireg pointed to RW page 0

        ld      a,0x80 | 4 | 0x20 ;color 2 - green (должен быть не 1)
        ld      (VIREG), a

        ld 	bc,0x55aa
        ld      (hl), b   
        dec 	hl
        ld 	(hl), c
        dec 	hl
        ld 	(hl), l
        inc 	hl
        inc 	hl

        ld      a, 0xE0 	;SCR_PAGE0|ATTR_RESET|RW_PAGE3
        ld      (NCREG), a

        ;читаем инвертированные значения не 0x55aa а 0xaa55
        ld 	a,c 		
        cp 	m
        jp 	nz,.zgu192k
        dec 	hl
        ld 	a,b
        cp 	m
        jp 	nz,.zgu192k
        dec 	hl
        ld 	a,l
        xor 	0xff
        cp 	m
        jp 	nz,.zgu192k

        ld 	a,_HW_GZU_SIZE_48K
        jp      .setgzusize 

.zgu192k:
        ld 	a,_HW_GZU_SIZE_192K

.setgzusize:
        ld      (HW_GZU_SIZE),a

        ld 	a,0x20		;SCR_PAGE0|ATTR_RESET|RW_PAGE0
        ld      (NCREG),a  	

        ;стереть следы проверки
        ld      a,0x80 | 2 | 0x10 ;color 1 - blue
        ld      (VIREG), a
        ld 	hl,0x7fff
        ld 	a,0xff
        ld 	(hl),a
        dec 	hl
        ld 	(hl),a
        dec 	hl
        ld 	(hl),a

	ld 	a,0x14;
	ld 	(SYSREG3C),a

	ret

hw_check_rom:
	ld 	hl,0
	ld 	c,0
	ccf
.romcrc:
	ld 	a,c
	adc 	a,(hl)
	ld 	c,a
	inc 	hl
	ld 	a,l
	or 	a
	jp 	nz,.romcrc
	ld 	a,c

	ld 	b,_HW_ROM_OPTS1
	ld 	c,_HW_ROM_MODEL_OPTS11
	cp 	0x9A
	jp 	z,.rom_found

	ld 	c,_HW_ROM_MODEL_KVANT8
	cp 	0x66
	jp 	z,.rom_found

	ld 	b,_HW_ROM_OPTS2
	ld 	c,_HW_ROM_MODEL_OPTS20
	cp 	0xBB
	jp 	z,.rom_found

	ld 	c,0
.rom_found:
	ld 	a,b
	ld 	(HW_ROM_OPTS_CLASS),a
	ld 	a,c
	ld 	(HW_ROM_MODEL),a

	ret