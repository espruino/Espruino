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
 'name' : "JSLCD",
 'link' :  [ "" ],
 'default_console' : "EV_SERIAL1",
 'default_console_tx' : "D1",
 'default_console_rx' : "D0",
 'default_console_baudrate' : "9600",
 'variables' : 2500, # How many variables are allocated for Espruino to use. RAM will be overflowed if this number is too high and code won't compile.
 'bootloader' : 1,
 'binary_name' : 'espruino_%v_jslcd.bin',
 'build' : {
   'optimizeflags' : '-Os',
   'libraries' : [
     'BLUETOOTH',
     'NET',
     'GRAPHICS',
     'NFC',
     'NEOPIXEL',
     'JSLCD'
   ],
   'makefile' : [
     'DEFINES+=-DHAL_NFC_ENGINEERING_BC_FTPAN_WORKAROUND=1', # Looks like proper production nRF52s had this issue
     'DEFINES+=-DBLUETOOTH_NAME_PREFIX=\'"JSLCD"\'',
     'DFU_PRIVATE_KEY=targets/nrf5x_dfu/dfu_private_key.pem',
     'DFU_SETTINGS=--application-version 0xff --hw-version 52 --sd-req 0x8C',
     'INCLUDE += -I$(ROOT)/libs/jslcd',
     'WRAPPERSOURCES += libs/jslcd/jswrap_jslcd.c'
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
    'address' : ((118 - 5) * 4096), # Bootloader takes pages 120-127, FS takes 118-119
    'page_size' : 4096,
    'pages' : 5,
    'flash_available' : 512 - ((31 + 8 + 1 + 5)*4) # Softdevice uses 31 pages of flash, bootloader 8, FS 1, code 5. Each page is 4 kb.
  },
};

devices = {
  'LED1' : { 'pin' : 'D16' }, # Pin negated in software
  'BTN1' : { 'pin' : 'D24', 'pinstate' : 'IN_PULLDOWN' }, # Pin negated in software
  'BTN2' : { 'pin' : 'D23', 'pinstate' : 'IN_PULLDOWN' }, # Pin negated in software
  'BTN3' : { 'pin' : 'D22', 'pinstate' : 'IN_PULLDOWN' }, # Pin negated in software
  'BTN4' : { 'pin' : 'D21', 'pinstate' : 'IN_PULLDOWN' }, # Pin negated in software
};

# left-right, or top-bottom order
board = {
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
  # Make buttons and LEDs negated
  pinutils.findpin(pins, "PD16", True)["functions"]["NEGATED"]=0;
  pinutils.findpin(pins, "PD21", True)["functions"]["NEGATED"]=0;
  pinutils.findpin(pins, "PD22", True)["functions"]["NEGATED"]=0;
  pinutils.findpin(pins, "PD23", True)["functions"]["NEGATED"]=0;
  pinutils.findpin(pins, "PD24", True)["functions"]["NEGATED"]=0;
  # everything is non-5v tolerant
  for pin in pins:
    pin["functions"]["3.3"]=0;
  #The boot/reset button will function as a reset button in normal operation. Pin reset on PD21 needs to be enabled on the nRF52832 device for this to work.
  return pins
