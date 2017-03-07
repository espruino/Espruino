include make/common/NRF5X.make

NRF5X=1
NRF5X_SDK_PATH=$(ROOT)/targetlibs/nrf5x

# ARCHFLAGS are shared by both CFLAGS and LDFLAGS.
ARCHFLAGS = -mcpu=cortex-m0 -mthumb -mabi=aapcs -mfloat-abi=soft # Use nRF51 makefiles provided in SDK as reference.

# nRF51 specific.
INCLUDE          += -I$(NRF5X_SDK_PATH)/components/softdevice/s130/headers
INCLUDE          += -I$(NRF5X_SDK_PATH)/components/softdevice/s130/headers/nrf51
TARGETSOURCES    += $(NRF5X_SDK_PATH)/components/toolchain/system_nrf51.c
PRECOMPILED_OBJS += $(NRF5X_SDK_PATH)/components/toolchain/gcc/gcc_startup_nrf51.o

DEFINES += -DNRF51 -DSWI_DISABLE0 -DSOFTDEVICE_PRESENT -DS130 -DBLE_STACK_SUPPORT_REQD # SoftDevice included by default.
DEFINES += -DNRF_SD_BLE_API_VERSION=2
LINKER_RAM:=$(shell python scripts/get_board_info.py $(BOARD) "board.chip['ram']")

SOFTDEVICE        = $(NRF5X_SDK_PATH)/components/softdevice/s130/hex/s130_nrf51_2.0.1_softdevice.hex

ifdef USE_BOOTLOADER
NRF_BOOTLOADER    = $(BOOTLOADER_PROJ_NAME).hex
LINKER_FILE = $(NRF5X_SDK_PATH)/nrf5x_linkers/linker_nrf51_ble_espruino_$(LINKER_RAM).ld
else
LINKER_FILE = $(NRF5X_SDK_PATH)/nrf5x_linkers/linker_nrf51_ble_espruino_$(LINKER_RAM).ld
INCLUDE += -I$(NRF5X_SDK_PATH)/nrf51_config
endif
