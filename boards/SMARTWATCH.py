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
 'name' : "Sony Smartwatch",
 'link' : [ "http://developer.sonymobile.com/services/open-smartwatch-project/smartwatch-hacker-guide/" ],
 'variables' : 4000,
 'binary_name' : 'espruino_%v_smartwatch.bin',
};
chip = {
  'part' : "STM32F205RG",
  'family' : "STM32F2",
  'package' : "LQFP48", # FIXME
  'hse' : 26000000, # oscillator
  'ram' : 132,
  'flash' : 1024,
  'speed' : 130,  # FIXME
  'usart' : 3, # FIXME
  'spi' : 2, # FIXME
  'i2c' : 2, # FIXME
};
# left-right, or top-bottom order
board = {
};
devices = {
  'OSC' : { 'pin_1' :  'D0',
            'pin_2' : 'D1' },
  'BTN1' : { 'pin' : 'B11' },
};

#define BUZZER          (&PIN_PB8 )
#define BUTTON          (&PIN_PB11)
#define POWER           (&PIN_PC3 )
#define USB_CONNECTED   (&PIN_PA9 )
#define LIGHT_SENSOR    (&PIN_PA9 )
#define BATTERY_VOLTAGE (&PIN_PA9 )

board_css = """
""";

def get_pins():
   # FIXME NEED STM32F2 PINS
  pins = pinutils.scan_pin_file([], 'stm32f103xe.csv', 6, 10, 11)
  return pinutils.only_from_package(pinutils.fill_gaps_in_pin_list(pins), chip["package"])
