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
 'name' : "Arduino Mega 2560",
 'default_console' : "EV_SERIAL1",
 'variables' : 1023,
 'binary_name' : 'espruino_%v_arduinomega2560',
};
chip = {
  'part' : "ATMEGA2560",
  'family' : "AVR",
  'package' : "",
  'ram' : 32,
  'flash' : 256,
  'speed' : 16,
  'usart' : 1,
  'spi' : 0,
  'i2c' : 0,
  'adc' : 0,
  'dac' : 0,
};
# left-right, or top-bottom order
board = {
};
devices = {
};

board_css = """
""";

def get_pins():
  pins = pinutils.generate_pins(0,7)  
  # just fake pins D0 .. D7
  return pins
