#include <string.h>
#include <stdio.h>

//#include "config.h"
#include "i8080.h"
#include "i8080dis.h"
#include "mem.h"

char *bregs[]={ "B", "C", "D", "E", "H", "L", "M", "A" };
char *wregs[]={ "B", "D", "H", "SP" };
char *pregs[]={ "B", "D", "H", "PSW" };
char *conds[]={ "NZ", "Z", "NC", "C", "PO", "PE", "P", "M" };
char *rsts[]={ "0", "1", "2", "3", "4", "5", "6", "7" };
int src, dst;

struct arg {
	int type; /* 1 - next byte, 2 - next word, 3 - in opcode */
	int shift;
	int mask;
	char **params;
};

struct opcode {
	unsigned char cmd;
	int size;
	char name[16];
	struct arg first, second;
} opcodes[] = {
	{ 0x76, 1, "HLT", { 0, 0, 0, 0 }, { 0, 0, 0, 0 } },
	{ 0x06, 2, "MVI %s, %02X", { 3, 3, 7, bregs }, { 1, 0, 0, 0 } },
	{ 0xc3, 3, "JMP %04X", { 2, 0, 0, 0 }, { 0, 0, 0, 0 } },
	{ 0x40, 1, "MOV %s, %s", { 3, 3, 7, bregs }, { 3, 0, 7, bregs } },
	{ 0x01, 3, "LXI %s, %04X", { 3, 4, 3, wregs }, { 2, 0, 0, 0 } },
	{ 0x32, 3, "STA %04X", { 2, 0, 0, 0 }, { 0, 0, 0, 0 } },
	{ 0x3a, 3, "LDA %04X", { 2, 0, 0, 0 }, { 0, 0, 0, 0 } },
	{ 0x2a, 3, "LHLD %04X", { 2, 0, 0, 0 }, { 0, 0, 0, 0 } },
	{ 0x22, 3, "SHLD %04X", { 2, 0, 0, 0 }, { 0, 0, 0, 0 } },
	{ 0x0a, 1, "LDAX %s", { 3, 4, 1, wregs }, { 0, 0, 0, 0 } },
	{ 0x02, 1, "STAX %s", { 3, 4, 1, wregs }, { 0, 0, 0, 0 } },
	{ 0xeb, 1, "XCHG", { 0, 0, 0, 0 }, { 0, 0, 0, 0 } },
	{ 0xf9, 1, "SPHL", { 0, 0, 0, 0 }, { 0, 0, 0, 0 } },
	{ 0xe3, 1, "XTHL", { 0, 0, 0, 0 }, { 0, 0, 0, 0 } },
	{ 0xc5, 1, "PUSH %s", { 3, 4, 3, pregs }, { 0, 0, 0, 0 } },
	{ 0xc1, 1, "POP %s", { 3, 4, 3, pregs }, { 0, 0, 0, 0 } },
	{ 0xdb, 2, "IN %02X", { 1, 0, 0, 0 }, { 0, 0, 0, 0 } },
	{ 0xd3, 2, "OUT %02X", { 1, 0, 0, 0 }, { 0, 0, 0, 0 } },
	{ 0x03, 1, "INX %s", { 3, 4, 3, wregs }, { 0, 0, 0, 0 } },
	{ 0x0b, 1, "DCX %s", { 3, 4, 3, wregs }, { 0, 0, 0, 0 } },
	{ 0x04, 1, "INR %s", { 3, 3, 7, bregs }, { 0, 0, 0, 0 } },
	{ 0x05, 1, "DCR %s", { 3, 3, 7, bregs }, { 0, 0, 0, 0 } },
	{ 0x09, 1, "DAD %s", { 3, 4, 3, wregs }, { 0, 0, 0, 0 } },
	{ 0x2f, 1, "CMA", { 0, 0, 0, 0 }, { 0, 0, 0, 0 } },
	{ 0x07, 1, "RLC", { 0, 0, 0, 0 }, { 0, 0, 0, 0 } },
	{ 0x0f, 1, "RRC", { 0, 0, 0, 0 }, { 0, 0, 0, 0 } },
	{ 0x17, 1, "RAL", { 0, 0, 0, 0 }, { 0, 0, 0, 0 } },
	{ 0x1f, 1, "RAR", { 0, 0, 0, 0 }, { 0, 0, 0, 0 } },
	{ 0xfb, 1, "EI", { 0, 0, 0, 0 }, { 0, 0, 0, 0 } },
	{ 0xf3, 1, "DI", { 0, 0, 0, 0 }, { 0, 0, 0, 0 } },
	{ 0x00, 1, "NOP", { 0, 0, 0, 0 }, { 0, 0, 0, 0 } },
	{ 0x37, 1, "STC", { 0, 0, 0, 0 }, { 0, 0, 0, 0 } },
	{ 0x3f, 1, "CMC", { 0, 0, 0, 0 }, { 0, 0, 0, 0 } },
	{ 0xe9, 1, "PCHL", { 0, 0, 0, 0 }, { 0, 0, 0, 0 } },
	{ 0x27, 1, "DAA", { 0, 0, 0, 0 }, { 0, 0, 0, 0 } },
	{ 0xcd, 3, "CALL %04X", { 2, 0, 0, 0 }, { 0, 0, 0, 0 } },
	{ 0xc9, 1, "RET", { 0, 0, 0, 0 }, { 0, 0, 0, 0 } },
	{ 0xc7, 1, "RST %s", { 3, 3, 7, rsts }, { 0, 0, 0, 0 } },
	{ 0xc0, 1, "R%s", { 3, 3, 7, conds }, { 0, 0, 0, 0 } },
	{ 0xc2, 3, "J%s %04X", { 3, 3, 7, conds }, { 2, 0, 0, 0 } },
	{ 0xc4, 3, "C%s %04X", { 3, 3, 7, conds }, { 2, 0, 0, 0 } },
	{ 0x80, 1, "ADD %s", { 3, 0, 7, bregs }, { 0, 0, 0, 0 } },
	{ 0x80|0x46, 2, "ADI %02X", { 1, 0, 0, 0 }, { 0, 0, 0, 0 } },
	{ 0x88, 1, "ADC %s", { 3, 0, 7, bregs }, { 0, 0, 0, 0 } },
	{ 0x88|0x46, 2, "ACI %02X", { 1, 0, 0, 0 }, { 0, 0, 0, 0 } },
	{ 0x90, 1, "SUB %s", { 3, 0, 7, bregs }, { 0, 0, 0, 0 } },
	{ 0x90|0x46, 2, "SUI %02X", { 1, 0, 0, 0 }, { 0, 0, 0, 0 } },
	{ 0x98, 1, "SBB %s", { 3, 0, 7, bregs }, { 0, 0, 0, 0 } },
	{ 0x98|0x46, 2, "SBI %02X", { 1, 0, 0, 0 }, { 0, 0, 0, 0 } },
	{ 0xa0, 1, "ANA %s", { 3, 0, 7, bregs }, { 0, 0, 0, 0 } },
	{ 0xa0|0x46, 2, "ANI %02X", { 1, 0, 0, 0 }, { 0, 0, 0, 0 } },
	{ 0xa8, 1, "XRA %s", { 3, 0, 7, bregs }, { 0, 0, 0, 0 } },
	{ 0xa8|0x46, 2, "XRI %02X", { 1, 0, 0, 0 }, { 0, 0, 0, 0 } },
	{ 0xb0, 1, "ORA %s", { 3, 0, 7, bregs }, { 0, 0, 0, 0 } },
	{ 0xb0|0x46, 2, "ORI %02X", { 1, 0, 0, 0 }, { 0, 0, 0, 0 } },
	{ 0xb8, 1, "CMP %s", { 3, 0, 7, bregs }, { 0, 0, 0, 0 } },
	{ 0xb8|0x46, 2, "CPI %02X", { 1, 0, 0, 0 }, { 0, 0, 0, 0 } },
	{ 0x00, 1, "NOP", { 0, 0, 0, 0 }, { 0, 0, 0, 0 } },
	{ 0x00, 0 }
};

#define check_flag(idx, flag, value) 	\
	case idx:                       \
		cond = flag==value;	\
		break

#define check_flags()				\
	switch(cond_dir) {			\
		check_flag(0, Z_FLAG, 0);	\
		check_flag(1, Z_FLAG, 1);	\
		check_flag(2, C_FLAG, 0);	\
		check_flag(3, C_FLAG, 1);	\
		check_flag(4, P_FLAG, 0);	\
		check_flag(5, P_FLAG, 1);	\
		check_flag(6, S_FLAG, 0);	\
		check_flag(7, S_FLAG, 1);	\
	}

int disasm(char *text, unsigned int offset)
{
	int cmd;
	unsigned char nextbyte;
	unsigned short nextword;
	int cond;
	int cond_dir;
	char bytes[16];
	int size;
	int i, j;

	offset&=0xffff;
	cmd=GET_BYTE(offset);
	nextbyte=GET_BYTE(offset+1);
	nextword=GET_WORD(offset+1);

	for(i=0; opcodes[i].size; i++)
		if ( (cmd & ~(opcodes[i].first.mask<<opcodes[i].first.shift | opcodes[i].second.mask<<opcodes[i].second.shift)) == opcodes[i].cmd) {
			size=opcodes[i].size;

			sprintf(text, "%04X: ", offset);

			for(j=0; j<size; j++)
				sprintf( text+j*2+6, "%02X", GET_BYTE(offset+j));

			memset(text+size*2+6, ' ', (3-size)*2+2);

			sprintf(text+3*2+7, opcodes[i].name,
				opcodes[i].first.type==3?opcodes[i].first.params[cmd>>opcodes[i].first.shift & opcodes[i].first.mask]:
				(opcodes[i].first.type==1?nextbyte:nextword),
				opcodes[i].second.type==3?opcodes[i].second.params[cmd>>opcodes[i].second.shift & opcodes[i].second.mask]:
				(opcodes[i].second.type==1?nextbyte:nextword) );
			/*
			 * Process branch prediction for conditional commands
			 * Cxx/Jxx/Rxx
			 * only when we are analysing current command
			 */
			if (PC==offset && (opcodes[i].cmd==0xc4 || opcodes[i].cmd==0xc2 || opcodes[i].cmd==0xc0)) {
				cond_dir=cmd>>opcodes[i].first.shift & opcodes[i].first.mask;
				check_flags();
				strcat(text, cond ? (opcodes[i].cmd==0xc0 ? " *" : (nextword<PC ? " \x18" : " \x19" ) ): "");
			}
			if (PC==offset && (opcodes[i].cmd==0xc3 || opcodes[i].cmd==0xcd)) {
				strcat(text, nextword<PC ? " \x18" : " \x19");
			}
			return(size);
		}
	sprintf(text, "%04X: %02X     DB %02X", offset, cmd, cmd);
	return(1);
}

int get_instr_length(unsigned int offset)
{
	int cmd;
	int i;

	offset&=0xffff;
	cmd=mem[offset];

	for(i=0; opcodes[i].size; i++)
		if ( (cmd & ~(opcodes[i].first.mask<<opcodes[i].first.shift | opcodes[i].second.mask<<opcodes[i].second.shift)) == opcodes[i].cmd) {
			return(opcodes[i].size);
		}
	return(1);
}

int is_jmp_ret_pchl(unsigned char opcode)
{
	/*
	 * PCHL, RET, JMP
	 */
	if(opcode==0xe9 || opcode==0xc9 || opcode==0xc3)
		return(1);

	if( (opcode & ~(7<<3))==0xc7 )		/* RST xx */
		return(1);

	if( (opcode & ~(7<<3))==0xc0 )		/* Rxx */
		return(1);

	if( (opcode & ~(7<<3))==0xc2 )		/* Jxx */
		return(1);

	return(0);
}

int is_call(unsigned char opcode)
{
	if(opcode==0xcd)			/* CALL */
		return(1);

	if( (opcode & ~(7<<3))==0xc4 )		/* Cxx */
		return(1);

	return(0);
}
