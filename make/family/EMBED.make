# This allows us to build Espruino into a single, embeddable C file for use
# in other projects. It doesn't contain any of the hardware access code, just the basic
# JS interpreter. See boards/EMBED.py

CFLAGS += -std=gnu99
DEFINES += -DESPR_EMBED=1
INCLUDE += -I$(ROOT)/targets/embed
SOURCES +=                              \
targets/embed/main.c
LIBS += -lpthread # thread lib for input processing
LIBS += -lstdc++

