ARM=1

ifndef LINKER_FILE # nRF5x targets define their own linker file.
  LINKER_FILE = $(GENDIR)/linker.ld
endif
DEFINES += -DARM
ifndef ARM_HAS_OWN_CMSIS # nRF5x targets do not use the shared CMSIS files.
  INCLUDE += -I$(ROOT)/targetlibs/arm
endif
OPTIMIZEFLAGS += -fno-common -fno-exceptions -fdata-sections -ffunction-sections

# I've no idea why this breaks the bootloader, but it does.
# Given we've left 10k for it, there's no real reason to enable LTO anyway.
ifndef BOOTLOADER
# Enable link-time optimisations (inlining across files)
OPTIMIZEFLAGS += -flto -fno-fat-lto-objects -Wl,--allow-multiple-definition
DEFINES += -DLINK_TIME_OPTIMISATION
endif

export CCPREFIX?=arm-none-eabi-
