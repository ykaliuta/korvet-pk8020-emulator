_BIOS_:NAME::
	setSUBBIOS 1

	;адресса актуальные для биоса 21_89___wiza

	;BIOS_LOGO 		=> "0xDAEE",
	;PPI2C_BIOS_IIT 	=> "0xE49F",
	;DRVREG_A 		=> "0xDAA0",
	;DRVREG_B 		=> "0xDA90",
	;DRVREG_C 		=> "0xDAB7",
	;DRVREG_D 		=> "0xDACE",
	;SELDSK_STOREDRV	=> "0xE70A",
	;SELDSK_ADDR		=> "0xDC8D",
	;CURRENT_DISKPTR   	=> "0xE06E",
	;BIOS_READ 		=> "0xDCEB",
	;BIOS_WRITE 		=> "0xDEAF",
	;SELDSK_GETINO_CALL	=> "0xdcbf",
	;BIOS_GETINFO		=> "0xE648",
	;WBOOT_READ_CLL 	=> "0xDB84",
	;GETINFO_POSTTOM_ADDR 	=> "0xE67F",
	;GETINFO_POSTTOM_CHKW 	=> "0xE6AF",
	;BIOS_VAR_DSK		=> "0xE064",
	;BIOS_VAR_SEC		=> "0xE068",
	;BIOS_VAR_TRK		=> "0xE069",
	;BIOS_VAR_DMA		=> "0xE06A",
	;VINTKBDADDR 		=> "0xe3e3",
	;VINTKBDVALUE		=> "0xdc46",
	;KBD_HOOK_PRO 		=> "kbd_hook_2133",
	;NAME 			=> '21_89___wiza',

	;Чекер/Патчер
	;есть три основных комманды
	;dbpatch addr oldbyte newbyte
	;dwpatch addr oldword newword
	;dwstore addr newword
	;и ряд вспомогательных
	;setCPM
	;setMICRODOS
	;checkOPTS1
	;checkOPTS2
	;unsupported
	;pSTOP 'biosname'

	;на первом этапе патчер проверяет что в памяти именно этот биос
	;для этого он проходтся по d(b|w)patch и проверяет 
	;что по адрессу addr записано значени old(byte|word)

	;если это не так (т.е. на первой же проверке которая не прошла)
	;то считаем что биос не соответсвует проверяемому и надо пробовать следующий

	;если же все проверки прошли - то запускается второй проход
	;правим значения в памяти (собственно patch)
	;т.е для d(b|w)patch по addr записывается значение new(byte|word)
	;для dwstore просто по адрессу addr записывается newword

	;если old = new - то это просто проверка, т.к. значение не изменяется

	:ROMCHECK:

	;
	;ниже описан "TEMPLATE" для поддержки CP/M BIOS
	;все примеры кода ниже взяты для биоса '21_89___wiza'
	;
	;я попробую возле каждого параметра описать откуда он получен
	;если вдруг будет необходимость расширить набор поддерживаемых
	;bios.
	;на сегодня поддерживается 13 уникальных CP/M bios для Корвета
	;под "уникальными" понимается разные биосы, в которых отличается код
	;во времена когда использовали Корвет было нормально править биос прямо на дискете
	;обычно правили функциональные клавиши, таблицы перекодировки принтера
	;и надписи ;)
	;по этой причине разно выглядящие биосы могут быть одинаковыми внутри
	;предварительный анализ позволил выделить 13 уникальных биосов среди сотен дискет
	;что сильно упростило дальнейшую работу.
	;
	;patcher же делает необходимое кол-во проверок и изменений биоса
	;чтобы подготовить его к работе с EXTROM API
	;
	;значения вида :NAME: должны быть заменены на значения
	;соотвествующие конкретному биосу

	;Lets go

	;установим тип системы CP/M (нужно для случая когда биос опознан но не может быть использован)
	;в случае с cp/m это важно для '1x_88_EPSON_V104'	'1x_89_03_30_RAVI'
	;они работают только с ОПТС 1
	;и по этой причине на ОПТС 2 надо fallback to default CP/M
	setCPM

	;BIOS_LOGO--------------------------------------------------------------
	;косметика,  
	;Биос при старте очищает экран, мы патчим его чтобы он это не делал
	;иначе он затрёт все сообщения патчера, а там много полезной информации
	;выгядит это в коде так
	
	;RAM:DA00          _BOOT:
	;RAM:DA00 C3 42 DB                 jp      j_BOOT
	;....
	;RAM:DB42          j_BOOT:                                 
	;RAM:DB42 F3                       di
	;RAM:DB43 31 80 00                 ld      sp, 80h 
	;RAM:DB46 3E 1C                    ld      a, 1Ch
	;RAM:DB48 32 7F FA                 ld      (_1C_SysReg1C), a
	;...
	;RAM:DB58 21 EE DA                 ld      hl, BIOS_LOGO   ; "\\x1F\\r\\nCP/M-80  vers. 2.2"
	;RAM:DB5B CD 2D E0                 call    putstr
	;...
	;RAM:DAEE 1F 0D 0A+BIOS_LOGO:      text "KOI8-R", '\\x1F\\r\\n'
	;RAM:DAEE 43 50 2F+                text "KOI8-R", 'CP/M-80  vers. 2.2 \\r\\n'
	;RAM:DAEE 4D 2D 38+                text "KOI8-R", 'BIOS  vers.2.1 (c) \\r\\n'
	;RAM:DAEE 30 20 20+                text "KOI8-R", 'МГУ н/п кооп."ВИЗА"\\r\\n'
	;RAM:DAEE 76 65 72+                text "KOI8-R", '   МОСКВА 1989 \\r\\n'
	;RAM:DAEE 73 2E 20+                text "KOI8-R", 0
	dbpatchmaybe	:_CPM1_BIOS_LOGO:	,0x1F	,0x0d
	dbpatchmaybe	:_CPM1_BIOS_LOGO:+2	,0x0a	,0x0d

	;PPI2C_BIOS_INIT--------------------------------------------------------
	;Биос при старте производит инициализацию перифирии, разных контроллеров
	;в том числе записывает значение 0x49 в порт PPI2.C 
	;(разрешаем вывод звука, ~SE, ResetForJoy)
	;для нас важно что тут он сбрабсывает PPI2.C.8 (ENABLE EXTROM)
	;без которого наш контроллер не работает.
	;мы модифицируем это значение чтобы бит8 был установлен 0x49 -> 0xC9 
	;выгядит это в коде так
	;нас интересует адресс E49F

	;RAM:DA00          _BOOT:
	;RAM:DA00 C3 42 DB                 jp      j_BOOT
	;....
	;RAM:DB42          j_BOOT:                                 
	;RAM:DB42 F3                       di
	;RAM:DB43 31 80 00                 ld      sp, 80h ; 'Ç'
	;RAM:DB46 3E 1C                    ld      a, 1Ch
	;RAM:DB48 32 7F FA                 ld      (_1C_SysReg1C), a
	;RAM:DB4E CD 0A E4                 call    HW_Init
	;...
	;RAM:E40A          HW_Init:                                
	;RAM:E40A                                                  
	;RAM:E40A F3                       di
	;RAM:E40B 11 D1 E3                 ld      de, IntstINTAB
	;RAM:E40E 21 C6 F7                 ld      hl, _xVTRAP
	;RAM:E411 01 3A 00                 ld      bc, 58
	;RAM:E414 CD 3A E0                 call    _LDIR
	;...
	;RAM:E495 3E FF                    ld      a, 0FFh
	;RAM:E497 32 31 F7                 ld      (SndFlag), a    ; flag key click (0 - no click)
	;RAM:E49A 3E 02                    ld      a, 2
	;RAM:E49C 32 2E F7                 ld      (F_Alft), a     ; 0 - russian
	;RAM:E49F 3E 49                    ld      a, 49h ; 'I'
	;RAM:E4A1 32 32 FB                 ld      (_1C_PPI2C_), a
	;RAM:E4A4 C9                       ret
	dbpatch	:_CPM1_PPI2C_BIOS_INIT:+1	,0x49	,0xc9

	;DRVREG_x---------------------------------------------------------------
	;DRVREG_A,DRVREG_B,DRVREG_C,DRVREG_D
	;драйверу дисковода необходимо знать какой логичесской букве соответствует какой диск
	;для этой цели мы используем байт который в оригинальном CP/M BIOS указывает
	;какое значение надо записать в DRVREG чтобы выбрать данный диск
	;это даёт возможность использовать и эмулятор и настоящие диски.
	;в нашем биосе если этот байт =0 то этот диск работает через EXTROM API
	;если же там не 0 то это физичесский диск.
	;эти значения также использует утилита MOUNT которая позволяет изменить это соответсвие
	;и подключить нужный образ диска к нужному дисководу.
	;по умолчанию 
	;при наличии дисковода
	;A: -> эмулятор образ disk/DISKA.KDI
	;B: -> эмулятор образ disk/DISKB.KDI
	;C: -> физичесский дисковод A:
	;D: -> физичесский дисковод B:
	;или при отсутствии дисковода
	;A: -> эмулятор образ disk/DISKA.KDI
	;B: -> эмулятор образ disk/DISKB.KDI
	;C: -> эмулятор образ disk/DISKC.KDI
	;D: -> эмулятор образ disk/DISKD.KDI

	;этот адресс можно найти так (это справедливо для всех известных CP/M биосов)
	;для каждого диска в биосе есть таблица DPH (Disk Parameter Header)
	;в этой таблице по смещению +0x0A находится адрес DPB (Disk Parameter Block)
	;интересующий нас байт находится по адрессу DPH-2
	;звучит сложно, но на практике всё просто.
	;найти DPH для всех дисков можно достаточно просто
	;есть вызов биоса SELDSK (0xDA1B)
	;иначе он попытается сделает операцию "Выбор диска" для указанного в регистре C диска
	;а для этого ему надо получить параметры диска для своих целей
	;а мы этим и воспользуемся
	;в коде ниже по адрессу 0xDC99
	;собственно тут нас интересует ссылка на таблицу DPH_TABLE
	;в которой и есть ссылки на DPH для дисков (0xDCCC)
	;A-DA33,B-DA43,etc
	;RAM:DC8D          j_SELDSK:                               
	;RAM:DC8D 79                       ld      a, c
	;RAM:DC8E FE FF                    cp      0FFh
	;RAM:DC90 21 C8 F7                 ld      hl, _xVTRAP0_EXT
	;RAM:DC93 C8                       ret     z
	;RAM:DC94 FE 08                    cp      8
	;RAM:DC96 D2 C4 DC                 jp      nc, loc_DCC4
	;RAM:DC99 21 CC DC                 ld      hl, DPH_TABLE
	;RAM:DC9C 32 64 E0                 ld      (_DSK), a
	;....
	;RAM:DCCC 33 DA    DPH_TABLE:      dw dph_a                ; DATA XREF: j_SELDSK+Co
	;RAM:DCCE 43 DA                    dw dph_b
	;RAM:DCD0 53 DA                    dw dph_c
	;RAM:DCD2 63 DA                    dw dph_d
	;RAM:DCD4 73 DA                    dw dph_e
	;RAM:DCD6 00 00                    dw loc_0
	;RAM:DCD8 00 00                    dw loc_0
	;RAM:DCDA 00 00                    dw loc_0
	;...

	;DPH             struc ; (sizeof=0x10)
	;_XLT:           dw ?
	;_1:             dw ?
	;_2:             dw ?
	;_3:             dw ?
	;DirBuf:         dw ?                    ; offset (00000000)
	;DPB:            dw ?                    ; offset (00000000)
	;CSV:            dw ?                    ; offset (00000000)
	;ALV:            dw ?                    ; offset (00000000)
	;DPH             ends

	;RAM:DA33 00 00 03+dph_a:          DPH <0, 3, 0, 0, dirBUF, dpbA, csvA, alwA>
	;RAM:DA43 00 00 00+dph_b:          DPH <0, 0, 0, 0, dirBUF, dpbB, csvB, alwB>
	;RAM:DA53 00 00 00+dph_c:          DPH <0, 0, 0, 0, dirBUF, dpbC, csvC, alwC>
	;RAM:DA63 00 00 00+dph_d:          DPH <0, 0, 0, 0, dirBUF, dpbD, csvD, alwD>
	;RAM:DA73 00 00 00+dph_e:          DPH <0, 0, 0, 0, dirBUF, dpbE, loc_0, alwE>
	;RAM:DA83 50                       db  50h ; P
	;RAM:DA84 80                       db  80h ; Ç
	;RAM:DA85 05                       db    5
	;RAM:DA86 01                       db    1
	;RAM:DA87 03                       db    3
	;RAM:DA88 01                       db    1
	;RAM:DA89 01       unk_DA89:       db    1                 
	;RAM:DA8A FF       			       db 0FFh                 
	;RAM:DA8B 28 00 04+dpbA:           DPB <28h, 4, 0Fh, 0, 18Ah, 7Fh, 0C0h, 0, 20h, 2>
	;
	;собственно по адрессу DPB_A-2 и находится интересующая нас ячейка для диска A
	;для остальных дисков - аналогично
	;для физичесского дисковода A там значение 1
	;для физичесского дисковода B там значение 2
	;для физичесского дисковода C там значение 4
	;для физичесского дисковода D там значение 8
	;0 будет означать эмуляцию

	dbpatch	:_CPM1_DRVREG_A: 	,0x01	,0x00
	dbpatch	:_CPM1_DRVREG_B: 	,0x02	,0x00
	dbpatch	:_CPM1_DRVREG_C: 	,0x04	,0x01
	dbpatch	:_CPM1_DRVREG_D: 	,0x08	,0x02

	;для нормальной работы MOUNT в биос добавлена новая подфункция - получить таблицу
	;адресов DRVREGxx
	;call SELDSK с C=0xFE, которая возвращает указатель на таблицу с адресами DRVTRG_X
	;собтвенно готовим эту таблицу используя уже исзвестные нам данные
	;сохраняя адреса в таблицу внутри кода "резедента".

	dwstore	_CPM1_res_DRIVE_TAB+0*2	,:_CPM1_DRVREG_A:
	dwstore	_CPM1_res_DRIVE_TAB+1*2	,:_CPM1_DRVREG_B:
	dwstore	_CPM1_res_DRIVE_TAB+2*2	,:_CPM1_DRVREG_C:
	dwstore	_CPM1_res_DRIVE_TAB+3*2	,:_CPM1_DRVREG_D:


	;DPH_TABLE-----------------------------------------------------
	;добавляем поддержку диска F - образа с утилитами
	;как найти DPH_TABLE см выше
	dwpatch :_CPM1_DPH_TABLE:+5*2 	,0x0000 ,_CPM1_dph_F

	dwstore _CPM1_dph_F+8	,:_CPM1_DIR_BUF:

	;SELDSK_ADDR------------------------------------------------------------
	;и модифицируем вызов SELDSK чтобы он поддерживал дополнительную проверку
	;модифицируем оригинальный перехо в SELDSK чтобы он переходил на наш код	
	;и сохраняем старый адресс (в данном коде нужный нам адресс 0xDCB6)
	;RAM:DA1B          _SELDSK:                         ; IN: C
	;RAM:DA1B C3 B6 DC          jp      __SELDSK        ; OUT: HL-DPB/0, HL=xVTRAP0 if c=0xff

	dwpatch 0xDA1B+1 	,:_CPM1_SELDSK_ADDR:	,_CPM1_res_seldsk
	dwstore	_CPM1_old_seld_dsk+1	,:_CPM1_SELDSK_ADDR:

	;SELDSK_STORE_DRV-------------------------------------------------------
	;в процессе работы SELDSK биос вызывает функцию GETINFO которая читает параметры диска
	;из первого сектора.
	;в процессе рабты она перезаписывает значение байта DRVREG_x
	;затрём эту комманду 
	;в примере ниже E70A 77 ld (hl), a - записав NOP на ее место.

	;RAM:DC8D          j_SELDSK:                               
	;RAM:DC8D                                                  
	;RAM:DC8D 79                       ld      a, c
	;RAM:DC8E FE FF                    cp      0FFh
	;RAM:DC90 21 C8 F7                 ld      hl, _xVTRAP0_EXT
	;RAM:DC93 C8                       ret     z
	;...
	;RAM:DCBB 22 6E E0                 ld      (off_E06E), hl
	;RAM:DCBE 3C                       inc     a
	;RAM:DCBF C4 48 E6                 call    nz, GETINFO
	;RAM:DCC2 E1                       pop     hl
	;RAM:DCC3 C8                       ret     z
	;...
	;RAM:E648          GETINFO:                                
	;RAM:E648 E5                       push    hl
	;RAM:E649 CD 4E DF                 call    sub_DF4E
	;RAM:E64C E1                       pop     hl
	;...
	;RAM:E703          loc_E703:                               
	;RAM:E703 E1                       pop     hl
	;RAM:E704 E5                       push    hl
	;RAM:E705 3A 39 FB                 ld      a, (_1C_PPI1B_DrvReg)
	;RAM:E708 E6 CF                    and     ~_SIDE1|_MOTOR
	;RAM:E70A 77                       ld      (hl), a
	;RAM:E70B 23                       inc     hl
	;RAM:E70C 36 FF                    ld      (hl), 0FFh
	;RAM:E70E 23                       inc     hl

	dbpatch	:_CPM1_SELDSK_STORE_DRV:	,0x77	,0x00

	;CURRENT_DISK_PTR-------------------------------------------------------
	;нашему резиденту для операций чтения и записи нужно знать к какому диску идёт обращение
	;и надо ли подменять обращение к эмулятору
	;у биоса есть переменная которая указывает на DRVREG_x для текушего диска
	;резидент читает из этого адресса и смотрит надо ли эмулировать диск
	;собственно в коде ниже по адрессу 0xDCBB комманда которая сохраняет в нужную нам переменную
	;ld      (off_E06E), hl
	;т.е. нужный нам адресс 0xE06E

	;RAM:DC8D          j_SELDSK:                               
	;RAM:DC8D                                                  
	;RAM:DC8D 79                       ld      a, c
	;RAM:DC8E FE FF                    cp      0FFh
	;RAM:DC90 21 C8 F7                 ld      hl, _xVTRAP0_EXT
	;RAM:DC93 C8                       ret     z
	;...
	;RAM:DCBB 22 6E E0                 ld      (off_E06E), hl
	;RAM:DCBE 3C                       inc     a
	;RAM:DCBF C4 48 E6                 call    nz, GETINFO

	dwstore	_CPM1_r_p_SELDSKDRIVER+1	,:_CPM1_CURRENT_DISK_PTR:
	dwstore	_CPM1_r_p_SELDSKDRIVEW+1	,:_CPM1_CURRENT_DISK_PTR:


	;CURRENT_DISK_PTR-------------------------------------------------------
	;патчим адресса в основной таблицк BIOS для READ и WRITE
	;теперь они указывают на наш "резидент"

	;RAM:DA27          _READ:                                  
	;RAM:DA27 C3 EB DC                 jp      j_READ
	;RAM:DA2A          _WRITE:                                 
	;RAM:DA2A C3 AF DE                 jp      j_WRITE

	dwpatch	0xDA27+1	,:_CPM1_BIOS_READ:	,_CPM1_res_READ
	dwpatch	0xDA2A+1	,:_CPM1_BIOS_WRITE:	,_CPM1_res_WRITE

	;WBOOT_READ_CALL--------------------------------------------------------
	;биосовоский wboot напрямую делает вызов READ
	;патчим его тоже
	;RAM:DB5E          j_WBOOT:                                
	;RAM:DB5E                                                  
	;RAM:DB5E F3                       di
	;RAM:DB5F 31 80 00                 ld      sp, 80h ; 'Ç'
	;RAM:DB62 CD 0A E4                 call    HW_Init
	;...
	;RAM:DB80 C5                       push    bc
	;RAM:DB81 CD E5 DC                 call    j_SETDMA
	;RAM:DB84 CD EB DC                 call    j_READ
	;RAM:DB87 B7                       or      a
	;RAM:DB88 C2 5E DB                 jp      nz, j_WBOOT

	dwpatch	:_CPM1_WBOOT_READ_CALL:+1	,:_CPM1_BIOS_READ:	,_CPM1_res_READ

	;SELDSK_GETINFO_CALL------------------------------------------------------------------------
	;внутри  SELDSK есть вызов GETINFO
	;используется для чтения параметров диска с нового диска
	;мы его перехватываем, и для дисково которые работают через EXTROM API
	;делаем сами его функции (частично)
	;для физичесских дисков работает оригинальный код
	;причина в том, что GETINFO обращается к диску напрямую, а нас это не устраивает
	;в примере ниже это адресс DCBF

	;RAM:DC8D          j_SELDSK:                               
	;RAM:DC8D                                                  
	;RAM:DC8D 79                       ld      a, c
	;RAM:DC8E FE FF                    cp      0FFh
	;RAM:DC90 21 C8 F7                 ld      hl, _xVTRAP0_EXT
	;RAM:DC93 C8                       ret     z
	;...
	;RAM:DCBB 22 6E E0                 ld      (off_E06E), hl
	;RAM:DCBE 3C                       inc     a
	;RAM:DCBF C4 48 E6                 call    nz, GETINFO
	;RAM:DCC2 E1                       pop     hl
	;RAM:DCC3 C8                       ret     z

	dwpatch	:_CPM1_SELDSK_GETINFO_CALL:+1	,:_CPM1_BIOS_GETINFO:	,_CPM1_res_GETINFO

	;BIOS_xxxx--------------------------------------------------------------
	;сохраняем в "резидет" адреса оригинальных функций
	;они вызываются если идёт обращение к физичесским дискам

	dwstore	_CPM1__old_read+1	,:_CPM1_BIOS_READ:
	dwstore	_CPM1__old_write+1	,:_CPM1_BIOS_WRITE:
	dwstore	_CPM1__old_getinfo+1	,:_CPM1_BIOS_GETINFO:

	;-------------------------------------------------------------------------------
	;наш эмулятор GETINFO производит чтение первого сектора из образа диска
	;в буфер чтения, остальную работу делает сатарый код
	;т.е. мы читаем сектор в буффер и передаём управление на код который
	;проверяет результат успешности чтения, проверяе контролльную сумму сектора 
	;и готовит сектор к дальнейшей работе
	;в коде ниже это 0xE67F 

	;RAM:E648          GETINFO:                                ; CODE XREF: j_SELDSK+32p
	;RAM:E648 E5                       push    hl
	;RAM:E649 CD 4E DF                 call    sub_DF4E
	;RAM:E64C E1                       pop     hl
	;RAM:E64D 22 6E E0                 ld      (off_E06E), hl
	;RAM:E650 E5                       push    hl
	;RAM:E651 7E                       ld      a, (hl)
	;RAM:E652 E6 0F                    and     0Fh
	;RAM:E654 32 70 E0                 ld      (byte_E070), a
	;RAM:E657 AF                       xor     a
	;RAM:E658 32 71 E0                 ld      (byte_E071), a
	;RAM:E65B 3C                       inc     a
	;RAM:E65C 32 72 E0                 ld      (byte_E072), a
	;RAM:E65F 21 6E E0                 ld      hl, off_E06E
	;RAM:E662 CD D3 DF                 call    sub_DFD3
	;RAM:E665 21 00 EE                 ld      hl, RD_BUF
	;RAM:E668 CD 9A E7                 call    DTOM
	;RAM:E66B CA 85 E6                 jp      z, CHKDO
	;RAM:E66E 3A 39 FB                 ld      a, (_1C_PPI1B_DrvReg)
	;RAM:E671 C6 40                    add     a, 40h ; '@'
	;RAM:E673 32 39 FB                 ld      (_1C_PPI1B_DrvReg), a
	;RAM:E676 CD 79 E8                 call    FDD_BREAK
	;RAM:E679 21 00 EE                 ld      hl, RD_BUF
	;RAM:E67C CD 9A E7                 call    DTOM
	;!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	;RAM:E67F 21 AF E6                 ld      hl, msgDiskNotRead 
	;RAM:E682 C2 A7 E6                 jp      nz, loc_E6A7
	;RAM:E685

	;для надёжности проверим что по этому аддреск находтся то что ожидаем 

	dbpatch	:_CPM1_GETINFO_POSTDTOM_ADDR:		,0x21	,0x21
	dwpatch	:_CPM1_GETINFO_POSTDTOM_ADDR:+1 	,:_CPM1_GETINFO_POSTDTOM_CHKW:	,:_CPM1_GETINFO_POSTDTOM_CHKW:

	;и устанавливаем в наш "резидент" переход на эту точку.
	
	dwstore	_CPM1__old_getinfo_chkdo+1	,:_CPM1_GETINFO_POSTDTOM_ADDR:

	;BIOS_VAR_xxx-----------------------------------------------------------------------
	;BIOS_VAR_DSK,BIOS_VAR_TRK,BIOS_VAR_SEC,BIOS_VAR_DMA
	;системные переменные BIOS которые необходимы "эмулятору" для работы 
	;функций READ/WRITE, т.к. их параметры предварительно записываются именно в
	;эти переменные 
	;сохраняем в эмулятор значения характерные именно для текущего биоса
	;
	;найти их совсем просто
	;
	;в стандартном CPM BIOS есть набор соответсвующих вызовов
	;TRK -> 0xE068 (инструкция по адрессу DC89)
	;SEC -> 0xE069 (инструкция по адрессу DCDD)
	;DMA -> 0xE06A (инструкция по адрессу DCE7)

	;RAM:DA1E          _SELTRK:                                
	;RAM:DA1E C3 88 DC                 jp      j_SELTRK
	;RAM:DA21          _SETSEC:                                
	;RAM:DA21 C3 DC DC                 jp      j_SETSEC
	;RAM:DA24          _SETDMA:
	;RAM:DA24 C3 E5 DC                 jp      j_SETDMA
	;
	;RAM:DC88          j_SELTRK:                               
	;RAM:DC88                                                  
	;RAM:DC88 79                       ld      a, c
	;RAM:DC89 32 68 E0                 ld      (_TRK), a
	;RAM:DC8C C9                       ret
	;...
	;RAM:DCDC          j_SETSEC:                               
	;RAM:DCDC                                                  
	;RAM:DCDC 79                       ld      a, c
	;RAM:DCDD 32 69 E0                 ld      (_SEC), a
	;RAM:DCE0 C9                       ret
	;...
	;RAM:DCE5          j_SETDMA:                               
	;RAM:DCE5                                                  
	;RAM:DCE5 69                       ld      l, c
	;RAM:DCE6 60                       ld      h, b
	;RAM:DCE7 22 6A E0                 ld      (_DMA), hl
	;RAM:DCEA C9                       ret

	;а с DSK - тоже, многострадальный код SELDSK
	;тут нужная нам число 0xE064 в инструкции по адрессу 0xDC9C

	;RAM:DC8D          j_SELDSK:                               
	;RAM:DC8D 79                       ld      a, c
	;RAM:DC8E FE FF                    cp      0FFh
	;RAM:DC90 21 C8 F7                 ld      hl, _xVTRAP0_EXT
	;RAM:DC93 C8                       ret     z
	;RAM:DC94 FE 08                    cp      8
	;RAM:DC96 D2 C4 DC                 jp      nc, loc_DCC4
	;RAM:DC99 21 CC DC                 ld      hl, DPH_TABLE
	;RAM:DC9C 32 64 E0                 ld      (_DSK), a

	dwstore	_CPM1_r_p_DSK1+1	,:_CPM1_BIOS_VAR_DSK: 	;read
	dwstore	_CPM1_r_p_TRK1+1	,:_CPM1_BIOS_VAR_TRK: 	
	dwstore	_CPM1_r_p_SEC1+1	,:_CPM1_BIOS_VAR_SEC: 	
	dwstore	_CPM1_r_p_DMA1+1	,:_CPM1_BIOS_VAR_DMA: 	

	dwstore	_CPM1_r_p_DSK2+1	,:_CPM1_BIOS_VAR_DSK: 	;write
	dwstore	_CPM1_r_p_TRK2+1	,:_CPM1_BIOS_VAR_TRK: 	
	dwstore	_CPM1_r_p_SEC2+1	,:_CPM1_BIOS_VAR_SEC: 	
	dwstore	_CPM1_r_p_DMA2+1	,:_CPM1_BIOS_VAR_DMA: 	

	dwstore	_CPM1_r_p_DSK3+1	,:_CPM1_BIOS_VAR_DSK: 	;readinfo


	;KEY_HOOK:---------------------------------------------------------------------------
	;VINTKBDADDR,VINTKBDVALUE,KBD_HOOK_PROC
	;перехватываем нажатие комбинации клавиш CTRL+SHIFT+STOP
	;планируется что по нажатии этой комбинации - содержимое диска E
	;будет перезаписано содержимым на котором записан - 
	;MOUNT и другие служебные утилиты для работы с EXTROM API 
	;к сожадлению нет общего для всех кода
	;точнее самые старые биосы - требуют "особых" отношений
	;там немного по другому драйвер клавиатуры возвращает коды.
	;
	;kbd_hook_noCtrl_03 - '12_87_09_NIIJAF'	и производных от него '1x_88_EPSON_V104','1x_89_03_30_RAVI'
	;kbd_hook_noCtrl_33 - '12_87_11_niijaf'
	;kbd_hook_2133      - все остальные

	;по сути, мы подменяем перехватываем обработчик VTRAP8_pseudoKBD
	;как найти 

	;в HWINIT есть копирование стандартной таблицы векторов VTRAP в рабочее место
	;вот ее мы и патчим
	;в коде ниже это адрес 0xdc46 и значение 0xdc46

	;RAM:DA00          _BOOT:
	;RAM:DA00 C3 42 DB                 jp      j_BOOT
	;...
	;RAM:DB42          j_BOOT:                                 
	;RAM:DB42 F3                       di
	;RAM:DB43 31 80 00                 ld      sp, 80h ; 'Ç'
	;RAM:DB46 3E 1C                    ld      a, 1Ch
	;RAM:DB48 32 7F FA                 ld      (_1C_SysReg1C), a
	;RAM:DB4B 32 03 F7                 ld      (SysCOPY), a
	;RAM:DB4E CD 0A E4                 call    HW_Init
	;....
	;RAM:E40A          HW_Init:                                
	;RAM:E40A                                                  
	;RAM:E40A F3                       di
	;RAM:E40B 11 D1 E3                 ld      de, IntstINTAB
	;RAM:E40E 21 C6 F7                 ld      hl, _xVTRAP
	;RAM:E411 01 3A 00                 ld      bc, 58
	;RAM:E414 CD 3A E0                 call    _LDIR
	;....
	;RAM:E3D1 D3 E3    IntstINTAB:     dw off_E3D3             
	;RAM:E3D3 0C DC    off_E3D3:       dw _EOI                 
	;RAM:E3D3                                                  
	;RAM:E3D5 0C DC                    dw _EOI
	;RAM:E3D7 0C DC                    dw _EOI
	;RAM:E3D9 0C DC                    dw _EOI
	;RAM:E3DB 1C DC                    dw VINT_VBL
	;RAM:E3DD 0C DC                    dw _EOI
	;RAM:E3DF 0C DC                    dw _EOI
	;RAM:E3E1 0C DC                    dw _EOI
	;!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	;RAM:E3E3 46 DC                    dw VINT_KBD
	;!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	;RAM:E3E5 00 00                    dw 0
	;RAM:E3E7 00 00                    dw 0
	;RAM:E3E9 00 00                    dw 0
	
	;support removed
	;dwpatch	#_CPM1_VINTKBDADDR#	,#_CPM1_VINTKBDVALUE#	,#_CPM1_KBD_HOOK_PROC#
	;dwstore	_CPM1_old_hook+1	,#_CPM1_VINTKBDVALUE#

:CUSTOM_CHECKERS:

	;STOP-------------------------------------------------------------------------------
	stop ':NAME:'
