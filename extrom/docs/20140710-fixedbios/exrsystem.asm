;**************************************************************
;*
;*             C P / M   version   2 . 2
;*
;**************************************************************
;   Данный исходный текст является полноценной ОС CP/M-80, предназначенный
; для работы с дисковым эмулятором Korvet-Extrom. Без аппаратной поддержки контроллера EXTROM система работать не будет!
; При работе системы буквы дисков отобажаются так:
;     A - эмулируемый диск А
;     B - эмулируемый диск В
;     C - физический дисковод 0
;     D - физический дисковод 1
;     E - рамдиск
; Дисководы 2 и 3 системой не поддерживаются, но этих дисководов на реальных корветах и не бывает.
;
; Исходный текст транслируется макроассемблером ZMAC. В результате получается двоичный файл, состоящий из 3 частей:
;
;      Адрес        Адрес    
;     на диске    в памяти         Описание
; ---------------------------------------------      
;    0000-007F    C380-C3FF   информационный сектор диска
;    0080-167F    C400-D9FF   BDOS cp/m 2.2
;    1680-258C    DA00-E90C   BIOS korvet-extrom
;
; Все байты за адресом E90C свободны, это пространство образуется при выравнивании на границу физического сектора.
; В будущем, при доработке BIOS, врехний адрес будет смещаться.
; Полученный в результате трансляции двоичный файл можно напрямую записывать в системные дорожки образа загружаемой дискеты А.
; Он имеет размер 10К, то есть как раз 2 дорожки по 5 секторов размером 1к.
;
;  Например:
;    dd if=exrsystem.cim of=disk.kdi conv=notrunc
;
; ============ ВНИМАНИЕ !!!! ============
; 1. В процессе изменения этих исходных тектов не забывайте о выравнивании компонентов системы. BDOS практически полностью
;    занимает свое адресное пространство, над ним свободны только 4 байта! При модификации BDOS следите, чтобы его верхняя граница
;    не залезла за DA00, иначе придется сдвигать BIOS, что нежелательно. Лучше сдвинуть вниз на килобайт нижнюю границу путем изменения 
;    значения ячейки MEM. При этом не забудьте о контрольной сумме инфосектора.
;
; 2. BIOS может расти вверх вплоть до EBA6h.
;
; 3. Контрольная сумма инфосектора автоматически не пересчитывается. Макроассемблер просто не умеет это делать. Если изменили поля
;    инфосектора - пересчитайте сумму внешними инструментами и впишите ее в код.
;
;
;====================================================================================================================================================
; MEM - верхняя граница доступной памяти в килобайтах. 56 - это E000. Соответственно, BDOS выравнивается так, чтобы не 
; пересекать эту границу. BIOS располагается сразу за BDOS с адреса DA00, выше BIOS идут фиксированный рабочие ячейки памяти
;
MEM	EQU	56		
;
IOBYTE	EQU	3		;i/o definition byte.
TDRIVE	EQU	4		;current drive name and user number.
ENTRY	EQU	5		;entry point for the cp/m bdos.
TFCB	EQU	5CH		;default file control block.
TBUFF	EQU	80H		;i/o buffer and command line storage.
TBASE	EQU	100H		;transiant program storage area.
;
;   Set control character equates.
;
CNTRLC	EQU	3		;control-c
CNTRLE	EQU	05H		;control-e
BS	EQU	08H		;backspace
TAB	EQU	09H		;tab
LF	EQU	0AH		;line feed
FF	EQU	0CH		;form feed
CR	EQU	0DH		;carriage return
CNTRLP	EQU	10H		;control-p
CNTRLR	EQU	12H		;control-r
CNTRLS	EQU	13H		;control-s
CNTRLU	EQU	15H		;control-u
CNTRLX	EQU	18H		;control-x
CNTRLZ	EQU	1AH		;control-z (end-of-file mark)
DEL	EQU	7FH		;rubout

;=========================================================================================================================================
;*      Информационный сектор диска 
;* Записывается в сектор 0 дорожки 0. 
;=========================================================================================================================================
	ORG	(MEM-7)*1024-128 ; Загружается в память на 128 байт ниже BDOS
MEMLOW	EQU	$		; используется для выравнивания верхней границы системных дорожек (размера выходного файла)	

; Информация для загрузчика
	DW	$ 		; адрес загрузки 
	DW	CBOOT		; точка входа 
	DW	10		; число загружаемых физических секторов
	
; Физические параметры диска	
	DB	00		; тип диска - 5"
	DB	01		; MFM
	DB	01 		; TPI
	DB	01		; таблица чередования секторов отсутствует
	DB	03 		; физ. сектор 1024 байта
	DB	01		; двухсторонний диск
	DW	05		; Phys SPT
	DW	80   		; число дорожек
	
; Логические параметры

	DW	28h		; Log SPT
	DB	04 		; фактор блокирования лог. секторов в единицу хранения данных
	DB	0fh		; маска блокирования единицы хранения данных
	DB	00		; маска размера
	DW	18ah		; объем диска в блоках -1
	DW	7fh		; макс. число записей каталога
	DB	0c0h		; зарезервированные блоки - AL0
	DB	00		; AL1
	DW	20h		; 
	DW	02		; число системных дорожек
	DB	10h		; Контрольная сумма инфосектора  

;=========================================================================================================================================
;   BDOS CP/M-80
;=========================================================================================================================================
;
;
	ORG	(MEM-7)*1024	; базовый адрес BDOS. Зазор между последним полем инфосектора и
				; этой точкой не содержит полезной информации и может заполняться чем угодно.
;
CBASE:	JP	COMMAND		;execute command processor (ccp).
	JP	CLEARBUF	;entry to empty input buffer before starting ccp.

;
;   Standard cp/m ccp input buffer. Format is (max length),
; (actual length), (char #1), (char #2), (char #3), etc.
;
INBUFF:	DEFB	127		;length of input buffer.
	DEFB	0		;current length of contents.
	DEFB	'Copyright'
	DEFB	' 1979 (c) by Digital Research      '
	DEFB	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	DEFB	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	DEFB	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	DEFB	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
INPOINT:DEFW	INBUFF+2	;input line pointer
NAMEPNT:DEFW	0		;input line pointer used for error message. Points to
;			;start of name in error.
;
;   Routine to print (A) on the console. All registers used.
;
PRINT:	LD	E,A		;setup bdos call.
	LD	C,2
	JP	ENTRY
;
;   Routine to print (A) on the console and to save (BC).
;
PRINTB:	PUSH	BC
	CALL	PRINT
	POP	BC
	RET	
;
;   Routine to send a carriage return, line feed combination
; to the console.
;
CRLF:	LD	A,CR
	CALL	PRINTB
	LD	A,LF
	JP	PRINTB
;
;   Routine to send one space to the console and save (BC).
;
SPACE:	LD	A,' '
	JP	PRINTB
;
;   Routine to print character string pointed to be (BC) on the
; console. It must terminate with a null byte.
;
PLINE:	PUSH	BC
	CALL	CRLF
	POP	HL
PLINE2:	LD	A,(HL)
	OR	A
	RET	Z
	INC	HL
	PUSH	HL
	CALL	PRINT
	POP	HL
	JP	PLINE2
;
;   Routine to reset the disk system.
;
RESDSK:	LD	C,13
	JP	ENTRY
;
;   Routine to select disk (A).
;
DSKSEL:	LD	E,A
	LD	C,14
	JP	ENTRY
;
;   Routine to call bdos and save the return code. The zero
; flag is set on a return of 0ffh.
;
ENTRY1:	CALL	ENTRY
	LD	(RTNCODE),A	;save return code.
	INC	A		;set zero if 0ffh returned.
	RET	
;
;   Routine to open a file. (DE) must point to the FCB.
;
OPEN:	LD	C,15
	JP	ENTRY1
;
;   Routine to open file at (FCB).
;
OPENFCB:XOR	A		;clear the record number byte at fcb+32
	LD	(FCB+32),A
	LD	DE,FCB
	JP	OPEN
;
;   Routine to close a file. (DE) points to FCB.
;
CLOSE:	LD	C,16
	JP	ENTRY1
;
;   Routine to search for the first file with ambigueous name
; (DE).
;
SRCHFST:LD	C,17
	JP	ENTRY1
;
;   Search for the next ambigeous file name.
;
SRCHNXT:LD	C,18
	JP	ENTRY1
;
;   Search for file at (FCB).
;
SRCHFCB:LD	DE,FCB
	JP	SRCHFST
;
;   Routine to delete a file pointed to by (DE).
;
DELETE:	LD	C,19
	JP	ENTRY
;
;   Routine to call the bdos and set the zero flag if a zero
; status is returned.
;
ENTRY2:	CALL	ENTRY
	OR	A		;set zero flag if appropriate.
	RET	
;
;   Routine to read the next record from a sequential file.
; (DE) points to the FCB.
;
RDREC:	LD	C,20
	JP	ENTRY2
;
;   Routine to read file at (FCB).
;
READFCB:LD	DE,FCB
	JP	RDREC
;
;   Routine to write the next record of a sequential file.
; (DE) points to the FCB.
;
WRTREC:	LD	C,21
	JP	ENTRY2
;
;   Routine to create the file pointed to by (DE).
;
CREATE:	LD	C,22
	JP	ENTRY1
;
;   Routine to rename the file pointed to by (DE). Note that
; the new name starts at (DE+16).
;
RENAM:	LD	C,23
	JP	ENTRY
;
;   Get the current user code.
;
GETUSR:	LD	E,0FFH
;
;   Routne to get or set the current user code.
; If (E) is FF then this is a GET, else it is a SET.
;
GETSETUC: LD	C,32
	JP	ENTRY
;
;   Routine to set the current drive byte at (TDRIVE).
;
SETCDRV:CALL	GETUSR		;get user number
	ADD	A,A		;and shift into the upper 4 bits.
	ADD	A,A
	ADD	A,A
	ADD	A,A
	LD	HL,CDRIVE	;now add in the current drive number.
	OR	(HL)
	LD	(TDRIVE),A	;and save.
	RET	
;
;   Move currently active drive down to (TDRIVE).
;
MOVECD:	LD	A,(CDRIVE)
	LD	(TDRIVE),A
	RET	
;
;   Routine to convert (A) into upper case ascii. Only letters
; are affected.
;
UPPER:	CP	'a'		;check for letters in the range of 'a' to 'z'.
	RET	C
	CP	'{'
	RET	NC
	AND	5FH		;convert it if found.
	RET	
;
;   Routine to get a line of input. We must check to see if the
; user is in (BATCH) mode. If so, then read the input from file
; ($$$.SUB). At the end, reset to console input.
;
GETINP:	LD	A,(BATCH)	;if =0, then use console input.
	OR	A
	JP	Z,GETINP1
;
;   Use the submit file ($$$.sub) which is prepared by a
; SUBMIT run. It must be on drive (A) and it will be deleted
; if and error occures (like eof).
;
	LD	A,(CDRIVE)	;select drive 0 if need be.
	OR	A
	LD	A,0		;always use drive A for submit.
	CALL	NZ,DSKSEL	;select it if required.
	LD	DE,BATCHFCB
	CALL	OPEN		;look for it.
	JP	Z,GETINP1	;if not there, use normal input.
	LD	A,(BATCHFCB+15)	;get last record number+1.
	DEC	A
	LD	(BATCHFCB+32),A
	LD	DE,BATCHFCB
	CALL	RDREC		;read last record.
	JP	NZ,GETINP1	;quit on end of file.
;
;   Move this record into input buffer.
;
	LD	DE,INBUFF+1
	LD	HL,TBUFF	;data was read into buffer here.
	LD	B,128		;all 128 characters may be used.
	CALL	HL2DE		;(HL) to (DE), (B) bytes.
	LD	HL,BATCHFCB+14
	LD	(HL),0		;zero out the 's2' byte.
	INC	HL		;and decrement the record count.
	DEC	(HL)
	LD	DE,BATCHFCB	;close the batch file now.
	CALL	CLOSE
	JP	Z,GETINP1	;quit on an error.
	LD	A,(CDRIVE)	;re-select previous drive if need be.
	OR	A
	CALL	NZ,DSKSEL	;don't do needless selects.
;
;   Print line just read on console.
;
	LD	HL,INBUFF+2
	CALL	PLINE2
	CALL	CHKCON		;check console, quit on a key.
	JP	Z,GETINP2	;jump if no key is pressed.
;
;   Terminate the submit job on any keyboard input. Delete this
; file such that it is not re-started and jump to normal keyboard
; input section.
;
	CALL	DELBATCH	;delete the batch file.
	JP	CMMND1		;and restart command input.
;
;   Get here for normal keyboard input. Delete the submit file
; incase there was one.
;
GETINP1:CALL	DELBATCH	;delete file ($$$.sub).
	CALL	SETCDRV		;reset active disk.
	LD	C,10		;get line from console device.
	LD	DE,INBUFF
	CALL	ENTRY
	CALL	MOVECD		;reset current drive (again).
;
;   Convert input line to upper case.
;
GETINP2:LD	HL,INBUFF+1
	LD	B,(HL)		;(B)=character counter.
GETINP3:INC	HL
	LD	A,B		;end of the line?
	OR	A
	JP	Z,GETINP4
	LD	A,(HL)		;convert to upper case.
	CALL	UPPER
	LD	(HL),A
	DEC	B		;adjust character count.
	JP	GETINP3
GETINP4:LD	(HL),A		;add trailing null.
	LD	HL,INBUFF+2
	LD	(INPOINT),HL	;reset input line pointer.
	RET	
;
;   Routine to check the console for a key pressed. The zero
; flag is set is none, else the character is returned in (A).
;
CHKCON:	LD	C,11		;check console.
	CALL	ENTRY
	OR	A
	RET	Z		;return if nothing.
	LD	C,1		;else get character.
	CALL	ENTRY
	OR	A		;clear zero flag and return.
	RET	
;
;   Routine to get the currently active drive number.
;
GETDSK:	LD	C,25
	JP	ENTRY
;
;   Set the stabdard dma address.
;
STDDMA:	LD	DE,TBUFF
;
;   Routine to set the dma address to (DE).
;
DMASET:	LD	C,26
	JP	ENTRY
;
;  Delete the batch file created by SUBMIT.
;
DELBATCH: LD	HL,BATCH	;is batch active?
	LD	A,(HL)
	OR	A
	RET	Z
	LD	(HL),0		;yes, de-activate it.
	XOR	A
	CALL	DSKSEL		;select drive 0 for sure.
	LD	DE,BATCHFCB	;and delete this file.
	CALL	DELETE
	LD	A,(CDRIVE)	;reset current drive.
	JP	DSKSEL
;
;   Check to two strings at (PATTRN1) and (PATTRN2). They must be
; the same or we halt....
;
VERIFY:	LD	DE,PATTRN1	;these are the serial number bytes.
	LD	HL,PATTRN2	;ditto, but how could they be different?
	LD	B,6		;6 bytes each.
VERIFY1:LD	A,(DE)
	CP	(HL)
	JP	NZ,HALT		;jump to halt routine.
	INC	DE
	INC	HL
	DEC	B
	JP	NZ,VERIFY1
	RET	
;
;   Print back file name with a '?' to indicate a syntax error.
;
SYNERR:	CALL	CRLF		;end current line.
	LD	HL,(NAMEPNT)	;this points to name in error.
SYNERR1:LD	A,(HL)		;print it until a space or null is found.
	CP	' '
	JP	Z,SYNERR2
	OR	A
	JP	Z,SYNERR2
	PUSH	HL
	CALL	PRINT
	POP	HL
	INC	HL
	JP	SYNERR1
SYNERR2:LD	A,'?'		;add trailing '?'.
	CALL	PRINT
	CALL	CRLF
	CALL	DELBATCH	;delete any batch file.
	JP	CMMND1		;and restart from console input.
;
;   Check character at (DE) for legal command input. Note that the
; zero flag is set if the character is a delimiter.
;
CHECK:	LD	A,(DE)
	OR	A
	RET	Z
	CP	' '		;control characters are not legal here.
	JP	C,SYNERR
	RET	Z		;check for valid delimiter.
	CP	'='
	RET	Z
	CP	'_'
	RET	Z
	CP	'.'
	RET	Z
	CP	':'
	RET	Z
	CP	';'
	RET	Z
	CP	'<'
	RET	Z
	CP	'>'
	RET	Z
	RET	
;
;   Get the next non-blank character from (DE).
;
NONBLANK: LD	A,(DE)
	OR	A		;string ends with a null.
	RET	Z
	CP	' '
	RET	NZ
	INC	DE
	JP	NONBLANK
;
;   Add (HL)=(HL)+(A)
;
ADDHL:	ADD	A,L
	LD	L,A
	RET	NC		;take care of any carry.
	INC	H
	RET	
;
;   Convert the first name in (FCB).
;
CONVFST:LD	A,0
;
;   Format a file name (convert * to '?', etc.). On return,
; (A)=0 is an unambigeous name was specified. Enter with (A) equal to
; the position within the fcb for the name (either 0 or 16).
;
CONVERT:LD	HL,FCB
	CALL	ADDHL
	PUSH	HL
	PUSH	HL
	XOR	A
	LD	(CHGDRV),A	;initialize drive change flag.
	LD	HL,(INPOINT)	;set (HL) as pointer into input line.
	EX	DE,HL
	CALL	NONBLANK	;get next non-blank character.
	EX	DE,HL
	LD	(NAMEPNT),HL	;save pointer here for any error message.
	EX	DE,HL
	POP	HL
	LD	A,(DE)		;get first character.
	OR	A
	JP	Z,CONVRT1
	SBC	A,'A'-1		;might be a drive name, convert to binary.
	LD	B,A		;and save.
	INC	DE		;check next character for a ':'.
	LD	A,(DE)
	CP	':'
	JP	Z,CONVRT2
	DEC	DE		;nope, move pointer back to the start of the line.
CONVRT1:LD	A,(CDRIVE)
	LD	(HL),A
	JP	CONVRT3
CONVRT2:LD	A,B
	LD	(CHGDRV),A	;set change in drives flag.
	LD	(HL),B
	INC	DE
;
;   Convert the basic file name.
;
CONVRT3:LD	B,08H
CONVRT4:CALL	CHECK
	JP	Z,CONVRT8
	INC	HL
	CP	'*'		;note that an '*' will fill the remaining
	JP	NZ,CONVRT5	;field with '?'.
	LD	(HL),'?'
	JP	CONVRT6
CONVRT5:LD	(HL),A
	INC	DE
CONVRT6:DEC	B
	JP	NZ,CONVRT4
CONVRT7:CALL	CHECK		;get next delimiter.
	JP	Z,GETEXT
	INC	DE
	JP	CONVRT7
CONVRT8:INC	HL		;blank fill the file name.
	LD	(HL),' '
	DEC	B
	JP	NZ,CONVRT8
;
;   Get the extension and convert it.
;
GETEXT:	LD	B,03H
	CP	'.'
	JP	NZ,GETEXT5
	INC	DE
GETEXT1:CALL	CHECK
	JP	Z,GETEXT5
	INC	HL
	CP	'*'
	JP	NZ,GETEXT2
	LD	(HL),'?'
	JP	GETEXT3
GETEXT2:LD	(HL),A
	INC	DE
GETEXT3:DEC	B
	JP	NZ,GETEXT1
GETEXT4:CALL	CHECK
	JP	Z,GETEXT6
	INC	DE
	JP	GETEXT4
GETEXT5:INC	HL
	LD	(HL),' '
	DEC	B
	JP	NZ,GETEXT5
GETEXT6:LD	B,3
GETEXT7:INC	HL
	LD	(HL),0
	DEC	B
	JP	NZ,GETEXT7
	EX	DE,HL
	LD	(INPOINT),HL	;save input line pointer.
	POP	HL
;
;   Check to see if this is an ambigeous file name specification.
; Set the (A) register to non zero if it is.
;
	LD	BC,11		;set name length.
GETEXT8:INC	HL
	LD	A,(HL)
	CP	'?'		;any question marks?
	JP	NZ,GETEXT9
	INC	B		;count them.
GETEXT9:DEC	C
	JP	NZ,GETEXT8
	LD	A,B
	OR	A
	RET	
;
;   CP/M command table. Note commands can be either 3 or 4 characters long.
;
NUMCMDS EQU	6		;number of commands
CMDTBL:	DEFB	'DIR '
	DEFB	'ERA '
	DEFB	'TYPE'
	DEFB	'SAVE'
	DEFB	'REN '
	DEFB	'USER'
;
;   The following six bytes must agree with those at (PATTRN2)
; or cp/m will HALT. Why?
;
PATTRN1:DEFB	0,22,0,0,0,0	;(* serial number bytes *).
;
;   Search the command table for a match with what has just
; been entered. If a match is found, then we jump to the
; proper section. Else jump to (UNKNOWN).
; On return, the (C) register is set to the command number
; that matched (or NUMCMDS+1 if no match).
;
SEARCH:	LD	HL,CMDTBL
	LD	C,0
SEARCH1:LD	A,C
	CP	NUMCMDS		;this commands exists.
	RET	NC
	LD	DE,FCB+1	;check this one.
	LD	B,4		;max command length.
SEARCH2:LD	A,(DE)
	CP	(HL)
	JP	NZ,SEARCH3	;not a match.
	INC	DE
	INC	HL
	DEC	B
	JP	NZ,SEARCH2
	LD	A,(DE)		;allow a 3 character command to match.
	CP	' '
	JP	NZ,SEARCH4
	LD	A,C		;set return register for this command.
	RET	
SEARCH3:INC	HL
	DEC	B
	JP	NZ,SEARCH3
SEARCH4:INC	C
	JP	SEARCH1
;
;   Set the input buffer to empty and then start the command
; processor (ccp).
;
CLEARBUF: XOR	A
	LD	(INBUFF+1),A	;second byte is actual length.
;
;**************************************************************
;*
;*
;* C C P  -   C o n s o l e   C o m m a n d   P r o c e s s o r
;*
;**************************************************************
;*
COMMAND: LD	SP,CCPSTACK	;setup stack area.
	PUSH	BC		;note that (C) should be equal to:
	LD	A,C		;(uuuudddd) where 'uuuu' is the user number
	RRA			;and 'dddd' is the drive number.
	RRA	
	RRA	
	RRA	
	AND	0FH		;isolate the user number.
	LD	E,A
	CALL	GETSETUC	;and set it.
	CALL	RESDSK		;reset the disk system.
	LD	(BATCH),A	;clear batch mode flag.
	POP	BC
	LD	A,C
	AND	0FH		;isolate the drive number.
	LD	(CDRIVE),A	;and save.
	CALL	DSKSEL		;...and select.
	LD	A,(INBUFF+1)
	OR	A		;anything in input buffer already?
	JP	NZ,CMMND2	;yes, we just process it.
;
;   Entry point to get a command line from the console.
;
CMMND1:	LD	SP,CCPSTACK	;set stack straight.
	CALL	CRLF		;start a new line on the screen.
	CALL	GETDSK		;get current drive.
	ADD	A,'A'
	CALL	PRINT		;print current drive.
	LD	A,'>'
	CALL	PRINT		;and add prompt.
	CALL	GETINP		;get line from user.
;
;   Process command line here.
;
CMMND2:	LD	DE,TBUFF
	CALL	DMASET		;set standard dma address.
	CALL	GETDSK
	LD	(CDRIVE),A	;set current drive.
	CALL	CONVFST		;convert name typed in.
	CALL	NZ,SYNERR	;wild cards are not allowed.
	LD	A,(CHGDRV)	;if a change in drives was indicated,
	OR	A		;then treat this as an unknown command
	JP	NZ,UNKNOWN	;which gets executed.
	CALL	SEARCH		;else search command table for a match.
;
;   Note that an unknown command returns
; with (A) pointing to the last address
; in our table which is (UNKNOWN).
;
	LD	HL,CMDADR	;now, look thru our address table for command (A).
	LD	E,A		;set (DE) to command number.
	LD	D,0
	ADD	HL,DE
	ADD	HL,DE		;(HL)=(CMDADR)+2*(command number).
	LD	A,(HL)		;now pick out this address.
	INC	HL
	LD	H,(HL)
	LD	L,A
	JP	(HL)		;now execute it.
;
;   CP/M command address table.
;
CMDADR:	DEFW	DIRECT,ERASE,TYPE,SAVE
	DEFW	RENAME,USER,UNKNOWN
;
;   Halt the system. Reason for this is unknown at present.
;
HALT:	LD	HL,76F3H	;'DI HLT' instructions.
	LD	(CBASE),HL
	LD	HL,CBASE
	JP	(HL)
;
;   Read error while TYPEing a file.
;
RDERROR:LD	BC,RDERR
	JP	PLINE
RDERR:	DEFB	'Read error',0
;
;   Required file was not located.
;
NONE:	LD	BC,NOFILE
	JP	PLINE
NOFILE:	DEFB	'No file',0
;
;   Decode a command of the form 'A>filename number{ filename}.
; Note that a drive specifier is not allowed on the first file
; name. On return, the number is in register (A). Any error
; causes 'filename?' to be printed and the command is aborted.
;
DECODE:	CALL	CONVFST		;convert filename.
	LD	A,(CHGDRV)	;do not allow a drive to be specified.
	OR	A
	JP	NZ,SYNERR
	LD	HL,FCB+1	;convert number now.
	LD	BC,11		;(B)=sum register, (C)=max digit count.
DECODE1:LD	A,(HL)
	CP	' '		;a space terminates the numeral.
	JP	Z,DECODE3
	INC	HL
	SUB	'0'		;make binary from ascii.
	CP	10		;legal digit?
	JP	NC,SYNERR
	LD	D,A		;yes, save it in (D).
	LD	A,B		;compute (B)=(B)*10 and check for overflow.
	AND	0E0H
	JP	NZ,SYNERR
	LD	A,B
	RLCA	
	RLCA	
	RLCA			;(A)=(B)*8
	ADD	A,B		;.......*9
	JP	C,SYNERR
	ADD	A,B		;.......*10
	JP	C,SYNERR
	ADD	A,D		;add in new digit now.
DECODE2:JP	C,SYNERR
	LD	B,A		;and save result.
	DEC	C		;only look at 11 digits.
	JP	NZ,DECODE1
	RET	
DECODE3:LD	A,(HL)		;spaces must follow (why?).
	CP	' '
	JP	NZ,SYNERR
	INC	HL
DECODE4:DEC	C
	JP	NZ,DECODE3
	LD	A,B		;set (A)=the numeric value entered.
	RET	
;
;   Move 3 bytes from (HL) to (DE). Note that there is only
; one reference to this at (A2D5h).
;
MOVE3:	LD	B,3
;
;   Move (B) bytes from (HL) to (DE).
;
HL2DE:	LD	A,(HL)
	LD	(DE),A
	INC	HL
	INC	DE
	DEC	B
	JP	NZ,HL2DE
	RET	
;
;   Compute (HL)=(TBUFF)+(A)+(C) and get the byte that's here.
;
EXTRACT:LD	HL,TBUFF
	ADD	A,C
	CALL	ADDHL
	LD	A,(HL)
	RET	
;
;  Check drive specified. If it means a change, then the new
; drive will be selected. In any case, the drive byte of the
; fcb will be set to null (means use current drive).
;
DSELECT:XOR	A		;null out first byte of fcb.
	LD	(FCB),A
	LD	A,(CHGDRV)	;a drive change indicated?
	OR	A
	RET	Z
	DEC	A		;yes, is it the same as the current drive?
	LD	HL,CDRIVE
	CP	(HL)
	RET	Z
	JP	DSKSEL		;no. Select it then.
;
;   Check the drive selection and reset it to the previous
; drive if it was changed for the preceeding command.
;
RESETDR:LD	A,(CHGDRV)	;drive change indicated?
	OR	A
	RET	Z
	DEC	A		;yes, was it a different drive?
	LD	HL,CDRIVE
	CP	(HL)
	RET	Z
	LD	A,(CDRIVE)	;yes, re-select our old drive.
	JP	DSKSEL
;
;**************************************************************
;*
;*           D I R E C T O R Y   C O M M A N D
;*
;**************************************************************
;
DIRECT:	CALL	CONVFST		;convert file name.
	CALL	DSELECT		;select indicated drive.
	LD	HL,FCB+1	;was any file indicated?
	LD	A,(HL)
	CP	' '
	JP	NZ,DIRECT2
	LD	B,11		;no. Fill field with '?' - same as *.*.
DIRECT1:LD	(HL),'?'
	INC	HL
	DEC	B
	JP	NZ,DIRECT1
DIRECT2:LD	E,0		;set initial cursor position.
	PUSH	DE
	CALL	SRCHFCB		;get first file name.
	CALL	Z,NONE		;none found at all?
DIRECT3:JP	Z,DIRECT9	;terminate if no more names.
	LD	A,(RTNCODE)	;get file's position in segment (0-3).
	RRCA	
	RRCA	
	RRCA	
	AND	60H		;(A)=position*32
	LD	C,A
	LD	A,10
	CALL	EXTRACT		;extract the tenth entry in fcb.
	RLA			;check system file status bit.
	JP	C,DIRECT8	;we don't list them.
	POP	DE
	LD	A,E		;bump name count.
	INC	E
	PUSH	DE
	AND	03H		;at end of line?
	PUSH	AF
	JP	NZ,DIRECT4
	CALL	CRLF		;yes, end this line and start another.
	PUSH	BC
	CALL	GETDSK		;start line with ('A:').
	POP	BC
	ADD	A,'A'
	CALL	PRINTB
	LD	A,':'
	CALL	PRINTB
	JP	DIRECT5
DIRECT4:CALL	SPACE		;add seperator between file names.
	LD	A,':'
	CALL	PRINTB
DIRECT5:CALL	SPACE
	LD	B,1		;'extract' each file name character at a time.
DIRECT6:LD	A,B
	CALL	EXTRACT
	AND	7FH		;strip bit 7 (status bit).
	CP	' '		;are we at the end of the name?
	JP	NZ,DRECT65
	POP	AF		;yes, don't print spaces at the end of a line.
	PUSH	AF
	CP	3
	JP	NZ,DRECT63
	LD	A,9		;first check for no extension.
	CALL	EXTRACT
	AND	7FH
	CP	' '
	JP	Z,DIRECT7	;don't print spaces.
DRECT63:LD	A,' '		;else print them.
DRECT65:CALL	PRINTB
	INC	B		;bump to next character psoition.
	LD	A,B
	CP	12		;end of the name?
	JP	NC,DIRECT7
	CP	9		;nope, starting extension?
	JP	NZ,DIRECT6
	CALL	SPACE		;yes, add seperating space.
	JP	DIRECT6
DIRECT7:POP	AF		;get the next file name.
DIRECT8:CALL	CHKCON		;first check console, quit on anything.
	JP	NZ,DIRECT9
	CALL	SRCHNXT		;get next name.
	JP	DIRECT3		;and continue with our list.
DIRECT9:POP	DE		;restore the stack and return to command level.
	JP	GETBACK
;
;**************************************************************
;*
;*                E R A S E   C O M M A N D
;*
;**************************************************************
;
ERASE:	CALL	CONVFST		;convert file name.
	CP	11		;was '*.*' entered?
	JP	NZ,ERASE1
	LD	BC,YESNO	;yes, ask for confirmation.
	CALL	PLINE
	CALL	GETINP
	LD	HL,INBUFF+1
	DEC	(HL)		;must be exactly 'y'.
	JP	NZ,CMMND1
	INC	HL
	LD	A,(HL)
	CP	'Y'
	JP	NZ,CMMND1
	INC	HL
	LD	(INPOINT),HL	;save input line pointer.
ERASE1:	CALL	DSELECT		;select desired disk.
	LD	DE,FCB
	CALL	DELETE		;delete the file.
	INC	A
	CALL	Z,NONE		;not there?
	JP	GETBACK		;return to command level now.
YESNO:	DEFB	'All (y/n)?',0
;
;**************************************************************
;*
;*            T Y P E   C O M M A N D
;*
;**************************************************************
;
TYPE:	CALL	CONVFST		;convert file name.
	JP	NZ,SYNERR	;wild cards not allowed.
	CALL	DSELECT		;select indicated drive.
	CALL	OPENFCB		;open the file.
	JP	Z,TYPE5		;not there?
	CALL	CRLF		;ok, start a new line on the screen.
	LD	HL,NBYTES	;initialize byte counter.
	LD	(HL),0FFH	;set to read first sector.
TYPE1:	LD	HL,NBYTES
TYPE2:	LD	A,(HL)		;have we written the entire sector?
	CP	128
	JP	C,TYPE3
	PUSH	HL		;yes, read in the next one.
	CALL	READFCB
	POP	HL
	JP	NZ,TYPE4	;end or error?
	XOR	A		;ok, clear byte counter.
	LD	(HL),A
TYPE3:	INC	(HL)		;count this byte.
	LD	HL,TBUFF	;and get the (A)th one from the buffer (TBUFF).
	CALL	ADDHL
	LD	A,(HL)
	CP	CNTRLZ		;end of file mark?
	JP	Z,GETBACK
	CALL	PRINT		;no, print it.
	CALL	CHKCON		;check console, quit if anything ready.
	JP	NZ,GETBACK
	JP	TYPE1
;
;   Get here on an end of file or read error.
;
TYPE4:	DEC	A		;read error?
	JP	Z,GETBACK
	CALL	RDERROR		;yes, print message.
TYPE5:	CALL	RESETDR		;and reset proper drive
	JP	SYNERR		;now print file name with problem.
;
;**************************************************************
;*
;*            S A V E   C O M M A N D
;*
;**************************************************************
;
SAVE:	CALL	DECODE		;get numeric number that follows SAVE.
	PUSH	AF		;save number of pages to write.
	CALL	CONVFST		;convert file name.
	JP	NZ,SYNERR	;wild cards not allowed.
	CALL	DSELECT		;select specified drive.
	LD	DE,FCB		;now delete this file.
	PUSH	DE
	CALL	DELETE
	POP	DE
	CALL	CREATE		;and create it again.
	JP	Z,SAVE3		;can't create?
	XOR	A		;clear record number byte.
	LD	(FCB+32),A
	POP	AF		;convert pages to sectors.
	LD	L,A
	LD	H,0
	ADD	HL,HL		;(HL)=number of sectors to write.
	LD	DE,TBASE	;and we start from here.
SAVE1:	LD	A,H		;done yet?
	OR	L
	JP	Z,SAVE2
	DEC	HL		;nope, count this and compute the start
	PUSH	HL		;of the next 128 byte sector.
	LD	HL,128
	ADD	HL,DE
	PUSH	HL		;save it and set the transfer address.
	CALL	DMASET
	LD	DE,FCB		;write out this sector now.
	CALL	WRTREC
	POP	DE		;reset (DE) to the start of the last sector.
	POP	HL		;restore sector count.
	JP	NZ,SAVE3	;write error?
	JP	SAVE1
;
;   Get here after writing all of the file.
;
SAVE2:	LD	DE,FCB		;now close the file.
	CALL	CLOSE
	INC	A		;did it close ok?
	JP	NZ,SAVE4
;
;   Print out error message (no space).
;
SAVE3:	LD	BC,NOSPACE
	CALL	PLINE
SAVE4:	CALL	STDDMA		;reset the standard dma address.
	JP	GETBACK
NOSPACE:DEFB	'No space',0
;
;**************************************************************
;*
;*           R E N A M E   C O M M A N D
;*
;**************************************************************
;
RENAME:	CALL	CONVFST		;convert first file name.
	JP	NZ,SYNERR	;wild cards not allowed.
	LD	A,(CHGDRV)	;remember any change in drives specified.
	PUSH	AF
	CALL	DSELECT		;and select this drive.
	CALL	SRCHFCB		;is this file present?
	JP	NZ,RENAME6	;yes, print error message.
	LD	HL,FCB		;yes, move this name into second slot.
	LD	DE,FCB+16
	LD	B,16
	CALL	HL2DE
	LD	HL,(INPOINT)	;get input pointer.
	EX	DE,HL
	CALL	NONBLANK	;get next non blank character.
	CP	'='		;only allow an '=' or '_' seperator.
	JP	Z,RENAME1
	CP	'_'
	JP	NZ,RENAME5
RENAME1:EX	DE,HL
	INC	HL		;ok, skip seperator.
	LD	(INPOINT),HL	;save input line pointer.
	CALL	CONVFST		;convert this second file name now.
	JP	NZ,RENAME5	;again, no wild cards.
	POP	AF		;if a drive was specified, then it
	LD	B,A		;must be the same as before.
	LD	HL,CHGDRV
	LD	A,(HL)
	OR	A
	JP	Z,RENAME2
	CP	B
	LD	(HL),B
	JP	NZ,RENAME5	;they were different, error.
RENAME2:LD	(HL),B		;	reset as per the first file specification.
	XOR	A
	LD	(FCB),A		;clear the drive byte of the fcb.
RENAME3:CALL	SRCHFCB		;and go look for second file.
	JP	Z,RENAME4	;doesn't exist?
	LD	DE,FCB
	CALL	RENAM		;ok, rename the file.
	JP	GETBACK
;
;   Process rename errors here.
;
RENAME4:CALL	NONE		;file not there.
	JP	GETBACK
RENAME5:CALL	RESETDR		;bad command format.
	JP	SYNERR
RENAME6:LD	BC,EXISTS	;destination file already exists.
	CALL	PLINE
	JP	GETBACK
EXISTS:	DEFB	'File exists',0
;
;**************************************************************
;*
;*             U S E R   C O M M A N D
;*
;**************************************************************
;
USER:	CALL	DECODE		;get numeric value following command.
	CP	16		;legal user number?
	JP	NC,SYNERR
	LD	E,A		;yes but is there anything else?
	LD	A,(FCB+1)
	CP	' '
	JP	Z,SYNERR	;yes, that is not allowed.
	CALL	GETSETUC	;ok, set user code.
	JP	GETBACK1
;
;**************************************************************
;*
;*        T R A N S I A N T   P R O G R A M   C O M M A N D
;*
;**************************************************************
;
UNKNOWN:CALL	VERIFY		;check for valid system (why?).
	LD	A,(FCB+1)	;anything to execute?
	CP	' '
	JP	NZ,UNKWN1
	LD	A,(CHGDRV)	;nope, only a drive change?
	OR	A
	JP	Z,GETBACK1	;neither???
	DEC	A
	LD	(CDRIVE),A	;ok, store new drive.
	CALL	MOVECD		;set (TDRIVE) also.
	CALL	DSKSEL		;and select this drive.
	JP	GETBACK1	;then return.
;
;   Here a file name was typed. Prepare to execute it.
;
UNKWN1:	LD	DE,FCB+9	;an extension specified?
	LD	A,(DE)
	CP	' '
	JP	NZ,SYNERR	;yes, not allowed.
UNKWN2:	PUSH	DE
	CALL	DSELECT		;select specified drive.
	POP	DE
	LD	HL,COMFILE	;set the extension to 'COM'.
	CALL	MOVE3
	CALL	OPENFCB		;and open this file.
	JP	Z,UNKWN9	;not present?
;
;   Load in the program.
;
	LD	HL,TBASE	;store the program starting here.
UNKWN3:	PUSH	HL
	EX	DE,HL
	CALL	DMASET		;set transfer address.
	LD	DE,FCB		;and read the next record.
	CALL	RDREC
	JP	NZ,UNKWN4	;end of file or read error?
	POP	HL		;nope, bump pointer for next sector.
	LD	DE,128
	ADD	HL,DE
	LD	DE,CBASE	;enough room for the whole file?
	LD	A,L
	SUB	E
	LD	A,H
	SBC	A,D
	JP	NC,UNKWN0	;no, it can't fit.
	JP	UNKWN3
;
;   Get here after finished reading.
;
UNKWN4:	POP	HL
	DEC	A		;normal end of file?
	JP	NZ,UNKWN0
	CALL	RESETDR		;yes, reset previous drive.
	CALL	CONVFST		;convert the first file name that follows
	LD	HL,CHGDRV	;command name.
	PUSH	HL
	LD	A,(HL)		;set drive code in default fcb.
	LD	(FCB),A
	LD	A,16		;put second name 16 bytes later.
	CALL	CONVERT		;convert second file name.
	POP	HL
	LD	A,(HL)		;and set the drive for this second file.
	LD	(FCB+16),A
	XOR	A		;clear record byte in fcb.
	LD	(FCB+32),A
	LD	DE,TFCB		;move it into place at(005Ch).
	LD	HL,FCB
	LD	B,33
	CALL	HL2DE
	LD	HL,INBUFF+2	;now move the remainder of the input
UNKWN5:	LD	A,(HL)		;line down to (0080h). Look for a non blank.
	OR	A		;or a null.
	JP	Z,UNKWN6
	CP	' '
	JP	Z,UNKWN6
	INC	HL
	JP	UNKWN5
;
;   Do the line move now. It ends in a null byte.
;
UNKWN6:	LD	B,0		;keep a character count.
	LD	DE,TBUFF+1	;data gets put here.
UNKWN7:	LD	A,(HL)		;move it now.
	LD	(DE),A
	OR	A
	JP	Z,UNKWN8
	INC	B
	INC	HL
	INC	DE
	JP	UNKWN7
UNKWN8:	LD	A,B		;now store the character count.
	LD	(TBUFF),A
	CALL	CRLF		;clean up the screen.
	CALL	STDDMA		;set standard transfer address.
	CALL	SETCDRV		;reset current drive.
	CALL	TBASE		;and execute the program.
;
;   Transiant programs return here (or reboot).
;
	LD	SP,BATCH	;set stack first off.
	CALL	MOVECD		;move current drive into place (TDRIVE).
	CALL	DSKSEL		;and reselect it.
	JP	CMMND1		;back to comand mode.
;
;   Get here if some error occured.
;
UNKWN9:	CALL	RESETDR		;inproper format.
	JP	SYNERR
UNKWN0:	LD	BC,BADLOAD	;read error or won't fit.
	CALL	PLINE
	JP	GETBACK
BADLOAD:DEFB	'Bad load',0
COMFILE:DEFB	'COM'		;command file extension.
;
;   Get here to return to command level. We will reset the
; previous active drive and then either return to command
; level directly or print error message and then return.
;
GETBACK:CALL	RESETDR		;reset previous drive.
GETBACK1: CALL	CONVFST		;convert first name in (FCB).
	LD	A,(FCB+1)	;if this was just a drive change request,
	SUB	' '		;make sure it was valid.
	LD	HL,CHGDRV
	OR	(HL)
	JP	NZ,SYNERR
	JP	CMMND1		;ok, return to command level.
;
;   ccp stack area.
;
	DEFB	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
CCPSTACK EQU	$	;end of ccp stack area.
;
;   Batch (or SUBMIT) processing information storage.
;
BATCH:	DEFB	0		;batch mode flag (0=not active).
BATCHFCB: DEFB	0,'$$$     SUB',0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
;
;   File control block setup by the CCP.
;
FCB:	DEFB	0,'           ',0,0,0,0,0,'           ',0,0,0,0,0
RTNCODE:DEFB	0		;status returned from bdos call.
CDRIVE:	DEFB	0		;currently active drive.
CHGDRV:	DEFB	0		;change in drives flag (0=no change).
NBYTES:	DEFW	0		;byte counter used by TYPE.
;
;   Room for expansion?
;
	DEFB	0,0,0,0,0,0,0,0,0,0,0,0,0
;
;   Note that the following six bytes must match those at
; (PATTRN1) or cp/m will HALT. Why?
;
PATTRN2:DEFB	0,22,0,0,0,0	;(* serial number bytes *).
;
;**************************************************************
;*
;*                    B D O S   E N T R Y
;*
;**************************************************************
;
FBASE:	JP	FBASE1
;
;   Bdos error table.
;
BADSCTR:DEFW	ERROR1		;bad sector on read or write.
BADSLCT:DEFW	ERROR2		;bad disk select.
RODISK:	DEFW	ERROR3		;disk is read only.
ROFILE:	DEFW	ERROR4		;file is read only.
;
;   Entry into bdos. (DE) or (E) are the parameters passed. The
; function number desired is in register (C).
;
FBASE1:	EX	DE,HL		;save the (DE) parameters.
	LD	(PARAMS),HL
	EX	DE,HL
	LD	A,E		;and save register (E) in particular.
	LD	(EPARAM),A
	LD	HL,0
	LD	(STATUS),HL	;clear return status.
	ADD	HL,SP
	LD	(USRSTACK),HL	;save users stack pointer.
	LD	SP,STKAREA	;and set our own.
	XOR	A		;clear auto select storage space.
	LD	(AUTOFLAG),A
	LD	(AUTO),A
	LD	HL,GOBACK	;set return address.
	PUSH	HL
	LD	A,C		;get function number.
	CP	NFUNCTS		;valid function number?
	RET	NC
	LD	C,E		;keep single register function here.
	LD	HL,FUNCTNS	;now look thru the function table.
	LD	E,A
	LD	D,0		;(DE)=function number.
	ADD	HL,DE
	ADD	HL,DE		;(HL)=(start of table)+2*(function number).
	LD	E,(HL)
	INC	HL
	LD	D,(HL)		;now (DE)=address for this function.
	LD	HL,(PARAMS)	;retrieve parameters.
	EX	DE,HL		;now (DE) has the original parameters.
	JP	(HL)		;execute desired function.
;
;   BDOS function jump table.
;
NFUNCTS EQU	41		;number of functions in followin table.
;
FUNCTNS:DEFW	WBOOT,GETCON,OUTCON,GETRDR,PUNCH,LIST,DIRCIO,GETIOB
	DEFW	SETIOB,PRTSTR,RDBUFF,GETCSTS,GETVER,RSTDSK,SETDSK,OPENFIL
	DEFW	CLOSEFIL,GETFST,GETNXT,DELFILE,READSEQ,WRTSEQ,FCREATE
	DEFW	RENFILE,GETLOG,GETCRNT,PUTDMA,GETALOC,WRTPRTD,GETROV,SETATTR
	DEFW	GETPARM,GETUSER,RDRANDOM,WTRANDOM,FILESIZE,SETRAN,LOGOFF,RTN
	DEFW	RTN,WTSPECL
;
;   Bdos error message section.
;
ERROR1:	LD	HL,BADSEC	;bad sector message.
	CALL	PRTERR		;print it and get a 1 char responce.
	CP	CNTRLC		;re-boot request (control-c)?
	JP	Z,0		;yes.
	RET			;no, return to retry i/o function.
;
ERROR2:	LD	HL,BADSEL	;bad drive selected.
	JP	ERROR5
;
ERROR3:	LD	HL,DISKRO	;disk is read only.
	JP	ERROR5
;
ERROR4:	LD	HL,FILERO	;file is read only.
;
ERROR5:	CALL	PRTERR
	JP	0		;always reboot on these errors.
;
BDOSERR:DEFB	'Bdos Err On '
BDOSDRV:DEFB	' : $'
BADSEC:	DEFB	'Bad Sector$'
BADSEL:	DEFB	'Select$'
FILERO:	DEFB	'File '
DISKRO:	DEFB	'R/O$'
;
;   Print bdos error message.
;
PRTERR:	PUSH	HL		;save second message pointer.
	CALL	OUTCRLF		;send (cr)(lf).
	LD	A,(ACTIVE)	;get active drive.
	ADD	A,'A'		;make ascii.
	LD	(BDOSDRV),A	;and put in message.
	LD	BC,BDOSERR	;and print it.
	CALL	PRTMESG
	POP	BC		;print second message line now.
	CALL	PRTMESG
;
;   Get an input character. We will check our 1 character
; buffer first. This may be set by the console status routine.
;
GETCHAR:LD	HL,CHARBUF	;check character buffer.
	LD	A,(HL)		;anything present already?
	LD	(HL),0		;...either case clear it.
	OR	A
	RET	NZ		;yes, use it.
	JP	CONIN		;nope, go get a character responce.
;
;   Input and echo a character.
;
GETECHO:CALL	GETCHAR		;input a character.
	CALL	CHKCHAR		;carriage control?
	RET	C		;no, a regular control char so don't echo.
	PUSH	AF		;ok, save character now.
	LD	C,A
	CALL	OUTCON		;and echo it.
	POP	AF		;get character and return.
	RET	
;
;   Check character in (A). Set the zero flag on a carriage
; control character and the carry flag on any other control
; character.
;
CHKCHAR:CP	CR		;check for carriage return, line feed, backspace,
	RET	Z		;or a tab.
	CP	LF
	RET	Z
	CP	TAB
	RET	Z
	CP	BS
	RET	Z
	CP	' '		;other control char? Set carry flag.
	RET	
;
;   Check the console during output. Halt on a control-s, then
; reboot on a control-c. If anything else is ready, clear the
; zero flag and return (the calling routine may want to do
; something).
;
CKCONSOL: LD	A,(CHARBUF)	;check buffer.
	OR	A		;if anything, just return without checking.
	JP	NZ,CKCON2
	CALL	CONST		;nothing in buffer. Check console.
	AND	01H		;look at bit 0.
	RET	Z		;return if nothing.
	CALL	CONIN		;ok, get it.
	CP	CNTRLS		;if not control-s, return with zero cleared.
	JP	NZ,CKCON1
	CALL	CONIN		;halt processing until another char
	CP	CNTRLC		;is typed. Control-c?
	JP	Z,0		;yes, reboot now.
	XOR	A		;no, just pretend nothing was ever ready.
	RET	
CKCON1:	LD	(CHARBUF),A	;save character in buffer for later processing.
CKCON2:	LD	A,1		;set (A) to non zero to mean something is ready.
	RET	
;
;   Output (C) to the screen. If the printer flip-flop flag
; is set, we will send character to printer also. The console
; will be checked in the process.
;
OUTCHAR:LD	A,(OUTFLAG)	;check output flag.
	OR	A		;anything and we won't generate output.
	JP	NZ,OUTCHR1
	PUSH	BC
	CALL	CKCONSOL	;check console (we don't care whats there).
	POP	BC
	PUSH	BC
	CALL	CONOUT		;output (C) to the screen.
	POP	BC
	PUSH	BC
	LD	A,(PRTFLAG)	;check printer flip-flop flag.
	OR	A
	CALL	NZ,LIST		;print it also if non-zero.
	POP	BC
OUTCHR1:LD	A,C		;update cursors position.
	LD	HL,CURPOS
	CP	DEL		;rubouts don't do anything here.
	RET	Z
	INC	(HL)		;bump line pointer.
	CP	' '		;and return if a normal character.
	RET	NC
	DEC	(HL)		;restore and check for the start of the line.
	LD	A,(HL)
	OR	A
	RET	Z		;ingnore control characters at the start of the line.
	LD	A,C
	CP	BS		;is it a backspace?
	JP	NZ,OUTCHR2
	DEC	(HL)		;yes, backup pointer.
	RET	
OUTCHR2:CP	LF		;is it a line feed?
	RET	NZ		;ignore anything else.
	LD	(HL),0		;reset pointer to start of line.
	RET	
;
;   Output (A) to the screen. If it is a control character
; (other than carriage control), use ^x format.
;
SHOWIT:	LD	A,C
	CALL	CHKCHAR		;check character.
	JP	NC,OUTCON	;not a control, use normal output.
	PUSH	AF
	LD	C,'^'		;for a control character, preceed it with '^'.
	CALL	OUTCHAR
	POP	AF
	OR	'@'		;and then use the letter equivelant.
	LD	C,A
;
;   Function to output (C) to the console device and expand tabs
; if necessary.
;
OUTCON:	LD	A,C
	CP	TAB		;is it a tab?
	JP	NZ,OUTCHAR	;use regular output.
OUTCON1:LD	C,' '		;yes it is, use spaces instead.
	CALL	OUTCHAR
	LD	A,(CURPOS)	;go until the cursor is at a multiple of 8

	AND	07H		;position.
	JP	NZ,OUTCON1
	RET	
;
;   Echo a backspace character. Erase the prevoius character
; on the screen.
;
BACKUP:	CALL	BACKUP1		;backup the screen 1 place.
	LD	C,' '		;then blank that character.
	CALL	CONOUT
BACKUP1:LD	C,BS		;then back space once more.
	JP	CONOUT
;
;   Signal a deleted line. Print a '#' at the end and start
; over.
;
NEWLINE:LD	C,'#'
	CALL	OUTCHAR		;print this.
	CALL	OUTCRLF		;start new line.
NEWLN1:	LD	A,(CURPOS)	;move the cursor to the starting position.
	LD	HL,STARTING
	CP	(HL)
	RET	NC		;there yet?
	LD	C,' '
	CALL	OUTCHAR		;nope, keep going.
	JP	NEWLN1
;
;   Output a (cr) (lf) to the console device (screen).
;
OUTCRLF:LD	C,CR
	CALL	OUTCHAR
	LD	C,LF
	JP	OUTCHAR
;
;   Print message pointed to by (BC). It will end with a '$'.
;
PRTMESG:LD	A,(BC)		;check for terminating character.
	CP	'$'
	RET	Z
	INC	BC
	PUSH	BC		;otherwise, bump pointer and print it.
	LD	C,A
	CALL	OUTCON
	POP	BC
	JP	PRTMESG
;
;   Function to execute a buffered read.
;
RDBUFF:	LD	A,(CURPOS)	;use present location as starting one.
	LD	(STARTING),A
	LD	HL,(PARAMS)	;get the maximum buffer space.
	LD	C,(HL)
	INC	HL		;point to first available space.
	PUSH	HL		;and save.
	LD	B,0		;keep a character count.
RDBUF1:	PUSH	BC
	PUSH	HL
RDBUF2:	CALL	GETCHAR		;get the next input character.
	AND	7FH		;strip bit 7.
	POP	HL		;reset registers.
	POP	BC
	CP	CR		;en of the line?
	JP	Z,RDBUF17
	CP	LF
	JP	Z,RDBUF17
	CP	BS		;how about a backspace?
	JP	NZ,RDBUF3
	LD	A,B		;yes, but ignore at the beginning of the line.
	OR	A
	JP	Z,RDBUF1
	DEC	B		;ok, update counter.
	LD	A,(CURPOS)	;if we backspace to the start of the line,
	LD	(OUTFLAG),A	;treat as a cancel (control-x).
	JP	RDBUF10
RDBUF3:	CP	DEL		;user typed a rubout?
	JP	NZ,RDBUF4
	LD	A,B		;ignore at the start of the line.
	OR	A
	JP	Z,RDBUF1
	LD	A,(HL)		;ok, echo the prevoius character.
	DEC	B		;and reset pointers (counters).
	DEC	HL
	JP	RDBUF15
RDBUF4:	CP	CNTRLE		;physical end of line?
	JP	NZ,RDBUF5
	PUSH	BC		;yes, do it.
	PUSH	HL
	CALL	OUTCRLF
	XOR	A		;and update starting position.
	LD	(STARTING),A
	JP	RDBUF2
RDBUF5:	CP	CNTRLP		;control-p?
	JP	NZ,RDBUF6
	PUSH	HL		;yes, flip the print flag filp-flop byte.
	LD	HL,PRTFLAG
	LD	A,1		;PRTFLAG=1-PRTFLAG
	SUB	(HL)
	LD	(HL),A
	POP	HL
	JP	RDBUF1
RDBUF6:	CP	CNTRLX		;control-x (cancel)?
	JP	NZ,RDBUF8
	POP	HL
RDBUF7:	LD	A,(STARTING)	;yes, backup the cursor to here.
	LD	HL,CURPOS
	CP	(HL)
	JP	NC,RDBUFF	;done yet?
	DEC	(HL)		;no, decrement pointer and output back up one space.
	CALL	BACKUP
	JP	RDBUF7
RDBUF8:	CP	CNTRLU		;cntrol-u (cancel line)?
	JP	NZ,RDBUF9
	CALL	NEWLINE		;start a new line.
	POP	HL
	JP	RDBUFF
RDBUF9:	CP	CNTRLR		;control-r?
	JP	NZ,RDBUF14
RDBUF10:PUSH	BC		;yes, start a new line and retype the old one.
	CALL	NEWLINE
	POP	BC
	POP	HL
	PUSH	HL
	PUSH	BC
RDBUF11:LD	A,B		;done whole line yet?
	OR	A
	JP	Z,RDBUF12
	INC	HL		;nope, get next character.
	LD	C,(HL)
	DEC	B		;count it.
	PUSH	BC
	PUSH	HL
	CALL	SHOWIT		;and display it.
	POP	HL
	POP	BC
	JP	RDBUF11
RDBUF12:PUSH	HL		;done with line. If we were displaying
	LD	A,(OUTFLAG)	;then update cursor position.
	OR	A
	JP	Z,RDBUF2
	LD	HL,CURPOS	;because this line is shorter, we must
	SUB	(HL)		;back up the cursor (not the screen however)
	LD	(OUTFLAG),A	;some number of positions.
RDBUF13:CALL	BACKUP		;note that as long as (OUTFLAG) is non
	LD	HL,OUTFLAG	;zero, the screen will not be changed.
	DEC	(HL)
	JP	NZ,RDBUF13
	JP	RDBUF2		;now just get the next character.
;
;   Just a normal character, put this in our buffer and echo.
;
RDBUF14:INC	HL
	LD	(HL),A		;store character.
	INC	B		;and count it.
RDBUF15:PUSH	BC
	PUSH	HL
	LD	C,A		;echo it now.
	CALL	SHOWIT
	POP	HL
	POP	BC
	LD	A,(HL)		;was it an abort request?
	CP	CNTRLC		;control-c abort?
	LD	A,B
	JP	NZ,RDBUF16
	CP	1		;only if at start of line.
	JP	Z,0
RDBUF16:CP	C		;nope, have we filled the buffer?
	JP	C,RDBUF1
RDBUF17:POP	HL		;yes end the line and return.
	LD	(HL),B
	LD	C,CR
	JP	OUTCHAR		;output (cr) and return.
;
;   Function to get a character from the console device.
;
GETCON:	CALL	GETECHO		;get and echo.
	JP	SETSTAT		;save status and return.
;
;   Function to get a character from the tape reader device.
;
GETRDR:	CALL	READER		;get a character from reader, set status and return.
	JP	SETSTAT
;
;  Function to perform direct console i/o. If (C) contains (FF)
; then this is an input request. If (C) contains (FE) then
; this is a status request. Otherwise we are to output (C).
;
DIRCIO:	LD	A,C		;test for (FF).
	INC	A
	JP	Z,DIRC1
	INC	A		;test for (FE).
	JP	Z,CONST
	JP	CONOUT		;just output (C).
DIRC1:	CALL	CONST		;this is an input request.
	OR	A
	JP	Z,GOBACK1	;not ready? Just return (directly).
	CALL	CONIN		;yes, get character.
	JP	SETSTAT		;set status and return.
;
;   Function to return the i/o byte.
;
GETIOB:	LD	A,(IOBYTE)
	JP	SETSTAT
;
;   Function to set the i/o byte.
;
SETIOB:	LD	HL,IOBYTE
	LD	(HL),C
	RET	
;
;   Function to print the character string pointed to by (DE)
; on the console device. The string ends with a '$'.
;
PRTSTR:	EX	DE,HL
	LD	C,L
	LD	B,H		;now (BC) points to it.
	JP	PRTMESG
;
;   Function to interigate the console device.
;
GETCSTS:CALL	CKCONSOL
;
;   Get here to set the status and return to the cleanup
; section. Then back to the user.
;
SETSTAT:LD	(STATUS),A
RTN:	RET	
;
;   Set the status to 1 (read or write error code).
;
IOERR1:	LD	A,1
	JP	SETSTAT
;
OUTFLAG:DEFB	0		;output flag (non zero means no output).
STARTING: DEFB	2		;starting position for cursor.
CURPOS:	DEFB	0		;cursor position (0=start of line).
PRTFLAG:DEFB	0		;printer flag (control-p toggle). List if non zero.
CHARBUF:DEFB	0		;single input character buffer.
;
;   Stack area for BDOS calls.
;
USRSTACK: DEFW	0		;save users stack pointer here.
;
	DEFB	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	DEFB	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
STKAREA EQU	$		;end of stack area.
;
USERNO:	DEFB	0		;current user number.
ACTIVE:	DEFB	0		;currently active drive.
PARAMS:	DEFW	0		;save (DE) parameters here on entry.
STATUS:	DEFW	0		;status returned from bdos function.
;
;   Select error occured, jump to error routine.
;
SLCTERR: LD	HL,BADSLCT
;
;   Jump to (HL) indirectly.
;
JUMPHL:	LD	E,(HL)
	INC	HL
	LD	D,(HL)		;now (DE) contain the desired address.
	EX	DE,HL
	JP	(HL)
;
;   Block move. (DE) to (HL), (C) bytes total.
;
DE2HL:	INC	C		;is count down to zero?
DE2HL1:	DEC	C
	RET	Z		;yes, we are done.
	LD	A,(DE)		;no, move one more byte.
	LD	(HL),A
	INC	DE
	INC	HL
	JP	DE2HL1		;and repeat.
;
;   Select the desired drive.
;
SELECT:	LD	A,(ACTIVE)	;get active disk.
	LD	C,A
	CALL	SELDSK		;select it.
	LD	A,H		;valid drive?
	OR	L		;valid drive?
	RET	Z		;return if not.
;
;   Here, the BIOS returned the address of the parameter block
; in (HL). We will extract the necessary pointers and save them.
;
	LD	E,(HL)		;yes, get address of translation table into (DE).
	INC	HL
	LD	D,(HL)
	INC	HL
	LD	(SCRATCH1),HL	;save pointers to scratch areas.
	INC	HL
	INC	HL
	LD	(SCRATCH2),HL	;ditto.
	INC	HL
	INC	HL
	LD	(SCRATCH3),HL	;ditto.
	INC	HL
	INC	HL
	EX	DE,HL		;now save the translation table address.
	LD	(XLATE),HL
	LD	HL,DIRBUF	;put the next 8 bytes here.
	LD	C,8		;they consist of the directory buffer
	CALL	DE2HL		;pointer, parameter block pointer,
	LD	HL,(DISKPB)	;check and allocation vectors.
	EX	DE,HL
	LD	HL,SECTORS	;move parameter block into our ram.
	LD	C,15		;it is 15 bytes long.
	CALL	DE2HL
	LD	HL,(DSKSIZE)	;check disk size.
	LD	A,H		;more than 256 blocks on this?
	LD	HL,BIGDISK
	LD	(HL),0FFH	;set to samll.
	OR	A
	JP	Z,SELECT1
	LD	(HL),0		;wrong, set to large.
SELECT1:LD	A,0FFH		;clear the zero flag.
	OR	A
	RET	
;
;   Routine to home the disk track head and clear pointers.
;
HOMEDRV:CALL	HOME		;home the head.
	XOR	A
	LD	HL,(SCRATCH2)	;set our track pointer also.
	LD	(HL),A
	INC	HL
	LD	(HL),A
	LD	HL,(SCRATCH3)	;and our sector pointer.
	LD	(HL),A
	INC	HL
	LD	(HL),A
	RET	
;
;   Do the actual disk read and check the error return status.
;
DOREAD:	CALL	READ
	JP	IORET
;
;   Do the actual disk write and handle any bios error.
;
DOWRITE:CALL	WRITE
IORET:	OR	A
	RET	Z		;return unless an error occured.
	LD	HL,BADSCTR	;bad read/write on this sector.
	JP	JUMPHL
;
;   Routine to select the track and sector that the desired
; block number falls in.
;
TRKSEC:	LD	HL,(FILEPOS)	;get position of last accessed file
	LD	C,2		;in directory and compute sector #.
	CALL	SHIFTR		;sector #=file-position/4.
	LD	(BLKNMBR),HL	;save this as the block number of interest.
	LD	(CKSUMTBL),HL	;what's it doing here too?
;
;   if the sector number has already been set (BLKNMBR), enter
; at this point.
;
TRKSEC1:LD	HL,BLKNMBR
	LD	C,(HL)		;move sector number into (BC).
	INC	HL
	LD	B,(HL)
	LD	HL,(SCRATCH3)	;get current sector number and
	LD	E,(HL)		;move this into (DE).
	INC	HL
	LD	D,(HL)
	LD	HL,(SCRATCH2)	;get current track number.
	LD	A,(HL)		;and this into (HL).
	INC	HL
	LD	H,(HL)
	LD	L,A
TRKSEC2:LD	A,C		;is desired sector before current one?
	SUB	E
	LD	A,B
	SBC	A,D
	JP	NC,TRKSEC3
	PUSH	HL		;yes, decrement sectors by one track.
	LD	HL,(SECTORS)	;get sectors per track.
	LD	A,E
	SUB	L
	LD	E,A
	LD	A,D
	SBC	A,H
	LD	D,A		;now we have backed up one full track.
	POP	HL
	DEC	HL		;adjust track counter.
	JP	TRKSEC2
TRKSEC3:PUSH	HL		;desired sector is after current one.
	LD	HL,(SECTORS)	;get sectors per track.
	ADD	HL,DE		;bump sector pointer to next track.
	JP	C,TRKSEC4
	LD	A,C		;is desired sector now before current one?
	SUB	L
	LD	A,B
	SBC	A,H
	JP	C,TRKSEC4
	EX	DE,HL		;not yes, increment track counter
	POP	HL		;and continue until it is.
	INC	HL
	JP	TRKSEC3
;
;   here we have determined the track number that contains the
; desired sector.
;
TRKSEC4:POP	HL		;get track number (HL).
	PUSH	BC
	PUSH	DE
	PUSH	HL
	EX	DE,HL
	LD	HL,(OFFSET)	;adjust for first track offset.
	ADD	HL,DE
	LD	B,H
	LD	C,L
	CALL	SETTRK		;select this track.
	POP	DE		;reset current track pointer.
	LD	HL,(SCRATCH2)
	LD	(HL),E
	INC	HL
	LD	(HL),D
	POP	DE
	LD	HL,(SCRATCH3)	;reset the first sector on this track.
	LD	(HL),E
	INC	HL
	LD	(HL),D
	POP	BC
	LD	A,C		;now subtract the desired one.
	SUB	E		;to make it relative (1-# sectors/track).
	LD	C,A
	LD	A,B
	SBC	A,D
	LD	B,A
	LD	HL,(XLATE)	;translate this sector according to this table.
	EX	DE,HL
	CALL	SECTRN		;let the bios translate it.
	LD	C,L
	LD	B,H
	JP	SETSEC		;and select it.
;
;   Compute block number from record number (SAVNREC) and
; extent number (SAVEXT).
;
GETBLOCK: LD	HL,BLKSHFT	;get logical to physical conversion.
	LD	C,(HL)		;note that this is base 2 log of ratio.
	LD	A,(SAVNREC)	;get record number.
GETBLK1:OR	A		;compute (A)=(A)/2^BLKSHFT.
	RRA	
	DEC	C
	JP	NZ,GETBLK1
	LD	B,A		;save result in (B).
	LD	A,8
	SUB	(HL)
	LD	C,A		;compute (C)=8-BLKSHFT.
	LD	A,(SAVEXT)
GETBLK2:DEC	C		;compute (A)=SAVEXT*2^(8-BLKSHFT).
	JP	Z,GETBLK3
	OR	A
	RLA	
	JP	GETBLK2
GETBLK3:ADD	A,B
	RET	
;
;   Routine to extract the (BC) block byte from the fcb pointed
; to by (PARAMS). If this is a big-disk, then these are 16 bit
; block numbers, else they are 8 bit numbers.
; Number is returned in (HL).
;
EXTBLK:	LD	HL,(PARAMS)	;get fcb address.
	LD	DE,16		;block numbers start 16 bytes into fcb.
	ADD	HL,DE
	ADD	HL,BC
	LD	A,(BIGDISK)	;are we using a big-disk?
	OR	A
	JP	Z,EXTBLK1
	LD	L,(HL)		;no, extract an 8 bit number from the fcb.
	LD	H,0
	RET	
EXTBLK1:ADD	HL,BC		;yes, extract a 16 bit number.
	LD	E,(HL)
	INC	HL
	LD	D,(HL)
	EX	DE,HL		;return in (HL).
	RET	
;
;   Compute block number.
;
COMBLK:	CALL	GETBLOCK
	LD	C,A
	LD	B,0
	CALL	EXTBLK
	LD	(BLKNMBR),HL
	RET	
;
;   Check for a zero block number (unused).
;
CHKBLK:	LD	HL,(BLKNMBR)
	LD	A,L		;is it zero?
	OR	H
	RET	
;
;   Adjust physical block (BLKNMBR) and convert to logical
; sector (LOGSECT). This is the starting sector of this block.
; The actual sector of interest is then added to this and the
; resulting sector number is stored back in (BLKNMBR). This
; will still have to be adjusted for the track number.
;
LOGICAL:LD	A,(BLKSHFT)	;get log2(physical/logical sectors).
	LD	HL,(BLKNMBR)	;get physical sector desired.
LOGICL1:ADD	HL,HL		;compute logical sector number.
	DEC	A		;note logical sectors are 128 bytes long.
	JP	NZ,LOGICL1
	LD	(LOGSECT),HL	;save logical sector.
	LD	A,(BLKMASK)	;get block mask.
	LD	C,A
	LD	A,(SAVNREC)	;get next sector to access.
	AND	C		;extract the relative position within physical block.
	OR	L		;and add it too logical sector.
	LD	L,A
	LD	(BLKNMBR),HL	;and store.
	RET	
;
;   Set (HL) to point to extent byte in fcb.
;
SETEXT:	LD	HL,(PARAMS)
	LD	DE,12		;it is the twelth byte.
	ADD	HL,DE
	RET	
;
;   Set (HL) to point to record count byte in fcb and (DE) to
; next record number byte.
;
SETHLDE:LD	HL,(PARAMS)
	LD	DE,15		;record count byte (#15).
	ADD	HL,DE
	EX	DE,HL
	LD	HL,17		;next record number (#32).
	ADD	HL,DE
	RET	
;
;   Save current file data from fcb.
;
STRDATA:CALL	SETHLDE
	LD	A,(HL)		;get and store record count byte.
	LD	(SAVNREC),A
	EX	DE,HL
	LD	A,(HL)		;get and store next record number byte.
	LD	(SAVNXT),A
	CALL	SETEXT		;point to extent byte.
	LD	A,(EXTMASK)	;get extent mask.
	AND	(HL)
	LD	(SAVEXT),A	;and save extent here.
	RET	
;
;   Set the next record to access. If (MODE) is set to 2, then
; the last record byte (SAVNREC) has the correct number to access.
; For sequential access, (MODE) will be equal to 1.
;
SETNREC:CALL	SETHLDE
	LD	A,(MODE)	;get sequential flag (=1).
	CP	2		;a 2 indicates that no adder is needed.
	JP	NZ,STNREC1
	XOR	A		;clear adder (random access?).
STNREC1:LD	C,A
	LD	A,(SAVNREC)	;get last record number.
	ADD	A,C		;increment record count.
	LD	(HL),A		;and set fcb's next record byte.
	EX	DE,HL
	LD	A,(SAVNXT)	;get next record byte from storage.
	LD	(HL),A		;and put this into fcb as number of records used.
	RET	
;
;   Shift (HL) right (C) bits.
;
SHIFTR:	INC	C
SHIFTR1:DEC	C
	RET	Z
	LD	A,H
	OR	A
	RRA	
	LD	H,A
	LD	A,L
	RRA	
	LD	L,A
	JP	SHIFTR1
;
;   Compute the check-sum for the directory buffer. Return
; integer sum in (A).
;
CHECKSUM: LD	C,128		;length of buffer.
	LD	HL,(DIRBUF)	;get its location.
	XOR	A		;clear summation byte.
CHKSUM1:ADD	A,(HL)		;and compute sum ignoring carries.
	INC	HL
	DEC	C
	JP	NZ,CHKSUM1
	RET	
;
;   Shift (HL) left (C) bits.
;
SHIFTL:	INC	C
SHIFTL1:DEC	C
	RET	Z
	ADD	HL,HL		;shift left 1 bit.
	JP	SHIFTL1
;
;   Routine to set a bit in a 16 bit value contained in (BC).
; The bit set depends on the current drive selection.
;
SETBIT:	PUSH	BC		;save 16 bit word.
	LD	A,(ACTIVE)	;get active drive.
	LD	C,A
	LD	HL,1
	CALL	SHIFTL		;shift bit 0 into place.
	POP	BC		;now 'or' this with the original word.
	LD	A,C
	OR	L
	LD	L,A		;low byte done, do high byte.
	LD	A,B
	OR	H
	LD	H,A
	RET	
;
;   Extract the write protect status bit for the current drive.
; The result is returned in (A), bit 0.
;
GETWPRT:LD	HL,(WRTPRT)	;get status bytes.
	LD	A,(ACTIVE)	;which drive is current?
	LD	C,A
	CALL	SHIFTR		;shift status such that bit 0 is the
	LD	A,L		;one of interest for this drive.
	AND	01H		;and isolate it.
	RET	
;
;   Function to write protect the current disk.
;
WRTPRTD:LD	HL,WRTPRT	;point to status word.
	LD	C,(HL)		;set (BC) equal to the status.
	INC	HL
	LD	B,(HL)
	CALL	SETBIT		;and set this bit according to current drive.
	LD	(WRTPRT),HL	;then save.
	LD	HL,(DIRSIZE)	;now save directory size limit.
	INC	HL		;remember the last one.
	EX	DE,HL
	LD	HL,(SCRATCH1)	;and store it here.
	LD	(HL),E		;put low byte.
	INC	HL
	LD	(HL),D		;then high byte.
	RET	
;
;   Check for a read only file.
;
CHKROFL:CALL	FCB2HL		;set (HL) to file entry in directory buffer.
CKROF1:	LD	DE,9		;look at bit 7 of the ninth byte.
	ADD	HL,DE
	LD	A,(HL)
	RLA	
	RET	NC		;return if ok.
	LD	HL,ROFILE	;else, print error message and terminate.
	JP	JUMPHL
;
;   Check the write protect status of the active disk.
;
CHKWPRT:CALL	GETWPRT
	RET	Z		;return if ok.
	LD	HL,RODISK	;else print message and terminate.
	JP	JUMPHL
;
;   Routine to set (HL) pointing to the proper entry in the
; directory buffer.
;
FCB2HL:	LD	HL,(DIRBUF)	;get address of buffer.
	LD	A,(FCBPOS)	;relative position of file.
;
;   Routine to add (A) to (HL).
;
ADDA2HL:ADD	A,L
	LD	L,A
	RET	NC
	INC	H		;take care of any carry.
	RET	
;
;   Routine to get the 's2' byte from the fcb supplied in
; the initial parameter specification.
;
GETS2:	LD	HL,(PARAMS)	;get address of fcb.
	LD	DE,14		;relative position of 's2'.
	ADD	HL,DE
	LD	A,(HL)		;extract this byte.
	RET	
;
;   Clear the 's2' byte in the fcb.
;
CLEARS2:CALL	GETS2		;this sets (HL) pointing to it.
	LD	(HL),0		;now clear it.
	RET	
;
;   Set bit 7 in the 's2' byte of the fcb.
;
SETS2B7:CALL	GETS2		;get the byte.
	OR	80H		;and set bit 7.
	LD	(HL),A		;then store.
	RET	
;
;   Compare (FILEPOS) with (SCRATCH1) and set flags based on
; the difference. This checks to see if there are more file
; names in the directory. We are at (FILEPOS) and there are
; (SCRATCH1) of them to check.
;
MOREFLS:LD	HL,(FILEPOS)	;we are here.
	EX	DE,HL
	LD	HL,(SCRATCH1)	;and don't go past here.
	LD	A,E		;compute difference but don't keep.
	SUB	(HL)
	INC	HL
	LD	A,D
	SBC	A,(HL)		;set carry if no more names.
	RET	
;
;   Call this routine to prevent (SCRATCH1) from being greater
; than (FILEPOS).
;
CHKNMBR:CALL	MOREFLS		;SCRATCH1 too big?
	RET	C
	INC	DE		;yes, reset it to (FILEPOS).
	LD	(HL),D
	DEC	HL
	LD	(HL),E
	RET	
;
;   Compute (HL)=(DE)-(HL)
;
SUBHL:	LD	A,E		;compute difference.
	SUB	L
	LD	L,A		;store low byte.
	LD	A,D
	SBC	A,H
	LD	H,A		;and then high byte.
	RET	
;
;   Set the directory checksum byte.
;
SETDIR:	LD	C,0FFH
;
;   Routine to set or compare the directory checksum byte. If
; (C)=0ffh, then this will set the checksum byte. Else the byte
; will be checked. If the check fails (the disk has been changed),
; then this disk will be write protected.
;
CHECKDIR: LD	HL,(CKSUMTBL)
	EX	DE,HL
	LD	HL,(ALLOC1)
	CALL	SUBHL
	RET	NC		;ok if (CKSUMTBL) > (ALLOC1), so return.
	PUSH	BC
	CALL	CHECKSUM	;else compute checksum.
	LD	HL,(CHKVECT)	;get address of checksum table.
	EX	DE,HL
	LD	HL,(CKSUMTBL)
	ADD	HL,DE		;set (HL) to point to byte for this drive.
	POP	BC
	INC	C		;set or check ?
	JP	Z,CHKDIR1
	CP	(HL)		;check them.
	RET	Z		;return if they are the same.
	CALL	MOREFLS		;not the same, do we care?
	RET	NC
	CALL	WRTPRTD		;yes, mark this as write protected.
	RET	
CHKDIR1:LD	(HL),A		;just set the byte.
	RET	
;
;   Do a write to the directory of the current disk.
;
DIRWRITE: CALL	SETDIR		;set checksum byte.
	CALL	DIRDMA		;set directory dma address.
	LD	C,1		;tell the bios to actually write.
	CALL	DOWRITE		;then do the write.
	JP	DEFDMA
;
;   Read from the directory.
;
DIRREAD:CALL	DIRDMA		;set the directory dma address.
	CALL	DOREAD		;and read it.
;
;   Routine to set the dma address to the users choice.
;
DEFDMA:	LD	HL,USERDMA	;reset the default dma address and return.
	JP	DIRDMA1
;
;   Routine to set the dma address for directory work.
;
DIRDMA:	LD	HL,DIRBUF
;
;   Set the dma address. On entry, (HL) points to
; word containing the desired dma address.
;
DIRDMA1:LD	C,(HL)
	INC	HL
	LD	B,(HL)		;setup (BC) and go to the bios to set it.
	JP	SETDMA
;
;   Move the directory buffer into user's dma space.
;
MOVEDIR:LD	HL,(DIRBUF)	;buffer is located here, and
	EX	DE,HL
	LD	HL,(USERDMA)	; put it here.
	LD	C,128		;this is its length.
	JP	DE2HL		;move it now and return.
;
;   Check (FILEPOS) and set the zero flag if it equals 0ffffh.
;
CKFILPOS: LD	HL,FILEPOS
	LD	A,(HL)
	INC	HL
	CP	(HL)		;are both bytes the same?
	RET	NZ
	INC	A		;yes, but are they each 0ffh?
	RET	
;
;   Set location (FILEPOS) to 0ffffh.
;
STFILPOS: LD	HL,0FFFFH
	LD	(FILEPOS),HL
	RET	
;
;   Move on to the next file position within the current
; directory buffer. If no more exist, set pointer to 0ffffh
; and the calling routine will check for this. Enter with (C)
; equal to 0ffh to cause the checksum byte to be set, else we
; will check this disk and set write protect if checksums are
; not the same (applies only if another directory sector must
; be read).
;
NXENTRY:LD	HL,(DIRSIZE)	;get directory entry size limit.
	EX	DE,HL
	LD	HL,(FILEPOS)	;get current count.
	INC	HL		;go on to the next one.
	LD	(FILEPOS),HL
	CALL	SUBHL		;(HL)=(DIRSIZE)-(FILEPOS)
	JP	NC,NXENT1	;is there more room left?
	JP	STFILPOS	;no. Set this flag and return.
NXENT1:	LD	A,(FILEPOS)	;get file position within directory.
	AND	03H		;only look within this sector (only 4 entries fit).
	LD	B,5		;convert to relative position (32 bytes each).
NXENT2:	ADD	A,A		;note that this is not efficient code.
	DEC	B		;5 'ADD A's would be better.
	JP	NZ,NXENT2
	LD	(FCBPOS),A	;save it as position of fcb.
	OR	A
	RET	NZ		;return if we are within buffer.
	PUSH	BC
	CALL	TRKSEC		;we need the next directory sector.
	CALL	DIRREAD
	POP	BC
	JP	CHECKDIR
;
;   Routine to to get a bit from the disk space allocation
; map. It is returned in (A), bit position 0. On entry to here,
; set (BC) to the block number on the disk to check.
; On return, (D) will contain the original bit position for
; this block number and (HL) will point to the address for it.
;
CKBITMAP: LD	A,C		;determine bit number of interest.
	AND	07H		;compute (D)=(E)=(C and 7)+1.
	INC	A
	LD	E,A		;save particular bit number.
	LD	D,A
;
;   compute (BC)=(BC)/8.
;
	LD	A,C
	RRCA			;now shift right 3 bits.
	RRCA	
	RRCA	
	AND	1FH		;and clear bits 7,6,5.
	LD	C,A
	LD	A,B
	ADD	A,A		;now shift (B) into bits 7,6,5.
	ADD	A,A
	ADD	A,A
	ADD	A,A
	ADD	A,A
	OR	C		;and add in (C).
	LD	C,A		;ok, (C) ha been completed.
	LD	A,B		;is there a better way of doing this?
	RRCA	
	RRCA	
	RRCA	
	AND	1FH
	LD	B,A		;and now (B) is completed.
;
;   use this as an offset into the disk space allocation
; table.
;
	LD	HL,(ALOCVECT)
	ADD	HL,BC
	LD	A,(HL)		;now get correct byte.
CKBMAP1:RLCA			;get correct bit into position 0.
	DEC	E
	JP	NZ,CKBMAP1
	RET	
;
;   Set or clear the bit map such that block number (BC) will be marked
; as used. On entry, if (E)=0 then this bit will be cleared, if it equals
; 1 then it will be set (don't use anyother values).
;
STBITMAP: PUSH	DE
	CALL	CKBITMAP	;get the byte of interest.
	AND	0FEH		;clear the affected bit.
	POP	BC
	OR	C		;and now set it acording to (C).
;
;  entry to restore the original bit position and then store
; in table. (A) contains the value, (D) contains the bit
; position (1-8), and (HL) points to the address within the
; space allocation table for this byte.
;
STBMAP1:RRCA			;restore original bit position.
	DEC	D
	JP	NZ,STBMAP1
	LD	(HL),A		;and stor byte in table.
	RET	
;
;   Set/clear space used bits in allocation map for this file.
; On entry, (C)=1 to set the map and (C)=0 to clear it.
;
SETFILE:CALL	FCB2HL		;get address of fcb
	LD	DE,16
	ADD	HL,DE		;get to block number bytes.
	PUSH	BC
	LD	C,17		;check all 17 bytes (max) of table.
SETFL1:	POP	DE
	DEC	C		;done all bytes yet?
	RET	Z
	PUSH	DE
	LD	A,(BIGDISK)	;check disk size for 16 bit block numbers.
	OR	A
	JP	Z,SETFL2
	PUSH	BC		;only 8 bit numbers. set (BC) to this one.
	PUSH	HL
	LD	C,(HL)		;get low byte from table, always
	LD	B,0		;set high byte to zero.
	JP	SETFL3
SETFL2:	DEC	C		;for 16 bit block numbers, adjust counter.
	PUSH	BC
	LD	C,(HL)		;now get both the low and high bytes.
	INC	HL
	LD	B,(HL)
	PUSH	HL
SETFL3:	LD	A,C		;block used?
	OR	B
	JP	Z,SETFL4
	LD	HL,(DSKSIZE)	;is this block number within the
	LD	A,L		;space on the disk?
	SUB	C
	LD	A,H
	SBC	A,B
	CALL	NC,STBITMAP	;yes, set the proper bit.
SETFL4:	POP	HL		;point to next block number in fcb.
	INC	HL
	POP	BC
	JP	SETFL1
;
;   Construct the space used allocation bit map for the active
; drive. If a file name starts with '$' and it is under the
; current user number, then (STATUS) is set to minus 1. Otherwise
; it is not set at all.
;
BITMAP:	LD	HL,(DSKSIZE)	;compute size of allocation table.
	LD	C,3
	CALL	SHIFTR		;(HL)=(HL)/8.
	INC	HL		;at lease 1 byte.
	LD	B,H
	LD	C,L		;set (BC) to the allocation table length.
;
;   Initialize the bitmap for this drive. Right now, the first
; two bytes are specified by the disk parameter block. However
; a patch could be entered here if it were necessary to setup
; this table in a special mannor. For example, the bios could
; determine locations of 'bad blocks' and set them as already
; 'used' in the map.
;
	LD	HL,(ALOCVECT)	;now zero out the table now.
BITMAP1:LD	(HL),0
	INC	HL
	DEC	BC
	LD	A,B
	OR	C
	JP	NZ,BITMAP1
	LD	HL,(ALLOC0)	;get initial space used by directory.
	EX	DE,HL
	LD	HL,(ALOCVECT)	;and put this into map.
	LD	(HL),E
	INC	HL
	LD	(HL),D
;
;   End of initialization portion.
;
	CALL	HOMEDRV		;now home the drive.
	LD	HL,(SCRATCH1)
	LD	(HL),3		;force next directory request to read
	INC	HL		;in a sector.
	LD	(HL),0
	CALL	STFILPOS	;clear initial file position also.
BITMAP2:LD	C,0FFH		;read next file name in directory
	CALL	NXENTRY		;and set checksum byte.
	CALL	CKFILPOS	;is there another file?
	RET	Z
	CALL	FCB2HL		;yes, get its address.
	LD	A,0E5H
	CP	(HL)		;empty file entry?
	JP	Z,BITMAP2
	LD	A,(USERNO)	;no, correct user number?
	CP	(HL)
	JP	NZ,BITMAP3
	INC	HL
	LD	A,(HL)		;yes, does name start with a '$'?
	SUB	'$'
	JP	NZ,BITMAP3
	DEC	A		;yes, set atatus to minus one.
	LD	(STATUS),A
BITMAP3:LD	C,1		;now set this file's space as used in bit map.
	CALL	SETFILE
	CALL	CHKNMBR		;keep (SCRATCH1) in bounds.
	JP	BITMAP2
;
;   Set the status (STATUS) and return.
;
STSTATUS: LD	A,(FNDSTAT)
	JP	SETSTAT
;
;   Check extents in (A) and (C). Set the zero flag if they
; are the same. The number of 16k chunks of disk space that
; the directory extent covers is expressad is (EXTMASK+1).
; No registers are modified.
;
SAMEXT:	PUSH	BC
	PUSH	AF
	LD	A,(EXTMASK)	;get extent mask and use it to
	CPL			;to compare both extent numbers.
	LD	B,A		;save resulting mask here.
	LD	A,C		;mask first extent and save in (C).
	AND	B
	LD	C,A
	POP	AF		;now mask second extent and compare
	AND	B		;with the first one.
	SUB	C
	AND	1FH		;(* only check buts 0-4 *)
	POP	BC		;the zero flag is set if they are the same.
	RET			;restore (BC) and return.
;
;   Search for the first occurence of a file name. On entry,
; register (C) should contain the number of bytes of the fcb
; that must match.
;
FINDFST:LD	A,0FFH
	LD	(FNDSTAT),A
	LD	HL,COUNTER	;save character count.
	LD	(HL),C
	LD	HL,(PARAMS)	;get filename to match.
	LD	(SAVEFCB),HL	;and save.
	CALL	STFILPOS	;clear initial file position (set to 0ffffh).
	CALL	HOMEDRV		;home the drive.
;
;   Entry to locate the next occurence of a filename within the
; directory. The disk is not expected to have been changed. If
; it was, then it will be write protected.
;
FINDNXT:LD	C,0		;write protect the disk if changed.
	CALL	NXENTRY		;get next filename entry in directory.
	CALL	CKFILPOS	;is file position = 0ffffh?
	JP	Z,FNDNXT6	;yes, exit now then.
	LD	HL,(SAVEFCB)	;set (DE) pointing to filename to match.
	EX	DE,HL
	LD	A,(DE)
	CP	0E5H		;empty directory entry?
	JP	Z,FNDNXT1	;(* are we trying to reserect erased entries? *)
	PUSH	DE
	CALL	MOREFLS		;more files in directory?
	POP	DE
	JP	NC,FNDNXT6	;no more. Exit now.
FNDNXT1:CALL	FCB2HL		;get address of this fcb in directory.
	LD	A,(COUNTER)	;get number of bytes (characters) to check.
	LD	C,A
	LD	B,0		;initialize byte position counter.
FNDNXT2:LD	A,C		;are we done with the compare?
	OR	A
	JP	Z,FNDNXT5
	LD	A,(DE)		;no, check next byte.
	CP	'?'		;don't care about this character?
	JP	Z,FNDNXT4
	LD	A,B		;get bytes position in fcb.
	CP	13		;don't care about the thirteenth byte either.
	JP	Z,FNDNXT4
	CP	12		;extent byte?
	LD	A,(DE)
	JP	Z,FNDNXT3
	SUB	(HL)		;otherwise compare characters.
	AND	7FH
	JP	NZ,FINDNXT	;not the same, check next entry.
	JP	FNDNXT4		;so far so good, keep checking.
FNDNXT3:PUSH	BC		;check the extent byte here.
	LD	C,(HL)
	CALL	SAMEXT
	POP	BC
	JP	NZ,FINDNXT	;not the same, look some more.
;
;   So far the names compare. Bump pointers to the next byte
; and continue until all (C) characters have been checked.
;
FNDNXT4:INC	DE		;bump pointers.
	INC	HL
	INC	B
	DEC	C		;adjust character counter.
	JP	FNDNXT2
FNDNXT5:LD	A,(FILEPOS)	;return the position of this entry.
	AND	03H
	LD	(STATUS),A
	LD	HL,FNDSTAT
	LD	A,(HL)
	RLA	
	RET	NC
	XOR	A
	LD	(HL),A
	RET	
;
;   Filename was not found. Set appropriate status.
;
FNDNXT6:CALL	STFILPOS	;set (FILEPOS) to 0ffffh.
	LD	A,0FFH		;say not located.
	JP	SETSTAT
;
;   Erase files from the directory. Only the first byte of the
; fcb will be affected. It is set to (E5).
;
ERAFILE:CALL	CHKWPRT		;is disk write protected?
	LD	C,12		;only compare file names.
	CALL	FINDFST		;get first file name.
ERAFIL1:CALL	CKFILPOS	;any found?
	RET	Z		;nope, we must be done.
	CALL	CHKROFL		;is file read only?
	CALL	FCB2HL		;nope, get address of fcb and
	LD	(HL),0E5H	;set first byte to 'empty'.
	LD	C,0		;clear the space from the bit map.
	CALL	SETFILE
	CALL	DIRWRITE	;now write the directory sector back out.
	CALL	FINDNXT		;find the next file name.
	JP	ERAFIL1		;and repeat process.
;
;   Look through the space allocation map (bit map) for the
; next available block. Start searching at block number (BC-1).
; The search procedure is to look for an empty block that is
; before the starting block. If not empty, look at a later
; block number. In this way, we return the closest empty block
; on either side of the 'target' block number. This will speed
; access on random devices. For serial devices, this should be
; changed to look in the forward direction first and then start
; at the front and search some more.
;
;   On return, (DE)= block number that is empty and (HL) =0
; if no empry block was found.
;
FNDSPACE: LD	D,B		;set (DE) as the block that is checked.
	LD	E,C
;
;   Look before target block. Registers (BC) are used as the lower
; pointer and (DE) as the upper pointer.
;
FNDSPA1:LD	A,C		;is block 0 specified?
	OR	B
	JP	Z,FNDSPA2
	DEC	BC		;nope, check previous block.
	PUSH	DE
	PUSH	BC
	CALL	CKBITMAP
	RRA			;is this block empty?
	JP	NC,FNDSPA3	;yes. use this.
;
;   Note that the above logic gets the first block that it finds
; that is empty. Thus a file could be written 'backward' making
; it very slow to access. This could be changed to look for the
; first empty block and then continue until the start of this
; empty space is located and then used that starting block.
; This should help speed up access to some files especially on
; a well used disk with lots of fairly small 'holes'.
;
	POP	BC		;nope, check some more.
	POP	DE
;
;   Now look after target block.
;
FNDSPA2:LD	HL,(DSKSIZE)	;is block (DE) within disk limits?
	LD	A,E
	SUB	L
	LD	A,D
	SBC	A,H
	JP	NC,FNDSPA4
	INC	DE		;yes, move on to next one.
	PUSH	BC
	PUSH	DE
	LD	B,D
	LD	C,E
	CALL	CKBITMAP	;check it.
	RRA			;empty?
	JP	NC,FNDSPA3
	POP	DE		;nope, continue searching.
	POP	BC
	JP	FNDSPA1
;
;   Empty block found. Set it as used and return with (HL)
; pointing to it (true?).
;
FNDSPA3:RLA			;reset byte.
	INC	A		;and set bit 0.
	CALL	STBMAP1		;update bit map.
	POP	HL		;set return registers.
	POP	DE
	RET	
;
;   Free block was not found. If (BC) is not zero, then we have
; not checked all of the disk space.
;
FNDSPA4:LD	A,C
	OR	B
	JP	NZ,FNDSPA1
	LD	HL,0		;set 'not found' status.
	RET	
;
;   Move a complete fcb entry into the directory and write it.
;
FCBSET:	LD	C,0
	LD	E,32		;length of each entry.
;
;   Move (E) bytes from the fcb pointed to by (PARAMS) into
; fcb in directory starting at relative byte (C). This updated
; directory buffer is then written to the disk.
;
UPDATE:	PUSH	DE
	LD	B,0		;set (BC) to relative byte position.
	LD	HL,(PARAMS)	;get address of fcb.
	ADD	HL,BC		;compute starting byte.
	EX	DE,HL
	CALL	FCB2HL		;get address of fcb to update in directory.
	POP	BC		;set (C) to number of bytes to change.
	CALL	DE2HL
UPDATE1:CALL	TRKSEC		;determine the track and sector affected.
	JP	DIRWRITE	;then write this sector out.
;
;   Routine to change the name of all files on the disk with a
; specified name. The fcb contains the current name as the
; first 12 characters and the new name 16 bytes into the fcb.
;
CHGNAMES: CALL	CHKWPRT		;check for a write protected disk.
	LD	C,12		;match first 12 bytes of fcb only.
	CALL	FINDFST		;get first name.
	LD	HL,(PARAMS)	;get address of fcb.
	LD	A,(HL)		;get user number.
	LD	DE,16		;move over to desired name.
	ADD	HL,DE
	LD	(HL),A		;keep same user number.
CHGNAM1:CALL	CKFILPOS	;any matching file found?
	RET	Z		;no, we must be done.
	CALL	CHKROFL		;check for read only file.
	LD	C,16		;start 16 bytes into fcb.
	LD	E,12		;and update the first 12 bytes of directory.
	CALL	UPDATE
	CALL	FINDNXT		;get te next file name.
	JP	CHGNAM1		;and continue.
;
;   Update a files attributes. The procedure is to search for
; every file with the same name as shown in fcb (ignoring bit 7)
; and then to update it (which includes bit 7). No other changes
; are made.
;
SAVEATTR: LD	C,12		;match first 12 bytes.
	CALL	FINDFST		;look for first filename.
SAVATR1:CALL	CKFILPOS	;was one found?
	RET	Z		;nope, we must be done.
	LD	C,0		;yes, update the first 12 bytes now.
	LD	E,12
	CALL	UPDATE		;update filename and write directory.
	CALL	FINDNXT		;and get the next file.
	JP	SAVATR1		;then continue until done.
;
;  Open a file (name specified in fcb).
;
OPENIT:	LD	C,15		;compare the first 15 bytes.
	CALL	FINDFST		;get the first one in directory.
	CALL	CKFILPOS	;any at all?
	RET	Z
OPENIT1:CALL	SETEXT		;point to extent byte within users fcb.
	LD	A,(HL)		;and get it.
	PUSH	AF		;save it and address.
	PUSH	HL
	CALL	FCB2HL		;point to fcb in directory.
	EX	DE,HL
	LD	HL,(PARAMS)	;this is the users copy.
	LD	C,32		;move it into users space.
	PUSH	DE
	CALL	DE2HL
	CALL	SETS2B7		;set bit 7 in 's2' byte (unmodified).
	POP	DE		;now get the extent byte from this fcb.
	LD	HL,12
	ADD	HL,DE
	LD	C,(HL)		;into (C).
	LD	HL,15		;now get the record count byte into (B).
	ADD	HL,DE
	LD	B,(HL)
	POP	HL		;keep the same extent as the user had originally.
	POP	AF
	LD	(HL),A
	LD	A,C		;is it the same as in the directory fcb?
	CP	(HL)
	LD	A,B		;if yes, then use the same record count.
	JP	Z,OPENIT2
	LD	A,0		;if the user specified an extent greater than
	JP	C,OPENIT2	;the one in the directory, then set record count to 0.
	LD	A,128		;otherwise set to maximum.
OPENIT2:LD	HL,(PARAMS)	;set record count in users fcb to (A).
	LD	DE,15
	ADD	HL,DE		;compute relative position.
	LD	(HL),A		;and set the record count.
	RET	
;
;   Move two bytes from (DE) to (HL) if (and only if) (HL)
; point to a zero value (16 bit).
;   Return with zero flag set it (DE) was moved. Registers (DE)
; and (HL) are not changed. However (A) is.
;
MOVEWORD: LD	A,(HL)		;check for a zero word.
	INC	HL
	OR	(HL)		;both bytes zero?
	DEC	HL
	RET	NZ		;nope, just return.
	LD	A,(DE)		;yes, move two bytes from (DE) into
	LD	(HL),A		;this zero space.
	INC	DE
	INC	HL
	LD	A,(DE)
	LD	(HL),A
	DEC	DE		;don't disturb these registers.
	DEC	HL
	RET	
;
;   Get here to close a file specified by (fcb).
;
CLOSEIT:XOR	A		;clear status and file position bytes.
	LD	(STATUS),A
	LD	(FILEPOS),A
	LD	(FILEPOS+1),A
	CALL	GETWPRT		;get write protect bit for this drive.
	RET	NZ		;just return if it is set.
	CALL	GETS2		;else get the 's2' byte.
	AND	80H		;and look at bit 7 (file unmodified?).
	RET	NZ		;just return if set.
	LD	C,15		;else look up this file in directory.
	CALL	FINDFST
	CALL	CKFILPOS	;was it found?
	RET	Z		;just return if not.
	LD	BC,16		;set (HL) pointing to records used section.
	CALL	FCB2HL
	ADD	HL,BC
	EX	DE,HL
	LD	HL,(PARAMS)	;do the same for users specified fcb.
	ADD	HL,BC
	LD	C,16		;this many bytes are present in this extent.
CLOSEIT1: LD	A,(BIGDISK)	;8 or 16 bit record numbers?
	OR	A
	JP	Z,CLOSEIT4
	LD	A,(HL)		;just 8 bit. Get one from users fcb.
	OR	A
	LD	A,(DE)		;now get one from directory fcb.
	JP	NZ,CLOSEIT2
	LD	(HL),A		;users byte was zero. Update from directory.
CLOSEIT2: OR	A
	JP	NZ,CLOSEIT3
	LD	A,(HL)		;directories byte was zero, update from users fcb.
	LD	(DE),A
CLOSEIT3: CP	(HL)		;if neither one of these bytes were zero,
	JP	NZ,CLOSEIT7	;then close error if they are not the same.
	JP	CLOSEIT5	;ok so far, get to next byte in fcbs.
CLOSEIT4: CALL	MOVEWORD	;update users fcb if it is zero.
	EX	DE,HL
	CALL	MOVEWORD	;update directories fcb if it is zero.
	EX	DE,HL
	LD	A,(DE)		;if these two values are no different,
	CP	(HL)		;then a close error occured.
	JP	NZ,CLOSEIT7
	INC	DE		;check second byte.
	INC	HL
	LD	A,(DE)
	CP	(HL)
	JP	NZ,CLOSEIT7
	DEC	C		;remember 16 bit values.
CLOSEIT5: INC	DE		;bump to next item in table.
	INC	HL
	DEC	C		;there are 16 entries only.
	JP	NZ,CLOSEIT1	;continue if more to do.
	LD	BC,0FFECH	;backup 20 places (extent byte).
	ADD	HL,BC
	EX	DE,HL
	ADD	HL,BC
	LD	A,(DE)
	CP	(HL)		;directory's extent already greater than the
	JP	C,CLOSEIT6	;users extent?
	LD	(HL),A		;no, update directory extent.
	LD	BC,3		;and update the record count byte in
	ADD	HL,BC		;directories fcb.
	EX	DE,HL
	ADD	HL,BC
	LD	A,(HL)		;get from user.
	LD	(DE),A		;and put in directory.
CLOSEIT6: LD	A,0FFH		;set 'was open and is now closed' byte.
	LD	(CLOSEFLG),A
	JP	UPDATE1		;update the directory now.
CLOSEIT7: LD	HL,STATUS	;set return status and then return.
	DEC	(HL)
	RET	
;
;   Routine to get the next empty space in the directory. It
; will then be cleared for use.
;
GETEMPTY: CALL	CHKWPRT		;make sure disk is not write protected.
	LD	HL,(PARAMS)	;save current parameters (fcb).
	PUSH	HL
	LD	HL,EMPTYFCB	;use special one for empty space.
	LD	(PARAMS),HL
	LD	C,1		;search for first empty spot in directory.
	CALL	FINDFST		;(* only check first byte *)
	CALL	CKFILPOS	;none?
	POP	HL
	LD	(PARAMS),HL	;restore original fcb address.
	RET	Z		;return if no more space.
	EX	DE,HL
	LD	HL,15		;point to number of records for this file.
	ADD	HL,DE
	LD	C,17		;and clear all of this space.
	XOR	A
GETMT1:	LD	(HL),A
	INC	HL
	DEC	C
	JP	NZ,GETMT1
	LD	HL,13		;clear the 's1' byte also.
	ADD	HL,DE
	LD	(HL),A
	CALL	CHKNMBR		;keep (SCRATCH1) within bounds.
	CALL	FCBSET		;write out this fcb entry to directory.
	JP	SETS2B7		;set 's2' byte bit 7 (unmodified at present).
;
;   Routine to close the current extent and open the next one
; for reading.
;
GETNEXT:XOR	A
	LD	(CLOSEFLG),A	;clear close flag.
	CALL	CLOSEIT		;close this extent.
	CALL	CKFILPOS
	RET	Z		;not there???
	LD	HL,(PARAMS)	;get extent byte.
	LD	BC,12
	ADD	HL,BC
	LD	A,(HL)		;and increment it.
	INC	A
	AND	1FH		;keep within range 0-31.
	LD	(HL),A
	JP	Z,GTNEXT1	;overflow?
	LD	B,A		;mask extent byte.
	LD	A,(EXTMASK)
	AND	B
	LD	HL,CLOSEFLG	;check close flag (0ffh is ok).
	AND	(HL)
	JP	Z,GTNEXT2	;if zero, we must read in next extent.
	JP	GTNEXT3		;else, it is already in memory.
GTNEXT1:LD	BC,2		;Point to the 's2' byte.
	ADD	HL,BC
	INC	(HL)		;and bump it.
	LD	A,(HL)		;too many extents?
	AND	0FH
	JP	Z,GTNEXT5	;yes, set error code.
;
;   Get here to open the next extent.
;
GTNEXT2:LD	C,15		;set to check first 15 bytes of fcb.
	CALL	FINDFST		;find the first one.
	CALL	CKFILPOS	;none available?
	JP	NZ,GTNEXT3
	LD	A,(RDWRTFLG)	;no extent present. Can we open an empty one?
	INC	A		;0ffh means reading (so not possible).
	JP	Z,GTNEXT5	;or an error.
	CALL	GETEMPTY	;we are writing, get an empty entry.
	CALL	CKFILPOS	;none?
	JP	Z,GTNEXT5	;error if true.
	JP	GTNEXT4		;else we are almost done.
GTNEXT3:CALL	OPENIT1		;open this extent.
GTNEXT4:CALL	STRDATA		;move in updated data (rec #, extent #, etc.)
	XOR	A		;clear status and return.
	JP	SETSTAT
;
;   Error in extending the file. Too many extents were needed
; or not enough space on the disk.
;
GTNEXT5:CALL	IOERR1		;set error code, clear bit 7 of 's2'
	JP	SETS2B7		;so this is not written on a close.
;
;   Read a sequential file.
;
RDSEQ:	LD	A,1		;set sequential access mode.
	LD	(MODE),A
RDSEQ1:	LD	A,0FFH		;don't allow reading unwritten space.
	LD	(RDWRTFLG),A
	CALL	STRDATA		;put rec# and ext# into fcb.
	LD	A,(SAVNREC)	;get next record to read.
	LD	HL,SAVNXT	;get number of records in extent.
	CP	(HL)		;within this extent?
	JP	C,RDSEQ2
	CP	128		;no. Is this extent fully used?
	JP	NZ,RDSEQ3	;no. End-of-file.
	CALL	GETNEXT		;yes, open the next one.
	XOR	A		;reset next record to read.
	LD	(SAVNREC),A
	LD	A,(STATUS)	;check on open, successful?
	OR	A
	JP	NZ,RDSEQ3	;no, error.
RDSEQ2:	CALL	COMBLK		;ok. compute block number to read.
	CALL	CHKBLK		;check it. Within bounds?
	JP	Z,RDSEQ3	;no, error.
	CALL	LOGICAL		;convert (BLKNMBR) to logical sector (128 byte).
	CALL	TRKSEC1		;set the track and sector for this block #.
	CALL	DOREAD		;and read it.
	JP	SETNREC		;and set the next record to be accessed.
;
;   Read error occured. Set status and return.
;
RDSEQ3:	JP	IOERR1
;
;   Write the next sequential record.
;
WTSEQ:	LD	A,1		;set sequential access mode.
	LD	(MODE),A
WTSEQ1:	LD	A,0		;allow an addition empty extent to be opened.
	LD	(RDWRTFLG),A
	CALL	CHKWPRT		;check write protect status.
	LD	HL,(PARAMS)
	CALL	CKROF1		;check for read only file, (HL) already set to fcb.
	CALL	STRDATA		;put updated data into fcb.
	LD	A,(SAVNREC)	;get record number to write.
	CP	128		;within range?
	JP	NC,IOERR1	;no, error(?).
	CALL	COMBLK		;compute block number.
	CALL	CHKBLK		;check number.
	LD	C,0		;is there one to write to?
	JP	NZ,WTSEQ6	;yes, go do it.
	CALL	GETBLOCK	;get next block number within fcb to use.
	LD	(RELBLOCK),A	;and save.
	LD	BC,0		;start looking for space from the start
	OR	A		;if none allocated as yet.
	JP	Z,WTSEQ2
	LD	C,A		;extract previous block number from fcb
	DEC	BC		;so we can be closest to it.
	CALL	EXTBLK
	LD	B,H
	LD	C,L
WTSEQ2:	CALL	FNDSPACE	;find the next empty block nearest number (BC).
	LD	A,L		;check for a zero number.
	OR	H
	JP	NZ,WTSEQ3
	LD	A,2		;no more space?
	JP	SETSTAT
WTSEQ3:	LD	(BLKNMBR),HL	;save block number to access.
	EX	DE,HL		;put block number into (DE).
	LD	HL,(PARAMS)	;now we must update the fcb for this
	LD	BC,16		;newly allocated block.
	ADD	HL,BC
	LD	A,(BIGDISK)	;8 or 16 bit block numbers?
	OR	A
	LD	A,(RELBLOCK)	;(* update this entry *)
	JP	Z,WTSEQ4	;zero means 16 bit ones.
	CALL	ADDA2HL		;(HL)=(HL)+(A)
	LD	(HL),E		;store new block number.
	JP	WTSEQ5
WTSEQ4:	LD	C,A		;compute spot in this 16 bit table.
	LD	B,0
	ADD	HL,BC
	ADD	HL,BC
	LD	(HL),E		;stuff block number (DE) there.
	INC	HL
	LD	(HL),D
WTSEQ5:	LD	C,2		;set (C) to indicate writing to un-used disk space.
WTSEQ6:	LD	A,(STATUS)	;are we ok so far?
	OR	A
	RET	NZ
	PUSH	BC		;yes, save write flag for bios (register C).
	CALL	LOGICAL		;convert (BLKNMBR) over to loical sectors.
	LD	A,(MODE)	;get access mode flag (1=sequential,
	DEC	A		;0=random, 2=special?).
	DEC	A
	JP	NZ,WTSEQ9
;
;   Special random i/o from function #40. Maybe for M/PM, but the
; current block, if it has not been written to, will be zeroed
; out and then written (reason?).
;
	POP	BC
	PUSH	BC
	LD	A,C		;get write status flag (2=writing unused space).
	DEC	A
	DEC	A
	JP	NZ,WTSEQ9
	PUSH	HL
	LD	HL,(DIRBUF)	;zero out the directory buffer.
	LD	D,A		;note that (A) is zero here.
WTSEQ7:	LD	(HL),A
	INC	HL
	INC	D		;do 128 bytes.
	JP	P,WTSEQ7
	CALL	DIRDMA		;tell the bios the dma address for directory access.
	LD	HL,(LOGSECT)	;get sector that starts current block.
	LD	C,2		;set 'writing to unused space' flag.
WTSEQ8:	LD	(BLKNMBR),HL	;save sector to write.
	PUSH	BC
	CALL	TRKSEC1		;determine its track and sector numbers.
	POP	BC
	CALL	DOWRITE		;now write out 128 bytes of zeros.
	LD	HL,(BLKNMBR)	;get sector number.
	LD	C,0		;set normal write flag.
	LD	A,(BLKMASK)	;determine if we have written the entire
	LD	B,A		;physical block.
	AND	L
	CP	B
	INC	HL		;prepare for the next one.
	JP	NZ,WTSEQ8	;continue until (BLKMASK+1) sectors written.
	POP	HL		;reset next sector number.
	LD	(BLKNMBR),HL
	CALL	DEFDMA		;and reset dma address.
;
;   Normal disk write. Set the desired track and sector then
; do the actual write.
;
WTSEQ9:	CALL	TRKSEC1		;determine track and sector for this write.
	POP	BC		;get write status flag.
	PUSH	BC
	CALL	DOWRITE		;and write this out.
	POP	BC
	LD	A,(SAVNREC)	;get number of records in file.
	LD	HL,SAVNXT	;get last record written.
	CP	(HL)
	JP	C,WTSEQ10
	LD	(HL),A		;we have to update record count.
	INC	(HL)
	LD	C,2
;
;*   This area has been patched to correct disk update problem
;* when using blocking and de-blocking in the BIOS.
;
WTSEQ10:NOP			;was 'dcr c'
	NOP			;was 'dcr c'
	LD	HL,0		;was 'jnz wtseq99'
;
; *   End of patch.
;
	PUSH	AF
	CALL	GETS2		;set 'extent written to' flag.
	AND	7FH		;(* clear bit 7 *)
	LD	(HL),A
	POP	AF		;get record count for this extent.
WTSEQ99:CP	127		;is it full?
	JP	NZ,WTSEQ12
	LD	A,(MODE)	;yes, are we in sequential mode?
	CP	1
	JP	NZ,WTSEQ12
	CALL	SETNREC		;yes, set next record number.
	CALL	GETNEXT		;and get next empty space in directory.
	LD	HL,STATUS	;ok?
	LD	A,(HL)
	OR	A
	JP	NZ,WTSEQ11
	DEC	A		;yes, set record count to -1.
	LD	(SAVNREC),A
WTSEQ11:LD	(HL),0		;clear status.
WTSEQ12:JP	SETNREC		;set next record to access.
;
;   For random i/o, set the fcb for the desired record number
; based on the 'r0,r1,r2' bytes. These bytes in the fcb are
; used as follows:
;
;       fcb+35            fcb+34            fcb+33
;  |     'r-2'      |      'r-1'      |      'r-0'     |
;  |7             0 | 7             0 | 7             0|
;  |0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0|
;  |    overflow   | | extra |  extent   |   record #  |
;  | ______________| |_extent|__number___|_____________|
;                     also 's2'
;
;   On entry, register (C) contains 0ffh if this is a read
; and thus we can not access unwritten disk space. Otherwise,
; another extent will be opened (for writing) if required.
;
POSITION: XOR	A		;set random i/o flag.
	LD	(MODE),A
;
;   Special entry (function #40). M/PM ?
;
POSITN1:PUSH	BC		;save read/write flag.
	LD	HL,(PARAMS)	;get address of fcb.
	EX	DE,HL
	LD	HL,33		;now get byte 'r0'.
	ADD	HL,DE
	LD	A,(HL)
	AND	7FH		;keep bits 0-6 for the record number to access.
	PUSH	AF
	LD	A,(HL)		;now get bit 7 of 'r0' and bits 0-3 of 'r1'.
	RLA	
	INC	HL
	LD	A,(HL)
	RLA	
	AND	1FH		;and save this in bits 0-4 of (C).
	LD	C,A		;this is the extent byte.
	LD	A,(HL)		;now get the extra extent byte.
	RRA	
	RRA	
	RRA	
	RRA	
	AND	0FH
	LD	B,A		;and save it in (B).
	POP	AF		;get record number back to (A).
	INC	HL		;check overflow byte 'r2'.
	LD	L,(HL)
	INC	L
	DEC	L
	LD	L,6		;prepare for error.
	JP	NZ,POSITN5	;out of disk space error.
	LD	HL,32		;store record number into fcb.
	ADD	HL,DE
	LD	(HL),A
	LD	HL,12		;and now check the extent byte.
	ADD	HL,DE
	LD	A,C
	SUB	(HL)		;same extent as before?
	JP	NZ,POSITN2
	LD	HL,14		;yes, check extra extent byte 's2' also.
	ADD	HL,DE
	LD	A,B
	SUB	(HL)
	AND	7FH
	JP	Z,POSITN3	;same, we are almost done then.
;
;  Get here when another extent is required.
;
POSITN2:PUSH	BC
	PUSH	DE
	CALL	CLOSEIT		;close current extent.
	POP	DE
	POP	BC
	LD	L,3		;prepare for error.
	LD	A,(STATUS)
	INC	A
	JP	Z,POSITN4	;close error.
	LD	HL,12		;put desired extent into fcb now.
	ADD	HL,DE
	LD	(HL),C
	LD	HL,14		;and store extra extent byte 's2'.
	ADD	HL,DE
	LD	(HL),B
	CALL	OPENIT		;try and get this extent.
	LD	A,(STATUS)	;was it there?
	INC	A
	JP	NZ,POSITN3
	POP	BC		;no. can we create a new one (writing?).
	PUSH	BC
	LD	L,4		;prepare for error.
	INC	C
	JP	Z,POSITN4	;nope, reading unwritten space error.
	CALL	GETEMPTY	;yes we can, try to find space.
	LD	L,5		;prepare for error.
	LD	A,(STATUS)
	INC	A
	JP	Z,POSITN4	;out of space?
;
;   Normal return location. Clear error code and return.
;
POSITN3:POP	BC		;restore stack.
	XOR	A		;and clear error code byte.
	JP	SETSTAT
;
;   Error. Set the 's2' byte to indicate this (why?).
;
POSITN4:PUSH	HL
	CALL	GETS2
	LD	(HL),0C0H
	POP	HL
;
;   Return with error code (presently in L).
;
POSITN5:POP	BC
	LD	A,L		;get error code.
	LD	(STATUS),A
	JP	SETS2B7
;
;   Read a random record.
;
READRAN:LD	C,0FFH		;set 'read' status.
	CALL	POSITION	;position the file to proper record.
	CALL	Z,RDSEQ1	;and read it as usual (if no errors).
	RET	
;
;   Write to a random record.
;
WRITERAN: LD	C,0		;set 'writing' flag.
	CALL	POSITION	;position the file to proper record.
	CALL	Z,WTSEQ1	;and write as usual (if no errors).
	RET	
;
;   Compute the random record number. Enter with (HL) pointing
; to a fcb an (DE) contains a relative location of a record
; number. On exit, (C) contains the 'r0' byte, (B) the 'r1'
; byte, and (A) the 'r2' byte.
;
;   On return, the zero flag is set if the record is within
; bounds. Otherwise, an overflow occured.
;
COMPRAND: EX	DE,HL		;save fcb pointer in (DE).
	ADD	HL,DE		;compute relative position of record #.
	LD	C,(HL)		;get record number into (BC).
	LD	B,0
	LD	HL,12		;now get extent.
	ADD	HL,DE
	LD	A,(HL)		;compute (BC)=(record #)+(extent)*128.
	RRCA			;move lower bit into bit 7.
	AND	80H		;and ignore all other bits.
	ADD	A,C		;add to our record number.
	LD	C,A
	LD	A,0		;take care of any carry.
	ADC	A,B
	LD	B,A
	LD	A,(HL)		;now get the upper bits of extent into
	RRCA			;bit positions 0-3.
	AND	0FH		;and ignore all others.
	ADD	A,B		;add this in to 'r1' byte.
	LD	B,A
	LD	HL,14		;get the 's2' byte (extra extent).
	ADD	HL,DE
	LD	A,(HL)
	ADD	A,A		;and shift it left 4 bits (bits 4-7).
	ADD	A,A
	ADD	A,A
	ADD	A,A
	PUSH	AF		;save carry flag (bit 0 of flag byte).
	ADD	A,B		;now add extra extent into 'r1'.
	LD	B,A
	PUSH	AF		;and save carry (overflow byte 'r2').
	POP	HL		;bit 0 of (L) is the overflow indicator.
	LD	A,L
	POP	HL		;and same for first carry flag.
	OR	L		;either one of these set?
	AND	01H		;only check the carry flags.
	RET	
;
;   Routine to setup the fcb (bytes 'r0', 'r1', 'r2') to
; reflect the last record used for a random (or other) file.
; This reads the directory and looks at all extents computing
; the largerst record number for each and keeping the maximum
; value only. Then 'r0', 'r1', and 'r2' will reflect this
; maximum record number. This is used to compute the space used
; by a random file.
;
RANSIZE:LD	C,12		;look thru directory for first entry with
	CALL	FINDFST		;this name.
	LD	HL,(PARAMS)	;zero out the 'r0, r1, r2' bytes.
	LD	DE,33
	ADD	HL,DE
	PUSH	HL
	LD	(HL),D		;note that (D)=0.
	INC	HL
	LD	(HL),D
	INC	HL
	LD	(HL),D
RANSIZ1:CALL	CKFILPOS	;is there an extent to process?
	JP	Z,RANSIZ3	;no, we are done.
	CALL	FCB2HL		;set (HL) pointing to proper fcb in dir.
	LD	DE,15		;point to last record in extent.
	CALL	COMPRAND	;and compute random parameters.
	POP	HL
	PUSH	HL		;now check these values against those
	LD	E,A		;already in fcb.
	LD	A,C		;the carry flag will be set if those
	SUB	(HL)		;in the fcb represent a larger size than
	INC	HL		;this extent does.
	LD	A,B
	SBC	A,(HL)
	INC	HL
	LD	A,E
	SBC	A,(HL)
	JP	C,RANSIZ2
	LD	(HL),E		;we found a larger (in size) extent.
	DEC	HL		;stuff these values into fcb.
	LD	(HL),B
	DEC	HL
	LD	(HL),C
RANSIZ2:CALL	FINDNXT		;now get the next extent.
	JP	RANSIZ1		;continue til all done.
RANSIZ3:POP	HL		;we are done, restore the stack and
	RET			;return.
;
;   Function to return the random record position of a given
; file which has been read in sequential mode up to now.
;
SETRAN:	LD	HL,(PARAMS)	;point to fcb.
	LD	DE,32		;and to last used record.
	CALL	COMPRAND	;compute random position.
	LD	HL,33		;now stuff these values into fcb.
	ADD	HL,DE
	LD	(HL),C		;move 'r0'.
	INC	HL
	LD	(HL),B		;and 'r1'.
	INC	HL
	LD	(HL),A		;and lastly 'r2'.
	RET	
;
;   This routine select the drive specified in (ACTIVE) and
; update the login vector and bitmap table if this drive was
; not already active.
;
LOGINDRV: LD	HL,(LOGIN)	;get the login vector.
	LD	A,(ACTIVE)	;get the default drive.
	LD	C,A
	CALL	SHIFTR		;position active bit for this drive
	PUSH	HL		;into bit 0.
	EX	DE,HL
	CALL	SELECT		;select this drive.
	POP	HL
	CALL	Z,SLCTERR	;valid drive?
	LD	A,L		;is this a newly activated drive?
	RRA	
	RET	C
	LD	HL,(LOGIN)	;yes, update the login vector.
	LD	C,L
	LD	B,H
	CALL	SETBIT
	LD	(LOGIN),HL	;and save.
	JP	BITMAP		;now update the bitmap.
;
;   Function to set the active disk number.
;
SETDSK:	LD	A,(EPARAM)	;get parameter passed and see if this
	LD	HL,ACTIVE	;represents a change in drives.
	CP	(HL)
	RET	Z
	LD	(HL),A		;yes it does, log it in.
	JP	LOGINDRV
;
;   This is the 'auto disk select' routine. The firsst byte
; of the fcb is examined for a drive specification. If non
; zero then the drive will be selected and loged in.
;
AUTOSEL:LD	A,0FFH		;say 'auto-select activated'.
	LD	(AUTO),A
	LD	HL,(PARAMS)	;get drive specified.
	LD	A,(HL)
	AND	1FH		;look at lower 5 bits.
	DEC	A		;adjust for (1=A, 2=B) etc.
	LD	(EPARAM),A	;and save for the select routine.
	CP	1EH		;check for 'no change' condition.
	JP	NC,AUTOSL1	;yes, don't change.
	LD	A,(ACTIVE)	;we must change, save currently active
	LD	(OLDDRV),A	;drive.
	LD	A,(HL)		;and save first byte of fcb also.
	LD	(AUTOFLAG),A	;this must be non-zero.
	AND	0E0H		;whats this for (bits 6,7 are used for
	LD	(HL),A		;something)?
	CALL	SETDSK		;select and log in this drive.
AUTOSL1:LD	A,(USERNO)	;move user number into fcb.
	LD	HL,(PARAMS)	;(* upper half of first byte *)
	OR	(HL)
	LD	(HL),A
	RET			;and return (all done).
;
;   Function to return the current cp/m version number.
;
GETVER:	LD	A,022H		;version 2.2
	JP	SETSTAT
;
;   Function to reset the disk system.
;
RSTDSK:	LD	HL,0		;clear write protect status and log
	LD	(WRTPRT),HL	;in vector.
	LD	(LOGIN),HL
	XOR	A		;select drive 'A'.
	LD	(ACTIVE),A
	LD	HL,TBUFF	;setup default dma address.
	LD	(USERDMA),HL
	CALL	DEFDMA
	JP	LOGINDRV	;now log in drive 'A'.
;
;   Function to open a specified file.
;
OPENFIL:CALL	CLEARS2		;clear 's2' byte.
	CALL	AUTOSEL		;select proper disk.
	JP	OPENIT		;and open the file.
;
;   Function to close a specified file.
;
CLOSEFIL: CALL	AUTOSEL		;select proper disk.
	JP	CLOSEIT		;and close the file.
;
;   Function to return the first occurence of a specified file
; name. If the first byte of the fcb is '?' then the name will
; not be checked (get the first entry no matter what).
;
GETFST:	LD	C,0		;prepare for special search.
	EX	DE,HL
	LD	A,(HL)		;is first byte a '?'?
	CP	'?'
	JP	Z,GETFST1	;yes, just get very first entry (zero length match).
	CALL	SETEXT		;get the extension byte from fcb.
	LD	A,(HL)		;is it '?'? if yes, then we want
	CP	'?'		;an entry with a specific 's2' byte.
	CALL	NZ,CLEARS2	;otherwise, look for a zero 's2' byte.
	CALL	AUTOSEL		;select proper drive.
	LD	C,15		;compare bytes 0-14 in fcb (12&13 excluded).
GETFST1:CALL	FINDFST		;find an entry and then move it into
	JP	MOVEDIR		;the users dma space.
;
;   Function to return the next occurence of a file name.
;
GETNXT:	LD	HL,(SAVEFCB)	;restore pointers. note that no
	LD	(PARAMS),HL	;other dbos calls are allowed.
	CALL	AUTOSEL		;no error will be returned, but the
	CALL	FINDNXT		;results will be wrong.
	JP	MOVEDIR
;
;   Function to delete a file by name.
;
DELFILE:CALL	AUTOSEL		;select proper drive.
	CALL	ERAFILE		;erase the file.
	JP	STSTATUS	;set status and return.
;
;   Function to execute a sequential read of the specified
; record number.
;
READSEQ:CALL	AUTOSEL		;select proper drive then read.
	JP	RDSEQ
;
;   Function to write the net sequential record.
;
WRTSEQ:	CALL	AUTOSEL		;select proper drive then write.
	JP	WTSEQ
;
;   Create a file function.
;
FCREATE:CALL	CLEARS2		;clear the 's2' byte on all creates.
	CALL	AUTOSEL		;select proper drive and get the next
	JP	GETEMPTY	;empty directory space.
;
;   Function to rename a file.
;
RENFILE:CALL	AUTOSEL		;select proper drive and then switch
	CALL	CHGNAMES	;file names.
	JP	STSTATUS
;
;   Function to return the login vector.
;
GETLOG:	LD	HL,(LOGIN)
	JP	GETPRM1
;
;   Function to return the current disk assignment.
;
GETCRNT:LD	A,(ACTIVE)
	JP	SETSTAT
;
;   Function to set the dma address.
;
PUTDMA:	EX	DE,HL
	LD	(USERDMA),HL	;save in our space and then get to
	JP	DEFDMA		;the bios with this also.
;
;   Function to return the allocation vector.
;
GETALOC:LD	HL,(ALOCVECT)
	JP	GETPRM1
;
;   Function to return the read-only status vector.
;
GETROV:	LD	HL,(WRTPRT)
	JP	GETPRM1
;
;   Function to set the file attributes (read-only, system).
;
SETATTR:CALL	AUTOSEL		;select proper drive then save attributes.
	CALL	SAVEATTR
	JP	STSTATUS
;
;   Function to return the address of the disk parameter block
; for the current drive.
;
GETPARM:LD	HL,(DISKPB)
GETPRM1:LD	(STATUS),HL
	RET	
;
;   Function to get or set the user number. If (E) was (FF)
; then this is a request to return the current user number.
; Else set the user number from (E).
;
GETUSER:LD	A,(EPARAM)	;get parameter.
	CP	0FFH		;get user number?
	JP	NZ,SETUSER
	LD	A,(USERNO)	;yes, just do it.
	JP	SETSTAT
SETUSER:AND	1FH		;no, we should set it instead. keep low
	LD	(USERNO),A	;bits (0-4) only.
	RET	
;
;   Function to read a random record from a file.
;
RDRANDOM: CALL	AUTOSEL		;select proper drive and read.
	JP	READRAN
;
;   Function to compute the file size for random files.
;
WTRANDOM: CALL	AUTOSEL		;select proper drive and write.
	JP	WRITERAN
;
;   Function to compute the size of a random file.
;
FILESIZE: CALL	AUTOSEL		;select proper drive and check file length
	JP	RANSIZE
;
;   Function #37. This allows a program to log off any drives.
; On entry, set (DE) to contain a word with bits set for those
; drives that are to be logged off. The log-in vector and the
; write protect vector will be updated. This must be a M/PM
; special function.
;
LOGOFF:	LD	HL,(PARAMS)	;get drives to log off.
	LD	A,L		;for each bit that is set, we want
	CPL			;to clear that bit in (LOGIN)
	LD	E,A		;and (WRTPRT).
	LD	A,H
	CPL	
	LD	HL,(LOGIN)	;reset the login vector.
	AND	H
	LD	D,A
	LD	A,L
	AND	E
	LD	E,A
	LD	HL,(WRTPRT)
	EX	DE,HL
	LD	(LOGIN),HL	;and save.
	LD	A,L		;now do the write protect vector.
	AND	E
	LD	L,A
	LD	A,H
	AND	D
	LD	H,A
	LD	(WRTPRT),HL	;and save. all done.
	RET	
;
;   Get here to return to the user.
;
GOBACK:	LD	A,(AUTO)	;was auto select activated?
	OR	A
	JP	Z,GOBACK1
	LD	HL,(PARAMS)	;yes, but was a change made?
	LD	(HL),0		;(* reset first byte of fcb *)
	LD	A,(AUTOFLAG)
	OR	A
	JP	Z,GOBACK1
	LD	(HL),A		;yes, reset first byte properly.
	LD	A,(OLDDRV)	;and get the old drive and select it.
	LD	(EPARAM),A
	CALL	SETDSK
GOBACK1:LD	HL,(USRSTACK)	;reset the users stack pointer.
	LD	SP,HL
	LD	HL,(STATUS)	;get return status.
	LD	A,L		;force version 1.4 compatability.
	LD	B,H
	RET			;and go back to user.
;
;   Function #40. This is a special entry to do random i/o.
; For the case where we are writing to unused disk space, this
; space will be zeroed out first. This must be a M/PM special
; purpose function, because why would any normal program even
; care about the previous contents of a sector about to be
; written over.
;
WTSPECL:CALL	AUTOSEL		;select proper drive.
	LD	A,2		;use special write mode.
	LD	(MODE),A
	LD	C,0		;set write indicator.
	CALL	POSITN1		;position the file.
	CALL	Z,WTSEQ1	;and write (if no errors).
	RET	
;
;**************************************************************
;*
;*     BDOS data storage pool.
;*
;**************************************************************
;
EMPTYFCB: DEFB	0E5H		;empty directory segment indicator.
WRTPRT:	DEFW	0		;write protect status for all 16 drives.
LOGIN:	DEFW	0		;drive active word (1 bit per drive).
USERDMA:DEFW	080H		;user's dma address (defaults to 80h).
;
;   Scratch areas from parameter block.
;
SCRATCH1: DEFW	0		;relative position within dir segment for file (0-3).
SCRATCH2: DEFW	0		;last selected track number.
SCRATCH3: DEFW	0		;last selected sector number.
;
;   Disk storage areas from parameter block.
;
DIRBUF:	DEFW	0		;address of directory buffer to use.
DISKPB:	DEFW	0		;contains address of disk parameter block.
CHKVECT:DEFW	0		;address of check vector.
ALOCVECT: DEFW	0		;address of allocation vector (bit map).
;
;   Parameter block returned from the bios.
;
SECTORS:DEFW	0		;sectors per track from bios.
BLKSHFT:DEFB	0		;block shift.
BLKMASK:DEFB	0		;block mask.
EXTMASK:DEFB	0		;extent mask.
DSKSIZE:DEFW	0		;disk size from bios (number of blocks-1).
DIRSIZE:DEFW	0		;directory size.
ALLOC0:	DEFW	0		;storage for first bytes of bit map (dir space used).
ALLOC1:	DEFW	0
OFFSET:	DEFW	0		;first usable track number.
XLATE:	DEFW	0		;sector translation table address.
;
;
CLOSEFLG: DEFB	0		;close flag (=0ffh is extent written ok).
RDWRTFLG: DEFB	0		;read/write flag (0ffh=read, 0=write).
FNDSTAT:DEFB	0		;filename found status (0=found first entry).
MODE:	DEFB	0		;I/o mode select (0=random, 1=sequential, 2=special random).
EPARAM:	DEFB	0		;storage for register (E) on entry to bdos.
RELBLOCK: DEFB	0		;relative position within fcb of block number written.
COUNTER:DEFB	0		;byte counter for directory name searches.
SAVEFCB:DEFW	0,0		;save space for address of fcb (for directory searches).
BIGDISK:DEFB	0		;if =0 then disk is > 256 blocks long.
AUTO:	DEFB	0		;if non-zero, then auto select activated.
OLDDRV:	DEFB	0		;on auto select, storage for previous drive.
AUTOFLAG: DEFB	0		;if non-zero, then auto select changed drives.
SAVNXT:	DEFB	0		;storage for next record number to access.
SAVEXT:	DEFB	0		;storage for extent number of file.
SAVNREC:DEFW	0		;storage for number of records in file.
BLKNMBR:DEFW	0		;block number (physical sector) used within a file or logical sect
LOGSECT:DEFW	0		;starting logical (128 byte) sector of block (physical sector).
FCBPOS:	DEFB	0		;relative position within buffer for fcb of file of interest.
FILEPOS:DEFW	0		;files position within directory (0 to max entries -1).
;
;   Disk directory buffer checksum bytes. One for each of the
; 16 possible drives.
;
CKSUMTBL: DEFB	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
;
;
;	DEFB	0,0,0,0
;
;*
;******************   E N D   O F   C P / M   *****************
;*

;============================================================================================================
;    *** BIOS ExtRom v1.5 ***
; Специализированный BIOS  для проекта korvet-extrom, эмуляции дисководов через порт расширения корвета
; Содержит в себе драйвер эмулируемых дисков и умеет с таких дисков загружаться
;============================================================================================================
;
;
BIAS	EQU (MEM-20)*1024
CCP	EQU 3400H+BIAS 	;base of CCP
BDOS	EQU CCP+806H   	;base of BDOS
BIOS	EQU CCP+1600H  	;base of BIOS
CDISK	EQU 0004H	;число дисков нумерация 0=A,...,15=P
IOBYTE	EQU 0003H	;intel i/o byte
BUFF	EQU 80H
CR	EQU 0DH
LF	EQU 0AH
ESC	EQU 1BH
BS	EQU 08H
STMK0	EQU 0FB00H	; 0 канал ВИ-53
STMRM	EQU 0FB03H	; Рег.режима ВИ-53
SERD	EQU 0FB10H	; Рег.данных ВВ-51 ИРПС
SERS	EQU 0FB11H	; Рег.состояния ВВ-51 ИРПС
LUTC	EQU 0FAFBH	; Адр.табл.писвоения цветов
SYSR1	EQU  3A7FH 	; Системный рег. 1 конфигурации
SYSR3	EQU 0FF7FH	; Системный рег. 3 конфигурации
SYSREG	EQU 0FA7FH	; Системный регистр: карта памяти
CLREG	EQU 0FABFH	; Регистр цвета
VIDBAS	EQU 0FC00H	; Начало ВидеоПамяти
TIMER	EQU 0FB00H	;
INT	EQU 0FB28H	;
ENDEBUF	EQU 0EDFEH	;
VIDEO	EQU 0FB3AH	; ВидеоРегистр
LSTDAT	EQU 0FB30H	; Рег. данных принтера
STBPORT EQU 0FB32H	;
LSTCTL	EQU 0FB33H	; Рег.управления (ИК 55)
DRVREG	EQU 0FB39H	; Регистр управления  дисковода
FDC	EQU 0FB18H	; Контроллер диска
FDC3	EQU  3B18H 	;
CASSIN	EQU 0FB38H	; Статусный регистр
KBDBAS	EQU 0F600H	; Начало поля клавиатуры
;;;;;;;;;;;;;
IRPS	EQU 0FB10H	;ирпс
IS51RS	EQU 40H		;сброс
IS51TR	EQU 035H	;прием и пер-ча раз-ны, DTR снят.
IS51CW	EQU 0CEH	;асинхр.,/16,8 бит,2 стоп бита
KI1	EQU 0F6H
KI2	EQU 0F7H
CON	EQU 4CH
SIDE1	EQU 10H
MOTOR	EQU 20H
DNOTR	EQU 80H		;для кода ошибки

; Некоторые команды конроллера дисков

DRNUM	EQU 08H		; Число накопителей
WRSECCMD EQU 0A0H
FORCECMD EQU 0D0H
NSECTS	EQU 1600H/128	;warm start sector countая ячейка

KB06	EQU 0F840H	;Основное поле клавиатуры
KEYB0	EQU 0F87FH	;Поле клавиатуры без клавиш 'МОДИФ'
MODKB	EQU 0F880H	;Линейка клавиш Модификаций
KB11	EQU 0F920H	;Поле клавиш F1 - F10
DOPK11	EQU 0F93FH	;Дополнительное поле клав-ры
;=============================
;   СИСТЕМНЫЕ ЯЧЕЙКИ ДЛЯ BIOS
;=============================
SYSCOPY	EQU 0F703H	;копия системного регистра
COLCOPY	EQU 0F704H	;копия регистра цвета
BELLDIV	EQU 0F715H	;высота сигнала BELL
BELLDEL	EQU 0F717H	;длительность сигнала BELL
SETFLAG	EQU 0F71AH	;флаг набора символов(0-основ)
COUNT	EQU 0F71FH	;рабочая ячейка
LONGVAL EQU 0F720H 	;задержка автоповтора
AUTOVAL	EQU 0F721H 	;ячейка константы автоповтора
CONTAB	EQU 0F729H 	;адрес таблицы допол.клавиш
FUNTAB	EQU 0F72BH 	;адрес таблици клав. F1-F10
FSHIFT	EQU 0F72DH 	;флаг регистра (0-нижний)
FALF	EQU 0F72EH 	;флаг алфавита (10B-латинск.)
FGRAPH	EQU 0F72FH 	;флаг граф-ий  (100B-граф.)
FSEL	EQU 0F730H 	;флаг цифр     (10000B-цифры)
SNDFLG	EQU 0F731H	;флаг подтверждения нажатия (0-нет)
CONT	EQU 0F736H	;таблица адресов доп.поля клав.
FUNT	EQU 0F752H	;таблица адресов функц.клавиш
LUTFL	EQU 0F76EH	;флаг изменения табл.LUT
ADRLUT	EQU 0F76FH	;адрес таблици LUT
TABLUT	EQU 0F771H	;копия системной табл.LUT
FLGINT	EQU 0F781H	;тип интерфейса принтера
TABINT	EQU 0F7E0H	;уровень прерывания 0
;=======================================================
WRBUFF	EQU 0F200H	; Буффер записи
RRBUFF	EQU (WRBUFF-1024) ; Буффер чтения
EBUF 	EQU RRBUFF-128	; E-диск буфер

; память, используемая BDOS'ом 
CHK03 	EQU EBUF-32  	;check vector 3
CHK02 	EQU CHK03-32 	;check vector 2
CHK01 	EQU CHK02-32 	;check vector 1
CHK00 	EQU CHK01-32 	;check vector 0
ALL04 	EQU CHK00-18 	; E-диск
ALL03 	EQU ALL04-50 	;allocation vector 3
ALL02 	EQU ALL03-50 	;allocation vector 2
ALL01 	EQU ALL02-50 	;allocation vector 1
ALL00  	EQU ALL01-50 	;allocation vector 0
DIRBF 	EQU ALL00-128	;буфер директории
;===========================================
	ORG	0DA00H

START:
;
;**************************************************************
; Таблица переходов BIOS
;**************************************************************
CBOOT:	  JP	 BOOT    ;"холодный" старт
WBOOTE:	  JP	 WBOOT    ;"теплый" старт
	  JP	 CONST    ;статус консоли
	  JP	 CONIN    ;ввод символа с консоли в (А)
	  JP	 CONOUT   ;вывод символа на консоль из (А)
	  JP	 LIST     ;вывод символа на принтер
	  JP	 PUNCH    ;вывод символа на посл.интерфейс
	  JP	 READER   ;прием символа с посл.интерфейса
	  JP	 HOME     ;установка на 0 дорожку
	  JP	 SELDSK   ;выбор диска
	  JP	 SETTRK   ;установка #трека
	  JP	 SETSEC   ;установка #сектора
	  JP	 SETDMA   ;установка вектора DMA
	  JP	 READ     ;операция чтения
	  JP	 WRITE    ;операция записи
PRSTAT:	  JP	 LISTST   ;готовность принтера
SECTRN:   JP	 SECTRAN  ;пересчет логического номера в физ.

; Таблицы параметров дисков
;----------------------------
;     Диск A:
DPH0:
	DW 0000H
;	; адрес таблицы перевода логических секторов
;	; в физические. Если таблица перевода
;	; секторов не используется, эти байты сод. 0
;________
	DS 6	; вспомагательные байты для BDOS
;________
	DW DIRBF
;	; адрес 128 байтовой области памяти,
;	; используемой BDOS для операций с
;	; директорией. DIRBUF один для всех устройств
;________
	DW CPMT00
;	; адрес таблицы параметров дисков. Таблица
;	; для одинаковых дисков может быть одна.
;________
	DW CHK00
;	; адрес выделенной области памяти,
;	; используемой для проверки смены диска.
;	; если диск не может менятся, то CSV = 0.
;	; Для каждого устройства эта область памяти
;	; своя. Размер этой области в байтах равен
;	; числу секторов, занятых директорией.
;________
	DW ALL00
;	; адрес выделенной области памяти для
;	; хранения информации о распределении памяти
;	; на диске. Для каждого устройства эта
;	; область памяти своя. Размер этой области
;	; определяется общим размером диска. 
;	; Резервируется по 1 биту на каждый блок.
;___________________________________________________
;     Диск В:

DPH1: DW 0000H,0000H
      DW 0000H,0000H
      DW DIRBF,CPMT01
      DW CHK01,ALL01

;     Диск С:

DPH2: DW 0000H,0000H
      DW 0000H,0000h
      DW DIRBF,CPMT02
      DW CHK02,ALL02

;     Диск D:

DPH3: DW 0000H,0000H
      DW 0000H,0000H
      DW DIRBF,CPMT03
      DW CHK03,ALL03

;     Диск Е: (электронный)
DPH4: DW 0000H,0000H
      DW 0000H,0000H
      DW DIRBF,CPMT04
      DW 0000H,ALL04

;======================  Таблицы описания дисков ===========================      
;      
;***************************************************
;     ДИСК  A: расширеный блок параметров
;***************************************************
;
;  ФИЗИЧЕСКИЕ ПАРАМЕТРЫ ДИСКА
;--------------------------------------
      DB 50H	;контрольная сумма

      DB 080H ;константа скорости движения головки.
;	; 3 - 30 мс на шаг, 2 - 20 мс, 1 - 12 мс,
;	; 0 - 6 мс и 80H - 3 мс.
      DB 05H  ; число физических секторов на дорожке
      DB 01H  ; информация о сторонах диска
;	;( 00 - односторонный диск
;	;  01 - двухсторонний диск
      DB 03	; 512 bytes/sector
;	;( 00 - 128 байт/сектор
;	;  01 - 256 байт/сектор
;	;  02 - 512 байт/сектор
;	;  03 - 1024 байт/сектор
      DB 0	; номер текущей дорожки.
      DB 0	; позиционный номер устройства (00000100) - для записи в регистр выбора
;---------------------------------------------------
DPBK00:
      DB 0	;INFO	; флаг успешного считывания
;	; информационного сектора
;=====================================================
	; начало таблицы логических параметров диска.
CPMT00:
      DW 28	;128 байтовых секторов/трек ; SPT
      DB 4	; размер блока миним.кбайт  ; BSH
;	; фактор сдвига блока распределения данных
;	; определяется размером блока данных. Блок 
;	; данных - минимально возможная частица файла.
;	; Размер блока - 1, 2, 4, 8 или 16 Кбайт. Если
;	; блок имеет большой размер, в каждом файле 
;	; теряется значительное число неиспользуемых
;	; секторов. При уменьшении размера блока
;	; увеличивается размер директории, описывающий
;	; расположение частей файла. BSH = log2[число
;	; логических секторов в блоке].
      DB 15	; число логических сек/блок  ; BLM
;	; маска блока распределения данных.
;	; BLM = (число логических секторов в блоке)-1.
      DB 0	; extent mask                ; EXM
;	; = (BLM+1)*128/1024 - 1 - [DSM/256]. EXM - 
;	; вспомогательная величина для определения
;	; номера extent'а.
;	; extent - часть файла, описываемая одним 
;	; входом в директорию.
      DW 394	; обьем диска в блоках-1     ; DSM
      DW 127	; входов в директорий-1      ; DRM
      DB 192	; определяет, какие блоки    ; AL0
;	; зарезервированы
      DB 0	; alloc 1                    ; AL1
;	; под директорию. Каждый  бит AL0,AL1, 
;	; начиная со старшего бита AL0 и кончая 
;	; младшим битом AL1, значением 1 резервирует
;	; один блок данных для директории. Нужно
;	; резервировать необходимое число блоков
;	; для хранения входов в директорию: 32*DRM/BLS
      DW 32	; размер вектора контроля директории. ; CKS
;	; CKS=(DRM+1)/4. Если диск не сменяем, CKS=0.
      DW 2	; число системных дорожек    ; OFS

      
;***************************************************
;     ДИСК  B: расширеный блок параметров
;***************************************************
;
;  ФИЗИЧЕСКИЕ ПАРАМЕТРЫ ДИСКА
;--------------------------------------
      DB 50H	; контрольная сумма служ.инф.

TRNS00:

      DB 080H ; скорость головки
      DB 05H  ; Сектор/трек
      DB 01H  ; информация о сторонах диска
        ;( 00 - односторонный диск
        ;  01 - двухсторонний диск
      DB 03	; 512 bytes/sector
        ;( 00 - 128, 01 - 256, 02 - 512, 03 - 1024
      DB 0    ; # текущего трека
      DB 0	; Позиционный # для регистра выбора дисковода

;=======================================

DPBK01:
      DB 0  ; INFO
;=======================================
;          CP/M parameters
CPMT01:
      DW 20  ; 128 байтовых секторов/трек ; SPT
      DB 4   ; размер блока миним.кбайт   ; BSH
      DB 15  ; число логических сек/блок  ; BLM
      DB 01  ; extent mask                ; EXM
      DW 195 ; обьем диска в блоках-1     ; DSM
      DW 127 ; входов в директорий-1      ; DRM
      DB 192 ; alloc 0                    ; AL0
      DB 0   ; alloc 1                    ; AL1
      DW 32  ; check size                 ; CKS
      DW 2   ; число системных дорожек    ; OFS

;***************************************************
;     ДИСК  C: расширеный блок параметров
;***************************************************
;
;  ФИЗИЧЕСКИЕ ПАРАМЕТРЫ ДИСКА
;--------------------------------------
      DB 50H ; контрольная сумма служ.инф.

TRNS01:
      DB 080H ; скорость головки
      DB 05H  ; Сектор/трек
      DB 01H	; информация о сторонах диска
        ;( 00 - односторонный диск
        ;  01 - двухсторонний диск
      DB 03H  ; 1024 bytes/sector
        ;( 00 - 128, 01 - 256, 02 - 512, 03 - 1024
      DB 0   ; # трека
      DB 1   	; Позиционный # для регистра выбора дисковода

;===============================
DPBK02:
      DB 0	; INFO
;===============================

;логические параметры диска
CPMT02:
      DW 40  ; 128 байтовых секторов/трек ; SPT
      DB 4   ; размер блока миним.кбайт   ; BSH
      DB 15  ; число логических сек/блок  ; BLM
      DB 01  ; extent mask                ; EXM
      DW 195 ; обьем диска в блоках-1     ; DSM
      DW 127 ; входов в директорий-1      ; DRM
      DB 192 ; alloc 0                    ; AL0
      DB 0   ; alloc 1                    ; AL1
      DW 32  ; check size                 ; CKS
      DW 00	; число системных дорожек    ; OFS

;***************************************************
;     ДИСК  D: расширеный блок параметров
;***************************************************
;
;  ФИЗИЧЕСКИЕ ПАРАМЕТРЫ ДИСКА
;--------------------------------------
      DB 50H ; контрольная сумма

TRNS02:
      DB 080H ; скорость головки
      DB 05H  ; Сектор/трек
      DB 01H  ; информация о сторонах диска
        ;( 00 - односторонный диск
        ;  01 - двухсторонний диск
      DB 02H  ; 512 bytes/sector
	;( 00 - 128, 01 - 256, 02 - 512, 03 - 1024
      DB 0    ; # текущего трека
      DB 2    	; Позиционный # для регистра выбора дисковода
;================================

DPBK03:
      DB 0  ; INFO
;          CP/M parameters
;================================

CPMT03:
      DW 20  ; 128 байтовых секторов/трек ; SPT
      DB 4   ; размер блока миним.кбайт   ; BSH
      DB 15  ; число логических сек/блок  ; BLM
      DB 01  ; extent mask                ; EXM
      DW 195 ; обьем диска в блоках-1     ; DSM
      DW 127 ; входов в директорий-1      ; DRM
      DB 192 ; alloc 0                    ; AL0
      DB 0   ; alloc 1                    ; AL1
      DW 32  ; check size                 ; CKS
      DW 2   ; число системных дорожек    ; OFS

;***************************************************
;     ДИСК  E: 
 
TRNS03:
;таблица параметров электронного диска-E
CPMT04:

      DW 128 ; 128 байтовых секторов/трек ; SPT
      DB 3   ; размер блока миним.кбайт   ; BSH
      DB 7   ; число логических сек/блок  ; BLM
      DB 00  ; extent mask                ; EXM
      DW 143 ; обьем диска в блоках-1     ; DSM
      DW 31  ; входов в директорий-1      ; DRM
      DB 128 ; alloc 0                    ; AL0
      DB 0   ; alloc 1                    ; AL1
      DW 00  ; check size                 ; CKS
      DW 00  ; число системных дорожек    ; OFS
;==========================================





SISNON:		;DAEE 
 	DB	1FH,0DH,0AH,'CP/M-80  vers. 2.2 ',0DH,0AH
	DB	'EXTROM disk emulation BIOS v1.02 ',0DH,0AH,0
BOOT:
	 DI		; Запрет прерываний
	 LD	 SP,80H	; Setup stack space
	 LD 	 A,1CH	; системный регистр
	 LD	 (SYSREG),A	; установили (на всякий случай)
	 LD	 (SYSCOPY),A
	 CALL	 INIT
	 CALL	 HINIT	;Иници-ия раб.ячеек,табл.клав-ры
	 XOR	 A
	 LD	 (CDISK),A	; активный диск А: & user 0
	 LD	 HL,SISNON
	 CALL	 PSTRNG	; печать начального сообщения
WBOOT:
	 DI
	 LD	 SP,80H	; стек ниже буфера 
	 CALL	 INIT
	 LD 	 C,0	;выбор диска 0
	 CALL	 SELDSK	;Чтение инфор-ого сектора(систем.инф)
	 CALL	 HOME	;трек 00
	 LD 	 B,NSECTS ;число секторов для загрузки
	 LD 	 C,0	;# текущего трека
	 LD 	 D,2	;# текущего сектора
			; сектор 1 содержит спец. информацию
	 LD	 HL,CCP	;начальный адрес загрузки
			; загружаем  1 сектор
LOAD1:
	 PUSH	 BC	;save sector count, current track
	 PUSH	 DE	;save next sector to read
	 PUSH	 HL	;save dma address
	 LD 	 C,D	;# тек.сектора в reg C
	 LD 	 B,00	; add MSB
	 CALL	 SETSEC	;Установка # сектора
	 POP	 BC	;recall dma address to b,c
	 PUSH	 BC	;replace on stack for later recall
	 CALL	 SETDMA	;Установ.адрес DMA
	 CALL	 READ	;Чтение
	 OR	 A	; any errors?
	 JP NZ,WBOOT	;retry boot if an error occurs
	 POP	 HL	;recall dma address
	 LD	 DE,128	;dma=dma+128
	 ADD HL,DE	;new dma address is in h,l
	 POP	 DE	;recall sector address
	 POP	 BC	;recall number of sectors remaining
			; and current trk
	 DEC	 B	;sectors=sectors-1
	 JP Z,GOCPM	;загружены все секторы на запуск CP/M
	 INC	 D
	 LD	 A,(CPMT00) 	;Читать макс. число секторов в треке
	 CP	 D	;сравнить с максимумом
	 JP NC,LOAD1	;carry generated if sector > maximum
	 LD 	 D,1	;счетчик номера сектора уст. в 1
	 INC	 C	;нарастить номер трека
	 PUSH	 BC
	 PUSH	 DE
	 PUSH	 HL
	 LD 	 B,00	; add MSB
	 CALL	 SETTRK	;Установить след.трек
	 POP	 HL
	 POP	 DE
	 POP	 BC
	 JP	 LOAD1	;подолжить чтение

;Запуск CP/M
GOCPM:
	 LD 	 A,0C3H	;c3 is a jmp instruction
	 LD	 (0),A	;for jmp to wboot
	 LD	 HL,WBOOTE ;wboot entry point
	 LD	 (1),HL	;set address field for jmp at 0
	 LD	 (5),A	;for jmp to bdos
	 LD	 HL,BDOS	;bdos entry point
	 LD	 (6),HL	;address field of jump at 5 to bdos
	 LD	 BC,80H	;default dma address is 80h
	 CALL	 SETDMA
	 LD	 A,(CDISK) 	;get current disk number
	 LD 	 C,A	;send to the ccp
	 JP	 CCP	;go to cp/m for further processing

	;Обработка прерываний
	;   Таблица перехода
DOINT0	 EQU	 0F7C8H
DOINT1	 EQU	 0F7CAH
DOINT2	 EQU	 0F7CCH
DOINT3	 EQU	 0F7CEH
DOINT4	 EQU	 0F7D0H
DOINT5	 EQU	 0F7D2H
DOINT6	 EQU	 0F7D4H
DOINT7	 EQU	 0F7D6H
DOINT8	 EQU	 0F7D8H
INT0:
	 PUSH	 HL
	 LD	 HL,(DOINT0)
	 JP	 DOINT
INT1:
	 PUSH	 HL
	 LD	 HL,(DOINT1)
	 JP	 DOINT
INT2:
	 PUSH	 HL
	 LD	 HL,(DOINT2)
	 JP	 DOINT
INT3:
	 PUSH	 HL
	 LD	 HL,(DOINT3)
	 JP	 DOINT
INT4:
	 PUSH	 HL
	 LD	 HL,(DOINT4)
	 JP	 DOINT
INT5:
	 PUSH	 HL
	 LD	 HL,(DOINT5)
	 JP	 DOINT
INT6:
	 PUSH	 HL
	 LD	 HL,(DOINT6)
	 JP	 DOINT
INT7:
	 PUSH	 HL
	 LD	 HL,(DOINT7)
DOINT:
	 PUSH	 AF
	 LD 	 A,1CH	;Конфигурация, принятая в CPM
	 LD	 (SYSREG),A
	 JP (HL)		;вложенные прерывания  не откраваем
OUTINT:		;Выход из 4 ур.прерывания
	 LD	 A,(SYSCOPY) 
	 LD	 (SYSREG),A
	 DI
	 LD 	 A,20H	;зрет прерывания
	 LD	 (INT),A	;уровня 5 (Таймер)
	 POP	 AF
	 POP	 HL
	 EI
	 RET
;Прерывание 4 уровня ( Системное прерывание )
SYST:
	 LD	 HL,0
	 ADD HL,SP
	 LD	 SP,HOME
	 PUSH	 HL	;спасли стек
	 PUSH	 DE
	 PUSH	 BC
;функция LUT - Обработка драйвера просмотровой таблици
	 LD	 A,(LUTFL) 
	 OR	 A
	 JP Z,KBD	;0-Обработка не нужна,на клав-ру
	 CALL	 LUT
	 XOR	 A
	 LD	 (LUTFL),A
KBD:	;Обработка прерываний Клавиатуры
	 LD	 A,(SYMBUF) 
	 OR	  A
	 JP NZ,SKIP1	;прыгаем, если символ в буфере
;			;Выход из прерываний
SKIP5:	; Опрос клавиатуры
	 CALL	 INKEY	;<B> содержит контрольную линейку
	 OR	 A	;<C> содержит скан-код
	 JP Z,SKIP1	;прыгаем, если кнопка не нажата
	 LD	 HL,(DOINT8)	;прерывание от клавиатуры
	 JP (HL)		;программное прерывание (SKIP3)
SKIP3:
	 LD 	 A,C	;Скан-код в A
	 OR	 A
	 JP Z,SKIP1	;контроль:Нет кода
	 LD	 (SYMBUF),A	;Код есть,сохранить
	 LD 	 A,B	;Код Модификации в A
	 LD	 (CNTRKEY),A ;Сохранить
	 LD	 A,(SNDFLG) 	;Флаг выдачи звука
	 OR	 A
	 JP Z,SKIP1	;0- Нет,без звука
	 CALL	 SNDON	;настроили TIMER
	 LD 	 A,01H
	 LD	 (STMK0),A	;щелчок
SKIP1:
	 POP	 BC
	 POP	 DE
	 POP	 HL	;Вернуть стек
	 LD 	 SP,HL
	 JP	 OUTINT
;====================================
	DW	0000H	;61CAH
	DW	0000H	;0CD26H
	DW	0000H	;0B08H
	DW	0000H	;0D7CDH
	DW	0000H	;0C20BH
	DW	0000H	;2649H
	DW	0000H	;0A0E6H
	DW	0000H	;54C2H
	DW	0000H	;0E174H
	DW	0000H	;0102H
	DW	0000H	;0E13FH
	DW	0000H	;0DC3EH
	DW	0000H	;00C3H
LDC82: 
	DW	0000H	;0DC44H
	DW	0000H	;17E1H
;=================================
	; Установка трека 0
HOME:
	 LD 	 C,0	;трек по регистру (C)
SETTRK:
	 LD 	 A,C
	 LD	 (TRKCPM),A
	 RET
	 
	 
;*******************************************************************
;*        Функция 09 - SELDSK
;*  Выбор диска по (С), (HL) содержит адрес
;*  таблицы (на выходе)
;*******************************************************************
SELDSK:
	 LD 	 A,C 		;Номер выбранног дисковода
	 CP	 0FFh		; FF - запрос адреса таблицы векторов
	 LD	 HL,DOINT0
	 RZ            		; Да - отдаем таблицу и на выход
	 CP	 DRNUM		; Проверяем номер устройства - не более DRNUM
	 JP      NC,DSERR	; Ошибка - слишком большой #
; определяем адрес таблицы
	 LD	 HL,DRVTAB
	 LD	 (DSKCPM),A	;Номер диска
	 LD 	 B,0
	 ADD 	 HL,BC
	 ADD 	 HL,BC		;адрес таблицы + 2*dskcpm
	 LD 	 A,(HL)
	 INC	 HL
	 LD 	 H,(HL)
	 LD 	 L,A		;NL = DRH
	 OR	 H
	 JP 	 Z,DSERR	; > 5  (HL = 00 )
	 LD 	 A,C
	 CP	 4
	 RET 	 NC		; Это не FDD а электронный диск - параметры фиксированны
	 PUSH	 HL		; сохранить адрес таблицы DPH 
	 LD	 BC,10
	 ADD 	 HL,BC		;Адрес вектора таблици
	 LD 	 A,(HL)
	 INC	 HL
	 LD 	 H,(HL)
	 LD 	 L,A	;CPMTXX
	 DEC	 HL
	 LD 	 A,(HL)		; Флаг прочитанного инфосектора
	 DEC	 HL
	 LD	 (CPMT),HL	;текущий диск
	 INC	 A		; FF - инфосектор уже прочитан?
	 CALL 	 NZ,GETINFO	; нет - читаем его
	 POP	 HL
	 RZ
DSERR:
	 LD	 HL,0000	; если ошибка, вернем 0
	 XOR	 A
	 LD	 (CDISK),A	; если ошибка, драйв А:
	 RET

	; таблица адресов дисковых таблиц
DRVTAB:
	 DW  DPH0 ; A 0
	 DW  DPH1 ; B 1
	 DW  DPH2 ; C 2 
	 DW  DPH3 ; D 3
	 DW  DPH4 ; E 4		Электронный
	 DW  0    ; F 5
	 DW  0    ; G 6
	 DW  0    ; H 7		HARD

;*******************************************************************
;*        Функция 11 - SETSEC
;*  Выбор сектор по (С)
;*******************************************************************
SETSEC:
	 LD 	 A,C
	 LD	 (SECCPM),A
	 RET
;  Перевод BC --> HL и нарастить HL

SECTRAN:
	LD 	H,B	; Make no translation
	LD 	L,C
	INC	HL	; физический  номер начинается с 1!
	RET
	;DMA устанавливается по регистру (BC)
SETDMA:
	LD 	L,C
	LD 	H,B
	LD	(DMACPM),HL
	RET

;**************************************************************
;   Функция 13 - READ
;      Чтение логического сектора
;      Параметры настроены заранее - SETTRK, SETSEC, SETDMA
;**************************************************************
READ:
	LD 	A,4		;Режим чтения
	LD 	(OPER),A
	LD 	A,(DSKCPM) 	; выбранный диск
	CP	4
	JP 	Z,EDISK		; 4 - рамдиск
	CP	2		; диски А или В ?
	JP	NC,READ_FDD	; нет
	
;
;  Чтение с эмулируемых дисков
;----------------------------------

;	LD	HL,(CPMT)	; Адрес табл. параметров
;	PUSH	HL
;	CALL	FLUSH		;Проверка смены диска
;	POP	HL
;	LD	(CPMT),HL

; формируем командный пакет
;
	LD	(EXR_DRV),A	; # устройства
	LD	A,(TRKCPM)	
	LD	(EXR_TRK),A	; дорожка
	LD	A,(SECCPM)	; сектор
	DEC	A		; у нас номер сектора начинается с 0
	LD	(EXR_SEC),A
	LD	A,1
	LD	(EXR_CMD),A	; 1 - команда чтения
	CALL	EXR_SENDCMD	; отсылаем команду
	DEC	A		; ответ, 0 - ошибка, 1 - ОК
	RET	NZ		; 0 - ошибка
	LD	HL,(DMACPM)	; адрес буфера для приема данных
	CALL	EXR_GETSEC	; принимаем данные
	XOR	A		; завершение всегда успешно
	RET
;
;  Чтение с реального дисковода
;
READ_FDD:	
	CALL 	C,MOTON		;Флоппи-диски
	LD	A,(ERRFL) 	;Читать флаг ошибки
	OR	A
	JP 	NZ,READ1
READ0:
	LD	HL,DREGWR ;HL = адрес хр-ия # тек. драйвера
	CALL	ETR1
	JP 	Z,READ2
READ1:
	LD	HL,(CPMT)	; Адресс табл. параметров
	PUSH	HL
	CALL	FLUSH	;Проверка смены диска
	POP	HL
	LD	(CPMT),HL
	CALL	READ3
	JP	READ0
READ2:
	LD	HL,RRBUFF
	RRA
	LD 	B,A
	LD 	A,0
	RRA
	LD 	C,A
	ADD 	HL,BC
	EX 	DE,HL
	LD	HL,(DMACPM)
	LD 	A,40H
	CALL	DET1
	EI
	LD	A,(ERRFL)  ;
	OR	A
	RET
READ3:
	LD	A,(TRKCPM) 
	LD	(TRKRDB),A
	LD	(TRKWRB),A
	LD	A,(SECCPM) 
	LD	(SECRDB),A
	LD	A,(DSKCPM) 
	LD	(DREGWR),A
	CALL	LTOF
	LD	A,(OFSSEK) 
	LD	(SECBLS),A
	LD	HL,R5+1
	LD 	(HL),10H	;Перезагрузить байт для ( CPI )
	LD 	A,0
	LD	(DREGRD),A	;Н.А. буфера чтения
	LD	HL,RRBUFF
	LD	(REA1),HL
READ4:
	LD	HL,CPMT
	CALL	DSETUP
READ5:
	LD	HL,(REA1)
	CALL	DTOM
	RET 	NZ;Выход "ошибка чтения"
	EX 	DE,HL
	LD	(REA1),HL	;След.адр.буфера чтения
	LD	HL,DREGRD
	LD	A,(OFSSEK+1)
	ADD	A,(HL)
	LD 	(HL),A
R5:
	CP	10H	;Перезагружаемый байт ( число логическ.
;			;секторов в блоке)
	RP
	LD	A,(REA2) 
	CP	0F6H	;Конец ?
	JP 	C,READ6	;Нет
	LD 	A,0FFH
	LD	(ERRFL),A
	RET
 READ6:
	LD	HL,SECSEK
	LD 	A,(HL)
	INC	(HL)
	LD	HL,(CPMT)
	LD	DE,0FFFCH ; Дополнительный код ( 0004 )
	ADD 	HL,DE
	CP	(HL)
	JP 	P,READ7	;Конец трека, на следующий
	INC	A
	LD	(FDC+2),A
	JP	READ5
READ7:
	LD	A,(TRKWRB) 
	INC	A	; Нарастить # трека
	LD	(TRKWRB),A
	LD 	A,1	; Номер сектора уст.1
	LD	(SECRDB),A
	CALL	LTOF
	LD	HL,(CPMT)
	LD	DE,0FFFAH ; Дополнительный код ( 0003 )
	ADD 	HL,DE
	LD	A,(TRKSEK) 
	CP	(HL)
	RP
	JP	READ4
REA1:
	DB 0	; Хранение адреса
REA2:		; Буфера RDBUF
	DB 0	; ( чтения )
EDISK:
	 LD	 A,(TRKCPM) 
	 ADD	 A,A
	 CP	 17
	 LD 	 E,A
	 LD 	 A,1
	 RET 	 NC		;RETURN, если трек большой
	 DI
	 LD 	 D,0
	 LD	 HL,ETRAK
	 ADD 	 HL,DE
	 LD	 A,(VIDEO) 
	 LD	 (STORE),A
	 AND	 3FH
	 OR	 (HL)
	 LD	 (VIDEO),A	;Нужная страница
	 INC	 HL
	 LD 	 A,(HL)
	 LD	 (STORE+1),A;Нужная маска цвета
	 LD	 A,(SECCPM) 
	 RRCA
	 LD 	 E,A
	 AND	 3FH
	 OR	 40H
	 LD 	 D,A	;Старщий байт адреса
	 LD 	 A,80H
	 AND	 E
	 LD 	 E,A	;Младший байт адреса
	 LD 	 B,80H	;Счетчик
	 LD	 HL,EBUF
	 LD	 A,(OPER) 
	 CP	 6
	 CALL 	 Z,EWRITE
	 LD	 A,(STORE+1)
	 INC	 A
	 LD	 (CLREG),A
	 LD 	 A,3CH
	 LD	 (SYSREG),A	;Конфигурация CP/M-80
LPEDISK:
	 LD	 A,(DE)	;Пересылка EBUFF --> E - диск при зап.
	 LD 	 (HL),A	;Пересылка EBUFF <-- E - диск при чтен.
	 INC	 DE
	 INC	 HL
	 DEC	 B
	 JP 	 NZ,LPEDISK
	 LD 	 A,1CH	;Конфигурация 3(графика)
	 LD	 (SYSR3),A	;
	 LD	 A,(STORE) 
	 LD	 (VIDEO),A
	 LD	 A,(OPER) 
	 CP	 4
	 JP 	 Z,EREAD
 LPED1:
	 LD	 A,(COLCOPY)   ;0F704H
	 LD	 (CLREG),A
	 EI
	 XOR	 A
	 RET
EREAD:
	 LD	 HL,(DMACPM)
	 LD	 DE,EBUF
	 LD 	 A,40H
	 CALL	 DET1	;Пересылка из DMA  в EBUFF 64 слова
	 JP	 LPED1
EWRITE:
	 PUSH	 DE
	 EX  	 DE,HL
	 LD	 HL,(DMACPM)
	 EX 	 DE,HL
	 LD 	 A,40H
	 CALL	 DET1
	 POP	 HL
	 PUSH	 HL
	 LD	 A,(STORE+1)
	 LD	 (CLREG),A
	 LD 	 A,3CH
	 LD	 (SYSREG),A
	 LD 	 A,0FFH
	 LD 	 B,80H
LPECLR:
	 LD 	 (HL),A	;Заполнение сектора "FF"
	 INC	 HL
	 DEC	 B
	 JP NZ,LPECLR 
	 LD 	 A,1CH
	 LD	 (SYSR3),A	;
	 POP	 HL
	 LD	 DE,EBUF
	 LD 	 B,80H
	 RET
STORE:
	 DW	 2AC1H	;(28CEH9
ETRAK:
	 DB	 40H,1CH ;1B - маска страницы; 2б- маска цвета
	 DB	 40H,2AH
	 DB	 40H,46H
	 DB	 80H,1CH
	 DB	 80H,2AH
	 DB	 80H,46H
	 DB	0C0H,1CH
	 DB	0C0H,2AH
	 DB	0C0H,46H
ETR1:
	LD	A,(DSKCPM) 	;# дисковода для BDOS
	CP	(HL)	;Сравнить с тек.# драйвера
	RET NZ; # драйвера не соотв.# диска
	INC	HL
	LD	A,(TRKCPM) 	;Читать # трека
	SUB	M
	RET C;Тек.# трека > максимального
	INC	HL
	LD 	E,(HL)
	INC	HL
	LD 	D,(HL)
	LD	HL,(CPMT)	;HL=CPMTх - 2
	INC	HL
	INC	HL	;HL= (SPT)
	JP Z,ETR3	;Тек.# трека = макс.треку
	DEC	A	;-----/----  < ----/----
	RNZ
	LD	A,(SECCPM) 
	ADD	A,(HL)
ETR2:
	SUB	E
	LD 	E,A
	LD 	A,D
	DEC	A
	CP	E
	RC
	XOR	A
	LD 	A,E
	RET
ETR3:
	LD	A,(SECCPM) 
	CP	E
	RET C;Тек.сектор < макс.
	JP	ETR2

;**************************************************************
;   Функция 14 - WRITE
;      Запись логического сектора
;      Параметры настроены заранее - SETTRK, SETSEC, SETDMA
;**************************************************************
WRITE:
	LD 	A,6	;Режим записи
	LD	(OPER),A
	LD 	A,C
	LD	(WRTYPE),A	; тип записи
				; 0 - обычная   1 - в каталог   2 - в новый блок
	LD	A,(DSKCPM) 
	CP	4
	JP 	Z,EDISK	;RAM-диск
	CP	2
	JNC	WRITE_FDD
;
;  Запись на эмулируемый диск
;
;	LD	HL,(CPMT)
;	PUSH	HL
;	CALL	FLUSH
;	POP	HL
;	LD	(CPMTWR),HL
; Формируем командный пакет	
	LD	(EXR_DRV),A
	LD	A,(TRKCPM)
	LD	(EXR_TRK),A
	LD	A,(SECCPM)
	DEC	A
	LD	(EXR_SEC),A
	LD	A,2		; 2 - команда записи
	LD	(EXR_CMD),A
	CALL	EXR_SENDCMD
	DEC	A		; ответ, 0 - ошибка, 1 - ОК
	RET	NZ		; 0 - ошибка
	LD	HL,(DMACPM)
	CALL	EXR_PUTSEC
	XOR	A		; запись успешна
	RET
;
;  Запись на физический дисковод
;
WRITE_FDD:	
	CALL C,MOTON
WRI1:
	LD	HL,DREGWR
	CALL	ETR1
	JP Z,WRI3
	LD	HL,(CPMT)
	PUSH	HL
	CALL	FLUSH
	POP	HL
	LD	(CPMTWR),HL
	LD	(CPMT),HL
	LD	A,(WRTYPE) 
	CP	2
	JP Z,WRI2
	CALL	READ3
	JP	WRI1
WRI2:
	LD	A,(DSKCPM) 
	LD	(DREGWR),A
	LD	HL,(TRKCPM)
	LD	(TRKRDB),HL
	LD	HL,(CPMT)
	LD	DE,0005
	ADD HL,DE
	LD 	A,(HL)
	INC	A
	LD	(DREGRD),A
	XOR	A
	LD	(ERRFL),A
	LD 	E,A
WRI3:
	CALL	WRI4
	LD	A,(WRTYPE) 
	DEC	A
	CALL Z,FLUSH
	LD	A,(ERRFL) 
	OR	A
	RET
WRI4:
	RRA
	LD 	B,A
	LD 	A,0
	RRA
	LD 	C,A
	LD	HL,RRBUFF
	ADD HL,BC
	EX DE,HL
	LD 	A,D
	CP	0F6H
	JP C,WRI5
	LD 	A,0FFH
	LD	(ERRFL),A
	RET
WRI5:
	LD	A,(DREGRD) 
	DEC	A
	CP	L
	JP NZ,WRI6
	LD 	A,1
	LD	(WRTYPE),A
WRI6:
	LD 	A,0FFH
	LD	(BUFFAC),A
	LD	HL,(CPMT)
	LD	(CPMTWR),HL
	LD	HL,(DMACPM)
	EX DE,HL
	LD 	A,40H
	CALL	DET1
	EI
	RET

;запись буфера на диск ( если буфер изменился )
FLUSH:
	LD	A,(BUFFAC) 	;Диск менялся ?
	OR	A
	RET Z; НЕТ
	XOR	A
 	LD	(BUFFAC),A	; Чистить флаг смены диска
	LD	HL,(CPMTWR)	; Адр.табл.нового диска
	LD	(CPMT),HL	; изменить
	LD	HL,(TRKRDB)	;
	LD	(TRKWRB),HL	; Обновить номер трека
	CALL	LTOF	; Контроль
	LD 	A,0
	LD	(NSEC),A	; Сектор 0
	LD	HL,RRBUFF; Буфер чтения
	LD	(REA1),HL	; Сохранить тек.адресс буфера
	LD 	A,164
	LD	(MTODR+2),A	;Изменение для MTOD
FLUS0:
	LD	HL,CPMT
	CALL	DSETUP	;Установить головку
FLUS1:
	LD	HL,(REA1)
	CALL	MTOD	;Чтение с диска
	EX DE,HL
	LD	(REA1),HL
	LD 	A,160
	LD	(MTODR+2),A ;Увеличить адресс RRBUFF
	LD	HL,NSEC
	LD	A,(OFSSEK+1); Чтен.смещение сектора
	ADD	A,(HL)
	LD 	(HL),A
	LD	A,(DREGRD) 	; Количество сторон диска
	CP	(HL)
	RZ
	RM
	LD	A,(REA2) 
	CP	0F6H	;Конец буфера ?
	JP C,FLUS2	;нет,продолжить чтение
	LD 	A,0FFH
	LD	(ERRFL),A	;Флаг ошибки
	RET
FLUS2:
	LD	HL,SECSEK  ;Тек.# сектора
	LD 	A,(HL)
	INC	(HL)	;Следующий # сектора
	LD	HL,(CPMT)	;Адрес драйвера
	LD	DE,0FFFCH ;Обр.код (0002)
	ADD HL,DE	;минус 2
	CP	(HL)	; сектор/трек
	JP P,FLUS4	; Трек весь
	INC	A	; След.сектор
	LD	(FDC+2),A	; # тек.сектора
	JP	FLUS1	; Продолжить чтение
;  Переход на следующий трек
FLUS4:
	LD	A,(TRKWRB) 
	INC	A
	LD	(TRKWRB),A	;След.тек.трек
	LD  	A,1
	LD	(SECRDB),A	;Сектор = 1 (начало трека)
	CALL	LTOF	;Определить сторону диска
	JP	FLUS0
WRTYPE:
	DB 0  	; тип записи
	   	; 0 - ординарная запись
	   	; 1 - в директорий запись
	   	; 2 - в новый блок запись
BUFFAC:
	DB 0	; Флаг смены буфера
		; =/= - Write to unallocated block
ERRFL:
	DB 0	; Флаг ошибки

;       DSETUP - Subprogramme to setup FDC and DRVREG before
;                      actual real/write
	;  HL points to Header in that way :


; ___________________________________________________________
; I 8"/5"I /DDEN I MOTO I SIDE I DS3  I  DS2 I  DS1 I  DS0  I
; I------I-------I------I------I------I------I------I-------I
; I  D7  I  D6I  I  D5  I  D4  I  D3  I  D2  I  D1  I   D0  I
; I______I_______I______I______I______I______I______I_______I
; I диам I плот- I вкл  I сто- I  Н О М Е Р А   Д И С К О В I
; I дискаI ность IмотораI рона I  'D' I 'C'  I 'B'  I 'A'   I
; I______I_______I______I______I______I______I______I_______I
;       Select disk
;  HL - Address of parameter block for setup

DSETUP:
	LD 	E,(HL)
	INC	HL
	LD 	D,(HL)	;Адрес CPMTxx В DE
	INC	HL	; # тек.дисковода
	CALL	FORCE	; Сбросить контроллер дисковода
	LD 	A,(HL)	; # тек.дисковода
	LD	(DRVREG),A
	PUSH	DE
	CALL	MOTON	;Вкл. мотор
	POP	DE
	DEC	DE	; DE points to current track now
	INC	HL
	LD 	A,(HL)
	OR	A
	LD 	C,A
	INC	HL
	LD 	A,(HL)
	LD	(FDC+2),A	;Рег. сектора
	JP 	Z,SK4
	LD	A,(DE)	;Читать # тек.трека
	LD	(FDC+1),A
	CP	C
	RZ
	LD 	A,C	;След.трек
	LD	(FDC+3),A
	LD 	C,18H	;Команда "ПОИСК"
	JP NZ,DZET1
SK4:
	LD 	A,C
	LD 	C,8	;Команда "_______________"
DZET1:
	LD	(DE),A
	DEC	DE
	DEC	DE
	DEC	DE
	DEC	DE	;сие указывает на скорость головки
	LD	A,(DE)	;читать константу скорости(=80H)
	AND	0C0H	; D7-5"/8"....D6-плотность(0-двойн)
	LD 	B,A
	LD	A,(DRVREG) 
	PUSH	AF
	OR	B
	LD	(DRVREG),A
	LD	A,(DE)
	AND	3
	OR	C
	CALL	DELAY3	;Задержка
WAITB2:
	LD	A,(FDC) 
	RRCA
	JP C,WAITB2	; If not DONE yet - wait
	POP	AF	;Есть готовность
	LD	(DRVREG),A
	LD 	B,0FH
	CALL	DELAY1	;Задержка 30 мск
	JP	FORCE

;Вывод сообщения на экран
PSTRNG:
	LD 	A,(HL)	; Print message pointed by HL to 0
	OR	A	; Zero ?
	RZ
	PUSH	HL	; Save HL
	LD 	C,A
	CALL	CONOUT
	POP	HL	; Restore HL
	INC	HL	; Bump pointer
	JP	PSTRNG

	;Загрузка массива из M (DE) в M (HL)
	; BC = Размер массива
DETOHL:
	LD 	A,B
	OR	C
	RZ
	LD	A,(DE)
	LD 	(HL),A
	INC	HL
	INC	DE
	DEC	BC
	JP	DETOHL
DET1:
	OR	A
	RZ
	PUSH	HL
	LD	HL,0002
	ADD HL,SP
	LD	(STACKS),HL
	EX DE,HL
	POP	DE
	DI
	LD SP,HL
	EX DE,HL
DET2:
	POP	DE
	LD 	(HL),E
	INC	HL
	LD 	(HL),D
	INC	HL
	DEC	A
	JP NZ,DET2
	LD	HL,(STACKS)
	LD SP,HL		;Вернуть стек
	RET
;***************************************

STACKS:
	DW	2AC1H   ; Хранение стека

	; Блок CPM/МикроДос параметров (128 байт/сектор)

DSKCPM:	DB	1	; # дисковода для BDOS
CHW:	DB	80H
OPER:	DB	4	;Режим (4-чтение, 6-запись)
NSEC:  	DB	1	; # сектора
TRKCPM: DB	0	; # трека   для BDOS
SECCPM: DB	1	; # сектора для BDOS
DMACPM: DW	80H	; DMA ADDRESS
UNUSED: DW	0

; Блок физических параметров (LTOF делает из CPM параметров)
CPMT: 	DW	CPMT00;Адрес таблици текущего диска
DREG: 	DB	1	; # тек.диска
TRKSEK: DB	0	; # тек.  трека (физического)
SECSEK: DB	0	; # тек.сектора (физического)
OFSSEK: DW	0000H	; смещение сектора в буфере
TRKWRB: DB	1	;

  ; ЛОГИЧЕСКИЕ ПАРАМЕТРЫ
SECRDB:	DB	1	;
CPMTWR: DW	CPMT00 ;
DREGWR: DB	0	; маска выбора дисковода
TRKRDB: DB	0	;
SECBLS: DB	0	;
DREGRD: DB	0	; 01 - одност-ний 02 - двухст-ний
;=================================================
;	Драйвер дисплея 
; C содержит код символа
CONOUT:
	LD 	A,C
	CP	7
	JP Z,BEEP
	LD	HL,0
	ADD HL,SP
	LD	SP,ENDEBUF  ;буфер электронного диска!
	PUSH	HL
	LD 	A,14H
	DI
	LD	(SYSCOPY),A	;сохраняем копию
	LD	(SYSREG),A
	EI
	CALL    CON	;печатаем
	LD      A,1CH
	DI
	LD     (SYSREG),A
	LD     (SYSCOPY),A	;сохраняем копию
	EI
	POP	HL
	LD SP,HL
	RET
BEEP:
	LD 	A,36H
	LD	(STMRM),A
	LD	HL,(BELLDIV)
	LD	DE,0FB00H
	EX DE,HL
	LD 	(HL),L
	LD 	(HL),D
	LD 	L,32H
	LD 	B,(HL)
	LD 	A,8
	OR	B
	LD 	(HL),A	;включили звук
	EX DE,HL
	LD	HL,(BELLDEL)
LPBEEP:	
	DEC	HL
	LD 	A,L
	OR	H
	JP NZ,LPBEEP
	EX DE,HL
	LD 	(HL),B
	LD	A,(SNDFLG) 
	OR	A
	RZ
SNDON:
	LD 	A,20H
	LD	(STMRM),A	;рег.режима
	LD 	A,7
	LD	(LSTCTL),A
	RET
;===========================================
;ДРАЙВЕР КЛАВИАТУРЫ (Сканирование клавиатуры)
INKEY:
	LD	A,(KEYB0) 		;Опрос основного поля
	OR	A
	JP NZ,INKEY5		; есть
	LD	A,(DOPK11) 	; опрос доп.поля и функц.клавиш
	OR	A
	JP NZ,INKEY4
	LD	A,(MODKB) 		; поле модификаторов
	OR	A
	JP NZ,INK2
INK1:
	XOR	A
	LD	(KMODIF),A	; Нет, очистить яч-ку KMODIF
	RET
   ;НАЖАТИЕ ПОЛЕ МОДИФИКАЦИИ
INK2:
	LD 	B,A
	AND	01001000B  ;Маска клавиш ESC , ФКС
	JP Z,INK1	; нет
	LD 	C,0	; Да,подготовить C для # кода KMOD
	CALL	L1INK	; Преобразовать в двоично/десятичный
	LD 	A,56	; Конец # в строке KMOD
	ADD	A,C	; Минус полученный номер
	LD 	C,A
	CP	59	; Клавиша 'ESC' ?
	JP Z,INKEY6	; Да
	LD	HL,KMODIF;
	SUB	M	; ФКС была раньше ?
	RET Z; Да,выйдем
	LD 	(HL),C	; Нет, обновим ячейку
	LD	HL,FSHIFT;
	LD 	A,10000001B  ; Маска клав.РГ (прав. и лев.)
	AND	B	;
	JP Z,INKEY3	; Нет нажатия РГ 
	LD 	A,0FFH	;
	XOR	(HL)	;
	LD 	(HL),A	;
INKEY3:
	INC	HL
	LD 	A,00000010B  ;Маска клав.АЛФ
	AND	B
	XOR	(HL)
	LD 	(HL),A
	LD 	A,00000100B  ;Маска клав.ГРАФ
	INC	HL
	AND	B
	XOR	(HL)
	LD 	(HL),A
	INC	HL
	LD 	A,00010000B  ;Маска клав.ЦИФР
	AND	B
	XOR	(HL)
	LD 	(HL),A
	XOR	A
	RET		; Выход = 00

   ; ПОЛЕ F1 -- F10
INKEY4:
	LD	HL,KB11
	CALL	INKEY10
	RZ
	LD 	A,40H
	ADD	A,C
	LD 	C,A
	JP	INKEY6

   ;ОСНОВНОЕ ПОЛЕ
INKEY5:
	LD	HL,KB06
	CALL	INKEY10
	RZ

   ; КЛАВИША 'ESC'
INKEY6:
	LD	DE,KMODIF
	LD	HL,COUNT
	LD	A,(DE)
	SUB	C
	JP Z,INKEY8
	LD	A,(LONGVAL) 

   ; ОБРАБОТКА ДРЕБЕЗГА КЛАВИШ
INKEY7:
	LD 	(HL),A
	LD	A,(MODKB) 
	LD	(CNTRKEY),A
	LD 	B,A
	LD 	A,C
	LD	(DE),A
	OR	A
	RET
INKEY8:
	LD	A,(AUTOVAL) 
	DEC	(HL)
	JP Z,INKEY7
	XOR	A
	RET

   ; СДВИГ БИТА ЛИНЕЙКИ
INKEY9:
	LD 	A,L
	RRA
	LD 	L,A	;Следующая линейка
	OR	A
	RET Z; Все линейки пройдены
INKEY10:
	LD 	A,(HL)
	OR	A
	JP Z,INKEY9
	LD 	C,0
	PUSH	AF	; Сохранить байт линейки
	LD 	A,L
	CALL	L1INK
	LD 	A,C
	ADD	A,A
	ADD	A,A
	ADD	A,A	;Умножить на 8(число символов в строке)
	INC	A
	LD 	C,A
	POP	AF

   ; Преобразование # линейки в номер скан/кода
L1INK:
	RRCA
	RET C;0 бит = 1
	INC	C	;Счетчик /двоично-десятичный/
	JP	L1INK
;****************************
;  ОПРОС СОСТОЯНИЯ КЛАВИАТУРЫ
;****************************
;Выход: A = 00 -  	;нет нажатия
;	A = FF/скан-код	;есть нажатие
CONST:
	LD	A,(ADR1) 
	OR	A
	RNZ
	LD	A,(SYMBUF) 	;Нет, читать скан-код
	OR	A
	RET Z;Нет такой клавиши
	LD 	A,0FFH
	RET		;Выход с кодом
;****************************
;  ВВОД СИМВОЛА С КЛАВИАТУРЫ
;****************************
CONIN:
	LD	A,(ADR1) 
	OR	A	;Есть F-последовательность ?
	LD	HL,(ADR)	;Да,
	JP NZ,LE2AD
CONIN1:
	EI
	LD	A,(SYMBUF) 
	OR	A
	JP Z,CONIN1
	LD 	C,A
	XOR	A
	LD	(SYMBUF),A
	DEC	C
	LD 	B,0
	LD	A,(CNTRKEY)  ;Читать линейку клавиш 'модификаций'
	LD 	E,A
	LD 	A,C	;Скан-код в A
	CP	32
	JP C,ALFKEY	; < 32 - Буквы
	CP	44
	JP C,DIGK1	; < 44 - Цифры,знаки
	CP	48
	JP C,DIGKEY	; < 48 - Знаки
	CP	56
	JP C,CTLKEY	; < 56 - Упр.клав.
	CP	64
	JP C,CTLK1	; < 64 - Клав.модификации
	CP	80
	JP C,AUXKEY	; < 80 - Цифры доп.поля
	CP	96
	JP C,FUNKEY	; < 96 - Клавиши F1 - F10
	CP	112
	JP C,AUXK1	; < 112 - Доп.поле / ESC- послед.
	XOR	A
	RET		; A = 00 - нет кода
ALFKEY:
	LD	HL,LALFTAB ; Адрес тавлицы букв
	ADD HL,BC	; HL+SKOD = адрес символа в таблице
	LD 	L,(HL)	; L - код символа
	LD 	A,E	; A - линейка 'модификатора'
	AND	00100000B ; УПР ?
	JP NZ,LE1DE	  ; ДА
	LD	A,(FGRAPH) 
	XOR	E	; Маска лин.модиф.
	AND	00000100B ; ГРАФ. ?
	JP Z,LE1E2	  ; НЕТ
	LD 	A,L
	XOR	0C0H	; Инверсия 6,7 бит
	RET

;  КОД - УПР (CTRL)
LE1DE:
	LD 	A,L	;Вернуть код символа
	AND	00011111B ; Сброс 5,7 бит
	RET

;  
LE1E2:
	LD	A,(FALF) 
	XOR	E
	AND	00000010B
	JP NZ,LE1F7
	LD	A,(SETFLAG) 
	OR	A
	JP Z,LE20D	;Основной набор
	LD	HL,SIMTAB1 ; Адрес табл.рус.букв по алф.порядку
	ADD HL,BC
	LD 	L,(HL)	; L = Символ
LE1F7:
	CALL	LE1FC
	ADD	A,L
	RET
LE1FC:
	LD 	A,81H	; Маска прав,лев. клав.SHIFT
	AND	E
	JP Z,LE204	;Клавиши РЕГ не нажата
	LD 	A,0FFH	; Флаг нажатия SHIFT
LE204:
	LD 	E,A
	LD	A,(FSHIFT) 
	XOR	E	; Инверсия
	RET Z; Если РЕГ нижний
	LD 	A,20H
	RET

;  АЛФАВИТ РУССКИЙ (основной набор)
LE20D:
	CALL	LE1FC
	XOR	0A0H
	ADD	A,L	;Код символа + 5 или 7 бит
	RET
LALFTAB:
	DB	'@ABCDEFGHIJKLMNO'
ALFTAB1:
	DB	'PQRSTUVWXYZ[\]^_'
SIMTAB1:
;		  н            ф              д
	DB	0CEH,0B0H,81H,0C6H,0B4H,0B5H,0C4H,0B3H
;                 е                          
	DB	0C5H,0B8H,0B9H,0BAH,0BBH,0BCH,0BDH,0BEH
;                     о    ю    а    б    ц    
	DB	0BFH,0CFH,0C0H,0C1H,0C2H,0C3H,0B6H,0B2H
;                л    к         х    м    и    г    й
	DB	0CCH,0CBH,0B7H,0C8H,0CDH,0C9H,0C7H,0CAH
CTLTAB:
	DB	0DH	;клавиша  'ВК'
	DB	8DH	;клавиша  'CLS'
	DB	03H	;клавиша  'СТОП'
	DB	8BH	;клавиша  'DEL'
	DB	8CH	;клавиша  'INS'
	DB	08H	;клавиша  'BS'    (<--- )
	DB	09H	;клавиша  'TAB'
	DB	20H	;клавиша  'SPACE' (пробел)

;  КЛАВИША ЦИФРА
DIGK1:
	LD 	A,E
	AND	10000001B
	LD 	A,C
	RNZ
	ADD	A,00010000B
	RET

;   Знаки
DIGKEY:
	LD 	A,E
	AND	10000001B
	LD 	A,C
	RZ
	ADD	A,00010000B
	RET

;   Управляющие клавиши
CTLKEY:
	LD	HL,ALFTAB1
	ADD HL,BC
	LD 	A,(HL)
	CP	80H
	RC
	LD 	C,A
	JP	LE299

CTLK1:	; КЛАВИША  'ESC'
	LD 	A,1BH
	RET

AUXKEY:	; Цифры доп.поля
	LD	A,(FSEL) 
	XOR	E
	AND	00010000B
	JP Z,AUXK1
LE284:
	LD 	A,60H
	XOR	C
	CP	2AH
	RNC
	ADD	A,00010000B
	RET

AUXK1:	;  Доп.поле / ESC-последовательность
	LD 	A,81H
	AND	E
	JP Z,LE299
	CALL	LE284
	ADD	A,60H
	RET

LE299:	;  Доп.поле
	LD 	A,0FH
	AND	C
	LD 	C,A
	LD	HL,(CONTAB)
	CP	0EH
	JP C,LE2A7
	LD 	C,0AH

LE2A7:	;  Определение адреса в таблице
	ADD HL,BC
	ADD HL,BC
	LD 	A,(HL)
	INC	HL
	LD 	H,(HL)
	LD 	L,A
LE2AD:
	LD 	A,(HL)
	INC	HL
	LD	(ADR),HL
	LD 	D,A
	OR	A
	JP Z,LE2B8
	LD 	A,(HL)
LE2B8:
	LD	(ADR1),A
	LD 	A,D
	RET

FUNKEY:	;  Функциональные клавиши F1 - F10
	LD	HL,(FUNTAB)
	LD 	A,0FH
	AND	C
	LD 	C,A
	CP	5
	JP NC,LE2A7
	LD 	A,81H
	AND	E
	JP Z,LE2A7
	LD 	A,C
	ADD	A,5
	LD 	C,A
	JP	LE2A7

KMODIF:
	DB	0	;
ADR1:
	DB	0	;
ADR:
	DW	0000H	;Текущий вектор F-последовательности
SYMBUF:
	DB	0	;Скан-код
CNTRKEY:
	DB	0	;Байт линейки 'модиф'

      ;*******П/П  ВЫВОД НА ПРИНТЕР********

LIST:
	CALL	LISTST
	JP Z,LIST	;ждать готовнось принтера
	LD 	A,C	;готовность есть,код в А
	CP	80H	;код меньше 80H ?
	JP C,LIST1	;да, символы латынь
	LD	HL,SIMTAB1+16  ;Н.адрес буфера символов(0Е307H - 80H)
	LD 	B,0
	ADD HL,BC	;в HL получим адрес для таблицы
	LD 	A,(HL)	; код из таблицы в А
LIST1:
	XOR	0FFH	; код инвертируем
	LD	(LSTDAT),A	; выводим в порт, на принтер
	LD	HL,LSTCTL
	LD 	(HL),0BH	;выдать строб (1)
	EX (SP),HL		;задержка
	EX (SP),HL
	LD 	(HL),0AH	;сбросить строб (0)
	RET
LISTST:
	LD	A,(CASSIN) 		;читать 
	AND	00000100B	;маска 2 
	RET Z;нет 
	LD 	A,0FFH		;готовность 
	RET

	;**************************************
	DB	80H,81H,82H,83H,84H,85H,86H,87H
	DB	88H,89H,8AH,8BH,8CH,8DH,8EH,8FH
	DB	90H,91H,92H,93H,94H,95H,96H,97H
	DB	98H,99H,9AH,9BH,9CH,9DH,9EH,9FH
	DB	0A0H,0A1H,0A2H,0A3H,0A4H,0A5H,0A6H,0A7H
	DB	0A8H,0A9H,0AAH,0ABH,0ACH,0ADH,0AEH,0AFH
	DB	0B0H,0B1H,0B2H,0B3H,0B4H,0B5H,0B6H,0B7H
	DB	0B8H,0B9H,0BAH,0BBH,0BCH,0BDH,0BEH,0BFH

	;таблица руского алфавита с C0H доFFH - пока забил нулями, потом надо вписать коды кои-8

;	DB	'юабцдефгхийклмно'
;	DB	'пярстужвьызшэщчъ'
;	DB	'ЮАБЦДЕФГХИЙКЛМНО'
;	DB	'ПЯРСТУЖВЬЫЗШЭЩЧ',0FFH
	DB	 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	DB	 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	DB	 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	DB	 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0xff

;	П/П ВЫВОД СИМВОЛА НА ИРПС
PUNCH:
	CALL	PUNC0	;опрос готовности
	JP Z,PUNCH	;нет готовности
	LD 	A,C	;есть готовнось,код в A
	LD	(SERD),A	;выдать байт данных
	RET
PUNC0:
	LD	A,(SERS) 	;читать регистр состояния ВИ 51
	AND	10000001B;маска готовности (данных) УВВ
	XOR	10000001B;инверсия 1 и 7 биты
	JP NZ,PUNC1
	XOR	0FFH
	RET
PUNC1:
	XOR	A
	RET		;выход по неготовности (00)

;      ПРИЕМ С ИРПС ПОСЛЕДОВАТЕЛЬНЫЙ ИНТЕРФЕЙС
READER:
	CALL	RED2	;опрос готовности
	JP NZ,RED1	;гот-ть есть, читать данные
	DI
	LD 	(HL),27H	;режим:ск-ть,прием/пер.
RED0:			;вкл контроль четн.1бит стопа
	CALL	RED3	;опрос гот-ти
	JP Z,RED0	;гот-ти нет
	LD 	(HL),25H	;програмный сброс
RED1:
	DEC	HL	;HL=рег.данных
	LD 	A,(HL)	;данные в A
	EI
	RET		;выйти с данными в A
RED2:
	LD	HL,SERS
RED3:
	LD 	A,(HL)	;читаем рег.состояния
	AND	00000010B;маска бита готовности данных
	RZ
	LD 	A,0FFH
	RET
;**************************************
	DW TABTRAP
TABTRAP:
	DW OUTINT	;0 расш. разъем
	DW OUTINT	;1 
	DW OUTINT	;2 
	DW OUTINT	;3 
	DW SYST		;4 системный clock (20мс)
	DW OUTINT	;5 timer
	DW OUTINT	;6 printer
	DW OUTINT	;7 одновибратор мотора
	DW SKIP3	;8 клавиатура (вызов опроса)
	DW 0		;AUXI1
	DW 0		;AUXI2
	DW 0		;AUXI3

TBINT:
	JP INT0	;0F7E0H
	DB 0		; USER
	JP INT1	;0F7E4H
	DB 0
	JP INT2	;0F7E8H
	DB 0
	JP INT3	;0F7ECH
	DB 0
	JP INT4	;0F7F0H
	DB 0
	JP INT5	;0F7F4H
	DB 0
	JP INT6	;0F7F8H
	DB 0
	JP INT7	;0F7FCH

;	конец таблиц
INIT:
;при вызове прерывания должны быть закрыты
;адрес INT-таблицы - 0F7E0H : забрасываем
	DI
;Иницилизация векторов переходов прерываний
	LD	DE,TABTRAP-2
	LD	HL,DOINT0-2   ;начало области ячеек прерывания
	LD	BC,58
	CALL	DETOHL
;Контроллер прерываний
	LD 	A,KI1	; INIT INTERUPT
	LD	(INT),A
	LD 	A,KI2
	LD	(INT+1),A	;
	LD 	A,0EFH	;maska
	LD	(INT+1),A
;Последовательный интерфейс
	LD	HL,IRPS+1	; INIT IRPS
	XOR	A
	LD 	(HL),A
	LD 	(HL),A
	LD 	(HL),A		; сброс програмный
	LD 	(HL),IS51RS
	LD 	(HL),IS51CW
	LD 	(HL),IS51TR
;TIMER
	LD	HL,TIMER+3
	LD 	(HL),7EH	; init irps timer. КАНАЛ 1 РЕЖИМ 3
	LD 	(HL),14H
	LD 	(HL),90H	; канал 2,режим 0,младший байт
	DEC	HL	; 2
	DEC	HL	; 1
	LD 	(HL),13	;9600 bod
	LD 	(HL),0
;Графика
	LD	A,(VIDEO) 
	AND     4
	OR	020H	;Графическая страница 0
	LD	(VIDEO),A
	LD     HL,0	;стираем страницу 0 (графика)
	ADD HL,SP
	EX DE,HL
	LD      A,3CH
	LD     (SYSREG),A
	LD     SP,4000H+4000H
	LD     HL,0FFFFH
	LD     BC,800H
	LD      A,0
	LD     (0FFBFH),A
LPCLS1:
	PUSH    HL
	PUSH    HL
	PUSH    HL
	PUSH    HL
	DEC     C
	JP NZ,LPCLS1
	DEC     B
	JP NZ,LPCLS1
	EX DE,HL
	LD SP,HL
	LD      A,1CH
	LD     (SYSR3),A
;Иницилизируем диски, клавиатуру и дисплей
	XOR	A	; очистить
	LD	(DPBK00),A	; флаг успешного считывания диска A
	LD	(DPBK01),A	; --------------/---------- диска B
	LD	(DPBK02),A	; --------------/---------- диска C
	LD	(DPBK03),A	; --------------/---------- диска D
	LD	(IOBYTE),A	; Select "wrong buffer "
	LD	(SYMBUF),A	; Clear Last Received Character
	LD 	A,0FFH
	LD	(LUTFL),A	; очистить флаг изменения LUT
	LD	HL,INITVD;|
	CALL	PSTRNG	;| Промт на экран
	LD 	A,0FFH
	LD	(SNDFLG),A	;включили  звук
;   Клавиатура
	LD 	A,02
	LD	(FALF),A	; Латынь
;   Принтер
	LD 	A,0C9H	;
	LD	(STBPORT),A	; Строб на принтер
	RET

;       Инициализация табл. LUT
LUT:
	CALL	TBLANK
	LD	HL,(ADRLUT)
	LD 	B,16
LTLP:
	LD 	A,(HL)	;Загрузить в копию просмотровую табл.
	LD	(LUTC),A
	INC	HL
	DEC	B
	JP NZ,LTLP
	RET
;         Ожидание конца кадрового импульса
TBLANK:
	LD	A,(CASSIN) 
	AND	2
	JP NZ,TBLANK
	LD 	A,50
LPDL:
	EX (SP),HL
	EX (SP),HL
	DEC	A
	JP NZ,LPDL
	RET
INITVD:
	DB 1BH,';',1BH,':',0	;включили курсор
HINIT:
	LD	DE,TABFK	;Таблица функц.клавиш
	LD	HL,KBDBAS;Адрес загрузки
	LD	BC,0F4H  ;Размер таблици
	CALL	DETOHL  ;Перегрузить массив
	LD	DE,TABPR	;Рабочие ячейки BIOS
	LD	HL,0F715H;Адрес загрузки
	LD	BC,006FH	;Размер таблици
	JP	DETOHL  ;Перегрузить массив

;  МАССИВ ПЕРЕНОСИТСЯ ПО АДРЕСУ 0F600H - 0F783H
TABFK:
	DB 'DIR ',0,0,0,0,0,0,0,0,0,0,0,0
	DB 'TYPE ',0,0,0,0,0,0,0,0,0,0,0
	DB 'ERA ',0,0,0,0,0,0,0,0,0,0,0,0
	DB 'REN ',0,0,0,0,0,0,0,0,0,0,0,0
	DB 'A:ST',0DH,0,0,0,0,0,0,0,0,0,0,0
	DB 'BASIC ',0,0,0,0,0,0,0,0,0,0
	DB 'F6',0,0,0,0,0,0,0,0,0,0,0,0,0,0
	DB 'SAVE ',0,0,0,0,0,0,0,0,0,0,0
	DB 'F8',0,0,0,0,0,0,0,0,0,0,0,0,0,0
	DB 'F9',0,0,0,0,0,0,0,0,0,0,0,0,0,0

;  ДОПОЛНИТЕЛЬНОЕ ПОЛЕ
	DB 80H,0,0,0,0,0,81H,0,0,0,0,0
	DB 82H,0,0,0,0,0,83H,0,0,0,0,0
	DB 84H,0,0,0,0,0,85H,0,0,0,0,0
	DB 86H,0,0,0,0,0,87H,0,0,0,0,0
	DB 88H,0,0,0,0,0,89H,0,0,0,0,0
	DB 8AH,0,0,0,0,0,'DEL',0,0,0
	DB 'INS',0,0,0,'CLS',0,0,0
;----------------------------------------------
;   РАБОЧИЕ ЯЧЕЙКИ BIOS  ( F715H - F783H )
TABPR:
	DW	0B00H	;Высота сигнала BELL
	DW	1000H	;Длительность   BELL
;  Рабочие ячейки драйвера клавиатуры
	DB	0FFH	;Буфер символа для задержки
	DB	0	;Флаг набора / 0- основной /
	DB	0
	DB	0
	DB	0
	DB	0
	DB	0	;Раб.яч. задержки автоповт.
	DB	1EH	;Задержка автоповтора
	DB	2	;Период автоповтора
	DB	0
	DW	SYMBUF	;Указатель буфера скан-кода
	DW	SYMBUF	;------------//-----------
	DB	0
	DB	0
	DW	CONT	;Адрес табл.доп.клав
	DW	FUNT	;-----/---- функ.клав
	DB	0	;Флаг регисра       (0 - нижний)
	DB	2	;Флаг алфавита      (0 - русский)
	DB	0	;Флаг графический   (4-граф)
	DB	0	;Флаг цифр          (10H-цифры)
	DB	0FFH	;Флаг подтв.нажатия (0-нет)
	DB	0	;
	DB	0	;
	DB	0	;
	DB	0	;
;   Таблица адресов дополнительног поля клавиатуры
	DW	0F6A0H	;C0
	DW	0F6A6H	;C1
	DW	0F6ACH	;C2
	DW	0F6B2H	;C3
	DW	0F6B8H	;C4
	DW	0F6BEH	;C5
	DW	0F6C4H	;C6
	DW	0F6CAH	;C7
	DW	0F6D0H	;C8
	DW	0F6D6H	;C9
	DW	0F6DCH	;CCOMMA
	DW	0F6E2H	;CEL
	DW	0F6E8H	;CINS
	DW	0F6EEH	;CCLS
;  Таблица адресов клавиш F1 - F10
	DW	0F600H	;F1
	DW	0F610H	;F2
	DW	0F620H	;F3
	DW	0F630H	;F4
	DW	0F640H	;F5
	DW	0F650H	;F6
	DW	0F660H	;F7
	DW	0F670H	;F8
	DW	0F680H	;F9
	DW	0F690H	;F10
;  Ячейки драйвера накопителя НКМЛ
	DB	61	;Длительность импульса записи 
	DB	68
	DB	27
	DB	33
	DB	0	;Рабочие ячейки
	DB	0	;НКМЛ
	DW	0000H	; Для пользователя
;  ЯЧЕЙКИ ТАБЛИЦИ LUT
	DB	0	;Флаг изменения LUT
	DW	0F771H	;Адрес копии LUT
;       ТАБЛИЦА 'LUT'
	DB	0
	DB	91H
	DB	0A2H
	DB	0B3H
	DB	0C4H
	DB	0D5H
	DB	0E6H
	DB	0F7H
	DB	0F8H
	DB	0F9H
	DB	0FAH
	DB	0FBH
	DB	0FCH
	DB	0FDH
	DB	0FEH
	DB	0FFH
;  ЯЧЕЙКИ ПРИНТЕРА
	DB	0	;Тип интерфейса принтера
	DW	0E318H	;Адрес таблици перекодировки

;*********************************************************************
;*  Процедура чтения и разбора информационного сектора диска
;*	HL - содержит CPMTXX-2, указывает на поле 
;*           маски выбора привода из таблицы параметров диска
;*********************************************************************
GETINFO:
	PUSH	HL	; сохраняем адрес таблицы
	CALL	FLUSH	; Вернуть буфер на диск,если изменился
	POP	HL
	LD	(CPMT),HL ; восстанавливаем адрес таблицы параметров
	PUSH	HL	; и снова прячем адрес в стек
	LD 	A,(HL)	; A= маска выбора привода
	AND	0FH	; макс. 4 дисковода
	LD	(DREG),A
	CP	0	; диск А или В? У них нет маски выбора
	JNZ	GETINFO_FDD  ; нет - читаем сектор с реального дисковода
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
	XOR	A
	LD	(TRKSEK),A
	INC	A
	LD	(SECSEK),A
	LD	HL,CPMT
	CALL	DSETUP
DENOK:
	LD	HL,RRBUFF ;Буфер для чтения с диска
	CALL	DTOM	; Чтение дисковых параметров
	JP Z,CHKDO	; Нрма, на подсчет K.S
	LD	A,(DRVREG) 	; Ошибка, попытка чт-ия с одинар.плотн.
	ADD	A,40H	; Одинарная плотность
	LD	(DRVREG),A
	CALL	FORCE
	LD	HL,RRBUFF
	CALL	DTOM	;Физическое чтение с диска
IERR:
	LD	HL,ERMSG	;Подготовить "ДИСК НЕ ЧИТАЕТСЯ"
	JP NZ,GIERR	; 1 сектор ERROR

;   ПОДСЧЕТ K.S
CHKDO:
	LD	A,(DREG) 
	LD	(DREGWR),A
	XOR	A
	LD	(TRKRDB),A
	INC	A	; HL =1
	LD	(SECBLS),A
	LD	HL,RRBUFF
	LD 	A,66H	; Константа для подсчета K.S
	LD 	B,1FH	; Число байт с 1 сектора
CHKDO1:
	ADD	A,(HL)	    ; calculate checksum
	INC	HL	    ; bump pointer
	DEC	B	    ; decrement counter
	JP NZ,CHKDO1	    ; Loop
	CP	(HL)	   ; Compare with precalculated checksu
	JP Z,CHOK	   ; If match - O.K.
	LD	HL,ERMSG1

;  ВЫВОД 'ОШИБКА'
GIERR:
	POP     DE
	CALL	PSTRNG	; Print error message
	LD 	A,1	; With error condition
	OR	A
	RET
ERMSG:
	DB	ESC,63H,CR,LF
	DB	'* DISK READ ERROR *'
	DB	CR,LF,ESC,63H
	DB	00
ERMSG1:
	DB	ESC,63H,CR,LF
	DB	'* NO SYSTEM INFO *'
	DB	CR,LF,ESC,63H
	DB	00

;  Загрузка считанной информации в табл.CPMTxx
CHOK:
	POP	HL	; HL points to DPBLKXX
	PUSH	HL	; Save HL
;	LD	A,(DRVREG) 
;	AND	0CFH	; (motor or side1)
;	LD 	(HL),A	;            запись маски не нужна - маски намертво вбиты в блоки параметров
	INC	HL
	LD 	(HL),0FFH	;flag out - there is info !
	INC	HL	; cpmtxx  СЕКТОР/ТРЕК
	LD	DE,RRBUFF+16
	LD	BC,15		;+lentrtab   ; info + tabl
	CALL	DETOHL	; Make transfer to DPBLKXX
GIN3:
	POP	DE	; restore pointer
	XOR	A
	DEC	DE
	LD	(DE),A	; mark - we are on track zero now
	DEC	DE	;Байт/сектор (0-128,2-256,3-1024)
	LD	HL,RRBUFF+10
	LD 	A,(HL)	;size sector
	LD	(DE),A	;Обьем физического сектора
	DEC	DE	;Сторон диска (0-одност.1-двухстор.)
	INC	HL
	LD 	A,(HL)	;нумерация секторов
	LD	(DE),A
	LD 	C,A
	INC	C
	LD 	A,80H
GIN4:
	RLCA
	DEC	C
	JP NZ,GIN4
	LD	(DREGRD),A
	DEC	DE
	INC	HL
	LD 	A,(HL)	;Сектор/трек
	LD	(DE),A
	DEC	DE
	DEC	DE
	INC	HL
	INC	HL
	LD 	A,(HL)
	LD	(DE),A
	XOR	A
	RET

LTOF:
	LD	HL,(CPMT)
	LD 	A,(HL)
	LD	(DREG),A	;# драйвера/устройства
	DEC	HL
	DEC	HL
	LD 	B,(HL)	;Размер физического сектора
	LD 	C,0FFH
	LD	A,(SECRDB) 
	DEC	A
	INC	B
	LD 	D,A
	LD 	E,A
LPTS:
	DEC	B
	JP Z,SKIPTS	;Сектор кончился
	OR	A
	LD 	A,E
	RRA	;---->	; Разделить на 2
	LD 	E,A
	LD 	A,C
	ADD	A,A
	LD 	C,A
	JP	LPTS
SKIPTS:
	LD 	A,C
	AND	D
	INC	A
	LD	(OFSSEK),A
	LD 	A,C
	CPL
	INC	A
	LD	(OFSSEK+1),A
	DEC	HL	;Информация о сторонах
	LD 	D,(HL)	;Флаг числа сторон
	OR	A
	DEC	D
	LD	A,(TRKWRB) 
	JP NZ,LE765	;Односторонний
	RRA
LE765:
	LD	(TRKSEK),A	;Физичецкий номер трека
	CALL C,SIDE10	;Если трек нечетный
	DEC	D
	JP NZ,DZIDE
	DEC	HL
	LD 	A,E
	SUB	M
	JP C,DZIDE
	LD 	E,A
	CALL	SIDE10
DZIDE:
	LD 	A,E
	INC	A
	LD	(SECSEK),A	;
	RET
SIDE10:
	LD	A,(DREG) 	;# текущего драйвера
	OR	SIDE1
	LD	(DREG),A
	RET
    
;     ФИЗИЧЕСКОЕ ЧТЕНИЕ ДИСКОВЫХ ПАРАМЕТРОВ
DTOB:
	DB 00
DTOM:
	LD 	A,3	;Кол-во попыток запуска двигателя
DTOM0:
	LD	(DTOB),A
	CALL	DTOM1
	RET Z;Есть включение мотора
	LD	A,(DTOB) 
	DEC	A
	JP NZ,DTOM0
	DEC	A
	RET		;Выход с ERROR
DTOM1:
	CALL	MOTON
	RNZ
	PUSH	HL
	DI
	LD 	A,80H	;Команда 'чтение сектора'
	CALL	DELAY3
	XOR	A
	LD	(SYSREG),A
	LD	BC,0301H
	LD	DE,FDC3
DTOM2:
	LD	A,(DE)	;Читать регистр состояния
	AND	B
	XOR	C
	JP Z,DTOM2
	AND	C
	LD	A,(FDC3+3)	;Читать DATA с диска
	JP NZ,DTOM3
	LD 	(HL),A	;Байт в буфер
	INC	HL
	JP	DTOM2	;За след. байтом
DTOM3:
	EX DE,HL
; 	                HL - buffer address
EXITRW:	
	LD 	A,1CH
	LD	(SYSR1),A   ;registr konfiguracii
	POP	HL
	LD	A,(FDC) 
	AND	0DDH	;Маска ошибок, учитываем READY
	EI
	LD     (ERRFL),A	;код ошибки
	RET

;   ФИЗИЧЕСКАЯ ЗАПИСЬ НА ДИСК
;                  HL - buffer address
MTOD:
	CALL	MOTON
	RNZ
MTODR:
	DI		;выключаем на время операции
	LD 	A,WRSECCMD
	CALL	DELAY3
	PUSH	HL	; save buffer pointer
	EX DE,HL
	LD	HL,0
	ADD HL,SP
	LD	(STACKS),HL	; спасаем 
	EX DE,HL
	LD SP,HL		; stack - buffer
	XOR	A
	LD	(SYSREG),A
	LD	HL,FDC3+3
	LD	DE,FDC3
MTOD1:
	POP	BC	; Данные из буфера
MTOD2:
	LD	A,(DE)	; get status
	XOR	1	; make test for busy and drq bits
	JP Z,MTOD2	; if not done - wait
	LD 	(HL),C	; DATA на диск
	RRA
	JP C,WDONE	; go if not busy
	RRA
	JP NC,MTOD2	; continue when busy still
MTOD3:
	LD	A,(DE)	; get status
	XOR	1	; make test for busy and drq bits
	JP Z,MTOD3	; if not done - wait
	LD 	(HL),B	; issue 2nd byte of data
	JP	MTOD1	; loop until all'll be done
WDONE:
	LD	HL,0FFFEH
	ADD 	HL,SP
	EX 	DE,HL
	LD	HL,(STACKS)
	LD 	SP,HL		; restore stack
	JP	EXITRW
MOTON:
	LD	DE,DRVREG
	LD 	A,0AH	; КУ3 - чтобы читать регистр запросов
	LD	(INT),A
	LD     A,(INT) 
	AND	80H	; для мотора
	PUSH	AF
	LD	A,(DE)
	AND	NOT MOTOR
	LD	(DE),A
	OR	MOTOR
	LD	(DE),A	;перезапустили мотор
	AND     NOT MOTOR
	LD    (DE),A
	POP	AF
	CALL NZ,DELAY	;если мотор был выключен
	LD      C,10	; <=> 2 сек.
LP0000:
	LD	A,(DE)
	AND	0DFH
	LD	(DE),A
	OR	20H	;ВКЛ.двигатель
	LD	(DE),A
	AND	0DFH	;Снять импульс
	LD	(DE),A
	LD     A,(FDC) 
	RLCA
	JP NC,READYOK		;ждем READY
	DEC     C
	JP Z,NREADY		;ошибка: NOT READY
	CALL    DELAY
	JP     LP0000
READYOK: 
	LD	DE,FDC		; SET FDC & FORCE
	JP	FORCE
NREADY:
	LD      A,DNOTR
	LD     (ERRFL),A
	OR     A
	RET
DELAY:
	LD      B,0
DELAY1:
	CALL	DELAY0
	DEC	B
	JP NZ,DELAY1
	RET
FORCE:
	LD 	A,FORCECMD
DELAY3:
	LD	(FDC),A
	LD 	A,0FH	
DELAY0:
	DEC	A
	JP NZ,DELAY0
	RET

;==================  Драйвер ExtROM-API ====================================
	
PORTA	EQU	0FB08H		; Порт Канала A интерфейса расширения
PORTC	EQU	0FB0AH		; Порт Канала С интерфейса расширения
PCWR	EQU	0FB0BH		; Порт управляющих команд
	
;****************************************
;*  Прием байта из порта А по стробу    *
;****************************************
EXR_GETBYTE:
	PUSH	HL
	LD	HL,PORTC
WG:
	LD	A,(HL)		; слово состояния ВВ55 - берется из порта С
	AND	20h		; выделяем сигнал IBF
	JP	Z,WG		; IBF=0 - данных еще нет
	LD	A,(PORTA)		; данные поступили - выбираем их из порта А
	POP	HL
	RET

;****************************************
;*  Отправка байта в порт A             *
;****************************************
EXR_PUTBYTE:
	PUSH	HL
	PUSH	AF
	LD	HL,PORTC
WP:
	LD	A,(HL)		; слово состояния ВВ55 - берется из порта С
	AND	80h		; выделяем сигнал -OBF
	JP	Z,WP		; -OBF=0 - в передатчике сидит незабранный байт
	POP	AF
	LD	(PORTA),A		; данные поступили - выбираем их из порта А
	POP	HL
	RET

;*******************************************
;*      Прием сектора                      *
;* HL - адрес размещения сектора в памяти  *
;*  Размер сектора - 128 байт              *
;*******************************************
EXR_GETSEC:
	PUSH	BC
	PUSH	DE
	LD	C,128		; счетчик байтов сектора, всего 128 байт
	EX	DE,HL		; адрес буфера -> DE
	LD	HL,PORTC
GSL:
	LD	A,(HL)		; слово состояния ВВ55 - берется из порта С
	AND	20h		; выделяем сигнал IBF
	JP	Z,GSL		; IBF=0 - данных еще нет
	LD	A,(PORTA)		; данные поступили - выбираем их из порта А
	LD	(DE),A		; принимаем и размещаем очередной байт
	INC	DE		; указатель буфера ++
	DEC	C		; принимаем остальные байты
	JP	NZ,GSL
	POP	DE
	POP	BC
	RET

;*******************************************
;*      Передача сектора                   *
;* HL - адрес размещения сектора в памяти  *
;*  Размер сектора - 128 байт              *
;*******************************************
EXR_PUTSEC:
	PUSH	BC
	PUSH	DE
	LD	C,128		; счетчик байтов сектора, всего 128 байт
	EX	DE,HL		; адрес буфера -> DE
	LD	HL,PORTC
PSL:
	LD	A,(HL)		; слово состояния ВВ55 - берется из порта С
	AND	80h		; выделяем сигнал -OBF
	JP	Z,PSL		; -OBF=0 - в передатчике сидит незабранный байт
	LD	A,(DE)		; принимаем и размещаем очередной байт
	LD	(PORTA),A		; данные поступили - выбираем их из порта А
	INC	DE
	DEC	C		; принимаем остальные байты
	JP	NZ,PSL
	POP	DE
	POP	BC
	RET

;****************************************
;*     Отправка командного пакета       *
;****************************************
EXR_SENDCMD:

	PUSH	HL
	PUSH	BC
	LD	A,0Ch
	LD	(PCWR),A	; переключаем порт в режим 2
	LD	HL,EXR_CMD
	LD	C,4		; пакет - 4 байта
	LD	B,0		; заготовка контрольной суммы
SCL:
	LD	A,(HL)		; очередной байт пакета 
	ADD 	B		; добавляем к КС
	LD	B,A
	LD	A,(HL)		; очередной байт пакета 
	CALL	EXR_PUTBYTE		; - в порт
	INC	HL
	DEC	C
	JP	NZ,SCL
	LD	A,B
	DEC 	A		; КС-1
	CALL	EXR_PUTBYTE     ; контрольная сумма
	CALL	EXR_GETBYTE	; ответ контроллера
	POP	BC
	POP	HL
	RET

;*******************************************
;*  Чтение информационного сектора
;*******************************************
EXR_READINFO:
	LD	HL,EXR_CMD	; командный пакет
	LD	(HL),1		; команда чтения
	INC	HL
	LD	A,(DSKCPM)	; вписываем # устройства
	LD	(HL),A
	INC	HL
	LD	(HL),0		; обнуляем, поскольку читаем сектор 0 дорожки 0 
	INC	HL
	LD	(HL),0		
	CALL	EXR_SENDCMD	; отправляем команду
	OR	A
	RET 	Z		; ошибка
	LD	HL,RRBUFF	; принимаем данные
	CALL	EXR_GETSEC
	LD	A,1
	RET


; Командный пакет интерфейса Extrom-API
;===============================================
EXR_CMD:	DB	0	; Команда чтения(0)-записи(1)
EXR_DRV:	DB	0	; Устройство - A(0), B(1)	
EXR_TRK:	DB	0	; логическая дорожка
EXR_SEC:	DB	0	; логический сектор (128b)

	ORG	MEMLOW+27FFh
	DB	0

	END
