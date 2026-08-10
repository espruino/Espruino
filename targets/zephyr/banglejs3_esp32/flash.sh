#!/bin/bash
esptool.py --chip esp32c2 -p /dev/ttyUSB0 -b 460800 --before=default_reset --after=hard_reset write_flash --flash_mode dio --flash_freq 60m --flash_size 4MB 0x0 bootloader.bin 0x60000 esp-at.bin 0x8000 partition-table.bin 0xd000 ota_data_initial.bin 0x1e000 at_customize.bin 0x1f000 mfg_nvs.bin
