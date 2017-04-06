#!/bin/false
# This file is part of Espruino, a JavaScript interpreter for Microcontrollers
#
# Copyright (C) 2013 Ruuvi Innovations Ltd <info@ruuvi.com>
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
 'name' : "RuuviTag",
 'link' :  [ "https://ruuvitag.com" ],
 'default_console' : "EV_SERIAL1",
 'default_console_tx' : "D4",
 'default_console_rx' : "D5",
 'default_console_baudrate' : "9600",
 # Number of variables can be WAY higher on this board
 'variables' : 2000, # How many variables are allocated for Espruino to use. RAM will be overflowed if this number is too high and code won't compile.
 'bootloader' : 1,
 'binary_name' : 'espruino_%v_ruuvitag.bin',
 'build' : {
   'optimizeflags' : '-Os',
   'libraries' : [
     'BLUETOOTH',
     'NET',
     'GRAPHICS',
     'CRYPTO',
     'NFC',
     'NEOPIXEL'
     #'HASHLIB'
     #'FILESYSTEM'
     #'TLS'
   ],
   'makefile' : [
     'DFU_PRIVATE_KEY=targets/nrf5x_dfu/ruuvi_open_private.pem',
     'DFU_SETTINGS=--debug-mode'
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
    'address' : ((114 - 3) * 4096), # Bootloader takes pages 117-127 on RuuviTag, FS takes 114-116
    'page_size' : 4096,
    'pages' : 3,
    'flash_available' : 512 - ((31 + 11 + 1 + 3)*4) # Softdevice uses 31 pages of flash, bootloader 11, code 3. Each page is 4 kb.
  },
};

devices = {
  'LED1' : { 'pin' : 'D17', 'inverted' : True },
  'LED2' : { 'pin' : 'D19', 'inverted' : True },
  'BTN1' : { 'pin' : 'D13', 'inverted' : True, 'pinstate' : 'IN_PULLUP' },
  'CSBME' : { 'pin' : 'D3', 'inverted' : True, 'pinstate' : 'IN_PULLUP' },
  'CSLIS' : { 'pin' : 'D8', 'inverted' : True, 'pinstate' : 'IN_PULLUP' },
  'RX_PIN_NUMBER' : { 'pin' : 'D5'},
  'TX_PIN_NUMBER' : { 'pin' : 'D4'},
  'CTS_PIN_NUMBER' : { 'pin' : 'D31'},
  'RTS_PIN_NUMBER' : { 'pin' : 'D30'},
  'NFC': { 'pin_a':'D9', 'pin_b':'D10' }#,
#  'BME280': { 'pin_pwr':'D18',
#           'pin_int':'D17',
#           'pin_sda':'D20',
#           'pin_scl':'D19' }#,
#  'LIS2DH12': { 'pin_pwr':'D18',
#           'pin_int':'D17',
#           'pin_sda':'D20',
#           'pin_scl':'D19' }
};

# left-right, or top-bottom order
board = {
  'left' : [ 'VDD', 'VDD', 'RESET', 'VDD','5V','GND','GND','PD3','PD4','PD28','PD29','PD30','PD31'],
  'right' : [ 'PD27', 'PD26', 'PD2', 'GND', 'PD25','PD24','PD23', 'PD22','PD20','PD19','PD18','PD17','PD16','PD15','PD14','PD13','PD12','PD11','PD10','PD9','PD8','PD7','PD6','PD5','PD21','PD1','PD0'],
};
board["_css"] = """
""";

def get_pins():
  pins = pinutils.generate_pins(0,31) # 32 General Purpose I/O Pins.
  pinutils.findpin(pins, "PD0", True)["functions"]["XL1"]=0;
  pinutils.findpin(pins, "PD1", True)["functions"]["XL2"]=0;
  pinutils.findpin(pins, "PD5", True)["functions"]["RTS"]=0;
  pinutils.findpin(pins, "PD6", True)["functions"]["TXD"]=0;
  pinutils.findpin(pins, "PD7", True)["functions"]["CTS"]=0;
  pinutils.findpin(pins, "PD8", True)["functions"]["RXD"]=0;
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
