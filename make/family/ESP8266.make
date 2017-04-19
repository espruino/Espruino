#
# Definitions for the build of the ESP8266

ESP8266=1

# Enable link-time optimisations (inlining across files), use -Os 'cause else we end up with
# too large a firmware (-Os is -O2 without optimizations that increase code size)
ifndef DISABLE_LTO
OPTIMIZEFLAGS+=-Os -g -std=gnu11 -fgnu89-inline -fno-fat-lto-objects -Wl,--allow-multiple-definition
#OPTIMIZEFLAGS+=-DLINK_TIME_OPTIMISATION # this actually slows things down!
else
# DISABLE_LTO is necessary in order to analyze static string sizes (see: topstring makefile target)
OPTIMIZEFLAGS+=-Os -std=gnu11 -fgnu89-inline -Wl,--allow-multiple-definition
endif


ifdef FLASH_4MB
ESP_FLASH_MAX       ?= 962560   # max bin file: 940KB
ESP_FLASH_SIZE      ?= 6        # 6->4MB (1024KB+1024KB)       
ESP_FLASH_MODE      ?= 0        # 0->QIO, 2->DIO
ESP_FLASH_FREQ_DIV  ?= 15       # 15->80Mhz
ET_FS               ?= 4MB-c1   # 32Mbit (4MB) flash size in esptool flash command
ET_FF               ?= 80m      # 80Mhz flash speed in esptool flash command
ET_BLANK            ?= 0x3FE000 # where to flash blank.bin
ET_DEFAULTS         ?= 0x3FC000 # where to flash esp_init_data_default.bin to default SDK settings
else ifdef 2MB
ESP_FLASH_MAX       ?= 479232   # max bin file: 468KB
ESP_FLASH_SIZE      ?= 3        # 3->2MB (512KB+512KB)
ESP_FLASH_MODE      ?= 0        # 0->QIO, 2->DIO
ESP_FLASH_FREQ_DIV  ?= 15       # 15->80Mhz
ET_FS               ?= 16m      # 16Mbit (2MB) flash size in esptool flash command
ET_FF               ?= 80m      # 80Mhz flash speed in esptool flash command
ET_BLANK            ?= 0x1FE000 # where to flash blank.bin
ET_DEFAULTS         ?= 0x1FC000 # where to flash esp_init_data_default.bin to default SDK settings
else ifdef 1MB
ESP_FLASH_MAX       ?= 479232   # max bin file: 468KB
ESP_FLASH_SIZE      ?= 2        # 2->1MB (512KB+512KB)
ESP_FLASH_MODE      ?= 0        # 0->QIO, 2->DIO
ESP_FLASH_FREQ_DIV  ?= 15       # 15->80Mhz
ET_FS               ?=  8m      # 8Mbit (1MB) flash size in esptool flash command
ET_FF               ?= 80m      # 80Mhz flash speed in esptool flash command
ET_BLANK            ?= 0xFE000  # where to flash blank.bin
ET_DEFAULTS         ?= 0xFC000  # where to flash esp_init_data_default.bin to default SDK settings
else # 512KB
ESP_FLASH_MAX       ?= 479232   # max bin file: 468KB
ESP_FLASH_SIZE      ?= 0        # 0->512KB
ESP_FLASH_MODE      ?= 0        # 0->QIO
ESP_FLASH_FREQ_DIV  ?= 0        # 0->40Mhz
ET_FS               ?= 4m       # 4Mbit (512KB) flash size in esptool flash command
ET_FF               ?= 40m      # 40Mhz flash speed in esptool flash command
ET_BLANK            ?= 0x7E000  # where to flash blank.bin
ET_DEFAULTS         ?= 0x7C000  # where to flash esp_init_data_default.bin to default SDK settings
endif

FLASH_BAUD ?= 115200 # The flash baud rate


# move os_printf strings into flash to save RAM space
DEFINES += -DUSE_OPTIMIZE_PRINTF
DEFINES += -D__ETS__ -DICACHE_FLASH -DXTENSA -DUSE_US_TIMER
ESP8266=1
LIBS += -lc -lgcc -lhal -lphy -lpp -lnet80211 -llwip_536 -lwpa -lmain -lpwm -lcrypto
CFLAGS+= -fno-builtin \
-Wno-maybe-uninitialized -Wno-old-style-declaration -Wno-conversion -Wno-unused-variable \
-Wno-unused-parameter -Wno-ignored-qualifiers -Wno-discarded-qualifiers -Wno-float-conversion \
-Wno-parentheses -Wno-type-limits -Wno-unused-function -Wno-unused-value \
-Wl,EL -Wl,--gc-sections -nostdlib -mlongcalls -mtext-section-literals

#
# The Root of the ESP8266_SDK distributed by Espressif
# This must be supplied as a Make environment variable.
ifndef ESP8266_SDK_ROOT
$(error "The ESP8266_SDK_ROOT variable must be set")
endif

# The pefix for the xtensa toolchain
CCPREFIX=xtensa-lx106-elf-
DEFINES += -DESP8266

# Extra flags passed to the linker
LDFLAGS += -L$(ESP8266_SDK_ROOT)/lib \
-nostdlib \
-Wl,--no-check-sections \
-u call_user_start \
-Wl,-static

# Extra source files specific to the ESP8266
SOURCES += targets/esp8266/uart.c \
	targets/esp8266/spi.c \
	targets/esp8266/user_main.c \
	targets/esp8266/log.c \
	targets/esp8266/jshardware.c \
	targets/esp8266/i2c_master.c \
	targets/esp8266/esp8266_board_utils.c \
	targets/esp8266/gdbstub.c \
	targets/esp8266/gdbstub-entry.S \
	libs/network/esp8266/network_esp8266.c

# The tool used for building the firmware and flashing
ESPTOOL    ?= $(ESP8266_SDK_ROOT)/esptool/esptool.py

# Extra include directories specific to the ESP8266
INCLUDE += -I$(ESP8266_SDK_ROOT)/include -I$(ROOT)/targets/esp8266
