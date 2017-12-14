#!/bin/false
# -*- coding: utf8 -*-
# This file is part of Espruino, a JavaScript interpreter for Microcontrollers
#
# Copyright (C) 2013 Gordon Williams <gw@pur3.co.uk>
# Copyright (C) 2017 ST for STem Cell specific lines of this file
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
 'name' : "STM32L496 STem Cell",
 'link' :  [""],			### No link since not yet published
 'default_console' : "EV_USBSERIAL",

 'variables' :  15359, 			# variables computes from the RAM size : (256-16)*1024/16-1
 'binary_name' : 'espruino_%v_stm32l496STemCell.bin',
 'build' : {
   'optimizeflags' : '-O3',
   'libraries' : [
     'NET',
     'GRAPHICS',
     'NEOPIXEL',
     'CRYPTO',
     'TLS'
   ],
   'makefile' : [
     'DEFINES+=-DUSE_USB_OTG_FS=1',
     'STLIB=STM32L496xx',
     'PRECOMPILED_OBJS+=$(ROOT)/targetlibs/stm32l4/lib/CMSIS/Device/ST/STM32L4xx/Source/Templates/gcc/startup_stm32l496xx.o'
   ]
  }
};

chip = {
  'part' : "STM32L496RG",
  'family' : "STM32L4",
  'package' : "BGA132",
  'ram' : 320,
  'flash' : 1024,
  'speed' : 80,				#core frequency up to 80 MHz
  'usart' : 3,
  'spi' : 4,
  'i2c' : 3,
  'adc' : 3,
  'dac' : 2,
  'saved_code' : {
    # code size 300000 = 0x493E0 starts at 0x0800 0000 ends at 0x0804 93E0
    # so we have some left room for Espruino firmware and no risk to clear it while saving
    'address' : 0x08050000, # flash_saved_code_start 0x080493E0 to 0x8100000
    # we have enough flash space in this single flash page to save all of the ram
    'page_size' :  2048, # size of pages : on STM32F411, last 2 pages are 128 Kbytes
    # we use the last flash page only, furthermore it persists after a firmware flash of the board
    'pages' : 352, # count of pages we're using to save RAM to Flash,
    'flash_available' : 704 # binary will have a hole in it, so we just want to test against full size
  },
};

devices = {
  'OSC' : { 'pin_1' : 'H0', 	#not connected
            'pin_2' : 'H1' }, 	#not connected
  'OSC_RTC' : { 'pin_1' : 'C14', # OSC32_IN (32kHz oscillator)
                'pin_2' : 'C15' },# OSC32_OUT
  'LED1' : { 'pin' : 'C7' },
  'JTAG' : {
        'pin_MS' : 'A13',
        'pin_CK' : 'A14',
        'pin_DI' : 'A15'
          },
  'USB' : { 'pin_charge' :  'A10',
            'pin_vsense' :  'A9',
            'pin_dm' : 'A11',
            'pin_dp' : 'A12' },
};


# left-right, or top-bottom order
board = {
  'left' :   [ 'D0', 'D4', 'D3', 'D1', 'GND', 'U5V', 'A14', 'B15', 'B14', 'A13' ],
  'right' :  [ 'D0', 'D4', 'D3', 'D1', 'GND', 'U5V', 'B3', 'C2', 'C4', 'B4' ],
  'bottom' :  [ 'E12', 'E15', 'E14', 'E13', 'GND', 'U5V', 'E10', 'A7', 'A6', 'C1' ],
  'bottom2' :  [ 'A15', 'A8', 'C11', 'C10', 'U5V', 'GND', 'C12', 'C8', 'C9', 'D2' ],
};
board["_css"] = """
#board {
  width: 713px;
  height: 800px;
  left: 200px;
  background-image: url(img/STEMCELL-L496.jpg);
}
#boardcontainer {
  height: 1020px;
}
#left {
  top: 310px;
  right: 640px;
}
#right  {
  top: 310px;
  left: 615px;
}
#bottom  {
  top: 310px;
  right: 105px;
}
#bottom2  {
  top: 310px;
  right: 105px;
}
""";

def get_pins():
  pins = pinutils.scan_pin_file([], 'stm32l496.csv', 6, 10, 11)
  pins = pinutils.scan_pin_af_file(pins, 'stm32l496_af.csv', 0, 1)
  return pinutils.only_from_package(pinutils.fill_gaps_in_pin_list(pins), chip["package"])
