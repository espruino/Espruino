How to flash Espruino ESP32C3
===========================

To flash an ESP32 using the serial port use:

```
esptool.py -p (PORT) -b 460800 --before default_reset --after hard_reset  \
    --chip esp32c3  write_flash \
    --flash_mode dio --flash_size detect --flash_freq 80m  \
    0x0 bootloader.bin  \
    0x8000 partition-table.bin  \
    0x10000 espruino-esp32c3.bin
```

On Windows, you can use the flash tools from espressif found here:
http://espressif.com/sites/default/files/tools/flash_download_tools_v3.4.4.zip
