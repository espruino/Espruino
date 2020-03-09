# If we're trying to make a full binary and there's no bootloader,
# create one automatically
ifndef DFU_UPDATE_BUILD
ifdef USE_BOOTLOADER
ifndef BOOTLOADER
ifneq ("$(MAKECMDGOALS)","boardjson")
ifneq ("$(MAKECMDGOALS)","docs")
ifeq ("$(wildcard $(NRF_BOOTLOADER))","")
$(info *************************************************************)
$(info NO BOOTLOADER $(NRF_BOOTLOADER) FOUND)
$(info *************************************************************)
$(info RUNNING MAKE CLEAN)
$(info $(shell BOARD=$(BOARD) RELEASE=1 BOOTLOADER=1 make clean))
$(info RUNNING MAKE TO CREATE BOOTLOADER)
$(info $(shell BOARD=$(BOARD) RELEASE=1 BOOTLOADER=1 make))
$(info RUNNING MAKE CLEAN)
$(info $(shell BOARD=$(BOARD) RELEASE=1 make clean))
$(info *************************************************************)
$(info BOOTLOADER BUILD COMPLETE)  
$(info *************************************************************)
endif 
endif
endif
endif #BOOTLOADER
endif #USE_BOOTLOADER
endif #DFU_UPDATE_BUILD

# Just try and get rid of the compile warnings.
CFLAGS += -Wno-sign-conversion -Wno-conversion -Wno-unused-parameter -fomit-frame-pointer #this is for device manager in nordic sdk
CFLAGS+=-Wno-expansion-to-defined # remove warnings created by Nordic's libs
DEFINES += -D$(BOARD) -D$(CHIP) -DNRF5X -DNRF5X_SDK_$(NRF5X_SDK)

ARM = 1
ARM_HAS_OWN_CMSIS = 1 # Nordic uses its own CMSIS files in its SDK, these are up-to-date.
INCLUDE += -I$(NRF5X_SDK_PATH)

# This is where the common linker for both nRF51 & nRF52 is stored.
ifdef NRF5X_SDK_11
LDFLAGS += -L$(NRF5X_SDK_PATH)/nrf5x_linkers 
endif
ifdef NRF5X_SDK_12
LDFLAGS += -L$(NRF5X_SDK_PATH)/nrf5x_linkers 
endif
ifdef NRF5X_SDK_14
LDFLAGS += -L$(NRF5X_SDK_PATH)/components/toolchain/gcc
endif
ifdef NRF5X_SDK_15
LDFLAGS += -L$(NRF5X_SDK_PATH)/modules/nrfx/mdk
endif

# These files are the Espruino HAL implementation.
INCLUDE += -I$(ROOT)/targets/nrf5x
ifdef BOOTLOADER
  BUILD_LINKER_FLAGS+=--bootloader
  PROJ_NAME=$(BOOTLOADER_PROJ_NAME)
  WRAPPERSOURCES =
  INCLUDE += -I$(ROOT)/targets/nrf5x_dfu
  DEFINES += -DSVC_INTERFACE_CALL_AS_NORMAL_FUNCTION
  SOURCES = \
    targets/nrf5x_dfu/dfu_public_key.c \
    targets/nrf5x_dfu/lcd.c \
    targets/nrf5x_dfu/main.c
ifdef NRF5X_SDK_12
  SOURCES += \
    targets/nrf5x_dfu/sdk12/dfu-cc.pb.c \
    targets/nrf5x_dfu/sdk12/dfu_req_handling.c 
endif
else
  SOURCES +=                              \
    targets/nrf5x/main.c                    \
    targets/nrf5x/jshardware.c              \
    targets/nrf5x/bluetooth.c              \
    targets/nrf5x/nrf5x_utils.c

  ifeq ($(FAMILY), NRF52)
    # Neopixel support (only NRF52)
    SOURCES    += targets/nrf5x/i2s_ws2812b_drive.c
  endif
endif

# Careful here.. All these includes and sources assume a SoftDevice. Not efficient/clean if softdevice (ble) is not enabled...
ifdef NRF5X_SDK_11
INCLUDE += -I$(NRF5X_SDK_PATH)/components/toolchain/CMSIS/Include
else
INCLUDE += -I$(NRF5X_SDK_PATH)/components/toolchain/cmsis/include
endif
INCLUDE += -I$(NRF5X_SDK_PATH)/components
INCLUDE += -I$(NRF5X_SDK_PATH)/components/toolchain/gcc
INCLUDE += -I$(NRF5X_SDK_PATH)/components/toolchain
INCLUDE += -I$(NRF5X_SDK_PATH)/components/drivers_nrf/config
INCLUDE += -I$(NRF5X_SDK_PATH)/components/libraries/util
INCLUDE += -I$(NRF5X_SDK_PATH)/components/drivers_nrf/delay
INCLUDE += -I$(NRF5X_SDK_PATH)/components/drivers_nrf/uart
INCLUDE += -I$(NRF5X_SDK_PATH)/components/libraries/fds
INCLUDE += -I$(NRF5X_SDK_PATH)/components/libraries/fds/config
INCLUDE += -I$(NRF5X_SDK_PATH)/components/ble/common
INCLUDE += -I$(NRF5X_SDK_PATH)/components/device
INCLUDE += -I$(NRF5X_SDK_PATH)/components/libraries/button
INCLUDE += -I$(NRF5X_SDK_PATH)/components/libraries/timer
INCLUDE += -I$(NRF5X_SDK_PATH)/components/libraries/fstorage
INCLUDE += -I$(NRF5X_SDK_PATH)/components/libraries/fstorage/config
INCLUDE += -I$(NRF5X_SDK_PATH)/components/libraries/queue
INCLUDE += -I$(NRF5X_SDK_PATH)/components/libraries/experimental_section_vars
INCLUDE += -I$(NRF5X_SDK_PATH)/components/drivers_nrf/gpiote
INCLUDE += -I$(NRF5X_SDK_PATH)/components/ble/ble_services/ble_nus
INCLUDE += -I$(NRF5X_SDK_PATH)/components/drivers_nrf/hal
INCLUDE += -I$(NRF5X_SDK_PATH)/components/drivers_nrf/common
INCLUDE += -I$(NRF5X_SDK_PATH)/components/ble/peer_manager
INCLUDE += -I$(NRF5X_SDK_PATH)/components/softdevice/common/softdevice_handler
INCLUDE += -I$(NRF5X_SDK_PATH)/components/drivers_nrf/twi_master
INCLUDE += -I$(NRF5X_SDK_PATH)/components/drivers_nrf/spi_master
INCLUDE += -I$(NRF5X_SDK_PATH)/components/drivers_nrf/ppi
INCLUDE += -I$(NRF5X_SDK_PATH)/components/drivers_nrf/hal/nrf_pwm
INCLUDE += -I$(NRF5X_SDK_PATH)/components/drivers_nrf/clock
INCLUDE += -I$(NRF5X_SDK_PATH)/components/drivers_nrf/rng
ifneq ($(or $(NRF5X_SDK_12),$(NRF5X_SDK_11)),)
INCLUDE += -I$(NRF5X_SDK_PATH)/components/libraries/log
INCLUDE += -I$(NRF5X_SDK_PATH)/components/libraries/log/src
else
INCLUDE += -I$(NRF5X_SDK_PATH)/components/softdevice/common
INCLUDE += -I$(NRF5X_SDK_PATH)/components/libraries/experimental_log
INCLUDE += -I$(NRF5X_SDK_PATH)/components/libraries/experimental_log/src
INCLUDE += -I$(NRF5X_SDK_PATH)/components/libraries/atomic
INCLUDE += -I$(NRF5X_SDK_PATH)/components/libraries/atomic_fifo
INCLUDE += -I$(NRF5X_SDK_PATH)/components/libraries/strerror
INCLUDE += -I$(NRF5X_SDK_PATH)/components/libraries/experimental_memobj
INCLUDE += -I$(NRF5X_SDK_PATH)/components/libraries/balloc
endif
ifdef NRF5X_SDK_15
INCLUDE += -I$(NRF5X_SDK_PATH)/modules/nrfx
INCLUDE += -I$(NRF5X_SDK_PATH)/modules/nrfx/mdk
INCLUDE += -I$(NRF5X_SDK_PATH)/modules/nrfx/hal
INCLUDE += -I$(NRF5X_SDK_PATH)/modules/nrfx/legacy
INCLUDE += -I$(NRF5X_SDK_PATH)/modules/nrfx/drivers/include
INCLUDE += -I$(NRF5X_SDK_PATH)/integration/nrfx
INCLUDE += -I$(NRF5X_SDK_PATH)/integration/nrfx/legacy
INCLUDE += -I$(NRF5X_SDK_PATH)/components/libraries/delay
INCLUDE += -I$(NRF5X_SDK_PATH)/components/ble/ble_link_ctx_manager
INCLUDE += -I$(NRF5X_SDK_PATH)/components/libraries/atomic_flags
endif


ifdef NRF5X_SDK_15
TARGETSOURCES += \
$(NRF5X_SDK_PATH)/modules/nrfx/drivers/src/nrfx_gpiote.c \
$(NRF5X_SDK_PATH)/modules/nrfx/drivers/src/nrfx_spi.c \
$(NRF5X_SDK_PATH)/modules/nrfx/drivers/src/nrfx_spim.c \
$(NRF5X_SDK_PATH)/modules/nrfx/drivers/src/nrfx_twi.c \
$(NRF5X_SDK_PATH)/modules/nrfx/drivers/src/nrfx_uart.c \
$(NRF5X_SDK_PATH)/modules/nrfx/drivers/src/nrfx_uarte.c \
$(NRF5X_SDK_PATH)/integration/nrfx/legacy/nrf_drv_ppi.c \
$(NRF5X_SDK_PATH)/integration/nrfx/legacy/nrf_drv_rng.c \
$(NRF5X_SDK_PATH)/integration/nrfx/legacy/nrf_drv_twi.c \
$(NRF5X_SDK_PATH)/integration/nrfx/legacy/nrf_drv_uart.c \
$(NRF5X_SDK_PATH)/integration/nrfx/legacy/nrf_drv_spi.c \
$(NRF5X_SDK_PATH)/components/libraries/atomic/nrf_atomic.c \
$(NRF5X_SDK_PATH)/components/libraries/atomic_flags/nrf_atflags.c \
$(NRF5X_SDK_PATH)/components/ble/ble_link_ctx_manager/ble_link_ctx_manager.c \
$(NRF5X_SDK_PATH)/components/libraries/util/app_error_handler_gcc.c
else
TARGETSOURCES += \
$(NRF5X_SDK_PATH)/components/drivers_nrf/common/nrf_drv_common.c \
$(NRF5X_SDK_PATH)/components/drivers_nrf/gpiote/nrf_drv_gpiote.c \
$(NRF5X_SDK_PATH)/components/drivers_nrf/uart/nrf_drv_uart.c \
$(NRF5X_SDK_PATH)/components/drivers_nrf/hal/nrf_nvmc.c \
$(NRF5X_SDK_PATH)/components/drivers_nrf/twi_master/nrf_drv_twi.c \
$(NRF5X_SDK_PATH)/components/drivers_nrf/spi_master/nrf_drv_spi.c \
$(NRF5X_SDK_PATH)/components/drivers_nrf/ppi/nrf_drv_ppi.c \
$(NRF5X_SDK_PATH)/components/drivers_nrf/clock/nrf_drv_clock.c
endif

TARGETSOURCES += \
$(NRF5X_SDK_PATH)/components/ble/common/ble_advdata.c \
$(NRF5X_SDK_PATH)/components/ble/common/ble_conn_params.c \
$(NRF5X_SDK_PATH)/components/ble/common/ble_srv_common.c \
$(NRF5X_SDK_PATH)/components/ble/common/ble_conn_state.c \
$(NRF5X_SDK_PATH)/components/ble/ble_services/ble_nus/ble_nus.c \
$(NRF5X_SDK_PATH)/components/ble/peer_manager/peer_manager.c \
$(NRF5X_SDK_PATH)/components/ble/peer_manager/peer_id.c \
$(NRF5X_SDK_PATH)/components/ble/peer_manager/peer_database.c \
$(NRF5X_SDK_PATH)/components/ble/peer_manager/peer_data_storage.c \
$(NRF5X_SDK_PATH)/components/ble/peer_manager/pm_buffer.c \
$(NRF5X_SDK_PATH)/components/ble/peer_manager/pm_mutex.c \
$(NRF5X_SDK_PATH)/components/ble/peer_manager/id_manager.c \
$(NRF5X_SDK_PATH)/components/ble/peer_manager/security_manager.c \
$(NRF5X_SDK_PATH)/components/ble/peer_manager/security_dispatcher.c \
$(NRF5X_SDK_PATH)/components/ble/peer_manager/gatt_cache_manager.c \
$(NRF5X_SDK_PATH)/components/ble/peer_manager/gatts_cache_manager.c \
$(NRF5X_SDK_PATH)/components/libraries/timer/app_timer.c \
$(NRF5X_SDK_PATH)/components/libraries/fds/fds.c \
$(NRF5X_SDK_PATH)/components/libraries/queue/nrf_queue.c \
$(NRF5X_SDK_PATH)/components/libraries/util/app_util_platform.c \
$(NRF5X_SDK_PATH)/components/libraries/util/sdk_mapped_flags.c \
$(NRF5X_SDK_PATH)/components/libraries/util/app_error.c \
$(NRF5X_SDK_PATH)/components/libraries/util/nrf_assert.c 

ifdef NRF5X_SDK_11
TARGETSOURCES += \
$(NRF5X_SDK_PATH)/components/drivers_nrf/delay/nrf_delay.c \
$(NRF5X_SDK_PATH)/components/drivers_nrf/hal/nrf_saadc.c
endif

ifneq ($(or $(NRF5X_SDK_12),$(NRF5X_SDK_11)),)
TARGETSOURCES += \
$(NRF5X_SDK_PATH)/components/softdevice/common/softdevice_handler/softdevice_handler.c \
$(NRF5X_SDK_PATH)/components/libraries/fstorage/fstorage.c \
$(NRF5X_SDK_PATH)/components/drivers_nrf/hal/nrf_adc.c 
else
TARGETSOURCES += \
$(NRF5X_SDK_PATH)/components/softdevice/common/nrf_sdh.c \
$(NRF5X_SDK_PATH)/components/softdevice/common/nrf_sdh_ble.c \
$(NRF5X_SDK_PATH)/components/softdevice/common/nrf_sdh_soc.c \
$(NRF5X_SDK_PATH)/components/libraries/experimental_section_vars/nrf_section_iter.c \
$(NRF5X_SDK_PATH)/components/libraries/fstorage/nrf_fstorage.c \
$(NRF5X_SDK_PATH)/components/libraries/fstorage/nrf_fstorage_sd.c \
$(NRF5X_SDK_PATH)/components/libraries/queue/nrf_queue.c \
$(NRF5X_SDK_PATH)/components/libraries/atomic_fifo/nrf_atfifo.c \
$(NRF5X_SDK_PATH)/components/libraries/strerror/nrf_strerror.c \
$(NRF5X_SDK_PATH)/components/libraries/experimental_memobj/nrf_memobj.c \
$(NRF5X_SDK_PATH)/components/libraries/balloc/nrf_balloc.c
endif



# $(NRF5X_SDK_PATH)/components/libraries/util/nrf_log.c

ifdef USE_BOOTLOADER
ifdef BOOTLOADER
  DEFINES += -DBOOTLOADER -DNRF_DFU_SETTINGS_VERSION=1
  TARGETSOURCES =
ifdef NRF5X_SDK_12
  DEFINES += -DuECC_ENABLE_VLI_API -DuECC_VLI_NATIVE_LITTLE_ENDIAN=1 -DuECC_SQUARE_FUNC=1 -DuECC_SUPPORTS_secp256r1=1 -DuECC_SUPPORT_COMPRESSED_POINT=0 -DuECC_OPTIMIZATION_LEVEL=3

  INCLUDE += -I$(ROOT)/targets/nrf5x_dfu/sdk12
  INCLUDE += -I$(NRF5X_SDK_PATH)/components/libraries/bootloader_dfu/ble_transport
  INCLUDE += -I$(NRF5X_SDK_PATH)/components/libraries/crc32
  INCLUDE += -I$(NRF5X_SDK_PATH)/components/libraries/crypto
  INCLUDE += -I$(NRF5X_SDK_PATH)/components/libraries/bootloader
  INCLUDE += -I$(NRF5X_SDK_PATH)/components/libraries/bootloader/dfu
  INCLUDE += -I$(NRF5X_SDK_PATH)/components/libraries/bootloader/ble_dfu
  INCLUDE += -I$(NRF5X_SDK_PATH)/components/libraries/svc
  INCLUDE += -I$(NRF5X_SDK_PATH)/components/libraries/scheduler
  INCLUDE += -I$(NRF5X_SDK_PATH)/external/nano-pb
  INCLUDE += -I$(NRF5X_SDK_PATH)/external/micro-ecc
  TARGETSOURCES += $(NRF5X_SDK_PATH)/components/libraries/util/app_error_weak.c
  TARGETSOURCES += $(NRF5X_SDK_PATH)/components/libraries/fifo/app_fifo.c
  TARGETSOURCES += $(NRF5X_SDK_PATH)/components/libraries/scheduler/app_scheduler.c
  TARGETSOURCES += $(NRF5X_SDK_PATH)/components/libraries/timer/app_timer.c
  TARGETSOURCES += $(NRF5X_SDK_PATH)/components/libraries/util/app_util_platform.c
  TARGETSOURCES += $(NRF5X_SDK_PATH)/components/libraries/crc32/crc32.c
  TARGETSOURCES += $(NRF5X_SDK_PATH)/components/libraries/ecc/ecc.c
  TARGETSOURCES += $(NRF5X_SDK_PATH)/components/libraries/hci/hci_mem_pool.c
  TARGETSOURCES += $(NRF5X_SDK_PATH)/components/libraries/util/nrf_assert.c
  TARGETSOURCES += $(NRF5X_SDK_PATH)/components/libraries/queue/nrf_queue.c 
  TARGETSOURCES += $(NRF5X_SDK_PATH)/components/ble/common/ble_advdata.c
  TARGETSOURCES += $(NRF5X_SDK_PATH)/components/ble/common/ble_conn_params.c
  TARGETSOURCES += $(NRF5X_SDK_PATH)/components/ble/common/ble_srv_common.c
  TARGETSOURCES += $(NRF5X_SDK_PATH)/components/libraries/timer/app_timer_appsh.c
  TARGETSOURCES += $(NRF5X_SDK_PATH)/components/libraries/fstorage/fstorage.c
  TARGETSOURCES += $(NRF5X_SDK_PATH)/components/drivers_nrf/common/nrf_drv_common.c
  TARGETSOURCES += $(NRF5X_SDK_PATH)/components/drivers_nrf/rng/nrf_drv_rng.c
  TARGETSOURCES += $(NRF5X_SDK_PATH)/components/drivers_nrf/hal/nrf_nvmc.c
  TARGETSOURCES += $(NRF5X_SDK_PATH)/components/toolchain/system_nrf52.c
  TARGETSOURCES += $(NRF5X_SDK_PATH)/components/softdevice/common/softdevice_handler/softdevice_handler.c
  TARGETSOURCES += $(NRF5X_SDK_PATH)/components/softdevice/common/softdevice_handler/softdevice_handler_appsh.c
  TARGETSOURCES += $(NRF5X_SDK_PATH)/components/libraries/bootloader/ble_dfu/nrf_ble_dfu.c
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
ifdef NRF_BL_DFU_INSECURE
  DEFINES += -DNRF_BL_DFU_INSECURE
else # NRF_BL_DFU_INSECURE
  INCLUDE += -I$(NRF5X_SDK_PATH)/components/libraries/ecc
  INCLUDE += -I$(NRF5X_SDK_PATH)/components/libraries/sha256
  TARGETSOURCES += $(NRF5X_SDK_PATH)/components/libraries/crypto/nrf_crypto.c
  TARGETSOURCES += $(NRF5X_SDK_PATH)/external/micro-ecc/uECC.c
  TARGETSOURCES += $(NRF5X_SDK_PATH)/components/libraries/sha256/sha256.c
endif # NRF_BL_DFU_INSECURE
else # NRF5X_SDK_12
  DEFINES += -DAPP_TIMER_V2
  DEFINES += -DAPP_TIMER_V2_RTC1_ENABLED
  DEFINES += -DNRF_DFU_SETTINGS_VERSION=1
  DEFINES += -DNRF_DFU_SVCI_ENABLED
  INCLUDE = \
    -I$(ROOT)/gen \
    -I$(ROOT)/src \
    -I$(ROOT)/targets/nrf5x_dfu \
    -I$(ROOT)/targets/nrf5x_dfu/sdk15 \
    -I$(NRF5X_SDK_PATH)/external/micro-ecc \
    -I$(NRF5X_SDK_PATH)/components/libraries/sha256 \
    -I$(NRF5X_SDK_PATH)/components/libraries/crypto/backend/micro_ecc \
    -I$(NRF5X_SDK_PATH)/modules/nrfx/hal \
    -I$(NRF5X_SDK_PATH)/components/softdevice/s140/headers/nrf52 \
    -I$(NRF5X_SDK_PATH)/components/libraries/crc32 \
    -I$(NRF5X_SDK_PATH)/components/libraries/experimental_section_vars \
    -I$(NRF5X_SDK_PATH)/components/libraries/mem_manager \
    -I$(NRF5X_SDK_PATH)/components/libraries/crypto/backend/nrf_sw \
    -I$(NRF5X_SDK_PATH)/components/libraries/util \
    -I$(NRF5X_SDK_PATH)/modules/nrfx \
    -I$(NRF5X_SDK_PATH)/components/libraries/timer/experimental \
    -I$(NRF5X_SDK_PATH)/components/libraries/timer \
    -I$(NRF5X_SDK_PATH)/components/libraries/crypto/backend/oberon \
    -I$(NRF5X_SDK_PATH)/components/libraries/crypto/backend/cifra \
    -I$(NRF5X_SDK_PATH)/components/libraries/atomic \
    -I$(NRF5X_SDK_PATH)/integration/nrfx \
    -I$(NRF5X_SDK_PATH)/components/libraries/crypto/backend/cc310_bl \
    -I$(NRF5X_SDK_PATH)/external/nrf_cc310/include \
    -I$(NRF5X_SDK_PATH)/components/libraries/bootloader/dfu \
    -I$(NRF5X_SDK_PATH)/components/ble/common \
    -I$(NRF5X_SDK_PATH)/components/libraries/delay \
    -I$(NRF5X_SDK_PATH)/components/libraries/svc \
    -I$(NRF5X_SDK_PATH)/components/libraries/stack_info \
    -I$(NRF5X_SDK_PATH)/components/libraries/crypto/backend/nrf_hw \
    -I$(NRF5X_SDK_PATH)/components/libraries/strerror \
    -I$(NRF5X_SDK_PATH)/components/libraries/crypto/backend/mbedtls \
    -I$(NRF5X_SDK_PATH)/components/libraries/crypto/backend/cc310 \
    -I$(NRF5X_SDK_PATH)/components/libraries/bootloader \
    -I$(NRF5X_SDK_PATH)/components/softdevice/s140/headers \
    -I$(NRF5X_SDK_PATH)/components/libraries/crypto \
    -I$(NRF5X_SDK_PATH)/components/libraries/scheduler \
    -I$(NRF5X_SDK_PATH)/external/nrf_cc310_bl/include \
    -I$(NRF5X_SDK_PATH)/components/libraries/experimental_log/src \
    -I$(NRF5X_SDK_PATH)/components/toolchain/cmsis/include \
    -I$(NRF5X_SDK_PATH)/components/libraries/balloc \
    -I$(NRF5X_SDK_PATH)/components/libraries/atomic_fifo \
    -I$(NRF5X_SDK_PATH)/components/libraries/sortlist \
    -I$(NRF5X_SDK_PATH)/components/libraries/fstorage \
    -I$(NRF5X_SDK_PATH)/modules/nrfx/mdk \
    -I$(NRF5X_SDK_PATH)/components/libraries/mutex \
    -I$(NRF5X_SDK_PATH)/components/libraries/bootloader/ble_dfu \
    -I$(NRF5X_SDK_PATH)/components/softdevice/common \
    -I$(NRF5X_SDK_PATH)/external/nano-pb \
    -I$(NRF5X_SDK_PATH)/components/libraries/queue \
    -I$(NRF5X_SDK_PATH)/components/libraries/experimental_log \
    -I$(NRF5X_SDK_PATH)/components/libraries/experimental_memobj
  TARGETSOURCES += \
  $(NRF5X_SDK_PATH)/external/micro-ecc/uECC.c \
  $(NRF5X_SDK_PATH)/components/libraries/sha256/sha256.c \
  $(NRF5X_SDK_PATH)/components/libraries/crypto/backend/nrf_sw/nrf_sw_backend_hash.c \
  $(NRF5X_SDK_PATH)/components/libraries/util/app_error_weak.c \
  $(NRF5X_SDK_PATH)/components/libraries/scheduler/app_scheduler.c \
  $(NRF5X_SDK_PATH)/components/libraries/timer/experimental/app_timer2.c \
  $(NRF5X_SDK_PATH)/components/libraries/util/app_util_platform.c \
  $(NRF5X_SDK_PATH)/components/libraries/crc32/crc32.c \
  $(NRF5X_SDK_PATH)/components/libraries/timer/experimental/drv_rtc.c \
  $(NRF5X_SDK_PATH)/components/libraries/mem_manager/mem_manager.c \
  $(NRF5X_SDK_PATH)/components/libraries/util/nrf_assert.c \
  $(NRF5X_SDK_PATH)/components/libraries/atomic_fifo/nrf_atfifo.c \
  $(NRF5X_SDK_PATH)/components/libraries/atomic/nrf_atomic.c \
  $(NRF5X_SDK_PATH)/components/libraries/balloc/nrf_balloc.c \
  $(NRF5X_SDK_PATH)/components/libraries/fstorage/nrf_fstorage.c \
  $(NRF5X_SDK_PATH)/components/libraries/fstorage/nrf_fstorage_nvmc.c \
  $(NRF5X_SDK_PATH)/components/libraries/fstorage/nrf_fstorage_sd.c \
  $(NRF5X_SDK_PATH)/components/libraries/queue/nrf_queue.c \
  $(NRF5X_SDK_PATH)/components/libraries/experimental_section_vars/nrf_section_iter.c \
  $(NRF5X_SDK_PATH)/components/libraries/sortlist/nrf_sortlist.c \
  $(NRF5X_SDK_PATH)/components/libraries/strerror/nrf_strerror.c \
  $(NRF5X_SDK_PATH)/modules/nrfx/mdk/system_nrf52840.c \
  $(NRF5X_SDK_PATH)/components/libraries/crypto/backend/cc310_bl/cc310_bl_backend_ecc.c \
  $(NRF5X_SDK_PATH)/components/libraries/crypto/backend/cc310_bl/cc310_bl_backend_ecdsa.c \
  $(NRF5X_SDK_PATH)/components/libraries/crypto/backend/cc310_bl/cc310_bl_backend_hash.c \
  $(NRF5X_SDK_PATH)/components/libraries/crypto/backend/cc310_bl/cc310_bl_backend_init.c \
  $(NRF5X_SDK_PATH)/components/libraries/crypto/backend/cc310_bl/cc310_bl_backend_shared.c \
  $(NRF5X_SDK_PATH)/components/libraries/crypto/backend/micro_ecc/micro_ecc_backend_ecc.c \
  $(NRF5X_SDK_PATH)/components/libraries/crypto/backend/micro_ecc/micro_ecc_backend_ecdsa.c \
  $(NRF5X_SDK_PATH)/modules/nrfx/hal/nrf_nvmc.c \
  $(NRF5X_SDK_PATH)/components/softdevice/common/nrf_sdh.c \
  $(NRF5X_SDK_PATH)/components/softdevice/common/nrf_sdh_ble.c \
  $(NRF5X_SDK_PATH)/components/softdevice/common/nrf_sdh_soc.c \
  $(NRF5X_SDK_PATH)/components/libraries/crypto/nrf_crypto_ecc.c \
  $(NRF5X_SDK_PATH)/components/libraries/crypto/nrf_crypto_ecdsa.c \
  $(NRF5X_SDK_PATH)/components/libraries/crypto/nrf_crypto_hash.c \
  $(NRF5X_SDK_PATH)/components/libraries/crypto/nrf_crypto_init.c \
  $(NRF5X_SDK_PATH)/components/libraries/crypto/nrf_crypto_shared.c \
  $(NRF5X_SDK_PATH)/components/libraries/svc/nrf_svc_handler.c \
  $(NRF5X_SDK_PATH)/components/ble/common/ble_srv_common.c \
  $(NRF5X_SDK_PATH)/external/nano-pb/pb_common.c \
  $(NRF5X_SDK_PATH)/external/nano-pb/pb_decode.c \
  $(NRF5X_SDK_PATH)/components/libraries/bootloader/dfu/dfu-cc.pb.c \
  $(NRF5X_SDK_PATH)/components/libraries/bootloader/dfu/nrf_dfu.c \
  $(NRF5X_SDK_PATH)/components/libraries/bootloader/ble_dfu/nrf_dfu_ble.c \
  $(NRF5X_SDK_PATH)/components/libraries/bootloader/dfu/nrf_dfu_svci.c \
  $(NRF5X_SDK_PATH)/components/libraries/bootloader/dfu/nrf_dfu_svci_handler.c \
  $(NRF5X_SDK_PATH)/components/libraries/bootloader/dfu/nrf_dfu_flash.c \
  $(NRF5X_SDK_PATH)/components/libraries/bootloader/dfu/nrf_dfu_handling_error.c \
  $(NRF5X_SDK_PATH)/components/libraries/bootloader/dfu/nrf_dfu_mbr.c \
  $(NRF5X_SDK_PATH)/components/libraries/bootloader/dfu/nrf_dfu_req_handler.c \
  $(NRF5X_SDK_PATH)/components/libraries/bootloader/dfu/nrf_dfu_settings.c \
  $(NRF5X_SDK_PATH)/components/libraries/bootloader/dfu/nrf_dfu_settings_svci.c \
  $(NRF5X_SDK_PATH)/components/libraries/bootloader/dfu/nrf_dfu_transport.c \
  $(NRF5X_SDK_PATH)/components/libraries/bootloader/dfu/nrf_dfu_utils.c \
  $(NRF5X_SDK_PATH)/components/libraries/bootloader/dfu/nrf_dfu_validation.c \
  $(NRF5X_SDK_PATH)/components/libraries/bootloader/dfu/nrf_dfu_ver_validation.c \
  $(NRF5X_SDK_PATH)/components/libraries/bootloader/nrf_bootloader.c \
  $(NRF5X_SDK_PATH)/components/libraries/bootloader/nrf_bootloader_app_start.c \
  $(NRF5X_SDK_PATH)/components/libraries/bootloader/nrf_bootloader_app_start_final.c \
  $(NRF5X_SDK_PATH)/components/libraries/bootloader/nrf_bootloader_dfu_timers.c \
  $(NRF5X_SDK_PATH)/components/libraries/bootloader/nrf_bootloader_fw_activation.c \
  $(NRF5X_SDK_PATH)/components/libraries/bootloader/nrf_bootloader_info.c \
  $(NRF5X_SDK_PATH)/components/libraries/bootloader/nrf_bootloader_wdt.c 
endif
endif
endif

# ==============================================================
include make/common/ARM.make

$(PROJ_NAME).app_hex: $(PROJ_NAME).elf
	python scripts/check_elf_size.py $(BOARD) $(PROJ_NAME).elf
	@echo $(call $(quiet_)obj_to_bin,ihex,hex)
	@$(call obj_to_bin,ihex,hex)
	@mv $(PROJ_NAME).hex $(PROJ_NAME).app_hex

$(PROJ_NAME).hex: $(PROJ_NAME).app_hex
 ifdef USE_BOOTLOADER
  ifdef BOOTLOADER
	@echo Bootloader - leaving hex file as-is
	@mv $(PROJ_NAME).app_hex $(PROJ_NAME).hex
	# for testing:
	#python scripts/hexmerge.py --overlap=replace $(SOFTDEVICE) $(PROJ_NAME).app_hex -o $(PROJ_NAME).hex
  else
	@echo Merging SoftDevice and Bootloader
	@# build a DFU settings file we can merge in...
	nrfutil settings generate --family NRF52 --application $(PROJ_NAME).app_hex --app-boot-validation VALIDATE_GENERATED_CRC --application-version 0xff --bootloader-version 0xff --bl-settings-version 2 dfu_settings.hex
	@echo FIXME - had to set --overlap=replace
	python scripts/hexmerge.py --overlap=replace $(SOFTDEVICE) $(NRF_BOOTLOADER) $(PROJ_NAME).app_hex dfu_settings.hex -o $(PROJ_NAME).hex
  endif
 else
	@echo Merging SoftDevice
	python scripts/hexmerge.py $(SOFTDEVICE) $(PROJ_NAME).app_hex -o $(PROJ_NAME).hex
 endif # USE_BOOTLOADER


$(PROJ_NAME).zip: $(PROJ_NAME).app_hex
ifdef NRF5X_SDK_11 # SDK11 requires non-secure DFU that the adafruit tools support
	@echo Creating DFU ZIP
	# adafruit-nrfutil dfu genpkg --help
	@cp $(PROJ_NAME).app_hex $(PROJ_NAME)_app.hex
	adafruit-nrfutil dfu genpkg --application $(PROJ_NAME)_app.hex $(DFU_SETTINGS) $(PROJ_NAME).zip 
	@rm $(PROJ_NAME)_app.hex
else
	@echo Creating DFU ZIP
	# nrfutil  pkg generate --help
	@cp $(PROJ_NAME).app_hex $(PROJ_NAME)_app.hex
ifdef BOOTLOADER
	nrfutil pkg generate $(PROJ_NAME).zip --bootloader $(PROJ_NAME)_app.hex --bootloader-version 0xff --hw-version 52 --sd-req 0x8C --key-file $(DFU_PRIVATE_KEY)
else
	nrfutil pkg generate $(PROJ_NAME).zip --application $(PROJ_NAME)_app.hex $(DFU_SETTINGS) --key-file $(DFU_PRIVATE_KEY)  
endif
	@rm $(PROJ_NAME)_app.hex
endif

flash: all
ifeq ($(BOARD),MICROBIT)
	if [ -d "/media/$(USER)/MICROBIT" ]; then cp $(PROJ_NAME).hex /media/$(USER)/MICROBIT;sync; fi
	if [ -d "/media/MICROBIT" ]; then cp $(PROJ_NAME).hex /media/MICROBIT;sync; fi
else
        # nrfjprog --family NRF52 --clockspeed 50000 --recover;  will recover a chip if write-protect was set on it
	if type nrfjprog 2>/dev/null; then nrfjprog --family $(FAMILY) --clockspeed 50000 --program $(PROJ_NAME).hex --chiperase --reset; \
	elif [ -d "/media/$(USER)/JLINK" ]; then cp $(PROJ_NAME).hex /media/$(USER)/JLINK;sync; \
	elif [ -d "/media/JLINK" ]; then cp $(PROJ_NAME).hex /media/JLINK;sync; fi
endif

ifdef DFU_UPDATE_BUILD_WITH_HEX
proj: $(PROJ_NAME).hex $(PROJ_NAME).zip
else
ifdef DFU_UPDATE_BUILD
proj: $(PROJ_NAME).zip
else
proj: $(PROJ_NAME).hex
endif
endif

