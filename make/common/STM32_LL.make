STM32_LL=1

DEFINES += -DSTM32_LL -DUSE_STDPERIPH_DRIVER=1 -D$(CHIP) -D$(BOARD) -D$(STLIB)
INCLUDE += -I$(ROOT)/targets/stm32_ll
ifndef BOOTLOADER
SOURCES +=                              \
targets/stm32_ll/main.c                    \
targets/stm32_ll/jshardware.c              \
targets/stm32_ll/stm32_it.c
endif

# ==============================================================
include make/common/ARM.make
