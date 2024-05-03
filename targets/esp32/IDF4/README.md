ESP32 IDF4 builds
=================

First install the 4.4.7 IDF and have it on your path: https://docs.espressif.com/projects/esp-idf/en/v4.4.7/esp32/get-started/index.html

```
BOARD=ESP32_IDF4 RELEASE=1 make
```

## STATUS

It builds, but fails as out of RAM.

**Pretty sure we need to toggle some flag in sdkconfig which sets the compiler to put all consts in flash**

## How it works

To enable, in `BOARD.py` set `family` to `ESP32_IDF4`. 

* `make/family/ESP32_IDF4.make` is then called, and creates a `build` folder in `bin/build`
* `make/targets/ESP32_IDF4.make` creates a CMakeFile and calls `idf.py` to do the build

## Setup

So far we need:

```
idf.py set-target esp32

idf.py menuconfig
# then
# Component - > FreeRTOS -> Kernel -> CONFIG_FREERTOS_ENABLE_BACKWARD_COMPATIBILITY must be enabled
# ESP NETIF Adapter -> Enable backward compatible tcpip_adapter interface
```
