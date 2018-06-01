#
# Definitions for the build of the ESP32
#

ESP32=1

CFLAGS+=-Og -Wpointer-arith -Wno-error=unused-function -Wno-error=unused-but-set-variable \
-Wno-error=unused-variable -Wall -ffunction-sections -fdata-sections -mlongcalls -nostdlib \
-MMD -MP -std=gnu99 -fstrict-volatile-bitfields -fgnu89-inline -mfix-esp32-psram-cache-issue
SOURCES += targets/esp32/jshardware.c
SOURCES += targets/esp32/esp32_neopixel.c
INCLUDE += -I$(ROOT)/targets/esp32

ifndef ESP_IDF_PATH
$(error "The ESP_IDF_PATH variable must be set")
endif
ifndef ESP_APP_TEMPLATE_PATH
$(error "The ESP_APP_TEMPLATE_PATH variable must be set")
endif
# The prefix for the ESP32 compiler
CCPREFIX=xtensa-esp32-elf-
SOURCES += targets/esp32/main.c
LDFLAGS += -L$(ESP_IDF_PATH)/ld \
-L$(ESP_IDF_PATH)/components/bt/lib \
-L$(ESP_IDF_PATH)/components/esp32/lib \
-L$(ESP_APP_TEMPLATE_PATH)/build/bootloader \
-L$(ESP_APP_TEMPLATE_PATH)/build/bt \
-L$(ESP_APP_TEMPLATE_PATH)/build/driver \
-L$(ESP_APP_TEMPLATE_PATH)/build/esp32 \
-L$(ESP_APP_TEMPLATE_PATH)/build/esptool_py \
-L$(ESP_APP_TEMPLATE_PATH)/build/expat \
-L$(ESP_APP_TEMPLATE_PATH)/build/freertos \
-L$(ESP_APP_TEMPLATE_PATH)/build/json \
-L$(ESP_APP_TEMPLATE_PATH)/build/log \
-L$(ESP_APP_TEMPLATE_PATH)/build/lwip \
-L$(ESP_APP_TEMPLATE_PATH)/build/mbedtls \
-L$(ESP_APP_TEMPLATE_PATH)/build/newlib \
-L$(ESP_APP_TEMPLATE_PATH)/build/nghttp \
-L$(ESP_APP_TEMPLATE_PATH)/build/nvs_flash \
-L$(ESP_APP_TEMPLATE_PATH)/build/partition_table \
-L$(ESP_APP_TEMPLATE_PATH)/build/pthread \
-L$(ESP_APP_TEMPLATE_PATH)/build/heap \
-L$(ESP_APP_TEMPLATE_PATH)/build/soc \
-L$(ESP_APP_TEMPLATE_PATH)/build/spi_flash \
-L$(ESP_APP_TEMPLATE_PATH)/build/tcpip_adapter \
-L$(ESP_APP_TEMPLATE_PATH)/build/vfs \
-L$(ESP_APP_TEMPLATE_PATH)/build/newlib \
-L$(ESP_APP_TEMPLATE_PATH)/build/wpa_supplicant \
-L$(ESP_APP_TEMPLATE_PATH)/build/ethernet \
-L$(ESP_APP_TEMPLATE_PATH)/build/app_update \
-L$(ESP_IDF_PATH)/components/esp32/ld \
-lgcc
ESPTOOL?=
INCLUDE+=\
-I$(ESP_APP_TEMPLATE_PATH)/build/include \
-I$(ESP_IDF_PATH)/components \
-I$(ESP_IDF_PATH)/components/newlib/include \
-I$(ESP_IDF_PATH)/components/pthread/include \
-I$(ESP_IDF_PATH)/components/bt/include \
-I$(ESP_IDF_PATH)/components/driver/include \
-I$(ESP_IDF_PATH)/components/esp32/include \
-I$(ESP_IDF_PATH)/components/freertos/include \
-I$(ESP_IDF_PATH)/components/json/include \
-I$(ESP_IDF_PATH)/components/log/include \
-I$(ESP_IDF_PATH)/components/lwip/include/lwip \
-I$(ESP_IDF_PATH)/components/lwip/include/lwip/port \
-I$(ESP_IDF_PATH)/components/lwip/include/lwip/posix \
-I$(ESP_IDF_PATH)/components/newlib/include \
-I$(ESP_IDF_PATH)/components/spi_flash/include \
-I$(ESP_IDF_PATH)/components/nvs_flash/include \
-I$(ESP_IDF_PATH)/components/tcpip_adapter/include \
-I$(ESP_IDF_PATH)/components/vfs/include \
-I$(ESP_IDF_PATH)/components/heap/include \
-I$(ESP_IDF_PATH)/components/soc/include \
-I$(ESP_IDF_PATH)/components/soc/esp32/include \
-I$(ESP_IDF_PATH)/components/soc/esp32/include/soc \
-Itargets/esp32/include
LDFLAGS+=-nostdlib -u call_user_start_cpu0 -Wl,--gc-sections -Wl,-static -Wl,-EL
LIBS+=-T esp32_out.ld \
-T$(ESP_IDF_PATH)/components/esp32/ld/esp32.common.ld \
-T$(ESP_IDF_PATH)/components/esp32/ld/esp32.rom.ld \
-T$(ESP_IDF_PATH)/components/esp32/ld/esp32.peripherals.ld \
$(ESP_IDF_PATH)/components/esp32/lib/librtc.a \
$(ESP_IDF_PATH)/components/esp32/lib/libnet80211.a \
$(ESP_IDF_PATH)/components/newlib/lib/libc.a \
$(ESP_IDF_PATH)/components/newlib/lib/libm.a \
$(ESP_IDF_PATH)/components/esp32/lib/libwpa2.a \
$(ESP_IDF_PATH)/components/esp32/lib/libwps.a \
$(ESP_IDF_PATH)/components/newlib/lib/libc-psram-workaround.a \
$(ESP_IDF_PATH)/components/newlib/lib/libm-psram-workaround.a \
$(ESP_IDF_PATH)/components/esp32/lib/libcore.a \
-lbt \
-lbtdm_app \
-ldriver \
-lesp32 \
$(ESP_IDF_PATH)/components/esp32/libhal.a  \
-lcore \
-lnet80211 \
-lphy \
-lwpa_supplicant \
-lrtc \
-lpp \
-lwpa \
-lexpat \
-lfreertos \
-ljson \
-llog \
-llwip \
-lmbedtls \
-lnghttp \
-lnvs_flash \
-lheap \
-lpthread \
-lsoc \
-lspi_flash \
-ltcpip_adapter \
-lvfs \
-lnewlib \
-lcoexist \
-lethernet \
-lapp_update \
-lstdc++ \
-lgcc

#needed for using ifdef in wrapper JSON
DEFINES += -DESP32

ifdef USE_BLUETOOTH
SOURCES+= targets/esp32/bluetooth.c \
targets/esp32/BLE/esp32_bluetooth_utils.c \
targets/esp32/BLE/esp32_gap_func.c \
targets/esp32/BLE/esp32_gatts_func.c \
targets/esp32/BLE/esp32_gattc_func.c \
targets/esp32/jshardwareESP32.c
INCLUDE+= -I$(ESP_IDF_PATH)/components/bt/bluedroid/include \
-I$(ESP_IDF_PATH)/components/bt/bluedroid/api/include \
-I$(ESP_IDF_PATH)/components/bt/bluedroid/bta/include \
-I$(ESP_IDF_PATH)/components/bt/bluedroid/stack/include \
-I$(ESP_IDF_PATH)/components/bt/bluedroid/stack/gatt/include \
-I$(ESP_IDF_PATH)/components/bt/bluedroid/osi/include 
LDFLAGS+= -L$(ESP_APP_TEMPLATE_PATH)/build/components/bt/bluedroid/api \
-L$(ESP_APP_TEMPLATE_PATH)/build/components/bt/bluedroid/bta 
endif

