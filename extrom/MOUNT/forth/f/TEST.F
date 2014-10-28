.( TEST )

: cclear
	buf_dirs buf.init 
	buf_images buf.init 
;



\ : aa 0 show_mount crlf ;
\ : bb 1 show_mount crlf ;
\ : cc 2 show_mount crlf ;
\ : dd 3 show_mount crlf ;


: curdir
  get_current_dir _current_dir put_to_z
;

(  DEBUG  )
: dirs
	read_dirs 
	buf_dirs show_buf
;
: images
	read_images 
	buf_images show_buf
;


: dd
	scr_init
	buf_dirs  buf.init 
	read_dirs 
	buf_dirs 1 1 3 3 0 menu_select
;

: ff
	scr_init
	buf_images buf.init
	read_images
	buf_images 1 5 8 3 0 menu_select
;

: letter_filter ( letter -- letter )
	dup [CHAR] A [CHAR] D BETWEEN if dup [CHAR] A - to m_cursor then
	dup [CHAR] a [CHAR] d BETWEEN if dup [CHAR] a - to m_cursor then

	filter_hex
;

: select_drive
	['] letter_filter IS MENU_FILTER
	\ ['] filter_hex IS MENU_FILTER

	scr_init
	buf_mount buf.init
	read_mount
	buf_mount 1 1 4 1 -1 menu_select

	\ 1 14 gotoxy cr ." selected:" . 

	['] NOOP IS MENU_FILTER
;

: select_image
	buf_images buf.init 
	read_images
	buf_images 1 8 5 3 -1 menu_select
;


: select_file_for_mount
 
 0 local idx_dir
 0 local idx_image

 \ scr_init
 \ cls
 cursor_off

 buf_dirs buf.init

 read_dirs 
 get_current_dir  

 \ cr ." getdirs:" .s

 buf_dirs _current_dir find_in_buf 
 dup to idx_dir

 \ cr ." if:" .s

 dup -1 = if
 	." _current_dir=" _current_dir put_to_z
	abort" ERR can't find current dir"
 else
 	>r 
 	buf_dirs 1 6 3 3 r> menu_select

 	dup -1 = not if 
 		dup to idx_dir

 		buf_dirs swap a.buf[n]

 		dup set_current_dir

 		2 5 xy2aczu dup
 		H# 8C swap c!
 		H# 8C swap 1+ c!

 		1 6 gotoxy 48 spaces
 		1 6 gotoxy ." Folder: " inv_on put_to_z inv_off ." , please select file"

 		select_image 

 		1 14 gotoxy 

 		dup -1 = if
 			drop
 			." getfile: ESC Presed"
 			0
 		else
 			." Selected file: "

 			dup to idx_image

 			buf_dirs idx_dir a.buf[n] 
 			14 type

 			[CHAR] / emit

 			buf_images swap a.buf[n] 

 			14 type

 			buf_images idx_image a.buf[n]  \ return name
 		then
 	else
 		drop
 		1 14 gotoxy 
 		." getdir: ESC pressed "  
 		0
 	then
 then

 cursor_on
;

: set_active_dir_from_drive ( drive  -- 0 if error )

	dup

	_mount_info swap get_mount_info
	_mount_info 1+ bufbuf 14 move_z

	bufbuf

	set_current_dir

	get_current_dir


	_current_dir 14 bufbuf 14 compare
;


: tt
 	0 local idx_drive
	select_drive ( -- drive )
	dup to idx_drive
	dup -1 = not if
		set_active_dir_from_drive
		if
			\ cr
			\ ." Folder doesn't exist, use DISK instead " cr
			S" DISK" bufbuf copy_str_to_zstr
			bufbuf set_current_dir
		then

		drop ( drive --  for use to get name in file selector )
		
		select_file_for_mount
		
		?dup if
			idx_drive mount_image 
			if 
				." MOUNT ERROR !" cr
 			then
 		then
	else
 		drop
 		1 14 gotoxy 
 		." tt: ESC pressed "  
	then
;