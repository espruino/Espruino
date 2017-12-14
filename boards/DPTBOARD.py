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
 'name' : "DPT-Board",
 'default_console' : "EV_USBSERIAL",
 'binary_name' : 'espruino',
 'build' : {
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
     'DEFINES += -DDPTBOARD -DSYSFS_GPIO_DIR="\"/sys/class/gpio\""',
     'OPENWRT_UCLIBC=1	# link with toolchain libc (uClibc or musl)'
   ]
 }
};
chip = {
  'part' : "DPTBOARD",
  'family' : "LINUX",
  'package' : "",
  'ram' : -1,
  'flash' : -1,
  'speed' : -1,
  'usart' : 1,
  'spi' : 1,
  'i2c' : 1,
  'adc' : 0,
  'dac' : 0,
};
# left-right, or top-bottom order
devices = {
};

def get_pins():
  pins = pinutils.generate_pins(0,27)

  return pins
