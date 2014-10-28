.( EXT-ROM )
HEX

(*
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

*)
code ex_put_byte
	d 		pop
	FB0A h 	lxi
 1 $:
	m 	a 	mov
	80 		ani
	1 $ 	jz

	l 		dcr
	l 		dcr
	e 	m 	mov
	next
end-code

(*
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
*)	
code ex_get_byte
	FB0A	h 	lxi
 1 $:
	m 	a 	mov
	20 		ani
	1 $ 	jz

	l 		dcr
	l 		dcr
	m 	l 	mov
	0 	h 	mvi
	h 		push
	next
end-code

decimal

create cmd_buf 4 allot
create buf80 128 allot

: ex_cmd cmd_buf 0 + ;
: ex_drv cmd_buf 1 + ;
: ex_trk cmd_buf 2 + ;
: ex_sec cmd_buf 3 + ;


: show_cmd
  base @ 
  hex
  ." cmd=" ex_cmd c@ . cr
  ." drv=" ex_drv c@ . cr
  ." trk=" ex_trk c@ . cr
  ." sec=" ex_sec c@ . cr
  base !
;


: send_cmd ( -- )
  \ ." send_cmd" show_cmd
  \ rPCWR	EQU	0FB0BH		; Порт управляющих команд

  \ 	LD	A,0Ch
  \ LD	(rPCWR),A	; переключаем порт в режим 2

	\  	H# 0e H# FB33 C!	\ set PPI2C.7 to 1 (Enable ExtRom)
  	H# 0C H# FB0B C!	\ set PPI to mode 2
	
	ex_cmd @ dup ex_put_byte
	ex_drv @ dup ex_put_byte
	ex_trk @ dup ex_put_byte
	ex_sec @ dup ex_put_byte
	+ + + 1- ex_put_byte 		\ CRC
	ex_get_byte
;

: do_cmd
 ex_cmd c!
 \ show_cmd
 send_cmd 0= if show_cmd ABORT" ERROR in do_cmd" then 
;

: get_stream ( addr count -- )
\ 	 ." getstream:" .s
	0 do
		ex_get_byte over c!
		1+
	loop
	drop ( addr -- )
 \   .s
;

: send_stream ( addr count -- )
 	\ hex ." send_stream:" .s decimal

	0 do
		dup c@ 
		\ dup . 
		ex_put_byte
		1+
	loop
	drop ( addr -- )
    \ .s
;


: get_stream_z ( addr count -- )
 \  ." getstream:" .s
	swap
	ex_get_byte dup >r 
		over c!
		1+ 
	swap r>
	0= not if
		1- dup 0> if  
			0 do
				ex_get_byte over c!
				1+
			loop
		else 
			drop \ drop size
		then
	else
		drop 	\ drop size
	then
	drop ( addr -- )
 \   .s
;

: f8487 ( addr -- ) 
 buf80
 begin
  dup 14 0 fill
  dup 14 get_stream_z
  dup  c@ 0= not while
  	2dup add_to_buf  
  repeat
  2drop
;


create _current_dir 15 allot

: get_current_dir
  H# 85 do_cmd
  buf80 14 get_stream
  buf80 _current_dir 14 move_z
;  

: set_current_dir ( buf14 -- )
  0 ex_sec c! \ temporary mount
  H# 86 do_cmd
  \ cr ." set_current_dir:" dup 14 type cr
  14 send_stream
  \ abort
;  


: get_mount_info ( buf drive -- )
 ex_drv c!
 H# 80 do_cmd
 29 get_stream
;

: mount_image ( buf drive -- )
    ex_drv c! \ DRV 
  0 ex_trk c! \ TRK 0 - r/w, 1 - r/o.
  0 ex_sec c! \ SEC 0 - temporary, 1 - permanent
  H# 81 do_cmd \ mount 

  14 send_stream
  
  H# 82 	   \ check mount
  ex_cmd c!
  send_cmd 0=  
;

