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
  'left' : [ 'B5', 'B4', 'B3' ],
  'left2' : [ '', 'A8', 'B10' ],
  'top' : [ 'VBAT', 'GND', 'B15','B14','B13','A7', 'A6', 'A5' ],
  'bottom' : [ 'B6', 'B7', 'B8','B9', 'A0', 'A1', '3.3', 'GND' ],

  'right' : ['','','','','','','','','',''],
  'right2' : ['','','','','','','','','',''],
        
};
devices = {
  'OSC' : { 'pin_in' :  'H0', # checked
            'pin_out' : 'H1' }, # checked
  'OSC_RTC' : { 'pin_in' :  'C14', # checked
                'pin_out' : 'C15' }, # checked
  'LED1' : { 'pin' : 'A13' },
  'LED2' : { 'pin' : 'A14' },
  'BTN1' : { 'pin' : 'B12' },
  'USB' : { 'pin_disc' :  'C13',
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
  width: 714px;
  height: 338px;
  top: 200px;
  left : 100px;
  background-image: url(img/ESPRUINIBOARD_R1_0.png);
}
#boardcontainer {
  height: 585px;
}
#left {
  top: 105px;
  right: 500px;  
}
#left2  {
  top: 105px;
  left: 303px;
}
#top {
  bottom: 320px;
  left: 210px;
}
#bottom {
  top: 320px;
  left: 210px;
}

#right  {
  top: 35px;
  left: 920px;
}
#right2  {
  top: 35px;
  right: -205px;
}

.leftpin { height: 48px; }
.left2pin { height: 48px; }
.toppin { width: 48px; }
.bottompin { width: 48px; }

.rightpin { height: 25px; }
.right2pin { height: 25px; }

""";

def get_pins():
  pins = pinutils.scan_pin_file([], 'stm32f401.csv', 5, 8, 9)
  return pinutils.only_from_package(pinutils.fill_gaps_in_pin_list(pins), chip["package"])
