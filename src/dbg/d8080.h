/*
 * AUTHOR: Sergey Erokhin                 esl@pisem.net,pk8020@gmail.com
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
char *Mnemo80[256]={"nop  ","lxi  ","stax ","inx  ","inr  ","dcr  ","mvi  ","rlc  ","nop  ","dad  ","ldax ","dcx  ","inr  ","dcr  ","mvi  ","rrc  ",
                    "nop  ","lxi  ","stax ","inx  ","inr  ","dcr  ","mvi  ","ral  ","nop  ","dad  ","ldax ","dcx  ","inr  ","dcr  ","mvi  ","rar  ",
                    "nop  ","lxi  ","shld ","inx  ","inr  ","dcr  ","mvi  ","daa  ","nop  ","dad  ","lhld ","dcx  ","inr  ","dcr  ","mvi  ","cma  ",
                    "nop  ","lxi  ","sta  ","inx  ","inr  ","dcr  ","mvi  ","stc  ","nop  ","dad  ","lda  ","dcx  ","inr  ","dcr  ","mvi  ","cmc  ",
                    "mov  ","mov  ","mov  ","mov  ","mov  ","mov  ","mov  ","mov  ","mov  ","mov  ","mov  ","mov  ","mov  ","mov  ","mov  ","mov  ",
                    "mov  ","mov  ","mov  ","mov  ","mov  ","mov  ","mov  ","mov  ","mov  ","mov  ","mov  ","mov  ","mov  ","mov  ","mov  ","mov  ",
                    "mov  ","mov  ","mov  ","mov  ","mov  ","mov  ","mov  ","mov  ","mov  ","mov  ","mov  ","mov  ","mov  ","mov  ","mov  ","mov  ",
                    "mov  ","mov  ","mov  ","mov  ","mov  ","mov  ","hlt  ","mov  ","mov  ","mov  ","mov  ","mov  ","mov  ","mov  ","mov  ","mov  ",
                    "add  ","add  ","add  ","add  ","add  ","add  ","add  ","add  ","adc  ","adc  ","adc  ","adc  ","adc  ","adc  ","adc  ","adc  ",
                    "sub  ","sub  ","sub  ","sub  ","sub  ","sub  ","sub  ","sub  ","sbb  ","sbb  ","sbb  ","sbb  ","sbb  ","sbb  ","sbb  ","sbb  ",
                    "ana  ","ana  ","ana  ","ana  ","ana  ","ana  ","ana  ","ana  ","xra  ","xra  ","xra  ","xra  ","xra  ","xra  ","xra  ","xra  ",
                    "ora  ","ora  ","ora  ","ora  ","ora  ","ora  ","ora  ","ora  ","cmp  ","cmp  ","cmp  ","cmp  ","cmp  ","cmp  ","cmp  ","cmp  ",
                    "rnz  ","pop  ","jnz  ","jmp  ","cnz  ","push ","adi  ","rst  ","rz   ","ret  ","jz   ","jmp  ","cz   ","call ","aci  ","rst  ",
                    "rnc  ","pop  ","jnc  ","out  ","cnc  ","push ","sui  ","rst  ","rc   ","ret  ","jc   ","in   ","cc   ","call ","sbi  ","rst  ",
                    "rpo  ","pop  ","jpo  ","xthl ","cpo  ","push ","ani  ","rst  ","rpe  ","pchl ","jpe  ","xchg ","cpe  ","call ","xri  ","rst  ",
                    "rp   ","pop  ","jp   ","di   ","cp   ","push ","ori  ","rst  ","rm   ","sphl ","jm   ","ei   ","cm   ","call ","cpi  ","rst  ",
                    };
char *Oper80[256]={"","b,","b","b","b","b","b,","","","b","b","b","c","c","c,","",
                   "","d,","d","d","d","d","d,","","","d","d","d","e","e","e,","",
                   "","h,","","h","h","h","h,","","","h","","h","l","l","l,","",
                   "","sp,","","sp","m","m","m,","","","sp","","sp","a","a","a,","",
                   "b,b","b,c","b,d","b,e","b,h","b,l","b,m","b,a","c,b","c,c","c,d","c,e","c,h","c,l","c,m","c,a",
                   "d,b","d,c","d,d","d,e","d,h","d,l","d,m","d,a","e,b","e,c","e,d","e,e","e,h","e,l","e,m","e,a",
                   "h,b","h,c","h,d","h,e","h,h","h,l","h,m","h,a","l,b","l,c","l,d","l,e","l,h","l,l","l,m","l,a",
                   "m,b","m,c","m,d","m,e","m,h","m,l","","m,a","a,b","a,c","a,d","a,e","a,h","a,l","a,m","a,a",
                   "b","c","d","e","h","l","m","a","b","c","d","e","h","l","m","a",
                   "b","c","d","e","h","l","m","a","b","c","d","e","h","l","m","a",
                   "b","c","d","e","h","l","m","a","b","c","d","e","h","l","m","a",
                   "b","c","d","e","h","l","m","a","b","c","d","e","h","l","m","a",
                   "","b","","","","b","","0","","","","","","","","1",
                   "","d","","","","d","","2","","","","","","","","3",
                   "","h","","","","h","","4","","","","","","","","5",
                   "","psw","","","","psw","","6","","","","","","","","7",
                   };

// 0 -NOP, 1 - db aa ; 'a', 2 - Call xxxx, 3 - Invalid opcode
byte Toper80[256]={
//                 0 1 2 3 4 5 6 7 8 9 A B C D E F
                   0,2,0,0,0,0,1,0,0,0,0,0,0,0,1,0, //00
                   0,2,0,0,0,0,1,0,0,0,0,0,0,0,1,0, //10
                   0,2,2,0,0,0,1,0,0,0,2,0,0,0,1,0, //20
                   0,2,2,0,0,0,1,0,0,0,2,0,0,0,1,0, //30
                   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //40
                   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //50
                   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //60
                   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //70
                   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //80
                   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //90
                   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //A0
                   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //B0
                   0,0,2,2,2,0,1,0,0,0,2,2,2,2,1,0, //C0
                   0,0,2,1,2,0,1,0,0,0,2,1,2,2,1,0, //D0
                   0,0,2,0,2,0,1,0,0,0,2,0,2,2,1,0, //E0
                   0,0,2,0,2,0,1,0,0,0,2,0,2,2,1,0, //F0
                   };
