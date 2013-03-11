#ifndef _I8080DIS_H
#define _I8080DIS_H

int disasm(char *text, unsigned int offset);
int get_instr_length(unsigned int offset);
int is_jmp_ret_pchl(unsigned char opcode);
int is_call(unsigned char opcode);

#endif
