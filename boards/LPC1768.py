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
# placeholder
info = {
 'name' : "MBed LPC1768",
 'link' : [ "http://www.espruino.com/kick" ],
 'variables' : 2000, #?
 'binary_name' : 'espruino_%v_lpc1768.bin',
};
chip = {
  'part' : "LPC1768",
  'family' : "LPC1768",
#  'package' : "LQFP64",
#  'ram' : ,
#  'flash' : ,
#  'speed' : ,
#  'usart' : ,
#  'spi' : ,
#  'i2c' : ,
#  'adc'
#  'dac'
};
# left-right, or top-bottom order
board = {
};
devices = {
};

board_css = """
""";

def get_pins():
  return []
