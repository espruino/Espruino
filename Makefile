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
# WIZNET=1                # If compiling for a non-linux target that has internet support, use WIZnet support
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

<<<<<<< HEAD
# ---------------------------------------------------------------------------------
# When adding stuff here, also remember build_pininfo, platform_config.h, jshardware.c
# TODO: Load more of this out of the BOARDNAME.py files if at all possible (see next section)
# ---------------------------------------------------------------------------------
ifdef ESPRUINO_1V3
EMBEDDED=1
DEFINES+=-DESPRUINO_1V3
USE_NET=1
USE_GRAPHICS=1
USE_FILESYSTEM=1
USE_HASHLIB=1
USE_NEOPIXEL=1
BOARD=ESPRUINOBOARD
STLIB=STM32F10X_XL
PRECOMPILED_OBJS+=$(ROOT)/targetlibs/stm32f1/lib/startup_stm32f10x_hd.o
OPTIMIZEFLAGS+=-Os

else ifdef PICO_1V3
EMBEDDED=1
#USE_DFU=1
DEFINES+= -DUSE_USB_OTG_FS=1  -DPICO -DPICO_1V3
USE_USB_HID=1
USE_NET=1
USE_GRAPHICS=1
USE_TV=1
USE_HASHLIB=1
USE_FILESYSTEM=1
USE_CRYPTO=1
USE_TLS=1
USE_NEOPIXEL=1
BOARD=PICO_R1_3
STLIB=STM32F401xE
PRECOMPILED_OBJS+=$(ROOT)/targetlibs/stm32f4/lib/startup_stm32f401xx.o
OPTIMIZEFLAGS+=-Os

else ifdef ESPRUINOWIFI
EMBEDDED=1
#USE_DFU=1
DEFINES+= -DUSE_USB_OTG_FS=1  -DESPRUINOWIFI
USE_USB_HID=1
USE_NET=1
USE_GRAPHICS=1
USE_TV=1
USE_HASHLIB=1
USE_FILESYSTEM=1
USE_CRYPTO=1
USE_TLS=1
USE_NEOPIXEL=1
BOARD=ESPRUINOWIFI
STLIB=STM32F411xE
#PRECOMPILED_OBJS+=$(ROOT)/targetlibs/stm32f4/lib/startup_stm32f40_41xxx.o # <- this seems like overkill
PRECOMPILED_OBJS+=$(ROOT)/targetlibs/stm32f4/lib/startup_stm32f401xx.o
OPTIMIZEFLAGS+=-Os

else ifdef EFM32GGSTK
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

else ifdef PUCKJS
EMBEDDED=1
BOARD=PUCKJS
OPTIMIZEFLAGS+=-O3
USE_BLUETOOTH=1
USE_NET=1
USE_GRAPHICS=1
#USE_HASHLIB=1
#USE_FILESYSTEM=1
USE_CRYPTO=1
#USE_TLS=1
USE_NFC=1
USE_NEOPIXEL=1

else ifdef ECU
# Gordon's car ECU (extremely beta!)
USE_TRIGGER=1
USE_FILESYSTEM=1
DEFINES +=-DECU -DSTM32F4DISCOVERY
DEFINES += -DUSE_USB_OTG_FS=1
BOARD=ECU
STLIB=STM32F4XX
PRECOMPILED_OBJS+=$(ROOT)/targetlibs/stm32f4/lib/startup_stm32f4xx.o
OPTIMIZEFLAGS+=-O3

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
DEFINES+=-DESP_PLATFORM -DESP32=1
OPTIMIZEFLAGS+=-Og

else ifdef ESP8266_BOARD
RELEASE=1
EMBEDDED=1
USE_NET=1
USE_TELNET=1
USE_GRAPHICS=1
USE_CRYPTO=1
USE_NEOPIXEL=1
BOARD=ESP8266_BOARD
# Enable link-time optimisations (inlining across files), use -Os 'cause else we end up with
# too large a firmware (-Os is -O2 without optimizations that increase code size)
ifndef DISABLE_LTO
OPTIMIZEFLAGS+=-Os -g -std=gnu11 -fgnu89-inline -fno-fat-lto-objects -Wl,--allow-multiple-definition
#OPTIMIZEFLAGS+=-DLINK_TIME_OPTIMISATION # this actually slows things down!
else
# DISABLE_LTO is necessary in order to analyze static string sizes (see: topstring makefile target)
OPTIMIZEFLAGS+=-Os -std=gnu11 -fgnu89-inline -Wl,--allow-multiple-definition
endif


ifdef FLASH_4MB_C1
ESP_FLASH_MAX       ?= 933888   # max bin file: 912KB
ESP_FLASH_SIZE      ?= 6        # 6->4MB (1024KB+1024KB)       
ESP_FLASH_MODE      ?= 0        # 0->QIO, 2->DIO
ESP_FLASH_FREQ_DIV  ?= 15       # 15->80Mhz
ET_FS               ?= 4MB-c1   # 32Mbit (4MB) flash size in esptool flash command
ET_FF               ?= 80m      # 80Mhz flash speed in esptool flash command
ET_BLANK            ?= 0x3FE000 # where to flash blank.bin to erase wireless settings
ET_DEFAULTS         ?= 0x3FC000 # where to flash esp_init_data_default.bin to default SDK settings
else ifdef FLASH_4MB
ESP_FLASH_MAX       ?= 479232   # max bin file: 468KB
ESP_FLASH_SIZE      ?= 4        # 4->4MB (512KB+512KB)
ESP_FLASH_MODE      ?= 0        # 0->QIO, 2->DIO
ESP_FLASH_FREQ_DIV  ?= 15       # 15->80Mhz
ET_FS               ?= 4MB      # 32Mbit (4MB) flash size in esptool flash command
ET_FF               ?= 80m      # 80Mhz flash speed in esptool flash command
ET_BLANK            ?= 0x3FE000 # where to flash blank.bin to erase wireless settings
ET_DEFAULTS         ?= 0x3FC000 # where to flash esp_init_data_default.bin to default SDK settings
else ifdef 2MB
ESP_FLASH_MAX       ?= 479232   # max bin file: 468KB
ESP_FLASH_SIZE      ?= 3        # 3->2MB (512KB+512KB)
ESP_FLASH_MODE      ?= 0        # 0->QIO, 2->DIO
ESP_FLASH_FREQ_DIV  ?= 15       # 15->80Mhz
ET_FS               ?= 2MB      # 16Mbit (2MB) flash size in esptool flash command
ET_FF               ?= 80m      # 80Mhz flash speed in esptool flash command
ET_BLANK            ?= 0x1FE000 # where to flash blank.bin to erase wireless settings
ET_DEFAULTS         ?= 0x1FC000 # where to flash esp_init_data_default.bin to default SDK settings
else ifdef 1MB
ESP_FLASH_MAX       ?= 479232   # max bin file: 468KB
ESP_FLASH_SIZE      ?= 2       # 2->1MB (512KB+512KB)
ESP_FLASH_MODE      ?= 0       # 0->QIO, 2->DIO
ESP_FLASH_FREQ_DIV  ?= 15      # 15->80Mhz
ET_FS               ?= 1MB     # 8Mbit (1MB) flash size in esptool flash command
ET_FF               ?= 80m     # 80Mhz flash speed in esptool flash command
ET_BLANK            ?= 0xFE000 # where to flash blank.bin to erase wireless settings
ET_DEFAULTS         ?= 0xFC000 # where to flash esp_init_data_default.bin to default SDK settings
else # 512KB
ESP_FLASH_MAX       ?= 479232   # max bin file: 468KB
ESP_FLASH_SIZE      ?= 0       # 0->512KB
ESP_FLASH_MODE      ?= 0       # 0->QIO
ESP_FLASH_FREQ_DIV  ?= 0       # 0->40Mhz
ET_FS               ?= 512KB   # 4Mbit (512KB) flash size in esptool flash command
ET_FF               ?= 40m     # 40Mhz flash speed in esptool flash command
ET_BLANK            ?= 0x7E000 # where to flash blank.bin to erase wireless settings
ET_DEFAULTS         ?= 0x7C000 # where to flash esp_init_data_default.bin to default SDK settings
endif

FLASH_BAUD ?= 460800 # The flash baud rate
# End of ESP8266_BOARD
else
ifeq ($(shell uname -m),armv6l)
RASPBERRYPI=1 # just a guess

else ifeq ($(shell uname -n),beaglebone)
BEAGLEBONE=1
=======
>>>>>>> d1e384e36df31ceb01005c413f31523cd7d8e5a8

ifeq ($(BOARD),)
 # Try and guess board names
 ifeq ($(shell uname -m),armv6l)
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
  DEFINES += -DBLUETOOTH
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

ifdef USE_NUCLEO
WRAPPERSOURCES += targets/nucleo/jswrap_nucleo.c
endif

ifdef WICED
WRAPPERSOURCES += targets/emw3165/jswrap_emw3165.c
endif

endif # BOOTLOADER ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ DON'T USE STUFF ABOVE IN BOOTLOADER

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
<<<<<<< HEAD
# Linking the esp8266... The Espruino source files get compiled into the .text section. The
# Espressif SDK libraries have .text and .irom0 sections. We need to put the libraries' .text into
# .iram0 (32KB on chip instruction ram) and we need to put the Esprunio .text and the libraries'
# .irom0 into .irom0 (demand-cached from flash). We do this dance by pre-linking the Espruino
# objects, then renaming .text to .irom0, and then finally linking with the SDK libraries.
# Note that a previous method of renaming .text to .irom0 in each object file doesn't work when
# we enable the link-time optimizer for inlining because it generates fresh code that all ends
# up in .iram0.
# We generate two binaries in order to support over-the-air updates, one per
# OTA partition (Espressif calls these user1.bin and user2.bin). In the 512KB flash case, there
# is only space for the first binary and updates are not possible. So we're really abusing the
# flash layout in that case because we tell the SDK that we have two 256KB partitions when in
# reality we're using one 512KB partition. This works out because the SDK doesn't use the
# user setting area that sits between the two 256KB partitions, so we can merrily use it for
# code.
ESP_ZIP     = $(PROJ_NAME).tgz
ESP_COMBINED512 = $(PROJ_NAME)_combined_512.bin
USER1_BIN   = espruino_esp8266_user1.bin
USER2_BIN   = espruino_esp8266_user2.bin
USER1_ELF   = espruino_esp8266_user1.elf
USER2_ELF   = espruino_esp8266_user2.elf
PARTIAL     = espruino_esp8266_partial.o
ifdef FLASH_4MB_C1
LD_SCRIPT1  = ./targets/esp8266/eagle.app.v6.new.2048.app1.ld
LD_SCRIPT2  = ./targets/esp8266/eagle.app.v6.new.2048.app2.ld
else
LD_SCRIPT1  = ./targets/esp8266/eagle.app.v6.new.1024.app1.ld
LD_SCRIPT2  = ./targets/esp8266/eagle.app.v6.new.1024.app2.ld
endif
APPGEN_TOOL = $(ESP8266_SDK_ROOT)/tools/gen_appbin.py
BOOTLOADER  = $(ESP8266_SDK_ROOT)/bin/boot_v1.6.bin
BLANK       = $(ESP8266_SDK_ROOT)/bin/blank.bin
INIT_DATA   = $(ESP8266_SDK_ROOT)/bin/esp_init_data_default.bin

proj: $(USER1_BIN) $(USER2_BIN) $(ESP_ZIP)
combined: $(ESP_COMBINED512)

# generate partially linked .o with all Esprunio source files linked
$(PARTIAL): $(OBJS) $(LINKER_FILE)
	@echo LD $@
ifdef USE_CRYPTO
	$(Q)$(OBJCOPY) --rename-section .rodata=.irom0.text libs/crypto/mbedtls/library/sha1.o
	$(Q)$(OBJCOPY) --rename-section .rodata=.irom0.text libs/crypto/mbedtls/library/sha256.o
	$(Q)$(OBJCOPY) --rename-section .rodata=.irom0.text libs/crypto/mbedtls/library/sha512.o
endif
	$(Q)$(LD) $(OPTIMIZEFLAGS) -nostdlib -Wl,--no-check-sections -Wl,-static -r -o $@ $(OBJS)
	$(Q)$(OBJCOPY) --rename-section .text=.irom0.text --rename-section .literal=.irom0.literal $@

# generate fully linked 'user1' .elf using linker script for first OTA partition
$(USER1_ELF): $(PARTIAL) $(LINKER_FILE)
	@echo LD $@
	$(Q)$(LD) $(LDFLAGS) -T$(LD_SCRIPT1) -o $@ $(PARTIAL) -Wl,--start-group $(LIBS) -Wl,--end-group
	$(Q)$(OBJDUMP) --headers -j .irom0.text -j .text $@ | tail -n +4
	@echo To disassemble: $(OBJDUMP) -d -l -x $@
	$(OBJDUMP) -d -l -x $@ >espruino_esp8266_user1.lst

# generate fully linked 'user2' .elf using linker script for second OTA partition
$(USER2_ELF): $(PARTIAL) $(LINKER_FILE)
	@echo LD $@
	$(Q)$(LD) $(LDFLAGS) -T$(LD_SCRIPT2) -o $@ $(PARTIAL) -Wl,--start-group $(LIBS) -Wl,--end-group
	@echo To disassemble: $(OBJDUMP) -d -l -x $@

# generate binary image for user1, i.e. first OTA partition
$(USER1_BIN): $(USER1_ELF)
	$(Q)$(OBJCOPY) --only-section .text -O binary $(USER1_ELF) eagle.app.v6.text.bin
	$(Q)$(OBJCOPY) --only-section .data -O binary $(USER1_ELF) eagle.app.v6.data.bin
	$(Q)$(OBJCOPY) --only-section .rodata -O binary $(USER1_ELF) eagle.app.v6.rodata.bin
	$(Q)$(OBJCOPY) --only-section .irom0.text -O binary $(USER1_ELF) eagle.app.v6.irom0text.bin
	@ls -ls eagle*bin
	$(Q)COMPILE=gcc python $(APPGEN_TOOL) $(USER1_ELF) 2 $(ESP_FLASH_MODE) $(ESP_FLASH_FREQ_DIV) $(ESP_FLASH_SIZE) 0 >/dev/null
	$(Q) rm -f eagle.app.v6.*.bin
	$(Q) mv eagle.app.flash.bin $@
	@echo "** user1.bin uses $$( stat $(STAT_FLAGS) $@) bytes of" $(ESP_FLASH_MAX) "available"
	@if [ $$( stat $(STAT_FLAGS) $@) -gt $$(( $(ESP_FLASH_MAX) )) ]; then echo "$@ too big!"; false; fi

# generate binary image for user2, i.e. second OTA partition
# we make this rule dependent on user1.bin in order to serialize the two rules because they use
# stupid static filenames (go blame the Espressif tool)
$(USER2_BIN): $(USER2_ELF) $(USER1_BIN)
	$(Q)$(OBJCOPY) --only-section .text -O binary $(USER2_ELF) eagle.app.v6.text.bin
	$(Q)$(OBJCOPY) --only-section .data -O binary $(USER2_ELF) eagle.app.v6.data.bin
	$(Q)$(OBJCOPY) --only-section .rodata -O binary $(USER2_ELF) eagle.app.v6.rodata.bin
	$(Q)$(OBJCOPY) --only-section .irom0.text -O binary $(USER2_ELF) eagle.app.v6.irom0text.bin
	$(Q)COMPILE=gcc python $(APPGEN_TOOL) $(USER2_ELF) 2 $(ESP_FLASH_MODE) $(ESP_FLASH_FREQ_DIV) $(ESP_FLASH_SIZE) 1 >/dev/null
	$(Q) rm -f eagle.app.v6.*.bin
	$(Q) mv eagle.app.flash.bin $@

$(ESP_ZIP): $(USER1_BIN) $(USER2_BIN)
	$(Q)rm -rf build/$(basename $(ESP_ZIP))
	$(Q)mkdir -p build/$(basename $(ESP_ZIP))
	$(Q)cp $(USER1_BIN) $(USER2_BIN) scripts/wiflash.sh $(BLANK) \
	  $(INIT_DATA) $(BOOTLOADER) targets/esp8266/README_flash.txt \
	  build/$(basename $(ESP_ZIP))
	$(Q)tar -C build -zcf $(ESP_ZIP) ./$(basename $(ESP_ZIP))

# Combined 512k binary that includes everything that's needed and can be
# flashed to 0 in 512k parts
$(ESP_COMBINED512): $(USER1_BIN) $(USER2_BIN)
	dd if=/dev/zero ibs=1k count=512 | tr "\000" "\377" > $@
	dd bs=1 if=$(BOOTLOADER) of=$@ conv=notrunc
	dd bs=1 seek=4096 if=$(USER1_BIN) of=$@ conv=notrunc
	dd bs=1 seek=507904 if=$(INIT_DATA) of=$@ conv=notrunc

# Analyze all the .o files and rank them by the amount of static string area used, useful to figure
# out where to optimize and move strings to flash
# IMPORTANT: this only works if DISABLE_LTO is defined, e.g. `DISABLE_LTO=1 make`
topstrings: $(PARTIAL)
	$(Q)for f in `find . -name \*.o`; do \
	  str=$$($(OBJDUMP) -j .rodata.str1.1 -j .rodata.str1.4 -h $$f 2>/dev/null | \
	    egrep -o 'rodata.str1.. [0-9a-f]+' | \
	    awk $$(expr match "$$(awk --version)" "GNU.*" >/dev/null && echo --non-decimal-data) \
	      -e '{printf "%d\n", ("0x" $$2);}'); \
	  [ "$$str" ] && echo "$$str $$f"; \
	done | \
	sort -rn >topstrings
	$(Q)echo "Top 20 from ./topstrings:"
	$(Q)head -20 topstrings
	$(Q)echo "To get details: $(OBJDUMP) -j .rodata.str1.1 -j .rodata.str1.4 -s src/FILENAME.o"

# Same as topstrings but consider all read-only data
topreadonly: $(PARTIAL)
	$(Q)for f in `find . -name \*.o`; do \
	  str=$$($(OBJDUMP) -j .rodata -h $$f 2>/dev/null | \
	    egrep -o 'rodata +[0-9a-f]+' | \
	    awk $$(expr match "$$(awk --version)" "GNU.*" >/dev/null && echo --non-decimal-data) \
	      -e '{printf "%d\n", ("0x" $$2);}'); \
	  [ "$$str" ] && echo "$$str $$f"; \
	done | \
	sort -rn >topreadonly
	$(Q)echo "Top 20 from ./topreadonly:"
	$(Q)head -20 topreadonly
	$(Q)echo "To get details: $(OBJDUMP) -j .rodata -s src/FILENAME.o"


flash: all $(USER1_BIN) $(USER2_BIN)
ifndef COMPORT
	$(error "In order to flash, we need to have the COMPORT variable defined")
endif
	-$(ESPTOOL) --port $(COMPORT) --baud $(FLASH_BAUD) write_flash --flash_freq $(ET_FF) --flash_mode qio --flash_size $(ET_FS) 0x0000 $(BOOTLOADER) 0x1000 $(USER1_BIN) $(ET_DEFAULTS) $(INIT_DATA) $(ET_BLANK) $(BLANK)

# just flash user1 and don't mess with bootloader or wifi settings
quickflash: all $(USER1_BIN) $(USER2_BIN)
ifndef COMPORT
	$(error "In order to flash, we need to have the COMPORT variable defined")
endif
	-$(ESPTOOL) --port $(COMPORT) --baud $(FLASH_BAUD) write_flash 0x1000 $(USER1_BIN)

wiflash: all $(USER1_BIN) $(USER2_BIN)
ifndef ESPHOSTNAME
	$(error "In order to flash over wifi, we need to have the ESPHOSTNAME variable defined")
endif
	./scripts/wiflash.sh $(ESPHOSTNAME) $(USER1_BIN) $(USER2_BIN)

#else ifdef WICED
#
#proj: $(WICED_ROOT)/apps/snip/espruino/espruino_lib.o
#
#$(PROJ_NAME).o: $(OBJS)
#	@echo LD $@
#	$(Q)$(LD) $(OPTIMIZEFLAGS) -nostdlib -Wl,--no-check-sections -Wl,-static -r -o $@ $(OBJS)
#
#$(WICED_ROOT)/apps/snip/espruino/espruino_lib.o: $(PROJ_NAME).o
#	cp $< $@

else # embedded, so generate bin, etc ---------------------------

$(PROJ_NAME).elf: $(OBJS) $(LINKER_FILE)
	@echo $($(quiet_)link)
	@$(call link)

$(PROJ_NAME).lst : $(PROJ_NAME).elf
	@echo $($(quiet_)obj_dump)
	@$(call obj_dump)

$(PROJ_NAME).hex: $(PROJ_NAME).elf
	@echo $(call $(quiet_)obj_to_bin,ihex,hex)
	@$(call obj_to_bin,ihex,hex)
ifdef SOFTDEVICE # Shouldn't do this when we want to be able to perform DFU OTA!
 ifdef USE_BOOTLOADER
  ifdef DFU_UPDATE_BUILD
	@echo Not merging softdevice or bootloader with application
	# nrfutil  pkg generate --help
	nrfutil pkg generate $(PROJ_NAME).zip --application $(PROJ_NAME).hex --application-version 0xff --hw-version 52 --sd-req 0x8C --key-file targets/nrf5x_dfu/dfu_private_key.pem
  else
  ifdef BOOTLOADER
	@echo Not merging anything with bootloader
  else
	@echo Merging SoftDevice and Bootloader
        # We can build a DFU settings file we can merge in...
	# nrfutil settings generate --family NRF52 --application $(PROJ_NAME).hex --application-version 0xff --bootloader-version 0xff --bl-settings-version 1 dfu_settings.hex
	@echo FIXME - had to set --overlap=replace
	python scripts/hexmerge.py --overlap=replace $(SOFTDEVICE) $(NRF_BOOTLOADER) $(PROJ_NAME).hex -o tmp.hex
	mv tmp.hex $(PROJ_NAME).hex
  endif
  endif
 else
	@echo Merging SoftDevice
	python scripts/hexmerge.py $(SOFTDEVICE) $(PROJ_NAME).hex -o tmp.hex
	mv tmp.hex $(PROJ_NAME).hex
 endif # USE_BOOTLOADER
endif # SOFTDEVICE

$(PROJ_NAME).srec : $(PROJ_NAME).elf
	@echo $(call $(quiet_)obj_to_bin,srec,srec)
	@$(call obj_to_bin,srec,srec)

$(PROJ_NAME).bin : $(PROJ_NAME).elf
	@echo $(call $(quiet_)obj_to_bin,binary,bin)
	@$(call obj_to_bin,binary,bin)
ifndef TRAVIS
	bash scripts/check_size.sh $(PROJ_NAME).bin
endif

ifdef NRF5X
proj: $(PROJ_NAME).lst $(PROJ_NAME).hex
else
proj: $(PROJ_NAME).lst $(PROJ_NAME).bin $(PROJ_NAME).hex
endif

#proj: $(PROJ_NAME).lst $(PROJ_NAME).hex $(PROJ_NAME).srec $(PROJ_NAME).bin

flash: all
ifdef USE_DFU
	sudo dfu-util -a 0 -s 0x08000000 -D $(PROJ_NAME).bin
else ifdef OLIMEXINO_STM32_BOOTLOADER
	@echo Olimexino Serial bootloader
	dfu-util -a1 -d 0x1EAF:0x0003 -D $(PROJ_NAME).bin
else ifdef NUCLEO
	if [ -d "/media/$(USER)/NUCLEO" ]; then cp $(PROJ_NAME).bin /media/$(USER)/NUCLEO;sync; fi
	if [ -d "/media/NUCLEO" ]; then cp $(PROJ_NAME).bin /media/NUCLEO;sync; fi
else ifdef MICROBIT
	if [ -d "/media/$(USER)/MICROBIT" ]; then cp $(PROJ_NAME).hex /media/$(USER)/MICROBIT;sync; fi
	if [ -d "/media/MICROBIT" ]; then cp $(PROJ_NAME).hex /media/MICROBIT;sync; fi
else ifdef NRF5X
	if type nrfjprog 2>/dev/null; then nrfjprog --family $(FAMILY) --clockspeed 50000 --program $(PROJ_NAME).hex --chiperase --reset; \
	elif [ -d "/media/$(USER)/JLINK" ]; then cp $(PROJ_NAME).hex /media/$(USER)/JLINK;sync; \
	elif [ -d "/media/JLINK" ]; then cp $(PROJ_NAME).hex /media/JLINK;sync; fi
else
	@echo ST-LINK flash
	st-flash --reset write $(PROJ_NAME).bin $(BASEADDRESS)
endif

serialflash: all
	@echo STM32 inbuilt serial bootloader, set BOOT0=1, BOOT1=0
	python scripts/stm32loader.py -b 460800 -a $(BASEADDRESS) -ew $(STM32LOADER_FLAGS) $(PROJ_NAME).bin
#	python scripts/stm32loader.py -b 460800 -a $(BASEADDRESS) -ewv $(STM32LOADER_FLAGS) $(PROJ_NAME).bin

gdb:
	@echo "target extended-remote :4242" > gdbinit
	@echo "file $(PROJ_NAME).elf" >> gdbinit
	#echo "load" >> gdbinit
	@echo "break main" >> gdbinit
	@echo "break HardFault_Handler" >> gdbinit
	$(GDB) -x gdbinit
	rm gdbinit
=======
include make/targets/ESP8266.make
else # ARM/etc, so generate bin, etc ---------------------------
include make/targets/ARM.make
>>>>>>> d1e384e36df31ceb01005c413f31523cd7d8e5a8
endif	    # ---------------------------------------------------

else # NO_COMPILE
# log WRAPPERSOURCES to help Firmware creation tool
$(info WRAPPERSOURCES=$(WRAPPERSOURCES));
endif

clean:
	@echo Cleaning targets
	$(Q)find . -name \*.o | grep -v arm-bcm2708 | xargs rm -f
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
