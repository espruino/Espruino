NRF5X=1
NRF5X_SDK_PATH=$(ROOT)/targetlibs/nrf5x

# ARCHFLAGS are shared by both CFLAGS and LDFLAGS.
ARCHFLAGS = -mcpu=cortex-m4 -mthumb -mabi=aapcs -mfloat-abi=hard -mfpu=fpv4-sp-d16

# nRF52 specific.
INCLUDE          += -I$(NRF5X_SDK_PATH)/components/softdevice/s132/headers
INCLUDE          += -I$(NRF5X_SDK_PATH)/components/softdevice/s132/headers/nrf52
TARGETSOURCES    += $(NRF5X_SDK_PATH)/components/toolchain/system_nrf52.c \
                    $(NRF5X_SDK_PATH)/components/drivers_nrf/hal/nrf_saadc.c
PRECOMPILED_OBJS += $(NRF5X_SDK_PATH)/components/toolchain/gcc/gcc_startup_nrf52.o

DEFINES += -DSWI_DISABLE0 -DSOFTDEVICE_PRESENT -DNRF52 -DCONFIG_GPIO_AS_PINRESET -DS132 -DBLE_STACK_SUPPORT_REQD
DEFINES += -DNRF_SD_BLE_API_VERSION=3

SOFTDEVICE        = $(NRF5X_SDK_PATH)/components/softdevice/s132/hex/s132_nrf52_3.0.0_softdevice.hex

ifdef USE_BOOTLOADER
NRF_BOOTLOADER    = $(BOOTLOADER_PROJ_NAME).hex
ifdef BOOTLOADER
  # we're trying to compile the bootloader itself
  LINKER_FILE = $(NRF5X_SDK_PATH)/nrf5x_linkers/secure_dfu_gcc_nrf52.ld
  OPTIMIZEFLAGS=-Os # try to reduce bootloader size
else
  LINKER_FILE = $(NRF5X_SDK_PATH)/nrf5x_linkers/linker_nrf52_ble_espruino_bootloader.ld
  INCLUDE += -I$(NRF5X_SDK_PATH)/nrf52_config
endif
else
LINKER_FILE = $(NRF5X_SDK_PATH)/nrf5x_linkers/linker_nrf52_ble_espruino.ld
INCLUDE += -I$(NRF5X_SDK_PATH)/nrf52_config
endif

# BLE HID Support (only NRF52)
INCLUDE          += -I$(NRF5X_SDK_PATH)/components/ble/ble_services/ble_hids
TARGETSOURCES    += $(NRF5X_SDK_PATH)/components/ble/ble_services/ble_hids/ble_hids.c
# Neopixel support (only NRF52)
INCLUDE += -I$(NRF5X_SDK_PATH)/components/drivers_nrf/i2s
TARGETSOURCES += $(NRF5X_SDK_PATH)/components/drivers_nrf/i2s/nrf_drv_i2s.c

include make/common/NRF5X.make
