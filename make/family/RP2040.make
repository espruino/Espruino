#
# Definitions for the build of the RP2040
#

RP2040=1
RP2_FAMILY = rp2040

DEFINES += -DRP2040 -DARM -DESPR_DEFINES_ON_COMMANDLINE
INCLUDE += -I$(ROOT)/targets/rp2xxx
INCLUDE += -I$(ROOT)/targetlibs/arm

SOURCES += targets/rp2xxx/main.c \
targets/rp2xxx/jshardware.c \
targets/rp2xxx/usb_descriptors.c
