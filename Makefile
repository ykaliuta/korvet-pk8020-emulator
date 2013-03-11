CC       = gcc.exe 
LIBS     = -mwindows -lalleg -liberty
BIN      = a.exe
CFLAGS   = -O7 -MMD -fomit-frame-pointer -mcpu=pentium3 -funroll-loops -ISRC
#CFLAGS   = -O7 -MMD -fomit-frame-pointer 
#CFLAGS   = -O7 -MMD -mcpu=pentium
#CFLAGS   = -MMD -g -Wall
sources = _main.c \
          vg.c \
          floppy.c \
          i8080.c \
          keyboard.c memory.c   \
          pic.c ppi.c printer.c screen.c  \
          serial.c timer.c      \
          wav.c gui.c osd.c \
	  mouse.c joystick.c \
          dbg/dbg.c dbg/_dasm.c dbg/dasm80.c dbg/_dump.c dbg/_regs.c dbg/_history.c dbg/dbg_tools.c dbg/scremul.c dbg/kfonts.c \
          dbg/label.c dbg/asm80.c dbg/readwrite.c dbg/sym.c dbg/lbl_korvet.c dbg/comname.c \
          dbg/gt_main.c


objs1	= $(patsubst %.c,%.o,$(sources))
objs2	= $(patsubst %.s,%.o,$(objs1))
objs	= $(addprefix OBJS/,$(objs2))

VPATH	= SRC 

.PHONY: all clean depend

all:    kdbg.exe

clean: 
	rm -f OBJS/dbg/*
	rm -f OBJS/*
	rm kdbg.exe

OBJS/%.o:	%.c
	gcc $(CFLAGS) -c -o $@ $<

OBJS/%.o:	%.s
	gcc $(CFLAGS) -c -o $@ $<


kdbg.exe:	$(objs)
	@gcc $^ -o $@ $(LIBS)

include $(wildcard OBJS/*.d)
include $(wildcard OBJS/DBG/*.d)
