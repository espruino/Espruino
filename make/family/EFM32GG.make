EFM32=1
EFM32GG=1

ARCHFLAGS += -mcpu=cortex-m3  -mthumb

GECKO_SDK_PATH=$(ROOT)/targetlibs/Gecko_SDK

ARM_HAS_OWN_CMSIS = 1
INCLUDE += -I$(GECKO_SDK_PATH)/cmsis/Include

LINKER_FILE = $(GECKO_SDK_PATH)/Device/SiliconLabs/EFM32GG/Source/GCC/efm32gg.ld

INCLUDE += -I$(ROOT)/targets/efm32
SOURCES +=                              \
targets/efm32/main.c                    \
targets/efm32/jshardware.c

INCLUDE += -I$(GECKO_SDK_PATH)/Device/SiliconLabs/EFM32GG/Include
INCLUDE += -I$(GECKO_SDK_PATH)/emdrv/gpiointerrupt/inc
#  INCLUDE += -I$(GECKO_SDK_PATH)/emdrv/ustimer/inc
INCLUDE += -I$(GECKO_SDK_PATH)/emdrv/rtcdrv/inc
INCLUDE += -I$(GECKO_SDK_PATH)/emdrv/nvm/inc
INCLUDE += -I$(GECKO_SDK_PATH)/emdrv/common/inc
INCLUDE += -I$(GECKO_SDK_PATH)/emlib/inc

TARGETSOURCES += \
$(GECKO_SDK_PATH)/Device/SiliconLabs/EFM32GG/Source/GCC/startup_efm32gg.c \
$(GECKO_SDK_PATH)/Device/SiliconLabs/EFM32GG/Source/system_efm32gg.c \
$(GECKO_SDK_PATH)/emlib/src/em_gpio.c \
$(GECKO_SDK_PATH)/emlib/src/em_cmu.c \
$(GECKO_SDK_PATH)/emlib/src/em_assert.c \
$(GECKO_SDK_PATH)/emlib/src/em_emu.c \
$(GECKO_SDK_PATH)/emlib/src/em_msc.c \
$(GECKO_SDK_PATH)/emlib/src/em_rtc.c \
$(GECKO_SDK_PATH)/emlib/src/em_int.c \
$(GECKO_SDK_PATH)/emlib/src/em_system.c \
$(GECKO_SDK_PATH)/emlib/src/em_timer.c \
$(GECKO_SDK_PATH)/emlib/src/em_usart.c \
$(GECKO_SDK_PATH)/emdrv/gpiointerrupt/src/gpiointerrupt.c \
$(GECKO_SDK_PATH)/emdrv/rtcdrv/src/rtcdriver.c \
$(GECKO_SDK_PATH)/emdrv/nvm/src/nvm_hal.c
#	$(GECKO_SDK_PATH)/emdrv/ustimer/src/ustimer.c

# $(GECKO_SDK_PATH)/emdrv/nvm/src/nvm.c \
# $(GECKO_SDK_PATH)/emdrv/nvm/src/nvm_hal.c \
# $(GECKO_SDK_PATH)/emlib/src/em_acmp.c \
# $(GECKO_SDK_PATH)/emlib/src/em_adc.c \
# $(GECKO_SDK_PATH)/emlib/src/em_aes.c \
# $(GECKO_SDK_PATH)/emlib/src/em_burtc.c \
# $(GECKO_SDK_PATH)/emlib/src/em_crc.c \
# $(GECKO_SDK_PATH)/emlib/src/em_cryotimer.c \
# $(GECKO_SDK_PATH)/emlib/src/em_crypto.c \
# $(GECKO_SDK_PATH)/emlib/src/em_dac.c \
# $(GECKO_SDK_PATH)/emlib/src/em_dbg.c \
# $(GECKO_SDK_PATH)/emlib/src/em_dma.c \
# $(GECKO_SDK_PATH)/emlib/src/em_ebi.c \
# $(GECKO_SDK_PATH)/emlib/src/em_i2c.c \
# $(GECKO_SDK_PATH)/emlib/src/em_idac.c \
# $(GECKO_SDK_PATH)/emlib/src/em_lcd.c \
# $(GECKO_SDK_PATH)/emlib/src/em_ldma.c \
# $(GECKO_SDK_PATH)/emlib/src/em_lesense.c \
# $(GECKO_SDK_PATH)/emlib/src/em_letimer.c \
# $(GECKO_SDK_PATH)/emlib/src/em_leuart.c \
# $(GECKO_SDK_PATH)/emlib/src/em_mpu.c \
# $(GECKO_SDK_PATH)/emlib/src/em_opamp.c \
# $(GECKO_SDK_PATH)/emlib/src/em_pcnt.c \
# $(GECKO_SDK_PATH)/emlib/src/em_prs.c \
# $(GECKO_SDK_PATH)/emlib/src/em_rmu.c \
# $(GECKO_SDK_PATH)/emlib/src/em_rtcc.c \
# $(GECKO_SDK_PATH)/emlib/src/em_vcmp.c \
# $(GECKO_SDK_PATH)/emlib/src/em_wdog.c

include make/common/ARM.make
