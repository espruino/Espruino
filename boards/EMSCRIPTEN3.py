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
 'name' : "Bangle.js 3 emulator",
 'default_console' : "EV_USBSERIAL",
 'variables' :  12000, # 0 = resizable variables, rather than fixed
 'binary_name' : 'emulator_banglejs3.js',
 'build' : {
   'libraries' : [
#     'NET',
#     'TENSORFLOW', # New emscripten won't compile it!
     'TERMINAL',
     'GRAPHICS',
     'LCD_MEMLCD',
   ],
   'makefile' : [
     'EMSCRIPTEN=1',
     'USE_DEBUGGER=0', # We can't use debugger in emulator as we're single-threaded and it uses IRQs on embedded to work
     'DEFINES += -DESPR_HWVERSION=2',
     'DEFINES += -DUSE_CALLFUNCTION_HACK', # required to handle calls properly
     'DEFINES += -DBANGLEJS -DBANGLEJS3 -DEMULATED -DEMSCRIPTEN',
     'DEFINES += -DSPIFLASH_BASE=0x8000000 -DSPIFLASH_LENGTH=8388608',
#     'DEFINES+=-DCUSTOM_GETBATTERY=jswrap_banglejs_getBattery',
     'DEFINES+=-DESPR_UNICODE_SUPPORT=1',
     'DEFINES+=-DDUMP_IGNORE_VARIABLES=\'"g\\0"\'',
     'DEFINES+=-DESPR_GRAPHICS_INTERNAL=1',
     'DEFINES+=-DESPR_BATTERY_FULL_VOLTAGE=0.3144',
     'DEFINES+=-DUSE_FONT_6X8 -DGRAPHICS_PALETTED_IMAGES -DGRAPHICS_ANTIALIAS -DESPR_PBF_FONTS',
     'DEFINES+=-DNO_DUMP_HARDWARE_INITIALISATION', # don't dump hardware init - not used and saves 1k of flash
     'INCLUDE += -I$(ROOT)/libs/banglejs -I$(ROOT)/libs/misc',
     'WRAPPERSOURCES += libs/banglejs/jswrap_bangle.c',
     'WRAPPERSOURCES += libs/misc/jswrap_emulated.c',
     'WRAPPERSOURCES += libs/graphics/jswrap_font_14.c',
     'WRAPPERSOURCES += libs/graphics/jswrap_font_17.c',
     'WRAPPERSOURCES += libs/graphics/jswrap_font_22.c',
     'WRAPPERSOURCES += libs/graphics/jswrap_font_28.c',
     'WRAPPERSOURCES += libs/graphics/jswrap_font_6x15.c',
     'WRAPPERSOURCES += libs/graphics/jswrap_font_12x20.c',
     'SOURCES += libs/misc/nmea.c',
     'SOURCES += libs/misc/stepcount.c',
     'SOURCES += libs/misc/heartrate.c',
# ------------------------
     'SOURCES += libs/misc/unistroke.c',
     'WRAPPERSOURCES += libs/misc/jswrap_unistroke.c',
     'DEFINES += -DESPR_BANGLE_UNISTROKE=1',
     'SOURCES += libs/banglejs/banglejs2_storage_default.c',
     'DEFINES += -DESPR_STORAGE_INITIAL_CONTENTS=1', # use banglejs2_storage_default
     'DEFINES += -DESPR_USE_STORAGE_CACHE=32', # Add a 32 entry cache to speed up finding files
     'JSMODULESOURCES += libs/js/banglejs/locale.min.js',
     'JSMODULESOURCES += libs/js/banglejs/Layout.min.js',
   ]
 }
};
chip = {
  'part' : "EMSCRIPTEN",
  'family' : "EMSCRIPTEN",
  'package' : "",
  'ram' : 0,
  'flash' : 8192, # size of file used to fake flash memory (kb)
  'speed' : -1,
  'usart' : 1,
  'spi' : 1,
  'i2c' : 1,
  'adc' : 0,
  'dac' : 0,
  'saved_code' : {
    'address' : 0x8000000,
    'page_size' : 4096,
    'pages' : 2048, # Entire 8MB of external flash
    'flash_available' : 1024 # lots - this is fake
  },
};

devices = {
  'USB' : {}, # to convince code that we have a USB port (it's used for the console ion Linux)
  'BTN1' : { 'pin' : 'D1' }, # Pin negated in software
  'BTN2' : { 'pin' : 'D2' }, # Pin negated in software
  'BTN3' : { 'pin' : 'D3' }, # Pin negated in software
  'BTN4' : { 'pin' : 'D4' }, # Pin negated in software
  'LED1' : { 'pin' : 'D5', 'novariable':True }, # torch
  'LCD' : {
            'width' : 240, 'height' : 240,
            'bpp' : 6,
            'controller' : 'ZJ012BD01A', # ZJ012BD-01A
          },
  'TOUCH' : {
            'device' : 'CTS816S', 'addr' : 0x15,
         },
  'VIBRATE' : { 'pin' : 'D19' },
  'BAT' : {
            'pin_charging' : 'D23', # active low
            'pin_voltage' : 'D3'
          },
};

def get_pins():
  pins = pinutils.generate_pins(0,47) # 48 General Purpose I/O Pins.
  # Make buttons negated
  pinutils.findpin(pins, "PD1", True)["functions"]["NEGATED"]=0; # button
  pinutils.findpin(pins, "PD2", True)["functions"]["NEGATED"]=0; # button
  pinutils.findpin(pins, "PD3", True)["functions"]["NEGATED"]=0; # button
  pinutils.findpin(pins, "PD4", True)["functions"]["NEGATED"]=0; # button
  return pins
