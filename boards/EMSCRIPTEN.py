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
#     'TENSORFLOW',
     'GRAPHICS',
#     'FILESYSTEM',
#     'CRYPTO','SHA256','SHA512',
#     'TLS',
#     'TELNET',
   ],
   'makefile' : [
     'EMSCRIPTEN=1',
#     'DEFINES += -DUSE_TENSORFLOW',
#     'DEFINES+=-DCUSTOM_GETBATTERY=jswrap_banglejs_getBattery',
     'DEFINES+=-DDUMP_IGNORE_VARIABLES=\'"g\\0"\'',
     'DEFINES+=-DUSE_FONT_6X8 -DGRAPHICS_PALETTED_IMAGES -DUSE_LCD_EMSCRTIPTEN',
     'INCLUDE += -I$(ROOT)/libs/emscripten',
     'WRAPPERSOURCES += libs/emscripten/jswrap_emscripten.c',
     'SOURCES += libs/graphics/lcd_emscripten.c',
     'JSMODULESOURCES += libs/js/graphical_menu.min.js',
   ]
 }
};
chip = {
  'part' : "EMSCRIPTEN",
  'family' : "EMSCRIPTEN",
  'package' : "",
  'ram' : 0,
  'flash' : 256, # size of file used to fake flash memory (kb)
  'speed' : -1,
  'usart' : 1,
  'spi' : 1,
  'i2c' : 1,
  'adc' : 0,
  'dac' : 0,
};

devices = {
  'USB' : {} # to convince code that we have a USB port (it's used for the console ion Linux)
};

def get_pins():
  pins = pinutils.generate_pins(0,32)
  # just fake pins D0 .. D32
  return pins
