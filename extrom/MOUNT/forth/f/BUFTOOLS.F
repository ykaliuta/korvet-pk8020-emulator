.( BUF-TOOLS ) cr

\ 128 VALUE max_rec_in_buf
\ 14 VALUE rec_size 

\ create bufbuf 14 2 * allot

: a.records  ( addr -- records_in_buf)         ;
: a.max_rec  ( addr -- max_records_in_buf)  1+ ;
: a.rec_size ( addr -- record size)         2+ ;

: a.records@  ( addr -- records_in_buf)         c@ ;
: a.max_rec@  ( addr -- max_records_in_buf)  1+ c@ ;
: a.rec_size@ ( addr -- record size)         2+ c@ ;


: .hdrsize 3 ;


: buf.init ( addr -- )
  0 over c!
  dup a.max_rec@ over a.rec_size@  * 
  swap .hdrsize + swap H# e5 
  fill
; 

\ 14 128 create_buffer buf_dirs 

-? : create_buffer ( record_size max_record -- )
  create

  here >r

  0 c,    \ actual records
  2dup
  c,      \ max_record
  c,      \ record_size
  * allot

  r>

  buf.init

  does>
;


: add_to_buf ( buf item -> )
  >r
  dup  a.records@ 
  over a.max_rec@  
  < if
    dup a.rec_size@ dup >r over a.records@ *  
    over + .hdrsize +          		     \ addr in bufeer for record
    r> 
    r> 
    swap >r swap r> 
    move_z
    a.records dup c@ 1+ swap c!  \ inc used record
	else
    r>
    a.max_rec@ decimal . 
		abort"  add buf overflow"
	then
;

\ : get_buf_nth ( addr idx -- addr )
: a.buf[n] ( addr idx -- addr )
	over a.rec_size@ * + 
  .hdrsize + \ counter maxrec recsize
;

: find_in_buf ( buf_addr string - -1/N )
  -1 locals| findflag str buff | 

  buff
  buff a.records@ 0 do 
    dup i a.buf[n] 
    over a.rec_size@ str over compare
    0= if i to findflag leave then
  loop
  drop
  findflag
;

: show_buf ( addr -- )
  dup a.records@ dup ." records in buf=" . cr
  dup 0 > if
  	0 do
  		dup i a.buf[n]
      over a.rec_size@ type cr
  	loop
  	drop
  else
    ." BUFFER EMPTY"
  	2drop
  then
;

false 
[if]

create bufbuf 128 2 * allot

14 128 create_buffer test_buf 

: badd 
    bufbuf copy_str_to_zstr
    test_buf bufbuf add_to_buf 
;

: binit
  test_buf buf.init


  S" 01234567891234"  badd
  S" L22"       badd
  S" L333"      badd
  S" L4444"       badd
  S" L55555"      badd
  S" L555550"     badd
  S" L555551"     badd
  S" L555552"     badd
  S" L555553567891"   badd
(*
  S" L-85554"     badd
  S" L-75555"     badd
  S" L-65556"     badd
  S" L-55557"     badd
  S" L-45558"     badd
  S" L-35559"     badd

  S" L-255510"    badd
  S" L-155511"    badd
*)
  S" LAST"      badd
\  test_buf show_buf
;
\ binit
[then]

