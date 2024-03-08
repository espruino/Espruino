#!/bin/false
# This file is part of Espruino, a JavaScript interpreter for Microcontrollers
#
# Copyright (C) 2013 Gordon Williams <gw@pur3.co.uk>
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# ----------------------------------------------------------------------------------------
# This file contains information for a specific board - the available pins, and where LEDs,
# Buttons, and other in-built peripherals are. It is used to build documentation as well
# as various source and header files for Espruino.
# ----------------------------------------------------------------------------------------

import pinutils;

info = {
 'name' : "Bangle.js",
 'link' :  [ "https://espruino.com/Bangle.js" ],
 'espruino_page_link' : 'Bangle.js',
 'default_console' : "EV_BLUETOOTH",
 'variables' : 2584, # How many variables are allocated for Espruino to use. RAM will be overflowed if this number is too high and code won't compile.
 'bootloader' : 1,
 'binary_name' : 'espruino_%v_banglejs.hex',
 'build' : {
   'optimizeflags' : '-Os',
   'libraries' : [
     'BLUETOOTH',
     'TERMINAL',
     'GRAPHICS',
     'LCD_ST7789_8BIT',
     'TENSORFLOW',
     'JIT'
   ],
   'makefile' : [
     'BLACKLIST=boards/BANGLEJS.blocklist', # force some stuff to be removed to save space
     'DEFINES += -DESPR_HWVERSION=1',
     'DEFINES += -DBANGLEJS_F18',
     'DEFINES += -DCONFIG_NFCT_PINS_AS_GPIOS', # Allow the reset pin to work
     'DEFINES += -DBUTTONPRESS_TO_REBOOT_BOOTLOADER',
     'DEFINES += -DDFU_APP_DATA_RESERVED=0', # allow firmware updates right up to the amount of available flash
     'DEFINES+=-DNRF_BLE_GATT_MAX_MTU_SIZE=53 -DNRF_BLE_MAX_MTU_SIZE=53', # increase MTU from default of 23
     'LDFLAGS += -Xlinker --defsym=LD_APP_RAM_BASE=0x2c40', # set RAM base to match MTU
     'DEFINES+=-DBLUETOOTH_NAME_PREFIX=\'"Bangle.js"\'',
     'DEFINES+=-DBLUETOOTH_ADVERTISING_INTERVAL=200', # since we don't care as much about ~20uA battery usage, raise this to make getting a connection faster
     'ESPR_BLUETOOTH_ANCS=1', # Enable ANCS (Apple notifications) support
     'DEFINES+=-DCUSTOM_GETBATTERY=jswrap_banglejs_getBattery',
     'DEFINES+=-DESPR_UNICODE_SUPPORT=1',
     'DEFINES+=-DESPR_NO_SOFTWARE_SERIAL=1',
     'DEFINES+=-DDUMP_IGNORE_VARIABLES=\'"g\\0"\'',
     'DEFINES+=-DESPR_GRAPHICS_INTERNAL=1',
     'DEFINES+=-DUSE_FONT_6X8 -DGRAPHICS_PALETTED_IMAGES -DGRAPHICS_ANTIALIAS -DESPR_PBF_FONTS',
     'DEFINES+=-DNO_DUMP_HARDWARE_INITIALISATION', # don't dump hardware init - not used and saves 1k of flash
     'DEFINES+=-DESPR_NO_LINE_NUMBERS=1', # we execute mainly from flash, so line numbers can be worked out
     'DEFINES+=-DAPP_TIMER_OP_QUEUE_SIZE=6', # Bangle.js accelerometer poll handler needs something else in queue size
     'DFU_PRIVATE_KEY=targets/nrf5x_dfu/dfu_private_key.pem',
     'DFU_SETTINGS=--application-version 0xff --hw-version 52 --sd-req 0x8C,0x91',
     'INCLUDE += -I$(ROOT)/libs/banglejs -I$(ROOT)/libs/misc',
     'WRAPPERSOURCES += libs/banglejs/jswrap_bangle.c',
     'SOURCES += libs/misc/nmea.c',
     'SOURCES += libs/misc/stepcount.c',
     'SOURCES += libs/misc/heartrate.c',
     'SOURCES += libs/misc/hrm_analog.c',
     'JSMODULESOURCES += libs/js/banglejs/locale.min.js',
     'NRF_BL_DFU_INSECURE=1',
     'LINKER_BOOTLOADER=targetlibs/nrf5x_12/nrf5x_linkers/banglejs_dfu.ld',
     'LINKER_ESPRUINO=targetlibs/nrf5x_12/nrf5x_linkers/banglejs_espruino.ld',
# Uncomment these lines to allow the Bangle.js 1 bootloader to check external flash for firmware and
# update from there. We do this by default on Bangle.js 2 but on Bangle.js 1 it's  bit tight to get it into
# available flash memory. See #2449
#     'DEFINES += -DESPR_BOOTLOADER_SPIFLASH', # Allow bootloader to flash direct from SPI flash
#     'BOOTLOADER_DEFINES += -DNRF_BL_DFU_TRIM_EXTREME',
#     'BOOTLOADER_LDFLAGS += -nostartfiles',
#     'BOOTLOADER_ASFLAGS += -D__STARTUP_CLEAR_BSS -D__START=main',
   ]
 }
};


chip = {
  'part' : "NRF52832",
  'family' : "NRF52",
  'package' : "QFN48",
  'ram' : 64,
  'flash' : 512,
  'speed' : 64,
  'usart' : 1,
  'spi' : 0, # hardware supports 1, but we don't use these
  'i2c' : 0, # hardware supports 1, but we don't use these
  'adc' : 1,
  'dac' : 0,
  'saved_code' : {
    'address' : 0x60000000, # put this in external spiflash (see below)
    'page_size' : 4096,
    'pages' : 1024, # Entire 4MB of external flash
    'flash_available' : 512 - ((31 + 8 + 2)*4) # Softdevice uses 31 pages of flash, bootloader 8, FS 2. Each page is 4 kb.
  },
};

devices = {
  'BTN1' : { 'pin' : 'D24', 'pinstate' : 'IN_PULLDOWN' }, # top
  'BTN2' : { 'pin' : 'D22', 'pinstate' : 'IN_PULLDOWN' }, # middle
  'BTN3' : { 'pin' : 'D23', 'pinstate' : 'IN_PULLDOWN' }, # bottom
  'BTN4' : { 'pin' : 'D11', 'pinstate' : 'IN_PULLDOWN' }, # touch left
  'BTN5' : { 'pin' : 'D16', 'pinstate' : 'IN_PULLDOWN' }, # touch right
  'VIBRATE' : { 'pin' : 'D13' },
  'SPEAKER' : { 'pin' : 'D18' },
  'LCD' : {
            'width' : 240, 'height' : 240, 'bpp' : 16,
            'controller' : 'st7789_8bit', # 8 bit parallel mode
            'pin_dc' : 'D8',
            'pin_cs' : 'D10',
#            'pin_rst' : '', # IO expander P6
            'pin_sck' : 'D9',
            'pin_d0' : 'D0',
            'pin_d1' : 'D1',
            'pin_d2' : 'D2',
            'pin_d3' : 'D3',
            'pin_d4' : 'D4',
            'pin_d5' : 'D5',
            'pin_d6' : 'D6',
            'pin_d7' : 'D7',
#            'pin_bl' : '', # IO expander P5
          },
  'GPS' : {
            'device' : 'M8130-KT',
#            'pin_en' : '', # IO expander P0
            'pin_rx' : 'D25',
            'pin_tx' : 'D26'
          },
  'BAT' : {
            'pin_charging' : 'D12', # active low, input pullup
            'pin_voltage' : 'D30'
          },
  'HEARTRATE' : {
            'device' : 'analog',
           # 'pin_led' : '', IO expander P7
            'pin_analog' : 'D29'
          },
  'ACCEL' : {
            'device' : 'KX023', 'addr' : 0x1e,
            'pin_sda' : 'D15',
            'pin_scl' : 'D14'
          },
  'MAG' : { # Magnetometer/compass
            'device' : 'GMC303',
            'addr' : 0x0C,
            'pin_sda' : 'D15',
            'pin_scl' : 'D14'
          },
  'SPIFLASH' : {
            'pin_cs' : 'D21',
            'pin_sck' : 'D19',
            'pin_mosi' : 'D27', # D0
            'pin_miso' : 'D20', # D1
            'pin_wp' : 'D31', # D2
            'pin_rst' : 'D17', # D3
            'size' : 4096*1024, # 4MB
            'memmap_base' : 0x60000000 # map into the address space (in software)
          },
  'MISC' : {
    'pin_nenable' : 'D18', # needed to get power to Bangle.js peripherals
    'pin_ioexp_reset' : 'D28',
  }
};

# left-right, or top-bottom order
board = {
};

def get_pins():
  pins = pinutils.generate_pins(0,31) # 32 General Purpose I/O Pins.
  pinutils.findpin(pins, "PD0", True)["functions"]["XL1"]=0;
  pinutils.findpin(pins, "PD1", True)["functions"]["XL2"]=0;
  pinutils.findpin(pins, "PD2", True)["functions"]["ADC1_IN0"]=0;
  pinutils.findpin(pins, "PD3", True)["functions"]["ADC1_IN1"]=0;
  pinutils.findpin(pins, "PD4", True)["functions"]["ADC1_IN2"]=0;
  pinutils.findpin(pins, "PD5", True)["functions"]["ADC1_IN3"]=0;
  pinutils.findpin(pins, "PD28", True)["functions"]["ADC1_IN4"]=0;
  pinutils.findpin(pins, "PD29", True)["functions"]["ADC1_IN5"]=0;
  pinutils.findpin(pins, "PD30", True)["functions"]["ADC1_IN6"]=0;
  pinutils.findpin(pins, "PD31", True)["functions"]["ADC1_IN7"]=0;
  # negate buttons
  pinutils.findpin(pins, "PD11", True)["functions"]["NEGATED"]=0; # btn
  pinutils.findpin(pins, "PD16", True)["functions"]["NEGATED"]=0; # btn
  pinutils.findpin(pins, "PD22", True)["functions"]["NEGATED"]=0; # btn
  pinutils.findpin(pins, "PD23", True)["functions"]["NEGATED"]=0; # btn
  pinutils.findpin(pins, "PD24", True)["functions"]["NEGATED"]=0; # btn
  for pin in pins:
    pin["functions"]["NO_BLOCKLY"]=0;  # hide in blockly

  # everything is non-5v tolerant
  for pin in pins:
    pin["functions"]["3.3"]=0;
  #The boot/reset button will function as a reset button in normal operation. Pin reset on PD21 needs to be enabled on the nRF52832 device for this to work.
  return pins
