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


FLASH_FREQ=80m
FLASH_MODE=qio
PORT=/dev/tty.wchusbserial*

.PHONY: erase flash-512k flash-1MB flash-2MB flash-4MB flash-4MB-1024

erase: ## Erase flash before updating to a new version 
	esptool.py --port ${PORT} --baud 115200 erase_flash

flash-512k: ## Flash a 512KB esp8266 (e.g. esp-01) using the serial
	esptool.py --port ${PORT} --baud 115200 write_flash \
		--flash_freq ${FLASH_FREQ} --flash_mode ${FLASH_MODE} --flash_size 512KB \
		0x0000 "boot_v1.6.bin" 0x1000 espruino_esp8266_user1.bin \
		0x7C000 esp_init_data_default.bin 0x7E000 blank.bin

flash-1MB: ## Flash a 1MB esp8266 using the serial
	esptool.py --port ${PORT} --baud 115200 write_flash \
		--flash_freq ${FLASH_FREQ} --flash_mode ${FLASH_MODE} --flash_size 1MB \
		0x0000 "boot_v1.6.bin" 0x1000 espruino_esp8266_user1.bin \
		0xFC000 esp_init_data_default.bin 0xFE000 blank.bin

flash-2MB: ## Flash a 2MB esp8266 using the serial
	esptool.py --port ${PORT} --baud 115200 write_flash \
		--flash_freq ${FLASH_FREQ} --flash_mode ${FLASH_MODE} --flash_size 2MB \
		0x0000 "boot_v1.6.bin" 0x1000 espruino_esp8266_user1.bin \
		0x1FC000 esp_init_data_default.bin 0x1FE000 blank.bin

flash-4MB: ## Flash a 4MB esp8266 (e.g. esp-12) using the serial
	esptool.py --port ${PORT} --baud 115200 write_flash \
		--flash_freq ${FLASH_FREQ} --flash_mode ${FLASH_MODE} --flash_size 4MB \
		0x0000 "boot_v1.6.bin" 0x1000 espruino_esp8266_user1.bin \
		0x3FC000 esp_init_data_default.bin 0x3FE000 blank.bin

flash-4MB-1024: ## Flash a 4MB eps866 with Flash map 4MB:1024/1024 (BOARD=ESP8266_4MB) using the serial
	esptool.py --port ${PORT} --baud 460800 write_flash \
		--flash_freq ${FLASH_FREQ} --flash_mode ${FLASH_MODE} --flash_size 4MB-c1 \
		0x0000 "boot_v1.6.bin" 0x1000 espruino_esp8266_user1.bin \
		0x3FC000  esp_init_data_default.bin 0x3FE000 blank.bin

.DEFAULT_GOAL := help
.PHONY: help
help:
	@grep -E '^##.*$$' $(MAKEFILE_LIST) | awk 'BEGIN {FS = "## "}; {printf "%s\n", $$2}'
	@grep -E '^[a-zA-Z0-9_-]+:.*?## .*$$' $(MAKEFILE_LIST) | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-20s\033[0m %s\n", $$1, $$2}'
	@echo
