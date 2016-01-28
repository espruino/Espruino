How to flash Espruino esp8266
=============================

*** To flash a 512KB esp8266 (e.g. esp-01) using the serial port use:
esptool.py --port [/dev/ttyUSB0|COM1] --baud 460800 write_flash \
  --flash_freq 40m --flash_mode qio --flash_size 4m \
  0x0000 "boot_v1.4(b1).bin" 0x1000 espruino_esp8266_user1.bin 0x7E000 blank.bin

*** To flash a 4MB esp8266 (e.g. esp-12) using the serial port use:
esptool.py --port [/dev/ttyUSB0|COM1] --baud 460800 write_flash \
  --flash_freq 80m --flash_mode qio --flash_size 32m \
  0x0000 "boot_v1.4(b1).bin" 0x1000 espruino_esp8266_user1.bin 0x37E000 blank.bin

*** Get esptool.py from https://github.com/themadinventor/esptool

*** To flash a 4MB esp8266 (e.g. esp-12) via wifi use (with appropriate hostname):
./scripts/wiflash espruino.local:88 espruino_esp8266_user1.bin espruino_esp8266_user2.bin

