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
 'binary_name' : 'espruino_%v_banglejs3.hex',
 'build' : {
   'libraries' : [
     'BLUETOOTH',
     'GRAPHICS',
     'JIT' # JIT compiler enabled
   ],
   'makefile' : [
     'DEFINES+=-DESPR_OFFICIAL_BOARD', # Don't display the donations nag screen
    # 'DEFINES += -DESPR_HWVERSION=3 -DBANGLEJS -DBANGLEJS3',
     'DEFINES+=-DBLUETOOTH_NAME_PREFIX=\'"Bangle.js"\'',
     'DEFINES+=-DESPR_UNICODE_SUPPORT=1',
     'DEFINES+=-DDUMP_IGNORE_VARIABLES=\'"g\\0"\'',
    #'DEFINES+=-DESPR_GRAPHICS_INTERNAL=1',
     'DEFINES+=-DUSE_FONT_6X8 -DGRAPHICS_PALETTED_IMAGES -DGRAPHICS_ANTIALIAS -DESPR_PBF_FONTS',
     'DEFINES+=-DNO_DUMP_HARDWARE_INITIALISATION', # don't dump hardware init - not used and saves 1k of flash
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
  'usart' : 1,
  'spi' : 0,
  'i2c' : 0,
  'adc' : 0,
  'dac' : 0,
  'saved_code' : {
    #'address' : 1024*1024,
    'page_size' : 4096,
    'flash_available' : 1024,
    # internal flash - debug only
    'pages' : 32,
    'address' : ((245 - 88) * 4096),
    # external flash

    #'address' : 0x60000000, # put this in external spiflash (see below)
    #'pages' : 2048, # Entire 8MB of external flash
  },
};

devices = {
  'SPIFLASH' : {
    'size' : 64*1024*1024, # 64MB
    'memmap_base' : 0x60000000 # map into the address space (in software)
  }
};

# left-right, or top-bottom order
board = {
};
board["_css"] = """
""";

def get_pins():
  # GPIO 0/1/2
  pins = pinutils.generate_pins(0,4,"A") + pinutils.generate_pins(0,14,"B") + pinutils.generate_pins(0,10,"C");
  # pinutils.findpin(pins, "PAxx", True)["functions"]["..."]=0;
  # ...


  # everything is non-5v tolerant
  for pin in pins:
    pin["functions"]["3.3"]=0;
    pin["functions"]["NO_BLOCKLY"]=0;  # hide in blockly
  #The boot/reset button will function as a reset button in normal operation. Pin reset on PD21 needs to be enabled on the nRF52832 device for this to work.
  return pins
