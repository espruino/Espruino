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

proj: $(PROJ_NAME).lst $(PROJ_NAME).bin

flash: $(PROJ_NAME).bin
ifdef USE_DFU
	sudo dfu-util -a 0 -s 0x08000000 -D $(PROJ_NAME).bin
else
	@echo ST-LINK flash
	st-flash --reset write $(PROJ_NAME).bin $(BASEADDRESS)
endif

