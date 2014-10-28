CC       = gcc.exe 
#LIBS     = -mwindows -lallegro-4.4.2-mt

LIBS     = -lalleg
CFLAGS   = -MMD -ggdb -Isrc
#CFLAGS   = -O3 -g -MMD -fomit-frame-pointer -funroll-loops -Isrc
#CFLAGS   = -O -MMD -funroll-loops -Isrc
#LIBS     = -lalleg -pg -g 
#CFLAGS   = -pg -g -fno-omit-frame-pointer -MMD -Isrc

sources = _main.c \
			tools.c \
			vg.c \
			floppy.c \
			i8080.c \
			keyboard.c memory.c   \
			pic.c ppi.c printer.c screen.c  \
			serial.c \
			lan.c \
			timer.c \
			wav.c gui.c osd.c \
			mouse.c joystick.c \
			ext_rom.c \
			dbg/dbg.c dbg/_dasm.c dbg/dasm80.c dbg/_dump.c dbg/_regs.c dbg/_history.c dbg/dbg_tools.c dbg/scremul.c dbg/kfonts.c \
			dbg/label.c dbg/asm80.c dbg/readwrite.c dbg/sym.c dbg/lbl_korvet.c dbg/comname.c \
			dbg/gt_main.c


objs1	= $(patsubst %.c,%.o,$(sources))
objs2	= $(patsubst %.s,%.o,$(objs1))
objs	= $(addprefix objs/,$(objs2))

VPATH	= src 

.PHONY: all clean depend

# all:    kdbg.exe
all:    kdbg

clean: 
	mkdir -p objs/dbg
	rm -r objs/*.o
	rm -r objs/*.d
	rm -r objs/dbg/*.o
	rm -r objs/dbg/*.d
	mkdir -p objs/dbg
	rm kdbg

objs/%.o:	%.c
	gcc $(CFLAGS) -c -o $@ $<

objs/%.o:	%.s
	gcc $(CFLAGS) -c -o $@ $<


kdbg.exe:	$(objs)
	@gcc $^ -o $@ $(LIBS)

kdbg:	$(objs)
	@gcc $^ -o $@ $(LIBS)


include $(wildcard OBJS/*.d)
include $(wildcard OBJS/DBG/*.d)
