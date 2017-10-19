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

#include "dbg.h"
void showHELP(int dbgMODE);
void help_CLS(int color);

char *_help_common[64]= {
    "Common - HELP",
    "    ",
    "    F7        - one STEP in",
    "    F8        - step one instruction (set breakpoint to next cmd after current)",
    "    F9        - Exit from DBG (run)",
    "    F10       - Toggle Color/BlackWhile palette",
    "    ",
    "    Ctrl+F12  - exit Emulator",
    "    ",
    "    Ctrl+UP   - move to UPPER window",
    "    Ctrl+Down - move to LOWER window",
    "    ",
    "    Ctrl+L    - redraw screen",
    "    ",
    "    F5        - toggle preview window LUT to/from DBG LUT",
    "    Ctrl+F5   - toggle DBG LUT type (all color default LUT/show ACZU only)",
    "    ",
    "    Ctrl+P    - DASM addr=PC",
    "    ",
    "    Alt+B     - Dump addr=BC",
    "    Alt+D     - Dump addr=DE",
    "    Alt+H     - Dump addr=HL",
    "    Alt+P     - Dump addr=PC",
    "    Alt+S     - Dump addr=SP",
    "    ",
    "    Alt+W     - Write memory to file",
    "    Alt+R     - Read memory from file",
    "    ",
    "    Ctrl+Y    - Read sYm (files with labels) from file",
    "    Alt+Y     - Write sYm to file",
    "    ",
    "    Z",
    "    Ctrl+Z    - Activate GameTool",
    "    ",
};

char *_help_REGISTERS[64]={
    "-== REGISTERS window ==-",
    "",
    "    Enter           - edit any registers value",
    "    Enter           - for words after SP value set DASM window to addr from stack",
    "    Enter/SPACE     - toggle flag in FLAGS field",
    "    SPACE           - for BC,DE,HL toggle \"dump\" type ",
    "                        4 byte from memory  (00 00 00 00)",
    "                        8 chars from memory (\"12345678\")",
    "                        2 char value        ('AB')",
    "",
    "",
    "",
    "",
    "",
    "",
};

char *_help_DASM[64]={
    "-== DASM window ==-",
    "",
    "    Ctrl+Left/Right - shift DUMP windows to Addr-1/+1",
    "    F2              - set/reset BreakPoint",
    "    F4              - run and STOP on addr under cursor (Run to HERE) (set Breakpoint to current addr)",
    "    Ctrl+K          - Kill command (replace command by NOP) (work only for RAM)",
    "    Ctrl+N          - PC = current addr (set new PC)",
    "    Enter           - start edit field",
    "    LABEL Area ----------------",
    "    if label exists - set addr to label, if not exist - add new label",
    "    Del             - remove label"
    "",
    "",
    "",
    "",
};

char *_help_DUMP[64]={
    "-== DUMP window ==-",
    "",
    "    Ctrl+Left/Right - shift DUMP windows to Addr-1/+1",
    "    F2              - set/reset BreakPoint for Read/Write access",
    "    Shift+F2        - set/reset BreakPoint for Read access",
    "    Ctrl+F2         - set/reset BreakPoint for Write access",
    "    LABEL Area ----------------",
    "    if label exists - set addr to label, if not exist - add new label",
    "    Del             - remove label",
    "    HEX/ASCII -----------------",
    "    Tab             - switch HEX <-> ASCII window",
    "",
    "",
};

char *_help_GAMETOOL[64]={
    "-== GAMETOOL window ==-",
    "",
    "    Allow find values related to LIFE/ENERGY/etc for game cheating  ... ",
    "    ",
    "    1. Start new GT session",
    "    2. Select Byte/Word mode",
    "    2. Change value in game",
    "    3. Start GT again and select corresponding 'change type'",
    "    4. Repeat step 3 while value will be found or restart search",
    "",
    "",
    "",
    "",
};

char **_help_idx[MAXDBG+1]={
    _help_REGISTERS,
    _help_DASM,
    _help_DUMP,
    _help_GAMETOOL,
};


int _HELP(int dbgMODE) {
    showHELP(dbgMODE);
    readkey();
    help_CLS(C_Default);
    tDoUpdate();

    return -1;
}

void showHELP(int dbgMODE) {
    int i;

    tFontSelect(PC_FONT);

    help_CLS(C_HELP);
    tSetUpdate(0);
    for (i=0;i<11;i++) {
        tScreenPutString(_help_idx[dbgMODE][i],C_HELP,0,i);
    }

    // tScreenPutString("----------------------------------------------------------------------------",C_HELP,0,11);
    draw_hline(0,11,102,C_HELP);
    for (i=0;i<34;i++) {
        tScreenPutString(_help_common[i],C_HELP,0,12+i);
    }
    tSetUpdate(1);
    tDoUpdate();
}


void help_CLS(int color) {
    int i;
    tSetUpdate(0);
    for (i=0;i<36+12;i++) {
        tScreenPutString("-------------------------------------------------------------------------------------------------------",color,0,i);
        tScreenPutString("                                                                                                       ",color,0,i);
    }
    tSetUpdate(1);
}
