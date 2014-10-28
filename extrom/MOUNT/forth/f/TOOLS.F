.( TOOLS )

\ Get the next word in the input stream as a hex single-number literal. (Adopted from Open Firmware.)
-? : H#  ( "hexnumber" -- n )  \  Simplified for easy porting.
	0 0 BL WORD COUNT                  
	BASE @ >R  HEX  >NUMBER  R> BASE !
	ABORT" Not Hex " 2DROP          ( n)
	STATE @ IF POSTPONE LITERAL  THEN
; IMMEDIATE

\ Get the next word in the input stream as a hex single-number literal. (Adopted from Open Firmware.)
-? : B#  ( "binnumber" -- n )  \  Simplified for easy porting.
	0 0 BL WORD COUNT                  
	BASE @ >R  2 BASE !  >NUMBER  R> BASE !
	ABORT" Not BIN " 2DROP          ( n)
	STATE @ IF POSTPONE LITERAL  THEN
; IMMEDIATE


H# FB3A VALUE DRVREG

: [hex] hex ; immediate
: [dec] decimal ; immediate
: cls H# 1F emit ;
: crlf H# 0d H# 0a emit emit ;
: spc bl emit ;

: set_inv ( flag -- )
	
	DRVREG c@
	B# 11001111 and

	swap
	
	if 
	  B# 00010000
	else
	  B# 00100000
    then
    
    or DRVREG c!
;

: xy2aczu ( x y -- )
  64 * + H# fc00 +
;

: gotoxy ( x y -- )
	xy2aczu H# F705 !
;

: cursor_off
	0 H# F707 c!
;
: cursor_on
	1 H# F707 c!
;

: inv_on true set_inv ;
: inv_off false set_inv ;

: copy_str_to_zstr ( str u1 dst -- )
  2dup + >r ( r: strend )
  swap move
  0 r> c!  ( store terminating zero)
;

(*
FB38
_vbl_:
        ld  a, 1Ch.
        ld  (regbase6C+SYSREG), a

.waitVBL:
        ld  a,(portbase1C+VISTS)..
        and     2
;         jp      z,.waitVBL    ;if z start from first screen line, no vbl
       jp      nz,.waitVBL    ;if z start from first screen line, no vbl

        ld  a, 6Ch.
        ld  (regbase1C+SYSREG), a
        ret

code ex_put_byte
  d     pop
  FB0A h  lxi
 1 $:
  m   a   mov
  80    ani
  1 $   jz

  l     dcr
  l     dcr
  e   m   mov
  next
end-code
*)

code vbl
  H# FB38 h lxi
  1 $:
  m   a   mov
  2       ani
  1 $     jnz  
  next
end-code



: scr_init
  cls
  cursor_off
;

: move_z ( from to size -- )
  2dup >r >r
  move
  r> 1 ( addr flag )
  r> 0 do 
  	if
  	 	dup c@ 0= if
  	 		0
  		else
  			1
  		then
  	else
  	 	0 over c!
  	 	0
  	then
  	swap 1+ swap
  loop

  2drop
;

: put_to_z ( addr -- )
 \ ." put_to_zero" .s
  begin
    dup
    c@
  dup 0 = not while
    emit
    1+
  repeat
  drop
  drop
 \   .s
;

: length_z ( string -- len )  ( length of the string at addr )
  dup >r begin dup c@ while 1+ repeat  r> - ;

crlf

