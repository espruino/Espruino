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
# make boardjson     # JSON file for a board
# make docs          # Reference HTML for a board
# make varsonly      # Dump Makefile vars - good for debugging
# make wrappersources # Show the WRAPPERSOURCES - list of C files that contain functions to load into Espruino environment
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
# ESPRUINO_WRAPPERSOURCES=jswrap_x.c
#                         # Compile in a wrapper file to the build (with JS functions in). WRAPPERSOURCES can be used too, but this adds the files to the END of the sources list
# PYTHON=python3          # The python command used for this build
#
# WIZNET=1                # If compiling for a non-linux target that has internet support, use WIZnet W5500 support
# W5100=1                 # Compile for WIZnet W5100 (not W5500)
# CC3000=1                # If compiling for a non-linux target that has internet support, use CC3000 support
# USB_PRODUCT_ID=0x1234   # force a specific USB Product ID (default 0x5740)
#
# GENDIR=MyGenDir		      # sets directory for intermediate files generated during make
# OBJDIR=MyObjDir		      # sets directory for object files generated during make
# BINDIR=MyBinDir    		  # sets directory for binaries generated during make
# SETDEFINES=FileDefines  # settings which are called after definitions for board are done
#                         # SETDEFINES=/home/mydir/myDefines
# UNSUPPORTEDMAKE=FileUnsu# Adds additional files from unsupported sources(means not supported by Gordon) to actual make
#                         # UNSUPPORTEDMAKE=/home/mydir/unsupportedCommands
# PROJECTNAME=myBigProject# Sets projectname
# BLACKLIST=fileBlacklist # Removes javascript commands given in a file from compilation and therefore from project defined firmware
#                         # is used in build_jswrapper.py - of the form [{class,name}...]
#                         # BLACKLIST=/home/mydir/myBlackList
# VARIABLES=1700          # Sets number of variables for project defined firmware. This parameter can be dangerous, be careful before changing.
#                         # used in build_platform_config.py

#
# -- STM32 Only
# PAD_FOR_BOOTLOADER=1    # Pad the binary out with 0xFF where the bootloader should be (allows the Web IDE to flash the binary)
#
# -- NRF52 Only
# INCLUDE_BLANK_STORAGE=1 # Include storage inside hex, so firmware updates remove any saved code
# DFU_UPDATE_BUILD=1      # Uncomment this to build Espruino for a device firmware update over the air (nRF52).
#
# -- ESP32 Only
# RTOS=1                  # adds RTOS functions, available only for ESP32

include make/sanitycheck.make

ifndef GENDIR
GENDIR=gen
endif
ifndef OBJDIR
OBJDIR=obj
endif
ifndef BINDIR
BINDIR=bin
endif

ifndef SINGLETHREAD
MAKEFLAGS=-j$(shell nproc) # make multicore based on the number of cores available
endif

INCLUDE?=-I$(ROOT) -I$(ROOT)/targets -I$(ROOT)/src -I$(GENDIR)
LIBS?=
DEFINES?=

CFLAGS_C_COMPILER?= -Werror=implicit-function-declaration
CFLAGS?=-Wall -Wextra -Wconversion -fno-strict-aliasing -g
CFLAGS+=-Wno-packed-bitfield-compat # remove warnings from packed var usage

CCFLAGS?= # specific flags when compiling cc files
LDFLAGS?=-Winline -g
OPTIMIZEFLAGS?=
#-fdiagnostics-show-option - shows which flags can be used with -Werror
DEFINES+=-DGIT_COMMIT=$(shell git log -1 --format="%h")

ifeq ($(shell uname),Darwin)
MACOSX=1
CFLAGS+=-D__MACOSX__
STAT_FLAGS='-f ''%z'''
REALPATH='grealpath'
TAR='gtar'
else
STAT_FLAGS='-c ''%s'''
REALPATH='realpath'
TAR='tar'
endif

ifeq ($(OS),Windows_NT)
MINGW=1
endif

ifdef RELEASE
# force no asserts to be compiled in
DEFINES += -DNO_ASSERT -DRELEASE
endif

PYTHON?=python

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
 ifneq ("$(wildcard /usr/include/wiringPi.h)","")
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

$(shell rm -f $(GENDIR)/CURRENT_BOARD.make)
$(shell $(PYTHON) scripts/get_makefile_decls.py $(BOARD) > $(GENDIR)/CURRENT_BOARD.make)
include $(GENDIR)/CURRENT_BOARD.make

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
else ifeq ($(FAMILY),ESP32_IDF4)
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
  OPTIMIZEFLAGS=-g -Os -Wl,--allow-multiple-definition
  CFLAGS_C_COMPILER= -std=gnu11 -fgnu89-inline
 else
  OPTIMIZEFLAGS=-g
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

# These are jswrap_.c files specified on the command-line that get added to the build after other WRAPPERSOURCES
ESPRUINO_WRAPPERSOURCES ?=

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
src/jswrap_functions.c \
src/jswrap_json.c \
src/jswrap_number.c \
src/jswrap_object.c \
src/jswrap_regexp.c \
src/jswrap_string.c \
src/jswrap_modules.c \
src/jswrap_math.c


ifndef ESPR_EMBED # These are wrapper sources to do with hardware, if embedding we don't need these
WRAPPERSOURCES += \
src/jswrap_espruino.c \
src/jswrap_flash.c \
src/jswrap_interactive.c \
src/jswrap_io.c \
src/jswrap_pin.c \
src/jswrap_pipe.c \
src/jswrap_process.c \
src/jswrap_onewire.c \
src/jswrap_promise.c \
src/jswrap_serial.c \
src/jswrap_storage.c \
src/jswrap_spi_i2c.c \
src/jswrap_stepper.c \
src/jswrap_stream.c \
src/jswrap_waveform.c
endif

# it is important that _pin comes before stuff which uses
# integers (as the check for int *includes* the chek for pin)
SOURCES += \
src/jslex.c \
src/jsflags.c \
src/jsvar.c \
src/jsvariterator.c \
src/jsutils.c \
src/jsnative.c \
src/jsparse.c \
$(WRAPPERFILE)

ifndef ESPR_EMBED # These are sources to do with hardware, if embedding we don't need these
SOURCES += \
src/jsflash.c \
src/jspin.c \
src/jsinteractive.c \
src/jsdevices.c \
src/jstimer.c \
src/jsi2c.c \
src/jsserial.c \
src/jsspi.c \
src/jshardware_common.c
endif


CPPSOURCES =
CCSOURCES =

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

# If we have enough flash, include the debugger
ifneq ($(USE_DEBUGGER),0)
DEFINES+=-DUSE_DEBUGGER
endif
# Use use tab complete
ifneq ($(USE_TAB_COMPLETE),0)
DEFINES+=-DUSE_TAB_COMPLETE
endif

# Heatshrink compression library and wrapper - better compression when saving code to flash
DEFINES+=-DUSE_HEATSHRINK
INCLUDE += -I$(ROOT)/libs/compression -I$(ROOT)/libs/compression/heatshrink
SOURCES += \
libs/compression/heatshrink/heatshrink_encoder.c \
libs/compression/heatshrink/heatshrink_decoder.c \
libs/compression/compress_heatshrink.c
WRAPPERSOURCES += \
libs/compression/jswrap_heatshrink.c
endif

ifndef BOOTLOADER # ------------------------------------------------------------------------------ DON'T USE IN BOOTLOADER

ifeq ($(FAMILY),ESP8266)
# special ESP8266 maths lib that doesn't go into RAM
LIBS += -lmirom
LDFLAGS += -L$(ROOT)/targets/esp8266
else
# everything else uses normal maths lib
LIBS += -lm
endif

ifeq ($(USE_FILESYSTEM),1)
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

ifeq ($(USE_FILESYSTEM_SDIO),1)
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

ifeq ($(USE_GRAPHICS),1)
DEFINES += -DUSE_GRAPHICS
INCLUDE += -I$(ROOT)/libs/graphics
WRAPPERSOURCES += libs/graphics/jswrap_graphics.c
SOURCES += \
libs/graphics/bitmap_font_4x6.c \
libs/graphics/bitmap_font_6x8.c \
libs/graphics/vector_font.c \
libs/graphics/pbf_font.c \
libs/graphics/graphics.c \
libs/graphics/lcd_arraybuffer.c \
libs/graphics/lcd_js.c

ifeq ($(USE_LCD_SDL),1)
  DEFINES += -DUSE_LCD_SDL
  SOURCES += libs/graphics/lcd_sdl.c
  LIBS += -lSDL
  INCLUDE += -I/usr/include/SDL
endif

ifdef USE_LCD_FSMC
  DEFINES += -DUSE_LCD_FSMC
  SOURCES += libs/graphics/lcd_fsmc.c
endif

ifdef USE_LCD_SPI
  DEFINES += -DUSE_LCD_SPI
  SOURCES += libs/graphics/lcd_spilcd.c
endif

ifdef USE_LCD_ST7789_8BIT
  DEFINES += -DUSE_LCD_ST7789_8BIT
  SOURCES += libs/graphics/lcd_st7789_8bit.c
endif

ifdef USE_LCD_MEMLCD
  DEFINES += -DUSE_LCD_MEMLCD
  SOURCES += libs/graphics/lcd_memlcd.c
endif

ifdef USE_LCD_SPI_UNBUF
  DEFINES += -DUSE_LCD_SPI_UNBUF
  WRAPPERSOURCES += libs/graphics/lcd_spi_unbuf.c
endif

ifeq ($(USE_TERMINAL),1)
  DEFINES += -DUSE_TERMINAL
  WRAPPERSOURCES += libs/graphics/jswrap_terminal.c
endif

ifeq ($(USE_SWDCON),1)
  DEFINES += -DUSE_SWDCON
  WRAPPERSOURCES += libs/swdcon/jswrap_swdcon.c
# directly included so not needed SOURCES += libs/swdcon/SEGGER_RTT_custom.c
endif

endif

ifeq ($(USE_USB_HID),1)
  DEFINES += -DUSE_USB_HID
endif

ifeq ($(USE_NET),1)
 DEFINES += -DUSE_NET
 INCLUDE += -I$(ROOT)/libs/network -I$(ROOT)/libs/network -I$(ROOT)/libs/network/http
 WRAPPERSOURCES += \
 libs/network/jswrap_net.c \
 libs/network/http/jswrap_http.c
 SOURCES += \
 libs/network/network.c \
 libs/network/socketserver.c \
 libs/network/socketerrors.c

ifneq ($(USE_NETWORK_JS),0)
 DEFINES += -DUSE_NETWORK_JS
 WRAPPERSOURCES += libs/network/js/jswrap_jsnetwork.c
 INCLUDE += -I$(ROOT)/libs/network/js
 SOURCES += libs/network/js/network_js.c
endif

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
 libs/network/esp8266/pktbuf.c

 ifndef NO_FOTA
   SOURCES += libs/network/esp8266/ota.c
 else
   DEFINES += -DNO_FOTA
 endif
 endif

 ifdef USE_TELNET
  DEFINES += -DUSE_TELNET
  WRAPPERSOURCES += libs/network/telnet/jswrap_telnet.c
  INCLUDE += -I$(ROOT)/libs/network/telnet
 endif
endif # USE_NET

ifeq ($(USE_TV),1)
  DEFINES += -DUSE_TV
  WRAPPERSOURCES += libs/tv/jswrap_tv.c
  INCLUDE += -I$(ROOT)/libs/tv
  SOURCES += \
  libs/tv/tv.c
endif

ifeq ($(USE_TRIGGER),1)
  DEFINES += -DUSE_TRIGGER
  WRAPPERSOURCES += libs/trigger/jswrap_trigger.c
  INCLUDE += -I$(ROOT)/libs/trigger
  SOURCES += \
  libs/trigger/trigger.c
endif

ifeq ($(USE_WIRINGPI),1)
  DEFINES += -DUSE_WIRINGPI
  LIBS += -lwiringPi
endif

ifeq ($(USE_BLUETOOTH),1)
  DEFINES += -DBLUETOOTH
  INCLUDE += -I$(ROOT)/libs/bluetooth
  WRAPPERSOURCES += libs/bluetooth/jswrap_bluetooth.c
  SOURCES += libs/bluetooth/bluetooth_utils.c
endif

ifeq ($(USE_CRYPTO),1)
  cryptofound:=$(shell if test -f make/crypto/$(FAMILY).make; then echo yes;fi)
  ifeq ($(cryptofound),yes)
    include make/crypto/$(FAMILY).make
  else
    include make/crypto/default.make
  endif
endif

ifeq ($(USE_NEOPIXEL),1)
  DEFINES += -DUSE_NEOPIXEL
  INCLUDE += -I$(ROOT)/libs/neopixel
  WRAPPERSOURCES += libs/neopixel/jswrap_neopixel.c
endif

ifeq ($(USE_NFC),1)
  DEFINES += -DUSE_NFC -DNFC_HAL_ENABLED=1
  INCLUDE          += -I$(NRF5X_SDK_PATH)/components/nfc/t2t_lib
  INCLUDE          += -I$(NRF5X_SDK_PATH)/components/nfc/ndef/uri
  INCLUDE          += -I$(NRF5X_SDK_PATH)/components/nfc/ndef/generic/message
  INCLUDE          += -I$(NRF5X_SDK_PATH)/components/nfc/ndef/generic/record
  INCLUDE          += -I$(NRF5X_SDK_PATH)/components/nfc/ndef/connection_handover/ble_pair_msg
  INCLUDE          += -I$(NRF5X_SDK_PATH)/components/nfc/ndef/connection_handover/hs_rec
  INCLUDE          += -I$(NRF5X_SDK_PATH)/components/nfc/ndef/connection_handover/ac_rec
  INCLUDE          += -I$(NRF5X_SDK_PATH)/components/nfc/ndef/connection_handover/le_oob_rec
  INCLUDE          += -I$(NRF5X_SDK_PATH)/components/nfc/ndef/connection_handover/ble_oob_advdata
  INCLUDE          += -I$(NRF5X_SDK_PATH)/components/nfc/ndef/connection_handover/ep_oob_rec
  INCLUDE          += -I$(NRF5X_SDK_PATH)/components/nfc/ndef/connection_handover/common
  INCLUDE          += -I$(NRF5X_SDK_PATH)/components/nfc/ndef/launchapp
  TARGETSOURCES    += $(NRF5X_SDK_PATH)/components/nfc/ndef/uri/nfc_uri_msg.c
  TARGETSOURCES    += $(NRF5X_SDK_PATH)/components/nfc/ndef/uri/nfc_uri_rec.c
  TARGETSOURCES    += $(NRF5X_SDK_PATH)/components/nfc/ndef/generic/message/nfc_ndef_msg.c
  TARGETSOURCES    += $(NRF5X_SDK_PATH)/components/nfc/ndef/generic/record/nfc_ndef_record.c
  TARGETSOURCES    += $(NRF5X_SDK_PATH)/components/nfc/ndef/connection_handover/ble_pair_msg/nfc_ble_pair_msg.c
  TARGETSOURCES    += $(NRF5X_SDK_PATH)/components/nfc/ndef/connection_handover/hs_rec/nfc_hs_rec.c
  TARGETSOURCES    += $(NRF5X_SDK_PATH)/components/nfc/ndef/connection_handover/le_oob_rec/nfc_le_oob_rec.c
  TARGETSOURCES    += $(NRF5X_SDK_PATH)/components/nfc/ndef/connection_handover/ep_oob_rec/nfc_ep_oob_rec.c
  TARGETSOURCES    += $(NRF5X_SDK_PATH)/components/nfc/ndef/connection_handover/ac_rec/nfc_ac_rec.c
  TARGETSOURCES    += $(NRF5X_SDK_PATH)/components/nfc/ndef/connection_handover/hs_rec/nfc_hs_rec.c
  TARGETSOURCES    += $(NRF5X_SDK_PATH)/components/nfc/ndef/connection_handover/ble_oob_advdata/nfc_ble_oob_advdata.c
  TARGETSOURCES    += $(NRF5X_SDK_PATH)/components/nfc/ndef/connection_handover/common/nfc_ble_pair_common.c
  TARGETSOURCES    += $(NRF5X_SDK_PATH)/components/nfc/ndef/launchapp/nfc_launchapp_msg.c
  TARGETSOURCES    += $(NRF5X_SDK_PATH)/components/nfc/ndef/launchapp/nfc_launchapp_rec.c
  TARGETSOURCES    += $(NRF5X_SDK_PATH)/components/nfc/t2t_lib/hal_t2t/hal_nfc_t2t.c
endif

ifeq ($(USE_WIO_LTE),1)
  INCLUDE += -I$(ROOT)/libs/wio_lte
  WRAPPERSOURCES += libs/wio_lte/jswrap_wio_lte.c
  SOURCES += targets/stm32/stm32_ws2812b_driver.c
endif

ifeq ($(USE_TENSORFLOW),1)
include make/misc/tensorflow.make
endif

ifeq ($(USE_JIT),1)
  DEFINES += -DESPR_JIT
  SOURCES += src/jsjit.c src/jsjitc.c
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
WRAPPERSOURCES += $(ESPRUINO_WRAPPERSOURCES)
SOURCES += $(WRAPPERSOURCES) $(TARGETSOURCES)
SOURCEOBJS = $(patsubst %.c,$(OBJDIR)/%.o,$(SOURCES))
ifdef CPPSOURCES
SOURCEOBJS += $(patsubst %.cpp,$(OBJDIR)/%.cpp.o,$(CPPSOURCES))
endif
ifdef CCSOURCES
SOURCEOBJS += $(patsubst %.cc,$(OBJDIR)/%.cc.o,$(CCSOURCES))
endif
OBJS = $(PRECOMPILED_OBJS) $(SOURCEOBJS)


# -ffreestanding -nodefaultlibs -nostdlib -fno-common
# -nodefaultlibs -nostdlib -nostartfiles
# -fdata-sections -ffunction-sections are to help remove unused code

# See the build_platform_config.py/platform_config.h for notes on why we define ESPR_DEFINES_ON_COMMANDLINE
CFLAGS += $(OPTIMIZEFLAGS) -c $(ARCHFLAGS) $(DEFINES) $(INCLUDE) -DESPR_DEFINES_ON_COMMANDLINE

# -Wl,--gc-sections helps remove unused code
# -Wl,--whole-archive checks for duplicates
# --specs=nano.specs uses newlib-nano
ifdef NRF5X
 LDFLAGS += $(OPTIMIZEFLAGS) $(ARCHFLAGS) --specs=nano.specs -lc -lnosys
else ifdef STM32
 LDFLAGS += $(OPTIMIZEFLAGS) $(ARCHFLAGS) --specs=nano.specs -lc -lnosys
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
ifdef BLACKLIST
  # to allow blacklist to take effect if defined
  # inside a BOARD.py file
  export BLACKLIST
endif

# =============================================================================
# =============================================================================
# =============================================================================

boardjson: scripts/build_board_json.py $(WRAPPERSOURCES)
	@echo ================================== Generating Board JSON
	$(Q)echo WRAPPERSOURCES = $(WRAPPERSOURCES)
	$(Q)echo DEFINES =  $(DEFINES)
ifdef USE_NET
        # hack to ensure that Pico/etc have all possible firmware configs listed
	$(Q)$(PYTHON) scripts/build_board_json.py $(WRAPPERSOURCES) $(DEFINES) -DUSE_WIZNET=1 -DUSE_CC3000=1 -B$(BOARD)
else
	$(Q)$(PYTHON) scripts/build_board_json.py $(WRAPPERSOURCES) $(DEFINES) -B$(BOARD)
endif

docs:
	@echo ================================== Generating Board docs
	$(Q)python scripts/build_docs.py $(WRAPPERSOURCES) $(DEFINES) -B$(BOARD)
	@echo functions.html created

$(WRAPPERFILE): scripts/build_jswrapper.py $(WRAPPERSOURCES)
	@echo ================================== Generating JS wrappers
	$(Q)echo WRAPPERSOURCES = $(WRAPPERSOURCES)
	$(Q)echo DEFINES =  $(DEFINES)
	$(Q)$(PYTHON) scripts/build_jswrapper.py $(WRAPPERSOURCES) $(JSMODULESOURCES) $(DEFINES) -B$(BOARD) -F$(WRAPPERFILE)

ifdef PININFOFILE
$(PININFOFILE).c $(PININFOFILE).h: scripts/build_pininfo.py
	@echo ================================== Generating pin info
	$(Q)$(PYTHON) scripts/build_pininfo.py $(BOARD) $(PININFOFILE).c $(PININFOFILE).h
endif

ifndef NRF5X # nRF5x devices use their own linker files that aren't automatically generated.
$(LINKER_FILE): scripts/build_linker.py
	@echo ================================== Generating linker scripts
	$(Q)$(PYTHON) scripts/build_linker.py $(BOARD) $(LINKER_FILE) $(BUILD_LINKER_FLAGS)
endif # NRF5X

$(PLATFORM_CONFIG_FILE): boards/$(BOARD).py scripts/build_platform_config.py
	@echo ================================== Generating platform configs
	$(Q)$(PYTHON) scripts/build_platform_config.py $(BOARD) $(HEADERFILENAME) $(DEFINES)

# If realpath exists, use relative paths
ifneq ("$(shell ${REALPATH} --version > /dev/null;echo "$$?")","0")
compile=$(CC) $(CFLAGS_C_COMPILER) $(CFLAGS) $< -o $@
else
# when macros use __FILE__ this stops us including the whole build path
compile=$(CC) $(CFLAGS_C_COMPILER) $(CFLAGS) $(shell ${REALPATH} --relative-to $(shell pwd) $<) -o $@
endif

link=$(LD) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

# note: link is ignored for the ESP8266
obj_dump=$(OBJDUMP) -x -S $(PROJ_NAME).elf > $(PROJ_NAME).lst
obj_to_bin=$(OBJCOPY) -O $1 $(PROJ_NAME).elf $(PROJ_NAME).$2

quiet_compile= CC $@
quiet_link= LD $@
quiet_obj_dump= GEN $(PROJ_NAME).lst
quiet_obj_to_bin= GEN $(PROJ_NAME).$2

$(OBJDIR)/%.o: %.c $(PLATFORM_CONFIG_FILE) $(PININFOFILE).h
	@echo $($(quiet_)compile)
	@mkdir -p $(shell dirname $@) # create directory if it doesn't exist
	@$(call compile)

$(OBJDIR)/%.cc.o: %.cc $(PLATFORM_CONFIG_FILE) $(PININFOFILE).h
	@echo $($(quiet_)compile)
	@mkdir -p $(shell dirname $@) # create directory if it doesn't exist
	@$(CC) $(CCFLAGS) $(CFLAGS) $< -o $@

$(OBJDIR)/%.cpp.o: $(PLATFORM_CONFIG_FILE) $(PININFOFILE).h
	@echo $($(quiet_)compile)
	@mkdir -p $(shell dirname $@) # create directory if it doesn't exist
	@$(call compile)

# case sensitive - Nordic's files are capitals
$(OBJDIR)/%.s.o:
	@echo $($(quiet_)compile)
	@mkdir -p $(shell dirname $@) # create directory if it doesn't exist
	@$(call compile)

$(OBJDIR)/%.S.o:
	@echo $($(quiet_)compile)
	@mkdir -p $(shell dirname $@) # create directory if it doesn't exist
	@$(call compile)

ifdef LINUX # ---------------------------------------------------
include make/targets/LINUX.make
else ifdef EMSCRIPTEN
include make/targets/EMSCRIPTEN.make
else ifdef ESP32_IDF4
include make/targets/ESP32_IDF4.make
else ifdef ESP32
include make/targets/ESP32.make
else ifdef ESP8266
include make/targets/ESP8266.make
else ifdef ESPR_EMBED
include make/targets/EMBED.make
else # ARM/etc, so generate bin, etc ---------------------------
include make/targets/ARM.make
endif	    # ---------------------------------------------------

lst: $(PROJ_NAME).lst

clean:
	@echo Cleaning targets
	$(Q)rm -rf $(OBJDIR)/* $(BINDIR)/build $(BINDIR)/main
	$(Q)rm -f $(GENDIR)/*.c $(GENDIR)/*.h $(GENDIR)/*.ld
	$(Q)rm -f $(ROOT)/scripts/*.pyc $(ROOT)/boards/*.pyc
	$(Q)rm -f $(PROJ_NAME).elf
	$(Q)rm -f $(PROJ_NAME).hex
	$(Q)rm -f $(PROJ_NAME).bin
	$(Q)rm -f $(PROJ_NAME).srec
	$(Q)rm -f $(PROJ_NAME).lst
	$(Q)rm -f $(BINDIR)/espruino_embedded.h
	$(Q)rm -f $(BINDIR)/espruino_embedded.c
	$(Q)rm -f $(BINDIR)/jstypes.h
	$(Q)rm -f $(ROOT)/targetlibs/nrf5x_*/components/toolchain/gcc/gcc_startup_nrf5*.o $(ROOT)/targetlibs/nrf5x_*/modules/nrfx/mdk/gcc_startup_nrf5*.o $(ROOT)/targetlibs/stm32f4/lib/startup_stm32f4*.o $(ROOT)/targetlibs/stm32f1/lib/startup_stm32f10x_*.o

wrappersources:
	$(info WRAPPERSOURCES=$(WRAPPERSOURCES))

# start make like this "make varsonly" to get all variables created and used during make process without compiling
# this helps to better understand linking, or to find oddities
varsonly:
	$(foreach v, $(.VARIABLES), $(info $(v) = $($(v))))
