CC=gcc
DEBUGFLAGS=
#DEBUGFLAGS=-pg
#gprof ./TinyJSC > gprof.txt
DEBUGFLAGS=-g -O0
CFLAGS=-c $(DEBUGFLAGS) -Wall -Wextra -Wconversion 
LDFLAGS=$(DEBUGFLAGS)
LIBS=

ifdef ARM
CC=arm-none-eabi-gcc
CFLAGS=-c -Wall -Wextra -Wconversion -Os
else
CFLAGS+=-DUSE_MATH
LIBS+=-lm
endif

ifdef SDCC
CC=sdcc
CFLAGS=-c --std-sdcc99 -DUSE_FLOATS 
# SDCC is defined as a compiler directive
endif

SOURCES=  \
jslex.c \
jsparse.c \
jsutils.c \
jsvar.c \
jsfunctions.c \
jsinteractive.c \
jshardware.c \
TinyJSC.c


OBJECTS=$(SOURCES:.c=.o)

all: TinyJSC

TinyJSC: $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@ $(LIBS)

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f TinyJSC $(OBJECTS)
