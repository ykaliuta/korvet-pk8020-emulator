_BIOS_CPM_NET_KORNET_drive_b:
	setUNSUPPORTED
	;custom checkers
	dwpatch	0x8000  	,0x00EE  ,0x00EE 
	dwpatch	0x8000+2	,0x0000 ,0x0000
	dwpatch	0x8000+4	,0x000E ,0x000E
	stop 'CPM_NET_KORNET_drive_b'
