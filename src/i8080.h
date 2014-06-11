/*
 * Korvet Team                                               2000...2005
 * ETALON Korvet Emulator                         http://pk8020.narod.ru
 * ---------------------------------------------------------------------
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 */
#ifndef _I8080_H
#define _I8080_H

typedef unsigned char           uns8;
typedef unsigned short          uns16;
typedef unsigned long int       uns32;
typedef signed char             sgn8;
typedef signed short            sgn16;
typedef signed long int         sgn32;
typedef void (*opcode_fn) (void);

extern void invalid_opcode(void);
extern void I8080_Init(void);
extern void I8080_Reset(void);
extern void I8080_StoreFlags(void);
extern void I8080_RetrieveFlags(void);
extern struct i8080 Cpu;
extern int Tstates;
extern uns32 work32;
extern uns16 work16;
extern uns8 work8, work8_2;
extern int idx;
extern uns8 carry, add, sub;
extern int parityTbl[];
extern int halfCarryTbl[];
extern int overflowTbl[];
extern int subHalfCarryTbl[];
extern int subOverflowTbl[];
extern volatile int trace;
extern volatile int reset;
extern volatile int shutdown;

typedef union {
	struct { uns8 l, h; } b;
	uns16 w;
} pair;

typedef struct {
        uns8 carry_flag           : 1;
	uns8 add_subtract_flag    : 1;
        uns8 parity_overflow_flag : 1;
        uns8 unused3              : 1;
        uns8 half_carry_flag      : 1;
        uns8 unused5              : 1;
        uns8 zero_flag            : 1;
        uns8 sign_flag            : 1;
} flag_reg;

struct i8080 {
        pair af, bc, de, hl;
        pair sp, pc;
        uns16 iff;
        uns16 last_pc;
};

#define AF                      Cpu.af.w
#define BC                      Cpu.bc.w
#define DE                      Cpu.de.w
#define HL                      Cpu.hl.w
#define SP                      Cpu.sp.w
#define PC                      Cpu.pc.w
#define A                       Cpu.af.b.h
#define F                       Cpu.af.b.l
#define B                       Cpu.bc.b.h
#define C                       Cpu.bc.b.l
#define D                       Cpu.de.b.h
#define E                       Cpu.de.b.l
#define H                       Cpu.hl.b.h
#define L                       Cpu.hl.b.l
#define HSP                     Cpu.sp.b.h
#define LSP                     Cpu.sp.b.l
#define HPC                     Cpu.pc.b.h
#define LPC                     Cpu.pc.b.l
#define IFF                     Cpu.iff
#define LAST_PC                 Cpu.last_pc

#define F_CARRY                 0x01
#define F_NADD                  0x02
#define F_PARITY                0x04
#define F_OVERFLOW              F_PARITY
#define F_HCARRY                0x10
#define F_ZERO                  0x40
#define F_NEG                   0x80

#define C_FLAG                  ((flag_reg *)&F)->carry_flag
#define N_FLAG                  ((flag_reg *)&F)->add_subtract_flag
#define P_FLAG                  ((flag_reg *)&F)->parity_overflow_flag
#define V_FLAG                  ((flag_reg *)&F)->parity_overflow_flag
#define H_FLAG                  ((flag_reg *)&F)->half_carry_flag
#define Z_FLAG                  ((flag_reg *)&F)->zero_flag
#define S_FLAG                  ((flag_reg *)&F)->sign_flag
#define UN3_FLAG                ((flag_reg *)&F)->unused3
#define UN5_FLAG                ((flag_reg *)&F)->unused5

#define SET(flag)               (flag = 1)
#define CLR(flag)               (flag = 0)
#define TST(flag)               (flag)
#define CPL(flag)               (flag = !flag)

#define LOWBYTE(reg)            ((reg) & 0xff)
#define HIGHBYTE(reg)           (((reg) >> 8) & 0xff)

#define POP(reg)                { (reg) = RD_WORD(SP); SP += 2; }
#define PUSH(reg)               { SP -= 2; WR_WORD(SP, (reg)); }
#define RET()                   { POP(PC); }
#define STC()                   { SET(C_FLAG); CLR(N_FLAG); }
#define CMC()                   { CPL(C_FLAG); CLR(N_FLAG); }

#define I8080_M1(op) \
        {                                               \
		(op) = RD_BYTE(PC++);                   \
	}

#define INR(reg) \
	{                                               \
		++(reg);                                \
		S_FLAG = (((reg) & 0x80) != 0);         \
		Z_FLAG = ((reg) == 0);                  \
		H_FLAG = (((reg) & 0x0f) == 0);         \
		P_FLAG = PARITY(reg);			\
		CLR(N_FLAG);                            \
	}
#define INX(reg) \
	{                                               \
		++(reg);                                \
	}
#define DCR(reg) \
	{                                               \
		--(reg);                                \
		S_FLAG = (((reg) & 0x80) != 0);         \
		Z_FLAG = ((reg) == 0);                  \
		H_FLAG = (((reg) & 0x0f) == 0x0f);      \
		P_FLAG = PARITY(reg);			\
		SET(N_FLAG);                            \
	}
#define DCX(reg) \
	{                                               \
		--(reg);                                \
	}
#define ADD(val) \
	{                                               \
		work16 = (uns16)A + (val);		\
		idx = ((A & 0x88) >> 1) |               \
			  (((val) & 0x88) >> 2) |       \
			  ((work16 & 0x88) >> 3);       \
		A = work16 & 0xff;                      \
		S_FLAG = ((A & 0x80) != 0);             \
		Z_FLAG = (A == 0);                      \
		H_FLAG = halfCarryTbl[idx & 0x7];       \
		P_FLAG = PARITY(A);			\
		C_FLAG = ((work16 & 0x0100) != 0);      \
		CLR(N_FLAG);                            \
	}
#define ADC(val) \
	{                                               \
		work16 = (uns16)A + (val) + C_FLAG;	\
		idx = ((A & 0x88) >> 1) |               \
			  (((val) & 0x88) >> 2) |       \
			  ((work16 & 0x88) >> 3);       \
		A = work16 & 0xff;                      \
		S_FLAG = ((A & 0x80) != 0);             \
		Z_FLAG = (A == 0);                      \
		H_FLAG = halfCarryTbl[idx & 0x7];       \
		P_FLAG = PARITY(A);			\
		C_FLAG = ((work16 & 0x0100) != 0);      \
		CLR(N_FLAG);                            \
	}
#define SUB(val) \
	{                                               \
		work16 = (uns16)A - (val);		\
		idx = ((A & 0x88) >> 1) |               \
			  (((val) & 0x88) >> 2) |       \
			  ((work16 & 0x88) >> 3);       \
		A = work16 & 0xff;                      \
		S_FLAG = ((A & 0x80) != 0);             \
		Z_FLAG = (A == 0);                      \
		H_FLAG = subHalfCarryTbl[idx & 0x7];    \
		P_FLAG = PARITY(A);			\
		C_FLAG = ((work16 & 0x0100) != 0);      \
		SET(N_FLAG);                            \
	}
#define SBB(val) \
	{                                               \
		work16 = (uns16)A - (val) - C_FLAG;	\
		idx = ((A & 0x88) >> 1) |               \
			  (((val) & 0x88) >> 2) |       \
			  ((work16 & 0x88) >> 3);       \
		A = work16 & 0xff;                      \
		S_FLAG = ((A & 0x80) != 0);             \
		Z_FLAG = (A == 0);                      \
		H_FLAG = subHalfCarryTbl[idx & 0x7];    \
		P_FLAG = PARITY(A);			\
		C_FLAG = ((work16 & 0x0100) != 0);      \
		SET(N_FLAG);                            \
	}
#define ANA(val) \
	{                                               \
		A &= (val);                             \
		S_FLAG = ((A & 0x80) != 0);             \
		Z_FLAG = (A == 0);                      \
		SET(H_FLAG);                            \
		P_FLAG = PARITY(A);                     \
		CLR(C_FLAG);                            \
		CLR(N_FLAG);                            \
	}
#define XRA(val) \
	{                                               \
		A ^= (val);                             \
		S_FLAG = ((A & 0x80) != 0);             \
		Z_FLAG = (A == 0);                      \
		CLR(H_FLAG);                            \
		P_FLAG = PARITY(A);                     \
		CLR(C_FLAG);                            \
                CLR(N_FLAG);                            \
        }
#define ORA(val) \
        {                                               \
                A |= (val);                             \
                S_FLAG = ((A & 0x80) != 0);             \
                Z_FLAG = (A == 0);                      \
                CLR(H_FLAG);                            \
		P_FLAG = PARITY(A);                     \
		CLR(C_FLAG);                            \
		CLR(N_FLAG);                            \
	}
#define CMP(val) \
	{                                               \
		work16 = (uns16)A - (val);		\
		idx = ((A & 0x88) >> 1) |               \
			  (((val) & 0x88) >> 2) |       \
			  ((work16 & 0x88) >> 3);       \
		S_FLAG = ((work16 & 0x80) != 0);        \
		Z_FLAG = ((work16 & 0xff) == 0);        \
		H_FLAG = subHalfCarryTbl[idx & 0x7];    \
		C_FLAG = ((work16 & 0x0100) != 0);      \
		P_FLAG = PARITY(A);                     \
		SET(N_FLAG);                            \
	}

#define _CMP_(val) \
	{                                               \
		work16 = (uns16)A - (val);		\
		idx = ((A & 0x88) >> 1) |               \
			  (((val) & 0x88) >> 2) |       \
			  ((work16 & 0x88) >> 3);       \
		S_FLAG = ((work16 & 0x80) != 0);        \
		Z_FLAG = ((work16 & 0xff) == 0);        \
		H_FLAG = subHalfCarryTbl[idx & 0x7];    \
		P_FLAG = PARITY(A);                     \
		CLR(N_FLAG);                            \
	}
#define DAD(reg) \
	{                                               \
		work32 = (uns32)HL + (reg);		\
		idx = ((HL & 0x0800) >> 9) |            \
			  (((reg) & 0x0800) >> 10) |    \
			  ((work32 & 0x0800) >> 11);    \
		HL = work32 & 0xffff;                   \
		C_FLAG = ((work32 & 0x10000L) != 0);    \
		CLR(N_FLAG);                            \
	}
#define RLC() \
	{                                               \
		C_FLAG = (((A) & 0x80) != 0);           \
		(A) = ((A) << 1) | C_FLAG;              \
	}
#define RRC() \
	{                                               \
		C_FLAG = (A) & 0x01;                    \
		(A) = ((A) >> 1) | (C_FLAG << 7);       \
	}
#define RAL() \
	{                                               \
		work8_2 = (((A) & 0x80) != 0);          \
		(A) = ((A) << 1) | C_FLAG;              \
		C_FLAG = work8_2;                       \
	}
#define RAR() \
	{                                               \
		work8_2 = (A) & 0x01;                   \
		(A) = ((A) >> 1) | (C_FLAG << 7);       \
		C_FLAG = work8_2;                       \
	}
#define IN() \
        {                                               \
                (A) = in_byte((port));                  \
        }
#define CALL \
        {                                               \
                PUSH(PC + 2);                           \
                PC = RD_WORD(PC);                       \
        }
#define RST(addr) \
        {                                               \
                PUSH(PC);                               \
                PC = (addr);                            \
        }

#define PARITY(reg)             parityTbl[(reg)]

#endif
