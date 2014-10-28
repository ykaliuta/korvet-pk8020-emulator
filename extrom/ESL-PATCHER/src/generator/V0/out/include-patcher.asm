bios_variants:
	dw	_BIOS_CPM_21_EXTROM
	dw	_BIOS_CPM_12_88_3_alternativa
	dw	_BIOS_CPM_12_88_3_niijaf
	dw	_BIOS_CPM_21_89___wiza
	dw	_BIOS_MICRODOS_OPTS2_880630
	dw	_BIOS_CPM_21_89_2_niijaf2
	dw	_BIOS_CPM_21_89_2_niijaf
	dw	_BIOS_MICRODOS_OPTS1_870430
	dw	_BIOS_CPM_1x_89_03_30_RAVI
	dw	_BIOS_MICRODOS_OPTS1_861011
	dw	_BIOS_CPM_21_91___LAP
	dw	_BIOS_CPM_20_88___miks
	dw	_BIOS_CPM_12_90_5_kontur
	dw	_BIOS_MICRODOS_OPTS2_900105
	dw	_BIOS_CPM_12_87_11_niijaf
	dw	_BIOS_CPM_21m_____Shkanov
	dw	_BIOS_CPM_12_87_09_NIIJAF
	dw	_BIOS_MICRODOS_OPTS1_871220
	dw	_BIOS_MICRODOS_OPTS1_861115
	dw	_BIOS_CPM_NET_SFERA1
	dw	_BIOS_CPM_NET_KORNET_drive_b
	dw	_BIOS_CPM_NET_KORNET_drive_a
	dw	_BIOS_CPM_1x_88_EPSON_V104
	dw	_BIOS_CPM_NET_SFERA2
	dw	0

	db 	"PATCHER DATA>>"

	include "generator/V0/out/patcher-cpm_chk-CPM_21_EXTROM.asm"
	include "generator/V0/out/patcher-cpm1-CPM_12_88_3_alternativa.asm"	; 25 images in collection
	include "generator/V0/out/patcher-cpm1-CPM_12_88_3_niijaf.asm"	; 78 images in collection
	include "generator/V0/out/patcher-cpm1-CPM_21_89___wiza.asm"	; 42 images in collection
	include "generator/V0/out/patcher-microdos-MICRODOS_OPTS2_880630.asm"	; 42 images in collection
	include "generator/V0/out/patcher-cpm1-CPM_21_89_2_niijaf2.asm"	; 36 images in collection
	include "generator/V0/out/patcher-cpm1-CPM_21_89_2_niijaf.asm"	; 28 images in collection
	include "generator/V0/out/patcher-microdos-MICRODOS_OPTS1_870430.asm"	; 28 images in collection
	include "generator/V0/out/patcher-cpm2-CPM_1x_89_03_30_RAVI.asm"	; 17 images in collection
	include "generator/V0/out/patcher-microdos-MICRODOS_OPTS1_861011.asm"	;  7 images in collection
	include "generator/V0/out/patcher-cpm1-CPM_21_91___LAP.asm"	;  4 images in collection
	include "generator/V0/out/patcher-cpm1-CPM_20_88___miks.asm"	;  4 images in collection
	include "generator/V0/out/patcher-cpm1-CPM_12_90_5_kontur.asm"	;  4 images in collection
	include "generator/V0/out/patcher-microdos-MICRODOS_OPTS2_900105.asm"	;  4 images in collection
	include "generator/V0/out/patcher-cpm2-CPM_12_87_11_niijaf.asm"	;  3 images in collection
	include "generator/V0/out/patcher-cpm1-CPM_21m_____Shkanov.asm"	;  2 images in collection
	include "generator/V0/out/patcher-cpm1-CPM_12_87_09_NIIJAF.asm"	;  2 images in collection
	include "generator/V0/out/patcher-microdos-MICRODOS_OPTS1_871220.asm"	;  2 images in collection
	include "generator/V0/out/patcher-microdos-MICRODOS_OPTS1_861115.asm"	;  2 images in collection
	include "generator/V0/out/patcher-unsopported-CPM_NET_SFERA1.asm"	;  2 images in collection
	include "generator/V0/out/patcher-unsopported-CPM_NET_KORNET_drive_b.asm"	;  2 images in collection
	include "generator/V0/out/patcher-unsopported-CPM_NET_KORNET_drive_a.asm"	;  2 images in collection
	include "generator/V0/out/patcher-cpm2-CPM_1x_88_EPSON_V104.asm"	;  1 images in collection
	include "generator/V0/out/patcher-unsopported-CPM_NET_SFERA2.asm"	;  1 images in collection
	db 0xff
