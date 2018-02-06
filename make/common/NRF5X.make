# If we're trying to make a full binary and there's no bootloader,
# create one automatically
ifndef DFU_UPDATE_BUILD
ifdef USE_BOOTLOADER
ifndef BOOTLOADER
ifeq ("$(wildcard $(NRF_BOOTLOADER))","")
$(info *************************************************************)
$(info NO BOOTLOADER $(NRF_BOOTLOADER) FOUND)
$(info *************************************************************)
$(info RUNNING MAKE CLEAN)
$(info $(shell BOARD=$(BOARD) RELEASE=1 BOOTLOADER=1 make clean))
$(info RUNNING MAKE TO CREATE BOOTLOADER)
$(info $(shell BOARD=$(BOARD) RELEASE=1 BOOTLOADER=1 make))
$(info *************************************************************)
$(info BOOTLOADER BUILD COMPLETE)  
$(info *************************************************************)
endif 
endif #BOOTLOADER
endif #USE_BOOTLOADER
endif #DFU_UPDATE_BUILD

# Just try and get rid of the compile warnings.
CFLAGS += -Wno-sign-conversion -Wno-conversion -Wno-unused-parameter -fomit-frame-pointer #this is for device manager in nordic sdk
DEFINES += -D$(BOARD) -D$(CHIP) -DNRF5X -DNRF5X_SDK_$(NRF5X_SDK)

ARM = 1
ARM_HAS_OWN_CMSIS = 1 # Nordic uses its own CMSIS files in its SDK, these are up-to-date.
INCLUDE += -I$(NRF5X_SDK_PATH)

ifdef NRF5X_SDK_12
LDFLAGS += -L$(NRF5X_SDK_PATH)/nrf5x_linkers # This is where the common linker for both nRF51 & nRF52 is stored.
else
LDFLAGS += -L$(NRF5X_SDK_PATH)/components/toolchain/gcc # This is where the common linker for both nRF51 & nRF52 is stored.
endif

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
    targets/nrf5x_dfu/lcd.c \
    targets/nrf5x_dfu/main.c
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
INCLUDE += -I$(NRF5X_SDK_PATH)/components
INCLUDE += -I$(NRF5X_SDK_PATH)/components/toolchain/cmsis/include
INCLUDE += -I$(NRF5X_SDK_PATH)/components/toolchain/gcc
INCLUDE += -I$(NRF5X_SDK_PATH)/components/toolchain
INCLUDE += -I$(NRF5X_SDK_PATH)/components/drivers_nrf/config
INCLUDE += -I$(NRF5X_SDK_PATH)/components/libraries/util
INCLUDE += -I$(NRF5X_SDK_PATH)/components/drivers_nrf/delay
INCLUDE += -I$(NRF5X_SDK_PATH)/components/drivers_nrf/uart
INCLUDE += -I$(NRF5X_SDK_PATH)/components/libraries/fds
INCLUDE += -I$(NRF5X_SDK_PATH)/components/ble/common
INCLUDE += -I$(NRF5X_SDK_PATH)/components/device
INCLUDE += -I$(NRF5X_SDK_PATH)/components/libraries/button
INCLUDE += -I$(NRF5X_SDK_PATH)/components/libraries/timer
INCLUDE += -I$(NRF5X_SDK_PATH)/components/libraries/fstorage
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
ifdef NRF5X_SDK_12
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


TARGETSOURCES += \
$(NRF5X_SDK_PATH)/components/drivers_nrf/common/nrf_drv_common.c \
$(NRF5X_SDK_PATH)/components/drivers_nrf/gpiote/nrf_drv_gpiote.c \
$(NRF5X_SDK_PATH)/components/drivers_nrf/uart/nrf_drv_uart.c \
$(NRF5X_SDK_PATH)/components/drivers_nrf/hal/nrf_nvmc.c \
$(NRF5X_SDK_PATH)/components/drivers_nrf/twi_master/nrf_drv_twi.c \
$(NRF5X_SDK_PATH)/components/drivers_nrf/spi_master/nrf_drv_spi.c \
$(NRF5X_SDK_PATH)/components/drivers_nrf/ppi/nrf_drv_ppi.c \
$(NRF5X_SDK_PATH)/components/drivers_nrf/clock/nrf_drv_clock.c \
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


ifdef NRF5X_SDK_12
TARGETSOURCES += \
$(NRF5X_SDK_PATH)/components/softdevice/common/softdevice_handler/softdevice_handler.c \
$(NRF5X_SDK_PATH)/components/libraries/fstorage/fstorage.c \
$(NRF5X_SDK_PATH)/components/drivers_nrf/hal/nrf_adc.c 
else
TARGETSOURCES += \
$(NRF5X_SDK_PATH)/components/softdevice/common/nrf_sdh.c \
$(NRF5X_SDK_PATH)/components/softdevice/common/nrf_sdh_ble.c \
$(NRF5X_SDK_PATH)/components/drivers_nrf/hal/nrf_saadc.c \
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
  TARGETSOURCES += $(NRF5X_SDK_PATH)/components/libraries/queue/nrf_queue.c 
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

$(PROJ_NAME).app_hex: $(PROJ_NAME).elf
	@echo $(call $(quiet_)obj_to_bin,ihex,hex)
	@$(call obj_to_bin,ihex,hex)
	@mv $(PROJ_NAME).hex $(PROJ_NAME).app_hex

$(PROJ_NAME).hex: $(PROJ_NAME).app_hex
 ifdef USE_BOOTLOADER
  ifdef BOOTLOADER
	@echo Bootloader - leaving hex file as-is
	@mv $(PROJ_NAME).app_hex $(PROJ_NAME).hex
  else
	@echo Merging SoftDevice and Bootloader
	# We can build a DFU settings file we can merge in...
	# nrfutil settings generate --family NRF52 --application $(PROJ_NAME).app_hex --application-version 0xff --bootloader-version 0xff --bl-settings-version 1 dfu_settings.hex
	@echo FIXME - had to set --overlap=replace
	python scripts/hexmerge.py --overlap=replace $(SOFTDEVICE) $(NRF_BOOTLOADER) $(PROJ_NAME).app_hex -o $(PROJ_NAME).hex
  endif
 else
	@echo Merging SoftDevice
	python scripts/hexmerge.py $(SOFTDEVICE) $(PROJ_NAME).app_hex -o $(PROJ_NAME).hex
 endif # USE_BOOTLOADER


$(PROJ_NAME).zip: $(PROJ_NAME).app_hex
	@echo Not merging softdevice or bootloader, creating DFU ZIP
	# nrfutil  pkg generate --help
	@cp $(PROJ_NAME).app_hex $(PROJ_NAME)_app.hex
	nrfutil pkg generate $(PROJ_NAME).zip --application $(PROJ_NAME)_app.hex $(DFU_SETTINGS) --key-file $(DFU_PRIVATE_KEY)
	 @rm $(PROJ_NAME)_app.hex

flash: all
ifeq ($(BOARD),MICROBIT)
	if [ -d "/media/$(USER)/MICROBIT" ]; then cp $(PROJ_NAME).hex /media/$(USER)/MICROBIT;sync; fi
	if [ -d "/media/MICROBIT" ]; then cp $(PROJ_NAME).hex /media/MICROBIT;sync; fi
else
	if type nrfjprog 2>/dev/null; then nrfjprog --family $(FAMILY) --clockspeed 50000 --program $(PROJ_NAME).hex --chiperase --reset; \
	elif [ -d "/media/$(USER)/JLINK" ]; then cp $(PROJ_NAME).hex /media/$(USER)/JLINK;sync; \
	elif [ -d "/media/JLINK" ]; then cp $(PROJ_NAME).hex /media/JLINK;sync; fi
endif

ifdef DFU_UPDATE_BUILD
proj: $(PROJ_NAME).zip
else
proj: $(PROJ_NAME).hex
endif

