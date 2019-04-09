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
 'name' : "SparkFun nRF52832 Breakout",
 'link' :  [ "https://www.sparkfun.com/products/13990" ],
 'espruino_page_link' : 'nRF52832_SPARKFUN',
  # This is the PCA10036
 'default_console' : "EV_SERIAL1",
 'default_console_tx' : "D27",
 'default_console_rx' : "D26",
 'default_console_baudrate' : "9600",
 'variables' : 2250, # How many variables are allocated for Espruino to use. RAM will be overflowed if this number is too high and code won't compile.
# 'bootloader' : 1,
 'binary_name' : 'espruino_%v_nrf52832_sparkfun.hex',
 'build' : {
   'optimizeflags' : '-Os',
   'libraries' : [
     'BLUETOOTH',
     'NET',
     'GRAPHICS',
     'NFC',
     'NEOPIXEL'
   ],
   'makefile' : [
     'DEFINES+=-DCONFIG_GPIO_AS_PINRESET' # Allow the reset pin to work
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
    'address' : ((118 - 10) * 4096), # Bootloader takes pages 120-127, FS takes 118-119
    'page_size' : 4096,
    'pages' : 10,
    'flash_available' : 512 - ((31 + 8 + 2 + 10)*4) # Softdevice uses 31 pages of flash, bootloader 8, FS 2, code 10. Each page is 4 kb.
  },
};

devices = {
  'BTN1' : { 'pin' : 'D6', 'pinstate' : 'IN_PULLDOWN' }, # Pin negated in software
  'LED1' : { 'pin' : 'D7' }, # Pin negated in software
  'RX_PIN_NUMBER' : { 'pin' : 'D26'},
  'TX_PIN_NUMBER' : { 'pin' : 'D27'},
};

# left-right, or top-bottom order
board = {
  'bottom' : [ '', 'D26', 'D27', 'VCC', '', 'GND'],
  'left' : [ 'D16','D17','D18','D19','D20','D22','D23','D24','D25','D28','D29','D30','D31','RST','3V3','VIN', 'GND'],
  'right' : [ 'D15','D14','D13','D12','D11','D10','D9','D8','D5','D4','D3','D2','D1','D0','3V3','VIN', 'GND'],
  '_notes' : {
    'D26' : "Serial console RX",
    'D27' : "Serial console TX"
  }
};
board["_css"] = """
#board {
  width: 600px;
  height: 600px;
  top: 0px;
  left : 200px;
  background-image: url(img/NRF52832_SPARKFUN.jpg);
}
#boardcontainer {
  height: 900px;
}

#bottom {
    top: 539px;
    left: 236px;
}
#left {
    top: 119px;
    right: 380px;
}
#right {
    top: 119px;
    left: 390px;
}

.bottompin { width: 18px; }
.leftpin { height: 22px; }
.rightpin { height: 22px; }
""";

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
  pinutils.findpin(pins, "PD26", True)["functions"]["USART1_RX"]=0;
  pinutils.findpin(pins, "PD27", True)["functions"]["USART1_TX"]=0;
  pinutils.findpin(pins, "PD28", True)["functions"]["ADC1_IN4"]=0;
  pinutils.findpin(pins, "PD29", True)["functions"]["ADC1_IN5"]=0;
  pinutils.findpin(pins, "PD30", True)["functions"]["ADC1_IN6"]=0;
  pinutils.findpin(pins, "PD31", True)["functions"]["ADC1_IN7"]=0;
  # Make buttons and LEDs negated
  pinutils.findpin(pins, "PD6", True)["functions"]["NEGATED"]=0;
  pinutils.findpin(pins, "PD7", True)["functions"]["NEGATED"]=0;

  # everything is non-5v tolerant
  for pin in pins:
    pin["functions"]["3.3"]=0;
  #The boot/reset button will function as a reset button in normal operation. Pin reset on PD21 needs to be enabled on the nRF52832 device for this to work.
  return pins
