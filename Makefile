#OLIMEX=1 for OLIMEXINO
#STM32VLDISCOVERY
#STM32F4DISCOVERY

#OLIMEX=1
#STM32VLDISCOVERY=1
#STM32F4DISCOVERY=1

MAKEFLAGS=-j5 # multicore

INCLUDE=-I$(ROOT)/targets -I$(ROOT)/src
DEFINES=-DFAKE_STDLIB
USE_MATH=1
OPTIMIZEFLAGS=

CWD = $(shell pwd)
export ROOT=$(CWD)

###################################################
# 4.6
#export CCPREFIX=arm-linux-gnueabi-
# 4.5
#export CCPREFIX=~/sat/bin/arm-none-eabi-
# 4.4
export CCPREFIX=arm-none-eabi-


export CC=$(CCPREFIX)gcc
export LD=$(CCPREFIX)gcc
export AR=$(CCPREFIX)ar
export AS=$(CCPREFIX)as
export OBJCOPY=$(CCPREFIX)objcopy
export OBJDUMP=$(CCPREFIX)objdump

ifdef OLIMEX
PROJ_NAME=espruino_olimexino_stm32
USB=1
FAMILY=STM32F1
CHIP=STM32F103
BOARD=OLIMEXINO_STM32
STLIB=STM32F10X_MD
STARTUP=$(ROOT)/targets/stm32f1/lib/startup_stm32f10x_md
OPTIMIZEFLAGS+=-O2 # short on program memory
else ifdef STM32F4DISCOVERY
PROJ_NAME=espruino_stm32f4discovery
USB=1
DEFINES += -DUSE_USB_OTG_FS=1
FAMILY=STM32F4
CHIP=STM32F407
BOARD=STM32F4DISCOVERY
STLIB=STM32F4XX
STARTUP=$(ROOT)/targets/stm32f4/lib/startup_stm32f4xx
OPTIMIZEFLAGS+=-O3
else ifdef STM32VLDISCOVERY
PROJ_NAME=espruino_stm32vldiscovery
FAMILY=STM32F1
CHIP=STM32F100
BOARD=STM32VLDISCOVERY
STLIB=STM32F10X_MD_VL
STARTUP=$(ROOT)/targets/stm32f1/lib/startup_stm32f10x_md_vl
OPTIMIZEFLAGS+=-O2 # short on program memory
else
$(error Must give a device name (eg. STM32F4DISCOVERY=1 make)- see head of makefile)
endif


SOURCES= \
src/jslex.c \
src/jsvar.c \
src/jsfunctions.c \
src/jsutils.c \
src/jsparse.c \
src/jsinteractive.c \
src/jsdevices.c \
src/jshardware.c 

ifdef USE_MATH
DEFINES += -DUSE_MATH 
INCLUDE += -I$(ROOT)/math
SOURCES += \
math/acosh.c \
math/asin.c \
math/asinh.c \
math/atan.c \
math/atanh.c \
math/cbrt.c \
math/chbevl.c \
math/clog.c \
math/cmplx.c \
math/const.c \
math/cosh.c \
math/drand.c \
math/exp10.c \
math/exp2.c \
math/exp.c \
math/fabs.c \
math/floor.c \
math/isnan.c \
math/log10.c \
math/log2.c \
math/log.c \
math/mtherr.c \
math/polevl.c \
math/pow.c \
math/powi.c \
math/round.c \
math/setprec.c \
math/sin.c \
math/sincos.c \
math/sindg.c \
math/sinh.c \
math/sqrt.c \
math/tan.c \
math/tandg.c \
math/tanh.c \
math/unity.c
#math/mod2pi.c 
#math/mtst.c 
#math/dtestvec.c 
endif

ifdef USB
DEFINES += -DUSB
endif

ifeq ($(FAMILY), STM32F1)
export ARCHFLAGS=-mlittle-endian -mthumb -mcpu=cortex-m3  -mfix-cortex-m3-ldrd  -mthumb-interwork -mfloat-abi=soft
ARM=1
STM32=1
INCLUDE += -I$(ROOT)/targets/stm32f1 -I$(ROOT)/targets/stm32f1/lib
DEFINES += -DSTM32F1
SOURCES +=                              \
targets/stm32f1/lib/misc.c              \
targets/stm32f1/lib/stm32f10x_adc.c     \
targets/stm32f1/lib/stm32f10x_bkp.c     \
targets/stm32f1/lib/stm32f10x_can.c     \
targets/stm32f1/lib/stm32f10x_cec.c     \
targets/stm32f1/lib/stm32f10x_crc.c     \
targets/stm32f1/lib/stm32f10x_dac.c     \
targets/stm32f1/lib/stm32f10x_dbgmcu.c  \
targets/stm32f1/lib/stm32f10x_dma.c     \
targets/stm32f1/lib/stm32f10x_exti.c    \
targets/stm32f1/lib/stm32f10x_flash.c   \
targets/stm32f1/lib/stm32f10x_fsmc.c    \
targets/stm32f1/lib/stm32f10x_gpio.c    \
targets/stm32f1/lib/stm32f10x_i2c.c     \
targets/stm32f1/lib/stm32f10x_iwdg.c    \
targets/stm32f1/lib/stm32f10x_pwr.c     \
targets/stm32f1/lib/stm32f10x_rcc.c     \
targets/stm32f1/lib/stm32f10x_rtc.c     \
targets/stm32f1/lib/stm32f10x_sdio.c    \
targets/stm32f1/lib/stm32f10x_spi.c     \
targets/stm32f1/lib/stm32f10x_tim.c     \
targets/stm32f1/lib/stm32f10x_usart.c   \
targets/stm32f1/lib/stm32f10x_wwdg.c    \
targets/stm32f1/lib/system_stm32f10x.c

ifdef USB
INCLUDE += -I$(ROOT)/targets/stm32f1/usblib -I$(ROOT)/targets/stm32f1/usb
SOURCES +=                              \
targets/stm32f1/usblib/otgd_fs_cal.c       \
targets/stm32f1/usblib/otgd_fs_dev.c       \
targets/stm32f1/usblib/otgd_fs_int.c       \
targets/stm32f1/usblib/otgd_fs_pcd.c       \
targets/stm32f1/usblib/usb_core.c          \
targets/stm32f1/usblib/usb_init.c          \
targets/stm32f1/usblib/usb_int.c           \
targets/stm32f1/usblib/usb_mem.c           \
targets/stm32f1/usblib/usb_regs.c          \
targets/stm32f1/usblib/usb_sil.c           \
targets/stm32f1/usb/usb_desc.c              \
targets/stm32f1/usb/usb_endp.c              \
targets/stm32f1/usb/usb_istr.c              \
targets/stm32f1/usb/usb_prop.c              \
targets/stm32f1/usb/usb_pwr.c               \
targets/stm32f1/usb/usb_utils.c
endif

endif

ifeq ($(FAMILY), STM32F4)
export ARCHFLAGS=-mlittle-endian -mthumb -mcpu=cortex-m4 -mthumb-interwork -mfpu=fpv4-sp-d16 -mfloat-abi=softfp
ARM=1
STM32=1
INCLUDE += -I$(ROOT)/targets/stm32f4 -I$(ROOT)/targets/stm32f4/lib
DEFINES += -DSTM32F4
SOURCES +=                                 \
targets/stm32f4/lib/misc.c                 \
targets/stm32f4/lib/stm32f4xx_adc.c        \
targets/stm32f4/lib/stm32f4xx_can.c        \
targets/stm32f4/lib/stm32f4xx_crc.c        \
targets/stm32f4/lib/stm32f4xx_cryp_aes.c   \
targets/stm32f4/lib/stm32f4xx_cryp.c       \
targets/stm32f4/lib/stm32f4xx_cryp_des.c   \
targets/stm32f4/lib/stm32f4xx_cryp_tdes.c  \
targets/stm32f4/lib/stm32f4xx_dac.c        \
targets/stm32f4/lib/stm32f4xx_dbgmcu.c     \
targets/stm32f4/lib/stm32f4xx_dcmi.c       \
targets/stm32f4/lib/stm32f4xx_dma.c        \
targets/stm32f4/lib/stm32f4xx_exti.c       \
targets/stm32f4/lib/stm32f4xx_flash.c      \
targets/stm32f4/lib/stm32f4xx_fsmc.c       \
targets/stm32f4/lib/stm32f4xx_gpio.c       \
targets/stm32f4/lib/stm32f4xx_hash.c       \
targets/stm32f4/lib/stm32f4xx_hash_md5.c   \
targets/stm32f4/lib/stm32f4xx_hash_sha1.c  \
targets/stm32f4/lib/stm32f4xx_i2c.c        \
targets/stm32f4/lib/stm32f4xx_iwdg.c       \
targets/stm32f4/lib/stm32f4xx_pwr.c        \
targets/stm32f4/lib/stm32f4xx_rcc.c        \
targets/stm32f4/lib/stm32f4xx_rng.c        \
targets/stm32f4/lib/stm32f4xx_rtc.c        \
targets/stm32f4/lib/stm32f4xx_sdio.c       \
targets/stm32f4/lib/stm32f4xx_spi.c        \
targets/stm32f4/lib/stm32f4xx_syscfg.c     \
targets/stm32f4/lib/stm32f4xx_tim.c        \
targets/stm32f4/lib/stm32f4xx_usart.c      \
targets/stm32f4/lib/stm32f4xx_wwdg.c       \
targets/stm32f4/lib/system_stm32f4xx.c

ifdef USB
INCLUDE += -I$(ROOT)/targets/stm32f4/usblib -I$(ROOT)/targets/stm32f4/usb
SOURCES +=                                 \
targets/stm32f4/usblib/usb_core.c          \
targets/stm32f4/usblib/usbd_cdc_core.c     \
targets/stm32f4/usblib/usb_dcd.c           \
targets/stm32f4/usblib/usb_dcd_int.c       \
targets/stm32f4/usblib/usbd_core.c         \
targets/stm32f4/usblib/usbd_ioreq.c        \
targets/stm32f4/usblib/usbd_req.c          \
targets/stm32f4/usb/usb_bsp.c              \
targets/stm32f4/usb/usbd_cdc_vcp.c         \
targets/stm32f4/usb/usbd_desc.c            \
targets/stm32f4/usb/usbd_usr.c


#targets/stm32f4/usblib/usb_hcd.c           
#targets/stm32f4/usblib/usb_hcd_int.c
#targets/stm32f4/usblib/usb_otg.c           
       

endif

endif

ifdef ARM
DEFINES += -DARM
INCLUDE += -I$(ROOT)/targets/arm
endif

ifdef STM32
DEFINES += -DSTM32
INCLUDE += -I$(ROOT)/targets/stm32
SOURCES +=                              \
targets/stm32/main.c                    \
targets/stm32/stm32_it.c
endif

SOURCEOBJS=$(SOURCES:.c=.o)
OBJS = $(SOURCEOBJS)
OBJS += $(LIB_ROOT)/$(STARTUP).o # add startup file to build


# -ffreestanding -nodefaultlibs -nostdlib -fno-common
# -nodefaultlibs -nostdlib -nostartfiles

DEFINES += -D$(CHIP) -D$(BOARD) -D$(STLIB)

# -fdata-sections -ffunction-sections are to help remove unused code
export CFLAGS=$(OPTIMIZEFLAGS) -c -fno-common -fno-exceptions -fdata-sections -ffunction-sections $(ARCHFLAGS) -DUSE_STDPERIPH_DRIVER=1 $(DEFINES) $(INCLUDE)
#export CFLAGS=-g -O1 -c -fno-common $(ARCHFLAGS) -DUSE_STDPERIPH_DRIVER=1 -DARM -DFAKE_STDLIB
#can use O2 here


# -Wl,--gc-sections helps remove unused code
# -Wl,--whole-archive checks for duplicates
export LDFLAGS=$(ARCHFLAGS) -Wl,--gc-sections -Tlinker/$(CHIP).ld -Llib 


.PHONY:  proj

all: 	 proj

proj: 	$(PROJ_NAME).elf

$(PROJ_NAME).elf: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)
	$(OBJDUMP) -x -S $(PROJ_NAME).elf > $(PROJ_NAME).lst
	$(OBJCOPY) -O ihex $(PROJ_NAME).elf $(PROJ_NAME).hex
	$(OBJCOPY) -O srec $(PROJ_NAME).elf $(PROJ_NAME).srec
	$(OBJCOPY) -O binary $(PROJ_NAME).elf $(PROJ_NAME).bin

.c.o:
	$(CC) $(CFLAGS) $(DEFINES) $< -o $@

.s.o:
	$(CC) $(CFLAGS) $(DEFINES) $< -o $@

flash: all
	~/bin/st-flash write $(PROJ_NAME).bin 0x08000000

clean:
	rm -f $(SOURCEOBJS)
	rm -f src/*.o
	rm -f lib/*.o
	rm -f lib/*.a
	rm -f tinyjs/*.o
	rm -f $(PROJ_NAME).elf
	rm -f $(PROJ_NAME).hex
	rm -f $(PROJ_NAME).bin
	rm -f $(PROJ_NAME).srec
	rm -f $(PROJ_NAME).lst
