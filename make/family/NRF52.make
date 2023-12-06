NRF5X=1

ifeq ($(CHIP),NRF52832)
SOFTDEVICE_PATH = $(NRF5X_SDK_PATH)/components/softdevice/s132
DEFINES += -DS132
else  # NRF52833/40
SOFTDEVICE_PATH = $(NRF5X_SDK_PATH)/components/softdevice/s140
DEFINES += -DS140
endif

ifdef NRF_SDK17
# Use SDK17
NRF5X_SDK=17
NRF5X_SDK_17=1
NRF5X_SDK_PATH=targetlibs/nrf5x_17
DEFINES += -DNRF_SD_BLE_API_VERSION=7
DEFINES += -D__HEAP_SIZE=0
ifeq ($(CHIP),NRF52840)
TARGETSOURCES += $(NRF5X_SDK_PATH)/modules/nrfx/mdk/system_nrf52840.c
SOFTDEVICE      = $(SOFTDEVICE_PATH)/hex/s140_nrf52_7.2.0_softdevice.hex
PRECOMPILED_OBJS += $(NRF5X_SDK_PATH)/modules/nrfx/mdk/gcc_startup_nrf52840.S
else  # NRF52832/3
ifeq ($(CHIP),NRF52833)
TARGETSOURCES += $(NRF5X_SDK_PATH)/modules/nrfx/mdk/system_nrf52833.c
SOFTDEVICE      = $(SOFTDEVICE_PATH)/hex/s140_nrf52_7.2.0_softdevice.hex
PRECOMPILED_OBJS += $(NRF5X_SDK_PATH)/modules/nrfx/mdk/gcc_startup_nrf52833.S
else
TARGETSOURCES += $(NRF5X_SDK_PATH)/modules/nrfx/mdk/system_nrf52.c
SOFTDEVICE        = $(SOFTDEVICE_PATH)/hex/s132_nrf52_7.2.0_softdevice.hex
PRECOMPILED_OBJS += $(NRF5X_SDK_PATH)/modules/nrfx/mdk/gcc_startup_nrf52.S
endif
endif
else ifdef NRF_SDK15
# Use SDK15
NRF5X_SDK=15
NRF5X_SDK_15=1
DEFINES += -DNRF_SD_BLE_API_VERSION=6
DEFINES += -D__HEAP_SIZE=0
NRF5X_SDK_PATH=targetlibs/nrf5x_15
ifeq ($(CHIP),NRF52840)
TARGETSOURCES += $(NRF5X_SDK_PATH)/modules/nrfx/mdk/system_nrf52840.c
SOFTDEVICE      = $(SOFTDEVICE_PATH)/hex/s140_nrf52_6.0.0_softdevice.hex
PRECOMPILED_OBJS += $(NRF5X_SDK_PATH)/modules/nrfx/mdk/gcc_startup_nrf52840.S
else  # NRF52832
TARGETSOURCES += $(NRF5X_SDK_PATH)/modules/nrfx/mdk/system_nrf52.c
SOFTDEVICE        = $(SOFTDEVICE_PATH)/hex/s132_nrf52_6.0.0_softdevice.hex
PRECOMPILED_OBJS += $(NRF5X_SDK_PATH)/modules/nrfx/mdk/gcc_startup_nrf52.S
endif
else ifdef NRF_SDK15_3
# Use SDK15
NRF5X_SDK=15
NRF5X_SDK_15=1
DEFINES += -DNRF_SD_BLE_API_VERSION=6
DEFINES += -D__HEAP_SIZE=0
NRF5X_SDK_15_3=1
DEFINES += -DNRF5X_SDK_15_3=1
NRF5X_SDK_PATH=targetlibs/nrf5x_15_3
ifeq ($(CHIP),NRF52840)
TARGETSOURCES += $(NRF5X_SDK_PATH)/modules/nrfx/mdk/system_nrf52840.c
SOFTDEVICE      = $(SOFTDEVICE_PATH)/hex/s140_nrf52_6.1.1_softdevice.hex
PRECOMPILED_OBJS += $(NRF5X_SDK_PATH)/modules/nrfx/mdk/gcc_startup_nrf52840.S
else  # NRF52832
TARGETSOURCES += $(NRF5X_SDK_PATH)/modules/nrfx/mdk/system_nrf52.c
SOFTDEVICE        = $(SOFTDEVICE_PATH)/hex/s132_nrf52_6.1.1_softdevice.hex
# SOFTDEVICE        = targetlibs/nrf5x_15/components/softdevice/s132/hex/s132_nrf52_6.0.0_softdevice.hex # force old SDK15 softdevice
PRECOMPILED_OBJS += $(NRF5X_SDK_PATH)/modules/nrfx/mdk/gcc_startup_nrf52.S
endif
else # not SDK15/SDK17
PRECOMPILED_OBJS += $(NRF5X_SDK_PATH)/components/toolchain/gcc/gcc_startup_nrf52.o
TARGETSOURCES    += $(NRF5X_SDK_PATH)/components/toolchain/system_nrf52.c
ifdef NRF_SDK14
# Use SDK14
NRF5X_SDK=14
NRF5X_SDK_14=1
NRF5X_SDK_PATH=targetlibs/nrf5x_14
DEFINES += -DNRF_SD_BLE_API_VERSION=5
DEFINES += -D__HEAP_SIZE=0
ifeq ($(CHIP),NRF52840)
SOFTDEVICE      = $(SOFTDEVICE_PATH)/hex/s140_nrf52840_5.0.0-2.alpha_softdevice.hex
else  # NRF52832
SOFTDEVICE        = $(SOFTDEVICE_PATH)/hex/s132_nrf52_5.0.0_softdevice.hex
endif
else ifdef NRF_SDK11
# Use SDK11
NRF5X_SDK=11
NRF5X_SDK_11=1
NRF5X_SDK_PATH=targetlibs/nrf5x_11
DEFINES += -DNRF_SD_BLE_API_VERSION=2
SOFTDEVICE        = $(SOFTDEVICE_PATH)/hex/s132_nrf52_2.0.0_softdevice.hex
else # Use SDK12!
NRF5X_SDK=12
NRF5X_SDK_12=1
NRF5X_SDK_PATH=targetlibs/nrf5x_12
DEFINES += -DNRF_SD_BLE_API_VERSION=3
SOFTDEVICE        = $(SOFTDEVICE_PATH)/hex/s132_nrf52_3.1.0_softdevice.hex
endif
endif

# ARCHFLAGS are shared by both CFLAGS and LDFLAGS.
ARCHFLAGS += -mcpu=cortex-m4 -mthumb -mabi=aapcs
ARCHFLAGS += -mfloat-abi=softfp -mfpu=fpv4-sp-d16
# Espruino uses doubles, not floats - so actually using hardfp doesn't actually help us much and adds register-swapping overhead

# nRF52 specific.
INCLUDE          += -I$(SOFTDEVICE_PATH)/headers
INCLUDE          += -I$(SOFTDEVICE_PATH)/headers/nrf52

DEFINES += -DBLE_STACK_SUPPORT_REQD
DEFINES += -DSWI_DISABLE0 -DSOFTDEVICE_PRESENT
DEFINES += -DNRF52_SERIES
# Nordic screwed over anyone who used -DNRF52 in new SDK versions
# but then old SDKs won't work without it
ifneq ($(or $(NRF5X_SDK_12),$(NRF5X_SDK_11)),)
DEFINES += -DNRF52
endif

# NOTE: nrf.h needs tweaking as Nordic randomly changed NRF52 to NRF52_SERIES
ifeq ($(CHIP),NRF52840)
DEFINES += -DNRF52840_XXAA
LINKER_BOOTLOADER ?= $(NRF5X_SDK_PATH)/nrf5x_linkers/secure_bootloader_gcc_nrf52.ld
ifdef USE_BOOTLOADER
LINKER_ESPRUINO ?= $(NRF5X_SDK_PATH)/nrf5x_linkers/linker_nrf52840_ble_espruino.ld
else # USE_BOOTLOADER
LINKER_ESPRUINO ?= $(NRF5X_SDK_PATH)/nrf5x_linkers/linker_nrf52840_ble_espruino.ld
endif # USE_BOOTLOADER
else # not NRF52840
ifeq ($(CHIP),NRF52833)
DEFINES += -DNRF52833_XXAA
LINKER_BOOTLOADER ?= $(NRF5X_SDK_PATH)/nrf5x_linkers/secure_bootloader_gcc_nrf52.ld
ifdef USE_BOOTLOADER
LINKER_ESPRUINO ?= $(NRF5X_SDK_PATH)/nrf5x_linkers/linker_nrf52833_ble_espruino.ld
else # USE_BOOTLOADER
LINKER_ESPRUINO ?= $(NRF5X_SDK_PATH)/nrf5x_linkers/linker_nrf52833_ble_espruino.ld
endif # USE_BOOTLOADER
else # must be NRF52832
DEFINES += -DNRF52832_XXAA -DNRF52_PAN_74
LINKER_BOOTLOADER ?= $(NRF5X_SDK_PATH)/nrf5x_linkers/secure_dfu_gcc_nrf52.ld
ifdef USE_BOOTLOADER
LINKER_ESPRUINO ?= $(NRF5X_SDK_PATH)/nrf5x_linkers/linker_nrf52_ble_espruino_bootloader.ld
else # USE_BOOTLOADER
LINKER_ESPRUINO ?= $(NRF5X_SDK_PATH)/nrf5x_linkers/linker_nrf52_ble_espruino.ld
endif # !USE_BOOTLOADER
endif # NRF52832 / NRF52833
endif # NRF52840 / other


ifdef USE_BOOTLOADER
NRF_BOOTLOADER    = $(BOOTLOADER_PROJ_NAME).hex
endif # USE_BOOTLOADER
ifdef BOOTLOADER
# we're trying to compile the bootloader itself
LINKER_FILE = $(LINKER_BOOTLOADER)
OPTIMIZEFLAGS=-Os -flto -fno-fat-lto-objects -Wl,--allow-multiple-definition # try to reduce bootloader size
else # not BOOTLOADER - compiling something to run under a bootloader
LINKER_FILE = $(LINKER_ESPRUINO)
INCLUDE += -I$(NRF5X_SDK_PATH)/nrf52_config
endif

ifneq (,$(findstring NRF_USB,$(DEFINES)))
ifdef NRF5X_SDK_17
DEFINES += -DNRFX_USBD_ENABLED=1
INCLUDE += -I$(NRF5X_SDK_PATH)/components/libraries/usbd
INCLUDE += -I$(NRF5X_SDK_PATH)/components/libraries/usbd/class/cdc
INCLUDE += -I$(NRF5X_SDK_PATH)/components/libraries/usbd/class/cdc/acm
INCLUDE += -I$(NRF5X_SDK_PATH)/components/libraries/usbd/class/hid/generic
INCLUDE += -I$(NRF5X_SDK_PATH)/components/libraries/usbd/class/msc
INCLUDE += -I$(NRF5X_SDK_PATH)/components/libraries/usbd/class/hid
INCLUDE += -I$(NRF5X_SDK_PATH)/external/utf_converter
TARGETSOURCES += $(NRF5X_SDK_PATH)/components/libraries/usbd/app_usbd.c
TARGETSOURCES += $(NRF5X_SDK_PATH)/components/libraries/usbd/class/cdc/acm/app_usbd_cdc_acm.c
TARGETSOURCES += $(NRF5X_SDK_PATH)/components/libraries/usbd/app_usbd_core.c
TARGETSOURCES += $(NRF5X_SDK_PATH)/components/libraries/usbd/app_usbd_serial_num.c
TARGETSOURCES += $(NRF5X_SDK_PATH)/components/libraries/usbd/app_usbd_string_desc.c
TARGETSOURCES += $(NRF5X_SDK_PATH)/modules/nrfx/drivers/src/nrfx_usbd.c
TARGETSOURCES += $(NRF5X_SDK_PATH)/modules/nrfx/soc/nrfx_atomic.c
endif
ifdef NRF5X_SDK_15
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
endif # NRF5X_SDK_15
endif # USB

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
TARGETSOURCES += $(NRF5X_SDK_PATH)/integration/nrfx/legacy/nrf_drv_clock.c
TARGETSOURCES += $(NRF5X_SDK_PATH)/integration/nrfx/legacy/nrf_drv_power.c
TARGETSOURCES += $(NRF5X_SDK_PATH)/modules/nrfx/drivers/src/nrfx_clock.c
TARGETSOURCES += $(NRF5X_SDK_PATH)/modules/nrfx/drivers/src/nrfx_power.c
endif
ifdef NRF5X_SDK_17
TARGETSOURCES += $(NRF5X_SDK_PATH)/modules/nrfx/drivers/src/nrfx_i2s.c
TARGETSOURCES += $(NRF5X_SDK_PATH)/modules/nrfx/drivers/src/nrfx_saadc.c
TARGETSOURCES += $(NRF5X_SDK_PATH)/modules/nrfx/drivers/src/nrfx_rng.c
TARGETSOURCES += $(NRF5X_SDK_PATH)/integration/nrfx/legacy/nrf_drv_clock.c
TARGETSOURCES += $(NRF5X_SDK_PATH)/integration/nrfx/legacy/nrf_drv_power.c
TARGETSOURCES += $(NRF5X_SDK_PATH)/modules/nrfx/drivers/src/nrfx_clock.c
TARGETSOURCES += $(NRF5X_SDK_PATH)/modules/nrfx/drivers/src/nrfx_power.c
endif
# Secure connection support
INCLUDE += -I$(NRF5X_SDK_PATH)/components/libraries/ecc
TARGETSOURCES += $(NRF5X_SDK_PATH)/components/libraries/ecc/ecc.c
INCLUDE += -I$(NRF5X_SDK_PATH)/components/drivers_nrf/rng
INCLUDE += -I$(NRF5X_SDK_PATH)/external/micro-ecc
TARGETSOURCES += $(NRF5X_SDK_PATH)/external/micro-ecc/uECC.c
endif # !BOOTLOADER

include make/common/NRF5X.make
