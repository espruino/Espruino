CC=gcc
CFLAGS=-c -g -Wall -Wextra -Wconversion
LDFLAGS=-g

SOURCES=  \
jslex.c \
jsparse.c \
jsutils.c \
jsvar.c \
TinyJSC.c


OBJECTS=$(SOURCES:.c=.o)

all: TinyJSC

TinyJSC: $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f TinyJSC $(OBJECTS)
