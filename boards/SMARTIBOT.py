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
 'name' : "Smartibot",
 'link' :  [ "http://thecraftyrobot.net" ],
 'espruino_page_link' : 'Smartibot',
 'bootloader' : 1,
 'default_console' : "EV_SERIAL1",
 'default_console_tx' : "D22",
 'default_console_rx' : "D23",
 'default_console_baudrate' : "9600",
 'variables' : 2250, # How many variables are allocated for Espruino to use. RAM will be overflowed if this number is too high and code won't compile.
 'binary_name' : 'espruino_%v_smartibot.hex',
 'build' : {
   'optimizeflags' : '-Os',
   'libraries' : [
     'BLUETOOTH',
     'GRAPHICS',
     'NEOPIXEL'
   ],
   'makefile' : [
     'DFU_PRIVATE_KEY=targets/nrf5x_dfu/dfu_private_key.pem',
     'DFU_SETTINGS=--application-version 0xff --hw-version 52 --sd-req 0x8C,0x91',
     'DEFINES += -DCONFIG_GPIO_AS_PINRESET', # Allow the reset pin to work
     'DEFINES += -DCONFIG_NFCT_PINS_AS_GPIOS=1', # Use NFC for GPIOs
     'DEFINES+=-DBLUETOOTH_NAME_PREFIX=\'"Smartibot"\'',
     'DEFINES += -DNEOPIXEL_SCK_PIN=16 -DNEOPIXEL_LRCK_PIN=15', # SCK pin needs defining as something unused for neopixel (HW errata means they can't be disabled) see https://github.com/espruino/Espruino/issues/2071
#     'JSMODULESOURCES+=libs/js/PCA9685.min.js',
#     'JSMODULESOURCES+=libs/js/Smartibot.min.js'
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
  'spi' : 1,
  'i2c' : 1,
  'adc' : 1,
  'dac' : 0,
  'saved_code' : {
    'address' : ((118 - 10) * 4096), # Bootloader takes pages 120-127, FS takes 118-119
    'page_size' : 4096,
    'pages' : 10,
    'flash_available' : 512 - ((31 + 8 + 2 + 10)*4) # Softdevice uses 31 pages of flash, bootloader 8, FS 2, code 10. Each page is 4 kb.
  },
};

devices = {
  'BTN1' : { 'pin' : 'D29', 'pinstate' : 'IN_PULLUP' }, # Pin negated in software
  'BTN2' : { 'pin' : 'D13', 'pinstate' : 'IN_PULLUP' }, # Pin negated in software
  'LED1' : { 'pin' : 'D20' },
  'RX_PIN_NUMBER' : { 'pin' : 'D23'},
  'TX_PIN_NUMBER' : { 'pin' : 'D22'},
};

# left-right, or top-bottom order
board = {
  '_notes' : {
    'D23' : "Serial console RX",
    'D22' : "Serial console TX"
  }
};
board["_css"] = """
#board {
  width: 600px;
  height: 595px;
  background-image: url(img/SMARTIBOT.jpg);
}
""";

def get_pins():
  pins = pinutils.generate_pins(0,31) # 32 General Purpose I/O Pins.
  pinutils.findpin(pins, "PD0", True)["functions"]["XL1"]=0;
  pinutils.findpin(pins, "PD1", True)["functions"]["XL2"]=0;
  pinutils.findpin(pins, "PD5", True)["functions"]["RTS"]=0;
  pinutils.findpin(pins, "PD22", True)["functions"]["TXD"]=0;
  pinutils.findpin(pins, "PD7", True)["functions"]["CTS"]=0;
  pinutils.findpin(pins, "PD23", True)["functions"]["RXD"]=0;
  pinutils.findpin(pins, "PD2", True)["functions"]["ADC1_IN0"]=0;
  pinutils.findpin(pins, "PD3", True)["functions"]["ADC1_IN1"]=0;
  pinutils.findpin(pins, "PD4", True)["functions"]["ADC1_IN2"]=0;
  pinutils.findpin(pins, "PD5", True)["functions"]["ADC1_IN3"]=0;
  pinutils.findpin(pins, "PD28", True)["functions"]["ADC1_IN4"]=0;
  pinutils.findpin(pins, "PD29", True)["functions"]["ADC1_IN5"]=0;
  pinutils.findpin(pins, "PD30", True)["functions"]["ADC1_IN6"]=0;
  pinutils.findpin(pins, "PD31", True)["functions"]["ADC1_IN7"]=0;
  # Make BTN1 + BTN2 negated
  pinutils.findpin(pins, "PD29", True)["functions"]["NEGATED"]=0; # BTN1
  pinutils.findpin(pins, "PD13", True)["functions"]["NEGATED"]=0; # BTN2

  # everything is non-5v tolerant
  for pin in pins:
    pin["functions"]["3.3"]=0;
  #The boot/reset button will function as a reset button in normal operation. Pin reset on PD21 needs to be enabled on the nRF52832 device for this to work.
  return pins
