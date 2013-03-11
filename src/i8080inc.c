/*
 * &Korvet Team                                              2000...2005
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
/*
2003-04-01
некотоpыми пеpифеpийными устpойствами по гpаницам машинной команды.  Реали-
зована поддеpжка недокументиpованных команд КР580ВМ80А:
      08h, 10h, 18h, 20h, 28h, 30h, 38h  -  NOP
      0CBh                               -  JMP
      0D9h                               -  RET
      0DDh, 0EDh, 0FDh                   -  CALL
*/

	switch(op) {
		case 0x00:			/*              nop */
		case 0x08:			/* undocumented nop */
		case 0x10:			/* undocumented nop */
		case 0x18:			/* undocumented nop */
		case 0x20:			/* undocumented nop */
		case 0x28:			/* undocumented nop */
                case 0x30:			/* undocumented nop */
		case 0x38:			/* undocumented nop */
			Tstates += 4;
			break;

		case 0x01:			/* lxi bc, data16 */
			Tstates += 10;
			BC = RD_WORD(PC);
			PC += 2;
			break;

		case 0x02:                      /* stax b */
			Tstates += 7;
			WR_BYTE(BC, A);
			break;

		case 0x03:			/* inx b */
			Tstates += 5;
			BC++;
			break;

		case 0x04:			/* inr b */
			Tstates += 5;
			INR(B);
			break;

		case 0x05:           		/* dcr b */
			Tstates += 5;
			DCR(B);
			break;

		case 0x06:                      /* mvi b, data8 */
			Tstates += 7;
			B = RD_BYTE(PC++);
			break;

		case 0x07:                      /* rlc */
			Tstates += 4;
			C_FLAG = ((A & 0x80) != 0);
			A = (A << 1) | C_FLAG;
			break;
/*
		case 0x08:
			invalid_opcode();
                        break;
*/

		case 0x09:			/* dad b */
			Tstates += 10;
			DAD(BC);
			break;

		case 0x0A:                      /* ldax b */
			Tstates += 7;
			A = RD_BYTE(BC);
			break;

		case 0x0B:                      /* dcx b */
			Tstates += 5;
			BC--;
			break;

		case 0x0C:			/* inr c */
			Tstates += 5;
			INR(C);
			break;

		case 0x0D: 			/* dcr c */
			Tstates += 5;
			DCR(C);
			break;

		case 0x0E:			/* mvi c, data8 */
			Tstates += 7;
			C = RD_BYTE(PC++);
			break;

		case 0x0F:                      /* rrc */
			Tstates += 4;
			C_FLAG = A & 0x01;
			A = (A >> 1) | (C_FLAG << 7);
			break;
/*
		case 0x10:
			invalid_opcode();
			break;
*/
		case 0x11:			/* lxi de, data16 */
			Tstates += 10;
			DE = RD_WORD(PC);
			PC += 2;
			break;

		case 0x12:                      /* stax d */
			Tstates += 7;
			WR_BYTE(DE, A);
			break;

		case 0x13:			/* inx d */
			Tstates += 5;
			DE++;
			break;

		case 0x14:			/* inr d */
			Tstates += 5;
			INR(D);
			break;

		case 0x15:           		/* dcr d */
			Tstates += 5;
			DCR(D);
			break;

		case 0x16:                      /* mvi d, data8 */
			Tstates += 7;
			D = RD_BYTE(PC++);
			break;

		case 0x17:			/* ral */
			Tstates += 4;
			work8 = (uns8)C_FLAG;
			C_FLAG = ((A & 0x80) != 0);
			A = (A << 1) | work8;
			break;
/*
		case 0x18:
			invalid_opcode();
			break;
*/
		case 0x19:			/* dad d */
			Tstates += 10;
			DAD(DE);
			break;

		case 0x1A:                      /* ldax d */
			Tstates += 7;
			A = RD_BYTE(DE);
			break;

		case 0x1B:                      /* dcx d */
			Tstates += 5;
			DE--;
			break;

		case 0x1C:			/* inr e */
			Tstates += 5;
			INR(E);
			break;

		case 0x1D: 			/* dcr e */
			Tstates += 5;
			DCR(E);
			break;

		case 0x1E:			/* mvi e, data8 */
			Tstates += 7;
			E = RD_BYTE(PC++);
			break;

		case 0x1F:			/* rar */
			Tstates += 4;
			work8 = (uns8)C_FLAG;
			C_FLAG = A & 0x01;
			A = (A >> 1) | (work8 << 7);
			break;
/*
		case 0x20:
			invalid_opcode();
			break;
*/
		case 0x21: 			/* lxi hl, data16 */
			Tstates += 10;
			HL = RD_WORD(PC);
			PC += 2;
			break;

		case 0x22:			/* shld addr */
			Tstates += 16;
			WR_WORD(RD_WORD(PC), HL);
			PC += 2;
			break;

		case 0x23:          		/* inx h */
			Tstates += 5;
			HL++;
			break;

		case 0x24:          		/* inr h */
			Tstates += 5;
			INR(H);
			break;

		case 0x25:          		/* dcr h */
			Tstates += 5;
			DCR(H);
			break;

		case 0x26:                      /* mvi h, data8 */
			Tstates += 7;
			H = RD_BYTE(PC++);
			break;

		case 0x27:			/* daa */
			Tstates += 4;
			carry = (uns8)C_FLAG;
			if (!(sub = (uns8)N_FLAG))
			{
				add = 0;
				if (H_FLAG || (A & 0x0f) > 9)
				{
					add = 0x06;
				}
				if (C_FLAG || (A >> 4) > 9 || ((A >> 4) >= 9 && (A & 0x0f) > 9))
				{
					add |= 0x60;
					carry = 1;
				}
			}
			else
			{
				if (C_FLAG)
				{
					add = TST(H_FLAG) ? 0x9a : 0xa0;
				}
				else
				{
					add = TST(H_FLAG) ? 0xfa : 0x00;
				}
			}
			ADD(add);
			N_FLAG = sub;
			P_FLAG = PARITY(A);
			C_FLAG = carry;
			break;
/*
		case 0x28:
			invalid_opcode();
			break;
*/
		case 0x29:			/* dad hl */
			Tstates += 10;
			DAD(HL);
			break;

		case 0x2A:			/* ldhl addr */
			Tstates += 16;
			HL = RD_WORD(RD_WORD(PC));
			PC += 2;
			break;

		case 0x2B:			/* dcx h */
			Tstates += 5;
			HL--;
			break;

		case 0x2C:			/* inr l */
			Tstates += 5;
			INR(L);
			break;

		case 0x2D:			/* dcr l */
			Tstates += 5;
			DCR(L);
			break;

		case 0x2E:           		/* mvi l, data8 */
			Tstates += 7;
			L = RD_BYTE(PC++);
			break;

		case 0x2F:			/* cma */
			Tstates += 4;
			A ^= 0xff;
			SET(N_FLAG);
			break;
/*
		case 0x30:
			invalid_opcode();
			break;
*/
		case 0x31:               	/* lxi sp, data16 */
			Tstates += 10;
			SP = RD_WORD(PC);
			PC += 2;
			break;

		case 0x32:                      /* sta addr */
			Tstates += 13;
			WR_BYTE(RD_WORD(PC), A);
			PC += 2;
			break;

		case 0x33:          		/* inx sp */
			Tstates += 5;
			SP++;
			break;

		case 0x34:                      /* inr m */
			Tstates += 10;
			work8 = RD_BYTE(HL);
			INR(work8);
			WR_BYTE(HL, work8);
			break;

		case 0x35: 			/* dcr m */
			Tstates += 10;
			work8 = RD_BYTE(HL);
			DCR(work8);
			WR_BYTE(HL, work8);
			break;

		case 0x36:			/* mvi m, data8 */
			Tstates += 10;
			WR_BYTE(HL, RD_BYTE(PC++));
			break;

		case 0x37:			/* stc */
			Tstates += 4;
			CLR(H_FLAG);
			CLR(N_FLAG);
			SET(C_FLAG);
			break;
/*
		case 0x38:
			invalid_opcode();
			break;
*/
		case 0x39:           		/* dad sp */
			Tstates += 10;
			DAD(SP);
			break;

		case 0x3A: 			/* lda addr */
			Tstates += 13;
			A = RD_BYTE(RD_WORD(PC));
			PC += 2;
			break;

		case 0x3B:          		/* dcx sp */
			Tstates += 5;
			SP--;
			break;

		case 0x3C: 			/* inr a */
			Tstates += 5;
			INR(A);
			break;

		case 0x3D: 			/* dcr a */
			Tstates += 5;
			DCR(A);
			break;

		case 0x3E:          		/* mvi a, data8 */
			Tstates += 7;
			A = RD_BYTE(PC++);
			break;

		case 0x3F:			/* cmc */
			Tstates += 4;
			CLR(N_FLAG);
			CPL(C_FLAG);
			break;

		case 0x40:			/* mov b, b */
			Tstates += 4;
			break;

		case 0x41:
			Tstates += 5;
			B = C;
			break;

		case 0x42:
			Tstates += 5;
			B = D;
			break;

		case 0x43:
			Tstates += 5;
			B = E;
			break;

		case 0x44:
			Tstates += 5;
			B = H;
			break;

		case 0x45:
			Tstates += 5;
			B = L;
			break;

		case 0x46:
			Tstates += 7;
			B = RD_BYTE(HL);
			break;

		case 0x47:
			Tstates += 5;
			B = A;
			break;

		case 0x48:
			Tstates += 5;
			C = B;
			break;

		case 0x49:
			Tstates += 5;
			break;

		case 0x4A:
			Tstates += 5;
			C = D;
			break;

		case 0x4B:
			Tstates += 5;
			C = E;
			break;

		case 0x4C:
			Tstates += 5;
			C = H;
			break;

		case 0x4D:
			Tstates += 5;
			C = L;
			break;

		case 0x4E:
			Tstates += 7;
			C = RD_BYTE(HL);
			break;

		case 0x4F:
			Tstates += 5;
			C = A;
			break;

		case 0x50:
			Tstates += 5;
			D = B;
			break;

		case 0x51:
			Tstates += 5;
			D = C;
			break;

		case 0x52:
			Tstates += 5;
			break;

		case 0x53:
			Tstates += 5;
			D = E;
			break;

		case 0x54:
			Tstates += 5;
			D = H;
			break;

		case 0x55:
			Tstates += 5;
			D = L;
			break;

		case 0x56:
			Tstates += 7;
			D = RD_BYTE(HL);
			break;

		case 0x57:
			Tstates += 5;
			D = A;
			break;

		case 0x58:
			Tstates += 5;
			E = B;
			break;

		case 0x59:
			Tstates += 5;
			E = C;
			break;

		case 0x5A:
			Tstates += 5;
			E = D;
			break;

		case 0x5B:
			Tstates += 5;
			break;

		case 0x5C:
			Tstates += 5;
			E = H;
			break;

		case 0x5D:
			Tstates += 5;
			E = L;
			break;

		case 0x5E:
			Tstates += 7;
			E = RD_BYTE(HL);
			break;

		case 0x5F:
			Tstates += 5;
			E = A;
			break;

		case 0x60:
			Tstates += 5;
			H = B;
			break;

		case 0x61:
			Tstates += 5;
			H = C;
			break;

		case 0x62:
			Tstates += 5;
			H = D;
			break;

		case 0x63:
			Tstates += 5;
			H = E;
			break;

		case 0x64:
			Tstates += 5;
			break;

		case 0x65:
			Tstates += 5;
			H = L;
			break;

		case 0x66:
			Tstates += 7;
			H = RD_BYTE(HL);
			break;

		case 0x67:
			Tstates += 5;
			H = A;
			break;

		case 0x68:
			Tstates += 5;
			L = B;
			break;

		case 0x69:
			Tstates += 5;
			L = C;
			break;

		case 0x6A:
			Tstates += 5;
			L = D;
			break;

		case 0x6B:
			Tstates += 5;
			L = E;
			break;

		case 0x6C:
			Tstates += 5;
			L = H;
			break;

		case 0x6D:
			Tstates += 5;
			break;

		case 0x6E:
			Tstates += 7;
			L = RD_BYTE(HL);
			break;

		case 0x6F:
			Tstates += 5;
			L = A;
			break;

		case 0x70:
			Tstates += 7;
			WR_BYTE(HL, B);
			break;

		case 0x71:
			Tstates += 7;
			WR_BYTE(HL, C);
			break;

		case 0x72:
			Tstates += 7;
			WR_BYTE(HL, D);
			break;

		case 0x73:
			Tstates += 7;
			WR_BYTE(HL, E);
			break;

		case 0x74:
			Tstates += 7;
			WR_BYTE(HL, H);
			break;

		case 0x75:
			Tstates += 7;
			WR_BYTE(HL, L);
			break;

		case 0x76:			//Halt
			Tstates += 4;
			PC--;
			break;

		case 0x77:
			Tstates += 7;
			WR_BYTE(HL, A);
			break;

		case 0x78:
			Tstates += 5;
			A = B;
			break;

		case 0x79:
			Tstates += 5;
			A = C;
			break;

		case 0x7A:
			Tstates += 5;
			A = D;
			break;

		case 0x7B:
			Tstates += 5;
			A = E;
			break;

		case 0x7C:
			Tstates += 5;
			A = H;
			break;

		case 0x7D:
			Tstates += 5;
			A = L;
			break;

		case 0x7E:
			Tstates += 7;
			A = RD_BYTE(HL);
			break;

		case 0x7F:
			Tstates += 5;
			break;

		case 0x80:
			Tstates += 4;
			ADD(B);
			break;

		case 0x81:
			Tstates += 4;
			ADD(C);
			break;

		case 0x82:
			Tstates += 4;
			ADD(D);
			break;

		case 0x83:
			Tstates += 4;
			ADD(E);
			break;

		case 0x84:
			Tstates += 4;
			ADD(H);
			break;

		case 0x85:
			Tstates += 4;
			ADD(L);
			break;

		case 0x86:
			Tstates += 7;
			work8 = RD_BYTE(HL);
			ADD(work8);
			break;

		case 0x87:
			Tstates += 4;
			ADD(A);
			break;

		case 0x88:
			Tstates += 4;
			ADC(B);
			break;

		case 0x89:
			Tstates += 4;
			ADC(C);
			break;

		case 0x8A:
			Tstates += 4;
			ADC(D);
			break;

		case 0x8B:
			Tstates += 4;
			ADC(E);
			break;

		case 0x8C:
			Tstates += 4;
			ADC(H);
			break;

		case 0x8D:
			Tstates += 4;
			ADC(L);
			break;

		case 0x8E:
			Tstates += 7;
			work8 = RD_BYTE(HL);
			ADC(work8);
			break;

		case 0x8F:
			Tstates += 4;
			ADC(A);
			break;

		case 0x90:
			Tstates += 4;
			SUB(B);
			break;

		case 0x91:
			Tstates += 4;
			SUB(C);
			break;

		case 0x92:
			Tstates += 4;
			SUB(D);
			break;

		case 0x93:
			Tstates += 4;
			SUB(E);
			break;

		case 0x94:
			Tstates += 4;
			SUB(H);
			break;

		case 0x95:
			Tstates += 4;
			SUB(L);
			break;

		case 0x96:
			Tstates += 7;
			work8 = RD_BYTE(HL);
			SUB(work8);
			break;

		case 0x97:
			Tstates += 7;
			A = 0;
			SET(Z_FLAG);
			CLR(S_FLAG);
			CLR(H_FLAG);
			SET(N_FLAG);
			CLR(C_FLAG);
			break;

		case 0x98:
			Tstates += 4;
			SBB(B);
			break;

		case 0x99:
			Tstates += 4;
			SBB(C);
			break;

		case 0x9A:
			Tstates += 4;
			SBB(D);
			break;

		case 0x9B:
			Tstates += 4;
			SBB(E);
			break;

		case 0x9C:
			Tstates += 4;
			SBB(H);
			break;

		case 0x9D:
			Tstates += 4;
			SBB(L);
			break;

		case 0x9E:
			Tstates += 7;
			work8 = RD_BYTE(HL);
			SBB(work8);
			break;

		case 0x9F:
			Tstates += 4;
			SBB(A);
			break;

		case 0xA0:
			Tstates += 4;
			ANA(B);
			break;

		case 0xA1:
			Tstates += 4;
			ANA(C);
			break;

		case 0xA2:
			Tstates += 4;
			ANA(D);
			break;

		case 0xA3:
			Tstates += 4;
			ANA(E);
			break;

		case 0xA4:
			Tstates += 4;
			ANA(H);
			break;

		case 0xA5:
			Tstates += 4;
			ANA(L);
			break;

		case 0xA6:
			Tstates += 7;
			work8 = RD_BYTE(HL);
			ANA(work8);
			break;

		case 0xA7:
			Tstates += 4;
			ANA(A);
			break;

		case 0xA8:
			Tstates += 4;
			XRA(B);
			break;

		case 0xA9:
			Tstates += 4;
			XRA(C);
			break;

		case 0xAA:
			Tstates += 4;
			XRA(D);
			break;

		case 0xAB:
			Tstates += 4;
			XRA(E);
			break;

		case 0xAC:
			Tstates += 4;
			XRA(H);
			break;

		case 0xAD:
			Tstates += 4;
			XRA(L);
			break;

		case 0xAE:
			Tstates += 7;
			work8 = RD_BYTE(HL);
			XRA(work8);
			break;

		case 0xAF:
			Tstates += 4;
			A = 0;
			SET(Z_FLAG);
			CLR(S_FLAG);
			CLR(H_FLAG);
			SET(P_FLAG);
			CLR(N_FLAG);
			CLR(C_FLAG);
			break;

		case 0xB0:
			Tstates += 4;
			ORA(B);
			break;

		case 0xB1:
			Tstates += 4;
			ORA(C);
			break;

		case 0xB2:
			Tstates += 4;
			ORA(D);
			break;

		case 0xB3:
			Tstates += 4;
			ORA(E);
			break;

		case 0xB4:
			Tstates += 4;
			ORA(H);
			break;

		case 0xB5:
			Tstates += 4;
			ORA(L);
			break;

		case 0xB6:
			Tstates += 7;
			work8 = RD_BYTE(HL);
			ORA(work8);
			break;

		case 0xB7:
			Tstates += 4;
			ORA(A);
			break;

		case 0xB8:
			Tstates += 4;
			CMP(B);
			break;

		case 0xB9:
			Tstates += 4;
				CMP(C);
			break;

		case 0xBA:
			Tstates += 4;
			CMP(D);
			break;

		case 0xBB:
			Tstates += 4;
			CMP(E);
			break;

		case 0xBC:
			Tstates += 4;
			CMP(H);
			break;

		case 0xBD:
			Tstates += 4;
			CMP(L);
			break;

		case 0xBE:
			Tstates += 7;
			work8 = RD_BYTE(HL);
			CMP(work8);
			break;

		case 0xBF:
			Tstates += 4;
			SET(Z_FLAG);
			CLR(S_FLAG);
			CLR(H_FLAG);
			SET(N_FLAG);
			CLR(C_FLAG);
			break;

		case 0xC0:			/* rnz */
			Tstates += 5;
			if (!TST(Z_FLAG))
			{
				Tstates += 11;
				POP(PC);
			}
			break;

		case 0xC1:           		/* pop b */
			Tstates += 11;
			POP(BC);
			break;

		case 0xC2:			/* jnz addr */
			Tstates += 10;
			if (!TST(Z_FLAG))
			{
				PC = RD_WORD(PC);
			}
			else
			{
				PC += 2;
			}
			break;

		case 0xC3:			/*              jmp addr */
                case 0xcb:                      /* undocumented jmp addr */
			Tstates += 10;
			PC = RD_WORD(PC);
			break;

		case 0xC4:			/* cnz addr */
			if (!TST(Z_FLAG))
			{
				Tstates += 17;
				CALL;
			}
			else
			{
				Tstates += 11;
				PC += 2;
			}
			break;

		case 0xC5:			/* push b */
			Tstates += 11;
			PUSH(BC);
			break;

		case 0xC6:			/* adi data8 */
			Tstates += 7;
			work8 = RD_BYTE(PC++);
			ADD(work8);
			break;

		case 0xC7:			/* rst 0 */
			Tstates += 11;
			RST(0x0000);
			break;

		case 0xC8:			/* rz */
			Tstates += 5;
			if (TST(Z_FLAG))
			{
				Tstates += 11;
				POP(PC);
			}
			break;

		case 0xC9:			/*              ret */
                case 0xd9:                      /* undocumented ret */
			Tstates += 10;
			POP(PC);
			break;

		case 0xCA:			/* jz addr */
			Tstates += 10;
			if (TST(Z_FLAG))
			{
				PC = RD_WORD(PC);
			}
			else
			{
				PC += 2;
			}
			break;
/*
		case 0xCB:
			invalid_opcode();
			break;
*/
		case 0xCC:			/* cz addr */
			if (TST(Z_FLAG))
			{
				Tstates += 17;
				CALL;
			}
			else
			{
				Tstates += 11;
				PC += 2;
			}
			break;

		case 0xCD:			/*              call addr */
		case 0xDD:			/* undocumented call addr */
		case 0xED:			/* undocumented call addr */
		case 0xFD:			/* undocumented call addr */

			Tstates += 17;
			CALL;
			break;

		case 0xCE:			/* aci data8 */
			Tstates += 7;
			work8 = RD_BYTE(PC++);
			ADC(work8);
			break;

		case 0xCF:			/* rst 1 */
			Tstates += 11;
			RST(0x0008);
			break;

		case 0xD0:			/* rnc */
			Tstates += 5;
			if (!TST(C_FLAG))
			{
				Tstates += 11;
				POP(PC);
			}
			break;

		case 0xD1:			/* pop d */
			Tstates += 11;
			POP(DE);
			break;

		case 0xD2:			/* jnc addr */
			Tstates += 10;
			if (!TST(C_FLAG))
			{
				PC = RD_WORD(PC);
			}
			else
			{
				PC += 2;
			}
			break;

		case 0xD3:			/* out port8 */
			Tstates += 10;
			out_byte(RD_BYTE(PC++));
			break;

		case 0xD4:			/* cnc addr */
			if (!TST(C_FLAG))
			{
				Tstates += 17;
				CALL;
			}
			else
			{
				Tstates += 11;
				PC += 2;
			}
			break;

		case 0xD5:			/* push d */
			Tstates += 11;
			PUSH(DE);
			break;

		case 0xD6:			/* sui data8 */
			Tstates += 7;
			work8 = RD_BYTE(PC++);
			SUB(work8);
			break;

		case 0xD7:			/* rst 2 */
			Tstates += 11;
			RST(0x0010);
			break;

		case 0xD8:			/* rc */
			Tstates += 5;
			if (TST(C_FLAG))
			{
				Tstates += 11;
				POP(PC);
			}
			break;
/*
		case 0xD9:
			invalid_opcode();
			break;
*/
		case 0xDA:			/* jc addr */
			Tstates += 10;
			if (TST(C_FLAG))
			{
				PC = RD_WORD(PC);
			}
			else
			{
				PC += 2;
			}
			break;

		case 0xDB:			/* in port8 */
			Tstates += 10;
			A = in_byte( RD_BYTE(PC++));
			break;

		case 0xDC:			/* cc addr */
			if (TST(C_FLAG))
			{
				Tstates += 17;
				CALL;
			}
			else
			{
				Tstates += 11;
				PC += 2;
			}
			break;
/*
		case 0xDD:
			invalid_opcode();
			break;
*/
		case 0xDE:			/* sbi data8 */
			Tstates += 7;
			work8 = RD_BYTE(PC++);
			SBB(work8);
			break;

		case 0xDF:			/* rst 3 */
			Tstates += 11;
			RST(0x0018);
			break;

		case 0xE0:			/* rpo */
			Tstates += 5;
			if (!TST(P_FLAG))
			{
				Tstates += 11;
				POP(PC);
			}
			break;

		case 0xE1:			/* pop h */
			Tstates += 11;
			POP(HL);
			break;

		case 0xE2:			/* jpo addr */
			Tstates += 10;
			if (!TST(P_FLAG))
			{
				PC = RD_WORD(PC);
			}
			else
			{
				PC += 2;
			}
			break;

		case 0xE3:			/* xthl */
			Tstates += 18;
			work16 = RD_WORD(SP);
			WR_WORD(SP, HL);
			HL = work16;
			break;

		case 0xE4:			/* cpo addr */
			if (!TST(P_FLAG))
			{
				Tstates += 17;
				CALL;
			}
			else
			{
				Tstates += 11;
				PC += 2;
			}
			break;

		case 0xE5:			/* push h */
			Tstates += 11;
			PUSH(HL);
			break;

		case 0xE6:			/* ani data8 */
			Tstates += 7;
			work8 = RD_BYTE(PC++);
			ANA(work8);
			break;

		case 0xE7:			/* rst 4 */
			Tstates += 11;
			RST(0x0020);
			break;

		case 0xE8:			/* rpe */
			Tstates += 5;
			if (TST(P_FLAG))
			{
				Tstates += 11;
				POP(PC);
			}
			break;

		case 0xE9:			/* pchl */
			Tstates += 5;   	/* (?!) may be 3 */
			PC = HL;
			break;

		case 0xEA:			/* jpe addr */
			Tstates += 10;
			if (TST(P_FLAG))
			{
				PC = RD_WORD(PC);
			}
			else
			{
				PC += 2;
			}
			break;

		case 0xEB:			/* xchg */
			Tstates += 4;
			work16 = DE, DE = HL, HL = work16;
			break;

		case 0xEC:			/* cpe addr */
			if (TST(P_FLAG))
			{
				Tstates += 17;
				CALL;
			}
			else
			{
				Tstates += 11;
				PC += 2;
			}
			break;
/*
		case 0xED:
			invalid_opcode();
			break;
*/

		case 0xEE:			/* xri data8 */
			Tstates += 7;
			work8 = RD_BYTE(PC++);
			XRA(work8);
			break;

		case 0xEF:			/* rst 5 */
			Tstates += 11;
			RST(0x0028);
			break;

		case 0xF0:			/* rp */
			Tstates += 5;
			if (!TST(S_FLAG))
			{
				Tstates += 11;
				POP(PC);
			}
			break;

		case 0xF1:			/* pop psw */
			Tstates += 10;
			POP(AF);
			I8080_RetrieveFlags();
			break;

		case 0xF2:			/* jp addr */
			Tstates += 10;
			if (!TST(S_FLAG))
			{
				PC = RD_WORD(PC);
			}
			else
			{
				PC += 2;
			}
			break;

		case 0xF3:			/* di */
//			sound_off();
			Tstates += 4;
			IFF = 0;
			break;

		case 0xF4:			/* cp addr */
			if (!TST(S_FLAG))
			{
				Tstates += 17;
				CALL;
			}
			else
			{
				Tstates += 11;
				PC += 2;
			}
			break;

		case 0xF5:			/* push psw */
			Tstates += 11;
			I8080_StoreFlags();
			PUSH(AF);
			break;

		case 0xF6:			/* ori data8 */
			Tstates += 7;
			work8 = RD_BYTE(PC++);
			ORA(work8);
			break;

		case 0xF7:			/* rst 6 */
			Tstates += 11;
			RST(0x0030);
			break;

		case 0xF8:			/* rm */
			Tstates += 5;
			if (TST(S_FLAG))
			{
				Tstates += 11;
				POP(PC);
			}
			break;

		case 0xF9:			/* sphl */
			Tstates += 5;
			SP = HL;
			break;

		case 0xFA:			/* jm addr */
			Tstates += 10;
			if (TST(S_FLAG))
			{
				PC = RD_WORD(PC);
			}
			else
			{
				PC += 2;
			}
			break;

		case 0xFB:			/* ei */
//			sound_on();
			Tstates += 4;
 			IFF = 1;
                        IntDelay=1;
//                        if (IntREQ) DoINT();
			break;

		case 0xFC:			/* cm addr */
			if (TST(S_FLAG))
			{
				Tstates += 17;
				CALL;
			}
			else
			{
				Tstates += 11;
				PC += 2;
			}
			break;
/*
		case 0xFD:
			invalid_opcode();
			break;
*/

		case 0xFE:			/* cpi data8 */
			Tstates += 7;
			work8  = RD_BYTE(PC++);
			CMP(work8);
			break;

		case 0xFF:			/* rst 7 */
			Tstates += 11;
			RST(0x0038);
			break;
	}
