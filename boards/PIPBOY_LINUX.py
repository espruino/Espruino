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
 'name' : "Pipboy Linux Debug Compilation",
 'default_console' : "EV_USBSERIAL",
 'variables' :  15000, # 0 = resizable variables, rather than fixed
 'binary_name' : 'espruino_pb',
 'build' : {
   'libraries' : [
     'GRAPHICS','LCD_SDL',
     'FILESYSTEM'
   ],
   'makefile' : [
#     'DEFINES+=-DFLASH_64BITS_ALIGNMENT=1', # For testing 64 bit flash writes
#     'CFLAGS+=-m32', 'LDFLAGS+=-m32', 'DEFINES+=-DUSE_CALLFUNCTION_HACK', # For testing 32 bit builds
     'DEFINES+=-DLINUX -DEMULATED',
     'DEFINES+=-DESPR_UNICODE_SUPPORT=1',
     'DEFINES+=-DESPR_FS_PREPEND_PATH=\'"sdcard/"\'',
     'DEFINES+=-DUSE_FONT_6X8 -DGRAPHICS_PALETTED_IMAGES -DGRAPHICS_ANTIALIAS -DESPR_PBF_FONTS -DESPR_GRAPHICS_INTERNAL -DESPR_GRAPHICS_SELF_INIT',
     'DEFINES+=-DSPIFLASH_BASE=0 -DSPIFLASH_LENGTH=FLASH_SAVED_CODE_LENGTH', # For Testing Flash Strings
     'LINUX=1',
     'INCLUDE += -I$(ROOT)/libs/pipboy',
     'WRAPPERSOURCES += libs/pipboy/avi.c libs/pipboy/stm32_i2s.c',
     'WRAPPERSOURCES += libs/pipboy/jswrap_pipboy.c',
     'WRAPPERSOURCES += libs/pipboy/jswrap_font_monofonto_120.c',
     'WRAPPERSOURCES += libs/pipboy/jswrap_font_monofonto_96.c',
     'WRAPPERSOURCES += libs/pipboy/jswrap_font_monofonto_36.c',
     'WRAPPERSOURCES += libs/pipboy/jswrap_font_monofonto_28.c',
     'WRAPPERSOURCES += libs/pipboy/jswrap_font_monofonto_23.c',
     'WRAPPERSOURCES += libs/pipboy/jswrap_font_monofonto_18.c',
     'WRAPPERSOURCES += libs/pipboy/jswrap_font_monofonto_16.c'
   ]
 }
};
chip = {
  'part' : "LINUX",
  'family' : "LINUX",
  'package' : "",
  'ram' : 0,
  'flash' : 256, # size of file used to fake flash memory (kb)
  'speed' : -1,
  'usart' : 6,
  'spi' : 3,
  'i2c' : 3,
  'adc' : 0,
  'dac' : 0
};

devices = {
  'USB' : {}, # to convince code that we have a USB port (it's used for the console ion Linux)
    'LCD' : {
            'width' : 480, 'height' : 320, 'bpp' : 16, 'controller' : 'sdl'
            }
};

def get_pins():
  pins = pinutils.generate_pins(0,32)
  # just fake pins D0 .. D32
  return pins
