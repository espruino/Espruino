#! /bin/bash

/home/parallels/Desktop/openwrt/package/espruino/Espruino/esp-idf/components/esptool_py/esptool/esptool.py \
--chip esp8266  \
--port /dev/ttyUSB0  \
--baud 115200  \
erase_flash 

/home/parallels/Desktop/openwrt/package/espruino/Espruino/esp-idf/components/esptool_py/esptool/esptool.py \
--chip esp8266  \
--port /dev/ttyUSB0  \
--baud 115200  \
write_flash \
--flash_mode "qio"  \
--flash_freq "80m"  \
--flash_size "4MB-c1"   \
0x0000 "./esp_iot_sdk_v2.0.0.p1/bin/boot_v1.6.bin" \
0x1000 espruino_esp8266_user1.bin \
0x3FC000 ./esp_iot_sdk_v2.0.0.p1/bin/esp_init_data_default.bin \
0x3FE000 ./esp_iot_sdk_v2.0.0.p1/bin/blank.bin
