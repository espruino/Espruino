#!/bin/false
# This file is part of Espruino, a JavaScript interpreter for Microcontrollers
#
# Copyright (C) 2015 Gordon Williams <gw@pur3.co.uk>
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
# placeholder
info = {
 'name' : "DO-003 watch",
 'link' : [ "http://forum.espruino.com/conversations/280747" ],
  # Very experimental firmware for DO 003 fitness watch
 'espruino_page_link' : '',
 'default_console' : "EV_SERIAL1",
 'default_console_tx' : "D18", # pin 24
 'default_console_rx' : "D17", # pin 25
 'default_console_baudrate' : "9600",
 'variables' : 350,
 'binary_name' : 'espruino_%v_do.bin',
 'build' : {
   'optimizeflags' : '-Os',
   'libraries' : [
     'BLUETOOTH',
     'GRAPHICS',
   ],
   'makefile' : [
     'SAVE_ON_FLASH=1',
     'DEFINES+=-DUSE_DEBUGGER -DUSE_TAB_COMPLETE',
   ]
 }
};
chip = {
  'part' : "NRF51822",
  'family' : "NRF51",
  'package' : "QFN48",
  'ram' : 16,
  'flash' : 256,
  'speed' : 96,
  'usart' : 1,
  'spi' : 1,
  'i2c' : 1,
  'adc' : 0,
  'dac' : 0,
   # If using DFU bootloader, it sits at 0x3C000 - 0x40000 (0x40000 is end of flash)
   # Might want to change 256 -> 240 in the code below
  'saved_code' : {
    'address' : ((256 - 3) * 1024),
    'page_size' : 1024,
    'pages' : 3,
    'flash_available' : (256 - (96 + 3)) # softdevice + saved code
  }
};

devices = {

  'LCD' : {
            'width' : 64, 'height' : 32, 'bpp' : 1, 'controller' : 'ssd1306',
            'pin_mosi' : 'D29',
            'pin_sck' : 'D30',
            'pin_dc' : 'D0',
            'pin_rst' : 'D1',
            'pin_cs' : 'D2'
          },
  'BTN1' : { 'pin' : 'D4', 'pinstate' : 'IN_PULLUP' },
  'LED1' : { 'pin' : 'D7' },  # no LED - actually vibrate!
};

# left-right, or top-bottom order
board = {

};


def get_pins():
  pins = pinutils.generate_pins(0,31) # 32 General Purpose I/O Pins.
  return pins
