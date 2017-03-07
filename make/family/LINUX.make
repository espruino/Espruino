$(info QHO)
ifeq ($(BOARD),CARAMBOLA)
TOOLCHAIN_DIR=$(shell cd ~/workspace/carambola/staging_dir/toolchain-*/bin;pwd)
export STAGING_DIR=$(TOOLCHAIN_DIR)
export CCPREFIX=$(TOOLCHAIN_DIR)/mipsel-openwrt-linux-
endif

ifeq ($(BOARD),DPTBOARD)
export STAGING_DIR=$(shell cd ~/breakoutopenwrt/staging_dir/toolchain-*/bin;pwd)
export CCPREFIX=$(STAGING_DIR)/mips-openwrt-linux-
endif

ifeq ($(BOARD),RASPBERRYPI)
 ifneq ($(shell uname -m),armv6l)
  $(info *********************************)
  $(info *         CROSS COMPILING       *)
  $(info *********************************)
  export CCPREFIX=targetlibs/raspberrypi/tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian/bin/arm-linux-gnueabihf-
 else
  # compiling in-place, so give it a normal name
  PROJ_NAME=espruino
 endif
endif

DEFINES += -DLINUX
INCLUDE += -I$(ROOT)/targets/linux
SOURCES +=                              \
targets/linux/main.c                    \
targets/linux/jshardware.c
LIBS += -lpthread # thread lib for input processing
ifdef OPENWRT_UCLIBC
LIBS += -lc
else
LIBS += -lstdc++
endif
