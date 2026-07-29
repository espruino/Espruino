# Shared build for the Raspberry Pi RP2xxx families.
#
# This is a delegated build: we generate a CMakeLists.txt from Espruino's SOURCES
# and INCLUDE, then hand compiling, linking, crt0/boot2 and the linker script to
# the Pico SDK. Families set RP2_FAMILY before this is included; the board's
# build.makefile supplies PICO_BOARD (the Pico SDK board header to use).

ifndef RP2_FAMILY
$(error RP2_FAMILY is not set)
endif
ifndef PICO_BOARD
$(error PICO_BOARD is not set - add "PICO_BOARD?=<sdk board>" to the board's build.makefile)
endif

RP2_CMAKE_DIR := $(BINDIR)/$(RP2_FAMILY)
RP2_CMAKE_BUILD_DIR := $(RP2_CMAKE_DIR)/build
RP2_CMAKEFILE := $(RP2_CMAKE_DIR)/CMakeLists.txt
RP2_BUILD_STAMP := $(RP2_CMAKE_BUILD_DIR)/.build_stamp
RP2_TARGET := espruino_$(RP2_FAMILY)

RP2_SDK_LIBS ?= pico_stdlib pico_unique_id pico_rand hardware_adc hardware_i2c \
hardware_pwm hardware_spi pico_flash tinyusb_device tinyusb_board tinyusb_additions

# 'gen' has a relative path - get rid of it and add it manually
INCLUDE_WITHOUT_GEN = $(subst -Igen,,$(INCLUDE)) -I$(ROOT)/gen

ifneq ("$(wildcard $(ROOT)/picotool-install/picotool/picotool)","")
picotool_DIR ?= $(ROOT)/picotool-install/picotool
endif

# printf (not echo) for the list expansions - echo only expands \n under some shells
$(RP2_CMAKEFILE): FORCE
	@mkdir -p $(RP2_CMAKE_DIR)
	@echo "cmake_minimum_required(VERSION 3.13)" > $(RP2_CMAKEFILE)
	@echo "set(CMAKE_C_STANDARD 11)" >> $(RP2_CMAKEFILE)
	@echo "set(CMAKE_CXX_STANDARD 17)" >> $(RP2_CMAKEFILE)
	@echo "if(NOT DEFINED ENV{PICO_SDK_PATH})" >> $(RP2_CMAKEFILE)
	@echo "  message(FATAL_ERROR \"PICO_SDK_PATH is not set\")" >> $(RP2_CMAKEFILE)
	@echo "endif()" >> $(RP2_CMAKEFILE)
	@echo "include(\$$ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)" >> $(RP2_CMAKEFILE)
	@echo "project($(RP2_TARGET) C CXX ASM)" >> $(RP2_CMAKEFILE)
	@echo "pico_sdk_init()" >> $(RP2_CMAKEFILE)
	@echo "add_executable($(RP2_TARGET)" >> $(RP2_CMAKEFILE)
	@printf '  "%s"\n' $(patsubst %,$(ROOT)/%,$(SOURCES)) >> $(RP2_CMAKEFILE)
	@echo ")" >> $(RP2_CMAKEFILE)
	@echo "target_include_directories($(RP2_TARGET) PRIVATE" >> $(RP2_CMAKEFILE)
	@printf '  "%s"\n' $(patsubst -I%,%,$(INCLUDE_WITHOUT_GEN)) >> $(RP2_CMAKEFILE)
	@echo ")" >> $(RP2_CMAKEFILE)
	@echo "target_compile_options($(RP2_TARGET) PRIVATE $(DEFINES))" >> $(RP2_CMAKEFILE)
	@echo "target_compile_definitions($(RP2_TARGET) PRIVATE PICO_FLASH_ASSUME_CORE1_SAFE=1)" >> $(RP2_CMAKEFILE)
	@echo "target_compile_options($(RP2_TARGET) PRIVATE -Wno-format -Wno-unused-function)" >> $(RP2_CMAKEFILE)
	@echo "target_link_libraries($(RP2_TARGET) $(RP2_SDK_LIBS))" >> $(RP2_CMAKEFILE)
	@echo "pico_enable_stdio_usb($(RP2_TARGET) 0)" >> $(RP2_CMAKEFILE)
	@echo "pico_enable_stdio_uart($(RP2_TARGET) 1)" >> $(RP2_CMAKEFILE)
	@echo "pico_add_extra_outputs($(RP2_TARGET))" >> $(RP2_CMAKEFILE)

$(RP2_BUILD_STAMP): $(RP2_CMAKEFILE) $(PLATFORM_CONFIG_FILE) $(PININFOFILE).h $(PININFOFILE).c $(WRAPPERFILE)
	@if [ -z "$(PICO_SDK_PATH)" ]; then \
		echo "ERROR: PICO_SDK_PATH is not set. Run: source scripts/provision.sh $(BOARD)"; \
		exit 1; \
	fi
	@cmake -S $(RP2_CMAKE_DIR) -B $(RP2_CMAKE_BUILD_DIR) -DPICO_BOARD=$(PICO_BOARD) $(if $(picotool_DIR),-Dpicotool_DIR=$(picotool_DIR),)
	@cmake --build $(RP2_CMAKE_BUILD_DIR)
	@touch $(RP2_BUILD_STAMP)

$(PROJ_NAME).elf: $(RP2_BUILD_STAMP)
	@cp $(RP2_CMAKE_BUILD_DIR)/$(RP2_TARGET).elf $(PROJ_NAME).elf

$(PROJ_NAME).bin: $(RP2_BUILD_STAMP)
	@cp $(RP2_CMAKE_BUILD_DIR)/$(RP2_TARGET).bin $(PROJ_NAME).bin

$(PROJ_NAME).uf2: $(RP2_BUILD_STAMP)
	@cp $(RP2_CMAKE_BUILD_DIR)/$(RP2_TARGET).uf2 $(PROJ_NAME).uf2

proj: $(PROJ_NAME).elf $(PROJ_NAME).bin $(PROJ_NAME).uf2

FORCE:
