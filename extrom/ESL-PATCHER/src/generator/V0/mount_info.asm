API_GET_MOUNT_NAME	EQU	0x80
API_GET_MOUNT_STATUS 	EQU	0x82

; x123:
; 	db 0dh,0ah
; 	db '0         1         2         3         4         5         6   '
; 	db '0123456789012345678901234567890123456789012345678901234567890123'
; 	db 0

msgBootFromImage:
	db 	'--EXTROM: Booting from image: ',0
show_boot_image_name:
	ld 	hl,msgBootFromImage
	call 	PSTR


	ld 	(DRV),a

	ld 	a,0
	ld 	(drive_rw_status),a
	call 	get_mount_info
	call 	build_emu_name

	jp 	show_disk_info

show_mount_info:

	ld 	hl,msg_default_mount
	call 	PSTR
	
	ld 	a,0
	call 	show_mount_drive

	ld 	a,1
	call 	show_mount_drive

	ld 	a,2
	call 	show_mount_drive

	ld 	a,3
	call 	show_mount_drive

; 	ld 	hl,mount_crlf
; 	call 	PSTR

	;show info about drive F 	
	ld 	a,(flag_microdos)
	or 	a
	ret 	nz	

	ld 	hl,msgDRIVE_F
	call 	PSTR

	ret

show_mount_drive:
	ld 	(DRV),a

	ld 	a,0
	ld 	(drive_rw_status),a

	call 	get_drvinfo_ptr
	jp 	z,skip_disk 	;de=0, no drive at all

	ld 	a,(de)
	or 	a
	jp 	nz,.physical_drive

	call 	get_mount_info
	call 	build_disk_name
	call 	build_emu_name

	jp 	show_disk_info

.physical_drive:

	push 	af
	call 	build_disk_name
	pop 	af

	ld 	c,'A'
	cp 	0001b
	jp 	z,fdd_found
	ld 	c,'B'
	cp 	0010b
	jp 	z,fdd_found
	ld 	c,'C'
	cp 	0100b
	jp 	z,fdd_found
	ld 	c,'D'
	cp 	1000b
	jp 	z,fdd_found

	ld 	hl,msg_fdd_err
	jp 	.cpfddname

fdd_found:
	ld 	a,c
	ld 	(msg_fdd_drv),a

	ld 	hl,msg_fdd

.cpfddname

	ld 	de,out_buffer
	ld 	c,32
	ld 	b,14
	call 	append_z

show_disk_info:

	ld 	hl,out_buffer
	ld 	c,32+1
	call 	fix_width
	call 	PSTR

; 	ld 	hl,mount_crlf
; 	call 	PSTR

skip_disk:
	ret

build_disk_name:
	xor 	a
	ld 	(out_buffer+00),a
; 	ld 	(out_buffer+32),a

	ld 	a,(DRV)
	add 	a,'A'
	ld 	(mount_msg_drv),a

	ld 	bc,'RW'
	ld 	a,(drive_rw_status)
	or 	a
	ld 	a,c
	jp 	nz,.rw
	ld 	a,b
.rw:
	ld 	(mount_msg_rw),a

;--
	ld 	de,out_buffer
	ld 	hl,mount_msg
	ld 	c,32
	ld 	b,14
	call 	append_z

	ret

;in (DRV)
;de - addr
;z=0 if de=0
get_drvinfo_ptr:
	push 	hl
	ld 	a,(DRV)
	add 	a,a
	ld 	e,a
	ld 	d,0
	ld 	hl,(drivetab_mount_info)
	
	ld 	a,h
	or 	l
	jp 	z,errZeroDrvInfoTab
	add 	hl,de
	ld 	e,(hl)
	inc 	hl
	ld 	d,(hl)
	ld 	a,e
	or 	d
	pop 	hl
	ret
errZeroDrvInfoTab:
	ld 	hl,msgZeroDrvInfoTab
	call 	PSTR
	halt
msgZeroDrvInfoTab:
	db 	'ShowInfo: DrvTab=0000',0

build_emu_name:

	ld 	de,out_buffer
	ld 	c,32
	ld 	b,14

	ld 	hl,mount_info_from_emu+1
	call 	append_z
	
	ld 	a,'/'
	ld 	(de),a
	inc 	de
	xor 	a
	ld 	(de),a


	ld 	de,out_buffer
	ld 	hl,mount_info_from_emu+1+14
	ld 	c,32
	ld 	b,14
	call 	append_z

	xor 	a
	ld 	(de),a

	ret




;append (skip all not 0 and then copy_z)
append_z:
	ld 	a,c
	or 	a
	ret 	z

	ld 	a,(hl)
	or 	a
	ret 	z

.skipAppend:
	ld 	a,(de)
	or 	a
	jp 	z,copy_z
	inc 	de
	dec 	c
	jp 	z,copy_z	

	jp 	.skipAppend

;copy upto C char (or till 0) from HL to DE
;c - dstbuf sizw
;b - from buf size
;hl pointed to NULL terminated string

copy_z:
	ld 	a,c	;to str size left =0
	or 	a
	ret 	z

	ld 	a,b 	;from str size left =0
	or 	a
	ret 	z

	ld 	a,(hl)
	ld 	(de),a
	or 	a
	ret 	z
; 	jp 	nz,.copy_z_store
; 	ld 	a,'_'
; .copy_z_store:
	inc 	hl
	inc 	de
	dec 	c
	dec 	b
	jp 	copy_z

;hl ptr
;c  len
;дополняет хвост строки пробелами делая её длинной len
fix_width:
	ld 	a,c
	or 	a
	ret 	z

	push 	hl

	dec 	hl
.skipLP:	
	dec 	c
	jp 	z,.endfill
	inc 	hl
	ld 	a,(hl)
	or 	a
	jp 	nz, .skipLP

	ld 	a,' '

.fillLoop:
	ld 	(hl),a
	inc 	hl
	dec 	c
	jp 	nz,.fillLoop

	xor 	a
	ld 	(hl),a 	;0
.endfill:
	pop 	hl
	ret

get_mount_info:
	ld 	a,API_GET_MOUNT_NAME
	ld 	(CMD),a
	call 	SENDCMD

	ld 	c,29

receive_c_bytes_to_hl:

	ld 	hl,mount_info_from_emu
get_lp:
	call 	GETBYTE
	ld 	(HL),a
	inc 	hl
	dec 	c
	jp 	nz,get_lp
	ret

;GETBYTE:
; 	PUSH	HL
; 	LD	HL,PORTC
; WG:
; 	LD	A,(HL)		; слово состояния ВВ55 - берется из порта С
; 	AND	20h		; выделяем сигнал IBF
; 	JP	Z,WG		; IBF=0 - данных еще нет
; 	DEC	L
; 	DEC	L
; 	LD	A,(HL)		; данные поступили - выбираем их из порта А
; 	POP	HL
; 	RET


drivetab_mount_info:
		dw 	0

msg_default_mount:
		db 	'Drive map:',0dh,0ah
;		db 	'          1         2         3         4         5         6   '
;		db 	'0123456789012345678901234567890123456789012345678901234567890123'
		db 	0
mount_msg:	
mount_msg_drv: 	db 	'A:'
mount_msg_rw:	db 	'W '
		db 	0
out_buffer:
		ds 	32
		db 	0
msg_fdd:
		db	'$FDD - '
msg_fdd_drv:	db 	'A',0

msg_fdd_err:
		db 	'??ERROR??',0

mount_info_from_emu: 	
drive_rw_status:
		db 	0 			;=1 if R/O
		db 	'              ' 	;Folder 14 asciiZ 
		db 	'              ' 	;Name   14 asciiZ
		db 	0

mount_crlf: 	db 	0dh,0ah,0	

msgDRIVE_F:
		db 	'F: - /EXRTOOLS.KDI with MOUNT and other EXTROM related utils'
		db 	0x0d,0x0a,0

; CMD:	DB	1		; Команда чтения
; DRV:	DB	0
; TRK:	DB	0
; SEC:	DB	0


; 80 — получить имя образа

; Команда предназначена для получения имени KDI-файла, смонтированного в данный момент на логическом устройстве A-D.  
;  Номер устройства задается в поле DRV. Возвращается буфер размером 29 байт, содержащий следующую информацию:

; 1 байт флага разрешение записи (0 — чтение/запись, 1 — только чтение)
; 14-байтовый буфер с именем каталога, из которого смонтирован файл образа, заканчивающийся 0.
; 14-байтовый буфер с именем файла, заканчивающийся 0.

; 82 — проверка состояния устройства
; 	Команда возвращает ответ ОК, если устройство смонтировано, или Fail, если не смонтировано. 
; Флаг состояния автоматически опускается в случае ошибок ввода-вывода на данном устройстве. 
; С помощью этой команды можно проверить результат предыдущего вызова mount (81)
