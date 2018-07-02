# This file is part of Espruino, a JavaScript interpreter for Microcontrollers
#
# Copyright (C) 2013 Gordon Williams <gw@pur3.co.uk>
# Copyright (C) 2014 Alain Sézille for NucleoF401RE, NucleoF411RE specific lines of this file
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# -----------------------------------------------------------------------------
# Makefile for Espruino
# -----------------------------------------------------------------------------
# Set the BOARD environment variable to one of the names of the .py file in
# the `boards` directory. Eg. PICO, PUCKJS, ESPRUINOWIFI, etc
#
# make               # make whatever the default binary is
# make flash         # Try and flash using the platform's normal flash tool
# make serialflash   # flash over USB serial bootloader (STM32)
# make lst           # Make listing files
#
# Also:
#
# DEBUG=1                 # add debug symbols (-g)
# RELEASE=1               # Force release-style compile (no asserts, etc)
# SINGLETHREAD=1          # Compile single-threaded to make compilation errors easier to find
# BOOTLOADER=1            # make the bootloader (not Espruino)
# PROFILE=1               # Compile with gprof profiling info
# CFILE=test.c            # Compile in the supplied C file
# CPPFILE=test.cpp        # Compile in the supplied C++ file
#
# WIZNET=1                # If compiling for a non-linux target that has internet support, use WIZnet W5500 support
#   W5100=1               # Compile for WIZnet W5100 (not W5500)
# CC3000=1                # If compiling for a non-linux target that has internet support, use CC3000 support
# USB_PRODUCT_ID=0x1234   # force a specific USB Product ID (default 0x5740)
#
# GENDIR=MyGenDir		  # sets directory for files generated during make
#					      # GENDIR=/home/mydir/mygendir
# SETDEFINES=FileDefines  # settings which are called after definitions for board are done
#                         # SETDEFINES=/home/mydir/myDefines
# UNSUPPORTEDMAKE=FileUnsu# Adds additional files from unsupported sources(means not supported by Gordon) to actual make
#                         # UNSUPPORTEDMAKE=/home/mydir/unsupportedCommands
# PROJECTNAME=myBigProject# Sets projectname
# BLACKLIST=fileBlacklist # Removes javascript commands given in a file from compilation and therefore from project defined firmware
#                         # is used in build_jswrapper.py
#                         # BLACKLIST=/home/mydir/myBlackList
# VARIABLES=1700          # Sets number of variables for project defined firmware. This parameter can be dangerous, be careful before changing.
#                         # used in build_platform_config.py
# NO_COMPILE=1            # skips compiling and linking part, used to echo WRAPPERSOURCES only
# RTOS=1                  # adds RTOS functions, available only for ESP32 (yet)
# DFU_UPDATE_BUILD=1      # Uncomment this to build Espruino for a device firmware update over the air (nRF52).
# PAD_FOR_BOOTLOADER=1    # When building for Espruino STM32 boards, pad the binary out with 0xFF where the bootloader should be (allows the Web IDE to flash the binary)

include make/sanitycheck.make

ifndef GENDIR
GENDIR=gen
endif

ifndef SINGLETHREAD
MAKEFLAGS=-j5 # multicore
endif

INCLUDE?=-I$(ROOT) -I$(ROOT)/targets -I$(ROOT)/src -I$(GENDIR)
LIBS?=
DEFINES?=
CFLAGS?=-Wall -Wextra -Wconversion -Werror=implicit-function-declaration -fno-strict-aliasing -g
LDFLAGS?=-Winline -g
OPTIMIZEFLAGS?=
#-fdiagnostics-show-option - shows which flags can be used with -Werror
DEFINES+=-DGIT_COMMIT=$(shell git log -1 --format="%h")

ifeq ($(shell uname),Darwin)
MACOSX=1
CFLAGS+=-D__MACOSX__
STAT_FLAGS='-f ''%z'''
else
STAT_FLAGS='-c ''%s'''
endif

ifeq ($(OS),Windows_NT)
MINGW=1
endif

ifdef RELEASE
# force no asserts to be compiled in
DEFINES += -DNO_ASSERT -DRELEASE
endif

ifndef ALT_RELEASE
# Default release labeling.  (This may fail and give inconsistent results due to the fact that
# travis does a shallow clone.)
LATEST_RELEASE=$(shell git tag | grep RELEASE_ | sort | tail -1)
# use egrep to count lines instead of wc to avoid whitespace error on Mac
COMMITS_SINCE_RELEASE=$(shell git log --oneline $(LATEST_RELEASE)..HEAD | egrep -c .)
ifneq ($(COMMITS_SINCE_RELEASE),0)
DEFINES += -DBUILDNUMBER=\"$(COMMITS_SINCE_RELEASE)\"
endif

else
# Alternate release labeling, which works nicely in travis and allows other developers to put their
# initials into the build number.
# The release label is constructed by appending the value of ALT_RELEASE followed by the branch
# name as build number instead of commit info. For example, you can set ALT_RELEASE=peter and
# then your builds for branch "experiment" come out with a version like
# v1.81.peter_experiment_83bd432, where the last letters are the short of the current commit SHA.
# Warning: this same release label derivation is also in scripts/common.py in get_version()
LATEST_RELEASE=$(shell egrep "define JS_VERSION .*\"$$" src/jsutils.h | egrep -o '[0-9]v[0-9]+')
COMMITS_SINCE_RELEASE=$(ALT_RELEASE)_$(subst -,_,$(shell git name-rev --name-only HEAD))_$(shell git rev-parse --short HEAD)
# Figure out whether we're building a tagged commit (true release) or not
TAGGED:=$(shell if git describe --tags --exact-match >/dev/null 2>&1; then echo yes; fi)
ifeq ($(TAGGED),yes)
$(info %%%%% Release label: $(LATEST_RELEASE))
else
DEFINES += -DBUILDNUMBER=\"$(COMMITS_SINCE_RELEASE)\"
$(info %%%%% Build label: $(LATEST_RELEASE).$(COMMITS_SINCE_RELEASE))
endif

endif

CWD = $(CURDIR)
ROOT = $(CWD)
PRECOMPILED_OBJS=
PLATFORM_CONFIG_FILE=$(GENDIR)/platform_config.h
WRAPPERFILE=$(GENDIR)/jswrapper.c
HEADERFILENAME=$(GENDIR)/platform_config.h
BASEADDRESS=0x08000000


ifeq ($(BOARD),)
 # Try and guess board names
 ifneq ($(shell grep Raspbian /etc/os-release),)
  BOARD=RASPBERRYPI # just a guess
 else ifeq ($(shell uname -n),beaglebone)
  BOARD=BEAGLEBONE
 else ifeq ($(shell uname -n),arietta)
  BOARD=ARIETTA
 else
  #$(info *************************************************************)
  #$(info *           To build, use BOARD=my_board make               *)
  #$(info *************************************************************)
  BOARD=LINUX
  DEFINES+=-DSYSFS_GPIO_DIR="\"/sys/class/gpio\""
 endif
endif

ifeq ($(BOARD),RASPBERRYPI)
 ifneq ("$(wildcard /usr/local/include/wiringPi.h)","")
 USE_WIRINGPI=1
 else
 DEFINES+=-DSYSFS_GPIO_DIR="\"/sys/class/gpio\""
 $(info *************************************************************)
 $(info *  WIRINGPI NOT FOUND, and you probably want it             *)
 $(info *  see  http://wiringpi.com/download-and-install/           *)
 $(info *************************************************************)
 endif
endif

# ---------------------------------------------------------------------------------
#                                                      Get info out of BOARDNAME.py
# ---------------------------------------------------------------------------------
# TODO: could check board here and make clean if it's different?
$(shell rm -f CURRENT_BOARD.make)
$(shell python scripts/get_makefile_decls.py $(BOARD) > CURRENT_BOARD.make)
include CURRENT_BOARD.make

#set or reset defines like USE_GRAPHIC from an external file to customize firmware
ifdef SETDEFINES
include $(SETDEFINES)
endif

# ----------------------------- end of board defines ------------------------------
# ---------------------------------------------------------------------------------


# ---------------------------------------------------------------------------------


# If we're not on Linux and we want internet, we need either CC3000 or WIZnet support
ifdef USE_NET
ifndef LINUX
ifdef WIZNET
USE_WIZNET=1
ifdef W5100
USE_WIZNET_W5100=1
endif
else ifeq ($(FAMILY),ESP8266)
USE_ESP8266=1
else ifeq ($(FAMILY),ESP32)
USE_ESP32=1
else ifdef EMW3165
USE_WICED=1
else ifdef CC3000
USE_CC3000=1
endif
endif
endif

ifdef DEBUG
#OPTIMIZEFLAGS=-Os -g
 ifeq ($(FAMILY),ESP8266)
  OPTIMIZEFLAGS=-g -Os -std=gnu11 -fgnu89-inline -Wl,--allow-multiple-definition
 else
  OPTIMIZEFLAGS=-g
 endif
 ifeq ($(FAMILY),EFM32GG)
  DEFINES += -DDEBUG_EFM=1 -DDEBUG=1
 endif
DEFINES+=-DDEBUG
endif

ifdef PROFILE
OPTIMIZEFLAGS+=-pg
endif

# These are files for platform-specific libraries
TARGETSOURCES ?=

# These are JS files to be included as pre-built Espruino modules
JSMODULESOURCES ?=

# Files that contains objects/functions/methods that will be
# exported to JS. The order here actually determines the order
# objects will be matched in. So for example Pins must come
# above ints, since a Pin is also matched as an int.
WRAPPERSOURCES += \
src/jswrap_array.c \
src/jswrap_arraybuffer.c \
src/jswrap_dataview.c \
src/jswrap_date.c \
src/jswrap_error.c \
src/jswrap_espruino.c \
src/jswrap_flash.c \
src/jswrap_functions.c \
src/jswrap_interactive.c \
src/jswrap_io.c \
src/jswrap_json.c \
src/jswrap_modules.c \
src/jswrap_pin.c \
src/jswrap_number.c \
src/jswrap_object.c \
src/jswrap_onewire.c \
src/jswrap_pipe.c \
src/jswrap_process.c \
src/jswrap_promise.c \
src/jswrap_regexp.c \
src/jswrap_serial.c \
src/jswrap_storage.c \
src/jswrap_spi_i2c.c \
src/jswrap_stream.c \
src/jswrap_string.c \
src/jswrap_waveform.c \

# it is important that _pin comes before stuff which uses
# integers (as the check for int *includes* the chek for pin)
SOURCES += \
src/jslex.c \
src/jsflags.c \
src/jsflash.c \
src/jsvar.c \
src/jsvariterator.c \
src/jsutils.c \
src/jsnative.c \
src/jsparse.c \
src/jspin.c \
src/jsinteractive.c \
src/jsdevices.c \
src/jstimer.c \
src/jsi2c.c \
src/jsspi.c \
src/jshardware_common.c \
$(WRAPPERFILE)
CPPSOURCES =

ifdef CFILE
WRAPPERSOURCES += $(CFILE)
endif
ifdef CPPFILE
CPPSOURCES += $(CPPFILE)
endif

ifdef USB_PRODUCT_ID
DEFINES+=-DUSB_PRODUCT_ID=$(USB_PRODUCT_ID)
endif

ifdef SAVE_ON_FLASH
DEFINES+=-DSAVE_ON_FLASH

# Smaller, RLE compression for code
INCLUDE += -I$(ROOT)/libs/compression -I$(ROOT)/libs/compression
SOURCES += \
libs/compression/compress_rle.c

else

ifneq ($(FAMILY),ESP8266)
# If we have enough flash, include the debugger
# ESP8266 can't do it because it expects tasks to finish within set time
DEFINES+=-DUSE_DEBUGGER
endif
# Use use tab complete
DEFINES+=-DUSE_TAB_COMPLETE

# Heatshrink compression library and wrapper - better compression when saving code to flash
DEFINES+=-DUSE_HEATSHRINK
INCLUDE += -I$(ROOT)/libs/compression -I$(ROOT)/libs/compression/heatshrink
SOURCES += \
libs/compression/heatshrink/heatshrink_encoder.c \
libs/compression/heatshrink/heatshrink_decoder.c \
libs/compression/compress_heatshrink.c

endif

ifndef BOOTLOADER # ------------------------------------------------------------------------------ DON'T USE IN BOOTLOADER

ifdef USE_FILESYSTEM
DEFINES += -DUSE_FILESYSTEM
INCLUDE += -I$(ROOT)/libs/filesystem
WRAPPERSOURCES += \
libs/filesystem/jswrap_fs.c \
libs/filesystem/jswrap_file.c
ifndef LINUX
INCLUDE += -I$(ROOT)/libs/filesystem/fat_sd
SOURCES += \
libs/filesystem/fat_sd/fattime.c \
libs/filesystem/fat_sd/ff.c \
libs/filesystem/fat_sd/option/unicode.c # for LFN support (see _USE_LFN in ff.h)

ifdef USE_FILESYSTEM_SDIO
DEFINES += -DUSE_FILESYSTEM_SDIO
SOURCES += \
libs/filesystem/fat_sd/sdio_diskio.c \
libs/filesystem/fat_sd/sdio_sdcard.c
else #USE_FILESYSTEM_SDIO
ifdef USE_FLASHFS
DEFINES += -DUSE_FLASHFS
SOURCES += \
libs/filesystem/fat_sd/flash_diskio.c
else
SOURCES += \
libs/filesystem/fat_sd/spi_diskio.c
endif #USE_FLASHFS
endif #USE_FILESYSTEM_SDIO
endif #!LINUX
endif #USE_FILESYSTEM

DEFINES += -DUSE_MATH
INCLUDE += -I$(ROOT)/libs/math
WRAPPERSOURCES += libs/math/jswrap_math.c
ifeq ($(FAMILY),ESP8266)
# special ESP8266 maths lib that doesn't go into RAM
LIBS += -lmirom
LDFLAGS += -L$(ROOT)/targets/esp8266
else
# everything else uses normal maths lib
LIBS += -lm
endif

ifdef USE_GRAPHICS
DEFINES += -DUSE_GRAPHICS
INCLUDE += -I$(ROOT)/libs/graphics
WRAPPERSOURCES += libs/graphics/jswrap_graphics.c
SOURCES += \
libs/graphics/bitmap_font_4x6.c \
libs/graphics/graphics.c \
libs/graphics/lcd_arraybuffer.c \
libs/graphics/lcd_js.c

ifdef USE_LCD_SDL
  DEFINES += -DUSE_LCD_SDL
  SOURCES += libs/graphics/lcd_sdl.c
  LIBS += -lSDL
  INCLUDE += -I/usr/include/SDL
endif

ifdef USE_LCD_FSMC
  DEFINES += -DUSE_LCD_FSMC
  SOURCES += libs/graphics/lcd_fsmc.c
endif

ifdef USE_TERMINAL
  DEFINES += -DUSE_TERMINAL
  WRAPPERSOURCES += libs/graphics/jswrap_terminal.c
endif

endif

ifdef USE_USB_HID
  DEFINES += -DUSE_USB_HID
endif

ifdef USE_NET
 DEFINES += -DUSE_NET
 INCLUDE += -I$(ROOT)/libs/network -I$(ROOT)/libs/network -I$(ROOT)/libs/network/http
 WRAPPERSOURCES += \
 libs/network/jswrap_net.c \
 libs/network/http/jswrap_http.c
 SOURCES += \
 libs/network/network.c \
 libs/network/socketserver.c \
 libs/network/socketerrors.c

 WRAPPERSOURCES += libs/network/js/jswrap_jsnetwork.c
 INCLUDE += -I$(ROOT)/libs/network/js
 SOURCES += \
 libs/network/js/network_js.c

 ifdef LINUX
 INCLUDE += -I$(ROOT)/libs/network/linux
 SOURCES += \
 libs/network/linux/network_linux.c
 endif

 ifdef USE_CC3000
 DEFINES += -DUSE_CC3000 -DSEND_NON_BLOCKING
 WRAPPERSOURCES += libs/network/cc3000/jswrap_cc3000.c
 INCLUDE += -I$(ROOT)/libs/network/cc3000
 SOURCES += \
 libs/network/cc3000/network_cc3000.c \
 libs/network/cc3000/board_spi.c \
 libs/network/cc3000/cc3000_common.c \
 libs/network/cc3000/evnt_handler.c \
 libs/network/cc3000/hci.c \
 libs/network/cc3000/netapp.c \
 libs/network/cc3000/nvmem.c \
 libs/network/cc3000/security.c \
 libs/network/cc3000/socket.c \
 libs/network/cc3000/wlan.c
 endif

 ifdef USE_WIZNET
 DEFINES += -DUSE_WIZNET
 WRAPPERSOURCES += libs/network/wiznet/jswrap_wiznet.c
 INCLUDE += -I$(ROOT)/libs/network/wiznet -I$(ROOT)/libs/network/wiznet/Ethernet
 SOURCES += \
 libs/network/wiznet/network_wiznet.c \
 libs/network/wiznet/DNS/dns_parse.c \
 libs/network/wiznet/DNS/dns.c \
 libs/network/wiznet/DHCP/dhcp.c \
 libs/network/wiznet/Ethernet/wizchip_conf.c \
 libs/network/wiznet/Ethernet/socket.c
  ifdef USE_WIZNET_W5100
   DEFINES += -D_WIZCHIP_=5100
   SOURCES += libs/network/wiznet/W5100/w5100.c
  else
   DEFINES += -D_WIZCHIP_=5500
   SOURCES += libs/network/wiznet/W5500/w5500.c
  endif
 endif

 ifdef USE_WICED
 # For EMW3165 use SDIO to access BCN43362 rev A2
 INCLUDE += -I$(ROOT)/targetlibs/wiced/include \
            -I$(ROOT)/targetlibs/wiced/wwd/include \
            -I$(ROOT)/targetlibs/wiced/wwd/include/network \
            -I$(ROOT)/targetlibs/wiced/wwd/include/RTOS \
            -I$(ROOT)/targetlibs/wiced/wwd/internal/bus_protocols/SDIO \
            -I$(ROOT)/targetlibs/wiced/wwd/internal/chips/43362A2

 SOURCES += targetlibs/wiced/wwd/internal/wwd_thread.c \
            targetlibs/wiced/wwd/internal/wwd_sdpcm.c \
            targetlibs/wiced/wwd/internal/wwd_internal.c \
            targetlibs/wiced/wwd/internal/wwd_management.c \
            targetlibs/wiced/wwd/internal/wwd_wifi.c \
            targetlibs/wiced/wwd/internal/wwd_crypto.c \
            targetlibs/wiced/wwd/internal/wwd_logging.c \
            targetlibs/wiced/wwd/internal/wwd_eapol.c \
            targetlibs/wiced/wwd/internal/bus_protocols/wwd_bus_common.c \
            targetlibs/wiced/wwd/internal/bus_protocols/SDIO/wwd_bus_protocol.c
 endif

 ifdef USE_ESP32
 DEFINES += -DUSE_ESP32
 WRAPPERSOURCES += \
   libs/network/jswrap_wifi.c \
   libs/network/esp32/jswrap_esp32_network.c \
   targets/esp32/jswrap_esp32.c
 INCLUDE += -I$(ROOT)/libs/network/esp32
 SOURCES +=  libs/network/esp32/network_esp32.c \
  targets/esp32/jshardwareI2c.c \
  targets/esp32/jshardwareSpi.c \
  targets/esp32/jshardwareUart.c \
  targets/esp32/jshardwareAnalog.c \
  targets/esp32/jshardwarePWM.c \
  targets/esp32/rtosutil.c \
  targets/esp32/jshardwareTimer.c \
  targets/esp32/jshardwarePulse.c
  ifdef RTOS
   DEFINES += -DRTOS
   WRAPPERSOURCES += targets/esp32/jswrap_rtos.c
  endif # RTOS
 endif # USE_ESP32

 ifdef USE_ESP8266
 DEFINES += -DUSE_ESP8266
 WRAPPERSOURCES += \
   libs/network/jswrap_wifi.c \
   libs/network/esp8266/jswrap_esp8266_network.c \
   targets/esp8266/jswrap_esp8266.c \
   targets/esp8266/jswrap_nodemcu.c
 INCLUDE += -I$(ROOT)/libs/network/esp8266
 SOURCES += \
 libs/network/esp8266/network_esp8266.c\
 libs/network/esp8266/pktbuf.c\
 libs/network/esp8266/ota.c
 endif

 ifdef USE_TELNET
  DEFINES += -DUSE_TELNET
  WRAPPERSOURCES += libs/network/telnet/jswrap_telnet.c
  INCLUDE += -I$(ROOT)/libs/network/telnet
 endif
endif # USE_NET

ifdef USE_TV
  DEFINES += -DUSE_TV
  WRAPPERSOURCES += libs/tv/jswrap_tv.c
  INCLUDE += -I$(ROOT)/libs/tv
  SOURCES += \
  libs/tv/tv.c
endif

ifdef USE_TRIGGER
  DEFINES += -DUSE_TRIGGER
  WRAPPERSOURCES += libs/trigger/jswrap_trigger.c
  INCLUDE += -I$(ROOT)/libs/trigger
  SOURCES += \
  libs/trigger/trigger.c
endif

ifdef USE_HASHLIB
  INCLUDE += -I$(ROOT)/libs/hashlib
  WRAPPERSOURCES += \
  libs/hashlib/jswrap_hashlib.c
  SOURCES += \
  libs/hashlib/sha2.c
endif

ifdef USE_WIRINGPI
  DEFINES += -DUSE_WIRINGPI
  LIBS += -lwiringPi
  INCLUDE += -I/usr/local/include -L/usr/local/lib
endif

ifdef USE_BLUETOOTH
  DEFINES += -DBLUETOOTH
  INCLUDE += -I$(ROOT)/libs/bluetooth
  WRAPPERSOURCES += libs/bluetooth/jswrap_bluetooth.c
  SOURCES += libs/bluetooth/bluetooth_utils.c
endif

ifdef USE_CRYPTO
  DEFINES += -DUSE_CRYPTO
  INCLUDE += -I$(ROOT)/libs/crypto
  INCLUDE += -I$(ROOT)/libs/crypto/mbedtls
  INCLUDE += -I$(ROOT)/libs/crypto/mbedtls/include
  WRAPPERSOURCES += libs/crypto/jswrap_crypto.c
  SOURCES += libs/crypto/mbedtls/library/sha1.c 
ifdef USE_SHA256
  DEFINES += -DUSE_SHA256
  SOURCES += libs/crypto/mbedtls/library/sha256.c
endif
ifdef USE_SHA512
  DEFINES += -DUSE_SHA512
  SOURCES += libs/crypto/mbedtls/library/sha512.c
endif

ifdef USE_TLS
  USE_AES=1
  DEFINES += -DUSE_TLS
  SOURCES += \
libs/crypto/mbedtls/library/bignum.c \
libs/crypto/mbedtls/library/ctr_drbg.c \
libs/crypto/mbedtls/library/debug.c \
libs/crypto/mbedtls/library/ecp.c \
libs/crypto/mbedtls/library/ecp_curves.c \
libs/crypto/mbedtls/library/entropy.c \
libs/crypto/mbedtls/library/entropy_poll.c \
libs/crypto/mbedtls/library/md5.c \
libs/crypto/mbedtls/library/pk.c \
libs/crypto/mbedtls/library/pkparse.c \
libs/crypto/mbedtls/library/pk_wrap.c \
libs/crypto/mbedtls/library/rsa.c \
libs/crypto/mbedtls/library/ssl_ciphersuites.c \
libs/crypto/mbedtls/library/ssl_cli.c \
libs/crypto/mbedtls/library/ssl_tls.c \
libs/crypto/mbedtls/library/ssl_srv.c \
libs/crypto/mbedtls/library/x509.c \
libs/crypto/mbedtls/library/x509_crt.c
endif
ifdef USE_AES
  DEFINES += -DUSE_AES
  SOURCES += \
libs/crypto/mbedtls/library/aes.c \
libs/crypto/mbedtls/library/asn1parse.c \
libs/crypto/mbedtls/library/cipher.c \
libs/crypto/mbedtls/library/cipher_wrap.c \
libs/crypto/mbedtls/library/md.c \
libs/crypto/mbedtls/library/md_wrap.c \
libs/crypto/mbedtls/library/oid.c \
libs/crypto/mbedtls/library/pkcs5.c
endif
endif

ifdef USE_NEOPIXEL
  DEFINES += -DUSE_NEOPIXEL
  INCLUDE += -I$(ROOT)/libs/neopixel
  WRAPPERSOURCES += libs/neopixel/jswrap_neopixel.c
endif

ifdef USE_NFC
  DEFINES += -DUSE_NFC -DNFC_HAL_ENABLED=1
  INCLUDE          += -I$(NRF5X_SDK_PATH)/components/nfc/t2t_lib
  INCLUDE          += -I$(NRF5X_SDK_PATH)/components/nfc/ndef/uri
  INCLUDE          += -I$(NRF5X_SDK_PATH)/components/nfc/ndef/generic/message
  INCLUDE          += -I$(NRF5X_SDK_PATH)/components/nfc/ndef/generic/record
  TARGETSOURCES    += $(NRF5X_SDK_PATH)/components/nfc/ndef/uri/nfc_uri_msg.c
  TARGETSOURCES    += $(NRF5X_SDK_PATH)/components/nfc/ndef/uri/nfc_uri_rec.c
  TARGETSOURCES    += $(NRF5X_SDK_PATH)/components/nfc/ndef/generic/message/nfc_ndef_msg.c
  TARGETSOURCES    += $(NRF5X_SDK_PATH)/components/nfc/ndef/generic/record/nfc_ndef_record.c
  TARGETSOURCES    += $(NRF5X_SDK_PATH)/components/nfc/t2t_lib/hal_t2t/hal_nfc_t2t.c
endif

ifdef USE_WIO_LTE
  INCLUDE += -I$(ROOT)/libs/wio_lte
  WRAPPERSOURCES += libs/wio_lte/jswrap_wio_lte.c
  SOURCES += targets/stm32/stm32_ws2812b_driver.c
endif


endif # BOOTLOADER ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ DON'T USE STUFF ABOVE IN BOOTLOADER

# =========================================================================

.PHONY:  proj

all: 	 proj

# =========================================================================
ifneq ($(FAMILY),)
include make/family/$(FAMILY).make
endif
# =========================================================================

ifdef USB
DEFINES += -DUSB
endif

PININFOFILE=$(GENDIR)/jspininfo
SOURCES += $(PININFOFILE).c

SOURCES += $(WRAPPERSOURCES) $(TARGETSOURCES)
SOURCEOBJS = $(SOURCES:.c=.o) $(CPPSOURCES:.cpp=.o)
OBJS = $(SOURCEOBJS) $(PRECOMPILED_OBJS)


# -ffreestanding -nodefaultlibs -nostdlib -fno-common
# -nodefaultlibs -nostdlib -nostartfiles

# -fdata-sections -ffunction-sections are to help remove unused code
CFLAGS += $(OPTIMIZEFLAGS) -c $(ARCHFLAGS) $(DEFINES) $(INCLUDE)

# -Wl,--gc-sections helps remove unused code
# -Wl,--whole-archive checks for duplicates
# --specs=nano.specs uses newlib-nano
ifdef NRF5X
 LDFLAGS += $(OPTIMIZEFLAGS) $(ARCHFLAGS) --specs=nano.specs -lc -lnosys
else ifdef STM32
 LDFLAGS += $(OPTIMIZEFLAGS) $(ARCHFLAGS) --specs=nano.specs -lc -lnosys
else ifdef EFM32
 LDFLAGS += $(OPTIMIZEFLAGS) $(ARCHFLAGS)
 LDFLAGS += -Wl,--start-group -lgcc -lc -lnosys -Wl,--end-group
else
 LDFLAGS += $(OPTIMIZEFLAGS) $(ARCHFLAGS)
endif

ifdef EMBEDDED
DEFINES += -DEMBEDDED
LDFLAGS += -Wl,--gc-sections
endif

ifdef LINKER_FILE
  LDFLAGS += -T$(LINKER_FILE)
endif

# Adds additional files from unsupported sources(means not supported by Gordon) to actual make
ifdef UNSUPPORTEDMAKE
include $(UNSUPPORTEDMAKE)
endif
# sets projectname for actual make
ifdef PROJECTNAME
  PROJ_NAME=$(PROJECTNAME)
endif

export CC=$(CCPREFIX)gcc
export LD=$(CCPREFIX)gcc
export AR=$(CCPREFIX)ar
export AS=$(CCPREFIX)as
export OBJCOPY=$(CCPREFIX)objcopy
export OBJDUMP=$(CCPREFIX)objdump
export GDB=$(CCPREFIX)gdb

ifeq ($(V),1)
        quiet_=
        Q=
else
        quiet_=quiet_
        Q=@
  export SILENT=1
endif

# =============================================================================
# =============================================================================
# =============================================================================

boardjson: scripts/build_board_json.py $(WRAPPERSOURCES)
	@echo Generating Board JSON
	$(Q)echo WRAPPERSOURCES = $(WRAPPERSOURCES)
	$(Q)echo DEFINES =  $(DEFINES)
ifdef USE_NET
        # hack to ensure that Pico/etc have all possible firmware configs listed
	$(Q)python scripts/build_board_json.py $(WRAPPERSOURCES) $(DEFINES) -DUSE_WIZNET=1 -DUSE_CC3000=1 -B$(BOARD)
else
	$(Q)python scripts/build_board_json.py $(WRAPPERSOURCES) $(DEFINES) -B$(BOARD)
endif

$(WRAPPERFILE): scripts/build_jswrapper.py $(WRAPPERSOURCES)
	@echo Generating JS wrappers
	$(Q)echo WRAPPERSOURCES = $(WRAPPERSOURCES)
	$(Q)echo DEFINES =  $(DEFINES)
	$(Q)python scripts/build_jswrapper.py $(WRAPPERSOURCES) $(JSMODULESOURCES) $(DEFINES) -B$(BOARD) -F$(WRAPPERFILE)

ifdef PININFOFILE
$(PININFOFILE).c $(PININFOFILE).h: scripts/build_pininfo.py
	@echo Generating pin info
	$(Q)python scripts/build_pininfo.py $(BOARD) $(PININFOFILE).c $(PININFOFILE).h
endif

ifndef NRF5X # nRF5x devices use their own linker files that aren't automatically generated.
ifndef EFM32
$(LINKER_FILE): scripts/build_linker.py
	@echo Generating linker scripts
	$(Q)python scripts/build_linker.py $(BOARD) $(LINKER_FILE) $(BUILD_LINKER_FLAGS)
endif # EFM32
endif # NRF5X

$(PLATFORM_CONFIG_FILE): boards/$(BOARD).py scripts/build_platform_config.py
	@echo Generating platform configs
	$(Q)python scripts/build_platform_config.py $(BOARD) $(HEADERFILENAME)

# skips compiling and linking, if NO_COMPILE is defined
# Generation of temporary files and setting of wrappersources is already done this moment
ifndef NO_COMPILE

compile=$(CC) $(CFLAGS) $< -o $@

link=$(LD) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

# note: link is ignored for the ESP8266
obj_dump=$(OBJDUMP) -x -S $(PROJ_NAME).elf > $(PROJ_NAME).lst
obj_to_bin=$(OBJCOPY) -O $1 $(PROJ_NAME).elf $(PROJ_NAME).$2

quiet_compile= CC $@
quiet_link= LD $@
quiet_obj_dump= GEN $(PROJ_NAME).lst
quiet_obj_to_bin= GEN $(PROJ_NAME).$2

%.o: %.c $(PLATFORM_CONFIG_FILE) $(PININFOFILE).h
	@echo $($(quiet_)compile)
	@$(call compile)

.cpp.o: $(PLATFORM_CONFIG_FILE) $(PININFOFILE).h
	@echo $($(quiet_)compile)
	@$(call compile)

# case sensitive - Nordic's files are capitals
.s.o:
	@echo $($(quiet_)compile)
	@$(call compile)

.S.o:
	@echo $($(quiet_)compile)
	@$(call compile)

ifdef LINUX # ---------------------------------------------------
include make/targets/LINUX.make
else ifdef ESP32
include make/targets/ESP32.make
else ifdef ESP8266
include make/targets/ESP8266.make
else # ARM/etc, so generate bin, etc ---------------------------
include make/targets/ARM.make
endif	    # ---------------------------------------------------

else # NO_COMPILE
# log WRAPPERSOURCES to help Firmware creation tool
$(info WRAPPERSOURCES=$(WRAPPERSOURCES));
endif

lst: $(PROJ_NAME).lst

clean:
	@echo Cleaning targets
	$(Q)find . -name \*.o | grep -v "./arm-bcm2708\|./gcc-arm-none-eabi" | xargs rm -f
	$(Q)find . -name \*.d | grep -v "./arm-bcm2708\|./gcc-arm-none-eabi" | xargs rm -f
	$(Q)rm -f $(ROOT)/gen/*.c $(ROOT)/gen/*.h $(ROOT)/gen/*.ld
	$(Q)rm -f $(ROOT)/scripts/*.pyc $(ROOT)/boards/*.pyc
	$(Q)rm -f $(PROJ_NAME).elf
	$(Q)rm -f $(PROJ_NAME).hex
	$(Q)rm -f $(PROJ_NAME).bin
	$(Q)rm -f $(PROJ_NAME).srec
	$(Q)rm -f $(PROJ_NAME).lst

# start make like this "make varsonly" to get all variables created and used during make process without compiling
# this helps to better understand linking, or to find oddities
varsonly:
	$(foreach v, $(.VARIABLES), $(info $(v) = $($(v))))
