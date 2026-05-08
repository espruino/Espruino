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
 'name' : "nRF54L15 Development Kit",
# 'default_console' : "EV_BLUETOOTH",
 #'default_console_tx' : "D0",
 #'default_console_rx' : "D1",
 #'default_console_baudrate' : "9600",
 'variables' : 2630, # How many variables are allocated for Espruino to use. RAM will be overflowed if this number is too high and code won't compile.
# 'bootloader' : 1,
 'binary_name' : 'espruino_%v_nrf54l15.hex',
 'build' : {
   'libraries' : [
     'BLUETOOTH'
   ],
   'makefile' : [
   ]
 }
};


chip = {
  'part' : "NRF54L15",
  'family' : "ZEPHYR",
  'package' : "",
  'ram' : 256,
  'flash' : 1536,
  'speed' : 128,
  'usart' : 1,
  'spi' : 0,
  'i2c' : 0,
  'adc' : 0,
  'dac' : 0,
  'saved_code' : {
    'address' : 1024*1024, 
    'page_size' : 4096,
    'pages' : 128,
    'flash_available' : 1024
  },
};

devices = {
  'BTN1' : { 'pin' : 'B13', 'pinstate' : 'IN_PULLDOWN' }, # Pin negated in software P1.13
  'BTN2' : { 'pin' : 'B9', 'pinstate' : 'IN_PULLDOWN' }, # Pin negated in software P1.09
  'BTN3' : { 'pin' : 'B8', 'pinstate' : 'IN_PULLDOWN' }, # Pin negated in software P1.08
  'BTN4' : { 'pin' : 'A4', 'pinstate' : 'IN_PULLDOWN' }, # Pin negated in software P1.04
  'LED1' : { 'pin' : 'C9' }, 
  'LED2' : { 'pin' : 'B10' },
  'LED3' : { 'pin' : 'C7' }, 
  'LED4' : { 'pin' : 'B14' },
};

# left-right, or top-bottom order
board = {
  'left' : [ 'VDD', 'VDD', 'RESET', 'VDD','5V','GND','GND','','','D3','D4','D28','D29','D30','D31'],
  'right' : [
     'D27', 'D26', 'D2', 'GND', 'D25','D24','D23', 'D22','D20','D19','',
     'D18','D17','D16','D15','D14','D13','D12','D11','',
     'D10','D9','D8','D7','D6','D5','D21','D1','D0'],
  '_notes' : {
    'D6' : "Serial console RX",
    'D8' : "Serial console TX"
  }
};
board["_css"] = """
""";

def get_pins():
  # GPIO 0/1/2
  pins = pinutils.generate_pins(0,4,"A") + pinutils.generate_pins(0,14,"B") + pinutils.generate_pins(0,10,"C"); 
  # pinutils.findpin(pins, "PAxx", True)["functions"]["XL1"]=0;
  # ...
  # Make buttons and LEDs negated
  pinutils.findpin(pins, "PB13", True)["functions"]["NEGATED"]=0;
  pinutils.findpin(pins, "PB9", True)["functions"]["NEGATED"]=0;
  pinutils.findpin(pins, "PB8", True)["functions"]["NEGATED"]=0;
  pinutils.findpin(pins, "PA4", True)["functions"]["NEGATED"]=0;

  # everything is non-5v tolerant
  for pin in pins:
    pin["functions"]["3.3"]=0;
  #The boot/reset button will function as a reset button in normal operation. Pin reset on PD21 needs to be enabled on the nRF52832 device for this to work.
  return pins
