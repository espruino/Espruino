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
 'name' : "nRF51 Tag",
 'link' :  [ "" ],
 'default_console' : "EV_SERIAL1",
 'default_console_tx' : "D15",
 'default_console_rx' : "D17",
 'default_console_baudrate' : "9600",
 'variables' : 95, # How many variables are allocated for Espruino to use. RAM will be overflowed if this number is too high and code won't compile.
 'binary_name' : 'espruino_%v_nrf51tag.bin',
};

chip = {
  'part' : "NRF51822",
  'family' : "NRF51",
  'package' : "QFN48",
  'ram' : 16,
  'flash' : 256,
  'speed' : 16,
  'usart' : 1, #THIS IS INCORRECT!!!
  'spi' : 3,
  'i2c' : 2,
  'adc' : 1,
  'dac' : 0,
};

board = {
};

devices = {
#  'LED1' : { 'pin' : 'D22' },
#  'LED2' : { 'pin' : 'D21' },
#  'LED3' : { 'pin' : 'D23' }
};

board_css = """
""";

def get_pins():
  pins = pinutils.generate_pins(0,31) # 32 General Purpose I/O Pins.
  pinutils.findpin(pins, "PD27", True)["functions"]["XL1"]=0;
  pinutils.findpin(pins, "PD26", True)["functions"]["XL2"]=0;
  #The boot/reset button will function as a reset button in normal operation. Pin reset on PD21 needs to be enabled on the nRF52832 device for this to work.
  return pins
