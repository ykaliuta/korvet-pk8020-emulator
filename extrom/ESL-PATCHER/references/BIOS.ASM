	
	PAGE	64

;	CBIOS --  КОРВЕТ ПК 8020.
;	*************************
MSIZE	EQU 56		;Размер памяти для CP/M
BIAS	EQU (MSIZE-20)*1024
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
RDBUFF	EQU WRBUFF-1024 ; Буффер чтения
EBUF 	EQU RDBUFF-128	; E-диск буфер

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
		PAGE
;;       .PHASE	 0DA00H
	ORG	0DA00H

START:
; Таблица переходов BIOS
 JMP	 BOOT    ;"холодный" старт
WBOOTE:
 JMP	 WBOOT    ;"теплый" старт
 JMP	 CONST    ;статус консоли
 JMP	 CONIN    ;ввод символа с консоли в (А)
 JMP	 CONOUT   ;вывод символа на консоль из (А)
 JMP	 LIST     ;вывод символа на принтер
 JMP	 PUNCH    ;вывод символа на посл.интерфейс
 JMP	 READER   ;прием символа с посл.интерфейса
 JMP	 HOME     ;установка на 0 дорожку
 JMP	 SELDSK   ;выбор диска
 JMP	 SETTRK   ;установка #трека
 JMP	 SETSEC   ;установка #сектора
 JMP	 SETDMA   ;установка вектора DMA
 JMP	 READ     ;операция чтения
 JMP	 WRITE    ;операция записи
 JMP	 LISTST   ;готовность принтера
 JMP	 SECTRAN  ;пересчет логического номера в физ.

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

;******************************
;  ФИЗИЧЕСКИЕ ПАРАМЕТРЫ ДИСКА

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
DB 1	; позиционный номер устройства (00000100)
;________________________________________________

;================================================
;     ДИСК - A

DPBK00:
DB 0	;INFO	; флаг успешного считывания
;	; информационного сектора
;=====================================================
	; начало таблицы логических параметров диска.
;          CP/M parameters
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
DB 2	; DREG1

;=======================================
;    ДИСК - B

DPBK01:
 DB 0  ; INFO
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
 DB 4   ; DREG2

;===============================
;    ДИСК - C
DPBK02:
 DB 0	; INFO

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
DB 8    ;   DREG3
;================================
;  ДИСК - D

DPBK03:
 DB 0  ; INFO
;          CP/M parameters

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
		PAGE 
SISNON:		;DAEE 
	DB	1FH,0DH,0AH,'CP/M-80  vers. '
	DB	'2.2 ',0DH,0AH,'BIOS  vers.2.1 '
	DB	'(c) ',0DH,0AH,'МГУ      '
	DB	'          ',0DH,0AH,'   '
	DB	'МОСКВА 1989 ',0DH,0AH,0
BOOT:
	 DI		; Запрет прерываний
	 LXI	 SP,80H	; Setup stack space
	 MVI	 A,1CH	; системный регистр
	 STA	 SYSREG	; установили (на всякий случай)
	 STA	 SYSCOPY
	 CALL	 INIT
	 CALL	 HINIT	;Иници-ия раб.ячеек,табл.клав-ры
	 XRA	 A
	 STA	 CDISK	; активный диск А: & user 0
	 LXI	 H,SISNON
	 CALL	 PSTRNG	; печать начального сообщения
WBOOT:
	 DI
	 LXI	 SP,80H	; стек ниже буфера 
	 CALL	 INIT
	 MVI	 C,0	;выбор диска 0
	 CALL	 SELDSK	;Чтение инфор-ого сектора(систем.инф)
	 CALL	 HOME	;трек 00
	 MVI	 B,NSECTS ;число секторов для загрузки
	 MVI	 C,0	;# текущего трека
	 MVI	 D,2	;# текущего сектора
			; сектор 1 содержит спец. информацию
	 LXI	 H,CCP	;начальный адрес загрузки
			; загружаем  1 сектор
LOAD1:
	 PUSH	 B	;save sector count, current track
	 PUSH	 D	;save next sector to read
	 PUSH	 H	;save dma address
	 MOV	 C,D	;# тек.сектора в reg C
	 MVI	 B,00	; add MSB
	 CALL	 SETSEC	;Установка # сектора
	 POP	 B	;recall dma address to b,c
	 PUSH	 B	;replace on stack for later recall
	 CALL	 SETDMA	;Установ.адрес DMA
	 CALL	 READ	;Чтение
	 ORA	 A	; any errors?
	 JNZ	 WBOOT	;retry boot if an error occurs
	 POP	 H	;recall dma address
	 LXI	 D,128	;dma=dma+128
	 DAD	 D	;new dma address is in h,l
	 POP	 D	;recall sector address
	 POP	 B	;recall number of sectors remaining
			; and current trk
	 DCR	 B	;sectors=sectors-1
	 JZ	 GOCPM	;загружены все секторы на запуск CP/M
	 INR	 D
	 LDA	 CPMT00	;Читать макс. число секторов в треке
	 CMP	 D	;сравнить с максимумом
	 JNC	 LOAD1	;carry generated if sector > maximum
	 MVI	 D,1	;счетчик номера сектора уст. в 1
	 INR	 C	;нарастить номер трека
	 PUSH	 B
	 PUSH	 D
	 PUSH	 H
	 MVI	 B,00	; add MSB
	 CALL	 SETTRK	;Установить след.трек
	 POP	 H
	 POP	 D
	 POP	 B
	 JMP	 LOAD1	;подолжить чтение

;Запуск CP/M
GOCPM:
	 MVI	 A,JMP	;c3 is a jmp instruction
	 STA	 0	;for jmp to wboot
	 LXI	 H,WBOOTE ;wboot entry point
	 SHLD	 1	;set address field for jmp at 0
	 STA	 5	;for jmp to bdos
	 LXI	 H,BDOS	;bdos entry point
	 SHLD	 6	;address field of jump at 5 to bdos
	 LXI	 B,80H	;default dma address is 80h
	 CALL	 SETDMA
	 LDA	 CDISK	;get current disk number
	 MOV	 C,A	;send to the ccp
	 JMP	 CCP	;go to cp/m for further processing

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
	 PUSH	 H
	 LHLD	 DOINT0
	 JMP	 DOINT
INT1:
	 PUSH	 H
	 LHLD	 DOINT1
	 JMP	 DOINT
INT2:
	 PUSH	 H
	 LHLD	 DOINT2
	 JMP	 DOINT
INT3:
	 PUSH	 H
	 LHLD	 DOINT3
	 JMP	 DOINT
INT4:
	 PUSH	 H
	 LHLD	 DOINT4
	 JMP	 DOINT
INT5:
	 PUSH	 H
	 LHLD	 DOINT5
	 JMP	 DOINT
INT6:
	 PUSH	 H
	 LHLD	 DOINT6
	 JMP	 DOINT
INT7:
	 PUSH	 H
	 LHLD	 DOINT7
DOINT:
	 PUSH	 PSW
	 MVI	 A,1CH	;Конфигурация, принятая в CPM
	 STA	 SYSREG
	 PCHL		;вложенные прерывания  не откраваем
OUTINT:		;Выход из 4 ур.прерывания
	 LDA	 SYSCOPY
	 STA	 SYSREG
	 DI
	 MVI	 A,20H	;зрет прерывания
	 STA	 INT	;уровня 5 (Таймер)
	 POP	 PSW
	 POP	 H
	 EI
	 RET
;Прерывание 4 уровня ( Системное прерывание )
SYST:
	 LXI	 H,0
	 DAD	 SP
	 LXI	 SP,HOME
	 PUSH	 H	;спасли стек
	 PUSH	 D
	 PUSH	 B
;функция LUT - Обработка драйвера просмотровой таблици
	 LDA	 LUTFL
	 ORA	 A
	 JZ	 KBD	;0-Обработка не нужна,на клав-ру
	 CALL	 LUT
	 XRA	 A
	 STA	 LUTFL
KBD:	;Обработка прерываний Клавиатуры
	 LDA	 SYMBUF
	 ORA	  A
	 JNZ	 SKIP1	;прыгаем, если символ в буфере
;			;Выход из прерываний
SKIP5:	; Опрос клавиатуры
	 CALL	 INKEY	;<B> содержит контрольную линейку
	 ORA	 A	;<C> содержит скан-код
	 JZ	 SKIP1	;прыгаем, если кнопка не нажата
	 LHLD	 DOINT8	;прерывание от клавиатуры
	 PCHL		;программное прерывание (SKIP3)
SKIP3:
	 MOV	 A,C	;Скан-код в A
	 ORA	 A
	 JZ	 SKIP1	;контроль:Нет кода
	 STA	 SYMBUF	;Код есть,сохранить
	 MOV	 A,B	;Код Модификации в A
	 STA	 CNTRKEY;Сохранить
	 LDA	 SNDFLG	;Флаг выдачи звука
	 ORA	 A
	 JZ	 SKIP1	;0- Нет,без звука
	 CALL	 SNDON	;настроили TIMER
	 MVI	 A,01H
	 STA	 STMK0	;щелчок
SKIP1:
	 POP	 B
	 POP	 D
	 POP	 H	;Вернуть стек
	 SPHL
	 JMP	 OUTINT
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
	 MVI	 C,0	;трек по регистру (C)
SETTRK:
	 MOV	 A,C
	 STA	 TRKCPM
	 RET

	; Выбор диска по (С), (HL) содержит адрес
	;таблицы (на выходе)
SELDSK:
	 MOV	 A,C
	 CPI	 255	;запрос адреса таблицы векторов
	 LXI	 H,DOINT0
	 RZ
	 CPI	 DRNUM	; не более DRNUM
	 JNC	 DSERR	; > DROUN
; определяем адрес таблицы
	 LXI	 H,DRVTAB
	 STA	 DSKCPM	;Номер диска
	 MVI	 B,0
	 DAD	 B
	 DAD	 B	;адрес таблицы + 2*dskcpm
	 MOV	 A,M
	 INX	 H
	 MOV	 H,M
	 MOV	 L,A	;NL = DRH
	 ORA	 H
	 JZ	 DSERR	; > 5  (HL = 00 )
	 MOV	 A,C
	 CPI	 4
	 RNC		;не floppy > 5
	 PUSH	 H	;адрес таблицы DPH сохранить
	 LXI	 B,10
	 DAD	 B	;Адрес вектора таблици
	 MOV	 A,M
	 INX	 H
	 MOV	 H,M
	 MOV	 L,A	;CPMTXX
	 DCX	 H
	 MOV	 A,M	;INFO
	 DCX	 H
	 SHLD	 CPMT	;текущий драйв
	 INR	 A
	 CNZ	 GETINFO
	 POP	 H
	 RZ
DSERR:
	 LXI	 H,0000	; если ошибка, вернем 0
	 XRA	 A
	 STA	 CDISK	; если ошибка, драйв А:
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
;============================================
;;XGETINF:
;;	DB	0	;информация для опр.формата диска B
;;            ДЛЯ BIOS 56кб
;============================================


	; сектор по регистру (С)
SETSEC:
	 MOV	 A,C
	 STA	 SECCPM
	 RET
;  Перевод BC --> HL и нарастить HL

SECTRAN:
	MOV	H,B	; Make no translation
	MOV	L,C
	INX	H	; физический  номер начинается с 1!
	RET
	;DMA устанавливается по регистру (BC)
SETDMA:
	MOV	L,C
	MOV	H,B
	SHLD	DMACPM
	RET
;	Чтение сектора
READ:
	MVI	 A,4	;Режим чтения
	STA 	OPER
	LDA 	DSKCPM
	CPI	 4
	JZ  	EDISK	;Больше 4 E-диск
	CC	MOTON	;Флоппи-диски
	LDA	ERRFL	;Читать флаг ошибки
	ORA	A
	JNZ	READ1
READ0:
	LXI	H,DREGWR ;HL = адрес хр-ия # тек. драйвера
	CALL	ETR1
	JZ	READ2
READ1:
	LHLD	CPMT	; Адресс табл. параметров
	PUSH	H
	CALL	FLUSH	;Проверка смены диска
	POP	H
	SHLD	CPMT
	CALL	READ3
	JMP	READ0
READ2:
	LXI	H,RDBUFF
	RAR
	MOV	B,A
	MVI	A,0
	RAR
	MOV	C,A
	DAD	B
	XCHG
	LHLD	DMACPM
	MVI	A,40H
	CALL	DET1
	EI
	LDA	ERRFL ;
	ORA	A
	RET
READ3:
	LDA	TRKCPM
	STA	TRKRDB
	STA	TRKWRB
	LDA	SECCPM
	STA	SECRDB
	LDA	DSKCPM
	STA	DREGWR
	CALL	LTOF
	LDA	OFSSEK
	STA	SECBLS
	LXI	H,R5+1
	MVI	M,10H	;Перезагрузить байт для ( CPI )
	MVI	A,0
	STA	DREGRD	;Н.А. буфера чтения
	LXI	H,RDBUFF
	SHLD	REA1
READ4:
	LXI	H,CPMT
	CALL	DSETUP
READ5:
	LHLD	REA1
	CALL	DTOM
	RNZ		;Выход "ошибка чтения"
	XCHG
	SHLD	REA1	;След.адр.буфера чтения
	LXI	H,DREGRD
	LDA	OFSSEK+1
	ADD	M
	MOV	M,A
R5:
	CPI	10H	;Перезагружаемый байт ( число логическ.
;			;секторов в блоке)
	RP
	LDA	REA2
	CPI	0F6H	;Конец ?
	JC	READ6	;Нет
	MVI	A,0FFH
	STA	ERRFL
	RET
 READ6:
	LXI	H,SECSEK
	MOV	A,M
	INR	M
	LHLD	CPMT
	LXI	D,0FFFCH ; Дополнительный код ( 0004 )
	DAD	D
	CMP	M
	JP	READ7	;Конец трека, на следующий
	INR	A
	STA	FDC+2
	JMP	READ5
READ7:
	LDA	TRKWRB
	INR	A	; Нарастить # трека
	STA	TRKWRB
	MVI	A,1	; Номер сектора уст.1
	STA	SECRDB
	CALL	LTOF
	LHLD	CPMT
	LXI	D,0FFFAH ; Дополнительный код ( 0003 )
	DAD	D
	LDA	TRKSEK
	CMP	M
	RP
	JMP	READ4
REA1:
	DB 0	; Хранение адреса
REA2:		; Буфера RDBUF
	DB 0	; ( чтения )
EDISK:
	 LDA	 TRKCPM
	 ADD	 A
	 CPI	 17
	 MOV	 E,A
	 MVI	 A,1
	 RNC		;RETURN, если трек большой
	 DI
	 MVI	 D,0
	 LXI	 H,ETRAK
	 DAD	 D
	 LDA	 VIDEO
	 STA	 STORE
	 ANI	 3FH
	 ORA	 M
	 STA	 VIDEO	;Нужная страница
	 INX	 H
	 MOV	 A,M
	 STA	 STORE+1;Нужная маска цвета
	 LDA	 SECCPM
	 RRC
	 MOV	 E,A
	 ANI	 3FH
	 ORI	 40H
	 MOV	 D,A	;Старщий байт адреса
	 MVI	 A,80H
	 ANA	 E
	 MOV	 E,A	;Младший байт адреса
	 MVI	 B,80H	;Счетчик
	 LXI	 H,EBUF
	 LDA	 OPER
	 CPI	 6
	 CZ	 EWRITE
	 LDA	 STORE+1
	 INR	 A
	 STA	 CLREG
	 MVI	 A,3CH
	 STA	 SYSREG	;Конфигурация CP/M-80
LPEDISK:
	 LDAX	 D	;Пересылка EBUFF --> E - диск при зап.
	 MOV	 M,A	;Пересылка EBUFF <-- E - диск при чтен.
	 INX	 D
	 INX	 H
	 DCR	 B
	 JNZ	 LPEDISK
	 MVI	 A,1CH	;Конфигурация 3(графика)
	 STA	 SYSR3	;
	 LDA	 STORE
	 STA	 VIDEO
	 LDA	 OPER
	 CPI	 4
	 JZ	 EREAD
 LPED1:
	 LDA	 COLCOPY  ;0F704H
	 STA	 CLREG
	 EI
	 XRA	 A
	 RET
EREAD:
	 LHLD	 DMACPM
	 LXI	 D,EBUF
	 MVI	 A,40H
	 CALL	 DET1	;Пересылка из DMA  в EBUFF 64 слова
	 JMP	 LPED1
EWRITE:
	 PUSH	 D
	 XCHG
	 LHLD	 DMACPM
	 XCHG
	 MVI	 A,40H
	 CALL	 DET1
	 POP	 H
	 PUSH	 H
	 LDA	 STORE+1
	 STA	 CLREG
	 MVI	 A,3CH
	 STA	 SYSREG
	 MVI	 A,0FFH
	 MVI	 B,80H
LPECLR:
	 MOV	 M,A	;Заполнение сектора "FF"
	 INX	 H
	 DCR	 B
	 JNZ	 LPECLR 
	 MVI	 A,1CH
	 STA	 SYSR3	;
	 POP	 H
	 LXI	 D,EBUF
	 MVI	 B,80H
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
	LDA	DSKCPM	;# дисковода для BDOS
	CMP	M	;Сравнить с тек.# драйвера
	RNZ		; # драйвера не соотв.# диска
	INX	H
	LDA	TRKCPM	;Читать # трека
	SUB	M
	RC		;Тек.# трека > максимального
	INX	H
	MOV	E,M
	INX	H
	MOV	D,M
	LHLD	CPMT	;HL=CPMTх - 2
	INX	H
	INX	H	;HL= (SPT)
	JZ	ETR3	;Тек.# трека = макс.треку
	DCR	A	;-----/----  < ----/----
	RNZ
	LDA	SECCPM
	ADD	M
ETR2:
	SUB	E
	MOV	E,A
	MOV	A,D
	DCR	A
	CMP	E
	RC
	XRA	A
	MOV	A,E
	RET
ETR3:
	LDA	SECCPM
	CMP	E
	RC		;Тек.сектор < макс.
	JMP	ETR2

	; Операция записи
WRITE:
	MVI	A,6	;Режим записи
	STA	OPER
	LDA	DSKCPM
	CPI	4
	JZ	EDISK	;RAM-диск
	MOV	A,C
	STA	WRTYPE	; тип записи
	CC	MOTON
WRI1:
	LXI	H,DREGWR
	CALL	ETR1
	JZ	WRI3
	LHLD	CPMT
	PUSH	H
	CALL	FLUSH
	POP	H
	SHLD	CPMTWR
	SHLD	CPMT
	LDA	WRTYPE
	CPI	2
	JZ	WRI2
	CALL	READ3
	JMP	WRI1
WRI2:
	LDA	DSKCPM
	STA	DREGWR
	LHLD	TRKCPM
	SHLD	TRKRDB
	LHLD	CPMT
	LXI	D,0005
	DAD	D
	MOV	A,M
	INR	A
	STA	DREGRD
	XRA	A
	STA	ERRFL
	MOV	E,A
WRI3:
	CALL	WRI4
	LDA	WRTYPE
	DCR	A
	CZ	FLUSH
	LDA	ERRFL
	ORA	A
	RET
WRI4:
	RAR
	MOV	B,A
	MVI	A,0
	RAR
	MOV	C,A
	LXI	H,RDBUFF
	DAD	B
	XCHG
	MOV	A,D
	CPI	0F6H
	JC	WRI5
	MVI	A,0FFH
	STA	ERRFL
	RET
WRI5:
	LDA	DREGRD
	DCR	A
	CMP	L
	JNZ	WRI6
	MVI	A,1
	STA	WRTYPE
WRI6:
	MVI	A,0FFH
	STA	BUFFAC
	LHLD	CPMT
	SHLD	CPMTWR
	LHLD	DMACPM
	XCHG
	MVI	A,40H
	CALL	DET1
	EI
	RET

;запись буфера на диск ( если буфер изменился )
FLUSH:
	LDA	BUFFAC	;Диск менялся ?
	ORA	A
	RZ		; НЕТ
	XRA	A
 	STA	BUFFAC	; Чистить флаг смены диска
	LHLD	CPMTWR	; Адр.табл.нового диска
	SHLD	CPMT	; изменить
	LHLD	TRKRDB	;
	SHLD	TRKWRB	; Обновить номер трека
	CALL	LTOF	; Контроль
	MVI	A,0
	STA	NSEC	; Сектор 0
	LXI	H,RDBUFF; Буфер чтения
	SHLD	REA1	; Сохранить тек.адресс буфера
	MVI	A,164
	STA	MTODR+2	;Изменение для MTOD
FLUS0:
	LXI	H,CPMT
	CALL	DSETUP	;Установить головку
FLUS1:
	LHLD	REA1
	CALL	MTOD	;Чтение с диска
	XCHG
	SHLD	REA1
	MVI	A,160
	STA	MTODR+2 ;Увеличить адресс RDBUFF
	LXI	H,NSEC
	LDA	OFSSEK+1; Чтен.смещение сектора
	ADD	M
	MOV	M,A
	LDA	DREGRD	; Количество сторон диска
	CMP	M
	RZ
	RM
	LDA	REA2
	CPI	0F6H	;Конец буфера ?
	JC	FLUS2	;нет,продолжить чтение
	MVI	A,0FFH
	STA	ERRFL	;Флаг ошибки
	RET
FLUS2:
	LXI	H,SECSEK  ;Тек.# сектора
	MOV	A,M
	INR	M	;Следующий # сектора
	LHLD	CPMT	;Адрес драйвера
	LXI	D,0FFFCH ;Обр.код (0002)
	DAD	D	;минус 2
	CMP	M	; сектор/трек
	JP	FLUS4	; Трек весь
	INR	A	; След.сектор
	STA	FDC+2	; # тек.сектора
	JMP	FLUS1	; Продолжить чтение
;  Переход на следующий трек
FLUS4:
	LDA	TRKWRB
	INR	A
	STA	TRKWRB	;След.тек.трек
	MVI 	A,1
	STA	SECRDB	;Сектор = 1 (начало трека)
	CALL	LTOF	;Определить сторону диска
	JMP	FLUS0
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
	MOV	E,M
	INX	H
	MOV	D,M	;Адрес CPMTxx В DE
	INX	H	; # тек.дисковода
	CALL	FORCE	; Сбросить контроллер дисковода
	MOV	A,M	; # тек.дисковода
	STA	DRVREG
	PUSH	D
	CALL	MOTON	;Вкл. мотор
	POP	D
	DCX	D	; DE points to current track now
	INX	H
	MOV	A,M
	ORA	A
	MOV	C,A
	INX	H
	MOV	A,M
	STA	FDC+2	;Рег. сектора
	JZ	SK4
	LDAX	D	;Читать # тек.трека
	STA	FDC+1
	CMP	C
	RZ
	MOV	A,C	;След.трек
	STA	FDC+3
	MVI	C,18H	;Команда "ПОИСК"
	JNZ	DZET1
SK4:
	MOV	A,C
	MVI	C,8	;Команда "_______________"
DZET1:
	STAX	D
	DCX	D
	DCX	D
	DCX	D
	DCX	D	;сие указывает на скорость головки
	LDAX	D	;читать константу скорости(=80H)
	ANI	0C0H	; D7-5"/8"....D6-плотность(0-двойн)
	MOV	B,A
	LDA	DRVREG
	PUSH	PSW
	ORA	B
	STA	DRVREG
	LDAX	D
	ANI	3
	ORA	C
	CALL	DELAY3	;Задержка
WAITB2:
	LDA	FDC
	RRC
	JC	WAITB2	; If not DONE yet - wait
	POP	PSW	;Есть готовность
	STA	DRVREG
	MVI	B,0FH
	CALL	DELAY1	;Задержка 30 мск
	JMP	FORCE

;Вывод сообщения на экран
PSTRNG:
	MOV	A,M	; Print message pointed by HL to 0
	ORA	A	; Zero ?
	RZ
	PUSH	H	; Save HL
	MOV	C,A
	CALL	CONOUT
	POP	H	; Restore HL
	INX	H	; Bump pointer
	JMP	PSTRNG

	;Загрузка массива из M (DE) в M (HL)
	; BC = Размер массива
DETOHL:
	MOV	A,B
	ORA	C
	RZ
	LDAX	D
	MOV	M,A
	INX	H
	INX	D
	DCX	B
	JMP	DETOHL
DET1:
	ORA	A
	RZ
	PUSH	H
	LXI	H,0002
	DAD	SP
	SHLD	STACKS
	XCHG
	POP	D
	DI
	SPHL
	XCHG
DET2:
	POP	D
	MOV	M,E
	INX	H
	MOV	M,D
	INX	H
	DCR	A
	JNZ	DET2
	LHLD	STACKS
	SPHL		;Вернуть стек
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
CPMT: 	DW	CPMT00;Адрес таблици тек.драйвера
DREG: 	DB	1	; # тек.драйвера
TRKSEK: DB	0	; # тек.  трека (физического)
SECSEK: DB	0	; # тек.сектора (физического)
OFSSEK: DW	0000H	; смещение сектора в буфере
TRKWRB: DB	1	;

  ; ЛОГИЧЕСКИЕ ПАРАМЕТРЫ
SECRDB:	DB	1	;
CPMTWR: DW	CPMT00 ;
DREGWR: DB	0	;
TRKRDB: DB	0	;
SECBLS: DB	0	;
DREGRD: DB	0	; 01 - одност-ний 02 - двухст-ний
;=================================================
;	Драйвер дисплея 
; C содержит код символа
CONOUT:
	MOV	A,C
	CPI	7
	JZ	BEEP
	LXI	H,0
	DAD	SP
	LXI	SP,ENDEBUF  ;буфер электронного диска!
	PUSH	H
	MVI	A,14H
	DI
	STA	SYSCOPY	;сохраняем копию
	STA	SYSREG
	EI
	CALL    CON	;печатаем
	MVI     A,1CH
	DI
	STA     SYSREG
	STA     SYSCOPY	;сохраняем копию
	EI
	POP	H
	SPHL
	RET
BEEP:
	MVI	A,36H
	STA	STMRM
	LHLD	BELLDIV
	LXI	D,0FB00H
	XCHG
	MOV	M,L
	MOV	M,D
	MVI	L,32H
	MOV	B,M
	MVI	A,8
	ORA	B
	MOV	M,A	;включили звук
	XCHG
	LHLD	BELLDEL
LPBEEP:	
	DCX	H
	MOV	A,L
	ORA	H
	JNZ	LPBEEP
	XCHG
	MOV	M,B
	LDA	SNDFLG
	ORA	A
	RZ
SNDON:
	MVI	A,20H
	STA	STMRM	;рег.режима
	MVI	A,7
	STA	LSTCTL
	RET
;===========================================
;ДРАЙВЕР КЛАВИАТУРЫ (Сканирование клавиатуры)
INKEY:
	LDA	KEYB0		;Опрос основного поля
	ORA	A
	JNZ	INKEY5		; есть
	LDA	DOPK11	; опрос доп.поля и функц.клавиш
	ORA	A
	JNZ	INKEY4
	LDA	MODKB		; поле модификаторов
	ORA	A
	JNZ	INK2
INK1:
	XRA	A
	STA	KMODIF	; Нет, очистить яч-ку KMODIF
	RET
   ;НАЖАТИЕ ПОЛЕ МОДИФИКАЦИИ
INK2:
	MOV	B,A
	ANI	01001000B  ;Маска клавиш ESC , ФКС
	JZ	INK1	; нет
	MVI	C,0	; Да,подготовить C для # кода KMOD
	CALL	L1INK	; Преобразовать в двоично/десятичный
	MVI	A,56	; Конец # в строке KMOD
	ADD	C	; Минус полученный номер
	MOV	C,A
	CPI	59	; Клавиша 'ESC' ?
	JZ	INKEY6	; Да
	LXI	H,KMODIF;
	SUB	M	; ФКС была раньше ?
	RZ		; Да,выйдем
	MOV	M,C	; Нет, обновим ячейку
	LXI	H,FSHIFT;
	MVI	A,10000001B  ; Маска клав.РГ (прав. и лев.)
	ANA	B	;
	JZ	INKEY3	; Нет нажатия РГ 
	MVI	A,0FFH	;
	XRA	M	;
	MOV	M,A	;
INKEY3:
	INX	H
	MVI	A,00000010B  ;Маска клав.АЛФ
	ANA	B
	XRA	M
	MOV	M,A
	MVI	A,00000100B  ;Маска клав.ГРАФ
	INX	H
	ANA	B
	XRA	M
	MOV	M,A
	INX	H
	MVI	A,00010000B  ;Маска клав.ЦИФР
	ANA	B
	XRA	M
	MOV	M,A
	XRA	A
	RET		; Выход = 00

   ; ПОЛЕ F1 -- F10
INKEY4:
	LXI	H,KB11
	CALL	INKEY10
	RZ
	MVI	A,40H
	ADD	C
	MOV	C,A
	JMP	INKEY6

   ;ОСНОВНОЕ ПОЛЕ
INKEY5:
	LXI	H,KB06
	CALL	INKEY10
	RZ

   ; КЛАВИША 'ESC'
INKEY6:
	LXI	D,KMODIF
	LXI	H,COUNT
	LDAX	D
	SUB	C
	JZ	INKEY8
	LDA	LONGVAL

   ; ОБРАБОТКА ДРЕБЕЗГА КЛАВИШ
INKEY7:
	MOV	M,A
	LDA	MODKB
	STA	CNTRKEY
	MOV	B,A
	MOV	A,C
	STAX	D
	ORA	A
	RET
INKEY8:
	LDA	AUTOVAL
	DCR	M
	JZ	INKEY7
	XRA	A
	RET

   ; СДВИГ БИТА ЛИНЕЙКИ
INKEY9:
	MOV	A,L
	RAR
	MOV	L,A	;Следующая линейка
	ORA	A
	RZ		; Все линейки пройдены
INKEY10:
	MOV	A,M
	ORA	A
	JZ	INKEY9
	MVI	C,0
	PUSH	PSW	; Сохранить байт линейки
	MOV	A,L
	CALL	L1INK
	MOV	A,C
	ADD	A
	ADD	A
	ADD	A	;Умножить на 8(число символов в строке)
	INR	A
	MOV	C,A
	POP	PSW

   ; Преобразование # линейки в номер скан/кода
L1INK:
	RRC
	RC		;0 бит = 1
	INR	C	;Счетчик /двоично-десятичный/
	JMP	L1INK
;****************************
;  ОПРОС СОСТОЯНИЯ КЛАВИАТУРЫ
;****************************
;Выход: A = 00 -  	;нет нажатия
;	A = FF/скан-код	;есть нажатие
CONST:
	LDA	ADR1
	ORA	A
	RNZ
	LDA	SYMBUF	;Нет, читать скан-код
	ORA	A
	RZ		;Нет такой клавиши
	MVI	A,0FFH
	RET		;Выход с кодом
;****************************
;  ВВОД СИМВОЛА С КЛАВИАТУРЫ
;****************************
CONIN:
	LDA	ADR1
	ORA	A	;Есть F-последовательность ?
	LHLD	ADR	;Да,
	JNZ	LE2AD
CONIN1:
	EI
	LDA	SYMBUF
	ORA	A
	JZ	CONIN1
	MOV	C,A
	XRA	A
	STA	SYMBUF
	DCR	C
	MVI	B,0
	LDA	CNTRKEY ;Читать линейку клавиш 'модификаций'
	MOV	E,A
	MOV	A,C	;Скан-код в A
	CPI	32
	JC	ALFKEY	; < 32 - Буквы
	CPI	44
	JC	DIGK1	; < 44 - Цифры,знаки
	CPI	48
	JC	DIGKEY	; < 48 - Знаки
	CPI	56
	JC	CTLKEY	; < 56 - Упр.клав.
	CPI	64
	JC	CTLK1	; < 64 - Клав.модификации
	CPI	80
	JC	AUXKEY	; < 80 - Цифры доп.поля
	CPI	96
	JC	FUNKEY	; < 96 - Клавиши F1 - F10
	CPI	112
	JC	AUXK1	; < 112 - Доп.поле / ESC- послед.
	XRA	A
	RET		; A = 00 - нет кода
ALFKEY:
	LXI	H,LALFTAB ; Адрес тавлицы букв
	DAD	B	; HL+SKOD = адрес символа в таблице
	MOV	L,M	; L - код символа
	MOV	A,E	; A - линейка 'модификатора'
	ANI	00100000B ; УПР ?
	JNZ	LE1DE	  ; ДА
	LDA	FGRAPH
	XRA	E	; Маска лин.модиф.
	ANI	00000100B ; ГРАФ. ?
	JZ	LE1E2	  ; НЕТ
	MOV	A,L
	XRI	0C0H	; Инверсия 6,7 бит
	RET

;  КОД - УПР (CTRL)
LE1DE:
	MOV	A,L	;Вернуть код символа
	ANI	00011111B ; Сброс 5,7 бит
	RET

;  
LE1E2:
	LDA	FALF
	XRA	E
	ANI	00000010B
	JNZ	LE1F7
	LDA	SETFLAG
	ORA	A
	JZ	LE20D	;Основной набор
	LXI	H,SIMTAB1 ; Адрес табл.рус.букв по алф.порядку
	DAD	B
	MOV	L,M	; L = Символ
LE1F7:
	CALL	LE1FC
	ADD	L
	RET
LE1FC:
	MVI	A,81H	; Маска прав,лев. клав.SHIFT
	ANA	E
	JZ	LE204	;Клавиши РЕГ не нажата
	MVI	A,0FFH	; Флаг нажатия SHIFT
LE204:
	MOV	E,A
	LDA	FSHIFT
	XRA	E	; Инверсия
	RZ		; Если РЕГ нижний
	MVI	A,20H
	RET

;  АЛФАВИТ РУССКИЙ (основной набор)
LE20D:
	CALL	LE1FC
	XRI	0A0H
	ADD	L	;Код символа + 5 или 7 бит
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
	MOV	A,E
	ANI	10000001B
	MOV	A,C
	RNZ
	ADI	00010000B
	RET

;   Знаки
DIGKEY:
	MOV	A,E
	ANI	10000001B
	MOV	A,C
	RZ
	ADI	00010000B
	RET

;   Управляющие клавиши
CTLKEY:
	LXI	H,ALFTAB1
	DAD	B
	MOV	A,M
	CPI	80H
	RC
	MOV	C,A
	JMP	LE299

CTLK1:	; КЛАВИША  'ESC'
	MVI	A,1BH
	RET

AUXKEY:	; Цифры доп.поля
	LDA	FSEL
	XRA	E
	ANI	00010000B
	JZ	AUXK1
LE284:
	MVI	A,60H
	XRA	C
	CPI	2AH
	RNC
	ADI	00010000B
	RET

AUXK1:	;  Доп.поле / ESC-последовательность
	MVI	A,81H
	ANA	E
	JZ	LE299
	CALL	LE284
	ADI	60H
	RET

LE299:	;  Доп.поле
	MVI	A,0FH
	ANA	C
	MOV	C,A
	LHLD	CONTAB
	CPI	0EH
	JC	LE2A7
	MVI	C,0AH

LE2A7:	;  Определение адреса в таблице
	DAD	B
	DAD	B
	MOV	A,M
	INX	H
	MOV	H,M
	MOV	L,A
LE2AD:
	MOV	A,M
	INX	H
	SHLD	ADR
	MOV	D,A
	ORA	A
	JZ	LE2B8
	MOV	A,M
LE2B8:
	STA	ADR1
	MOV	A,D
	RET

FUNKEY:	;  Функциональные клавиши F1 - F10
	LHLD	FUNTAB
	MVI	A,0FH
	ANA	C
	MOV	C,A
	CPI	5
	JNC	LE2A7
	MVI	A,81H
	ANA	E
	JZ	LE2A7
	MOV	A,C
	ADI	5
	MOV	C,A
	JMP	LE2A7

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
	JZ	LIST	;ждать готовнось принтера
	MOV	A,C	;готовность есть,код в А
	CPI	80H	;код меньше 80H ?
	JC	LIST1	;да, символы латынь
	LXI	H,0E287H;Н.адрес буфера символов(0Е307H - 80H)
	MVI	B,0
	DAD	B	;в HL получим адрес для таблицы
	MOV	A,M	; код из таблицы в А
LIST1:
	XRI	0FFH	; код инвертируем
	STA	LSTDAT	; выводим в порт, на принтер
	LXI	H,LSTCTL
	MVI	M,0BH	;выдать строб (1)
	XTHL		;задержка
	XTHL
	MVI	M,0AH	;сбросить строб (0)
	RET
LISTST:
	LDA	CASSIN		;читать 
	ANI	00000100B	;маска 2 
	RZ			;нет 
	MVI	A,0FFH		;готовность 
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

	;таблица руского алфавита с C0H доFFH

	DB	'юабцдефгхийклмно'
	DB	'пярстужвьызшэщчъ'
	DB	'ЮАБЦДЕФГХИЙКЛМНО'
	DB	'ПЯРСТУЖВЬЫЗШЭЩЧ',0FFH

;	П/П ВЫВОД СИМВОЛА НА ИРПС
PUNCH:
	CALL	PUNC0	;опрос готовности
	JZ	PUNCH	;нет готовности
	MOV	A,C	;есть готовнось,код в A
	STA	SERD	;выдать байт данных
	RET
PUNC0:
	LDA	SERS	;читать регистр состояния ВИ 51
	ANI	10000001B;маска готовности (данных) УВВ
	XRI	10000001B;инверсия 1 и 7 биты
	JNZ	PUNC1
	XRI	0FFH
	RET
PUNC1:
	XRA	A
	RET		;выход по неготовности (00)

;      ПРИЕМ С ИРПС ПОСЛЕДОВАТЕЛЬНЫЙ ИНТЕРФЕЙС
READER:
	CALL	RED2	;опрос готовности
	JNZ	RED1	;гот-ть есть, читать данные
	DI
	MVI	M,27H	;режим:ск-ть,прием/пер.
RED0:			;вкл контроль четн.1бит стопа
	CALL	RED3	;опрос гот-ти
	JZ	RED0	;гот-ти нет
	MVI	M,25H	;програмный сброс
RED1:
	DCX	H	;HL=рег.данных
	MOV	A,M	;данные в A
	EI
	RET		;выйти с данными в A
RED2:
	LXI	H,SERS
RED3:
	MOV	A,M	;читаем рег.состояния
	ANI	00000010B;маска бита готовности данных
	RZ
	MVI	A,0FFH
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
	JMP INT0	;0F7E0H
	DB 0		; USER
	JMP INT1	;0F7E4H
	DB 0
	JMP INT2	;0F7E8H
	DB 0
	JMP INT3	;0F7ECH
	DB 0
	JMP INT4	;0F7F0H
	DB 0
	JMP INT5	;0F7F4H
	DB 0
	JMP INT6	;0F7F8H
	DB 0
	JMP INT7	;0F7FCH

;	конец таблиц
INIT:
;при вызове прерывания должны быть закрыты
;адрес INT-таблицы - 0F7E0H : забрасываем
	DI
;Иницилизация векторов переходов прерываний
	LXI	D,TABTRAP-2
	LXI	H,DOINT0-2   ;начало области ячеек прерывания
	LXI	B,58
	CALL	DETOHL
;Контроллер прерываний
	MVI	A,KI1	; INIT INTERUPT
	STA	INT
	MVI	A,KI2
	STA	INT+1	;
	MVI	A,0EFH	;maska
	STA	INT+1
;Последовательный интерфейс
	LXI	H,IRPS+1	; INIT IRPS
	XRA	A
	MOV	M,A
	MOV	M,A
	MOV	M,A		; сброс програмный
	MVI	M,IS51RS
	MVI	M,IS51CW
	MVI	M,IS51TR
;TIMER
	LXI	H,TIMER+3
	MVI	M,7EH	; init irps timer. КАНАЛ 1 РЕЖИМ 3
	MVI	M,14H
	MVI	M,90H	; канал 2,режим 0,младший байт
	DCX	H	; 2
	DCX	H	; 1
	MVI	M,13	;9600 bod
	MVI	M,0
;Графика
	LDA	VIDEO
	ANI     4
	ORI	020H	;Графическая страница 0
	STA	VIDEO
	LXI     H,0	;стираем страницу 0 (графика)
	DAD     SP
	XCHG
	MVI     A,3CH
	STA     SYSREG
	LXI     SP,4000H+4000H
	LXI     H,0FFFFH
	LXI     B,800H
	MVI     A,0
	STA     0FFBFH
LPCLS1:
	PUSH    H
	PUSH    H
	PUSH    H
	PUSH    H
	DCR     C
	JNZ     LPCLS1
	DCR     B
	JNZ     LPCLS1
	XCHG
	SPHL
	MVI     A,1CH
	STA     SYSR3
;Иницилизируем диски, клавиатуру и дисплей
	XRA	A	; очистить
	STA	DPBK00	; флаг успешного считывания диска A
	STA	DPBK01	; --------------/---------- диска B
	STA	DPBK02	; --------------/---------- диска C
	STA	DPBK03	; --------------/---------- диска D
	STA	IOBYTE	; Select "wrong buffer "
	STA	SYMBUF	; Clear Last Received Character
	MVI	A,0FFH
	STA	LUTFL	; очистить флаг изменения LUT
	LXI	H,INITVD;|
	CALL	PSTRNG	;| Промт на экран
	MVI	A,0FFH
	STA	SNDFLG	;включили  звук
;   Клавиатура
	MVI	A,02
	STA	FALF	; Латынь
;   Принтер
	MVI	A,49H	;
	STA	STBPORT	; Строб на принтер
	RET

;       Инициализация табл. LUT
LUT:
	CALL	TBLANK
	LHLD	ADRLUT
	MVI	B,16
LTLP:
	MOV	A,M	;Загрузить в копию просмотровую табл.
	STA	LUTC
	INX	H
	DCR	B
	JNZ	LTLP
	RET
;         Ожидание конца кадрового импульса
TBLANK:
	LDA	CASSIN
	ANI	2
	JNZ	TBLANK
	MVI	A,50
LPDL:
	XTHL
	XTHL
	DCR	A
	JNZ	LPDL
	RET
INITVD:
	DB 1BH,';',1BH,':',0	;включили курсор
HINIT:
	LXI	D,TABFK	;Таблица функц.клавиш
	LXI	H,KBDBAS;Адрес загрузки
	LXI	B,0F4H  ;Размер таблици
	CALL	DETOHL  ;Перегрузить массив
	LXI	D,TABPR	;Рабочие ячейки BIOS
	LXI	H,0F715H;Адрес загрузки
	LXI	B,006FH	;Размер таблици
	JMP	DETOHL  ;Перегрузить массив

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

;====================================================
;	HL - содержит CPMTXX - 2
GETINFO:
	PUSH	H	; таблица
	CALL	FLUSH	; Вернуть буфер на диск,если изменился
	POP	H
	SHLD	CPMT
	PUSH	H
	MOV	A,M	; A= номер драйвера
	ANI	0FH	; Маска /макс. 16 дисководов/
	STA	DREG

;forth32 insert patch here

	XRA	A
	STA	TRKSEK
	INR	A
	STA	SECSEK
	LXI	H,CPMT
	CALL	DSETUP
DENOK:
	LXI	H,RDBUFF ;Буфер для чтения с диска
	CALL	DTOM	; Чтение дисковых параметров
	JZ	CHKDO	; Нрма, на подсчет K.S
	LDA	DRVREG	; Ошибка, попытка чт-ия с одинар.плотн.
	ADI	40H	; Одинарная плотность
	STA	DRVREG
	CALL	FORCE
	LXI	H,RDBUFF
	CALL	DTOM	;Физическое чтение с диска
	LXI	H,ERMSG	;Подготовить "ДИСК НЕ ЧИТАЕТСЯ"
	JNZ	GIERR	; 1 сектор ERROR

;   ПОДСЧЕТ K.S
CHKDO:
	LDA	DREG
	STA	DREGWR
	XRA	A
	STA	TRKRDB
	INR	A	; HL =1
	STA	SECBLS
	LXI	H,RDBUFF
	MVI	A,66H	; Константа для подсчета K.S
	MVI	B,1FH	; Число байт с 1 сектора
CHKDO1:
	ADD	M	    ; calculate checksum
	INX	H	    ; bump pointer
	DCR	B	    ; decrement counter
	JNZ	CHKDO1	    ; Loop
	CMP	M	   ; Compare with precalculated checksu
	JZ	CHOK	   ; If match - O.K.
;==============================================
;  Добавлено из BIOS 56K
;
;	LDA	XGETINF
;	ANI	0FH
;	CPI	1
;	JZ	GIN5
;==============================================

	LXI	H,ERMSG1

;  ВЫВОД 'ОШИБКА'
GIERR:
	POP     D
	CALL	PSTRNG	; Print error message
	MVI	A,1	; With error condition
	ORA	A
	RET
ERMSG:
	DB	ESC,63H,CR,LF
	DB	'***** Диск не читается	*****'
	DB	CR,LF,ESC,63H
	DB	00
ERMSG1:
	DB	ESC,63H,CR,LF
	DB	'****** Нет системной информации ******'
	DB	CR,LF,ESC,63H
	DB	00

;  Загрузка считанной информации в табл.CPMTxx
CHOK:
	POP	H	; HL points to DPBLKXX
	PUSH	H	; Save HL
	LDA	DRVREG
	ANI	0CFH	; (motor or side1)
	MOV	M,A	;dregx
	INX	H
	MVI	M,0FFH	;flag out - there is info !
	INX	H	; cpmtxx  СЕКТОР/ТРЕК
	LXI	D,RDBUFF+16
	LXI	B,15		;+lentrtab   ; info + tabl
	CALL	DETOHL	; Make transfer to DPBLKXX
GIN3:
	POP	D	; restore pointer
	XRA	A
	DCX	D
	STAX	D	; mark - we are on track zero now
	DCX	D	;Байт/сектор (0-128,2-256,3-1024)
	LXI	H,RDBUFF+10
	MOV	A,M	;size sector
	STAX	D	;Обьем физического сектора
	DCX	D	;Сторон диска (0-одност.1-двухстор.)
	INX	H
	MOV	A,M	;нумерация секторов
	STAX	D
	MOV	C,A
	INR	C
	MVI	A,80H
GIN4:
	RLC
	DCR	C
	JNZ	GIN4
	STA	DREGRD
	DCX	D
	INX	H
	MOV	A,M	;Сектор/трек
	STAX	D
	DCX	D
	DCX	D
	INX	H
	INX	H
	MOV	A,M
	STAX	D
	XRA	A
	RET
;================================================
;  Добавление для BIOS 56K
;
;GIN5:
;	POP	H
;	PUSH	H
;	LDA	DRVREG
;	ANI	0CFH
;	MOV	M,A	;__________________________
;	INX	H
;	MVI	M,0FFH	;__________________________
;	INX	H
;	POP	D	;DE = Сохраненый HL
;	XRA	A
;	DCX	D
;	STAX	D	;_________________________
;	DCX	D
;	MVI	A,3
;	STAX	D	;_________________________
;	DCX	D
;	INX	H
;	MVI	A,1
;	STAX	D	;_________________________
;	MOV	C,A
;	INR	C
;	MVI	A,80H
;GIN6:
;	RLC
;	DCR	C
;	JNZ	GIN6
;	STA	DREGRD	;_________________________
;	DCX	D
;	INX	H
;	MVI	A,5
;	STAX	D	;_________________________
;	DCX	D
;	DCX	D
;	INX	H
;	INX	H
;	MVI	A,50H
;	STAX	D	;_________________________
;	XRA	A
;	RET
;============================================
LTOF:
	LHLD	CPMT
	MOV	A,M
	STA	DREG	;# драйвера/устройства
	DCX	H
	DCX	H
	MOV	B,M	;Размер физического сектора
	MVI	C,0FFH
	LDA	SECRDB
	DCR	A
	INR	B
	MOV	D,A
	MOV	E,A
LPTS:
	DCR	B
	JZ	SKIPTS	;Сектор кончился
	ORA	A
	MOV	A,E
	RAR	;---->	; Разделить на 2
	MOV	E,A
	MOV	A,C
	ADD	A
	MOV	C,A
	JMP	LPTS
SKIPTS:
	MOV	A,C
	ANA	D
	INR	A
	STA	OFSSEK
	MOV	A,C
	CMA
	INR	A
	STA	OFSSEK+1
	DCX	H	;Информация о сторонах
	MOV	D,M	;Флаг числа сторон
	ORA	A
	DCR	D
	LDA	TRKWRB
	JNZ	LE765	;Односторонний
	RAR
LE765:
	STA	TRKSEK	;Физичецкий номер трека
	CC	SIDE10	;Если трек нечетный
	DCR	D
	JNZ	DZIDE
	DCX	H
	MOV	A,E
	SUB	M
	JC	DZIDE
	MOV	E,A
	CALL	SIDE10
DZIDE:
	MOV	A,E
	INR	A
	STA	SECSEK	;
	RET
SIDE10:
	LDA	DREG	;# текущего драйвера
	ORI	SIDE1
	STA	DREG
	RET
    
;     ФИЗИЧЕСКОЕ ЧТЕНИЕ ДИСКОВЫХ ПАРАМЕТРОВ
DTOB:
	DB 00
DTOM:
	MVI	A,3	;Кол-во попыток запуска двигателя
DTOM0:
	STA	DTOB
	CALL	DTOM1
	RZ		;Есть включение мотора
	LDA	DTOB
	DCR	A
	JNZ	DTOM0
	DCR	A
	RET		;Выход с ERROR
DTOM1:
	CALL	MOTON
	RNZ
	PUSH	H
	DI
	MVI	A,80H	;Команда 'чтение сектора'
	CALL	DELAY3
	XRA	A
	STA	SYSREG
	LXI	B,0301H
	LXI	D,FDC3
DTOM2:
	LDAX	D	;Читать регистр состояния
	ANA	B
	XRA	C
	JZ	DTOM2
	ANA	C
	LDA	FDC3+3	;Читать DATA с диска
	JNZ	DTOM3
	MOV	M,A	;Байт в буфер
	INX	H
	JMP	DTOM2	;За след. байтом
DTOM3:
	XCHG
; 	                HL - buffer address
EXITRW:	
	MVI	A,1CH
	STA	SYSR1   ;registr konfiguracii
	POP	H
	LDA	FDC
	ANI	0DDH	;Маска ошибок, учитываем READY
	EI
	STA     ERRFL	;код ошибки
	RET

;   ФИЗИЧЕСКАЯ ЗАПИСЬ НА ДИСК
;                  HL - buffer address
MTOD:
	CALL	MOTON
	RNZ
MTODR:
	DI		;выключаем на время операции
	MVI	A,WRSECCMD
	CALL	DELAY3
	PUSH	H	; save buffer pointer
	XCHG
	LXI	H,0
	DAD	SP
	SHLD	STACKS	; спасаем 
	XCHG
	SPHL		; stack - buffer
	XRA	A
	STA	SYSREG
	LXI	H,FDC3+3
	LXI	D,FDC3
MTOD1:
	POP	B	; Данные из буфера
MTOD2:
	LDAX	D	; get status
	XRI	1	; make test for busy and drq bits
	JZ	MTOD2	; if not done - wait
	MOV	M,C	; DATA на диск
	RAR
	JC      WDONE	; go if not busy
	RAR
	JNC	MTOD2	; continue when busy still
MTOD3:
	LDAX	D	; get status
	XRI	1	; make test for busy and drq bits
	JZ	MTOD3	; if not done - wait
	MOV	M,B	; issue 2nd byte of data
	JMP	MTOD1	; loop until all'll be done
WDONE:
	LXI	H,0FFFEH
	DAD	SP
	XCHG
	LHLD	STACKS
	SPHL		; restore stack
	JMP	EXITRW
MOTON:
	LXI	D,DRVREG
	MVI	A,0AH	; КУ3 - чтобы читать регистр запросов
	STA	INT
	LDA     INT
	ANI	80H	; для мотора
	PUSH	PSW
	LDAX	D
	ANI	NOT MOTOR
	STAX	D
	ORI	MOTOR
	STAX	D	;перезапустили мотор
	ANI     NOT MOTOR
	STAX    D
	POP	PSW
	CNZ	DELAY	;если мотор был выключен
	MVI     C,10	; <=> 2 сек.
LP0000:
	LDAX	D
	ANI	0DFH
	STAX	D
	ORI	20H	;ВКЛ.двигатель
	STAX	D
	ANI	0DFH	;Снять импульс
	STAX	D
	LDA     FDC
	RLC
	JNC     READYOK		;ждем READY
	DCR     C
	JZ      NREADY		;ошибка: NOT READY
	CALL    DELAY
	JMP     LP0000
READYOK: 
	LXI	D,FDC		; SET FDC & FORCE
	JMP	FORCE
NREADY:
	MVI     A,DNOTR
	STA     ERRFL
	ORA     A
	RET
DELAY:
	MVI     B,0
DELAY1:
	CALL	DELAY0
	DCR	B
	JNZ	DELAY1
	RET
FORCE:
	MVI	A,FORCECMD
DELAY3:
	STA	FDC
	MVI	A,0FH	
DELAY0:
	DCR	A
	JNZ	DELAY0
	RET

	END
& FORCE
	JMP	FORCE
NREADY:
	MVI     A,DNOTR
	STA     ERRFL
	ORA     A
	RET
DE