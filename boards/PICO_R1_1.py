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
 'name' : "Espruino Pico rev 1.1",
 'link' : [ "http://www.espruino.com/Pico" ],
 'default_console' : "EV_SERIAL1",
 'default_console_tx' : "B6",
 'default_console_rx' : "B7",
 'variables' : 3040,
 'binary_name' : 'espruino_%v_pico_1r1.bin',
};
chip = {
  'part' : "STM32F401CCU6",
  'family' : "STM32F4",
  'package' : "UQFN48", 
  'ram' : 64,
  'flash' : 256,
  'speed' : 84,
  'usart' : 6,
  'spi' : 3,
  'i2c' : 3,
  'adc' : 1,
  'dac' : 0,
  'saved_code' : {
    'address' : 0x08004000,
    'page_size' : 16384, # size of pages
    'pages' : 3, # number of pages we're using
    'flash_available' : 256 # binary will have a hole in it, so we just want to test against full size
  },
  'place_text_section' : 0x00010000, # note flash_available above
};
# left-right, or top-bottom order
board = {
  'top' : [ 'GND', '5V', 'VDD', 'B3', 'B4', 'B5', 'B6', 'B7'],
  'bottom' : [ 'BAT_IN','B15', 'B14', 'B13', 'B10', 'B1', 'A7', 'A5' ], 


  'right2' : ['B8','GND','A0','A1','A3','B2'],
  'right' : ['B9','A10','A8','A2','A4','A6'],        
};
devices = {
  'OSC' : { 'pin_in' :  'H0', # checked
            'pin_out' : 'H1' }, # checked
  'OSC_RTC' : { 'pin_in' :  'C14', # checked
                'pin_out' : 'C15' }, # checked
  'BTN1' : { 'pin' : 'C13', 'pinstate' : 'IN_PULLDOWN' }, 
  'LED1' : { 'pin' : 'B12' }, 
  'LED2' : { 'pin' : 'B2' },
  'USB' : { 'pin_charge' :  'B0',
            'pin_vsense' :  'A9',
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
  width: 507px;
  height: 256px;
  top: 300px;
  left : 100px;
  background-image: url(img/PICO_R1_1.png);
}
#boardcontainer {
  height: 800px;
}
#top {
  bottom: 240px;
  left: 148px;
}
#bottom {
  top: 270px;
  left: 148px;
}

#right  {
  top: 30px;
  left: 600px;
}
#right2  {
  top: 30px;
  right: -80px;
}

.toppin { width: 32px; }
.bottompin { width: 32px; }

.rightpin { height: 34px; }
.right2pin { height: 34px; }

""";

def get_pins():
  pins = pinutils.scan_pin_file([], 'stm32f401.csv', 5, 8, 9)
  pins = pinutils.scan_pin_af_file(pins, 'stm32f401_af.csv', 0, 1)
  return pinutils.only_from_package(pinutils.fill_gaps_in_pin_list(pins), chip["package"])
