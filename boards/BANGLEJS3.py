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
'''

See "targets/zephyr/Banglejs 3 Notes.md"

V0 = BTN1
V1 = BTN2
V2 = BTN3
V3 = BTN4
V4 = PY32_OUT_LCD_BL
V5 = PY32_OUT_TORCH_ON
V6 = PY32_OUT_RGB_ON
V7 = PY32_OUT_SPEAKER_ON
V8 = PY32_OUT_VIBRATE_ON
V9 = PY32_OUT_CHARGE_EN
V10 = PY32_OUT_WIFI_ON
V11 = PY32_OUT_WIFI_BOOTLOADER
V12 = PY32_OUT_AUX_SWAP
V13 = PY32_OUT_AUX_POWER
V14 = PY32_OUT_TOUCH_RST
V15 = PY32_OUT_HRM_AUX

FIXME: separate virtual outputs for torch R/G/B?
'''

import pinutils;

info = {
 'name' : "Bangle.js 3",
# 'default_console' : "EV_BLUETOOTH",
 #'default_console_tx' : "B6",
 #'default_console_rx' : "B7",
 #'default_console_baudrate' : "9600",
 'variables' : 2630, # How many variables are allocated for Espruino to use. RAM will be overflowed if this number is too high and code won't compile.
# 'bootloader' : 1,
 'io_buffer_size' : 2048, # How big is the input buffer (in bytes). Default on nRF52 is 1024
 'binary_name' : 'espruino_%v_banglejs3.hex',
 'build' : {
   'libraries' : [
     'BLUETOOTH',
     'NET',
     'GRAPHICS',
     'LCD_MEMLCD',
     'JIT' # JIT compiler enabled
   ],
   'makefile' : [
     'DEFINES+=-DESPR_OFFICIAL_BOARD', # Don't display the donations nag screen
     'DEFINES += -DESPR_HWVERSION=3 -DBANGLEJS -DBANGLEJS3',
     'DEFINES+=-DBLUETOOTH_NAME_PREFIX="Bangle.js"',
     'DEFINES+=-DCUSTOM_GETBATTERY=jswrap_banglejs_getBattery',
     'DEFINES+=-DESPR_UNICODE_SUPPORT=1',
     'DEFINES+=-DDUMP_IGNORE_VARIABLES="g\\0"',
     'DEFINES+=-DESPR_GRAPHICS_INTERNAL=1',
     'DEFINES+=-DESPR_BATTERY_FULL_VOLTAGE=0.3144',
     'DEFINES+=-DUSE_FONT_6X8 -DGRAPHICS_PALETTED_IMAGES -DGRAPHICS_ANTIALIAS -DESPR_PBF_FONTS',
     'DEFINES+=-DNO_DUMP_HARDWARE_INITIALISATION', # don't dump hardware init - not used and saves 1k of flash
     'INCLUDE += -I$(ROOT)/libs/banglejs -I$(ROOT)/libs/misc',
     'WRAPPERSOURCES += libs/banglejs/jswrap_bangle.c',
     'WRAPPERSOURCES += libs/banglejs/jswrap_bangle3.c',
     'WRAPPERSOURCES += libs/graphics/jswrap_font_14.c',
     'WRAPPERSOURCES += libs/graphics/jswrap_font_17.c',
     'WRAPPERSOURCES += libs/graphics/jswrap_font_22.c',
     'WRAPPERSOURCES += libs/graphics/jswrap_font_28.c',
     'WRAPPERSOURCES += libs/graphics/jswrap_font_6x15.c',
     'WRAPPERSOURCES += libs/graphics/jswrap_font_12x20.c',
     'DEFINES+=-DESPR_FONT_14 -DESPR_FONT_17 -DESPR_FONT_22 -DESPR_FONT_28 -DESPR_FONT_6x15 -DESPR_FONT_12x20',
     'SOURCES += libs/misc/stepcount.c',
     'INCLUDE += -I$(ROOT)/libs/neopixel',
     'SOURCES += libs/neopixel/neopixel_bitbang.c',
# ------------------------
     'SOURCES += libs/misc/unistroke.c',
     'WRAPPERSOURCES += libs/misc/jswrap_unistroke.c',
     'DEFINES += -DESPR_BANGLE_UNISTROKE=1',
     'SOURCES += libs/banglejs/banglejs3_storage_default.c',
     'DEFINES += -DESPR_STORAGE_INITIAL_CONTENTS=1', # use banglejs3_storage_default
     'DEFINES += -DESPR_USE_STORAGE_CACHE=32', # Add a 32 entry cache to speed up finding files
     'JSMODULESOURCES += libs/js/banglejs/locale.min.js',
     'JSMODULESOURCES += libs/js/banglejs/Layout.min.js',
     'JSMODULESOURCES+=libs/js/AT.min.js',
     'JSMODULESOURCES+=libs/js/banglejs/Wifi.js', # FIXME:Minify
   ]
 }
};


chip = {
  'part' : "NRF54L15",
  'family' : "ZEPHYR",
  'package' : "",
  'ram' : 256,
  'flash' : 1536,
  'speed' : 128,
  'usart' : 2,
  'spi' : 1,
  'i2c' : 0,
  'adc' : 0,
  'dac' : 0,
  'saved_code' : {
    'page_size' : 4096,
    'flash_available' : 1024,
    # internal flash - debug only
    #'pages' : 64,
    #'address' : (357-64)*4096, # (357->384 seem protected?)

    # external flash
    'address' : 0x60000000, # put this in external spiflash (see below)
    'pages' : 2048, # Entire 8MB of external flash
  },
};

devices = { # 'V' pins are virtual
  'BTN1' : { 'pin' : 'V0' },
  'BTN2' : { 'pin' : 'V1' },
  'BTN3' : { 'pin' : 'V2' },
  'BTN4' : { 'pin' : 'V3' },
  'LED1' : { 'pin' : 'V5' },
  'SPIFLASH' : {
    'size' : 64*1024*1024, # 64MB
    'memmap_base' : 0x60000000 # map into the address space (in software)
  },
  'LCD' : {
            'width' : 240, 'height' : 240,
            'bpp' : 6,
            'controller' : 'ZJ012BD01A', # ZJ012BD-01A (via Puya PY32)
            'pin_bl' : 'V4',
            'pin_irq' : 'A4',
            'pin_cs' : 'A3',
            'pin_sck' : 'A0',
            'pin_mosi' : 'A1',
            'pin_miso' : 'A2',
          },
  'BAT' : {
            'pin_charging' : 'C9', # active low
            'pin_voltage' : 'B14',
            'pin_charge_en' : 'V9',
          },
  'TOUCH' : {
            'device' : 'CST816S', 'addr' : 0x15,
            'pin_rst' : 'V14',
          },
  'VIBRATE' : {
            'pin' : 'C6',
            'pin_en' : 'V8'
  },
  'SPEAKER' : {
            'pin' : 'B8',
            'pin_en' : 'V7'
  },
  'ACCEL' : {
            'device' : 'LSM6DSOTR', 'addr' : 106,
            'pin_sda' : 'B5', 'pin_scl' : 'B4' # every other I2C device is on this too
          },
  'MAG' : { # Magnetometer/compass
            'device' : 'MMC5603NJ',
            'addr' : 48
          },
  'PRESSURE' : {
            'device' : 'BME690', # v2.1 uses Goertek SPL06-001 - we handle both
            'addr' : 118, # both versions use the same address
          },
  'MISC' : {
            'pin_torch' : 'V5',
            'pin_rgb_en' : 'V6',
            'pin_rgb_data' : 'C10',
            'pin_wifi' : 'V10',
            'pin_wifi_boot' : 'V11',
            'pin_aux_swap' : 'V12',
            'pin_aux_power' : 'V13',
            'pin_hrm_aux' : 'V15'
  }
};

# left-right, or top-bottom order
board = {
};
board["_css"] = """
""";

def get_pins():
  # GPIO 0/1/2 + virtual pins which come from the PY32 (acting as IO expander)
  pins = pinutils.generate_pins(0,4,"A") + pinutils.generate_pins(0,14,"B") + pinutils.generate_pins(0,10,"C") + pinutils.generate_pins(0,15,"V");
  # pinutils.findpin(pins, "PAxx", True)["functions"]["..."]=0;
  pinutils.findpin(pins, "PB4", True)["functions"]["ADC1_IN0"]=0;
  pinutils.findpin(pins, "PB5", True)["functions"]["ADC1_IN1"]=0;
  pinutils.findpin(pins, "PB6", True)["functions"]["ADC1_IN2"]=0;
  pinutils.findpin(pins, "PB7", True)["functions"]["ADC1_IN3"]=0;
  pinutils.findpin(pins, "PB11", True)["functions"]["ADC1_IN4"]=0;
  pinutils.findpin(pins, "PB12", True)["functions"]["ADC1_IN5"]=0;
  pinutils.findpin(pins, "PB13", True)["functions"]["ADC1_IN6"]=0;
  pinutils.findpin(pins, "PB14", True)["functions"]["ADC1_IN7"]=0;
  # timer/etc that are set in banglejs3_nrf54l15_cpuapp.overlay
  pinutils.findpin(pins, "PB8", True)["functions"]["TIM1_CH1"]=0; # speaker
  pinutils.findpin(pins, "PC6", True)["functions"]["TIM1_CH2"]=0; # vibrate
  # everything is non-5v tolerant
  for pin in pins:
    pin["functions"]["3.3"]=0;
    pin["functions"]["NO_BLOCKLY"]=0;  # hide in blockly
  #The boot/reset button will function as a reset button in normal operation. Pin reset on PD21 needs to be enabled on the nRF52832 device for this to work.
  return pins
