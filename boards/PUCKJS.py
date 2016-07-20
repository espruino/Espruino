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
 'name' : "PuckJS",
 'link' :  [ "" ],
 'default_console' : "EV_SERIAL1",
 'default_console_tx' : "D28",
 'default_console_rx' : "D29",
 'default_console_baudrate' : "9600",
 # Number of variables can be WAY higher on this board
 'variables' : 1020, # How many variables are allocated for Espruino to use. RAM will be overflowed if this number is too high and code won't compile.
 'binary_name' : 'espruino_%v_puckjs.bin',
 'build' : {
  'defines' : [
     'USE_BLUETOOTH'
   ]
 }
};

chip = {
  'part' : "NRF52832",
  'family' : "NRF52",
  'package' : "QFN48",
  'ram' : 64,
  'flash' : 512,
  'speed' : 64,
  'usart' : 1, 
  'spi' : 3,
  'i2c' : 2,
  'adc' : 1,
  'dac' : 0,
  'saved_code' : {
    'address' : ((128 - 3) * 4096),
    'page_size' : 4096,
    'pages' : 3,
    'flash_available' : (512 - 124 - 12) # Softdevice uses 31 plages of flash. Each page is 4 kb.
  },
};

devices = {
  'LED1' : { 'pin' : 'D5' },
  'LED2' : { 'pin' : 'D4' },
  'LED3' : { 'pin' : 'D3' },
  'LED4' : { 'pin' : 'D25', 'pin2' : 'D26' },
  'BTN1' : { 'pin' : 'D0', 'pinstate' : 'IN_PULLDOWN' }
# CAPSENSE D8
};

# left-right, or top-bottom order
board = {
  'left' : [ 'PD28', 'PD29', 'PD30', 'PD31'],
  'right' : [ 'GND', '3V', 'D4', 'D5' ],
};

def get_pins():
  pins = pinutils.generate_pins(0,31) # 32 General Purpose I/O Pins.
  pinutils.findpin(pins, "PD0", True)["functions"]["XL1"]=0;
  pinutils.findpin(pins, "PD1", True)["functions"]["XL2"]=0;
  pinutils.findpin(pins, "PD9", True)["functions"]["NFC1"]=0;
  pinutils.findpin(pins, "PD10", True)["functions"]["NFC2"]=0;
  pinutils.findpin(pins, "PD2", True)["functions"]["ADC1_IN0"]=0;
  pinutils.findpin(pins, "PD3", True)["functions"]["ADC1_IN1"]=0;
  pinutils.findpin(pins, "PD4", True)["functions"]["ADC1_IN2"]=0;
  pinutils.findpin(pins, "PD5", True)["functions"]["ADC1_IN3"]=0;
  pinutils.findpin(pins, "PD28", True)["functions"]["ADC1_IN4"]=0;
  pinutils.findpin(pins, "PD29", True)["functions"]["ADC1_IN5"]=0;
  pinutils.findpin(pins, "PD30", True)["functions"]["ADC1_IN6"]=0;
  pinutils.findpin(pins, "PD31", True)["functions"]["ADC1_IN7"]=0;

  #The boot/reset button will function as a reset button in normal operation. Pin reset on PD21 needs to be enabled on the nRF52832 device for this to work.
  return pins
