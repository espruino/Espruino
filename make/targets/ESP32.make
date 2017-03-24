ESP_ZIP     = $(PROJ_NAME).tgz

espruino_esp32.bin: $(OBJS)
	$(LD) $(LDFLAGS) -o espruino_esp32.elf -Wl,--start-group $(LIBS) $(OBJS) -Wl,--end-group
	python $(ESP_IDF_PATH)/components/esptool_py/esptool/esptool.py \
	--chip esp32 \
	elf2image \
	--flash_mode "dio" \
	--flash_freq "40m" \
	-o espruino_esp32.bin \
	espruino_esp32.elf

$(ESP_ZIP): espruino_esp32.bin
	$(Q)rm -rf build/$(basename $(ESP_ZIP))
	$(Q)mkdir -p build/$(basename $(ESP_ZIP))
	$(Q)cp $(ESP_APP_TEMPLATE_PATH)/build/bootloader/bootloader.bin \
	  espruino_esp32.bin \
	  $(ESP_APP_TEMPLATE_PATH)/build/partitions_singleapp.bin \
	  targets/esp32/README_flash.txt \
	  build/$(basename $(ESP_ZIP))
	$(Q)tar -C build -zcf $(ESP_ZIP) ./$(basename $(ESP_ZIP))

proj: espruino_esp32.bin $(ESP_ZIP)

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
	0x10000 espruino_esp32.bin \
	0x8000 $(ESP_APP_TEMPLATE_PATH)/build/partitions_singleapp.bin

erase_flash:
	python $(ESP_IDF_PATH)/components/esptool_py/esptool/esptool.py \
	--chip esp32 \
	--port "/dev/ttyUSB0" \
	--baud 921600 \
	erase_flash
