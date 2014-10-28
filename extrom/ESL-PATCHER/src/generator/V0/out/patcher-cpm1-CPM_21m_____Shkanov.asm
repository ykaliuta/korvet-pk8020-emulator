_BIOS_CPM_21m_____Shkanov:
	setSUBBIOS 1
	RQ_OPTS_ANY
	setCPM
	dbpatchmaybe	0xDAEE	,0x1F	,0x0d
	dbpatchmaybe	0xDAEE+2	,0x0a	,0x0d
	dbpatch	0xE135+1	,0x49	,0xc9
	dbpatch	0xDA89 	,0x01	,0x00
	dbpatch	0xDAA0 	,0x02	,0x00
	dbpatch	0xDAB7 	,0x04	,0x01
	dbpatch	0xDACE 	,0x08	,0x02
	dwstore	_CPM1_res_DRIVE_TAB+0*2	,0xDA89
	dwstore	_CPM1_res_DRIVE_TAB+1*2	,0xDAA0
	dwstore	_CPM1_res_DRIVE_TAB+2*2	,0xDAB7
	dwstore	_CPM1_res_DRIVE_TAB+3*2	,0xDACE
	dwpatch 0xDCB5+5*2 	,0x0000 ,_CPM1_dph_F
	dwstore _CPM1_dph_F+8	,0xEBA6
	dwpatch 0xDA1B+1 	,0xDC73	,_CPM1_res_seldsk
	dwstore	_CPM1_old_seld_dsk+1	,0xDC73
	dbpatch	0xE6FE	,0x77	,0x00
	dwstore	_CPM1_r_p_SELDSKDRIVER+1	,0xE058
	dwstore	_CPM1_r_p_SELDSKDRIVEW+1	,0xE058
	dwpatch	0xDA27+1	,0xDCD5	,_CPM1_res_READ
	dwpatch	0xDA2A+1	,0xDE99	,_CPM1_res_WRITE
	dwpatch	0xDB6A+1	,0xDCD5	,_CPM1_res_READ
	dwpatch	0xdca8+1	,0xE632	,_CPM1_res_GETINFO
	dwstore	_CPM1__old_read+1	,0xDCD5
	dwstore	_CPM1__old_write+1	,0xDE99
	dwstore	_CPM1__old_getinfo+1	,0xE632
	dbpatch	0xE669		,0x21	,0x21
	dwpatch	0xE669+1 	,0xE6A3	,0xE6A3
	dwstore	_CPM1__old_getinfo_chkdo+1	,0xE669
	dwstore	_CPM1_r_p_DSK1+1	,0xE04E 	;read
	dwstore	_CPM1_r_p_TRK1+1	,0xE052 	
	dwstore	_CPM1_r_p_SEC1+1	,0xE053 	
	dwstore	_CPM1_r_p_DMA1+1	,0xE054 	
	dwstore	_CPM1_r_p_DSK2+1	,0xE04E 	;write
	dwstore	_CPM1_r_p_TRK2+1	,0xE052 	
	dwstore	_CPM1_r_p_SEC2+1	,0xE053 	
	dwstore	_CPM1_r_p_DMA2+1	,0xE054 	
	dwstore	_CPM1_r_p_DSK3+1	,0xE04E 	;readinfo

	stop 'CPM_21m_____Shkanov'
