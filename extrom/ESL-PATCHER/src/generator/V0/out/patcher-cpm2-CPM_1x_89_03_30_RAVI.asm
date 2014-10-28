_BIOS_CPM_1x_89_03_30_RAVI:
	setSUBBIOS 2
	RQ_OPTS1
	setCPM
	dbpatchmaybe	0xDAEE	,0x1F	,0x0d
	dbpatchmaybe	0xDAEE+2	,0x0a	,0x0d
	dbpatch	0xE8B8+1	,0x49	,0xc9
	dbpatch	0xDA88 	,0x01	,0x00
	dbpatch	0xDA9F 	,0x02	,0x00
	dbpatch	0xDAB6 	,0x04	,0x01
	dbpatch	0xDACD 	,0x08	,0x02
	dwstore	_CPM2_res_DRIVE_TAB+0*2	,0xDA88
	dwstore	_CPM2_res_DRIVE_TAB+1*2	,0xDA9F
	dwstore	_CPM2_res_DRIVE_TAB+2*2	,0xDAB6
	dwstore	_CPM2_res_DRIVE_TAB+3*2	,0xDACD
	dwpatch 0xDEC6+5*2 	,0x0000 ,_CPM2_dph_F
	dwstore _CPM2_dph_F+8	,0xE1AB
	dwpatch 0xDA1B+1 	,0xDE82	,_CPM2_res_seldsk
	dwstore	_CPM2_old_seld_dsk+1	,0xDE82
	dbpatch	0xE6DF	,0x77	,0x00
	dwstore	_CPM2_r_p_SELDSKDRIVER+1	,0xE199
	dwstore	_CPM2_r_p_SELDSKDRIVEW+1	,0xE199
	dwpatch	0xDA27+1	,0xDEE5	,_CPM2_res_READ
	dwpatch	0xDA2A+1	,0xE01B	,_CPM2_res_WRITE
	dwpatch	0xDBC2+1	,0xDEE5	,_CPM2_res_READ
	dwpatch	0xdeb4+1	,0xE615	,_CPM2_res_GETINFO
	dwstore	_CPM2__old_read+1	,0xDEE5
	dwstore	_CPM2__old_write+1	,0xE01B
	dwstore	_CPM2__old_getinfo+1	,0xE615
	dbpatch	0xE655		,0x21	,0x21
	dwpatch	0xE655+1 	,0xE685	,0xE685
	dwstore	_CPM2__old_getinfo_chkdo+1	,0xE655
	dwstore	_CPM2_r_p_DSK1+1	,0xE18F 	;read
	dwstore	_CPM2_r_p_TRK1+1	,0xE193 	
	dwstore	_CPM2_r_p_SEC1+1	,0xE194 	
	dwstore	_CPM2_r_p_DMA1+1	,0xE195 	
	dwstore	_CPM2_r_p_DSK2+1	,0xE18F 	;write
	dwstore	_CPM2_r_p_TRK2+1	,0xE193 	
	dwstore	_CPM2_r_p_SEC2+1	,0xE194 	
	dwstore	_CPM2_r_p_DMA2+1	,0xE195 	
	dwstore	_CPM2_r_p_DSK3+1	,0xE18F 	;readinfo

	stop 'CPM_1x_89_03_30_RAVI'
