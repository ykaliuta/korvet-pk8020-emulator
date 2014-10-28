_BIOS_CPM_21_89_2_niijaf:
	setSUBBIOS 1
	RQ_OPTS_ANY
	setCPM
	dbpatchmaybe	0xDAEE	,0x1F	,0x0d
	dbpatchmaybe	0xDAEE+2	,0x0a	,0x0d
	dbpatch	0xE48E+1	,0x49	,0xc9
	dbpatch	0xDA89 	,0x01	,0x00
	dbpatch	0xDAA0 	,0x02	,0x00
	dbpatch	0xDAB7 	,0x04	,0x01
	dbpatch	0xDACE 	,0x08	,0x02
	dwstore	_CPM1_res_DRIVE_TAB+0*2	,0xDA89
	dwstore	_CPM1_res_DRIVE_TAB+1*2	,0xDAA0
	dwstore	_CPM1_res_DRIVE_TAB+2*2	,0xDAB7
	dwstore	_CPM1_res_DRIVE_TAB+3*2	,0xDACE
	dwpatch 0xDCBB+5*2 	,0x0000 ,_CPM1_dph_F
	dwstore _CPM1_dph_F+8	,0xEBA6
	dwpatch 0xDA1B+1 	,0xDC7C	,_CPM1_res_seldsk
	dwstore	_CPM1_old_seld_dsk+1	,0xDC7C
	dbpatch	0xE6F9	,0x77	,0x00
	dwstore	_CPM1_r_p_SELDSKDRIVER+1	,0xE05D
	dwstore	_CPM1_r_p_SELDSKDRIVEW+1	,0xE05D
	dwpatch	0xDA27+1	,0xDCDA	,_CPM1_res_READ
	dwpatch	0xDA2A+1	,0xDE9E	,_CPM1_res_WRITE
	dwpatch	0xDB73+1	,0xDCDA	,_CPM1_res_READ
	dwpatch	0xdcae+1	,0xE637	,_CPM1_res_GETINFO
	dwstore	_CPM1__old_read+1	,0xDCDA
	dwstore	_CPM1__old_write+1	,0xDE9E
	dwstore	_CPM1__old_getinfo+1	,0xE637
	dbpatch	0xE66E		,0x21	,0x21
	dwpatch	0xE66E+1 	,0xE69E	,0xE69E
	dwstore	_CPM1__old_getinfo_chkdo+1	,0xE66E
	dwstore	_CPM1_r_p_DSK1+1	,0xE053 	;read
	dwstore	_CPM1_r_p_TRK1+1	,0xE057 	
	dwstore	_CPM1_r_p_SEC1+1	,0xE058 	
	dwstore	_CPM1_r_p_DMA1+1	,0xE059 	
	dwstore	_CPM1_r_p_DSK2+1	,0xE053 	;write
	dwstore	_CPM1_r_p_TRK2+1	,0xE057 	
	dwstore	_CPM1_r_p_SEC2+1	,0xE058 	
	dwstore	_CPM1_r_p_DMA2+1	,0xE059 	
	dwstore	_CPM1_r_p_DSK3+1	,0xE053 	;readinfo

	stop 'CPM_21_89_2_niijaf'
