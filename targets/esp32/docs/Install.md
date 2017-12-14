# Installation of Espruino
A build of Espruino consists of three binary files called:

* espruino_esp32.bin
* bootloader.bin
* paritions_espruino.bin

These files are loaded into an ESP32 using the `esptool.py` command.  An example
of usage would be:

```
esptool.py \
  --chip esp32 \
  --port "/dev/ttyUSB0" \
  --baud 115200 \
  write_flash \
  -z \
  --flash_mode "dio" \
  --flash_freq "40m" \
  0x10000 espruino_esp32.bin \  
  0x1000 ../app/build/bootloader/bootloader.bin \
  0x8000 ../app/build/partitions_espruino.bin

```

Remember to put your ESP32 into flash mode before running the command and then
reboot after loading the program.

The JavaScript development tooling (WebIDE) is best run through Chrome by installing
the [Espruino Web IDE](https://chrome.google.com/webstore/detail/espruino-web-ide/bleoifhkdalbjfbobjackfdifdneehpo).
