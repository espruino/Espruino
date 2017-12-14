include make/common/STM32.make

ARCHFLAGS += -mlittle-endian -mthumb -mcpu=cortex-m3  -mfix-cortex-m3-ldrd -mfloat-abi=soft
INCLUDE += -I$(ROOT)/targetlibs/stm32f1 -I$(ROOT)/targetlibs/stm32f1/lib
DEFINES += -DSTM32F1
TARGETSOURCES +=                              \
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
include make/common/STM32_LEGACY_USB.make
endif
