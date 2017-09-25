# New STM32 Cube based USB
# This could be global for all STM32 once we figure out why it's so flaky on F1
STM32_USB=1

ifeq ($(FAMILY),STM32F4)
# LOW LAYER USB DRIVER
TARGETSOURCES +=                                 \
targetlibs/stm32usb/Src/stm32f4xx_ll_usb.c \
targetlibs/stm32usb/Src/stm32f4xx_hal_pcd.c \
targetlibs/stm32usb/Src/stm32f4xx_hal_pcd_ex.c
endif


ifeq ($(FAMILY),STM32L4)
# LOW LAYER USB DRIVER
TARGETSOURCES +=                                 \
targetlibs/stm32l4/lib/STM32L4xx_HAL_Driver/Src/stm32l4xx_ll_usb.c \
targetlibs/stm32l4/lib/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_pcd.c \
targetlibs/stm32l4/lib/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_pcd_ex.c

INCLUDE += -I$(ROOT)/targets/stm32
endif

INCLUDE += -I$(ROOT)/targetlibs/stm32usb -I$(ROOT)/targetlibs/stm32usb/Inc
TARGETSOURCES +=                                 \
targetlibs/stm32usb/usbd_conf.c \
targetlibs/stm32usb/usb_device.c \
targetlibs/stm32usb/usbd_cdc_hid.c \
targetlibs/stm32usb/Src/usbd_ctlreq.c \
targetlibs/stm32usb/Src/usbd_core.c \
targetlibs/stm32usb/Src/usbd_ioreq.c \
targetlibs/stm32usb/usbd_desc.c \
targetlibs/stm32usb/usb_irq.c
