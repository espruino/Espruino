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
# ESPRUINO_1V0=1          # Espruino board rev 1.0
# ESPRUINO_1V1=1          # Espruino board rev 1.1 and 1.2
# ESPRUINO_1V3=1          # Espruino board rev 1.3 and rev 1v4
# PICO_1V0=1              # Espruino Pico board rev 1.0
# PICO_1V1=1              # Espruino Pico board rev 1.1
# PICO_1V2=1              # Espruino Pico board rev 1.2
# PICO_1V3=1              # Espruino Pico board rev 1.3
# OLIMEXINO_STM32=1       # Olimexino STM32
# MAPLERET6_STM32=1       # Limited production Leaflabs Maple r5 with a STM32F103RET6
# MAPLEMINI=1             # Leaflabs Maple Mini
# EMBEDDED_PI=1           # COOCOX STM32 Embedded Pi boards
# HYSTM32_24=1            # HY STM32 2.4 Ebay boards
# HYSTM32_28=1            # HY STM32 2.8 Ebay boards
# HYSTM32_32=1            # HY STM32 3.2 VCT6 Ebay boards
# STM32VLDISCOVERY=1
# STM32F3DISCOVERY=1
# STM32F4DISCOVERY=1
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
# LPC1768=1 # beta
# LCTECH_STM32F103RBT6=1 # LC Technology STM32F103RBT6 Ebay boards
# ARMINARM=1
# NUCLEOF401RE=1
# NUCLEOF411RE=1
# MINISTM32_STRIVE=1
# MINISTM32_ANGLED_VE=1
# MINISTM32_ANGLED_VG=1
# ESP8266_BOARD=1         # Same as ESP8266_512KB
# ESP8266_4MB=1           # ESP8266 with 4MB flash: 512KB+512KB firmware + 3MB SPIFFS
# ESP8266_2MB=1           # ESP8266 with 2MB flash: 512KB+512KB firmware + 3MB SPIFFS
# ESP8266_1MB=1           # ESP8266 with 1MB flash: 512KB+512KB firmware + 32KB SPIFFS
# ESP8266_512KB=1         # ESP8266 with 512KB flash: 512KB firmware + 32KB SPIFFS
# EMW3165                 # MXCHIP EMW3165: STM32F411CE, BCM43362, 512KB flash 128KB RAM
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
#
# WIZNET=1                # If compiling for a non-linux target that has internet support, use WIZnet support, not TI CC3000
ifndef SINGLETHREAD
MAKEFLAGS=-j5 # multicore
endif

# backwards compatibility
ifdef ESP8266_BOARD
ESP8266_512KB=1
endif

INCLUDE=-I$(ROOT) -I$(ROOT)/targets -I$(ROOT)/src -I$(ROOT)/gen
LIBS=
DEFINES=
CFLAGS=-Wall -Wextra -Wconversion -Werror=implicit-function-declaration
LDFLAGS=-Winline
OPTIMIZEFLAGS=
#-fdiagnostics-show-option - shows which flags can be used with -Werror
DEFINES+=-DGIT_COMMIT=$(shell git log -1 --format="%H")

# Espruino flags...
USE_MATH=1

ifeq ($(shell uname),Darwin)
MACOSX=1
CFLAGS+=-D__MACOSX__
endif

ifeq ($(OS),Windows_NT)
MINGW=1
endif

ifdef RELEASE
# force no asserts to be compiled in
DEFINES += -DNO_ASSERT -DRELEASE
endif

LATEST_RELEASE=$(shell git tag | grep RELEASE_ | sort | tail -1)
# use egrep to count lines instead of wc to avoid whitespace error on Mac
COMMITS_SINCE_RELEASE=$(shell git log --oneline $(LATEST_RELEASE)..HEAD | egrep -c .)
ifneq ($(COMMITS_SINCE_RELEASE),0)
DEFINES += -DBUILDNUMBER=\"$(COMMITS_SINCE_RELEASE)\"
endif


CWD = $(CURDIR)
ROOT = $(CWD)
PRECOMPILED_OBJS=
PLATFORM_CONFIG_FILE=gen/platform_config.h
BASEADDRESS=0x08000000

# ---------------------------------------------------------------------------------
# When adding stuff here, also remember build_pininfo, platform_config.h, jshardware.c
# TODO: Load more of this out of the BOARDNAME.py files if at all possible (see next section)
# ---------------------------------------------------------------------------------
ifdef ESPRUINO_1V0
EMBEDDED=1
#USE_NET=1
USE_GRAPHICS=1
USE_FILESYSTEM=1
BOARD=ESPRUINOBOARD_R1_0
DEFINES+=-DESPRUINOBOARD
STLIB=STM32F10X_XL
PRECOMPILED_OBJS+=$(ROOT)/targetlibs/stm32f1/lib/startup_stm32f10x_hd.o
OPTIMIZEFLAGS+=-O3

else ifdef ESPRUINO_1V1
EMBEDDED=1
DEFINES+=-DESPRUINO_1V1
USE_NET=1
USE_GRAPHICS=1
USE_FILESYSTEM=1
BOARD=ESPRUINOBOARD_R1_1
DEFINES+=-DESPRUINOBOARD
STLIB=STM32F10X_XL
PRECOMPILED_OBJS+=$(ROOT)/targetlibs/stm32f1/lib/startup_stm32f10x_hd.o
OPTIMIZEFLAGS+=-Os

else ifdef ESPRUINO_1V3
EMBEDDED=1
DEFINES+=-DESPRUINO_1V3
USE_NET=1
USE_GRAPHICS=1
USE_FILESYSTEM=1
USE_TV=1
USE_HASHLIB=1
BOARD=ESPRUINOBOARD
STLIB=STM32F10X_XL
PRECOMPILED_OBJS+=$(ROOT)/targetlibs/stm32f1/lib/startup_stm32f10x_hd.o
OPTIMIZEFLAGS+=-Os

else ifdef PICO_1V0
EMBEDDED=1
USE_DFU=1
DEFINES+= -DUSE_USB_OTG_FS=1  -DPICO -DPICO_1V0
USE_GRAPHICS=1
BOARD=PICO_R1_0
STLIB=STM32F401xE
PRECOMPILED_OBJS+=$(ROOT)/targetlibs/stm32f4/lib/startup_stm32f401xx.o
OPTIMIZEFLAGS+=-O3

else ifdef PICO_1V1
EMBEDDED=1
USE_DFU=1
DEFINES+= -DUSE_USB_OTG_FS=1  -DPICO -DPICO_1V1
USE_NET=1
USE_GRAPHICS=1
USE_TV=1
BOARD=PICO_R1_1
STLIB=STM32F401xE
PRECOMPILED_OBJS+=$(ROOT)/targetlibs/stm32f4/lib/startup_stm32f401xx.o
OPTIMIZEFLAGS+=-O3

else ifdef PICO_1V2
EMBEDDED=1
#USE_DFU=1
DEFINES+= -DUSE_USB_OTG_FS=1  -DPICO -DPICO_1V2
USE_NET=1
USE_GRAPHICS=1
USE_TV=1
USE_HASHLIB=1
BOARD=PICO_R1_2
STLIB=STM32F401xE
PRECOMPILED_OBJS+=$(ROOT)/targetlibs/stm32f4/lib/startup_stm32f401xx.o
OPTIMIZEFLAGS+=-O3

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
BOARD=PICO_R1_3
STLIB=STM32F401xE
PRECOMPILED_OBJS+=$(ROOT)/targetlibs/stm32f4/lib/startup_stm32f401xx.o
OPTIMIZEFLAGS+=-O3

else ifdef OLIMEXINO_STM32
EMBEDDED=1
SAVE_ON_FLASH=1
USE_FILESYSTEM=1
BOARD=OLIMEXINO_STM32
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
BOARD=HYSTM32_24
STLIB=STM32F10X_HD
PRECOMPILED_OBJS+=$(ROOT)/targetlibs/stm32f1/lib/startup_stm32f10x_hd.o
OPTIMIZEFLAGS+=-O3

else ifdef HYSTM32_28
EMBEDDED=1
USE_GRAPHICS=1
USE_LCD_FSMC=1
DEFINES+=-DILI9325_BITBANG # bit-bang the LCD driver
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
BOARD=HYSTM32_32
STLIB=STM32F10X_HD
PRECOMPILED_OBJS+=$(ROOT)/targetlibs/stm32f1/lib/startup_stm32f10x_hd.o
OPTIMIZEFLAGS+=-Os

else ifdef NUCLEOF401RE
EMBEDDED=1
NUCLEO=1
USE_GRAPHICS=1
USE_NET=1
BOARD=NUCLEOF401RE
STLIB=STM32F401xE
PRECOMPILED_OBJS+=$(ROOT)/targetlibs/stm32f4/lib/startup_stm32f401xx.o
OPTIMIZEFLAGS+=-O3

else ifdef NUCLEOF411RE
EMBEDDED=1
NUCLEO=1
USE_GRAPHICS=1
USE_NET=1
BOARD=NUCLEOF411RE
STLIB=STM32F401xE
PRECOMPILED_OBJS+=$(ROOT)/targetlibs/stm32f4/lib/startup_stm32f401xx.o
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
DEFINES += -DUSE_USB_OTG_FS=1
BOARD=STM32F4DISCOVERY
STLIB=STM32F407xx
PRECOMPILED_OBJS+=$(ROOT)/targetlibs/stm32f4/lib/startup_stm32f40_41xxx.o
OPTIMIZEFLAGS+=-O3

else ifdef STM32F401CDISCOVERY
EMBEDDED=1
USE_NET=1
USE_GRAPHICS=1
DEFINES += -DUSE_USB_OTG_FS=1
BOARD=STM32F401CDISCOVERY
STLIB=STM32F401xE
PRECOMPILED_OBJS+=$(ROOT)/targetlibs/stm32f4/lib/startup_stm32f401xx.o
OPTIMIZEFLAGS+=-O3

else ifdef STM32F429IDISCOVERY
EMBEDDED=1
USE_GRAPHICS=1
DEFINES += -DUSE_USB_OTG_FS=1
BOARD=STM32F429IDISCOVERY
STLIB=STM32F429xx
PRECOMPILED_OBJS+=$(ROOT)/targetlibs/stm32f4/lib/startup_stm32f429_439xx.o
OPTIMIZEFLAGS+=-O3

else ifdef SMARTWATCH
EMBEDDED=1
DEFINES+=-DHSE_VALUE=26000000UL
BOARD=SMARTWATCH
STLIB=STM32F2XX
PRECOMPILED_OBJS+=$(ROOT)/targetlibs/stm32f2/lib/startup_stm32f2xx.o
OPTIMIZEFLAGS+=-O3

else ifdef STM32F3DISCOVERY
EMBEDDED=1
USE_NET=1
USE_GRAPHICS=1
BOARD=STM32F3DISCOVERY
STLIB=STM32F3XX
PRECOMPILED_OBJS+=$(ROOT)/targetlibs/stm32f3/lib/startup_stm32f30x.o
OPTIMIZEFLAGS+=-O3

else ifdef STM32VLDISCOVERY
EMBEDDED=1
SAVE_ON_FLASH=1
BOARD=STM32VLDISCOVERY
STLIB=STM32F10X_MD_VL
PRECOMPILED_OBJS+=$(ROOT)/targetlibs/stm32f1/lib/startup_stm32f10x_md_vl.o
OPTIMIZEFLAGS+=-Os # short on program memory

else ifdef MICROBIT
EMBEDDED=1
SAVE_ON_FLASH=1
BOARD=MICROBIT
OPTIMIZEFLAGS+=-Os
USE_BLUETOOTH=1

else ifdef NRF51TAG
EMBEDDED=1
SAVE_ON_FLASH=1
BOARD=NRF51TAG
OPTIMIZEFLAGS+=-Os
USE_BLUETOOTH=1

else ifdef NRF51822DK
EMBEDDED=1
SAVE_ON_FLASH=1
BOARD=NRF51822DK
OPTIMIZEFLAGS+=-Os
USE_BLUETOOTH=1

else ifdef NRF52832DK
EMBEDDED=1
BOARD=NRF52832DK
OPTIMIZEFLAGS+=-O3
USE_BLUETOOTH=1

else ifdef TINYCHIP
EMBEDDED=1
BOARD=TINYCHIP
STLIB=STM32F10X_MD
PRECOMPILED_OBJS+=$(ROOT)/targetlibs/stm32f1/lib/startup_stm32f10x_md.o
OPTIMIZEFLAGS+=-Os # short on program memory

else ifdef LPC1768
EMBEDDED=1
MBED=1
BOARD=LPC1768
MBED_GCC_CS_DIR=$(ROOT)/targetlibs/libmbed/LPC1768/GCC_CS
PRECOMPILED_OBJS+=$(MBED_GCC_CS_DIR)/sys.o $(MBED_GCC_CS_DIR)/cmsis_nvic.o $(MBED_GCC_CS_DIR)/system_LPC17xx.o $(MBED_GCC_CS_DIR)/core_cm3.o $(MBED_GCC_CS_DIR)/startup_LPC17xx.o
LIBS+=-L$(MBED_GCC_CS_DIR)  -lmbed
OPTIMIZEFLAGS+=-O3

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

else ifdef CARAMBOLA
EMBEDDED=1
BOARD=CARAMBOLA
DEFINES += -DCARAMBOLA -DSYSFS_GPIO_DIR="\"/sys/class/gpio\""
LINUX=1
USE_FILESYSTEM=1
USE_GRAPHICS=1
USE_NET=1

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

else ifdef LCTECH_STM32F103RBT6
EMBEDDED=1
SAVE_ON_FLASH=1
BOARD=LCTECH_STM32F103RBT6
STLIB=STM32F10X_MD
PRECOMPILED_OBJS+=$(ROOT)/targetlibs/stm32f1/lib/startup_stm32f10x_md.o
OPTIMIZEFLAGS+=-Os

else ifdef ESP8266_512KB
# esp8266 with 512KB flash chip, we're using the "none boot" layout, which doesn't support
# OTA update and allows for a firmware that almost fills the 512KB. The first 64KB are loaded
# into ram (dram+iram) and the last 16KB are reserved for the SDK. That leaves 432KB (0x6C000).
EMBEDDED=1
USE_NET=1
BOARD=ESP8266_BOARD
# We have to disable inlining to keep code size in check
OPTIMIZEFLAGS+=-Os -fno-inline-functions -std=gnu11 -fgnu89-inline -Wl,--allow-multiple-definition
ESP_FLASH_SIZE      ?= 0       # 0->512KB
ESP_FLASH_MODE      ?= 0       # 0->QIO
ESP_FLASH_FREQ_DIV  ?= 0       # 0->40Mhz
ESP_FLASH_MAX       ?= 442368  # max irom0 size for 512KB flash: 432KB
ET_FS               ?= 4m      # 4Mbit (512KB) flash size in esptool flash command
ET_FF               ?= 40m     # 40Mhz flash speed in esptool flash command
ET_BLANK            ?= 0x7E000 # where to flash blank.bin to erase wireless settings

else ifdef ESP8266_4MB
# esp8266 with 4MB flash chip with OTA support: we get 492KB in the first 512, 492 KB in the
# second 512KB for firmware; and then we have 3MB-16KB for SPIFFS
EMBEDDED=1
USE_NET=1
BOARD=ESP8266_12
# Enable link-time optimisations (inlining across files) but don't go beyond -O2 'cause of
# code size explosion, also -DLINK_TIME_OPTIMISATION leads to too big a firmware
OPTIMIZEFLAGS+=-O2 -std=gnu11 -fgnu89-inline -flto -fno-fat-lto-objects -Wl,--allow-multiple-definition
ESP_FLASH_SIZE      ?= 4        # 4->4MB (512KB+512KB)
ESP_FLASH_MODE      ?= 0        # 0->QIO, 2->DIO
ESP_FLASH_FREQ_DIV  ?= 15       # 15->80Mhz
ESP_FLASH_MAX       ?= 503808   # max bin file for 512KB flash partition: 492KB
ET_FS               ?= 32m      # 32Mbit (4MB) flash size in esptool flash command
ET_FF               ?= 80m      # 80Mhz flash speed in esptool flash command
ET_BLANK            ?= 0x3FE000 # where to flash blank.bin to erase wireless settings

else ifdef ESP8266_2MB
# *** WARNING: THIS FLASH SIZE HAS NOT BEEN TESTED ***
# esp8266 with 2MB flash chip with OTA support: same as 4MB except we have 1MB-16KB for SPIFFS
EMBEDDED=1
USE_NET=1
BOARD=ESP8266_12
# Enable link-time optimisations (inlining across files)
OPTIMIZEFLAGS+=-O3 -std=gnu11 -fgnu89-inline -flto -fno-fat-lto-objects -Wl,--allow-multiple-definition
DEFINES += -DLINK_TIME_OPTIMISATION
ESP_FLASH_SIZE      ?= 3        # 3->2MB (512KB+512KB)
ESP_FLASH_MODE      ?= 0        # 0->QIO, 2->DIO
ESP_FLASH_FREQ_DIV  ?= 15       # 15->80Mhz
ESP_FLASH_MAX       ?= 503808   # max bin file for 512KB flash partition: 492KB
ET_FS               ?= 16m      # 16Mbit (2MB) flash size in esptool flash command
ET_FF               ?= 80m      # 80Mhz flash speed in esptool flash command
ET_BLANK            ?= 0x1FE000 # where to flash blank.bin to erase wireless settings

else ifdef ESP8266_1MB
# *** WARNING: THIS FLASH SIZE HAS NOT BEEN TESTED ***
# esp8266 with 1MB flash chip with OTA support: this is a mix between the 512KB and 2MB versions:
# we get two partitions for firmware but we need to turn optimization off to leave some space
# for SPIFFS
EMBEDDED=1
USE_NET=1
BOARD=ESP8266_12
# We have to disable inlining to keep code size in check
OPTIMIZEFLAGS+=-O2 -fno-inline-functions -std=gnu11 -fgnu89-inline -Wl,--allow-multiple-definition
ESP_FLASH_SIZE      ?= 2       # 2->1MB (512KB+512KB)
ESP_FLASH_MODE      ?= 0       # 0->QIO, 2->DIO
ESP_FLASH_FREQ_DIV  ?= 15      # 15->80Mhz
ESP_FLASH_MAX       ?= 503808  # max bin file for 512KB flash partition: 492KB
ET_FS               ?=  8m     # 8Mbit (1MB) flash size in esptool flash command
ET_FF               ?= 80m     # 80Mhz flash speed in esptool flash command
ET_BLANK            ?= 0xFE000 # where to flash blank.bin to erase wireless settings

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

else ifdef ARIETTA
EMBEDDED=1
BOARD=ARIETTA_G25
DEFINES += -DARIETTA_G25 -DSYSFS_GPIO_DIR="\"/sys/class/gpio\""
LINUX=1
USE_FILESYSTEM=1
USE_GRAPHICS=1
USE_NET=1

else
BOARD=LINUX
LINUX=1
USE_FILESYSTEM=1
USE_HASHLIB=1
USE_GRAPHICS=1
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

# ----------------------------- end of board defines ------------------------------
# ---------------------------------------------------------------------------------


# ---------------------------------------------------------------------------------
#                                                      Get info out of BOARDNAME.py
# ---------------------------------------------------------------------------------

PROJ_NAME=$(shell python scripts/get_board_info.py $(BOARD) "common.get_board_binary_name(board)"  | sed -e "s/.bin$$//")
ifeq ($(PROJ_NAME),)
$(error Unable to work out binary name (PROJ_NAME))
endif
ifeq ($(BOARD),LINUX)
PROJ_NAME=espruino
endif

ifeq ($(shell python scripts/get_board_info.py $(BOARD) "'bootloader' in board.info and board.info['bootloader']==1"),True)
USE_BOOTLOADER:=1
BOOTLOADER_PROJ_NAME:=bootloader_$(PROJ_NAME)
endif

ifeq ($(shell python scripts/get_board_info.py $(BOARD) "'USB' in board.devices"),True)
USB:=1
endif

ifndef LINUX
FAMILY:=$(shell python scripts/get_board_info.py $(BOARD) "board.chip['family']")
CHIP:=$(shell python scripts/get_board_info.py $(BOARD) "board.chip['part']")
endif

# ---------------------------------------------------------------------------------



# If we're not on Linux and we want internet, we need either CC3000 or WIZnet support
ifdef USE_NET
ifndef LINUX
ifdef WIZNET
USE_WIZNET=1
else ifeq ($(FAMILY),ESP8266)
USE_ESP8266=1
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
OPTIMIZEFLAGS=-g -Os
else
OPTIMIZEFLAGS=-g
endif
DEFINES+=-DDEBUG
endif

ifdef PROFILE
OPTIMIZEFLAGS+=-pg
endif

# Files that contains objects/functions/methods that will be
# exported to JS. The order here actually determines the order
# objects will be matched in. So for example Pins must come
# above ints, since a Pin is also matched as an int.
WRAPPERFILE=gen/jswrapper.c
WRAPPERSOURCES = \
src/jswrap_array.c \
src/jswrap_arraybuffer.c \
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
src/jswrap_serial.c \
src/jswrap_spi_i2c.c \
src/jswrap_stream.c \
src/jswrap_string.c \
src/jswrap_waveform.c

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
src/jsspi.c \
$(WRAPPERFILE)
CPPSOURCES =

ifdef CFILE
WRAPPERSOURCES += $(CFILE)
endif
ifdef CPPFILE
CPPSOURCES += $(CPPFILE)
endif

ifdef BOOTLOADER
ifndef USE_BOOTLOADER
$(error Using bootloader on device that is not expecting one)
endif
DEFINES+=-DSAVE_ON_FLASH # hack, as without link time optimisation the always_inlines will fail (even though they are not used)
BUILD_LINKER_FLAGS+=--bootloader
PROJ_NAME=$(BOOTLOADER_PROJ_NAME)
WRAPPERSOURCES =
SOURCES = \
targets/stm32_boot/main.c \
targets/stm32_boot/utils.c
 ifndef DEBUG
  OPTIMIZEFLAGS=-Os
 endif
else # !BOOTLOADER but using a bootloader
 ifdef USE_BOOTLOADER
  BUILD_LINKER_FLAGS+=--using_bootloader
  # -k applies bootloader hack for Espruino 1v3 boards
  ifdef MACOSX
    STM32LOADER_FLAGS+=-k -p /dev/tty.usbmodem*
  else
    STM32LOADER_FLAGS+=-k -p /dev/ttyACM0
  endif
  BASEADDRESS=$(shell python scripts/get_board_info.py $(BOARD) "hex(0x08000000+common.get_espruino_binary_address(board))")
 endif
endif

ifdef USB_PRODUCT_ID
DEFINES+=-DUSB_PRODUCT_ID=$(USB_PRODUCT_ID)
endif

ifdef SAVE_ON_FLASH
DEFINES+=-DSAVE_ON_FLASH
else
# If we have enough flash, include the debugger
DEFINES+=-DUSE_DEBUGGER
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
ifndef LINUX
SOURCES += \
libs/math/acosh.c \
libs/math/asin.c \
libs/math/asinh.c \
libs/math/atan.c \
libs/math/atanh.c \
libs/math/cbrt.c \
libs/math/chbevl.c \
libs/math/clog.c \
libs/math/cmplx.c \
libs/math/const.c \
libs/math/cosh.c \
libs/math/drand.c \
libs/math/exp10.c \
libs/math/exp2.c \
libs/math/exp.c \
libs/math/fabs.c \
libs/math/floor.c \
libs/math/isnan.c \
libs/math/log10.c \
libs/math/log2.c \
libs/math/log.c \
libs/math/mtherr.c \
libs/math/polevl.c \
libs/math/pow.c \
libs/math/powi.c \
libs/math/round.c \
libs/math/setprec.c \
libs/math/sin.c \
libs/math/sincos.c \
libs/math/sindg.c \
libs/math/sinh.c \
libs/math/sqrt.c \
libs/math/tan.c \
libs/math/tandg.c \
libs/math/tanh.c \
libs/math/unity.c
#libs/math/mod2pi.c
#libs/math/mtst.c
#libs/math/dtestvec.c
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
libs/network/socketserver.c

# 
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

 ifdef USE_ESP8266
 DEFINES += -DUSE_ESP8266
 WRAPPERSOURCES += libs/network/esp8266/jswrap_esp8266.c
 INCLUDE += -I$(ROOT)/libs/network/esp8266
 SOURCES += \
 libs/network/esp8266/network_esp8266.c\
 libs/network/esp8266/ota.c
 endif
endif

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

endif # BOOTLOADER ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ DON'T USE STUFF ABOVE IN BOOTLOADER

ifdef USB
DEFINES += -DUSB
endif

ifeq ($(FAMILY), STM32F1)
ARCHFLAGS += -mlittle-endian -mthumb -mcpu=cortex-m3  -mfix-cortex-m3-ldrd -mfloat-abi=soft
ARM=1
STM32=1
INCLUDE += -I$(ROOT)/targetlibs/stm32f1 -I$(ROOT)/targetlibs/stm32f1/lib
DEFINES += -DSTM32F1
SOURCES +=                              \
targetlibs/stm32f1/lib/misc.c              \
targetlibs/stm32f1/lib/stm32f10x_adc.c     \
targetlibs/stm32f1/lib/stm32f10x_bkp.c     \
targetlibs/stm32f1/lib/stm32f10x_can.c     \
targetlibs/stm32f1/lib/stm32f10x_dac.c     \
targetlibs/stm32f1/lib/stm32f10x_dma.c     \
targetlibs/stm32f1/lib/stm32f10x_exti.c    \
targetlibs/stm32f1/lib/stm32f10x_flash.c   \
targetlibs/stm32f1/lib/stm32f10x_gpio.c    \
targetlibs/stm32f1/lib/stm32f10x_i2c.c     \
targetlibs/stm32f1/lib/stm32f10x_iwdg.c    \
targetlibs/stm32f1/lib/stm32f10x_pwr.c     \
targetlibs/stm32f1/lib/stm32f10x_rcc.c     \
targetlibs/stm32f1/lib/stm32f10x_rtc.c     \
targetlibs/stm32f1/lib/stm32f10x_sdio.c    \
targetlibs/stm32f1/lib/stm32f10x_spi.c     \
targetlibs/stm32f1/lib/stm32f10x_tim.c     \
targetlibs/stm32f1/lib/stm32f10x_usart.c   \
targetlibs/stm32f1/lib/stm32f10x_wwdg.c    \
targetlibs/stm32f1/lib/system_stm32f10x.c
    
#targetlibs/stm32f1/lib/stm32f10x_cec.c     
#targetlibs/stm32f1/lib/stm32f10x_crc.c     
#targetlibs/stm32f1/lib/stm32f10x_dbgmcu.c  
#targetlibs/stm32f1/lib/stm32f10x_fsmc.c    

ifdef USB
STM32_LEGACY_USB=1
endif
endif #STM32F1

ifeq ($(FAMILY), STM32F3)
ARCHFLAGS += -mlittle-endian -mthumb -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=softfp
ARM=1
STM32=1
INCLUDE += -I$(ROOT)/targetlibs/stm32f3 -I$(ROOT)/targetlibs/stm32f3/lib
DEFINES += -DSTM32F3
SOURCES +=                                 \
targetlibs/stm32f3/lib/stm32f30x_adc.c        \
targetlibs/stm32f3/lib/stm32f30x_can.c        \
targetlibs/stm32f3/lib/stm32f30x_comp.c       \
targetlibs/stm32f3/lib/stm32f30x_crc.c        \
targetlibs/stm32f3/lib/stm32f30x_dac.c        \
targetlibs/stm32f3/lib/stm32f30x_dbgmcu.c     \
targetlibs/stm32f3/lib/stm32f30x_dma.c        \
targetlibs/stm32f3/lib/stm32f30x_exti.c       \
targetlibs/stm32f3/lib/stm32f30x_flash.c      \
targetlibs/stm32f3/lib/stm32f30x_gpio.c       \
targetlibs/stm32f3/lib/stm32f30x_i2c.c        \
targetlibs/stm32f3/lib/stm32f30x_iwdg.c       \
targetlibs/stm32f3/lib/stm32f30x_misc.c       \
targetlibs/stm32f3/lib/stm32f30x_opamp.c      \
targetlibs/stm32f3/lib/stm32f30x_pwr.c        \
targetlibs/stm32f3/lib/stm32f30x_rcc.c        \
targetlibs/stm32f3/lib/stm32f30x_rtc.c        \
targetlibs/stm32f3/lib/stm32f30x_spi.c        \
targetlibs/stm32f3/lib/stm32f30x_syscfg.c     \
targetlibs/stm32f3/lib/stm32f30x_tim.c        \
targetlibs/stm32f3/lib/stm32f30x_usart.c      \
targetlibs/stm32f3/lib/stm32f30x_wwdg.c       \
targetlibs/stm32f3/lib/system_stm32f30x.c

ifdef USB
STM32_LEGACY_USB=1
endif
endif #STM32F3

ifeq ($(FAMILY), STM32F4)
ARCHFLAGS += -mlittle-endian -mthumb -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=softfp
ARM=1
DEFINES += -DSTM32F4
ifdef WICED_XXX
  DEFINES += -DWICED
  # DEFINES included here in bulk from a WICED compilation
  DEFINES += -DWICED_VERSION=\"3.3.1\" -DBUS=\"SDIO\" -DPLATFORM=\"EMW3165\"
  DEFINES += -DUSE_STDPERIPH_DRIVER -DOPENSSL -DSTDC_HEADERS
  DEFINES += -DMAX_WATCHDOG_TIMEOUT_SECONDS=22 -DFIRMWARE_WITH_PMK_CALC_SUPPORT
  DEFINES += -DADD_LWIP_EAPOL_SUPPORT -DNXD_EXTENDED_BSD_SOCKET_SUPPORT -DADD_NETX_EAPOL_SUPPORT
  DEFINES += -DWWD_STARTUP_DELAY=10
  DEFINES += -DNETWORK_LwIP=1 -DLwIP_VERSION=\"v1.4.0.rc1\"
  DEFINES += -DRTOS_FreeRTOS=1 -DconfigUSE_MUTEXES -DconfigUSE_RECURSIVE_MUTEXES
  DEFINES += -DFreeRTOS_VERSION=\"v7.5.2\" -DWWD_DIRECT_RESOURCES -DHSE_VALUE=26000000
  INCLUDE += 
endif
STM32=1
INCLUDE += -I$(ROOT)/targetlibs/stm32f4 -I$(ROOT)/targetlibs/stm32f4/lib
SOURCES +=                                 \
targetlibs/stm32f4/lib/misc.c                 \
targetlibs/stm32f4/lib/stm32f4xx_adc.c        \
targetlibs/stm32f4/lib/stm32f4xx_crc.c        \
targetlibs/stm32f4/lib/stm32f4xx_dac.c        \
targetlibs/stm32f4/lib/stm32f4xx_dbgmcu.c     \
targetlibs/stm32f4/lib/stm32f4xx_dma.c        \
targetlibs/stm32f4/lib/stm32f4xx_exti.c       \
targetlibs/stm32f4/lib/stm32f4xx_flash.c      \
targetlibs/stm32f4/lib/stm32f4xx_gpio.c       \
targetlibs/stm32f4/lib/stm32f4xx_i2c.c        \
targetlibs/stm32f4/lib/stm32f4xx_iwdg.c       \
targetlibs/stm32f4/lib/stm32f4xx_pwr.c        \
targetlibs/stm32f4/lib/stm32f4xx_rcc.c        \
targetlibs/stm32f4/lib/stm32f4xx_rtc.c        \
targetlibs/stm32f4/lib/stm32f4xx_sdio.c       \
targetlibs/stm32f4/lib/stm32f4xx_spi.c        \
targetlibs/stm32f4/lib/stm32f4xx_syscfg.c     \
targetlibs/stm32f4/lib/stm32f4xx_tim.c        \
targetlibs/stm32f4/lib/stm32f4xx_usart.c      \
targetlibs/stm32f4/lib/stm32f4xx_wwdg.c       \
targetlibs/stm32f4/lib/system_stm32f4xx.c
#targetlibs/stm32f4/lib/stm32f4xx_cryp_aes.c  
#targetlibs/stm32f4/lib/stm32f4xx_dcmi.c       
#targetlibs/stm32f4/lib/stm32f4xx_dma2d.c      
#targetlibs/stm32f4/lib/stm32f4xx_can.c        
#targetlibs/stm32f4/lib/stm32f4xx_cryp_des.c  
#targetlibs/stm32f4/lib/stm32f4xx_cryp_tdes.c  
#targetlibs/stm32f4/lib/stm32f4xx_cryp.c       
#targetlibs/stm32f4/lib/stm32f4xx_hash.c       
#targetlibs/stm32f4/lib/stm32f4xx_hash_md5.c   
#targetlibs/stm32f4/lib/stm32f4xx_hash_sha1.c  
#targetlibs/stm32f4/lib/stm32f4xx_ltdc.c       
#targetlibs/stm32f4/lib/stm32f4xx_rng.c        
#targetlibs/stm32f4/lib/stm32f4xx_sai.c        
#targetlibs/stm32f4/lib/stm324xx_fsmc.c
ifdef USB
STM32_USB=1
endif
endif #STM32F4


# New STM32 Cube based USB
# This could be global for all STM32 once we figure out why it's so flaky on F1
ifdef STM32_USB
SOURCES +=                                 \
targetlibs/stm32usb/Src/stm32f4xx_ll_usb.c \
targetlibs/stm32usb/Src/stm32f4xx_hal_pcd.c \
targetlibs/stm32usb/Src/stm32f4xx_hal_pcd_ex.c 

INCLUDE += -I$(ROOT)/targetlibs/stm32usb -I$(ROOT)/targetlibs/stm32usb/Inc
SOURCES +=                                 \
targetlibs/stm32usb/usbd_conf.c \
targetlibs/stm32usb/usb_device.c \
targetlibs/stm32usb/usbd_cdc_hid.c \
targetlibs/stm32usb/Src/usbd_ctlreq.c \
targetlibs/stm32usb/Src/usbd_core.c \
targetlibs/stm32usb/Src/usbd_ioreq.c \
targetlibs/stm32usb/usbd_desc.c \
targetlibs/stm32usb/usb_irq.c
endif #USB

# Old Legacy STM32 USB
# Used for F1 and F3
ifdef STM32_LEGACY_USB
DEFINES += -DLEGACY_USB
INCLUDE += -I$(ROOT)/targetlibs/stm32legacyusb/lib -I$(ROOT)/targetlibs/stm32legacyusb
SOURCES +=                              \
targetlibs/stm32legacyusb/lib/otgd_fs_cal.c       \
targetlibs/stm32legacyusb/lib/otgd_fs_dev.c       \
targetlibs/stm32legacyusb/lib/otgd_fs_int.c       \
targetlibs/stm32legacyusb/lib/otgd_fs_pcd.c       \
targetlibs/stm32legacyusb/lib/usb_core.c          \
targetlibs/stm32legacyusb/lib/usb_init.c          \
targetlibs/stm32legacyusb/lib/usb_int.c           \
targetlibs/stm32legacyusb/lib/usb_mem.c           \
targetlibs/stm32legacyusb/lib/usb_regs.c          \
targetlibs/stm32legacyusb/lib/usb_sil.c           \
targetlibs/stm32legacyusb/usb_desc.c             \
targetlibs/stm32legacyusb/usb_endp.c             \
targetlibs/stm32legacyusb/usb_istr.c             \
targetlibs/stm32legacyusb/usb_prop.c             \
targetlibs/stm32legacyusb/usb_pwr.c              \
targetlibs/stm32legacyusb/usb_utils.c            \
targetlibs/stm32legacyusb/legacy_usb.c
endif #USB

ifeq ($(FAMILY), NRF51)
  
  NRF5X=1
  NRF5X_SDK_PATH=$(ROOT)/targetlibs/nrf5x/nrf51_sdk
  
  # ARCHFLAGS are shared by both CFLAGS and LDFLAGS.
  ARCHFLAGS = -mcpu=cortex-m0 -mthumb -mabi=aapcs -mfloat-abi=soft # Use nRF51 makefiles provided in SDK as reference.
 
  # nRF51 specific. Main differences in SDK structure are softdevice, uart, delay...
  INCLUDE += -I$(NRF5X_SDK_PATH)/components/softdevice/s110/headers
  SOURCES += $(NRF5X_SDK_PATH)/components/toolchain/system_nrf51.c \
  $(NRF5X_SDK_PATH)/components/drivers_nrf/uart/app_uart_fifo.c

  PRECOMPILED_OBJS+=$(NRF5X_SDK_PATH)/components/toolchain/gcc/gcc_startup_nrf51.o

  DEFINES += -DNRF51 -DSWI_DISABLE0 -DSOFTDEVICE_PRESENT -DS110 -DBLE_STACK_SUPPORT_REQD # SoftDevice included by default.

  LINKER_RAM:=$(shell python scripts/get_board_info.py $(BOARD) "board.chip['ram']")
  LINKER_FILE = $(NRF5X_SDK_PATH)/components/toolchain/gcc/linker_nrf51_ble_espruino_$(LINKER_RAM).ld
  
  SOFTDEVICE = targetlibs/nrf5x/softdevice/s110_nrf51_8.0.0_softdevice.hex

endif # FAMILY == NRF51

ifeq ($(FAMILY), NRF52)
  
  NRF5X=1
  NRF5X_SDK_PATH=$(ROOT)/targetlibs/nrf5x/nrf52_sdk

  # ARCHFLAGS are shared by both CFLAGS and LDFLAGS.
  ARCHFLAGS = -mcpu=cortex-m4 -mthumb -mabi=aapcs -mfloat-abi=hard -mfpu=fpv4-sp-d16 # Use nRF52 makefiles provided in SDK as reference.
 
  # nRF52 specific... Different structure from nRF51 SDK for both delay and uart.
  INCLUDE += -I$(NRF5X_SDK_PATH)/components/softdevice/s132/headers
  INCLUDE += -I$(NRF5X_SDK_PATH)/components/softdevice/s132/headers/nrf52
  INCLUDE += -I$(NRF5X_SDK_PATH)/components/drivers_nrf/delay # this directory doesnt exist in nRF51 SDK.
  SOURCES += $(NRF5X_SDK_PATH)/components/toolchain/system_nrf52.c \
  $(NRF5X_SDK_PATH)/components/drivers_nrf/delay/nrf_delay.c \
  $(NRF5X_SDK_PATH)/components/drivers_nrf/uart/nrf_drv_uart.c \
  $(NRF5X_SDK_PATH)/components/libraries/uart/app_uart_fifo.c

  PRECOMPILED_OBJS+=$(NRF5X_SDK_PATH)/components/toolchain/gcc/gcc_startup_nrf52.o

  DEFINES += -DSWI_DISABLE0 -DSOFTDEVICE_PRESENT -DNRF52 -DBOARD_PCA10036 -DCONFIG_GPIO_AS_PINRESET -DS132 -DBLE_STACK_SUPPORT_REQD
  LINKER_FILE = $(NRF5X_SDK_PATH)/components/toolchain/gcc/linker_nrf52_ble_espruino.ld

  SOFTDEVICE = targetlibs/nrf5x/softdevice/s132_nrf52_1.0.0-3.alpha_softdevice.hex

endif #FAMILY == NRF52

ifdef NRF5X

  # Just try and get rid of the compile warnings
  CFLAGS += -Wno-sign-conversion -Wno-conversion -Wno-unused-parameter
  DEFINES += -DBLUETOOTH
  
  ARM = 1
  ARM_HAS_OWN_CMSIS = 1 # Nordic uses its own CMSIS files in its SDK, these are up-to-date.
  INCLUDE += -I$(ROOT)/targetlibs/nrf5x -I$(NRF5X_SDK_PATH)
  
  TEMPLATE_PATH = $(ROOT)/targetlibs/nrf5x # This is where common linker for both nRF51 & nRF52 is stored.
  LDFLAGS += -L$(TEMPLATE_PATH)

  # These files are the Espruino HAL implementation.
  INCLUDE += -I$(ROOT)/targets/nrf5x
  SOURCES +=                              \
  targets/nrf5x/main.c                    \
  targets/nrf5x/jshardware.c              \
  targets/nrf5x/communication_interface.c \
  targets/nrf5x/nrf5x_utils.c

  # Careful here.. All these includes and sources assume a SoftDevice. Not efficeint/clean if softdevice (ble) is not enabled...
  INCLUDE += -I$(NRF5X_SDK_PATH)/examples/ble_peripheral/ble_app_uart/config
  INCLUDE += -I$(NRF5X_SDK_PATH)/components/drivers_nrf/config
  INCLUDE += -I$(NRF5X_SDK_PATH)/examples/bsp
  INCLUDE += -I$(NRF5X_SDK_PATH)/components/libraries/fifo
  INCLUDE += -I$(NRF5X_SDK_PATH)/components/libraries/util
  INCLUDE += -I$(NRF5X_SDK_PATH)/components/drivers_nrf/uart
  INCLUDE += -I$(NRF5X_SDK_PATH)/components/ble/common
  INCLUDE += -I$(NRF5X_SDK_PATH)/components/drivers_nrf/pstorage
  INCLUDE += -I$(NRF5X_SDK_PATH)/components/libraries/uart  # Not nRF51?
  INCLUDE += -I$(NRF5X_SDK_PATH)/components/device
  INCLUDE += -I$(NRF5X_SDK_PATH)/components/libraries/button
  INCLUDE += -I$(NRF5X_SDK_PATH)/components/libraries/timer
  INCLUDE += -I$(NRF5X_SDK_PATH)/components/drivers_nrf/gpiote
  INCLUDE += -I$(NRF5X_SDK_PATH)/components/ble/ble_services/ble_nus
  INCLUDE += -I$(NRF5X_SDK_PATH)/components/drivers_nrf/hal
  INCLUDE += -I$(NRF5X_SDK_PATH)/components/toolchain/gcc
  INCLUDE += -I$(NRF5X_SDK_PATH)/components/toolchain
  INCLUDE += -I$(NRF5X_SDK_PATH)/components/drivers_nrf/common
  INCLUDE += -I$(NRF5X_SDK_PATH)/components/ble/ble_advertising
  INCLUDE += -I$(NRF5X_SDK_PATH)/components/libraries/trace
  INCLUDE += -I$(NRF5X_SDK_PATH)/components/softdevice/common/softdevice_handler

  SOURCES += \
  $(NRF5X_SDK_PATH)/components/libraries/button/app_button.c \
  $(NRF5X_SDK_PATH)/components/libraries/util/app_error.c \
  $(NRF5X_SDK_PATH)/components/libraries/fifo/app_fifo.c \
  $(NRF5X_SDK_PATH)/components/libraries/timer/app_timer.c \
  $(NRF5X_SDK_PATH)/components/libraries/trace/app_trace.c \
  $(NRF5X_SDK_PATH)/components/libraries/util/nrf_assert.c \
  $(NRF5X_SDK_PATH)/components/libraries/uart/retarget.c \
  $(NRF5X_SDK_PATH)/components/drivers_nrf/common/nrf_drv_common.c \
  $(NRF5X_SDK_PATH)/components/drivers_nrf/gpiote/nrf_drv_gpiote.c \
  $(NRF5X_SDK_PATH)/components/drivers_nrf/pstorage/pstorage.c \
  $(NRF5X_SDK_PATH)/components/ble/common/ble_advdata.c \
  $(NRF5X_SDK_PATH)/components/ble/ble_advertising/ble_advertising.c \
  $(NRF5X_SDK_PATH)/components/ble/common/ble_conn_params.c \
  $(NRF5X_SDK_PATH)/components/ble/ble_services/ble_nus/ble_nus.c \
  $(NRF5X_SDK_PATH)/components/ble/common/ble_srv_common.c \
  $(NRF5X_SDK_PATH)/components/softdevice/common/softdevice_handler/softdevice_handler.c \
  $(NRF5X_SDK_PATH)/components/drivers_nrf/hal/nrf_nvmc.c

endif #NRF5X

ifeq ($(FAMILY),ESP8266)
# move os_printf strings into flash to save RAM space
DEFINES += -DUSE_OPTIMIZE_PRINTF
DEFINES += -D__ETS__ -DICACHE_FLASH -DXTENSA -DUSE_US_TIMER
ESP8266=1
LIBS += -lc -lgcc -lhal -lphy -lpp -lnet80211 -llwip_536 -lwpa -lmain
CFLAGS+= -fno-builtin -fno-strict-aliasing \
-Wno-maybe-uninitialized -Wno-old-style-declaration -Wno-conversion -Wno-unused-variable \
-Wno-unused-parameter -Wno-ignored-qualifiers -Wno-discarded-qualifiers -Wno-float-conversion \
-Wno-parentheses -Wno-type-limits -Wno-unused-function -Wno-unused-value \
-Wl,EL -Wl,--gc-sections -nostdlib -mlongcalls -mtext-section-literals
endif


ifdef MBED
ARCHFLAGS += -mcpu=cortex-m3 -mthumb
ARM=1
INCLUDE+=-I$(ROOT)/targetlibs/libmbed -I$(ROOT)/targetlibs/libmbed/$(CHIP) -I$(ROOT)/targetlibs/libmbed/$(CHIP)/GCC_CS
DEFINES += -DMBED
INCLUDE += -I$(ROOT)/targetlibs/mbed
SOURCES += targets/mbed/main.c
CPPSOURCES += targets/mbed/jshardware.cpp
endif

ifdef ARM

  ifndef LINKER_FILE # nRF5x targets define their own linker file.
    LINKER_FILE = gen/linker.ld
  endif
  DEFINES += -DARM
  ifndef ARM_HAS_OWN_CMSIS # nRF5x targets do not use the shared CMSIS files.
    INCLUDE += -I$(ROOT)/targetlibs/arm
  endif
  OPTIMIZEFLAGS += -fno-common -fno-exceptions -fdata-sections -ffunction-sections

  # I've no idea why this breaks the bootloader, but it does.
  # Given we've left 10k for it, there's no real reason to enable LTO anyway.
  ifndef BOOTLOADER
	# Enable link-time optimisations (inlining across files)
	OPTIMIZEFLAGS += -flto -fno-fat-lto-objects -Wl,--allow-multiple-definition
	DEFINES += -DLINK_TIME_OPTIMISATION
  endif

  # Limit code size growth via inlining to 8% Normally 30% it seems... This reduces code size while still being able to use -O3
  OPTIMIZEFLAGS += --param inline-unit-growth=6

  export CCPREFIX?=arm-none-eabi-

endif # ARM

PININFOFILE=$(ROOT)/gen/jspininfo
ifdef PININFOFILE
SOURCES += $(PININFOFILE).c
endif

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

ifdef STM32
DEFINES += -DSTM32 -DUSE_STDPERIPH_DRIVER=1 -D$(CHIP) -D$(BOARD) -D$(STLIB)
DEFINES += -DFAKE_STDLIB
# FAKE_STDLIB is for Espruino - it uses its own standard library so we don't have to link in the normal one + get bloated
INCLUDE += -I$(ROOT)/targets/stm32
ifndef BOOTLOADER
SOURCES +=                              \
targets/stm32/main.c                    \
targets/stm32/jshardware.c              \
targets/stm32/stm32_it.c
endif
endif

ifdef LINUX
DEFINES += -DLINUX
INCLUDE += -I$(ROOT)/targets/linux
SOURCES +=                              \
targets/linux/main.c                    \
targets/linux/jshardware.c
LIBS += -lm # maths lib
LIBS += -lpthread # thread lib for input processing
ifdef OPENWRT_UCLIBC
LIBS += -lc
else
LIBS += -lstdc++
endif
endif

ifdef NUCLEO
WRAPPERSOURCES += targets/nucleo/jswrap_nucleo.c
endif

SOURCES += $(WRAPPERSOURCES)
SOURCEOBJS = $(SOURCES:.c=.o) $(CPPSOURCES:.cpp=.o)
OBJS = $(SOURCEOBJS) $(PRECOMPILED_OBJS)


# -ffreestanding -nodefaultlibs -nostdlib -fno-common
# -nodefaultlibs -nostdlib -nostartfiles

# -fdata-sections -ffunction-sections are to help remove unused code
CFLAGS += $(OPTIMIZEFLAGS) -c $(ARCHFLAGS) $(DEFINES) $(INCLUDE)

# -Wl,--gc-sections helps remove unused code
# -Wl,--whole-archive checks for duplicates
ifndef NRF5X
 LDFLAGS += $(OPTIMIZEFLAGS) $(ARCHFLAGS)
else ifdef NRF5X
 LDFLAGS += $(ARCHFLAGS)
 LDFLAGS += --specs=nano.specs -lc -lnosys
endif # NRF5X

ifdef EMBEDDED
DEFINES += -DEMBEDDED
LDFLAGS += -Wl,--gc-sections
endif

ifdef LINKER_FILE
  LDFLAGS += -T$(LINKER_FILE)
endif

#
# Definitions for the build of the ESP8266
#
ifdef ESP8266
# The Root of the ESP8266_SDK distributed by Espressif
# This must be supplied as a Make environment variable.
ifndef ESP8266_SDK_ROOT
$(error, "The ESP8266_SDK_ROOT variable must be set")
endif

# The pefix for the xtensa toolchain
CCPREFIX=xtensa-lx106-elf-

# Extra flags passed to the linker
LDFLAGS += -L$(ESP8266_SDK_ROOT)/lib \
-nostdlib \
-Wl,--no-check-sections \
-u call_user_start \
-Wl,-static

# Extra source files specific to the ESP8266
SOURCES += targets/esp8266/uart.c \
	   targets/esp8266/user_main.c \
	   targets/esp8266/jshardware.c \
	   targets/esp8266/i2c_master.c \
	   targets/esp8266/esp8266_board_utils.c \
	   libs/network/esp8266/network_esp8266.c
# if using the hw_timer:   targets/esp8266/hw_timer.c \

# The tool used for building the firmware and flashing
ESPTOOL_CK ?= esptool-ck
ESPTOOL    ?= $(ESP8266_SDK_ROOT)/esptool/esptool.py

# Extra include directories specific to the ESP8266
INCLUDE += -I$(ESP8266_SDK_ROOT)/include -I$(ROOT)/targets/esp8266
endif # ESP8266

export CC=$(CCPREFIX)gcc
export LD=$(CCPREFIX)gcc
export AR=$(CCPREFIX)ar
export AS=$(CCPREFIX)as
export OBJCOPY=$(CCPREFIX)objcopy
export OBJDUMP=$(CCPREFIX)objdump
export GDB=$(CCPREFIX)gdb


.PHONY:  proj

all: 	 proj

ifeq ($(V),1)
        quiet_=
        Q=
else
        quiet_=quiet_
        Q=@
  export SILENT=1
endif


$(WRAPPERFILE): scripts/build_jswrapper.py $(WRAPPERSOURCES)
	@echo Generating JS wrappers
	$(Q)echo WRAPPERSOURCES = $(WRAPPERSOURCES)
	$(Q)echo DEFINES =  $(DEFINES)
	$(Q)python scripts/build_jswrapper.py $(WRAPPERSOURCES) $(DEFINES) -B$(BOARD)

ifdef PININFOFILE
$(PININFOFILE).c $(PININFOFILE).h: scripts/build_pininfo.py
	@echo Generating pin info
	$(Q)python scripts/build_pininfo.py $(BOARD) $(PININFOFILE).c $(PININFOFILE).h
endif

ifndef NRF5X # nRF5x devices use their own linker files that aren't automatically generated.
$(LINKER_FILE): scripts/build_linker.py
	@echo Generating linker scripts
	$(Q)python scripts/build_linker.py $(BOARD) $(LINKER_FILE) $(BUILD_LINKER_FLAGS)
endif # NRF5X

$(PLATFORM_CONFIG_FILE): boards/$(BOARD).py scripts/build_platform_config.py
	@echo Generating platform configs
	$(Q)python scripts/build_platform_config.py $(BOARD)

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
proj: 	$(PLATFORM_CONFIG_FILE) $(PROJ_NAME)

$(PROJ_NAME): $(OBJS)
	@echo $($(quiet_)link)
	@$(call link)

else ifdef ESP8266_512KB
# for the 512KB flash we generate non-OTA binaries
proj: $(PROJ_NAME).elf $(PROJ_NAME)_0x00000.bin $(PROJ_NAME)_0x10000.bin $(PROJ_NAME).lst

# linking is complicated. The Espruino source files get compiled into the .text section. The
# Espressif SDK libraries have .text and .irom0 sections. We need to put the libraries' .text into
# .iram0 (32KB on chip instruction ram) and we need to put the Esprunio .text and the libraries'
# .irom0 into .irom0 (demand-cached from flash). We do this dance by pre-linking the Espruino
# objects, then renaming .text to .irom0, and then finally linking with the SDK libraries.
# Note that a previous method of renaming .text to .irom0 in each object file doesn't work when
# we enable the link-time optimizer for inlining because it generates fresh code that all ends
# up in .iram0.
$(PROJ_NAME).elf: $(OBJS) $(LINKER_FILE)
	$(Q)$(LD) $(OPTIMIZEFLAGS) -nostdlib -Wl,--no-check-sections -Wl,-static -r -o partial.o $(OBJS)
	$(Q)$(OBJCOPY) --rename-section .text=.irom0.text --rename-section .literal=.irom0.literal partial.o
	$(Q)$(LD) $(LDFLAGS) -Ttargets/esp8266/eagle.app.v6.0x10000.ld -o $@ partial.o -Wl,--start-group $(LIBS) -Wl,--end-group
	$(Q)rm partial.o
	$(Q)$(OBJDUMP) --headers -j .irom0.text -j .text $(PROJ_NAME).elf | tail -n +4
	@echo To disassemble: $(OBJDUMP) -d -l -x $(PROJ_NAME).elf

# binary image for idata0&iram0
$(PROJ_NAME)_0x00000.bin: $(PROJ_NAME).elf
# Note that ESPTOOL_CK always returns non zero, ignore errors
	$(Q)$(ESPTOOL_CK) -eo $< -bo $@ -bs .text -bs .data -bs .rodata -bs .iram0.text -bc -ec || true

# binary image for irom0
$(PROJ_NAME)_0x10000.bin: $(PROJ_NAME).elf
# Note that ESPTOOL_CK always returns non zero, ignore errors
	$(Q)$(ESPTOOL_CK) -eo $< -es .irom0.text $@ -ec || true
	
# Generate a symbol table file for subsequent debugging where we know an exception address.
$(PROJ_NAME).lst : $(PROJ_NAME).elf
	$(Q)@echo "Building Symbol table (.lst) file"
	$(Q)$(call obj_dump)

flash: all $(PROJ_NAME)_0x00000.bin $(PROJ_NAME)_0x10000.bin
ifndef COMPORT
	$(error, "In order to flash, we need to have the COMPORT variable defined")
endif
	-$(Q)$(ESPTOOL) --port $(COMPORT) --baud 115200 write_flash --flash_freq 40m --flash_mode qio --flash_size 4m 0x00000 $(PROJ_NAME)_0x00000.bin 0x10000 $(PROJ_NAME)_0x10000.bin

else ifdef ESP8266
# for the non-512KB flash we generate a single OTA binary, but we generate two of them, one per
# OTA partition (Espressif calls these user1.bin and user2.bin)
USER1_BIN   = $(PROJ_NAME)_user1.bin
USER2_BIN   = $(PROJ_NAME)_user2.bin
USER1_ELF   = $(PROJ_NAME)_user1.elf
USER2_ELF   = $(PROJ_NAME)_user2.elf
LD_SCRIPT1  = ./targets/esp8266/eagle.app.v6.new.1024.app1.ld
LD_SCRIPT2  = ./targets/esp8266/eagle.app.v6.new.1024.app2.ld
APPGEN_TOOL = $(ESP8266_SDK_ROOT)/tools/gen_appbin.py
BOOTLOADER  = $(ESP8266_SDK_ROOT)/bin/boot_v1.4(b1).bin
BLANK       = $(ESP8266_SDK_ROOT)/bin/blank.bin
INIT_DATA   = $(ESP8266_SDK_ROOT)/bin/esp_init_data_default.bin

proj: $(USER1_BIN) $(USER2_BIN)

# linking is complicated. The Espruino source files get compiled into the .text section. The
# Espressif SDK libraries have .text and .irom0 sections. We need to put the libraries' .text into
# .iram0 (32KB on chip instruction ram) and we need to put the Esprunio .text and the libraries'
# .irom0 into .irom0 (demand-cached from flash). We do this dance by pre-linking the Espruino
# objects, then renaming .text to .irom0, and then finally linking with the SDK libraries.
# Note that a previous method of renaming .text to .irom0 in each object file doesn't work when
# we enable the link-time optimizer for inlining because it generates fresh code that all ends
# up in .iram0.

# generate partially linked .o with all Esprunio source files linked
$(PROJ_NAME)_partial.o: $(OBJS) $(LINKER_FILE)
	@echo LD $@
	$(Q)$(LD) $(OPTIMIZEFLAGS) -nostdlib -Wl,--no-check-sections -Wl,-static -r -o $@ $(OBJS)
	$(Q)$(OBJCOPY) --rename-section .text=.irom0.text --rename-section .literal=.irom0.literal $@

# generate fully linked 'user1' .elf using linker script for first OTA partition
$(USER1_ELF): $(PROJ_NAME)_partial.o $(LINKER_FILE)
	@echo LD $@
	$(Q)$(LD) $(LDFLAGS) -T$(LD_SCRIPT1) -o $@ $(PROJ_NAME)_partial.o -Wl,--start-group $(LIBS) -Wl,--end-group
	$(Q)$(OBJDUMP) --headers -j .irom0.text -j .text $@ | tail -n +4
	@echo To disassemble: $(OBJDUMP) -d -l -x $@

# generate fully linked 'user2' .elf using linker script for second OTA partition
$(USER2_ELF): $(PROJ_NAME)_partial.o $(LINKER_FILE)
	@echo LD $@
	$(Q)$(LD) $(LDFLAGS) -T$(LD_SCRIPT2) -o $@ $(PROJ_NAME)_partial.o -Wl,--start-group $(LIBS) -Wl,--end-group
	@echo To disassemble: $(OBJDUMP) -d -l -x $@

# generate binary image for user1, i.e. first OTA partition
$(USER1_BIN): $(USER1_ELF)
	$(Q)$(OBJCOPY) --only-section .text -O binary $(USER1_ELF) eagle.app.v6.text.bin
	$(Q)$(OBJCOPY) --only-section .data -O binary $(USER1_ELF) eagle.app.v6.data.bin
	$(Q)$(OBJCOPY) --only-section .rodata -O binary $(USER1_ELF) eagle.app.v6.rodata.bin
	$(Q)$(OBJCOPY) --only-section .irom0.text -O binary $(USER1_ELF) eagle.app.v6.irom0text.bin
	@ls -ls eagle*bin
	$(Q)COMPILE=gcc python $(APPGEN_TOOL) $(USER1_ELF) 2 $(ESP_FLASH_MODE) $(ESP_FLASH_FREQ_DIV) $(ESP_FLASH_SIZE) >/dev/null
	$(Q) rm -f eagle.app.v6.*.bin
	$(Q) mv eagle.app.flash.bin $@
	@echo "** user1.bin uses $$(stat -c '%s' $@) bytes of" $(ESP_FLASH_MAX) "available"
	@if [ $$(stat -c '%s' $@) -gt $$(( $(ESP_FLASH_MAX) )) ]; then echo "$@ too big!"; false; fi

# generate binary image for user2, i.e. second OTA partition
# we make this rule dependent on user1.bin in order to serialize the two rules because they use
# stupid static filenames (go blame the Espressif tool)
$(USER2_BIN): $(USER2_ELF) $(USER1_BIN)
	$(Q)$(OBJCOPY) --only-section .text -O binary $(USER2_ELF) eagle.app.v6.text.bin
	$(Q)$(OBJCOPY) --only-section .data -O binary $(USER2_ELF) eagle.app.v6.data.bin
	$(Q)$(OBJCOPY) --only-section .rodata -O binary $(USER2_ELF) eagle.app.v6.rodata.bin
	$(Q)$(OBJCOPY) --only-section .irom0.text -O binary $(USER2_ELF) eagle.app.v6.irom0text.bin
	$(Q)COMPILE=gcc python $(APPGEN_TOOL) $(USER2_ELF) 2 $(ESP_FLASH_MODE) $(ESP_FLASH_FREQ_DIV) $(ESP_FLASH_SIZE) >/dev/null
	$(Q) rm -f eagle.app.v6.*.bin
	$(Q) mv eagle.app.flash.bin $@

flash: all $(USER1_BIN) $(USER2_BIN)
ifndef COMPORT
	$(error, "In order to flash, we need to have the COMPORT variable defined")
endif
	-$(Q)$(ESPTOOL) --port $(COMPORT) --baud 460800 write_flash --flash_freq $(ET_FF) --flash_mode qio --flash_size $(ET_FS) 0x0000 "$(BOOTLOADER)" 0x1000 $(USER1_BIN) $(ET_BLANK) $(BLANK)

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
ifdef SOFTDEVICE
	echo Merging SoftDevice
	@echo "    (use 'pip install IntelHex' if you don't have hexmerge.py)"
	hexmerge.py $(SOFTDEVICE) $(PROJ_NAME).hex -o tmp.hex
	mv tmp.hex $(PROJ_NAME).hex
endif

$(PROJ_NAME).srec : $(PROJ_NAME).elf
	@echo $(call $(quiet_)obj_to_bin,srec,srec)
	@$(call obj_to_bin,srec,srec)

$(PROJ_NAME).bin : $(PROJ_NAME).elf
	@echo $(call $(quiet_)obj_to_bin,binary,bin)
	@$(call obj_to_bin,binary,bin)
ifndef TRAVIS
	bash scripts/check_size.sh $(PROJ_NAME).bin
endif

proj: $(PROJ_NAME).lst $(PROJ_NAME).bin $(PROJ_NAME).hex

#proj: $(PROJ_NAME).lst $(PROJ_NAME).hex $(PROJ_NAME).srec $(PROJ_NAME).bin

flash: all
ifdef USE_DFU
	sudo dfu-util -a 0 -s 0x08000000 -D $(PROJ_NAME).bin
else ifdef OLIMEXINO_STM32_BOOTLOADER
	echo Olimexino Serial bootloader
	dfu-util -a1 -d 0x1EAF:0x0003 -D $(PROJ_NAME).bin
else ifdef NUCLEO
	if [ -d "/media/$(USER)/NUCLEO" ]; then cp $(PROJ_NAME).bin /media/$(USER)/NUCLEO;sync; fi
	if [ -d "/media/NUCLEO" ]; then cp $(PROJ_NAME).bin /media/NUCLEO;sync; fi
else ifdef MICROBIT
	if [ -d "/media/MICROBIT" ]; then cp $(PROJ_NAME).bin /media/MICROBIT;sync; fi
else ifdef NRF5X
	if [ -d "/media/JLINK" ]; then cp $(PROJ_NAME).bin /media/JLINK;sync; fi
else
	echo ST-LINK flash
	st-flash --reset write $(PROJ_NAME).bin $(BASEADDRESS)
endif

serialflash: all
	echo STM32 inbuilt serial bootloader, set BOOT0=1, BOOT1=0
	python scripts/stm32loader.py -b 460800 -a $(BASEADDRESS) -ew $(STM32LOADER_FLAGS) $(PROJ_NAME).bin
#	python scripts/stm32loader.py -b 460800 -a $(BASEADDRESS) -ewv $(STM32LOADER_FLAGS) $(PROJ_NAME).bin

ifdef NRF5X # This requires nrfjprog to be working and in your system PATH.
nordic_flash:
	@echo Flashing: s132_nrf52_1.0.0-3.alpha_softdevice.hex
	nrfjprog -f NRF52 --program $(ROOT)/targetlibs/nrf5x/s132_nrf52_1.0.0-3.alpha_softdevice.hex --chiperase --verify
	@echo Flashing: $(ROOT)/$(PROJ_NAME).hex
	nrfjprog -f NRF52 --program $(ROOT)/$(PROJ_NAME).hex --verify
	nrfjprog -f NRF52 --reset
endif

gdb:
	echo "target extended-remote :4242" > gdbinit
	echo "file $(PROJ_NAME).elf" >> gdbinit
	#echo "load" >> gdbinit
	echo "break main" >> gdbinit
	echo "break HardFault_Handler" >> gdbinit
	$(GDB) -x gdbinit
	rm gdbinit
endif	    # ---------------------------------------------------

clean:
	@echo Cleaning targets
	$(Q)find . -name *.o | grep -v libmbed | grep -v arm-bcm2708 | xargs rm -f
	$(Q)rm -f $(ROOT)/gen/*.c $(ROOT)/gen/*.h $(ROOT)/gen/*.ld
	$(Q)rm -f $(PROJ_NAME).elf
	$(Q)rm -f $(PROJ_NAME).hex
	$(Q)rm -f $(PROJ_NAME).bin
	$(Q)rm -f $(PROJ_NAME).srec
	$(Q)rm -f $(PROJ_NAME).lst
