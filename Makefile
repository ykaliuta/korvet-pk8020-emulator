
LDLIBS = $(shell pkg-config allegro --libs)
LDLIBS += -lpthread
CFLAGS = -g -O0 -Isrc
CFLAGS += -Wall -Wno-deprecated-declarations

# can update CFLAGS
-include config.mk

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
			ext_rom.c

ifneq ($(DBG),no)
sources += dbg/dbg.c dbg/_dasm.c dbg/dasm80.c \
			dbg/_dump.c dbg/_regs.c dbg/_help.c \
			dbg/_history.c dbg/dbg_tools.c dbg/scremul.c dbg/kfonts.c \
			dbg/label.c dbg/asm80.c dbg/readwrite.c dbg/sym.c dbg/lbl_korvet.c dbg/comname.c \
			dbg/gt_main.c
endif

sources += host-config.c libconfini/confini.c
sources += host.c host-events.c queue.c host-threads.c host-timers.c
sources += host-graphics.c

ifneq ($(SOUND),no)
sources += host-sound.c
endif

objs1	= $(patsubst %.c,%.o,$(sources))
objs	= $(addprefix objs/,$(objs1))

VPATH	= src 

.PHONY: all clean depend check

all:    kdbg

check:
	cppcheck  -j 4 --inconclusive --enable=all -Isrc src/*.c src/*.h  2>!cppcheck-errors.txt
	echo please check !cppcheck-errors.txt

clean: 
	rm -rf objs
	rm -f kdbg

objs objs/dbg objs/libconfini:
	mkdir -p $@

objs/libconfini/confini.o: | objs/libconfini

objs/%.o:	%.c | objs objs/dbg
	$(CC) $(CFLAGS) -c -o $@ $<


kdbg:	$(objs)
	$(CC) $^ -o $@ $(LDLIBS)

-include $(wildcard objs/*.d)
-include $(wildcard objs/dbg/*.d)
