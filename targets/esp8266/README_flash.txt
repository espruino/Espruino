How to flash Espruino esp8266
=============================

*** Flash Tool

Windows, Linux and MacOSX:

    esptool.py - serial flash tool
    https://github.com/themadinventor/esptool 
    
Linux and MacOSX:
   
    wififlash.sh - script to update flash espruino via wifi for 4MB esp8266 (e.g. esp-12)
    ./scripts/wiflash <esp8266 hostname or ip>:88 espruino_esp8266_user1.bin espruino_esp8266_user2.bin

*** Erase flash before updating to a new version 

esptool.py --port [/dev/ttyUSB0|COM1] --baud 115200 erase_flash

espruino_<version>_esp8266
--------------------------

*** To flash a 512KB esp8266 (e.g. esp-01) using the serial port use:
esptool.py --port [/dev/ttyUSB0|COM1] --baud 115200 write_flash \
  --flash_freq 40m --flash_mode qio --flash_size 4m \
  0x0000 "boot_v1.6.bin" 0x1000 espruino_esp8266_user1.bin \
  0x7C000 esp_init_data_default.bin 0x7E000 blank.bin

For 1MB flash use 0xFC000/0xFE000 and for 2MB flash use 0x1FC000/0x1FE000 on the last line.

*** To flash a 4MB esp8266 (e.g. esp-12) using the serial port use:
esptool.py --port [/dev/ttyUSB0|COM1] --baud 115200 write_flash \
  --flash_freq 80m --flash_mode qio --flash_size 32m \
  0x0000 "boot_v1.6.bin" 0x1000 espruino_esp8266_user1.bin \
  0x3FC000 esp_init_data_default.bin 0x3FE000 blank.bin

espruino_<version>_esp8266_4MB
------------------------------

*** To flash a 4MB eps866 with Flash map 4MB:1024/1024 (BOARD=ESP8266_4MB)
esptool.py --port [/dev/ttyUSB0|COM1] --baud 460800 write_flash \
  --flash_freq 80m --flash_mode qio --flash_size 4MB-c1 \
  0x0000 "boot_v1.6.bin" 0x1000 espruino_esp8266_user1.bin \
  0x3FC000  esp_init_data_default.bin 0x3FE000 blank.bin

