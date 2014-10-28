_BIOS_CPM_12_87_09_NIIJAF:
	setSUBBIOS 1
	RQ_OPTS_ANY
	setCPM
	dbpatchmaybe	0xDAEE	,0x1F	,0x0d
	dbpatchmaybe	0xDAEE+2	,0x0a	,0x0d
	dbpatch	0xE738+1	,0x49	,0xc9
	dbpatch	0xDA88 	,0x01	,0x00
	dbpatch	0xDA9F 	,0x02	,0x00
	dbpatch	0xDAB6 	,0x04	,0x01
	dbpatch	0xDACD 	,0x08	,0x02
	dwstore	_CPM1_res_DRIVE_TAB+0*2	,0xDA88
	dwstore	_CPM1_res_DRIVE_TAB+1*2	,0xDA9F
	dwstore	_CPM1_res_DRIVE_TAB+2*2	,0xDAB6
	dwstore	_CPM1_res_DRIVE_TAB+3*2	,0xDACD
	dwpatch 0xDCFA+5*2 	,0x0000 ,_CPM1_dph_F
	dwstore _CPM1_dph_F+8	,0xDFDF
	dwpatch 0xDA1B+1 	,0xDCB6	,_CPM1_res_seldsk
	dwstore	_CPM1_old_seld_dsk+1	,0xDCB6
	dbpatch	0xE531	,0x77	,0x00
	dwstore	_CPM1_r_p_SELDSKDRIVER+1	,0xDFCD
	dwstore	_CPM1_r_p_SELDSKDRIVEW+1	,0xDFCD
	dwpatch	0xDA27+1	,0xDD19	,_CPM1_res_READ
	dwpatch	0xDA2A+1	,0xDE4F	,_CPM1_res_WRITE
	dwpatch	0xDB6D+1	,0xDD19	,_CPM1_res_READ
	dwpatch	0xDCE8+1	,0xE467	,_CPM1_res_GETINFO
	dwstore	_CPM1__old_read+1	,0xDD19
	dwstore	_CPM1__old_write+1	,0xDE4F
	dwstore	_CPM1__old_getinfo+1	,0xE467
	dbpatch	0xE4A7		,0x21	,0x21
	dwpatch	0xE4A7+1 	,0xE4D7	,0xE4D7
	dwstore	_CPM1__old_getinfo_chkdo+1	,0xE4A7
	dwstore	_CPM1_r_p_DSK1+1	,0xDFC3 	;read
	dwstore	_CPM1_r_p_TRK1+1	,0xDFC7 	
	dwstore	_CPM1_r_p_SEC1+1	,0xDFC8 	
	dwstore	_CPM1_r_p_DMA1+1	,0xDFC9 	
	dwstore	_CPM1_r_p_DSK2+1	,0xDFC3 	;write
	dwstore	_CPM1_r_p_TRK2+1	,0xDFC7 	
	dwstore	_CPM1_r_p_SEC2+1	,0xDFC8 	
	dwstore	_CPM1_r_p_DMA2+1	,0xDFC9 	
	dwstore	_CPM1_r_p_DSK3+1	,0xDFC3 	;readinfo

	stop 'CPM_12_87_09_NIIJAF'
