#! /bin/bash
export RELEASE=1
export BOARD=ESP8266_4MB
export FLASH_4MB=1
export ESP8266_SDK_ROOT=/home/parallels/Desktop/openwrt/package/espruino/Espruino/esp_iot_sdk_v2.0.0.p1
export PATH=$PATH:/home/parallels/Desktop/openwrt/package/espruino/Espruino/xtensa-lx106-elf/bin
#export PATH=$PATH:/home/lancer/Desktop/Espruino/xtensa-lx106-elf/bin
#export COMPORT=/dev/ttyUSB0
make clean
make $* 

