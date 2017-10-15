DBG=yes
SOUND=yes
LAN_SUPPORT=yes
EGA=no
WAV=no
TRACETIMER=no

ifneq ($(DBG),no)
CFLAGS += -DDBG
endif

ifneq ($(SOUND),no)
CFLAGS += -DSOUND
endif

ifneq ($(LAN_SUPPORT),no)
CFLAGS += -DLAN_SUPPORT
endif

ifeq ($(EGA),yes)
CFLAGS += -DEGA
endif

ifeq ($(WAV),yes)
CFLAGS += -DWAV
endif

ifeq ($(TRACETIMER),yes)
CFLAGS += -DTRACETIMER
endif

