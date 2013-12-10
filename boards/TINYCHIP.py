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
 'name' : "Single Chip",
 'link' :  [ "http://www.st.com/stm32-discovery" ],
 'variables' : 700,
 'binary_name' : 'espruino_%v_stm32f103tb.bin',
};
chip = {
  'part' : "STM32F103TBU6",
  'family' : "STM32F1",
  'package' : "VFQFPN36",
  'ram' : 20,
  'flash' : 128,
  'speed' : 24,
  'usart' : 3,
  'spi' : 2,
  'i2c' : 2,
};
# left-right, or top-bottom order
board = {
};

devices = {
  'OSC' : { 'pin_1' :  'D0',
            'pin_2' : 'D1' },
  'LED1' : { 'pin' : 'C9' },
  'BTN1' : { 'pin' : 'A0' },
};

board_css = """
""";

def get_pins():
  pins = pinutils.scan_pin_file([], 'stm32f103xb.csv', 6, 10, 11)
  return pinutils.only_from_package(pinutils.fill_gaps_in_pin_list(pins), chip["package"])
