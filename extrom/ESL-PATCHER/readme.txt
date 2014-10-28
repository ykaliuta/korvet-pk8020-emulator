forh32

seldsk: вызывает getinfo для чтения параметров диска
у нее на входе CPMTXX - 2 = DPB-2 диска
в этой ячейче лежит значение которое записывается в DRVREG для выбора диска

выставив нужный бит тут можно ремапить физ диск на другую букву

в этот байт пинаем флаг (forth32 пишет 0)
а надо наверное 0x80+drvive
в diskinfo по флагу чтение физ диска
а читаем SD и прыгаем на проверку параметров 

	CP	0	; диск А или В? У них нет маски выбора
	JNZ	GETINFO_FDD 	JNZ	GETINFO_FDD  ; нет - читаем сектор с реального дисковода
;
;  Чтение с эмулируемых дисков A или B
;
	CALL	EXR_READINFO
	DEC	A		; ответ, 0 - ошибка, 1 - ОК
	JP	NZ,IERR		; 0 - ошибка
	JP	CHKDO
;
;  Чтение с реального дисковода
;
GETINFO_FDD:

еще патч
;  Загрузка считанной информации в табл.CPMTxx
CHOK:
	POP	HL	; HL points to DPBLKXX
	PUSH	HL	; Save HL
;	LD	A,(DRVREG) 
;	AND	0CFH	; (motor or side1)
;	LD 	(HL),A	;            запись маски не нужна - маски намертво вбиты в блоки параметров
т.е. забить 6 байт нопами

или просто проверить crc самому, и перенести нужное в dpb


read:
начало там такое
READ:
	MVI	 A,4	;Режим чтения
	STA 	OPER
	LDA 	DSKCPM
	CPI	 4
	JZ  	EDISK	;Больше 4 E-диск

можно вообще забрать управление на себя и вернуть только если не надо обрабатывать

write:
тоже самое
WRITE:
	MVI	A,6	;Режим записи
	STA	OPER
	LDA	DSKCPM
	CPI	4
	JZ	EDISK	;RAM-диск

http://www.gaby.de/cpm/manuals/archive/cpm22htm/ch6.htm#Section_6.6
On each call to WRITE, the BDOS provides the following information in register C:

 

0	(normal sector write)
1	(write to directory sector)
2	(write to the first sector of a new data block)
Condition 0 occurs whenever the next write operation is into a previously written area, such as a random mode record update; when the write is to other than the first sector of an unallocated block; or when the write is not into the directory area. Condition 1 occurs when a write into the directory area is performed. Condition 2 occurs when the first record (only) of a newly allocated data block is written. In most cases, application programs read or write multiple 128-byte sectors in sequence; thus, there is little overhead involved in either operation when blocking and deblocking records, because preread operations can be avoided when writing records.