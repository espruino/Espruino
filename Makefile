#OLIMEX=1 for OLIMEXINO
#STM32VLDISCOVERY
#STM32F4DISCOVERY
# Or nothing for standard linux compile

#OLIMEX=1
#STM32VLDISCOVERY=1
#STM32F4DISCOVERY=1


MAKEFLAGS=-j5 # multicore

INCLUDE=-I$(ROOT)/targets -I$(ROOT)/src
LIBS=
DEFINES=
CFLAGS=-Wall -Wextra -Wconversion
OPTIMIZEFLAGS= 

# Espruino flags...
USE_MATH=1

CWD = $(shell pwd)
ROOT=$(CWD)

###################################################

ifdef OLIMEX
PROJ_NAME=espruino_olimexino_stm32
USB=1
FAMILY=STM32F1
CHIP=STM32F103
BOARD=OLIMEXINO_STM32
STLIB=STM32F10X_MD
STARTUP_CODE=$(ROOT)/targets/stm32f1/lib/startup_stm32f10x_md
OPTIMIZEFLAGS+=-O2 # short on program memory
else ifdef STM32F4DISCOVERY
PROJ_NAME=espruino_stm32f4discovery
USB=1
DEFINES += -DUSE_USB_OTG_FS=1
FAMILY=STM32F4
CHIP=STM32F407
BOARD=STM32F4DISCOVERY
STLIB=STM32F4XX
STARTUP_CODE=$(ROOT)/targets/stm32f4/lib/startup_stm32f4xx
OPTIMIZEFLAGS+=-O3
else ifdef STM32VLDISCOVERY
PROJ_NAME=espruino_stm32vldiscovery
FAMILY=STM32F1
CHIP=STM32F100
BOARD=STM32VLDISCOVERY
STLIB=STM32F10X_MD_VL
STARTUP_CODE=$(ROOT)/targets/stm32f1/lib/startup_stm32f10x_md_vl
OPTIMIZEFLAGS+=-O2 # short on program memory
else
PROJ_NAME=espruino
LINUX=1
endif


SOURCES= \
src/jslex.c \
src/jsvar.c \
src/jsfunctions.c \
src/jsutils.c \
src/jsparse.c \
src/jsinteractive.c \
src/jsdevices.c 


ifdef USE_MATH
DEFINES += -DUSE_MATH
ifndef LINUX 
INCLUDE += -I$(ROOT)/math
SOURCES += \
libs/math/acosh.c \
libs/math/asin.c \
libs/math/asinh.c \
libs/math/atan.c \
libs/math/atanh.c \
libs/math/cbrt.c \
libs/math/chbevl.c \
libs/math/clog.c \
libs/math/cmplx.c \
libs/math/const.c \
libs/math/cosh.c \
libs/math/drand.c \
libs/math/exp10.c \
libs/math/exp2.c \
libs/math/exp.c \
libs/math/fabs.c \
libs/math/floor.c \
libs/math/isnan.c \
libs/math/log10.c \
libs/math/log2.c \
libs/math/log.c \
libs/math/mtherr.c \
libs/math/polevl.c \
libs/math/pow.c \
libs/math/powi.c \
libs/math/round.c \
libs/math/setprec.c \
libs/math/sin.c \
libs/math/sincos.c \
libs/math/sindg.c \
libs/math/sinh.c \
libs/math/sqrt.c \
libs/math/tan.c \
libs/math/tandg.c \
libs/math/tanh.c \
libs/math/unity.c
#libs/math/mod2pi.c 
#libs/math/mtst.c 
#libs/math/dtestvec.c 
endif
endif

ifdef USB
DEFINES += -DUSB
endif

ifeq ($(FAMILY), STM32F1)
ARCHFLAGS += -mlittle-endian -mthumb -mcpu=cortex-m3  -mfix-cortex-m3-ldrd  -mthumb-interwork -mfloat-abi=soft
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
ARCHFLAGS += -mlittle-endian -mthumb -mcpu=cortex-m4 -mthumb-interwork -mfpu=fpv4-sp-d16 -mfloat-abi=softfp
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
DEFINES += -DARM -DFAKE_STDLIB
# FAKE_STDLIB is for TinyJS - it uses its own standard library so we don't have to link in the normal one + get bloated 
INCLUDE += -I$(ROOT)/targets/arm
OPTIMIZEFLAGS += -fno-common -fno-exceptions -fdata-sections -ffunction-sections

# 4.6
#export CCPREFIX=arm-linux-gnueabi-
# 4.5
#export CCPREFIX=~/sat/bin/arm-none-eabi-
# 4.4
export CCPREFIX=arm-none-eabi-
endif

ifdef STM32
DEFINES += -DSTM32 -DUSE_STDPERIPH_DRIVER=1 -D$(CHIP) -D$(BOARD) -D$(STLIB)
INCLUDE += -I$(ROOT)/targets/stm32
SOURCES +=                              \
targets/stm32/main.c                    \
targets/stm32/jshardware.c              \
targets/stm32/stm32_it.c
endif

ifdef LINUX
OPTIMIZEFLAGS += -g # DEBUG
DEFINES += -DLINUX
INCLUDE += -I$(ROOT)/targets/linux 
SOURCES +=                              \
targets/linux/main.c                    \
targets/linux/jshardware.c  
LIBS += -lm # maths lib
endif

SOURCEOBJS = $(SOURCES:.c=.o)
OBJS = $(SOURCEOBJS)
ifdef STARTUP_CODE
OBJS += $(LIB_ROOT)/$(STARTUP_CODE).o # add startup file to build
endif


# -ffreestanding -nodefaultlibs -nostdlib -fno-common
# -nodefaultlibs -nostdlib -nostartfiles

# -fdata-sections -ffunction-sections are to help remove unused code
CFLAGS += $(OPTIMIZEFLAGS) -c $(ARCHFLAGS) $(DEFINES) $(INCLUDE)

# -Wl,--gc-sections helps remove unused code
# -Wl,--whole-archive checks for duplicates
LDFLAGS += $(ARCHFLAGS) -Wl,--gc-sections
ifdef CHIP 
LDFLAGS += -Tlinker/$(CHIP).ld
endif 

export CC=$(CCPREFIX)gcc
export LD=$(CCPREFIX)gcc
export AR=$(CCPREFIX)ar
export AS=$(CCPREFIX)as
export OBJCOPY=$(CCPREFIX)objcopy
export OBJDUMP=$(CCPREFIX)objdump
.PHONY:  proj

all: 	 proj

.c.o:
	$(CC) $(CFLAGS) $(DEFINES) $< -o $@

.s.o:
	$(CC) $(CFLAGS) $(DEFINES) $< -o $@

ifdef LINUX # ---------------------------------------------------
proj: 	$(PROJ_NAME)

$(PROJ_NAME): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)
else # embedded, so generate bin, etc ---------------------------
proj: 	$(PROJ_NAME).elf

$(PROJ_NAME).elf: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)
	$(OBJDUMP) -x -S $(PROJ_NAME).elf > $(PROJ_NAME).lst
	$(OBJCOPY) -O ihex $(PROJ_NAME).elf $(PROJ_NAME).hex
	$(OBJCOPY) -O srec $(PROJ_NAME).elf $(PROJ_NAME).srec
	$(OBJCOPY) -O binary $(PROJ_NAME).elf $(PROJ_NAME).bin
	
flash: all
	~/bin/st-flash write $(PROJ_NAME).bin 0x08000000	
endif	    # ---------------------------------------------------
 
clean:
	find . -name *.o -delete
	rm -f $(PROJ_NAME).elf
	rm -f $(PROJ_NAME).hex
	rm -f $(PROJ_NAME).bin
	rm -f $(PROJ_NAME).srec
	rm -f $(PROJ_NAME).lst
