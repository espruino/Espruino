ZEPHYR=1
SDKROOT = /home/gw/ncs/v3.3.0

DEFINES += -DESPR_DEFINES_ON_COMMANDLINE -DZEPHYR=1

INCLUDE += -I$(ROOT)/targets/zephyr
#INCLUDE += -I$(SDKROOT)/nrf-bm/include/bm/bluetooth/
#INCLUDE += -I$(SDKROOT)/nrf-bm/include/bm/bluetooth/services/
#INCLUDE += -I$(SDKROOT)/nrf-bm/include/bm/bluetooth/peer_manager/
#INCLUDE += -I$(SDKROOT)/nrf-bm/lib/bluetooth/peer_manager/include/
#INCLUDE += -I$(SDKROOT)/nrf-bm/lib/bluetooth/peer_manager/include/modules/
#INCLUDE += -I$(SDKROOT)/modules/hal/nordic/nrfx/bsp/stable/mdk/

SOURCES += targets/zephyr/main.c
SOURCES += targets/zephyr/jshardware.c
ifeq ($(USE_BLUETOOTH),1)
SOURCES += targets/zephyr/bluetooth.c
endif