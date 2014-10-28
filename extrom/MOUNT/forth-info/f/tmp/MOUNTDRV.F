.( MOUNTDRV )

create bufbuf 128 allot
create bufbuf2 128 allot

create _mount_info 29 allot

32   4 create_buffer buf_mount

14 128 create_buffer buf_images
14 128 create_buffer buf_dirs


: read_images
 H# 84 do_cmd
 buf_images f8487
;

: read_dirs
 H# 87 do_cmd
 buf_dirs f8487
;


: _append ( from to maxsize -- to_end)
	>r 2dup
	r@ move 
	nip \ drop from
	dup r> + 0 swap c!
	dup length_z +
;

: read_mount
 4 0 do
  	_mount_info i get_mount_info

  	bufbuf
  	[CHAR] A i + over c!
  	1+
  	[CHAR] : over c!
  	1+
  	bl over c!
  	1+
  	_mount_info 1 + over 14 _append

    [CHAR] / over c!
    1+
  	_mount_info 15 + swap 14 _append
  	2drop

 	buf_mount bufbuf 
 	add_to_buf 
 loop
; 	

false 
[if]

: put_name 
	bufbuf 1+ put_to_z
	[CHAR] / emit
	bufbuf 15 + put_to_z
;


: show_mount ( drive -- )
 dup
 [CHAR] A + EMIT
 [CHAR] : EMIT

 bufbuf 29 get_stream
 put_name

\ .s
;

: a
hex
buf_mount buf.init
read_mount 
buf_mount show_buf
;

[then]



