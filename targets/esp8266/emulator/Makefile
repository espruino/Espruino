# Build the Espruino Emulator for the ESP8266

ifndef ESP8266_SDK_ROOT
$(error, "ESP8266_SDK_ROOT must be defined!")
endif
$(info, "ESP8266_SDK_ROOT=$(ESP8266_SDK_ROOT)")

SOURCES=\
esp8266_stub.c \
esp8266_stub_sockets.c

CCPREFIX=

export CC=$(CCPREFIX)gcc
export LD=$(CCPREFIX)gcc
export AR=$(CCPREFIX)ar
export AS=$(CCPREFIX)as
export OBJCOPY=$(CCPREFIX)objcopy
export OBJDUMP=$(CCPREFIX)objdump
export GDB=$(CCPREFIX)gdb

INCLUDE = -I. \
-I$(ESP8266_SDK_ROOT)/include

SOURCEOBJS = $(SOURCES:.c=.o)

all: $(SOURCEOBJS)

.c.o:
	$(CC) $(INCLUDE) -c $<

clean:
	@echo "Cleaning ..."
	find . -name '*.o' | xargs rm -f 