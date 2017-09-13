#
# Definitions for the build of the ATSAM3X8E
#

ARM=1

INCLUDES =
INCLUDES += -I$(ROOT)/targetlibs/samd/include
INCLUDES += -I$(ROOT)/targetlibs/samd/sam
INCLUDES += -I$(ROOT)/targetlibs/samd/sam/libsam
INCLUDES += -I$(ROOT)/targetlibs/samd/sam/CMSIS/CMSIS/Include
INCLUDES += -I$(ROOT)/targetlibs/samd/sam/CMSIS/Device/ATMEL

#ARCHFLAGS += -mlittle-endian -mthumb -mcpu=cortex-m3  -mfix-cortex-m3-ldrd -mfloat-abi=soft

CFLAGS += -Wall --param max-inline-insns-single=500 -mcpu=cortex-m3 -mthumb -mlong-calls
CFLAGS += -ffunction-sections -fdata-sections -nostdlib -std=c99
CFLAGS += -Os $(INCLUDES)

LNKER_FILE=$(ROOT)/sam/linker_scripts/gcc/flash.ld

SOURCES += targets/samd/jshardware.c
INCLUDE += $(INCLUDES) 

SOURCES += targets/samd/main.c
LDFLAGS += $(INCLUDES)

include make/common/ARM.make 
