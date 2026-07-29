RP2040_CMAKE_DIR := $(BINDIR)/rp2040
RP2040_CMAKE_BUILD_DIR := $(RP2040_CMAKE_DIR)/build
RP2040_CMAKEFILE := $(RP2040_CMAKE_DIR)/CMakeLists.txt
RP2040_BUILD_STAMP := $(RP2040_CMAKE_BUILD_DIR)/.build_stamp
RP2040_TARGET := espruino_rp2040

PICO_BOARD ?= pico

# 'gen' has a relative path - get rid of it and add it manually
INCLUDE_WITHOUT_GEN = $(subst -Igen,,$(INCLUDE)) -I$(ROOT)/gen

ifneq ("$(wildcard $(ROOT)/picotool-install/picotool/picotool)","")
picotool_DIR ?= $(ROOT)/picotool-install/picotool
endif

$(RP2040_CMAKEFILE): FORCE
	@mkdir -p $(RP2040_CMAKE_DIR)
	@echo "cmake_minimum_required(VERSION 3.13)" > $(RP2040_CMAKEFILE)
	@echo "set(CMAKE_C_STANDARD 11)" >> $(RP2040_CMAKEFILE)
	@echo "set(CMAKE_CXX_STANDARD 17)" >> $(RP2040_CMAKEFILE)
	@echo "if(NOT DEFINED ENV{PICO_SDK_PATH})" >> $(RP2040_CMAKEFILE)
	@echo "  message(FATAL_ERROR \"PICO_SDK_PATH is not set\")" >> $(RP2040_CMAKEFILE)
	@echo "endif()" >> $(RP2040_CMAKEFILE)
	@echo "include(\$$ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)" >> $(RP2040_CMAKEFILE)
	@echo "project(espruino_rp2040 C CXX ASM)" >> $(RP2040_CMAKEFILE)
	@echo "pico_sdk_init()" >> $(RP2040_CMAKEFILE)
	@echo "add_executable($(RP2040_TARGET)" >> $(RP2040_CMAKEFILE)
	@echo "  $(patsubst %,\"$(ROOT)/%\"\n  ,$(SOURCES))" >> $(RP2040_CMAKEFILE)
	@echo ")" >> $(RP2040_CMAKEFILE)
	@echo "target_include_directories($(RP2040_TARGET) PRIVATE" >> $(RP2040_CMAKEFILE)
	@echo "  $(patsubst -I%,\"%\"\n  ,$(INCLUDE_WITHOUT_GEN))" >> $(RP2040_CMAKEFILE)
	@echo ")" >> $(RP2040_CMAKEFILE)
	@echo "target_compile_options($(RP2040_TARGET) PRIVATE $(DEFINES))" >> $(RP2040_CMAKEFILE)
	@echo "target_compile_definitions($(RP2040_TARGET) PRIVATE PICO_FLASH_ASSUME_CORE1_SAFE=1)" >> $(RP2040_CMAKEFILE)
	@echo "target_compile_options($(RP2040_TARGET) PRIVATE -Wno-format -Wno-unused-function)" >> $(RP2040_CMAKEFILE)
	@echo "target_link_libraries($(RP2040_TARGET) pico_stdlib pico_unique_id pico_rand hardware_adc hardware_i2c hardware_pwm hardware_spi pico_flash tinyusb_device tinyusb_board tinyusb_additions)" >> $(RP2040_CMAKEFILE)
	@echo "pico_enable_stdio_usb($(RP2040_TARGET) 0)" >> $(RP2040_CMAKEFILE)
	@echo "pico_enable_stdio_uart($(RP2040_TARGET) 1)" >> $(RP2040_CMAKEFILE)
	@echo "pico_add_extra_outputs($(RP2040_TARGET))" >> $(RP2040_CMAKEFILE)

$(RP2040_BUILD_STAMP): $(RP2040_CMAKEFILE) $(PLATFORM_CONFIG_FILE) $(PININFOFILE).h $(PININFOFILE).c $(WRAPPERFILE)
	@if [ -z "$(PICO_SDK_PATH)" ]; then \
		echo "ERROR: PICO_SDK_PATH is not set. Run: source scripts/provision.sh RP2040_PICO"; \
		exit 1; \
	fi
	@cmake -S $(RP2040_CMAKE_DIR) -B $(RP2040_CMAKE_BUILD_DIR) -DPICO_BOARD=$(PICO_BOARD) $(if $(picotool_DIR),-Dpicotool_DIR=$(picotool_DIR),)
	@cmake --build $(RP2040_CMAKE_BUILD_DIR)
	@touch $(RP2040_BUILD_STAMP)

$(PROJ_NAME).elf: $(RP2040_BUILD_STAMP)
	@cp $(RP2040_CMAKE_BUILD_DIR)/$(RP2040_TARGET).elf $(PROJ_NAME).elf

$(PROJ_NAME).bin: $(RP2040_BUILD_STAMP)
	@cp $(RP2040_CMAKE_BUILD_DIR)/$(RP2040_TARGET).bin $(PROJ_NAME).bin

$(PROJ_NAME).uf2: $(RP2040_BUILD_STAMP)
	@cp $(RP2040_CMAKE_BUILD_DIR)/$(RP2040_TARGET).uf2 $(PROJ_NAME).uf2

proj: $(PROJ_NAME).elf $(PROJ_NAME).bin $(PROJ_NAME).uf2

FORCE:
