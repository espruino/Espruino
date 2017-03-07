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
import json;
info = {
 'name' : "Haoyu HY-TinySTM103T",
 'link' :  [ "http://www.hotmcu.com/stm32f103tb-arm-cortex-m3-development-board-p-222.html" ],
 'variables' : 1020,
 'binary_name' : 'espruino_%v_hytiny_stm103t.bin',
 'build' : {
   'optimizeflags' : '-Os',
   'libraries' : [
     'GRAPHICS'
   ],
   'makefile' : [
     'SAVE_ON_FLASH=1',
     'STLIB=STM32F10X_MD',
     'PRECOMPILED_OBJS+=$(ROOT)/targetlibs/stm32f1/lib/startup_stm32f10x_md.o'
   ]
 }
};

chip = {
  'part' : "STM32F103TBT6",
  'family' : "STM32F1",
  'package' : "VFQFPN36",
  'ram' : 20,
  'flash' : 128,
  'speed' : 72,
  'usart' : 2,
  'spi' : 1,
  'i2c' : 1,
  'adc' : 2,
  'dac' : 0,
  'saved_code' : {
    'address' : 0x08000000 + ((128-6)*1024),
    'page_size' : 1024, # size of pages
    'pages' : 6, # number of pages we're using
    'flash_available' : 128-6 # 6 used for code
  },
};

devices = {
# 'OSC' : { 'pin_1' :  'D0',
#           'pin_2' : 'D1' },
  'LED1' : { 'pin' : 'A1' },
  'USB' : { 'pin_disc' : 'A0',
            'pin_dm' : 'A11',
            'pin_dp' : 'A12'
          },
};

# left-right, or top-bottom order
board = {
  'left' : [ '3.3','A3','A4','A5','A6','A7','B0','B1','B2','A8','A9','A10','A11','A12','GND' ],
  'right' : [ 'GND','A2','A1','A0','RST','ISP','A14','A13','B7','B6','B5','B4','B3','A15','5V' ],
  'bottom' : [ '3.3','A9','A10','A13/SWDIO','A14/SWCLK','GND' ],
};
board["_css"] = """
#board {
  width: 290px;
  height: 533px;
  left: 300px;
  background-image: url(img/HYTINY_STM103T.jpg);
}
#boardcontainer {
  height: 730px;
}
#left {
  top: 20px;
  right: 290px;
}
#right {
  top: 20px;
  left: 290px;
}
#bottom {
  top: 540px;
  left: 45px;
}
.leftpin { height: 32px; }
.rightpin { height: 32px; }
.bottompin { width: 28.5px; }
""";

def get_pins():
  pins = pinutils.scan_pin_file([], 'stm32f103xb.csv', 6, 10, 11)
  return pinutils.only_from_package(pinutils.fill_gaps_in_pin_list(pins), chip["package"])
