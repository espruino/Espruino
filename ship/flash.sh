#!/bin/bash
/media/psf/Home/Desktop/Espruino/esp-idf/components/esptool_py/esptool/esptool.py \
	--chip esp32 \
	--port "/dev/ttyUSB0" \
	--baud 921600 \
	write_flash \
	-z \
	--flash_mode "dio" \
	--flash_freq "40m" \
	0x1000 bootloader.bin \
	0x10000 espruino_esp32.bin \
	0x4000 partitions_singleapp.bin
