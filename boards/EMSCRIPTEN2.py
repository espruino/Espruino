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
 'name' : "Bangle.js 2 emulator",
 'default_console' : "EV_USBSERIAL",
 'variables' :  12000, # 0 = resizable variables, rather than fixed
 'binary_name' : 'emulator_banglejs2.js',
 'build' : {
   'libraries' : [
#     'NET',
     'TENSORFLOW',
     'TERMINAL',
     'GRAPHICS',
     'LCD_MEMLCD',
#     'FILESYSTEM',
     'CRYPTO','SHA256','SHA512',
#     'TLS',
#     'TELNET',
   ],
   'makefile' : [
     'EMSCRIPTEN=1',
     'DEFINES += -DESPR_HWVERSION=2',
     'DEFINES += -DUSE_CALLFUNCTION_HACK', # required to handle calls properly
     'DEFINES += -DBANGLEJS -DBANGLEJS_Q3 -DEMULATED -DEMSCRIPTEN',
     'DEFINES += -DSPIFLASH_BASE=0x8000000 -DSPIFLASH_LENGTH=8388608',
#     'DEFINES+=-DCUSTOM_GETBATTERY=jswrap_banglejs_getBattery',
     'DEFINES+=-DDUMP_IGNORE_VARIABLES=\'"g\\0"\'',
     'DEFINES+=-DUSE_FONT_6X8 -DGRAPHICS_PALETTED_IMAGES',
     'INCLUDE += -I$(ROOT)/libs/banglejs -I$(ROOT)/libs/misc',
     'WRAPPERSOURCES += libs/banglejs/jswrap_bangle.c',
     'WRAPPERSOURCES += libs/graphics/jswrap_font_6x15.c',
     'WRAPPERSOURCES += libs/misc/jswrap_emulated.c',
     'SOURCES += libs/misc/nmea.c',
     'SOURCES += libs/misc/stepcount.c',
     'SOURCES += libs/misc/heartrate.c',
     'JSMODULESOURCES += libs/js/banglejs/locale.min.js',
     'SOURCES += libs/banglejs/banglejs2_storage_default.c',
     'DEFINES += -DESPR_STORAGE_INITIAL_CONTENTS=1',
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
  'BAT' : {
            'pin_charging' : 'D23', # active low
            'pin_voltage' : 'D3'
          },
};

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
  return pins
