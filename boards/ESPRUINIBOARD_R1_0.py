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
 'name' : "Espruini Board rev 1.0",
 'link' : [ "http://www.espruino.com/EspruinoBoard" ],
 'variables' : 1800,
 'bootloader' : 1,
 'serial_bootloader' : True,
 'binary_name' : 'espruino_%v_espruini_1r0.bin',
};
chip = {
  'part' : "STM32F401CCU6",
  'family' : "STM32F4",
  'package' : "UQFN48", # UFQFPN48
  'ram' : 64,
  'flash' : 256,
  'speed' : 84,
  'usart' : 3,
  'spi' : 3,
  'i2c' : 3,
  'adc' : 1,
  'dac' : 0,
};
# left-right, or top-bottom order
board = {
  'left' : [ 'GND', 'A10', 'A9', 'A8' ],
#  'left2' : [ 'GND', 'A12', 'A11', 'VCC' ],
  'top' : [ '3.3', 'GND', 'A13', 'A14', 'A15', 'B10', 'B3', 'B4', 'B5', 'B6', 'B7'],
  'bottom' : [ 'VBAT','GND','B15', 'B14', 'B13', 'B2', 'B0', 'A7', 'A6', 'A5','A4' ], 


  'right2' : ['B9','','A1','A3'],
  'right' : ['B8','C13','A0','A2'],        
};
devices = {
  'OSC' : { 'pin_in' :  'H0', # checked
            'pin_out' : 'H1' }, # checked
  'OSC_RTC' : { 'pin_in' :  'C14', # checked
                'pin_out' : 'C15' }, # checked
  'LED1' : { 'pin' : 'B1' },
  'USB' : { 'pin_vbus' :  'B12',
            'pin_dm' : 'A11',   # checked
            'pin_dp' : 'A12' }, # checked
  'JTAG' : {
        'pin_MS' : 'A13',
        'pin_CK' : 'A14', 
        'pin_DI' : 'A15' 
          }
};

board_css = """
#board {
  width: 835px;
  height: 365px;
  top: 200px;
  left : 100px;
  background-image: url(img/ESPRUINIBOARD_R1_0.png);
}
#boardcontainer {
  height: 585px;
}
#left {
  top: 105px;
  right: 560px;  
}
#left2  {
  top: 105px;
  left: -100px;
}
#top {
  bottom: 320px;
  left: 240px;
}
#bottom {
  top: 320px;
  left: 240px;
}

#right  {
  top: 105px;
  left: 820px;
}
#right2  {
  top: 105px;
  right: 105px;
}

.leftpin { height: 48px; }
.left2pin { height: 48px; }
.toppin { width: 48px; }
.bottompin { width: 48px; }

.rightpin { height: 48px; }
.right2pin { height: 48px; }

""";

def get_pins():
  pins = pinutils.scan_pin_file([], 'stm32f401.csv', 5, 8, 9)
  return pinutils.only_from_package(pinutils.fill_gaps_in_pin_list(pins), chip["package"])
