ARCHFLAGS += -mlittle-endian -mthumb -mcpu=cortex-m7 -mfpu=fpv5-d16 -mfloat-abi=softfp
DEFINES += -DSTM32F7 -DSTM32 -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER=1
INCLUDE += -I$(ROOT)/targets/stm32 -I$(ROOT)/targetlibs/stm32f7 -I$(ROOT)/targetlibs/stm32f7/lib

STM32=1

TARGETSOURCES +=                                   \
targetlibs/stm32f7/lib/hal_stubs.c                 \
targetlibs/stm32f7/lib/stm32f7xx_hal_flash.c       \
targetlibs/stm32f7/lib/stm32f7xx_hal_flash_ex.c    \
targetlibs/stm32f7/lib/system_stm32f7xx.c          \
targetlibs/stm32f7/lib/stm32f7xx_ll_gpio.c         \
targetlibs/stm32f7/lib/stm32f7xx_ll_usart.c        \
targetlibs/stm32f7/lib/stm32f7xx_ll_spi.c          \
targetlibs/stm32f7/lib/stm32f7xx_ll_tim.c          \
targetlibs/stm32f7/lib/stm32f7xx_ll_utils.c        \
targetlibs/stm32f7/lib/stm32f7xx_ll_exti.c         \
targetlibs/stm32f7/lib/stm32f7xx_ll_i2c.c          \
targetlibs/stm32f7/lib/stm32f7xx_ll_rcc.c          \
targetlibs/stm32f7/lib/stm32f7xx_ll_pwr.c

ifdef USB
include make/common/STM32_USB.make
endif

include make/common/STM32_LL.make