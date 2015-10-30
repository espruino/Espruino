#
# This file is part of Espruino, a JavaScript interpreter for Microcontrollers
#
# Copyright (C) 2015 Gordon Williams <gw@pur3.co.uk>
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# ----------------------------------------------------------------------------
# This file is designed to be parsed during the build process
#
# Contains ESP8266 board specific functions.
# ----------------------------------------------------------------------------
#
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