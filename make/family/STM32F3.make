include make/common/STM32.make

ARCHFLAGS += -mlittle-endian -mthumb -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=softfp
INCLUDE += -I$(ROOT)/targetlibs/stm32f3 -I$(ROOT)/targetlibs/stm32f3/lib
DEFINES += -DSTM32F3
TARGETSOURCES +=                                 \
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
include make/common/STM32_LEGACY_USB.make
endif
