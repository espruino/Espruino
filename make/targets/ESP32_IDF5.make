ESP_ZIP     = $(PROJ_NAME).tgz

CMAKEFILE = $(BINDIR)/main/CMakeLists.txt
# 'gen' has a relative path - get rid of it and add it manually
INCLUDE_WITHOUT_GEN = $(subst -Igen,,$(INCLUDE)) -I$(ROOT)/gen

ifeq ($(CHIP),ESP32C3)
	SDKCONFIG = sdkconfig.defaults.esp32c3
	FMW_BIN_NAME = espruino-esp32c3
	PORT ?= /dev/ttyACM0
else 
	ifeq ($(CHIP),ESP32)
		SDKCONFIG = sdkconfig.defaults.esp32
		FMW_BIN_NAME = espruino-esp32
		PORT ?= /dev/ttyUSB0
	else
		ifeq ($(CHIP),ESP32S3)
			SDKCONFIG = sdkconfig.defaults.esp32s3
			FMW_BIN_NAME = espruino-esp32s3
			PORT ?= /dev/ttyUSB0
		else
			$(error Unknown ESP32 chip)
		endif
	endif
endif

$(CMAKEFILE):
	@echo "MAKE CMAKEFILE" 
	@echo "INCLUDE_WITHOUT_GEN: $(INCLUDE_WITHOUT_GEN)"
	@mkdir -p $(BINDIR)/main
	@touch $(CMAKEFILE)
	@echo 'set(ESPTOOLPY "$${IDF_PATH}/components/esptool_py/esptool/esptool.py" CACHE FILEPATH "" FORCE)' >> $(CMAKEFILE)
	@echo 'set(SDKCONFIG_DEFAULTS "sdkconfig.defaults $(SDKCONFIG)")' >> $(CMAKEFILE)
	@echo "idf_component_register(" >> $(CMAKEFILE)
	@echo "    SRCS" >> $(CMAKEFILE)
	@for s in $(SOURCES); do \
		echo "        $(ROOT)/$$s" >> $(CMAKEFILE); \
	done
	@echo "    INCLUDE_DIRS" >> $(CMAKEFILE)
	@for d in $(INCLUDE_WITHOUT_GEN); do \
		path=$${d#-I}; \
		path=$${path%/}; \
		path=$${path%;}; \
		echo "        $$path" >> $(CMAKEFILE); \
	done
	@echo "    PRIV_REQUIRES" >> $(CMAKEFILE)
	@for d in freertos \
		esp_common \
		esp_system \
		esp_hw_support \
		spi_flash \
		esp_event \
		esp_netif \
		esp_wifi \
		hal \
		lwip \
		mdns \
		nvs_flash \
		bt \
		app_update \
		esp_adc \
		mbedtls \
		vfs \
		heap \
		efuse \
		driver; do \
		echo "        $$d" >> $(CMAKEFILE); \
	done
	@echo ")" >> $(CMAKEFILE)
	@echo "target_compile_options(\$${COMPONENT_LIB} PUBLIC -DESP_IDF_VERSION_MAJOR=5)" >> $(CMAKEFILE)
	@echo "target_compile_options(\$${COMPONENT_LIB} PUBLIC $(DEFINES))" >> $(CMAKEFILE)
	@echo "target_compile_options(\$${COMPONENT_LIB} PUBLIC -Og -fno-strict-aliasing -ffunction-sections -fdata-sections -fstrict-volatile-bitfields -fgnu89-inline -nostdlib -MMD -MP -Wno-enum-compare)" >> $(CMAKEFILE)
	@echo "target_compile_options(\$${COMPONENT_LIB} PRIVATE -Wno-pointer-sign)" >> $(CMAKEFILE)
	@echo "target_compile_options(\$${COMPONENT_LIB} PRIVATE -Wno-implicit-int)" >> $(CMAKEFILE)
	@echo "target_compile_options(\$${COMPONENT_LIB} PRIVATE -Wno-maybe-uninitialized)" >> $(CMAKEFILE)
	@echo "target_compile_options(\$${COMPONENT_LIB} PRIVATE -Wno-return-type)" >> $(CMAKEFILE)
	@echo "target_compile_options(\$${COMPONENT_LIB} PRIVATE -Wno-switch)" >> $(CMAKEFILE)
	@echo "target_compile_options(\$${COMPONENT_LIB} PRIVATE -Wno-unused-variable)" >> $(CMAKEFILE)
	@echo "target_compile_options(\$${COMPONENT_LIB} PRIVATE -Wno-unused-function)" >> $(CMAKEFILE)
	@echo "target_compile_options(\$${COMPONENT_LIB} PRIVATE -Wno-unused-but-set-variable)" >> $(CMAKEFILE)
	@echo "target_compile_options(\$${COMPONENT_LIB} PRIVATE -Wno-cast-function-type)" >> $(CMAKEFILE)
	@echo "target_compile_options(\$${COMPONENT_LIB} PRIVATE -Wno-format)" >> $(CMAKEFILE)

$(PROJ_NAME).bin: $(CMAKEFILE) $(PLATFORM_CONFIG_FILE) $(PININFOFILE).h $(PININFOFILE).c $(WRAPPERFILE)
	$(Q)cp ${ROOT}/targets/esp32/IDF5/sdkconfig.defaults $(BINDIR)
	$(Q)cat ${ROOT}/targets/esp32/IDF5/${SDKCONFIG} >> $(BINDIR)/sdkconfig.defaults
	##$(Q)cp ${ROOT}/targets/esp32/IDF5/${SDKCONFIG} $(BINDIR)
	$(Q)cp ${ROOT}/targets/esp32/IDF5/CMakeLists.txt $(BINDIR)
	$(Q)cp ${ROOT}/targets/esp32/IDF5/partitions.csv $(BINDIR)
	$(Q)mkdir -p $(BINDIR)/main
	# Only add mdns dependency if missing
	$(Q)if [ ! -f "$(BINDIR)/main/idf_component.yml" ] || ! grep -q "espressif/mdns" "$(BINDIR)/main/idf_component.yml"; then cd $(BINDIR) && idf.py add-dependency "espressif/mdns^1.11.0"; fi
	cd $(BINDIR) && idf.py build
	$(Q)cp $(BINDIR)/build/espruino.bin $(PROJ_NAME).bin

$(ESP_ZIP): $(PROJ_NAME).bin
	$(Q)rm -rf $(PROJ_NAME)
	$(Q)mkdir -p $(PROJ_NAME)
	$(Q)cp $(PROJ_NAME).bin $(PROJ_NAME)/$(FMW_BIN_NAME).bin
	$(Q)cp $(BINDIR)/build/partition_table/partition-table.bin $(PROJ_NAME)/partition-table.bin
	$(Q)cp $(BINDIR)/build/bootloader/bootloader.bin $(PROJ_NAME)/bootloader.bin
	$(Q)cp targets/esp32/README_flash.txt $(PROJ_NAME)
	$(Q)cp targets/esp32/README_flash_C3.txt $(PROJ_NAME)
	$(Q)cp targets/esp32/README_flash_S3.txt $(PROJ_NAME)
	$(Q)$(TAR) -zcf $(ESP_ZIP) $(PROJ_NAME) --transform='s/$(BINDIR)\///g'
	@echo "Created $(ESP_ZIP)"

proj: $(ESP_ZIP)
#depend on $(PROJ_NAME).bin

flash-erase: $(PROJ_NAME).bin
	cd $(BINDIR) && idf.py erase-flash -p $(PORT)

flash: $(PROJ_NAME).bin
	cd $(BINDIR) && idf.py flash -p $(PORT)

monitor: $(PROJ_NAME).bin
	cd $(BINDIR) && idf.py monitor -p $(PORT)

# flashes but also automatically runs a terminal app right after
# Use Ctrl-] to exit
flashmonitor: $(PROJ_NAME).bin
	cd $(BINDIR) && idf.py flash -p $(PORT)
	cd $(BINDIR) && idf.py monitor -p $(PORT)

# Run QEMU in first windows and wait for debuger
debug:
	cd $(BINDIR) && idf.py qemu --gdb monitor

# Run QEMU in second window
# Use Ctrl-] to exit
qemu:
	cd $(BINDIR) && idf.py qemu monitor
