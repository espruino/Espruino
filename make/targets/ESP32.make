ESP_ZIP     = $(PROJ_NAME).tgz

$(PROJ_NAME).bin: $(OBJS)
	$(LD) $(LDFLAGS) -o $(PROJ_NAME).elf -Wl,--start-group $(LIBS) $(OBJS) -Wl,--end-group
	python $(ESP_IDF_PATH)/components/esptool_py/esptool/esptool.py \
	--chip esp32 \
	elf2image \
	--flash_mode "dio" \
	--flash_freq "40m" \
	-o $(PROJ_NAME).bin \
	$(PROJ_NAME).elf

$(ESP_ZIP): $(PROJ_NAME).bin
	$(Q)rm -rf build/$(basename $(ESP_ZIP))
	$(Q)mkdir -p build/$(basename $(ESP_ZIP))
	$(Q)cp $(ESP_APP_TEMPLATE_PATH)/build/bootloader/bootloader.bin \
	  $(PROJ_NAME).bin \
	  $(ESP_APP_TEMPLATE_PATH)/build/partitions_espruino.bin \
	  targets/esp32/README_flash.txt \
	  build/$(basename $(ESP_ZIP))
	$(Q)tar -C build -zcf $(ESP_ZIP) ./$(basename $(ESP_ZIP))

proj: $(PROJ_NAME).bin $(ESP_ZIP)

flash:
	python $(ESP_IDF_PATH)/components/esptool_py/esptool/esptool.py \
	--chip esp32 \
	--port "/dev/ttyUSB0" \
	--baud 921600 \
	write_flash \
	-z \
	--flash_mode "dio" \
	--flash_freq "40m" \
	0x1000 $(ESP_APP_TEMPLATE_PATH)/build/bootloader/bootloader.bin \
	0x10000 $(PROJ_NAME).bin \
	0x8000 $(ESP_APP_TEMPLATE_PATH)/build/partitions_espruino.bin

erase_flash:
	python $(ESP_IDF_PATH)/components/esptool_py/esptool/esptool.py \
	--chip esp32 \
	--port "/dev/ttyUSB0" \
	--baud 921600 \
	erase_flash
