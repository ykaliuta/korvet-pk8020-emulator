;по адресу bios_variants табличка ссылок на все известные биосы
;пробуем выяснить какой биос загружен сейчас.
;если не нашли - fallback to default forth32 cp/m bios
;TODO: fallback to microdos if required
;TODO: ask user about fallback

;алгоритм патчера
;на момент статрта в памяти уже есть загруженый в свои адреса биос, проверим что это нужный и пропатчим его
;проход 1, проверка, читаем байткоды (с доп параметрами) и проверяем что "первый параметр" совпадает, 
;если нет то считаем что это не нужный нам биос, и надо проверять следующий.
;проход 2, если на превом проходе всё проверки совпали
;то проходим и записываем занчение из "второго параметра"
;заодно выставляем флаги.
;чеки надо ставить в начале цепочки (по идее)
;последний байт обязательно p_STOP
;ругаемся на невалидный токен, для отладки.

;токены патчера и их значения

p_STOP 			EQU 	0x50	;должен быть последним токено в цепочке, патчер
					;за ним следует строка с 0 - имя биоса
					; 	db 	p_STOP
					; 	db 	'21_89___wiza',0
pW_CHK_PATCH 		EQU	0x51 	;проверить значение СЛОВА, записать новое значение, параметры DW ADDR, OLDVALUE, NEWVALUE
					;на этапе чека проверяем 1й параметр, на этапе патча записываем 2й
					; 	db 	pW_CHK_PATCH
					; 	dw 	0xDCBB+1,0xE06E,0xE06E
pB_CHK_PATCH 		EQU 	0x52 	;проверить значение БАЙТА, записать новое значение, параметры DW ADDR, DB OLDVALUE, NEWVALUE
					;на этапе чека проверяем 1й параметр, на этапе патча записываем 2й
					; 	db 	pB_CHK_PATCH
					; 	dw 	0xE70A
					; 	db	0x77,0x00
pW_STORE		EQU 	0x54 	;записать значение СЛОВА, параметры DW ADDR, NEWVALUE
					;на этапе чека игнорируется, на этапе патча - применяем
					; 	db 	pW_STORE
					; 	dw 	0xE06E,r_p_SELDSKDRIVER+1	 
pNotSupported 		EQU 	0x55 	;выставляет флаг что биос не поддерживается
					;используется для биосов которые детектятся но сейчас не поддерживаются
					;на этапе чека игнорируется
					;	db 	pNotSupported
pSETFLAG_MICRODOS 	EQU 	0x56	;выставляет флаг что биос текущий биос - микродос (по умолчанию считаем что cp/m)
					;на этапе чека игнорируется
					; 	db 	pSETFLAG_MICRODOS
pREQUIRED_OPTS1 	EQU 	0x57	;проверить версию ROM, если не ОПТС 1 то вывести сообщение и поставить флг Unsupported
					;на этапе чека игнорируется
					;	db 	pREQUIRED_OPTS2
pREQUIRED_OPTS2 	EQU 	0x58	;проверить версию ROM, если не ОПТС 2 то вывести сообщение и поставить флг Unsupported
					;на этапе чека игнорируется
					;	db 	pREQUIRED_OPTS2
pB_MAYBE_PATCH 		EQU 	0x59 	;записать новое значение если старое совпадает, параметры DW ADDR, DB OLDVALUE, NEWVALUE
					;на этапе чека не проверяем
					;на этапа патча проверяем 1й параметр и если совпал записываем 2й
					;применяется для патча текстовых строк которые могут быть изменены
					; 	db 	pB_CHK_PATCH
					; 	dw 	0xE70A
					; 	db	0x77,0x00
pSubBios		EQU 	0x5A	;записывает значение в переменную flag_subbios 
					;используется чтобы указать какой вариант резидента использовать
					; 	db 	pSubBios
					; 	db 	0


DEBUG_TRACE_PATCHER	EQU 	0

stop 	macro msg
	db 	p_STOP
	db 	msg
	db 	0
	endm

dbpatch macro addr,oldbyte,newbyte
	db 	pB_CHK_PATCH
	dw 	addr
	db 	oldbyte,newbyte
	endm

dbpatchmaybe macro addr,oldbyte,newbyte
	db 	pB_MAYBE_PATCH
	dw 	addr
	db 	oldbyte,newbyte
	endm

dwpatch macro addr,oldword,newword
	db 	pW_CHK_PATCH
	dw 	addr
	dw 	oldword,newword
	endm

dwstore macro addr,newword
	db 	pW_STORE
	dw 	newword
	dw 	addr
	endm

setCPM 	macro 	
	endm

setSUBBIOS 	macro  id
	db	pSubBios
	db 	id
	endm

setMICRODOS macro
	db 	pSETFLAG_MICRODOS
	endm

setUNSUPPORTED macro
	db 	pNotSupported
	endm

RQ_OPTS1	macro
	db 	pREQUIRED_OPTS1
	endm

RQ_OPTS2	macro
	db 	pREQUIRED_OPTS2
	endm

RQ_OPTS_ANY	macro
	endm


cpm_bios_patcher:

	;dirty hasck
	;set jp 0 as first jp
	;strange dos behaviour,
	;sometime hangon when dir
	ld 	hl,0
	ld 	(BASE+1),hl 	;

	ld 	hl,msgPATCHER
	call 	PSTR

	; mute message for second time (fallback code)
	ld 	a,0
	ld 	(msgPATCHER),a


; 	call 	get_rom_version

	ld 	de,bios_variants
chk_bios_lp:
	ld 	a,(de)
	ld 	l,a
	inc 	de
	ld 	a,(de)
	ld 	h,a
	inc 	de
	or 	l
	jp 	z,bios_not_found

	push 	de
	call 	try_bios
	pop 	de

	jp 	z,bios_found

	jp 	chk_bios_lp


bios_not_found:

	ld 	hl,msgNOTFOUND
	call 	PSTR
	
; 	halt

	jp 	force_subst

bios_found:
	dec 	hl
	ld 	a,(hl)
	inc 	hl
	cp 	p_STOP
	jp 	z,.found_cont

	ld 	hl,msg_StrangeHLAfterPatch
	call 	PSTR
	halt

.found_cont:
	push 	hl 		;bios name
	ld 	hl,msgFOUND
	call 	PSTR
	pop 	hl
	call 	PSTR
	ld 	hl,msgCRLF
	call 	PSTR

	ld 	hl,(msg_warning)
	ld 	a,l
	or 	h
	call 	nz,PSTR

	ld 	a,(flag_unsupported)
	or 	a
	jp 	nz,bios_not_found

	call 	show_mount_info

; 	halt
	ret

; get_rom_version:
; ;opts1.1
; ; RAM:003B FF                       db 0FFh
; ;opts2.0
; ; RAM:003B 02                       db    2
; ; kontur
; ; RAM:003B 05                       dec     b
; ;kvant8 terminal 
; ; RAM:003B 00                       db    0
; ;kvant8 - tigris
; ; RAM:003B FF                       db 0FFh

; ; RAM:050F EF F0 F4+msgOPTS:        text "KOI8-R", 'OPTS  1.1',0
; ; RAM:051B EF F0 F4+Logo1:          text "KOI8-R", 'OPTS  2.0',0 
; 	ld 	a,(0x003b)
; 	cp 	0xff
; 	jp 	z,.opts1

; 	cp 	2
; 	jp 	z,.opts2

; .opts1:
; 	ld 	a,'1'
; 	db 	0x21
; .opts2:
; 	ld 	a,'2'	
; 	ld 	(rom_version),a
; 	ret

; rom_version:
; 	db 	0 	;1 - opts1, 2-opts2



flag_unsupported:
	db 	0
flag_microdos:
	db 	0	
flag_subbios:
	db 	0
msg_warning:
	dw 	0


msgPATCHER:
	db 	0dh,0ah,'BIOS PATCHER V 1.52 by ESL 2014',0dh,0ah,0
msgFOUND:
	db 	'Detected: ',0
msgNOTFOUND:
	db 	'BIOS Unsupported or Broken, '
	db 	01bh,'6'
	db 	'fallback to DEFAULT bios.'
	db 	01bh,'7'
msgCRLF:
	db 	0dh,0ah,0

msgREQ_OPTS:
	db 	' !! '
	db 	01bh,'6'
	db 	'BIOS require OPTS '
msgREQ_OPTS_digit:
	db 	'1.x ROM'
	db 	01bh,'7'
	db 	' and not compatible with current.'
	db 	0dh,0ah
	db 	0

work_ptr:
	dw 	0;
patch_flag:
	db 	0

;проверит совпадает ли биос с табличкой
;на входе в HL табличка для биоса
;если совпало то HL укзывает на имя биоса
try_bios:
	ld 	(work_ptr),hl

	;check
	xor 	a
	ld 	hl,(work_ptr)
	call 	do_patch
	ret nz

; 	halt

	;скопировали резидент в дырку

	;preprare to PATCH

	;copy resident

	ld 	a,(flag_microdos)
	or 	a
	jp 	nz,move_microdos_resident

	ld 	a,(flag_subbios)
	cp 	1
	jp 	z,.cpmsubBios1
	cp 	2
	jp 	z,.cpmsubBios2
	cp 	3 			;no resident part
	jp 	z,.cpmNoResident

	ld 	hl,msgUndefindedsubBios
	call 	PSTR
; 	halt


.cpmsubBios1:

	ld 	hl,_CPM1_res_DRIVE_TAB
	ld 	(drivetab_mount_info),hl

	ld 	hl,resident_cpm1
	ld 	de,resident_cpm1_addr
	ld 	bc,resident_cpm1_len

	jp 	.subBiosldir

.cpmsubBios2:
	ld 	hl,_CPM2_res_DRIVE_TAB
	ld 	(drivetab_mount_info),hl

	ld 	hl,resident_cpm2
	ld 	de,resident_cpm2_addr
	ld 	bc,resident_cpm2_len

.subBiosldir:
	call	_p_ldir

.cpmNoResident:
	ld 	a,1
	ld 	hl,(work_ptr)
	call 	do_patch

; 	halt

	ld 	(hl_after_do_patch_patch),hl

	;если дисководов нет то диски C и D - эмулируемые
	ld 	a,(HW_FDC_PRESENT)
	or 	a
	jp 	nz,patch_done


	ld 	a,(flag_subbios)
	ld 	hl,_CPM1_res_DRIVE_TAB
	cp 	1
	jp 	z,.cpmRemapCD

	ld 	hl,_CPM2_res_DRIVE_TAB
	cp 	2
	jp 	z,.cpmRemapCD

	jp 	patch_done 	;no need to patch


.cpmRemapCD:


	push 	de
	ld	de,2*2
	add 	hl,de
	ld 	a,0 ;0=use emu

	ld 	e,(hl)
	inc 	hl
	ld 	d,(hl)
	inc 	hl
	ld 	(de),a 	;drive C - emu


	ld 	e,(hl)
	inc 	hl
	ld 	d,(hl)
	inc 	hl
	ld 	(de),a	;drive D - emu

	pop 	de

	jp 	patch_done

move_microdos_resident:


	ld 	hl,md_res_DRIVE_TAB
	ld 	(drivetab_mount_info),hl

	;microdos
	ld 	a,0x5c
	ld 	(SYSREG14),a

	ld 	hl,resident_md
	ld 	de,resident_md_addr
	ld 	bc,resident_md_len
	call	_p_ldir

	ld 	a,1
	ld 	hl,(work_ptr)
	call 	do_patch
	ld 	(hl_after_do_patch_patch),hl

	ld 	a,0x14
	ld 	(SYSREG5C),a


; 	HALT
patch_done:
	ld 	hl,(hl_after_do_patch_patch)

	xor 	a
	ret
hl_after_do_patch_patch:
	dw 	0
do_patch:
	ld 	(patch_flag),a

	if DEBUG_TRACE_PATCHER
		;debug
; 		or 	a
; 		jp 	nz,.noTrace1

		push 	hl


		LD	HL,HCRLF
		CALL	PSTR
		pop	hl

		push 	hl
		CALL	HEX4
		ld 	a,':'
		call 	PUTCH
		;DEBUG

		ld 	a,'>'
		call 	PUTCH
		ld 	a,(patch_flag)
		call 	HEX2
		ld 	a,':'
		call 	PUTCH

		pop 	hl
.noTrace1:
		; 	debug
	endif

	ld 	a,0
	ld 	(flag_microdos),a
	ld 	(flag_subbios),a

patch_loop:

	ld 	a,(hl)
	inc 	hl
	if DEBUG_TRACE_PATCHER

; 		;debug
; 		push 	af
; 		ld 	a,(patch_flag)
; 		or 	a
; 		jp 	nz,.noTrace2
; 		pop 	af
		push 	af
		call 	HEX2
	.noTrace2:
		pop 	af
		;debug
	endif

	cp 	p_STOP
	ret 	z 	;FOUND

B_CHK_PATCH:
	cp 	pB_CHK_PATCH
	jp 	nz,B_CHK_MAYBE

	call 	fetch_dw_to_bc 	;BC addr
	push 	bc
	call 	fetch_db_to_b 	;b

	ex 	de,hl
	pop 	hl
	ld 	a,(hl)
	ex 	de,hl 		;de - addr

	ld 	c,b

	call 	fetch_db_to_b 	;b store

	cp 	c
	ret 	nz


	ld 	a,(patch_flag)
	or 	a
	jp 	z,patch_loop

	ex 	de,hl
	ld 	(hl),b
	ex 	de,hl
	jp 	patch_loop


B_CHK_MAYBE:
	cp 	pB_MAYBE_PATCH
	jp 	nz,l_pW_CHK

	call 	fetch_dw_to_bc 	;BC addr
	push 	bc
	call 	fetch_db_to_b 	;b

	ex 	de,hl
	pop 	hl
	ld 	a,(hl)
	ex 	de,hl 		;de - addr

	ld 	c,b

	call 	fetch_db_to_b 	;b store

	cp 	c
	jp  	nz,patch_loop

	ex 	de,hl
	ld 	(hl),b
	ex 	de,hl
	jp 	patch_loop


l_pW_CHK:
	cp 	pW_CHK_PATCH
	jp 	nz,l_pW_STORE

	call 	fetch_dw_to_bc 	
	call 	store_bc_to_tmp

	call 	fetch_dw_to_bc 	;BC OLD value

	call 	read_de_at_tmp 	;de - value
	call 	cp_de_bc

	call 	fetch_dw_to_bc 	;bc NEW value

	ret 	nz

	ld 	a,(patch_flag)
	or 	a
	jp 	z,patch_loop

	call 	store_bc_at_tmp

	jp 	patch_loop

l_pW_STORE:
	cp 	pW_STORE
	jp 	nz,.pNotSupported


	call 	fetch_dw_to_bc 	;BC value
	push 	bc

	call 	fetch_dw_to_bc 	
	call 	store_bc_to_tmp

	pop 	bc

	ld 	a,(patch_flag)
	or 	a
	jp 	z,patch_loop

	call 	store_bc_at_tmp

	jp 	patch_loop

.pNotSupported:
	cp 	pNotSupported
	jp 	nz,.pSETFLAG_MICRODOS


	ld 	a,(patch_flag)
	or 	a
	jp 	z,patch_loop

	jp 	Unsupported	

.pSETFLAG_MICRODOS:
	cp 	pSETFLAG_MICRODOS
	jp 	nz,.pSubBios

; 	ld 	a,(patch_flag)
; 	or 	a
; 	jp 	z,patch_loop

	ld 	a,1
	ld 	(flag_microdos),a
	jp 	patch_loop

.pSubBios:
	cp 	pSubBios
	jp 	nz,.pREQUIRED_OPTS1

	ld 	a,(hl)
	inc 	hl
	ld 	(flag_subbios),a
	jp 	patch_loop

.pREQUIRED_OPTS1:
	cp 	pREQUIRED_OPTS1
	jp 	nz,.pREQUIRED_OPTS2

	ld 	a,(patch_flag)
	or 	a
	jp 	z,patch_loop

; 	ld 	a,(rom_version)
	ld 	a,(HW_ROM_OPTS_CLASS)
	cp 	_HW_ROM_OPTS1
	jp 	z,patch_loop

	ld 	a,'1'
	jp 	set_opts_warning

; 	jp 	patch_loop

.pREQUIRED_OPTS2:
	cp 	pREQUIRED_OPTS2
	jp 	nz,.pHALT

	ld 	a,(patch_flag)
	or 	a
	jp 	z,patch_loop

; 	ld 	a,(rom_version)
	ld 	a,(HW_ROM_OPTS_CLASS)
	cp 	_HW_ROM_OPTS2
	jp 	z,patch_loop

	ld 	a,'2'

set_opts_warning:
	ld 	(msgREQ_OPTS_digit),a

	push 	hl
	ld 	hl,msgREQ_OPTS
	ld 	(msg_warning),hl
	pop 	hl

Unsupported:
	ld 	a,1
	ld 	(flag_unsupported),a
	jp 	patch_loop

.pHALT:
	ld 	hl,msg_InvalidPatchCode
	call 	PSTR

	halt

	ret

msg_InvalidPatchCode:
	db 	'Invalid patchcode, check registers',0dh,0ah,0
msgUndefindedSubBios:
	db 	'CP/M subbios not defined, halted',0dh,0ah,0
msg_StrangeHLAfterPatch:
	db 	'Invalid HL after patch, no p_STOP before!',0dh,0ah,0
cp_de_bc:
	ld 	a,e
	cp 	c
	ret 	nz
	ld 	a,d
	cp 	b
	ret

tmp: 	dw 	0

;stor bc to tmp
store_bc_to_tmp:
	ld 	a,c
	ld 	(tmp),a

	ld 	a,b
	ld 	(tmp+1),a
	ret

store_bc_at_tmp:
	push 	hl
	ld 	hl,(tmp)
	ld 	(hl),c
	inc 	hl
	ld 	(hl),b
	pop 	hl
	ret

;de - value at (tmp)
read_de_at_tmp:
	push 	hl
	ld 	hl,(tmp)

	ld 	e,(hl)
	inc 	hl
	ld 	d,(hl)

	pop 	hl
	ret


fetch_dw_to_bc:
	ld 	c,(hl)
	inc 	hl
fetch_db_to_b:
	ld 	b,(hl)
	inc 	hl
	ret

_p_ldir:
	ld 	a,(hl)
	ld 	(de),a
	inc 	hl
	inc 	de
	dec 	bc
	ld 	a,c
	or 	b
	jp 	nz,_p_ldir
	ret

	include 	"mount_info.asm"
	include 	"extrom-patcher-resident-cpm-body.asm"
	include 	"extrom-patcher-resident-cpm1.asm"
	include 	"extrom-patcher-resident-cpm2.asm"
	include 	"extrom-patcher-resident-microdos.asm"
	include 	"out/include-patcher.asm"

	