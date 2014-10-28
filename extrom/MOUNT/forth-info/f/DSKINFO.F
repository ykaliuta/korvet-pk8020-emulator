
: .bin8 
 base @ >r
 2 base !
 s>d
 <# # # # # # # # # #> 
 type 
 r> base !
;

: .h base @ >r hex u. r> base ! ;

0 value DRVMASKTAB

: init_drv_mask_tab
	0 H# fe 9 BIOSHL to DRVMASKTAB
;  

: get_drv_mask ( drv -- mask_addr)
	2 * DRVMASKTAB + @
;

create mount_info_buf 1 14 + 14 +  allot

: show_drv_mount ( drv -- )
	mount_info_buf swap get_mount_info
	mount_info_buf 1 + buf80 14 move_z buf80 put_to_z
	[CHAR] / EMIT
	mount_info_buf 1 +  14 + buf80 14 move_z buf80 put_to_z
	."  (R/" mount_info_buf c@ 0= if [CHAR] W else [CHAR] O then emit ." )"
;

: show_drv_name ( drive -- )
	get_drv_mask c@
	case
		b# 0001 of 0 endof
		b# 0010 of 1 endof
		b# 0100 of 2 endof
		b# 1000 of 3 endof
		-1 swap
	endcase
	case
		-1 of ." INVALID value." endof
		dup ." Drive " [CHAR] A + emit
	endcase
;

: show_drv ( drv -- )
	dup [CHAR] A + EMIT ." : "
	dup get_drv_mask dup .h c@ dup .bin8 ."  - "
    ( drv mountmask )
    0= if
		show_drv_mount
	else
		show_drv_name 
	then
	cr
;


: z
	cr
	." EXTROM Diagnostic util" cr cr
	init_drv_mask_tab

	." SELDSK/FE HL=" DRVMASKTAB .h cr
	cr

	DRVMASKTAB 0= if
		." Current bios is incompatible with EXTDISK MOUNT" cr
		." BIOS SELDSK with C=FE return 0000" cr
	else
		0 show_drv 
		1 show_drv 
		2 show_drv 
		3 show_drv 
	then
;

: set_no_warm_boot
;

: tt 
 set_no_warm_boot
;

