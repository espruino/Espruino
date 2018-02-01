#
# Definitions for the build of the ATSAM3X8E
#

ARM=1
ARM_HAS_OWN_CMSIS=1
DEFINES += -DARM
export CCPREFIX?=arm-none-eabi-

CROSS_COMPILE = arm-none-eabi-
AR = $(CROSS_COMPILE)ar
CC = $(CROSS_COMPILE)gcc
CXX = $(CROSS_COMPILE)g++
AS = $(CROSS_COMPILE)as
NM = $(CROSS_COMPILE)nm
LKELF = $(CROSS_COMPILE)g++
OBJCP = $(CROSS_COMPILE)objcopy

INCLUDE += -I$(ROOT)/targetlibs/samd/include
INCLUDE += -I$(ROOT)/targetlibs/samd/sam
INCLUDE += -I$(ROOT)/targetlibs/samd/sam/libsam
INCLUDE += -I$(ROOT)/targetlibs/samd/sam/CMSIS/CMSIS/Include
INCLUDE += -I$(ROOT)/targetlibs/samd/sam/CMSIS/Device/ATMEL

CFLAGS += -Wall --param max-inline-insns-single=500 -mcpu=cortex-m3 -mthumb -mlong-calls
CFLAGS += -ffunction-sections -fdata-sections -nostdlib -std=c99
CFLAGS += -Os $(INCLUDE)

LINKER_FILE=$(ROOT)/targetlibs/samd/sam/linker_scripts/gcc/flash.ld

SOURCES += targets/samd/flash.c
SOURCES += targets/samd/jshardware.c
SOURCES += targets/samd/main.c

LDFLAGS += $(INCLUDES) -Os -L -lgcc -mthumb
LDFLAGS += -mcpu=cortex-m3 -Wl,--entry=Reset_Handler
LDFLAGS += -mthumb

LIBS += $(ROOT)/targetlibs/samd/lib/libsam_sam3x8e_gcc_rel.a

OPTIMIZEFLAGS += -fno-common -fno-exceptions -fdata-sections -ffunction-sections
OPTIMIZEFLAGS += -flto -fno-fat-lto-objects -Wl,--allow-multiple-definition
DEFINES += -DLINK_TIME_OPTIMISATION

include make/common/ARM.make

proj: $(PROJ_NAME).bin $(PROJ_NAME).hex

$(PROJ_NAME).hex: $(PROJ_NAME).elf
	@echo $(call $(quiet_)obj_to_bin,ihex,hex)
	@$(call obj_to_bin,ihex,hex)
