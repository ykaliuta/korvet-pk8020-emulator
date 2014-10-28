_BIOS_:NAME::
	
	setMICRODOS
	:ROMCHECK:

	;BIOS_LOAD_ADDR-------------------------------------------------------------------
	;BIOS_START_ADDR
	;правилнее проверять прямо в 8000 ????
	;копия бутсектора из 8000, 
	;
	;адресс загрузки, куда биос загружает сектора с диска
	;	dwpatch	0xBD80,	0xBD80, 0xBD80
	;адресс старта, куда биос передаёт управление
	;	dwpatch	0xBD82,	0xBE00,	0xBE00

	
	;BDOS_ENTRY_POINT---------------------------------------------------------
	;BDOS_ENTRY_POINT2
	;значение перехода, проверяим их для надёжности
	;0005 jp c000
	;RAM:C000          _BDOS:                                  
	;RAM:C000                                                  
	;RAM:C000 C3 87 C4                 jp      loc_C487
	;RAM:C003 C3 C0 DB                 jp      loc_DBC0
	; 	dwpatch	0xc000+1,0xC487,0xC487
	; 	dwpatch	0xc003+1,0xDBC0,0xDBC0

	;BIOS_LOGO_ADDR--------------------------------------------------------------
	;косметика,  
	;Биос при старте очищает экран, мы патчим его чтобы он это не делал
	;иначе он затрёт все сообщения патчера, а там много полезной информации
	;выгядит это в коде так
	;это значение там только во время старта, дальше там буфер .....

	; RAM:F04E 1B 45 EB+BOOT_LOGO:      text "koi8-r", '\\x1BКОРВЕТ ПК8020\\r\\n'
	; RAM:F04E E5 F4 20+                text "koi8-r", 'НИИСЧЁТМАШ 01.05.90\\r\\n'
	; RAM:F04E F0 EB 38+                text "koi8-r", '$'
	dbpatchmaybe	:BIOS_LOGO_ADDR:	,0x1B	,0x0d
	dbpatchmaybe	:BIOS_LOGO_ADDR:+1	,0x45	,0x0d

	;JP_READ-----------------------------------------------------------------------
	;JP_WRITE
	; перехватываем обращение к чтению/записи в биос
	; подменяем адресса для чтения/записи
	; собственно во всех это куче кода нас интересует адресса EAFF и EB04
	
	; точки входа обычного биоса
	; RAM:C027 C3 7B DD                 jp      do_READ
	; RAM:C02A C3 78 DD                 jp      do_WRITE
	;.....
	; RAM:DD78          do_WRITE:                               
	; RAM:DD78 3E 06                    ld      a, 6
	; RAM:DD7A 21                       db 21h
	; RAM:DD7B          do_READ:                                
	; RAM:DD7B                                                  
	; RAM:DD7B 3E 04                    ld      a, 4
	; RAM:DD7D 32 1D DF                 ld      (dsk_description.Fucntion), a
	; RAM:DD80 21 1B DF                 ld      hl, dsk_description
	; RAM:DD83 C3 12 E4                 jp      RW_DISK
	; ...
	; RAM:E412          RW_DISK:                                
	; RAM:E412                                                  
	; RAM:E412 C3 A5 EA                 jp      j_DSKIO

	; RAM:EAA5          j_DSKIO:                                
	; RAM:EAA5 3A 03 F7                 ld      a, (SysCOPY)
	; RAM:EAA8 F5                       push    af
	; RAM:EAA9 EB                       ex      de, hl
	; RAM:EAAA 06 5C                    ld      b, 5Ch ; '\'
	; RAM:EAAC CD 9F E4                 call    setSYSREG
	; RAM:EAAF EB                       ex      de, hl
	; RAM:EAB0 CD BF EA                 call    do_DSKIO
	; RAM:EAB3 47                       ld      b, a
	; RAM:EAB4 F1                       pop     af
	; RAM:EAB5 F3                       di
	; RAM:EAB6 32 7F FF                 ld      (_5C_SysReg1C), a
	; RAM:EAB9 32 03 F7                 ld      (SysCOPY), a
	; RAM:EABC FB                       ei
	; RAM:EABD 78                       ld      a, b
	; RAM:EABE C9                       ret
	; ....
	; RAM:EABF          do_DSKIO:                               
	; RAM:EABF 79                       ld      a, c
	; RAM:EAC0 32 F7 EE                 ld      (DSK_IO_WriteType), a ; 0       (normal sector write)
	; RAM:EAC0                                                  ; 1       (write to directory sector)
	; RAM:EAC0                                                  ; 2       (write to the first sector of a new data block)
	; RAM:EAC3 22 F8 EE                 ld      (DSC_IO_HL), hl ; DSCDESCR
	; RAM:EAC6          ;
	; RAM:EAC6 3A F6 EE                 ld      a, (E_DRIVE_FLAG)
	; RAM:EAC9 A6                       and     (hl)            ; DRIVE?
	; RAM:EACA FE 04                    cp      4
	; RAM:EACC CA 80 FC                 jp      z, E_DRIVE
	; RAM:EACF          ;
	; RAM:EACF 7E                       ld      a, (hl)
	; RAM:EAD0 FE 02                    cp      2
	; RAM:EAD2 D2 07 EB                 jp      nc, invalidDrive
	; RAM:EAD5          ;
	; ....
	; RAM:EAFA 22 FA EE                 ld      (_DMA??), hl
	; RAM:EAFD FE 04                    cp      4
	; RAM:EAFF CA 36 EB                 jp      z, jREAD
	; RAM:EB02 FE 06                    cp      6
	; RAM:EB04 CA A0 EB                 jp      z, jWRITE
	; RAM:EB07
	; RAM:EB07          invalidDrive:                           
	; RAM:EB07                                                  
	; RAM:EB07 3E FF                    ld      a, 0FFh
	; RAM:EB09 C9                       ret

	; а так как пока в резиденте нет поддержки работы с реальными дисками то 
	; адресса оригинальных пока не сохраняем к себе ....
	; TODO: microdos real disk support

	dwpatch	:JP_READ_ADDR:+1	,:JP_READ_VALUE:	,MD_READ			
	dwpatch	:JP_WRITE_ADDR:+1	,:JP_WRITE_VALUE:	,MD_WRITE

	;GET_INFO_FDC_SEEK -------------------------------------------------------------
	;GET_INFO_CALL_READ
	;кусок GET_INFO_C, подменяем чтение инфосектора
	; RAM:EC56          GET_INFO_C:                             ; CODE XREF: jGetDPH_Addr+29p
	; RAM:EC56 79                       ld      a, c            ; drive
	; RAM:EC57 B7                       or      a
	; RAM:EC58 3A 93 E6                 ld      a, (dsk_info_a.intiFlag)
	; RAM:EC5B CA 61 EC                 jp      z, loc_EC61
	; RAM:EC61
	;....
	; RAM:EC86 2B                       dec     hl
	; RAM:EC87 CD 14 EE                 call    SEL_DSK_SEEK0
	; RAM:EC8A CD 1E EE                 call    ReadToRDBUF
	; RAM:EC8D          ;
	; RAM:EC8D 3E 04                    ld      a, 4
	; RAM:EC8F 32 FF EE                 ld      (DSK_TRY_Count), a

	; для начала удаляем комманду которая готовит чтение с физичесского дисковода
	dbpatch	:GET_INFO_FDC_SEEK_ADDR:	,0xcd	,0x00
	dwpatch	:GET_INFO_FDC_SEEK_ADDR:+1	,:GET_INFO_FDC_SEEK2:	,0x0000

	;GET_INFO_CALL_READ------------------------------------------------------
	;подменяем чтение сектора с диска на нашу функцию
	dwpatch	:GET_INFO_CALL_READ_ADDR:+1	,:GET_INFO_CALL_READ_VALUE:	,MD_READ_INFOSECTOR

	;DISKINFO_DRIVE--------------------------------------------------------------------------------
	;эти два адресса нужны для DISKINFO
	;rdBufDiskInfo - это очередной блок параметров, по нужному адрессу там текущий диск для операции DISKINFO
	; в примере ниже чтение нашего байта по адрессу 0xED82
	; а собственно адресс по адрессу 0xEB7E (значение 0xEF06)
	; RAM:EB36          jREAD:                                  
	; RAM:EB36 2A F8 EE                 ld      hl, (DSC_IO_HL) 
	; ...
	; RAM:EB7E          ;
	; RAM:EB7E 21 06 EF                 ld      hl, rdBufDiskInfo
	; RAM:EB81 CD 7D ED                 call    FDD_Setup
	; ...
	; RAM:ED7D          FDD_Setup:                              
	; RAM:ED7D                                                  
	; RAM:ED7D 3E D0                    ld      a, FDC_3_FORCE_STOP
	; RAM:ED7F 32 18 FE                 ld      (_5C_FDC_Cmd), a
	; RAM:ED82          ;
	; RAM:ED82 7E                       ld      a, (hl)         ; drive
	; RAM:ED83 B7                       or      a
	; RAM:ED84 06 01                    ld      b, 1
	; RAM:ED86 CA 8B ED                 jp      z, loc_ED8B

	dwstore	MD_DRV2+1,	:DISKINFO_DRIVE:

	;DISKINFO_READ_BUFFER--------------------------------------------------------------------------
	;BUF_RD_1k
	;адресс буфера кудв мы читаем 0й сектор а дальше уже старый код в биосе
	;делает всё остальное
	; тут он присутсвует по адрессу 0xEC97 (значение 0xF04E)
	; собственно это подсчёт Контрольной суммы
	; RAM:EC56          GET_INFO_C:                             
	; RAM:EC56 79                       ld      a, c            
	; RAM:EC57 B7                       or      a
	; RAM:EC58 3A 93 E6                 ld      a, (dsk_info_a.intiFlag)
	; RAM:EC5B CA 61 EC                 jp      z, loc_EC61
	; RAM:EC5E 3A 97 E6                 ld      a, (dsk_info_b.intiFlag)
	; RAM:EC61
	; RAM:EC61          loc_EC61:                               
	; RAM:EC61 B7                       or      a
	; RAM:EC62 C2 EC EC                 jp      nz, diskAlreadyInit
	; RAM:EC65          ;
	; ...
	; RAM:EC97          ;
	; RAM:EC97 21 4E F0                 ld      hl, BUF_RD_1k
	; RAM:EC9A 0E 1F                    ld      c, 1Fh
	; RAM:EC9C 3E 66                    ld      a, 66h ; 'f'
	; RAM:EC9E
	; RAM:EC9E          CalcCRC_Loop:                           
	; RAM:EC9E 86                       add     a, (hl)
	; RAM:EC9F 23                       inc     hl
	; RAM:ECA0 0D                       dec     c
	; RAM:ECA1 C2 9E EC                 jp      nz, CalcCRC_Loop
	; RAM:ECA4 BE                       cp      (hl)
	; RAM:ECA5          ;
	; RAM:ECA5 3E 02                    ld      a, 2            ; CRC Error
	; RAM:ECA7 C0                       ret     nz

	dwstore	MD_RDBUF2+1,	:DISKINFO_READ_BUFFER:
	
	;DISK_IO_PARAM_BLOCK_ADDR------------------------------------------------------------------------
	;адресс таблички с параметрами дисководой операции
	;dsk,trk,sec,dma берутся тут
	;DSC_IO_HL
	dwstore	MD_PARAM2+1,	:DISK_IO_PARAM_BLOCK_ADDR:

	;DISABLE_DISK_CHECK------------------------------------------------------------------------------
	;убираем проверку в биосе на наличие дисководов
	; RAM:BE00          _BOOT:                                  
	; RAM:BE00                                                  
	; RAM:BE00 F3                       di
	; RAM:BE01 31 00 BE                 ld      sp, 0BE00h
	; RAM:BE04 3A 01 F7                 ld      a, (FDDFLAG)
	; RAM:BE07 E6 02                    and     2
	; RAM:BE09 C2 40 00                 jp      nz, R_RunBasic??	
	;dbpatch 0xbe07+1	,0x02	,0x00
	
	;FLUSH_WRITE_ADDR-------------------------------------------------------------------------------
	; FLUSH нам не нужен, т.к. наша запись идёт без буферезации.
	; TODO: JP invalidateWRBUF ???
	; RAM:EC3A          Flush?:                                 
	; RAM:EC3A                                                  
	; RAM:EC3A 21 FD EE                 ld      hl, flagWriteReuired?
	; RAM:EC3D 7E                       ld      a, (hl)
	; RAM:EC3E B7                       or      a
	; RAM:EC3F C8                       ret     z
	; RAM:EC40          ;
	; RAM:EC40 36 00                    ld      (hl), 0
	; RAM:EC42          ;
	; RAM:EC42 21 0A EF                 ld      hl, WrBufDiskInfo
	; RAM:EC45 CD 7D ED                 call    FDD_Setup
	; RAM:EC48          ;
	; RAM:EC48 CD 66 EE                 call    FDD_Write
	; RAM:EC4B C8                       ret     z
	; RAM:EC4C          invalidateWRBUF:                        
	; RAM:EC4C                                                  
	; RAM:EC4C 21 0A EF                 ld      hl, WrBufDiskInfo
	; RAM:EC4F 36 FF                    ld      (hl), 0FFh
	; RAM:EC51 C9                       ret	
	;dbpatch 0xEC42		,0x21	,0xC9

	;SELDSK_ADDR------------------------------------------------------------
	;и модифицируем вызов SELDSK чтобы он поддерживал дополнительную проверку
	;модифицируем оригинальный перехо в SELDSK чтобы он переходил на наш код	
	;и сохраняем старый адресс (в данном коде нужный нам адресс 0xDCB6)
	;RAM:DA1B          _SELDSK:                         ; IN: C
	;RAM:DA1B C3 B6 DC          jp      __SELDSK        ; OUT: HL-DPB/0, HL=xVTRAP0 if c=0xff

	dwpatch 0xc01b+1	 ,:SELDSK_ADDR:	,md_res_seldsk
	dwstore md_old_seld_dsk+1,:SELDSK_ADDR:	,0xdd4c

:CUSTOM_CHECKERS:

	;STOP-------------------------------------------------------------------------------
	stop ':NAME:'
	;'MICRODOS_2_900105'
