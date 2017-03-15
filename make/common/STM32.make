STM32=1

DEFINES += -DSTM32 -DUSE_STDPERIPH_DRIVER=1 -D$(CHIP) -D$(BOARD) -D$(STLIB)
INCLUDE += -I$(ROOT)/targets/stm32
ifndef BOOTLOADER
 SOURCES +=                              \
 targets/stm32/main.c                    \
 targets/stm32/jshardware.c              \
 targets/stm32/stm32_it.c
 ifdef USE_BOOTLOADER
  BUILD_LINKER_FLAGS+=--using_bootloader
  # -k applies bootloader hack for Espruino 1v3 boards
  ifdef MACOSX
    STM32LOADER_FLAGS+=-k -p /dev/tty.usbmodem*
  else
    STM32LOADER_FLAGS+=-k -p /dev/ttyACM0
  endif
  BASEADDRESS=$(shell python scripts/get_board_info.py $(BOARD) "hex(0x08000000+common.get_espruino_binary_address(board))")
 endif # USE_BOOTLOADER
else # !BOOTLOADER
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
endif # BOOTLOADER

# ==============================================================
include make/common/ARM.make
