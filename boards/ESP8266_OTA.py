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

# The ESP8266_OTA represents esp8266 modules with 1MB or more flash and using the OTA
# (over the air update) flash layout with two 512KB partitions in the first MB and the
# v1.4 or later bootloader at 0x0. Modules with >1MB flash will have a SPIFFS filesystem
# in the flash beyond the first 1MB.

import pinutils;
info = {
 'name'            : "ESP8266 OTA",
 'default_console' : "EV_SERIAL1",
 'variables'       : 1023,
 'binary_name'     : 'espruino_esp8266_ota',
};
chip = {
  'part'    : "ESP8266",
  'family'  : "ESP8266",
  'package' : "",
  'ram'     : 80,
  'flash'   : 1024,
  'speed'   : 80,
  'usart'   : 1,
  'spi'     : 0,
  'i2c'     : 0,
  'adc'     : 1,
  'dac'     : 0,
  'saved_code' : {
    # see https://github.com/espruino/Espruino/wiki/ESP8266-Design-Notes#flash-map-and-access
    'address' : 0x7C000,
    'page_size' : 4096,
    'pages' : 3, # there are really 4 pages reserved but we should only need 3
    'flash_available' : 492, # firmware can be up to this size
  },
};
# left-right, or top-bottom order
board = {
};
devices = {
};

board_css = """
""";

def get_pins():
  pins = pinutils.generate_pins(0,15)
  # just fake pins D0 .. D15
  return pins
