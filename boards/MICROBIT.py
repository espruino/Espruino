#!/bin/false
# This file is part of Espruino, a JavaScript interpreter for Microcontrollers
#
# Copyright (C) 2015 Gordon Williams <gw@pur3.co.uk>
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
# placeholder
info = {
 'name' : "BBC micro:bit",
 'link' : [ "" ],
 'variables' : 191, 
 'binary_name' : 'espruino_%v_microbit.bin',
};
chip = {
  'part' : "NRF51822",
  'family' : "NRF51",
#  'package' : "LQFP64",
  'ram' : 16,
  'flash' : 256,
  'speed' : 96,
  'usart' : 1,
  'spi' : 1,
  'i2c' : 1,
  'adc' : 0,
  'dac' : 0
};
# left-right, or top-bottom order
board = {
};
devices = {
  'BTN1' : { 'pin' : 'D4' }, # 'P0_17'
  'BTN2' : { 'pin' : 'D11' }, # 'P0_26'
};

board_css = """
""";

def get_pins():
  pins = [
   { "name":"PD0", "sortingname":"D00", "port":"D", "num":"3", "functions":{}, "csv":{} },
   { "name":"PD1", "sortingname":"D01", "port":"D", "num":"2", "functions":{}, "csv":{} },
   { "name":"PD2", "sortingname":"D02", "port":"D", "num":"1", "functions":{}, "csv":{} },
   { "name":"PD3", "sortingname":"D03", "port":"D", "num":"4", "functions":{}, "csv":{} },
   { "name":"PD4", "sortingname":"D04", "port":"D", "num":"17", "functions":{}, "csv":{} }, # BTNA
   { "name":"PD5", "sortingname":"D05", "port":"D", "num":"5", "functions":{}, "csv":{} },
   { "name":"PD6", "sortingname":"D06", "port":"D", "num":"14", "functions":{}, "csv":{} },
   { "name":"PD7", "sortingname":"D07", "port":"D", "num":"13", "functions":{}, "csv":{} },
   { "name":"PD8", "sortingname":"D08", "port":"D", "num":"18", "functions":{}, "csv":{} },
   { "name":"PD9", "sortingname":"D09", "port":"D", "num":"15", "functions":{}, "csv":{} },
   { "name":"PD10", "sortingname":"D10", "port":"D", "num":"6", "functions":{}, "csv":{} },
   { "name":"PD11", "sortingname":"D11", "port":"D", "num":"26", "functions":{}, "csv":{} }, # BTNB
   { "name":"PD12", "sortingname":"D12", "port":"D", "num":"20", "functions":{}, "csv":{} },
   { "name":"PD13", "sortingname":"D13", "port":"D", "num":"23", "functions":{ "SPI1_SCK":0 }, "csv":{} },
   { "name":"PD14", "sortingname":"D14", "port":"D", "num":"22", "functions":{ "SPI1_MISO":0 }, "csv":{} },
   { "name":"PD15", "sortingname":"D15", "port":"D", "num":"21", "functions":{ "SPI1_MOSI":0 }, "csv":{} },
   { "name":"PD16", "sortingname":"D16", "port":"D", "num":"16", "functions":{}, "csv":{} },
   { "name":"PD17", "sortingname":"D17", "port":"D", "num":"0", "functions":{ "I2C1_SCL":0 }, "csv":{} },
   { "name":"PD18", "sortingname":"D18", "port":"D", "num":"30", "functions":{ "I2C1_SDA":0 }, "csv":{} }
  ];
  return pins
