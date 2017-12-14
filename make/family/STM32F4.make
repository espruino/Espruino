include make/common/STM32.make

ARCHFLAGS += -mlittle-endian -mthumb -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=softfp
# The archflags below use the STM32F4's FPU for 32 bit floats, and pass doubles as 64 bit
# Thing is, we don't use 'float', and only use doubles so this is basically useless to us (and increases code size)
# ARCHFLAGS += -mlittle-endian -mthumb -mcpu=cortex-m4 -mabi=aapcs -mfloat-abi=hard -mfpu=fpv4-sp-d16

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
TARGETSOURCES +=                                 \
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
include make/common/STM32_USB.make
endif
