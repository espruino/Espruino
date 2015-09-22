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
 'name' : "nRF51 Development Kit",
 'link' :  [ "https://www.nordicsemi.com/Products/Bluetooth-Smart-Bluetooth-low-energy/nRF51822" ],
 'default_console' : "EV_SERIAL1",
 'variables' : 200, # How many variables are allocated for Espruino to use. RAM will be overflowed if this number is too high and code won't compile.
 'binary_name' : 'espruino_%v_nrf51822.bin',
};

chip = {
  'part' : "NRF51822",
  'family' : "NRF51",
  'package' : "QFN48",
  'ram' : 32,
  'flash' : 256,
  'speed' : 16,
  'usart' : 1, #THIS IS INCORRECT!!!
  'spi' : 3,
  'i2c' : 2,
  'adc' : 1,
  'dac' : 0,
};

# left-right, or top-bottom order THIS IS INCORRECT!!!!!
board = {
  'left' : [ 'VDD', 'VDD', 'RESET', 'VDD','5V','GND','GND','PD3','PD4','PD28','PD29','PD30','PD31'],
  'right' : [ 'PD27', 'PD26', 'PD2', 'GND', 'PD25','PD24','PD23', 'PD22','PD20','PD19','PD18','PD17','PD16','PD15','PD14','PD13','PD12','PD11','PD10','PD9','PD8','PD7','PD6','PD5','PD21','PD1','PD0'],
};

devices = {
  'LED_1' : { 'pin' : 'D21' },
  'LED_2' : { 'pin' : 'D22' },
  'LED_3' : { 'pin' : 'D23' },
  'LED_4' : { 'pin' : 'D24' },
  'BUTTON_1' : { 'pin' : 'D17'},
  'BUTTON_2' : { 'pin' : 'D18'},
  'BUTTON_3' : { 'pin' : 'D19'},
  'BUTTON_4' : { 'pin' : 'D20'},
  'RX_PIN_NUMBER' : { 'pin' : 'D11'},
  'TX_PIN_NUMBER' : { 'pin' : 'D9'},
  'CTS_PIN_NUMBER' : { 'pin' : 'D10'},
  'RTS_PIN_NUMBER' : { 'pin' : 'D8'},
};

board_css = """
""";

def get_pins():
  pins = pinutils.generate_pins(0,31) # 32 General Purpose I/O Pins.
  pinutils.findpin(pins, "PD27", True)["functions"]["XL1"]=0;
  pinutils.findpin(pins, "PD26", True)["functions"]["XL2"]=0;
  pinutils.findpin(pins, "PD8", True)["functions"]["RTS"]=0;
  pinutils.findpin(pins, "PD9", True)["functions"]["TXD"]=0;
  pinutils.findpin(pins, "PD10", True)["functions"]["CTS"]=0;
  pinutils.findpin(pins, "PD11", True)["functions"]["RXD"]=0;
  pinutils.findpin(pins, "PD17", True)["functions"]["Button_1"]=0;
  pinutils.findpin(pins, "PD18", True)["functions"]["Button_2"]=0;
  pinutils.findpin(pins, "PD19", True)["functions"]["Button_3"]=0;
  pinutils.findpin(pins, "PD20", True)["functions"]["Button_4"]=0;
  pinutils.findpin(pins, "PD21", True)["functions"]["LED_1"]=0;
  pinutils.findpin(pins, "PD22", True)["functions"]["LED_2"]=0;
  pinutils.findpin(pins, "PD23", True)["functions"]["LED_3"]=0;
  pinutils.findpin(pins, "PD24", True)["functions"]["LED_4"]=0;
  #The boot/reset button will function as a reset button in normal operation. Pin reset on PD21 needs to be enabled on the nRF52832 device for this to work.
  return pins
