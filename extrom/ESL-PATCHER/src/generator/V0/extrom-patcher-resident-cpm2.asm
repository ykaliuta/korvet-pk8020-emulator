	;резидент только для  12_87_11_niijaf
	;окно в EA06-EDFD = 1015  
	;
	
resident_cpm2	equ $

	.phase 0xea06

resident_cpm2_addr 	EQU 	$

	cpm_resident_body _cpm2_

	.dephase

resident_cpm2_len 		equ	$-resident_cpm2

; minimum hole_start .. min hole_end
max_resident_cpm2_len 	equ 	0xEDFD-0xEA06 ;=1015

available_resident_cpm2_len	equ 	max_resident_cpm2_len-resident_cpm2_len

	.assert max_resident_cpm2_len>=resident_cpm2_len
