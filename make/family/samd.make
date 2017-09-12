#
# Definitions for the build of the ATSAM3X8E
#

SAMD=1

ARCHFLAGS += -mcpu=cortex-m3

CFLAGS+=-c -Wall --param max-inline-insns-single=500 -mcpu=cortex-m3 -ffunction-sections -fdata-sections -nostdlib -std=c99 -Os
SOURCES += targets/samd/jshardware.c
INCLUDE += -I$(ROOT)/targets/samd

SOURCES += targets/samd/main.c
LDFLAGS += -L$(ROOT)/targetlibs/samd/include/ \
-L$(ROOT)/targetlibs/samd/sam/ \
-L$(ROOT)/targetlibs/samd/sam/libsam \
-L$(ROOT)/targetlibs/samd/sam/CMSIS/CMSIS/Include \
-L$(ROOT)/targetlibs/samd/sam/CMSIS/Device/ATMEL
