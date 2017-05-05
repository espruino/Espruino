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
 'name' : "Original Espruino Board rev 1.3/1.4",
 'link' : [ "http://www.espruino.com/EspruinoBoard" ],
 'espruino_page_link' : "EspruinoBoard",
 'default_console' : "EV_SERIAL1",
 'default_console_tx' : "A9",
 'default_console_rx' : "A10",
 'variables' : 2240,
 'bootloader' : 1,
 'serial_bootloader' : True,
 'binary_name' : 'espruino_%v_espruino_1r3.bin',
 'binaries' : [
  { 'filename' : 'espruino_%v_espruino_1r3_wiznet.bin', 'description' : "WIZNet W5500 Ethernet Networking"},
  { 'filename' : 'espruino_%v_espruino_1r3.bin', 'description' : "TI CC3000 WiFi Networking"},
 ],
 'build' : {
   'optimizeflags' : '-Os',
   'libraries' : [
     'NET',
     'GRAPHICS',
     'NEOPIXEL',
     'HASHLIB',
     'TV',
     'FILESYSTEM'
   ],
   'makefile' : [
     'DEFINES+=-DESPRUINO_1V3',
     'STLIB=STM32F10X_XL',
     'PRECOMPILED_OBJS+=$(ROOT)/targetlibs/stm32f1/lib/startup_stm32f10x_hd.o'
   ]
  }
};
chip = {
  'part' : "STM32F103RCT6",
  'family' : "STM32F1",
  'package' : "LQFP64",
  'ram' : 48,
  'flash' : 256,
  'speed' : 72,
  'usart' : 5,
  'spi' : 3,
  'i2c' : 2,
  'adc' : 3,
  'dac' : 2,
  'saved_code' : {
    'address' : 0x08000000 + ((256-20)*1024),
    'page_size' : 2048, # size of pages
    'pages' : 10, # number of pages we're using
    'flash_available' : 256-(20+10) # 20 used for code, 10 for bootloader
  }
};
devices = {
  'OSC' : { 'pin_in' :  'D0',
            'pin_out' : 'D1' },
  'OSC_RTC' : { 'pin_in' :  'C14',
                'pin_out' : 'C15' },
  'LED1' : { 'pin' : 'A13' },
  'LED2' : { 'pin' : 'A14' },
  'LED3' : { 'pin' : 'A15' },
  'BTN1' : { 'pin' : 'B12' },
  'USB' : { 'pin_disc' :  'C13',
            'pin_dm' : 'A11',
            'pin_dp' : 'A12' },
  'SD' :  { 'pin_cs' :  'D2',
            'pin_di' :  'B15',
            'pin_do' :  'B14',
            'pin_clk' :  'B13' },
  'BLUETOOTH' : { 'pin_tx' : 'A9',
                  'pin_rx' : 'A10' },
};

# left-right, or top-bottom order
board = {
  'right' : [ 'A15', 'A14', 'A13', 'A10', 'A9', 'A8', 'C11', 'C10', 'C9', 'C8', 'C7', 'C6', 'C5', 'C4', 'B15', 'B14', 'B13', '3.3', 'VBAT', 'GND' ],
  'left' : [ 'B2', 'B3', 'B4', 'B5', 'B6', 'B7', 'B8', 'B9', 'B12', 'C12', 'C15', 'C0', 'C1', 'C2', 'C3', 'A0', 'A1', '3.3', 'VBAT', 'GND' ],
  'bottom' : [ 'A2', 'A3', 'A4', 'A5', 'A6', 'A7', 'B0', 'B1', 'B10', 'B11' ],

  '_notes' : {
    'B4' : "The timers on pins B4 and B5 overlap with A6,A7,B0 and B1 (so can't be used at the same time) - to get the maximum amount of PWM outputs we'd recommend that you don't use B4 and B5 with analogWrite.",
    'B5' : "The timers on pins B4 and B5 overlap with A6,A7,B0 and B1 (so can't be used at the same time) - to get the maximum amount of PWM outputs we'd recommend that you don't use B4 and B5 with analogWrite.",
  }
};
board["_css"] = """
#board {
  width: 431px;
  height: 585px;
  left: 300px;
  background-image: url(img/ESPRUINOBOARD.jpg);
}
#boardcontainer {
  height: 585px;
}
#left {
  top: 45px;
  right: 431px;
}
#right  {
  top: 45px;
  left: 431px;
}
#bottom {
  top: 280px;
  left: 124px;
}
.leftpin { height: 24px; }
.rightpin { height: 24px; }

""";

def get_pins():
  pins = pinutils.scan_pin_file([], 'stm32f103xe.csv', 6, 10, 11)
  return pinutils.only_from_package(pinutils.fill_gaps_in_pin_list(pins), chip["package"])
