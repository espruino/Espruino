How to flash Espruino ESP32
===========================

*** To flash an ESP32 using the serial port use:
python ${ESP_IDF_PATH}/components/esptool_py/esptool/esptool.py \
	--chip esp32 \
	--port "/dev/ttyUSB0" \
	--baud 921600 \
	write_flash \
	-z \
	--flash_mode "dio" \
	--flash_freq "40m" \
	0x1000 bootloader.bin \
	0x10000 espruino_esp32.bin \
	0x8000 partitions_espruino.bin

On windows, its an option to use flash tools from espressif found here:
http://espressif.com/sites/default/files/tools/flash_download_tools_v3.4.4.zip
