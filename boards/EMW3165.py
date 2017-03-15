#!/bin/false
# -*- coding: utf8 -*-
# This file is part of Espruino, a JavaScript interpreter for Microcontrollers
#
# Copyright (C) 2013 Gordon Williams <gw@pur3.co.uk>
# Copyright (C) 2014 Alain Sézille for NucleoF411RE specific lines of this file
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

# ***** WARNING: The support for the EMW3165 is in development, WIFI in particular does
# *****          *not* work yet.

import pinutils;
info = {
  'name' : "EMW3165",
  'link' :  [ "http://www.mxchip.com/wireless/downloadcenter/wifi/32.html"],
  'default_console' : "EV_SERIAL2", # USART2 by default, the WifiMCU has the USB on that
  'default_console_tx' : "A2", # USART2_TX on PA2,
  'default_console_rx' : "A3", # USART2_RX on PA3
  'variables' :  2048, # was 7423: (128-12)*1024/16-1
  'binary_name' : 'espruino_%v_emw3165.bin',
 'build' : {
   'optimizeflags' : '-O2',
   'libraries' : [
   ],
   'makefile' : [
     'DEFINES+=-DPIN_NAMES_DIRECT -DHSE_VALUE=26000000UL',
     'STLIB=STM32F411xE',
     'PRECOMPILED_OBJS+=$(ROOT)/targetlibs/stm32f4/lib/startup_stm32f40_41xxx.o'
   ]
  }
};

chip = {
  'part' : "STM32F411CE",
  'family' : "STM32F4",
  'package' : "UQFN48",
  'ram' : 128, # 0x0001 8000 long, from 0x2000 0000 to 0x2001 7FFF
  'flash' : 512, # 0x0008 0000 long, from 0x0800 0000 to 0x0807 FFFF
  'speed' : 100,
  'usart' : 3,
  'spi' : 4,
  'i2c' : 3,
  'adc' : 1,
  'dac' : 0,
  'saved_code' : {
    # code size 225248 = 0x36FE0 starts at 0x0800 0000 ends at 0x0803 6FE0
    # so we have some left room for Espruino firmware and no risk to clear it while saving
    'address' : 0x08060000, # flash_saved_code_start 0x0806 0000 to 0x807 5000
    # we have enough flash space in this single flash page to save all of the ram
    'page_size' :  131072, # size of pages : on STM32F411, last 2 pages are 128 Kbytes
    # we use the last flash page only, furthermore it persists after a firmware flash of the board
    'pages' : 1, # count of pages we're using to save RAM to Flash,
    'flash_available' : 512 # binary will have a hole in it, so we just want to test against full size
  },
  #'place_text_section' : 0x08010000, # note flash_available above # TODO USELESS
};
devices = {
  'LED1' : {'pin': 'A4'},
};

# left-right, or top-bottom order
board = {
  'bottom' :   [ 'ANT', 'GND', 'NC', 'NC', 'NC', 'A14', 'A13', 'A12', 'NC', 'A10', 'B6', 'B8', 'NC',
      'B13', 'A5', 'A11', 'B1', 'B0', 'A4', 'VDD', 'VDD'],
  'top' :  [ 'NC', 'B2', 'NC', 'A7', 'A15', 'B3', 'B4', 'A2', 'A1', 'VBAT', 'NC', 'A3',
      'NRST', 'A0', 'NC', 'C13', 'B10', 'B9', 'B12', 'GND'],
};
board["top"].reverse()
board["_css"] = """
#board {
  width:  830px;
  height: 850px;
  left: 50px;
  top: 300px;
  background-image: url(img/emw3165.jpg);
}
#boardcontainer {
  height: 1400px;
}
#top {
  top: 0px;
  left: 245px;
}
#bottom  {
  top: 835px;
  left: 230px;
}
.toppin, .bottompin {
  margin: 0px;
  padding: 0px;
}
""";

def get_pins():
  pins = pinutils.scan_pin_file([], 'stm32f401.csv', 5, 8, 9)
  pins = pinutils.scan_pin_af_file(pins, 'stm32f401_af.csv', 0, 1)
  return pinutils.only_from_package(pinutils.fill_gaps_in_pin_list(pins), chip["package"])
