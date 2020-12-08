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
 'name' : "Compile for JS",
 'default_console' : "EV_USBSERIAL",
 'variables' :  2500, # 0 = resizable variables, rather than fixed
 'binary_name' : 'emulator_espruino.js',
 'build' : {
   'libraries' : [
#     'NET',
     'TENSORFLOW',
     'TERMINAL',
     'GRAPHICS',
     'LCD_ST7789_8BIT',
#     'FILESYSTEM',
#     'CRYPTO','SHA256','SHA512',
#     'TLS',
#     'TELNET',
   ],
   'makefile' : [
     'EMSCRIPTEN=1',
     'DEFINES += -DUSE_TENSORFLOW',
     'DEFINES += -DBANGLEJS',
     'DEFINES += -DSPIFLASH_BASE=0x8000000 -DSPIFLASH_LENGTH=4194304',
#     'DEFINES+=-DCUSTOM_GETBATTERY=jswrap_banglejs_getBattery',
     'DEFINES+=-DDUMP_IGNORE_VARIABLES=\'"g\\0"\'',
     'DEFINES+=-DUSE_FONT_6X8 -DGRAPHICS_PALETTED_IMAGES -DGRAPHICS_ANTIALIAS -DUSE_LCD_EMSCRTIPTEN',
     'INCLUDE += -I$(ROOT)/libs/banglejs -I$(ROOT)/libs/misc',
     'WRAPPERSOURCES += libs/banglejs/jswrap_bangle.c',
     'SOURCES += libs/misc/nmea.c',
     'JSMODULESOURCES += libs/js/banglejs/locale.min.js'
   ]
 }
};
chip = {
  'part' : "EMSCRIPTEN",
  'family' : "EMSCRIPTEN",
  'package' : "",
  'ram' : 0,
  'flash' : 4096, # size of file used to fake flash memory (kb)
  'speed' : -1,
  'usart' : 1,
  'spi' : 1,
  'i2c' : 1,
  'adc' : 0,
  'dac' : 0,
  'saved_code' : {
    'address' : 0x8000000,
    'page_size' : 4096,
    'pages' : 1024, # Entire 4MB of external flash
    'flash_available' : 1024 # lots - this is fake
  },
};

devices = {
  'USB' : {}, # to convince code that we have a USB port (it's used for the console ion Linux)
  'BTN1' : { 'pin' : 'D24', 'pinstate' : 'IN_PULLDOWN' }, # top
  'BTN2' : { 'pin' : 'D22', 'pinstate' : 'IN_PULLDOWN' }, # middle
  'BTN3' : { 'pin' : 'D23', 'pinstate' : 'IN_PULLDOWN' }, # bottom
  'BTN4' : { 'pin' : 'D11', 'pinstate' : 'IN_PULLDOWN' }, # touch left
  'BTN5' : { 'pin' : 'D16', 'pinstate' : 'IN_PULLDOWN' }, # touch right
  'VIBRATE' : { 'pin' : 'D13' },
  'SPEAKER' : { 'pin' : 'D18' },
  'LCD' : {
            'width' : 240, 'height' : 240, 'bpp' : 16,
            'controller' : 'st7789_8bit' # 8 bit parallel mode
          },
  'BAT' : {
            'pin_charging' : 'D12', # active low, input pullup
            'pin_voltage' : 'D30'
          },
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
  return pins
