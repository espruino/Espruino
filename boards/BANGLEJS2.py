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
 'name' : "Bangle.js 2", # Using SMA Q3
 'link' :  [ "https://www.espruino.com/Bangle.js2" ],
 'espruino_page_link' : 'Bangle.js2',
 'default_console' : "EV_TERMINAL",
 #'default_console' : "EV_SERIAL1",
# 'default_console_tx' : "D6",
# 'default_console_rx' : "D8",
# 'default_console_baudrate' : "9600",
 'variables' : 12000, # How many variables are allocated for Espruino to use. RAM will be overflowed if this number is too high and code won't compile.
 'bootloader' : 1,
 'binary_name' : 'espruino_%v_banglejs2.hex',
 'build' : {
   'optimizeflags' : '-Os',
   'libraries' : [
     'BLUETOOTH',
     'TERMINAL',
     'GRAPHICS',
     'LCD_MEMLCD',
     'TENSORFLOW'  
   ],
   'makefile' : [
     'DEFINES += -DESPR_HWVERSION=2 -DBANGLEJS -DBANGLEJS_Q3',
#     'DEFINES += -DCONFIG_GPIO_AS_PINRESET', # Allow the reset pin to work
     'DEFINES += -DCONFIG_NFCT_PINS_AS_GPIOS', # Allow us to use NFC pins as GPIO
     #'DEFINES += -DESPR_REGOUT0_1_8V=1', # this increases power draw, so probably not correct!
     'DEFINES += -DESPR_LSE_ENABLE ', # Ensure low speed external osc enabled
     'DEFINES += -DNRF_SDH_BLE_GATT_MAX_MTU_SIZE=131', # 23+x*27 rule as per https://devzone.nordicsemi.com/f/nordic-q-a/44825/ios-mtu-size-why-only-185-bytes
     'LDFLAGS += -Xlinker --defsym=LD_APP_RAM_BASE=0x2ec0', # set RAM base to match MTU
     'DEFINES += -DESPR_DCDC_ENABLE=1', # Use DC/DC converter
     'ESPR_BLUETOOTH_ANCS=1', # Enable ANCS (Apple notifications) support
     'DEFINES += -DSPIFLASH_SLEEP_CMD', # SPI flash needs to be explicitly slept and woken up
     'DEFINES += -DNRF_BOOTLOADER_NO_WRITE_PROTECT=1', # By default the bootloader protects flash. Avoid this (a patch for NRF_BOOTLOADER_NO_WRITE_PROTECT must be applied first)
     'DEFINES += -DBUTTONPRESS_TO_REBOOT_BOOTLOADER',
     'DEFINES += -DAPP_TIMER_OP_QUEUE_SIZE=6', # Bangle.js accelerometer poll handler needs something else in queue size
     'DEFINES+=-DBLUETOOTH_NAME_PREFIX=\'"Bangle.js"\'',
     'DEFINES+=-DCUSTOM_GETBATTERY=jswrap_banglejs_getBattery',
     'DEFINES+=-DDUMP_IGNORE_VARIABLES=\'"g\\0"\'',
     'DEFINES+=-DUSE_FONT_6X8 -DGRAPHICS_PALETTED_IMAGES',
     'DEFINES+=-DNO_DUMP_HARDWARE_INITIALISATION', # don't dump hardware init - not used and saves 1k of flash
     'INCLUDE += -I$(ROOT)/libs/banglejs -I$(ROOT)/libs/misc',
     'WRAPPERSOURCES += libs/banglejs/jswrap_bangle.c',
     'WRAPPERSOURCES += libs/graphics/jswrap_font_6x15.c',
     'WRAPPERSOURCES += libs/graphics/jswrap_font_12x20.c',
     'SOURCES += libs/misc/nmea.c',
     'SOURCES += libs/misc/stepcount.c',
     'SOURCES += libs/misc/heartrate.c',
     'SOURCES += libs/misc/hrm_vc31.c',
     'SOURCES += libs/banglejs/banglejs2_storage_default.c',
     'DEFINES += -DESPR_STORAGE_INITIAL_CONTENTS=1', # 
     'JSMODULESOURCES += libs/js/banglejs/locale.min.js',

     'DFU_SETTINGS=--application-version 0xff --hw-version 52 --sd-req 0xa9,0xae,0xb6',
     'DFU_PRIVATE_KEY=targets/nrf5x_dfu/dfu_private_key.pem',
     'BOOTLOADER_SETTINGS_FAMILY=NRF52840',
     'NRF_SDK15=1'
   ]
 }
};


chip = {
  'part' : "NRF52840",
  'family' : "NRF52",
  'package' : "AQFN73",
  'ram' : 256,
  'flash' : 1024,
  'speed' : 64,
  'usart' : 2,
  'spi' : 1,
  'i2c' : 1,
  'adc' : 1,
  'dac' : 0,
  'saved_code' : {
#   'address' : ((246 - 10) * 4096), # Bootloader takes pages 248-255, FS takes 246-247
#  'page_size' : 4096,
#   'pages' : 10,
#   'flash_available' : 1024 - ((38 + 8 + 2 + 10)*4) # Softdevice uses 0x26=38 pages of flash, bootloader 8, FS 2, code 10. Each page is 4 kb.
  'address' : 0x60000000, # put this in external spiflash (see below)
  'page_size' : 4096,
  'pages' : 2048, # Entire 8MB of external flash
  'flash_available' : 1024 - ((38 + 8 + 2)*4) # Softdevice uses 31 pages of flash, bootloader 8, FS 2, code 10. Each page is 4 kb.
  },
};

devices = {
  'BTN1' : { 'pin' : 'D17', 'pinstate' : 'IN_PULLDOWN' }, # Pin negated in software
  'LED1' : { 'pin' : 'D8', 'novariable':True }, # Backlight flash for low level debug - but in code we just use 'fake' LEDs
  'LCD' : {
            'width' : 176, 'height' : 176, 
            'bpp' : 3,
            'controller' : 'LPM013M126', # LPM013M126C
            'pin_cs' : 'D5',
            'pin_extcomin' : 'D6',
            'pin_disp' : 'D7',
            'pin_sck' : 'D26',
            'pin_mosi' : 'D27',
            'pin_bl' : 'D8',
          },
  'TOUCH' : {
            'device' : 'CTS816S', 'addr' : 0x15,
            'pin_sda' : 'D33',
            'pin_scl' : 'D34',
            'pin_rst' : 'D35',
            'pin_irq' : 'D36'
          },
  'VIBRATE' : { 'pin' : 'D19' },
  'GPS' : {
            'device' : 'Casic URANUS',
            'pin_en' : 'D29', # IO expander P0
            'pin_rx' : 'D30', 
            'pin_tx' : 'D31'
          },
  'BAT' : {
            'pin_charging' : 'D23', # active low
            'pin_voltage' : 'D3'
          },
  'HEARTRATE' : {
            'device' : 'VC31', 'addr' : 0x33,            
            'pin_sda' : 'D24', 
            'pin_scl' : 'D32', 
            'pin_en' : 'D21', 
            'pin_int' : 'D22'
          },
  'ACCEL' : {
            'device' : 'KX023', 'addr' : 0x1e,
            'pin_sda' : 'D38',
            'pin_scl' : 'D37'
          },
  'MAG' : { # Magnetometer/compass
            'device' : 'UNKNOWN_0C', 
            'addr' : 0x0C,
            'pin_sda' : 'D44',
            'pin_scl' : 'D45'
          },
  'PRESSURE' : {
            'device' : 'BMP280', 
            'addr' : 0x76,
            'pin_sda' : 'D47',
            'pin_scl' : 'D2'            
  },
  'SPIFLASH' : {
            'pin_cs' : 'D14',
            'pin_sck' : 'D16',
            'pin_mosi' : 'D15', # D0
            'pin_miso' : 'D13', # D1
#            'pin_wp' : 'D', # D2
#            'pin_rst' : 'D', # D3
            'size' : 4096*2048, # 8MB
            'memmap_base' : 0x60000000 # map into the address space (in software)
          }
};

# left-right, or top-bottom order
board = {
};
board["_css"] = """
""";

def get_pins():
  pins = pinutils.generate_pins(0,47) # 48 General Purpose I/O Pins.
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
  # Make buttons and LEDs negated
  pinutils.findpin(pins, "PD17", True)["functions"]["NEGATED"]=0; # button

  # everything is non-5v tolerant
  for pin in pins:
    pin["functions"]["3.3"]=0;
  #The boot/reset button will function as a reset button in normal operation. Pin reset on PD21 needs to be enabled on the nRF52832 device for this to work.
  return pins
