NRF5X=1

ifdef NRF_SDK15
# Use SDK15
NRF5X_SDK=15
NRF5X_SDK_15=1
NRF5X_SDK_PATH=$(ROOT)/targetlibs/nrf5x_15
DEFINES += -DNRF_SD_BLE_API_VERSION=6
DEFINES += -D__HEAP_SIZE=0
SOFTDEVICE_PATH = $(NRF5X_SDK_PATH)/components/softdevice/s132
SOFTDEVICE        = $(SOFTDEVICE_PATH)/hex/s132_nrf52_6.0.0_softdevice.hex
DEFINES += -DS132

TARGETSOURCES += $(NRF5X_SDK_PATH)/modules/nrfx/mdk/system_nrf52.c
else
TARGETSOURCES    += $(NRF5X_SDK_PATH)/components/toolchain/system_nrf52.c
ifdef NRF_SDK14
# Use SDK14
NRF5X_SDK=14
NRF5X_SDK_14=1
NRF5X_SDK_PATH=$(ROOT)/targetlibs/nrf5x_14
DEFINES += -DNRF_SD_BLE_API_VERSION=5
DEFINES += -D__HEAP_SIZE=0
SOFTDEVICE_PATH = $(NRF5X_SDK_PATH)/components/softdevice/s132
SOFTDEVICE        = $(SOFTDEVICE_PATH)/hex/s132_nrf52_5.0.0_softdevice.hex
DEFINES += -DS132
else
# Use SDK12
NRF5X_SDK=12
NRF5X_SDK_12=1
NRF5X_SDK_PATH=$(ROOT)/targetlibs/nrf5x_12
DEFINES += -DNRF_SD_BLE_API_VERSION=3
SOFTDEVICE_PATH = $(NRF5X_SDK_PATH)/components/softdevice/s132
SOFTDEVICE        = $(SOFTDEVICE_PATH)/hex/s132_nrf52_3.0.0_softdevice.hex
DEFINES += -DS132
endif
endif

# ARCHFLAGS are shared by both CFLAGS and LDFLAGS.
ARCHFLAGS = -mcpu=cortex-m4 -mthumb -mabi=aapcs -mfloat-abi=hard -mfpu=fpv4-sp-d16

# nRF52 specific.
INCLUDE          += -I$(SOFTDEVICE_PATH)/headers
INCLUDE          += -I$(SOFTDEVICE_PATH)/headers/nrf52
ifdef NRF5X_SDK_15
PRECOMPILED_OBJS += $(NRF5X_SDK_PATH)/modules/nrfx/mdk/gcc_startup_nrf52.S # FIXME variants for 52840 as well
else
PRECOMPILED_OBJS += $(NRF5X_SDK_PATH)/components/toolchain/gcc/gcc_startup_nrf52.o
endif

DEFINES += -DSWI_DISABLE0 -DSOFTDEVICE_PRESENT -DFLOAT_ABI_HARD -DNRF52 -DNRF52832_XXAA -DNRF52_PAN_74 -DBLE_STACK_SUPPORT_REQD

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
# Random hardware requirements (only NRF52)
INCLUDE += -I$(NRF5X_SDK_PATH)/components/drivers_nrf/i2s
ifdef NRF5X_SDK_14
TARGETSOURCES += $(NRF5X_SDK_PATH)/components/drivers_nrf/i2s/nrf_drv_i2s.c
TARGETSOURCES += $(NRF5X_SDK_PATH)/components/drivers_nrf/hal/nrf_saadc.c 
TARGETSOURCES += $(NRF5X_SDK_PATH)/components/drivers_nrf/rng/nrf_drv_rng.c
endif
ifdef NRF5X_SDK_15
TARGETSOURCES += $(NRF5X_SDK_PATH)/modules/nrfx/drivers/src/nrfx_i2s.c
TARGETSOURCES += $(NRF5X_SDK_PATH)/modules/nrfx/drivers/src/nrfx_saadc.c 
TARGETSOURCES += $(NRF5X_SDK_PATH)/modules/nrfx/drivers/src/nrfx_rng.c
endif

# Secure connection support
INCLUDE += -I$(NRF5X_SDK_PATH)/components/libraries/ecc
TARGETSOURCES += $(NRF5X_SDK_PATH)/components/libraries/ecc/ecc.c
INCLUDE += -I$(NRF5X_SDK_PATH)/components/drivers_nrf/rng
INCLUDE += -I$(NRF5X_SDK_PATH)/external/micro-ecc
TARGETSOURCES += $(NRF5X_SDK_PATH)/external/micro-ecc/uECC.c
endif

include make/common/NRF5X.make
