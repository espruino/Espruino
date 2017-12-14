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
 'name' : "STM32 VL Discovery",
 'link' :  [ "http://www.st.com/stm32-discovery" ],
 'variables' : 500,
 'binary_name' : 'espruino_%v_stm32vldiscovery.bin',
 'build' : {
   'optimizeflags' : '-Os',
   'libraries' : [
     'NEOPIXEL'
   ],
   'makefile' : [
     'SAVE_ON_FLASH=1',
     'STLIB=STM32F10X_MD_VL',
     'PRECOMPILED_OBJS+=$(ROOT)/targetlibs/stm32f1/lib/startup_stm32f10x_md_vl.o'
   ]
 }
};
chip = {
  'part' : "STM32F100RBT6",
  'family' : "STM32F1",
  'package' : "LQFP64",
  'ram' : 8,
  'flash' : 128,
  'speed' : 24,
  'usart' : 3,
  'spi' : 2,
  'i2c' : 2,
  'adc' : 3,
  'dac' : 0,
};
devices = {
  'OSC' : { 'pin_1' :  'D0',
            'pin_2' : 'D1' },
  'OSC_RTC' : { 'pin_1' :  'C14',
                'pin_2' : 'C15' },
  'LED1' : { 'pin' : 'C9' },
  'LED2' : { 'pin' : 'C8' },
  'BTN1' : { 'pin' : 'A0' },
  'JTAG' : {
        'pin_MS' : 'A13',
        'pin_CK' : 'A14',
        'pin_DI' : 'A15'
          },
};

# left-right, or top-bottom order
board = {
  'left' : [ 'GND', 'NC', '3.3', 'VBAT', 'C13', 'C14', 'C15', 'D0', 'D1', 'RST', 'C0', 'C1', 'C2', 'C3', 'A0', 'A1', 'A2', 'A3', 'A4', 'A5', 'A6', 'A7', 'C4', 'C5', 'B0', 'B1', 'B2', 'GND' ],
  'right' : [ 'GND', 'NC', '5V', 'B9', 'B8', 'BOOT', 'B7', 'B6', 'B5', 'B4', 'B3', 'D2', 'C12', 'C11', 'C10', 'A15', 'A14', 'A13', 'A12', 'A11', 'A10', 'A9', 'A8', 'C9', 'C8', 'C7', 'C6', 'GND' ],
  'bottom' : [ 'B10','B11','B12','B13','B14','B15' ],
};
board["_css"] = """
#board {
  width: 376px;
  height: 750px;
  left: 200px;
  background-image: url(img/STM32VLDISCOVERY.jpg);
}
#boardcontainer {
  height: 950px;
}
#left {
  top: 40px;
  right: 330px;
}
#right  {
  top: 40px;
  left: 330px;
}
#bottom  {
  top: 710px;
  left: 125px;
}
""";

def get_pins():
  pins = pinutils.scan_pin_file([], 'stm32f103xb.csv', 6, 10, 11)
  return pinutils.only_from_package(pinutils.fill_gaps_in_pin_list(pins), chip["package"])
