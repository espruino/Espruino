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
 'name' : "Arietta G25",
 'default_console' : "EV_USBSERIAL",
 'binary_name' : 'espruino_%v_arietta',
 'build' : {
   'optimizeflags' : '-O3',
   'libraries' : [
     'NET',
     'GRAPHICS',
     'FILESYSTEM',
     'CRYPTO',
     'TLS',
     'HASHLIB',
     'TELNET',
   ],
   'makefile' : [
     'LINUX=1',
     'DEFINES += -DARIETTA_G25 -DSYSFS_GPIO_DIR="\"/sys/class/gpio\"',
   ]
 }
};
chip = {
  'part' : "ARMV5TEJL",
  'family' : "LINUX",
  'package' : "",
  'ram' : -1,
  'flash' : -1,
  'speed' : -1,
  'usart' : 0,
  'spi' : 1,
  'i2c' : 0,
  'adc' : 0,
  'dac' : 0,
};
# left-right, or top-bottom order
devices = {
};

def get_pins():
  pins = pinutils.generate_pins(0,31)
  pinutils.findpin(pins, "PD7", True)["functions"]["SPI1_SCK"]=0;
  pinutils.findpin(pins, "PD8", True)["functions"]["SPI1_MOSI"]=0;
  pinutils.findpin(pins, "PD10", True)["functions"]["SPI1_MISO"]=0;
  return pins
