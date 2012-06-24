CC=gcc
CFLAGS=-c -g -Wall -Wextra -Wconversion 
LDFLAGS=-g

ifdef ARM
CC=arm-none-eabi-gcc
CFLAGS=-c -Wall -Wextra -Wconversion -Os
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
TinyJSC.c


OBJECTS=$(SOURCES:.c=.o)

all: TinyJSC

TinyJSC: $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f TinyJSC $(OBJECTS)
