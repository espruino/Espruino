ARCHFLAGS += -mlittle-endian -mthumb -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=softfp
DEFINES += -DSTM32L4
DEFINES += -DSTM32L476xx
DEFINES += -DUSE_FULL_LL_DRIVER
DEFINES += -DUSE_FULL_ASSERT
DEFINES += -DFLASH_64BITS_ALIGNEMENT=1 #L4 flash needs to be accessed with 64 bits
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
INCLUDE += -I$(ROOT)/targetlibs/stm32l4 -I$(ROOT)/targetlibs/stm32l4/lib -I$(ROOT)/targetlibs/stm32l4/lib/STM32L4xx_HAL_Driver/Inc -I$(ROOT)/targetlibs/stm32l4/lib/CMSIS/Device/ST/STM32L4xx/Include -I$(ROOT)/targetlibs/stm32l4/lib/CMSIS/Include -I$(ROOT)/targetlibs/stm32l4/lib/BSP/STM32L4xx_Nucleo -I$(ROOT)/targetlibs/stm32l4/lib/CMSIS/Device/ST/STM32L4xx/Source/Templates

TARGETSOURCES +=                                   \
targetlibs/stm32l4/lib/STM32L4xx_HAL_Driver/Src/stm32l4xx_ll_adc.c    \
targetlibs/stm32l4/lib/STM32L4xx_HAL_Driver/Src/stm32l4xx_ll_comp.c   \
targetlibs/stm32l4/lib/STM32L4xx_HAL_Driver/Src/stm32l4xx_ll_crc.c    \
targetlibs/stm32l4/lib/STM32L4xx_HAL_Driver/Src/stm32l4xx_ll_crs.c    \
targetlibs/stm32l4/lib/STM32L4xx_HAL_Driver/Src/stm32l4xx_ll_dac.c    \
targetlibs/stm32l4/lib/STM32L4xx_HAL_Driver/Src/stm32l4xx_ll_dma.c    \
targetlibs/stm32l4/lib/STM32L4xx_HAL_Driver/Src/stm32l4xx_ll_exti.c   \
targetlibs/stm32l4/lib/STM32L4xx_HAL_Driver/Src/stm32l4xx_ll_fmc.c    \
targetlibs/stm32l4/lib/STM32L4xx_HAL_Driver/Src/stm32l4xx_ll_gpio.c   \
targetlibs/stm32l4/lib/STM32L4xx_HAL_Driver/Src/stm32l4xx_ll_i2c.c    \
targetlibs/stm32l4/lib/STM32L4xx_HAL_Driver/Src/stm32l4xx_ll_lptim.c  \
targetlibs/stm32l4/lib/STM32L4xx_HAL_Driver/Src/stm32l4xx_ll_lpuart.c \
targetlibs/stm32l4/lib/STM32L4xx_HAL_Driver/Src/stm32l4xx_ll_opamp.c  \
targetlibs/stm32l4/lib/STM32L4xx_HAL_Driver/Src/stm32l4xx_ll_pwr.c    \
targetlibs/stm32l4/lib/STM32L4xx_HAL_Driver/Src/stm32l4xx_ll_rcc.c    \
targetlibs/stm32l4/lib/STM32L4xx_HAL_Driver/Src/stm32l4xx_ll_rng.c    \
targetlibs/stm32l4/lib/STM32L4xx_HAL_Driver/Src/stm32l4xx_ll_rtc.c    \
targetlibs/stm32l4/lib/STM32L4xx_HAL_Driver/Src/stm32l4xx_ll_sdmmc.c  \
targetlibs/stm32l4/lib/STM32L4xx_HAL_Driver/Src/stm32l4xx_ll_spi.c    \
targetlibs/stm32l4/lib/STM32L4xx_HAL_Driver/Src/stm32l4xx_ll_swpmi.c  \
targetlibs/stm32l4/lib/STM32L4xx_HAL_Driver/Src/stm32l4xx_ll_tim.c    \
targetlibs/stm32l4/lib/STM32L4xx_HAL_Driver/Src/stm32l4xx_ll_usart.c  \
targetlibs/stm32l4/lib/STM32L4xx_HAL_Driver/Src/stm32l4xx_ll_usb.c    \
targetlibs/stm32l4/lib/STM32L4xx_HAL_Driver/Src/stm32l4xx_ll_utils.c  \
targetlibs/stm32l4/lib/CMSIS/Device/ST/STM32L4xx/Source/Templates/system_stm32l4xx.c \
targetlibs/stm32l4/lib/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_flash.c \
targetlibs/stm32l4/lib/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_flash_ex.c \
targetlibs/stm32l4/lib/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal.c

ifdef USB
include make/common/STM32_USB.make
endif

include make/common/STM32_LL.make
