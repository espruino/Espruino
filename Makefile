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
# Set ONE of the following environment variables to compile for that board:
#
# ESPRUINOBOARD=1          # Espruino board rev 1.3 and rev 1v4
# PICO_R1_3=1              # Espruino Pico board rev 1.3
# ESPRUINOWIFI=1
# PUCKJS=1

# OLIMEXINO_STM32=1       # Olimexino STM32
# MAPLERET6_STM32=1       # Limited production Leaflabs Maple r5 with a STM32F103RET6
# MAPLEMINI=1             # Leaflabs Maple Mini
# EMBEDDED_PI=1           # COOCOX STM32 Embedded Pi boards
# HYSTM32_24=1            # HY STM32 2.4 Ebay boards
# HYSTM32_28=1            # HY STM32 2.8 Ebay boards
# HYSTM32_32=1            # HY STM32 3.2 VCT6 Ebay boards
# HYTINY_STM103T=1				# HY-TinySTM103T by Haoyu (hotmcu.com)
# STM32VLDISCOVERY=1
# STM32F3DISCOVERY=1
# STM32F4DISCOVERY=1
# STM32F411DISCOVERY=1
# STM32F429IDISCOVERY=1
# STM32F401CDISCOVERY=1
# MICROBIT=1
# NRF51TAG=1
# NRF51822DK=1
# NRF52832DK=1            # Ultra low power BLE (bluetooth low energy) enabled SoC. Arm Cortex-M4f processor. With NFC (near field communication).
# CARAMBOLA=1
# DPTBOARD=1              # DPTechnics IoT development board with BlueCherry.io IoT platform integration and DPT-WEB IDE.
# RASPBERRYPI=1
# BEAGLEBONE=1
# ARIETTA=1
# LCTECH_STM32F103RBT6=1 # LC Technology STM32F103RBT6 Ebay boards
# ARMINARM=1
# NUCLEOF401RE=1
# NUCLEOF411RE=1
# NUCLEOL476RG=1
# MINISTM32_STRIVE=1
# MINISTM32_ANGLED_VE=1
# MINISTM32_ANGLED_VG=1
# ESP8266_BOARD=1         # ESP8266
# ESP32=1                 # ESP32
# EFM32GGSTK=1            # Currently only works with DEBUG=1
# EMW3165=1               # MXCHIP EMW3165: STM32F411CE, BCM43362, 512KB flash 128KB RAM
# Or nothing for standard linux compile
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
# WIZNET=1                # If compiling for a non-linux target that has internet support, use WIZnet support, not TI CC3000
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
# RTOS                    # adds RTOS functions, available only for ESP32 (yet)

ifndef GENDIR
GENDIR=$(shell pwd)/gen
endif

ifndef SINGLETHREAD
MAKEFLAGS=-j5 # multicore
endif

INCLUDE=-I$(ROOT) -I$(ROOT)/targets -I$(ROOT)/src -I$(GENDIR)
LIBS=
DEFINES=
CFLAGS=-Wall -Wextra -Wconversion -Werror=implicit-function-declaration -fno-strict-aliasing
LDFLAGS=-Winline
OPTIMIZEFLAGS=
#-fdiagnostics-show-option - shows which flags can be used with -Werror
DEFINES+=-DGIT_COMMIT=$(shell git log -1 --format="%H")

# Espruino flags...
USE_MATH=1

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


# ---------------------------------------------------------------------------------
#                                                      Get info out of BOARDNAME.py
# ---------------------------------------------------------------------------------
ifeq ($(BOARD),)
$(info *************************************************************)
$(info *           To build, use "BOARD=my_board make              *")
$(info *************************************************************)
endif
$(shell rm -f CURRENT_BOARD.make)
$(shell python scripts/get_makefile_decls.py $(BOARD) > CURRENT_BOARD.make)


include CURRENT_BOARD.make

# ---------------------------------------------------------------------------------
# When adding stuff here, also remember build_pininfo, platform_config.h, jshardware.c
# TODO: Load more of this out of the BOARDNAME.py files if at all possible (see next section)
# ---------------------------------------------------------------------------------
ifdef OLD_CODE
ifdef EFM32GGSTK
EMBEDDED=1
DEFINES+= -DEFM32GG890F1024=1 # This should be EFM32GG990F1024, temporary hack to avoid the #USB on line 772 in jsinteractive.c
BOARD=EFM32GGSTK
OPTIMIZEFLAGS+=-Os

else ifdef OLIMEXINO_STM32
EMBEDDED=1
SAVE_ON_FLASH=1
USE_FILESYSTEM=1
BOARD=OLIMEXINO_STM32
STLIB=STM32F10X_MD
PRECOMPILED_OBJS+=$(ROOT)/targetlibs/stm32f1/lib/startup_stm32f10x_md.o
OPTIMIZEFLAGS+=-Os # short on program memory

else ifdef HYTINY_STM103T
EMBEDDED=1
USE_GRAPHICS=1
SAVE_ON_FLASH=1
BOARD=HYTINY_STM103T
STLIB=STM32F10X_MD
PRECOMPILED_OBJS+=$(ROOT)/targetlibs/stm32f1/lib/startup_stm32f10x_md.o
OPTIMIZEFLAGS+=-Os # short on program memory

else ifdef MAPLERET6_STM32
EMBEDDED=1
USE_NET=1
USE_GRAPHICS=1
USE_FILESYSTEM=1
USE_TV=1
USE_HASHLIB=1
BOARD=MAPLERET6_STM32
STLIB=STM32F10X_HD
PRECOMPILED_OBJS+=$(ROOT)/targetlibs/stm32f1/lib/startup_stm32f10x_hd.o
OPTIMIZEFLAGS+=-O3

else ifdef OLIMEXINO_STM32_RE
EMBEDDED=1
USE_NET=1
USE_GRAPHICS=1
USE_FILESYSTEM=1
USE_TV=1
USE_HASHLIB=1
BOARD=OLIMEXINO_STM32_RE
STLIB=STM32F10X_HD
PRECOMPILED_OBJS+=$(ROOT)/targetlibs/stm32f1/lib/startup_stm32f10x_hd.o
OPTIMIZEFLAGS+=-O3

else ifdef MAPLEMINI
EMBEDDED=1
SAVE_ON_FLASH=1
BOARD=MAPLEMINI
STLIB=STM32F10X_MD
PRECOMPILED_OBJS+=$(ROOT)/targetlibs/stm32f1/lib/startup_stm32f10x_md.o
OPTIMIZEFLAGS+=-Os # short on program memory

else ifdef EMBEDDED_PI
EMBEDDED=1
BOARD=EMBEDDED_PI
STLIB=STM32F10X_MD
PRECOMPILED_OBJS+=$(ROOT)/targetlibs/stm32f1/lib/startup_stm32f10x_md.o
OPTIMIZEFLAGS+=-Os # short on program memory

else ifdef MINISTM32_STRIVE
EMBEDDED=1
USE_GRAPHICS=1
USE_LCD_FSMC=1
DEFINES+=-DFSMC_BITBANG # software implementation because FSMC HW causes strange crashes
DEFINES+=-DUSE_RTC
DEFINES+=-DSWD_ONLY_NO_JTAG
USE_FILESYSTEM=1
USE_FILESYSTEM_SDIO=1
BOARD=MINISTM32_STRIVE
STLIB=STM32F10X_HD
PRECOMPILED_OBJS+=$(ROOT)/targetlibs/stm32f1/lib/startup_stm32f10x_hd.o
OPTIMIZEFLAGS+=-O3

else ifdef MINISTM32_ANGLED_VE
EMBEDDED=1
USE_GRAPHICS=1
USE_LCD_FSMC=1
DEFINES+=-DFSMC_BITBANG # software implementation because FSMC HW causes strange crashes
DEFINES+=-DUSE_RTC
DEFINES+=-DSWD_ONLY_NO_JTAG
USE_FILESYSTEM=1
USE_FILESYSTEM_SDIO=1
BOARD=MINISTM32_ANGLED_VE
STLIB=STM32F10X_HD
PRECOMPILED_OBJS+=$(ROOT)/targetlibs/stm32f1/lib/startup_stm32f10x_hd.o
OPTIMIZEFLAGS+=-O3

else ifdef MINISTM32_ANGLED_VG
EMBEDDED=1
USE_GRAPHICS=1
USE_LCD_FSMC=1
DEFINES+=-DFSMC_BITBANG # software implementation because FSMC HW causes strange crashes
DEFINES+=-DUSE_RTC
DEFINES+=-DSWD_ONLY_NO_JTAG
USE_FILESYSTEM=1
USE_FILESYSTEM_SDIO=1
BOARD=MINISTM32_ANGLED_VG
STLIB=STM32F10X_XL
PRECOMPILED_OBJS+=$(ROOT)/targetlibs/stm32f1/lib/startup_stm32f10x_xl.o
OPTIMIZEFLAGS+=-O3

else ifdef HYSTM32_24
EMBEDDED=1
USE_GRAPHICS=1
USE_LCD_FSMC=1
DEFINES+=-DFSMC_BITBANG # software implementation because FSMC HW causes strange crashes
USE_FILESYSTEM=1
USE_FILESYSTEM_SDIO=1
USE_NEOPIXEL=1
BOARD=HYSTM32_24
STLIB=STM32F10X_HD
PRECOMPILED_OBJS+=$(ROOT)/targetlibs/stm32f1/lib/startup_stm32f10x_hd.o
OPTIMIZEFLAGS+=-O3

else ifdef HYSTM32_28
EMBEDDED=1
USE_GRAPHICS=1
USE_LCD_FSMC=1
DEFINES+=-DILI9325_BITBANG # bit-bang the LCD driver
USE_NEOPIXEL=1
SAVE_ON_FLASH=1
#USE_FILESYSTEM=1 # just normal SPI
BOARD=HYSTM32_28
STLIB=STM32F10X_MD
PRECOMPILED_OBJS+=$(ROOT)/targetlibs/stm32f1/lib/startup_stm32f10x_md.o
OPTIMIZEFLAGS+=-Os

else ifdef HYSTM32_32
EMBEDDED=1
USE_GRAPHICS=1
USE_LCD_FSMC=1
DEFINES+=-DFSMC_BITBANG # software implementation because FSMC HW causes strange crashes
USE_FILESYSTEM=1
USE_FILESYSTEM_SDIO=1
USE_NEOPIXEL=1
BOARD=HYSTM32_32
STLIB=STM32F10X_HD
PRECOMPILED_OBJS+=$(ROOT)/targetlibs/stm32f1/lib/startup_stm32f10x_hd.o
OPTIMIZEFLAGS+=-Os

else ifdef NUCLEOF401RE
EMBEDDED=1
NUCLEO=1
USE_GRAPHICS=1
USE_NET=1
USE_NEOPIXEL=1
BOARD=NUCLEOF401RE
STLIB=STM32F401xE
PRECOMPILED_OBJS+=$(ROOT)/targetlibs/stm32f4/lib/startup_stm32f401xx.o
OPTIMIZEFLAGS+=-O3

else ifdef NUCLEOF411RE
EMBEDDED=1
NUCLEO=1
USE_GRAPHICS=1
USE_NET=1
USE_NEOPIXEL=1
BOARD=NUCLEOF411RE
STLIB=STM32F401xE
PRECOMPILED_OBJS+=$(ROOT)/targetlibs/stm32f4/lib/startup_stm32f401xx.o
OPTIMIZEFLAGS+=-O3

else ifdef NUCLEOL476RG
EMBEDDED=1
NUCLEO=1
USE_GRAPHICS=1
USE_NET=1
BOARD=NUCLEOL476RG
STLIB=STM32L476xx
#PRECOMPILED_OBJS+=$(ROOT)/targetlibs/stm32l4/lib/startup_stm32f401xx.o
PRECOMPILED_OBJS+=$(ROOT)/targetlibs/stm32l4/lib/CMSIS/Device/ST/STM32L4xx/Source/Templates/gcc/startup_stm32l476xx.o
OPTIMIZEFLAGS+=-O3

else ifdef EMW3165
#ifndef WICED_ROOT
#$(error WICED_ROOT must be defined)
#endif
EMBEDDED=1
#USE_GRAPHICS=1
#USE_NET=1
BOARD=EMW3165
STLIB=STM32F411xE
PRECOMPILED_OBJS+=$(ROOT)/targetlibs/stm32f4/lib/startup_stm32f40_41xxx.o
OPTIMIZEFLAGS+=-O2
#WICED=1
DEFINES += -DPIN_NAMES_DIRECT -DHSE_VALUE=26000000UL

else ifdef STM32F4DISCOVERY
EMBEDDED=1
USE_NET=1
USE_GRAPHICS=1
USE_NEOPIXEL=1
DEFINES += -DUSE_USB_OTG_FS=1
BOARD=STM32F4DISCOVERY
STLIB=STM32F407xx
PRECOMPILED_OBJS+=$(ROOT)/targetlibs/stm32f4/lib/startup_stm32f40_41xxx.o
OPTIMIZEFLAGS+=-O3

else ifdef STM32F411DISCOVERY
EMBEDDED=1
USE_NET=1
USE_GRAPHICS=1
USE_NEOPIXEL=1
DEFINES += -DUSE_USB_OTG_FS=1
BOARD=STM32F411DISCOVERY
STLIB=STM32F411xE
PRECOMPILED_OBJS+=$(ROOT)/targetlibs/stm32f4/lib/startup_stm32f401xx.o
OPTIMIZEFLAGS+=-O3

else ifdef STM32F401CDISCOVERY
EMBEDDED=1
USE_NET=1
USE_GRAPHICS=1
USE_NEOPIXEL=1
DEFINES += -DUSE_USB_OTG_FS=1
BOARD=STM32F401CDISCOVERY
STLIB=STM32F401xE
PRECOMPILED_OBJS+=$(ROOT)/targetlibs/stm32f4/lib/startup_stm32f401xx.o
OPTIMIZEFLAGS+=-O3

else ifdef STM32F429IDISCOVERY
EMBEDDED=1
USE_GRAPHICS=1
USE_NEOPIXEL=1
DEFINES += -DUSE_USB_OTG_FS=1
BOARD=STM32F429IDISCOVERY
STLIB=STM32F429xx
PRECOMPILED_OBJS+=$(ROOT)/targetlibs/stm32f4/lib/startup_stm32f429_439xx.o
OPTIMIZEFLAGS+=-O3

else ifdef STM32F3DISCOVERY
EMBEDDED=1
USE_NET=1
USE_GRAPHICS=1
USE_NEOPIXEL=1
BOARD=STM32F3DISCOVERY
STLIB=STM32F3XX
PRECOMPILED_OBJS+=$(ROOT)/targetlibs/stm32f3/lib/startup_stm32f30x.o
OPTIMIZEFLAGS+=-Os

else ifdef STM32VLDISCOVERY
EMBEDDED=1
SAVE_ON_FLASH=1
USE_NEOPIXEL=1
BOARD=STM32VLDISCOVERY
STLIB=STM32F10X_MD_VL
PRECOMPILED_OBJS+=$(ROOT)/targetlibs/stm32f1/lib/startup_stm32f10x_md_vl.o
OPTIMIZEFLAGS+=-Os # short on program memory

else ifdef MICROBIT
EMBEDDED=1
SAVE_ON_FLASH=1
# Save on flash, but we still want the debugger and tab complete
DEFINES+=-DUSE_DEBUGGER -DUSE_TAB_COMPLETE
BOARD=MICROBIT
OPTIMIZEFLAGS+=-Os
USE_BLUETOOTH=1
USE_GRAPHICS=1

else ifdef DO003
EMBEDDED=1
SAVE_ON_FLASH=1
# Save on flash, but we still want the debugger and tab complete
DEFINES+=-DUSE_DEBUGGER -DUSE_TAB_COMPLETE
BOARD=DO-003
OPTIMIZEFLAGS+=-Os
USE_BLUETOOTH=1
USE_GRAPHICS=1

else ifdef NRF51TAG
EMBEDDED=1
SAVE_ON_FLASH=1
# Save on flash, but we still want the debugger and tab complete
DEFINES+=-DUSE_DEBUGGER -DUSE_TAB_COMPLETE
BOARD=NRF51TAG
OPTIMIZEFLAGS+=-Os
USE_BLUETOOTH=1

else ifdef NRF51822DK
EMBEDDED=1
SAVE_ON_FLASH=1
# Save on flash, but we still want the debugger and tab complete
DEFINES+=-DUSE_DEBUGGER -DUSE_TAB_COMPLETE
BOARD=NRF51822DK
OPTIMIZEFLAGS+=-Os
USE_BLUETOOTH=1
DEFINES += -DBOARD_PCA10028

# DFU_UPDATE_BUILD=1 # Uncomment this to build Espruino for a device firmware update over the air.

else ifdef NRF52832DK
EMBEDDED=1
BOARD=NRF52832DK
OPTIMIZEFLAGS+=-O3
USE_BLUETOOTH=1
USE_NET=1
USE_GRAPHICS=1
USE_NFC=1
USE_NEOPIXEL=1
DEFINES += -DBOARD_PCA10040 -DPCA10040

# DFU_UPDATE_BUILD=1 # Uncomment this to build Espruino for a device firmware update over the air.

else ifdef ARMINARM
EMBEDDED=1
USE_NET=1
USE_GRAPHICS=1
USE_FILESYSTEM=1
BOARD=ARMINARM
DEFINES+=-DESPRUINOBOARD
STLIB=STM32F10X_HD
PRECOMPILED_OBJS+=$(ROOT)/targetlibs/stm32f1/lib/startup_stm32f10x_hd.o
OPTIMIZEFLAGS+=-O3

else ifdef LCTECH_STM32F103RBT6
EMBEDDED=1
SAVE_ON_FLASH=1
BOARD=LCTECH_STM32F103RBT6
STLIB=STM32F10X_MD
PRECOMPILED_OBJS+=$(ROOT)/targetlibs/stm32f1/lib/startup_stm32f10x_md.o
OPTIMIZEFLAGS+=-Os

else ifdef CARAMBOLA
EMBEDDED=1
BOARD=CARAMBOLA
DEFINES += -DCARAMBOLA -DSYSFS_GPIO_DIR="\"/sys/class/gpio\""
LINUX=1
USE_FILESYSTEM=1
USE_GRAPHICS=1
USE_NET=1
USE_CRYPTO=1
USE_TLS=1

else ifdef DPTBOARD
EMBEDDED=1
BOARD=DPTBOARD
DEFINES += -DDPTBOARD -DSYSFS_GPIO_DIR="\"/sys/class/gpio\""
LINUX=1
OPENWRT_UCLIBC=1	# link with toolchain libc (uClibc or musl)
FIXED_OBJ_NAME=1	# when defined the linker will always produce 'espruino' as executable name, for packaging in .ipk, .deb,
USE_FILESYSTEM=1
USE_GRAPHICS=1
USE_NET=1

else ifdef ESP32
BOARD=ESP32
EMBEDDED=1
USE_NET=1
#USE_HASHLIB=1
USE_GRAPHICS=1
USE_CRYPTO=1
USE_TLS=1
USE_TELNET=1
USE_NEOPIXEL=1
USE_FILESYSTEM=1
DEFINES+=-DESP_PLATFORM -DESP32=1
OPTIMIZEFLAGS+=-Og

else
ifeq ($(shell uname -m),armv6l)
RASPBERRYPI=1 # just a guess

else ifeq ($(shell uname -n),beaglebone)
BEAGLEBONE=1

else ifeq ($(shell uname -n),arietta)
ARIETTA=1
endif

ifdef RASPBERRYPI
EMBEDDED=1
BOARD=RASPBERRYPI
DEFINES += -DRASPBERRYPI
LINUX=1
USE_FILESYSTEM=1
USE_GRAPHICS=1
#USE_LCD_SDL=1
USE_NET=1
USE_CRYPTO=1
USE_TLS=1
OPTIMIZEFLAGS+=-O3
ifneq ("$(wildcard /usr/local/include/wiringPi.h)","")
USE_WIRINGPI=1
else
DEFINES+=-DSYSFS_GPIO_DIR="\"/sys/class/gpio\""
$(info *************************************************************)
$(info *  WIRINGPI NOT FOUND, and you probably want it             *)
$(info *  see  http://wiringpi.com/download-and-install/           *)
$(info *************************************************************)
endif

else ifdef BEAGLEBONE
EMBEDDED=1
BOARD=BEAGLEBONE_BLACK
DEFINES += -DBEAGLEBONE_BLACK -DSYSFS_GPIO_DIR="\"/sys/class/gpio\""
LINUX=1
USE_FILESYSTEM=1
USE_GRAPHICS=1
USE_NET=1
USE_CRYPTO=1
USE_TLS=1

else ifdef ARIETTA
EMBEDDED=1
BOARD=ARIETTA_G25
DEFINES += -DARIETTA_G25 -DSYSFS_GPIO_DIR="\"/sys/class/gpio\""
LINUX=1
USE_FILESYSTEM=1
USE_GRAPHICS=1
USE_NET=1
USE_CRYPTO=1
USE_TLS=1

else
BOARD=LINUX
LINUX=1
USE_FILESYSTEM=1
USE_HASHLIB=1
USE_GRAPHICS=1
USE_CRYPTO=1
USE_TLS=1
USE_TELNET=1
#USE_LCD_SDL=1

ifdef MACOSX
USE_NET=1
else ifdef MINGW
#USE_NET=1 # http libs need some tweaks before net can compile
#LIBS += -lwsock32
DEFINES += -DHAS_STDLIB=1
else  # Linux
USE_NET=1
endif
endif
endif
endif # OLD_CODE

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
else ifeq ($(FAMILY),ESP8266)
USE_ESP8266=1
else ifeq ($(FAMILY),ESP32)
USE_ESP32=1
else ifdef EMW3165
USE_WICED=1
else
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
 ifdef EFM32
  DEFINES += -DDEBUG_EFM=1 -DDEBUG=1
 endif
DEFINES+=-DDEBUG
endif

ifdef PROFILE
OPTIMIZEFLAGS+=-pg
endif

# These are files for platform-specific libraries
TARGETSOURCES =

# Files that contains objects/functions/methods that will be
# exported to JS. The order here actually determines the order
# objects will be matched in. So for example Pins must come
# above ints, since a Pin is also matched as an int.
WRAPPERSOURCES = \
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
src/jswrap_serial.c \
src/jswrap_spi_i2c.c \
src/jswrap_stream.c \
src/jswrap_string.c \
src/jswrap_waveform.c \

# it is important that _pin comes before stuff which uses
# integers (as the check for int *includes* the chek for pin)
SOURCES = \
src/jslex.c \
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
# If we have enough flash, include the debugger
DEFINES+=-DUSE_DEBUGGER
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
SOURCES += \
libs/filesystem/fat_sd/spi_diskio.c
endif #USE_FILESYSTEM_SDIO
endif #!LINUX
endif #USE_FILESYSTEM

ifdef USE_MATH
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
 libs/network/wiznet/Ethernet/socket.c \
 libs/network/wiznet/W5500/w5500.c
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
 WRAPPERSOURCES += libs/network/esp32/jswrap_esp32_network.c \
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
 WRAPPERSOURCES += libs/network/esp8266/jswrap_esp8266_network.c \
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
  INCLUDE += -I$(ROOT)/libs/bluetooth
  WRAPPERSOURCES += libs/bluetooth/jswrap_bluetooth.c
endif

ifeq ($(BOARD),MICROBIT)
  INCLUDE += -I$(ROOT)/libs/microbit
  WRAPPERSOURCES += libs/microbit/jswrap_microbit.c
endif

ifeq ($(BOARD),PUCKJS)
  INCLUDE += -I$(ROOT)/libs/puckjs
  WRAPPERSOURCES += libs/puckjs/jswrap_puck.c
endif

ifdef USE_CRYPTO
  DEFINES += -DUSE_CRYPTO
  INCLUDE += -I$(ROOT)/libs/crypto
  INCLUDE += -I$(ROOT)/libs/crypto/mbedtls
  INCLUDE += -I$(ROOT)/libs/crypto/mbedtls/include
  WRAPPERSOURCES += libs/crypto/jswrap_crypto.c
  SOURCES += \
libs/crypto/mbedtls/library/sha1.c \
libs/crypto/mbedtls/library/sha256.c \
libs/crypto/mbedtls/library/sha512.c

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
  PRECOMPILED_OBJS += $(NRF5X_SDK_PATH)/components/nfc/t2t_lib/nfc_t2t_lib_gcc.a
endif

endif # BOOTLOADER ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ DON'T USE STUFF ABOVE IN BOOTLOADER

ifdef USB
DEFINES += -DUSB
endif

ifeq ($(FAMILY), STM32F1)
include make/family/STM32F1.make
endif #STM32F1

ifeq ($(FAMILY), STM32F3)
include make/family/STM32F3.make
endif #STM32F3

ifeq ($(FAMILY), STM32F4)
include make/family/STM32F4.make
endif #STM32F4

ifeq ($(FAMILY), STM32L4)
include make/family/STM32L4.make
endif #STM32L4

ifeq ($(FAMILY), NRF51)
include make/family/NRF51.make
endif # FAMILY == NRF51

ifeq ($(FAMILY), NRF52)
include make/family/NRF52.make
endif #FAMILY == NRF52

ifeq ($(FAMILY), EFM32GG)
include make/family/EFM32.make
endif #FAMILY == EFM32


ifdef CARAMBOLA
TOOLCHAIN_DIR=$(shell cd ~/workspace/carambola/staging_dir/toolchain-*/bin;pwd)
export STAGING_DIR=$(TOOLCHAIN_DIR)
export CCPREFIX=$(TOOLCHAIN_DIR)/mipsel-openwrt-linux-
endif

ifdef DPTBOARD
export STAGING_DIR=$(shell cd ~/breakoutopenwrt/staging_dir/toolchain-*/bin;pwd)
export CCPREFIX=$(STAGING_DIR)/mips-openwrt-linux-
endif

ifdef RASPBERRYPI
 ifneq ($(shell uname -m),armv6l)
  # eep. let's cross compile
  export CCPREFIX=targetlibs/raspberrypi/tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian/bin/arm-linux-gnueabihf-
 else
  # compiling in-place, so give it a normal name
  PROJ_NAME=espruino
 endif
endif

ifdef WICED
#WRAPPERSOURCES += targets/emw3165/jswrap_emw3165.c
endif

ifdef NUCLEO
WRAPPERSOURCES += targets/nucleo/jswrap_nucleo.c
endif

ifdef LINUX
DEFINES += -DLINUX
INCLUDE += -I$(ROOT)/targets/linux
SOURCES +=                              \
targets/linux/main.c                    \
targets/linux/jshardware.c
LIBS += -lpthread # thread lib for input processing
ifdef OPENWRT_UCLIBC
LIBS += -lc
else
LIBS += -lstdc++
endif
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

.PHONY:  proj

all: 	 proj

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
	$(Q)python scripts/build_jswrapper.py $(WRAPPERSOURCES) $(DEFINES) -B$(BOARD) -F$(WRAPPERFILE)

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

ifdef FIXED_OBJ_NAME
link=$(LD) $(LDFLAGS) -o espruino $(OBJS) $(LIBS)
else
link=$(LD) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)
endif

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

.s.o:
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

clean:
	@echo Cleaning targets
	$(Q)find . -name \*.o | grep -v arm-bcm2708 | xargs rm -f
	$(Q)rm -f $(ROOT)/gen/*.c $(ROOT)/gen/*.h $(ROOT)/gen/*.ld
	$(Q)rm -f $(PROJ_NAME).elf
	$(Q)rm -f $(PROJ_NAME).hex
	$(Q)rm -f $(PROJ_NAME).bin
	$(Q)rm -f $(PROJ_NAME).srec
	$(Q)rm -f $(PROJ_NAME).lst

# start make like this "make varsonly" to get all variables created and used during make process without compiling
# this helps to better understand linking, or to find oddities
varsonly:
	$(foreach v, $(.VARIABLES), $(info $(v) = $($(v))))
