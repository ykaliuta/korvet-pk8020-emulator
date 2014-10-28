_BIOS_CPM_12_87_11_niijaf:
	setSUBBIOS 2
	RQ_OPTS_ANY
	setCPM
	dbpatchmaybe	0xDAEE	,0x1F	,0x0d
	dbpatchmaybe	0xDAEE+2	,0x0a	,0x0d
	dbpatch	0xE30E+1	,0x49	,0xc9
	dbpatch	0xDA88 	,0x01	,0x00
	dbpatch	0xDA9F 	,0x02	,0x00
	dbpatch	0xDAB6 	,0x04	,0x01
	dbpatch	0xDACD 	,0x08	,0x02
	dwstore	_CPM2_res_DRIVE_TAB+0*2	,0xDA88
	dwstore	_CPM2_res_DRIVE_TAB+1*2	,0xDA9F
	dwstore	_CPM2_res_DRIVE_TAB+2*2	,0xDAB6
	dwstore	_CPM2_res_DRIVE_TAB+3*2	,0xDACD
	dwpatch 0xDCF6+5*2 	,0x0000 ,_CPM2_dph_F
	dwstore _CPM2_dph_F+8	,0xDFDB
	dwpatch 0xDA1B+1 	,0xDCB2	,_CPM2_res_seldsk
	dwstore	_CPM2_old_seld_dsk+1	,0xDCB2
	dbpatch	0xE789	,0x77	,0x00
	dwstore	_CPM2_r_p_SELDSKDRIVER+1	,0xDFC9
	dwstore	_CPM2_r_p_SELDSKDRIVEW+1	,0xDFC9
	dwpatch	0xDA27+1	,0xDD15	,_CPM2_res_READ
	dwpatch	0xDA2A+1	,0xDE4B	,_CPM2_res_WRITE
	dwpatch	0xDB70+1	,0xDD15	,_CPM2_res_READ
	dwpatch	0xDCE4+1	,0xE6BF	,_CPM2_res_GETINFO
	dwstore	_CPM2__old_read+1	,0xDD15
	dwstore	_CPM2__old_write+1	,0xDE4B
	dwstore	_CPM2__old_getinfo+1	,0xE6BF
	dbpatch	0xE6FF		,0x21	,0x21
	dwpatch	0xE6FF+1 	,0xe72F	,0xe72F
	dwstore	_CPM2__old_getinfo_chkdo+1	,0xE6FF
	dwstore	_CPM2_r_p_DSK1+1	,0xDFBF 	;read
	dwstore	_CPM2_r_p_TRK1+1	,0xDFC3 	
	dwstore	_CPM2_r_p_SEC1+1	,0xDFC4 	
	dwstore	_CPM2_r_p_DMA1+1	,0xDFC5 	
	dwstore	_CPM2_r_p_DSK2+1	,0xDFBF 	;write
	dwstore	_CPM2_r_p_TRK2+1	,0xDFC3 	
	dwstore	_CPM2_r_p_SEC2+1	,0xDFC4 	
	dwstore	_CPM2_r_p_DMA2+1	,0xDFC5 	
	dwstore	_CPM2_r_p_DSK3+1	,0xDFBF 	;readinfo

	stop 'CPM_12_87_11_niijaf'
