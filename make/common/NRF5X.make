# Just try and get rid of the compile warnings.
CFLAGS += -Wno-sign-conversion -Wno-conversion -Wno-unused-parameter -fomit-frame-pointer #this is for device manager in nordic sdk
DEFINES += -D$(BOARD) -D$(CHIP)

ARM = 1
ARM_HAS_OWN_CMSIS = 1 # Nordic uses its own CMSIS files in its SDK, these are up-to-date.
INCLUDE += -I$(ROOT)/targetlibs/nrf5x -I$(NRF5X_SDK_PATH)

TEMPLATE_PATH = $(ROOT)/targetlibs/nrf5x/nrf5x_linkers # This is where the common linker for both nRF51 & nRF52 is stored.
LDFLAGS += -L$(TEMPLATE_PATH)

# These files are the Espruino HAL implementation.
INCLUDE += -I$(ROOT)/targets/nrf5x
ifdef BOOTLOADER
  BUILD_LINKER_FLAGS+=--bootloader
  PROJ_NAME=$(BOOTLOADER_PROJ_NAME)
  WRAPPERSOURCES =
  INCLUDE += -I$(ROOT)/targets/nrf5x_dfu
  DEFINES += -DSVC_INTERFACE_CALL_AS_NORMAL_FUNCTION
  DEFINES += -DuECC_ENABLE_VLI_API -DuECC_VLI_NATIVE_LITTLE_ENDIAN=1 -DuECC_SQUARE_FUNC=1 -DuECC_SUPPORTS_secp256r1=1 -DuECC_SUPPORT_COMPRESSED_POINT=0 -DuECC_OPTIMIZATION_LEVEL=3
  SOURCES = \
    targets/nrf5x_dfu/dfu-cc.pb.c \
    targets/nrf5x_dfu/dfu_public_key.c \
    targets/nrf5x_dfu/dfu_req_handling.c \
    targets/nrf5x_dfu/main.c
else
  SOURCES +=                              \
    targets/nrf5x/main.c                    \
    targets/nrf5x/jshardware.c              \
    targets/nrf5x/bluetooth.c              \
    targets/nrf5x/bluetooth_utils.c              \
    targets/nrf5x/nrf5x_utils.c

  ifeq ($(FAMILY), NRF52)
    # Neopixel support (only NRF52)
    SOURCES    += targets/nrf5x/i2s_ws2812b_drive.c
  endif
endif

# Careful here.. All these includes and sources assume a SoftDevice. Not efficeint/clean if softdevice (ble) is not enabled...
INCLUDE += -I$(NRF5X_SDK_PATH)/components
INCLUDE += -I$(NRF5X_SDK_PATH)/components/toolchain/cmsis/include
INCLUDE += -I$(NRF5X_SDK_PATH)/components/toolchain/gcc
INCLUDE += -I$(NRF5X_SDK_PATH)/components/toolchain
INCLUDE += -I$(NRF5X_SDK_PATH)/components/libraries/log
INCLUDE += -I$(NRF5X_SDK_PATH)/components/libraries/log/src
INCLUDE += -I$(NRF5X_SDK_PATH)/components/drivers_nrf/config
INCLUDE += -I$(NRF5X_SDK_PATH)/components/libraries/fstorage/config
INCLUDE += -I$(NRF5X_SDK_PATH)/components/libraries/util
INCLUDE += -I$(NRF5X_SDK_PATH)/components/drivers_nrf/delay
INCLUDE += -I$(NRF5X_SDK_PATH)/components/drivers_nrf/uart
INCLUDE += -I$(NRF5X_SDK_PATH)/components/ble/common
INCLUDE += -I$(NRF5X_SDK_PATH)/components/libraries/uart
INCLUDE += -I$(NRF5X_SDK_PATH)/components/device
INCLUDE += -I$(NRF5X_SDK_PATH)/components/libraries/button
INCLUDE += -I$(NRF5X_SDK_PATH)/components/libraries/timer
INCLUDE += -I$(NRF5X_SDK_PATH)/components/libraries/fstorage
INCLUDE += -I$(NRF5X_SDK_PATH)/components/libraries/experimental_section_vars
INCLUDE += -I$(NRF5X_SDK_PATH)/components/drivers_nrf/gpiote
INCLUDE += -I$(NRF5X_SDK_PATH)/components/ble/ble_services/ble_nus
INCLUDE += -I$(NRF5X_SDK_PATH)/components/drivers_nrf/hal
INCLUDE += -I$(NRF5X_SDK_PATH)/components/drivers_nrf/common
INCLUDE += -I$(NRF5X_SDK_PATH)/components/ble/ble_advertising
INCLUDE += -I$(NRF5X_SDK_PATH)/components/softdevice/common/softdevice_handler
INCLUDE += -I$(NRF5X_SDK_PATH)/components/drivers_nrf/twi_master
INCLUDE += -I$(NRF5X_SDK_PATH)/components/drivers_nrf/spi_master
INCLUDE += -I$(NRF5X_SDK_PATH)/components/drivers_nrf/ppi
INCLUDE += -I$(NRF5X_SDK_PATH)/components/drivers_nrf/hal/nrf_pwm
INCLUDE += -I$(NRF5X_SDK_PATH)/components/drivers_nrf/clock
INCLUDE += -I$(NRF5X_SDK_PATH)/components/drivers_nrf/rng

TARGETSOURCES += \
$(NRF5X_SDK_PATH)/components/libraries/util/app_error.c \
$(NRF5X_SDK_PATH)/components/libraries/timer/app_timer.c \
$(NRF5X_SDK_PATH)/components/libraries/fstorage/fstorage.c \
$(NRF5X_SDK_PATH)/components/libraries/util/nrf_assert.c \
$(NRF5X_SDK_PATH)/components/libraries/uart/app_uart.c \
$(NRF5X_SDK_PATH)/components/drivers_nrf/common/nrf_drv_common.c \
$(NRF5X_SDK_PATH)/components/drivers_nrf/gpiote/nrf_drv_gpiote.c \
$(NRF5X_SDK_PATH)/components/drivers_nrf/uart/nrf_drv_uart.c \
$(NRF5X_SDK_PATH)/components/ble/common/ble_advdata.c \
$(NRF5X_SDK_PATH)/components/ble/ble_advertising/ble_advertising.c \
$(NRF5X_SDK_PATH)/components/ble/common/ble_conn_params.c \
$(NRF5X_SDK_PATH)/components/ble/ble_services/ble_nus/ble_nus.c \
$(NRF5X_SDK_PATH)/components/ble/common/ble_srv_common.c \
$(NRF5X_SDK_PATH)/components/softdevice/common/softdevice_handler/softdevice_handler.c \
$(NRF5X_SDK_PATH)/components/drivers_nrf/hal/nrf_nvmc.c \
$(NRF5X_SDK_PATH)/components/drivers_nrf/twi_master/nrf_drv_twi.c \
$(NRF5X_SDK_PATH)/components/drivers_nrf/spi_master/nrf_drv_spi.c \
$(NRF5X_SDK_PATH)/components/drivers_nrf/ppi/nrf_drv_ppi.c \
$(NRF5X_SDK_PATH)/components/drivers_nrf/hal/nrf_adc.c \
$(NRF5X_SDK_PATH)/components/drivers_nrf/clock/nrf_drv_clock.c \
$(NRF5X_SDK_PATH)/components/libraries/util/app_util_platform.c

# $(NRF5X_SDK_PATH)/components/libraries/util/nrf_log.c

ifdef USE_BOOTLOADER
ifdef BOOTLOADER
  DEFINES += -DBOOTLOADER -DNRF_DFU_SETTINGS_VERSION=1
  INCLUDE += -I$(NRF5X_SDK_PATH)/components/libraries/bootloader_dfu/ble_transport
  INCLUDE += -I$(NRF5X_SDK_PATH)/components/libraries/crc32
  INCLUDE += -I$(NRF5X_SDK_PATH)/components/libraries/bootloader
  INCLUDE += -I$(NRF5X_SDK_PATH)/components/libraries/bootloader/dfu
  INCLUDE += -I$(NRF5X_SDK_PATH)/components/libraries/bootloader/ble_dfu
  INCLUDE += -I$(NRF5X_SDK_PATH)/components/libraries/crypto
  INCLUDE += -I$(NRF5X_SDK_PATH)/components/libraries/svc
  INCLUDE += -I$(NRF5X_SDK_PATH)/components/libraries/scheduler
  INCLUDE += -I$(NRF5X_SDK_PATH)/components/libraries/ecc
  INCLUDE += -I$(NRF5X_SDK_PATH)/components/libraries/sha256
  INCLUDE += -I$(NRF5X_SDK_PATH)/external/nano-pb
  INCLUDE += -I$(NRF5X_SDK_PATH)/external/micro-ecc
  TARGETSOURCES =
  TARGETSOURCES += $(NRF5X_SDK_PATH)/components/libraries/util/app_error_weak.c
  TARGETSOURCES += $(NRF5X_SDK_PATH)/components/libraries/fifo/app_fifo.c
  TARGETSOURCES += $(NRF5X_SDK_PATH)/components/libraries/scheduler/app_scheduler.c
  TARGETSOURCES += $(NRF5X_SDK_PATH)/components/libraries/timer/app_timer.c
  TARGETSOURCES += $(NRF5X_SDK_PATH)/components/libraries/timer/app_timer_appsh.c
  TARGETSOURCES += $(NRF5X_SDK_PATH)/components/libraries/util/app_util_platform.c
  TARGETSOURCES += $(NRF5X_SDK_PATH)/components/libraries/crc32/crc32.c
  TARGETSOURCES += $(NRF5X_SDK_PATH)/components/libraries/ecc/ecc.c
  TARGETSOURCES += $(NRF5X_SDK_PATH)/components/libraries/fstorage/fstorage.c
  TARGETSOURCES += $(NRF5X_SDK_PATH)/components/libraries/hci/hci_mem_pool.c
  TARGETSOURCES += $(NRF5X_SDK_PATH)/components/libraries/util/nrf_assert.c
  TARGETSOURCES += $(NRF5X_SDK_PATH)/components/libraries/crypto/nrf_crypto.c
  TARGETSOURCES += $(NRF5X_SDK_PATH)/components/libraries/sha256/sha256.c
  TARGETSOURCES += $(NRF5X_SDK_PATH)/components/drivers_nrf/common/nrf_drv_common.c
  TARGETSOURCES += $(NRF5X_SDK_PATH)/components/drivers_nrf/rng/nrf_drv_rng.c
  TARGETSOURCES += $(NRF5X_SDK_PATH)/components/drivers_nrf/hal/nrf_nvmc.c
  TARGETSOURCES += $(NRF5X_SDK_PATH)/components/libraries/bootloader/ble_dfu/nrf_ble_dfu.c
  TARGETSOURCES += $(NRF5X_SDK_PATH)/components/ble/common/ble_advdata.c
  TARGETSOURCES += $(NRF5X_SDK_PATH)/components/ble/common/ble_conn_params.c
  TARGETSOURCES += $(NRF5X_SDK_PATH)/components/ble/common/ble_srv_common.c
  TARGETSOURCES += $(NRF5X_SDK_PATH)/components/toolchain/system_nrf52.c
  TARGETSOURCES += $(NRF5X_SDK_PATH)/components/softdevice/common/softdevice_handler/softdevice_handler.c
  TARGETSOURCES += $(NRF5X_SDK_PATH)/components/softdevice/common/softdevice_handler/softdevice_handler_appsh.c
  TARGETSOURCES += $(NRF5X_SDK_PATH)/components/libraries/bootloader/nrf_bootloader.c
  TARGETSOURCES += $(NRF5X_SDK_PATH)/components/libraries/bootloader/nrf_bootloader_app_start.c
  TARGETSOURCES += $(NRF5X_SDK_PATH)/components/libraries/bootloader/nrf_bootloader_info.c
  TARGETSOURCES += $(NRF5X_SDK_PATH)/components/libraries/bootloader/dfu/nrf_dfu.c
  TARGETSOURCES += $(NRF5X_SDK_PATH)/components/libraries/bootloader/dfu/nrf_dfu_flash.c
  TARGETSOURCES += $(NRF5X_SDK_PATH)/components/libraries/bootloader/dfu/nrf_dfu_mbr.c
  TARGETSOURCES += $(NRF5X_SDK_PATH)/components/libraries/bootloader/dfu/nrf_dfu_settings.c
  TARGETSOURCES += $(NRF5X_SDK_PATH)/components/libraries/bootloader/dfu/nrf_dfu_transport.c
  TARGETSOURCES += $(NRF5X_SDK_PATH)/components/libraries/bootloader/dfu/nrf_dfu_utils.c
  TARGETSOURCES += $(NRF5X_SDK_PATH)/external/nano-pb/pb_common.c
  TARGETSOURCES += $(NRF5X_SDK_PATH)/external/nano-pb/pb_decode.c
  TARGETSOURCES += $(NRF5X_SDK_PATH)/external/micro-ecc/uECC.c
endif
endif

# ==============================================================
include make/common/ARM.make
