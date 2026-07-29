#
# Definitions for the build of the RP2040
#

RP2040=1

DEFINES += -DRP2040 -DARM -DESPR_DEFINES_ON_COMMANDLINE
INCLUDE += -I$(ROOT)/targets/rp2040
INCLUDE += -I$(ROOT)/targetlibs/arm

SOURCES += targets/rp2040/main.c \
targets/rp2040/jshardware.c \
targets/rp2040/usb_descriptors.c
