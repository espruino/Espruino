NRF5X=1

ifeq ($(CHIP),NRF52840)
SOFTDEVICE_PATH = $(NRF5X_SDK_PATH)/components/softdevice/s140
DEFINES += -DS140
else  # NRF52832
SOFTDEVICE_PATH = $(NRF5X_SDK_PATH)/components/softdevice/s132
DEFINES += -DS132
endif

ifdef NRF_SDK15
# Use SDK15
NRF5X_SDK=15
NRF5X_SDK_15=1
NRF5X_SDK_PATH=$(ROOT)/targetlibs/nrf5x_15
DEFINES += -DNRF_SD_BLE_API_VERSION=6
DEFINES += -D__HEAP_SIZE=0
ifeq ($(CHIP),NRF52840)
TARGETSOURCES += $(NRF5X_SDK_PATH)/modules/nrfx/mdk/system_nrf52840.c
SOFTDEVICE      = $(SOFTDEVICE_PATH)/hex/s140_nrf52_6.0.0_softdevice.hex
PRECOMPILED_OBJS += $(NRF5X_SDK_PATH)/modules/nrfx/mdk/gcc_startup_nrf52840.S
else  # NRF52832
TARGETSOURCES += $(NRF5X_SDK_PATH)/modules/nrfx/mdk/system_nrf52.c
SOFTDEVICE        = $(SOFTDEVICE_PATH)/hex/s132_nrf52_6.0.0_softdevice.hex
PRECOMPILED_OBJS += $(NRF5X_SDK_PATH)/modules/nrfx/mdk/gcc_startup_nrf52.S
endif
else # not SDK15
PRECOMPILED_OBJS += $(NRF5X_SDK_PATH)/components/toolchain/gcc/gcc_startup_nrf52.o
TARGETSOURCES    += $(NRF5X_SDK_PATH)/components/toolchain/system_nrf52.c
ifdef NRF_SDK14
# Use SDK14
NRF5X_SDK=14
NRF5X_SDK_14=1
NRF5X_SDK_PATH=$(ROOT)/targetlibs/nrf5x_14
DEFINES += -DNRF_SD_BLE_API_VERSION=5
DEFINES += -D__HEAP_SIZE=0
ifeq ($(CHIP),NRF52840)
SOFTDEVICE      = $(SOFTDEVICE_PATH)/hex/s140_nrf52840_5.0.0-2.alpha_softdevice.hex
else  # NRF52832
SOFTDEVICE        = $(SOFTDEVICE_PATH)/hex/s132_nrf52_5.0.0_softdevice.hex
endif
else
ifdef NRF_SDK11
# Use SDK11
NRF5X_SDK=11
NRF5X_SDK_11=1
NRF5X_SDK_PATH=$(ROOT)/targetlibs/nrf5x_11
DEFINES += -DNRF_SD_BLE_API_VERSION=2
SOFTDEVICE        = $(SOFTDEVICE_PATH)/hex/s132_nrf52_2.0.0_softdevice.hex
else
# Use SDK12
NRF5X_SDK=12
NRF5X_SDK_12=1
NRF5X_SDK_PATH=$(ROOT)/targetlibs/nrf5x_12
DEFINES += -DNRF_SD_BLE_API_VERSION=3
SOFTDEVICE        = $(SOFTDEVICE_PATH)/hex/s132_nrf52_3.0.0_softdevice.hex
endif
endif
endif

# ARCHFLAGS are shared by both CFLAGS and LDFLAGS.
ARCHFLAGS = -mcpu=cortex-m4 -mthumb -mabi=aapcs -mfloat-abi=hard -mfpu=fpv4-sp-d16

# nRF52 specific.
INCLUDE          += -I$(SOFTDEVICE_PATH)/headers
INCLUDE          += -I$(SOFTDEVICE_PATH)/headers/nrf52

DEFINES += -DBLE_STACK_SUPPORT_REQD
DEFINES += -DSWI_DISABLE0 -DSOFTDEVICE_PRESENT -DFLOAT_ABI_HARD 

# NOTE: nrf.h needs tweaking as Nordic randomly changed NRF52 to NRF52_SERIES
ifeq ($(CHIP),NRF52840)
DEFINES += -DNRF52 -DNRF52840_XXAA
LINKER_FILE = $(NRF5X_SDK_PATH)/nrf5x_linkers/linker_nrf52840_ble_espruino.ld
INCLUDE += -I$(NRF5X_SDK_PATH)/nrf52_config
INCLUDE += -I$(NRF5X_SDK_PATH)/components/libraries/usbd
INCLUDE += -I$(NRF5X_SDK_PATH)/components/libraries/usbd/class/cdc
INCLUDE += -I$(NRF5X_SDK_PATH)/components/libraries/usbd/class/cdc/acm
INCLUDE += -I$(NRF5X_SDK_PATH)/components/drivers_nrf/usbd
INCLUDE += -I$(NRF5X_SDK_PATH)/components/drivers_nrf/power
INCLUDE += -I$(NRF5X_SDK_PATH)/components/drivers_nrf/systick
TARGETSOURCES += $(NRF5X_SDK_PATH)/components/libraries/usbd/app_usbd.c
TARGETSOURCES += $(NRF5X_SDK_PATH)/components/libraries/usbd/app_usbd_core.c
TARGETSOURCES += $(NRF5X_SDK_PATH)/components/libraries/usbd/app_usbd_string_desc.c
TARGETSOURCES += $(NRF5X_SDK_PATH)/components/libraries/usbd/class/cdc/acm/app_usbd_cdc_acm.c
TARGETSOURCES += $(NRF5X_SDK_PATH)/components/drivers_nrf/usbd/nrf_drv_usbd.c
TARGETSOURCES += $(NRF5X_SDK_PATH)/components/libraries/usbd/app_usbd_serial_num.c
TARGETSOURCES += $(NRF5X_SDK_PATH)/integration/nrfx/legacy/nrf_drv_clock.c
TARGETSOURCES += $(NRF5X_SDK_PATH)/integration/nrfx/legacy/nrf_drv_power.c
TARGETSOURCES += $(NRF5X_SDK_PATH)/modules/nrfx/drivers/src/nrfx_clock.c
TARGETSOURCES += $(NRF5X_SDK_PATH)/modules/nrfx/drivers/src/nrfx_power.c
else # NRF52832
DEFINES += -DNRF52 -DNRF52832_XXAA -DNRF52_PAN_74 

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
else # not USE_BOOTLOADER
LINKER_FILE = $(NRF5X_SDK_PATH)/nrf5x_linkers/linker_nrf52_ble_espruino.ld
INCLUDE += -I$(NRF5X_SDK_PATH)/nrf52_config
endif # USE_BOOTLOADER
endif # NRF52832 / NRF52840



ifndef BOOTLOADER
# BLE HID Support (only NRF52)
INCLUDE          += -I$(NRF5X_SDK_PATH)/components/ble/ble_services/ble_hids
TARGETSOURCES    += $(NRF5X_SDK_PATH)/components/ble/ble_services/ble_hids/ble_hids.c
# Random hardware requirements (only NRF52)
INCLUDE += -I$(NRF5X_SDK_PATH)/components/drivers_nrf/i2s
ifdef NRF5X_SDK_12
TARGETSOURCES += $(NRF5X_SDK_PATH)/components/drivers_nrf/i2s/nrf_drv_i2s.c
TARGETSOURCES += $(NRF5X_SDK_PATH)/components/drivers_nrf/hal/nrf_saadc.c 
TARGETSOURCES += $(NRF5X_SDK_PATH)/components/drivers_nrf/rng/nrf_drv_rng.c
endif
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
endif # BOOTLOADER

include make/common/NRF5X.make
