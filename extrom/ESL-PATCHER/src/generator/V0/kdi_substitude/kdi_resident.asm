loader_place 	equ 	0xF1D5
ldir_de_hl_c	equ 	0xE496
cuntinue_in_bios equ 	0xBF00	

sysreg1C	equ 	0xfa7f
sysreg5c 	equ 	0xff7f

stop 	macro msg
	endm

dbpatch macro addr,oldbyte,newbyte
	ld 	a,newbyte
	ld 	(addr),a
	endm

dbpatchmaybe macro addr,oldbyte,newbyte
	endm

dwpatch macro addr,oldword,newword
	ld 	hl,newword
	ld 	(addr),hl
	endm

dwstore macro addr,newword
	ld 	hl,newword
	ld 	(addr),hl
	endm

setCPM 	macro 	
	endm

setMICRODOS macro
	endm

setUNSUPPORTED macro
	endm

RQ_OPTS1	macro
	endm

RQ_OPTS2	macro
	endm

RQ_OPTS_ANY	macro
	endm


	org loader_place

	di

; 	ld 	hl,0xFB0B
; 	ld 	(hl),0xc0 ; Конифгурация ВВ55 - Порт А в режиме 2, порт В на вывод

; 	ld 	hl,0xFB32 
; 	ld 	(hl),0xc9 ; set ppic.7=1

	ld 	a,0x5c
	ld 	(sysreg1C),a 	

	ld      hl, resident_md_addr
	ld      de, resident_md
	ld      c, resident_md_len
	call 	ldir_de_hl_c

	include "../out/patcher-microdos-MICRODOS_OPTS2_900105.asm"

	ld 	a,0x1c
	ld 	(sysreg5C),a

	ld 	hl,msgPATCHED
	call    0xE486   

	ei
	jp 	cuntinue_in_bios



	include "../extrom-patcher-resident-microdos.asm"

	;ldir use C as conter
	.assert 0xff>resident_md_len
msgPATCHED:
	db 	' + EXTROM PATCH BY ESL 16 Oct 2014'
	db 	0x0d,0x0a,'$'
;           RAM:BED5          _BOOT2:                                 ; CODE XREF: RAM:BE7Bj
;           RAM:BED5                                                  ; RAM:BE8Dj
;           RAM:BED5 3E 20                    ld      a, 20h ; ' '
;           RAM:BED7 32 3A FB                 ld      (_1C_PPI1C_Vireg), a
;           RAM:BEDA FB                       ei
;           RAM:BEDB CD B6 E4                 call    PCLS
;           RAM:BEDE 21 4E F0                 ld      hl, BOOT_LOGO   ; "\x1BEdn=˜s( =d8020\r\neTT=¦s(fßv 01.05.90\r\n$"
;           RAM:BEE1 CD 86 E4                 call    PUT_STR
;           !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
;           RAM:BEE4 C3 00 BF                 jp      loc_BF00        
;           !!!!!!!!!!!!!!!!!!!!!

;           0xBEE4 -> fdd:0x0164
;           сюда ставим переход на нас, а там переход на BF00

;           используем код из загрузчика
;           RAM:BE9A 21 80 FC                 ld      hl, E_DRIVE
;           RAM:BE9D 11 00 F1                 ld      de, E_DRIVE_IMAGE
;           RAM:BEA0 0E D5                    ld      c, 213
;           RAM:BEA2 CD 96 E4                 call    ldir_de_hl_c
;           т.е. наш код это что-то типа
;           call  0xE496
