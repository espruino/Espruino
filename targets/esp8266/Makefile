## How to flash Espruino esp8266
## =============================
##
## *** Flash Tool
##
##  Windows, Linux and MacOSX:
##
##    esptool.py - serial flash tool
##      https://github.com/themadinventor/esptool 
##
##  Linux and MacOSX:
##
##    wififlash.sh - script to update flash espruino via wifi for 4MB esp8266 (e.g. esp-12)
##    ./scripts/wiflash <esp8266 hostname or ip>:88 espruino_esp8266_user1.bin espruino_esp8266_user2.bin
##
##
## The following targets are available (use esptool.py):
##


PORT=/dev/wchusbserial*

.PHONY: erase flash-esp-512k flash-esp-1MB flash-esp-2MB flash-esp-4MB flash-esp-4MB-1024

erase: ## Erase flash before updating to a new version 
	esptool.py --port ${PORT} --baud 115200 erase_flash

flash-esp-512k: ## Flash a 512KB esp8266 (e.g. esp-01) using the serial port use:
	esptool.py --port ${PORT} --baud 115200 write_flash \
		--flash_freq 40m --flash_mode qio --flash_size 4m \
		0x0000 "boot_v1.6.bin" 0x1000 espruino_esp8266_user1.bin \
		0x7C000 esp_init_data_default.bin 0x7E000 blank.bin

flash-esp-1MB: ## Flash a 1MB esp8266 using the serial port use:
	esptool.py --port ${PORT} --baud 115200 write_flash \
		--flash_freq 40m --flash_mode qio --flash_size 4m \
		0x0000 "boot_v1.6.bin" 0x1000 espruino_esp8266_user1.bin \
		0xFC000 esp_init_data_default.bin 0xFE000 blank.bin

flash-esp-2MB: ## Flash a 2MB esp8266 using the serial port use:
	esptool.py --port ${PORT} --baud 115200 write_flash \
		--flash_freq 40m --flash_mode qio --flash_size 4m \
		0x0000 "boot_v1.6.bin" 0x1000 espruino_esp8266_user1.bin \
		0x1FC000 esp_init_data_default.bin 0x1FE000 blank.bin

flash-esp-4MB: ## Flash a 4MB esp8266 (e.g. esp-12) using the serial port use:
	esptool.py --port ${PORT} --baud 115200 write_flash \
		--flash_freq 80m --flash_mode qio --flash_size 32m \
		0x0000 "boot_v1.6.bin" 0x1000 espruino_esp8266_user1.bin \
		0x3FC000 esp_init_data_default.bin 0x3FE000 blank.bin

flash-esp-4MB-1024: ## Flash a 4MB eps866 with Flash map 4MB:1024/1024 (BOARD=ESP8266_4MB)
	esptool.py --port ${PORT} --baud 460800 write_flash \
		--flash_freq 80m --flash_mode qio --flash_size 4MB-c1 \
		0x0000 "boot_v1.6.bin" 0x1000 espruino_esp8266_user1.bin \
		0x3FC000  esp_init_data_default.bin 0x3FE000 blank.bin

.DEFAULT_GOAL := help
.PHONY: help
help:
	@grep -E '^##.*$$' $(MAKEFILE_LIST) | awk 'BEGIN {FS = "## "}; {printf "%s\n", $$2}'
	@grep -E '^[a-zA-Z0-9_-]+:.*?## .*$$' $(MAKEFILE_LIST) | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-20s\033[0m %s\n", $$1, $$2}'
	@echo
