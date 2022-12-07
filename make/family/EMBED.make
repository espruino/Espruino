ESPR_EMBED=1
USE_DEBUGGER=0

CFLAGS += -std=gnu99
DEFINES += -DESPR_EMBED=1
INCLUDE += -I$(ROOT)/targets/embed
SOURCES +=                              \
targets/embed/main.c
LIBS += -lpthread # thread lib for input processing
LIBS += -lstdc++

