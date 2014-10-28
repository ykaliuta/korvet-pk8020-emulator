_BIOS_CPM_21_91___LAP:
	setSUBBIOS 1
	RQ_OPTS_ANY
	setCPM
	dbpatchmaybe	0xDAEE	,0x1F	,0x0d
	dbpatchmaybe	0xDAEE+2	,0x0a	,0x0d
	dbpatch	0xE0D2+1	,0x49	,0xc9
	dbpatch	0xDA88 	,0x01	,0x00
	dbpatch	0xDA9F 	,0x02	,0x00
	dbpatch	0xDAB6 	,0x04	,0x01
	dbpatch	0xDACD 	,0x08	,0x02
	dwstore	_CPM1_res_DRIVE_TAB+0*2	,0xDA88
	dwstore	_CPM1_res_DRIVE_TAB+1*2	,0xDA9F
	dwstore	_CPM1_res_DRIVE_TAB+2*2	,0xDAB6
	dwstore	_CPM1_res_DRIVE_TAB+3*2	,0xDACD
	dwpatch 0xDCF2+5*2 	,0x0000 ,_CPM1_dph_F
	dwstore _CPM1_dph_F+8	,0xEBA6
	dwpatch 0xDA1B+1 	,0xDCAE	,_CPM1_res_seldsk
	dwstore	_CPM1_old_seld_dsk+1	,0xDCAE
	dbpatch	0xE69F	,0x77	,0x00
	dwstore	_CPM1_r_p_SELDSKDRIVER+1	,0xDFED
	dwstore	_CPM1_r_p_SELDSKDRIVEW+1	,0xDFED
	dwpatch	0xDA27+1	,0xDD11	,_CPM1_res_READ
	dwpatch	0xDA2A+1	,0xDE47	,_CPM1_res_WRITE
	dwpatch	0xDB5F+1	,0xDD11	,_CPM1_res_READ
	dwpatch	0xdce0+1	,0xE5D5	,_CPM1_res_GETINFO
	dwstore	_CPM1__old_read+1	,0xDD11
	dwstore	_CPM1__old_write+1	,0xDE47
	dwstore	_CPM1__old_getinfo+1	,0xE5D5
	dbpatch	0xE615		,0x21	,0x21
	dwpatch	0xE615+1 	,0xE645	,0xE645
	dwstore	_CPM1__old_getinfo_chkdo+1	,0xE615
	dwstore	_CPM1_r_p_DSK1+1	,0xDFE3 	;read
	dwstore	_CPM1_r_p_TRK1+1	,0xDFE7 	
	dwstore	_CPM1_r_p_SEC1+1	,0xDFE8 	
	dwstore	_CPM1_r_p_DMA1+1	,0xDFE9 	
	dwstore	_CPM1_r_p_DSK2+1	,0xDFE3 	;write
	dwstore	_CPM1_r_p_TRK2+1	,0xDFE7 	
	dwstore	_CPM1_r_p_SEC2+1	,0xDFE8 	
	dwstore	_CPM1_r_p_DMA2+1	,0xDFE9 	
	dwstore	_CPM1_r_p_DSK3+1	,0xDFE3 	;readinfo

	stop 'CPM_21_91___LAP'
