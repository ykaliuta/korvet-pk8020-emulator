_BIOS_CPM_12_90_5_kontur:
	setSUBBIOS 1
	RQ_OPTS_ANY
	setCPM
	dbpatchmaybe	0xDAEF	,0x1F	,0x0d
	dbpatchmaybe	0xDAEF+2	,0x0a	,0x0d
	dbpatch	0xE09B+1	,0x49	,0xc9
	dbpatch	0xDA88 	,0x01	,0x00
	dbpatch	0xDA9F 	,0x02	,0x00
	dbpatch	0xDAB6 	,0x04	,0x01
	dbpatch	0xDACD 	,0x08	,0x02
	dwstore	_CPM1_res_DRIVE_TAB+0*2	,0xDA88
	dwstore	_CPM1_res_DRIVE_TAB+1*2	,0xDA9F
	dwstore	_CPM1_res_DRIVE_TAB+2*2	,0xDAB6
	dwstore	_CPM1_res_DRIVE_TAB+3*2	,0xDACD
	dwpatch 0xDCB6+5*2 	,0x0000 ,_CPM1_dph_F
	dwstore _CPM1_dph_F+8	,0xEBA6
	dwpatch 0xDA1B+1 	,0xDC72	,_CPM1_res_seldsk
	dwstore	_CPM1_old_seld_dsk+1	,0xDC72
	dbpatch	0xE6C0	,0x77	,0x00
	dwstore	_CPM1_r_p_SELDSKDRIVER+1	,0xDFB6
	dwstore	_CPM1_r_p_SELDSKDRIVEW+1	,0xDFB6
	dwpatch	0xDA27+1	,0xDCD5	,_CPM1_res_READ
	dwpatch	0xDA2A+1	,0xDE22	,_CPM1_res_WRITE
	dwpatch	0xDB6B+1	,0xDCD5	,_CPM1_res_READ
	dwpatch	0xdca4+1	,0xE5F6	,_CPM1_res_GETINFO
	dwstore	_CPM1__old_read+1	,0xDCD5
	dwstore	_CPM1__old_write+1	,0xDE22
	dwstore	_CPM1__old_getinfo+1	,0xE5F6
	dbpatch	0xE636		,0x21	,0x21
	dwpatch	0xE636+1 	,0xE666	,0xE666
	dwstore	_CPM1__old_getinfo_chkdo+1	,0xE636
	dwstore	_CPM1_r_p_DSK1+1	,0xDFAC 	;read
	dwstore	_CPM1_r_p_TRK1+1	,0xDFB0 	
	dwstore	_CPM1_r_p_SEC1+1	,0xDFB1 	
	dwstore	_CPM1_r_p_DMA1+1	,0xDFB2 	
	dwstore	_CPM1_r_p_DSK2+1	,0xDFAC 	;write
	dwstore	_CPM1_r_p_TRK2+1	,0xDFB0 	
	dwstore	_CPM1_r_p_SEC2+1	,0xDFB1 	
	dwstore	_CPM1_r_p_DMA2+1	,0xDFB2 	
	dwstore	_CPM1_r_p_DSK3+1	,0xDFAC 	;readinfo

	stop 'CPM_12_90_5_kontur'
