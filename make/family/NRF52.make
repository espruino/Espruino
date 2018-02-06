NRF5X=1

# Use SDK12
NRF5X_SDK=12
NRF5X_SDK_12=1
NRF5X_SDK_PATH=$(ROOT)/targetlibs/nrf5x_12
DEFINES += -DNRF_SD_BLE_API_VERSION=3
SOFTDEVICE        = $(NRF5X_SDK_PATH)/components/softdevice/s132/hex/s132_nrf52_3.0.0_softdevice.hex

# Use SDK14
#NRF5X_SDK=14
#NRF5X_SDK_14=1
#NRF5X_SDK_PATH=$(ROOT)/targetlibs/nrf5x_14
#DEFINES += -DNRF_SD_BLE_API_VERSION=5
#SOFTDEVICE        = $(NRF5X_SDK_PATH)/components/softdevice/s132/hex/s132_nrf52_5.0.0_softdevice.hex

# ARCHFLAGS are shared by both CFLAGS and LDFLAGS.
ARCHFLAGS = -mcpu=cortex-m4 -mthumb -mabi=aapcs -mfloat-abi=hard -mfpu=fpv4-sp-d16

# nRF52 specific.
INCLUDE          += -I$(NRF5X_SDK_PATH)/components/softdevice/s132/headers
INCLUDE          += -I$(NRF5X_SDK_PATH)/components/softdevice/s132/headers/nrf52
TARGETSOURCES    += $(NRF5X_SDK_PATH)/components/toolchain/system_nrf52.c \
                    $(NRF5X_SDK_PATH)/components/drivers_nrf/hal/nrf_saadc.c
PRECOMPILED_OBJS += $(NRF5X_SDK_PATH)/components/toolchain/gcc/gcc_startup_nrf52.o

DEFINES += -DSWI_DISABLE0 -DSOFTDEVICE_PRESENT -DFLOAT_ABI_HARD -DNRF52 -DNRF52832_XXAA -DNRF52_PAN_74 -DS132 -DBLE_STACK_SUPPORT_REQD

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

ifndef BOOTLOADER
# BLE HID Support (only NRF52)
INCLUDE          += -I$(NRF5X_SDK_PATH)/components/ble/ble_services/ble_hids
TARGETSOURCES    += $(NRF5X_SDK_PATH)/components/ble/ble_services/ble_hids/ble_hids.c
# Neopixel support (only NRF52)
INCLUDE += -I$(NRF5X_SDK_PATH)/components/drivers_nrf/i2s
TARGETSOURCES += $(NRF5X_SDK_PATH)/components/drivers_nrf/i2s/nrf_drv_i2s.c
# Secure connection support

INCLUDE += -I$(NRF5X_SDK_PATH)/components/libraries/ecc
TARGETSOURCES += $(NRF5X_SDK_PATH)/components/libraries/ecc/ecc.c
INCLUDE += -I$(NRF5X_SDK_PATH)/components/drivers_nrf/rng
TARGETSOURCES += $(NRF5X_SDK_PATH)/components/drivers_nrf/rng/nrf_drv_rng.c
INCLUDE += -I$(NRF5X_SDK_PATH)/external/micro-ecc
TARGETSOURCES += $(NRF5X_SDK_PATH)/external/micro-ecc/uECC.c
endif

include make/common/NRF5X.make
