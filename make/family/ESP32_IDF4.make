#
# Definitions for the build of the ESP32
#

ESP32_IDF4=1
#needed for using ifdef in wrapper JSON
DEFINES += -DESP32
DEFINES += -DESPR_DEFINES_ON_COMMANDLINE

SOURCES += targets/esp32/main.c \
targets/esp32/jshardware.c \
targets/esp32/jshardwareESP32.c \
targets/esp32/esp32_neopixel.c
INCLUDE += -I$(ROOT)/targets/esp32

ifdef USE_BLUETOOTH
SOURCES+= targets/esp32/bluetooth.c \
targets/esp32/BLE/esp32_bluetooth_utils.c \
targets/esp32/BLE/esp32_gap_func.c \
targets/esp32/BLE/esp32_gatts_func.c \
targets/esp32/BLE/esp32_gattc_func.c
endif

