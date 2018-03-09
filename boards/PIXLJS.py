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
 'name' : "Pixl.js",
 'link' :  [ "http://www.espruino.com/Pixl.js" ],
 'default_console' : "EV_SERIAL1",
 'default_console_tx' : "D1",
 'default_console_rx' : "D0",
 'default_console_baudrate' : "9600",
 'variables' : 2500, # How many variables are allocated for Espruino to use. RAM will be overflowed if this number is too high and code won't compile.
 'bootloader' : 1,
 'binary_name' : 'espruino_%v_pixljs.hex',
 'build' : {
   'optimizeflags' : '-Os',
   'libraries' : [
     'BLUETOOTH',
     'NET',
     'GRAPHICS',
     'NFC',
     'NEOPIXEL',
     'PIXLJS',
     'CRYPTO',
     'FILESYSTEM',
     'TERMINAL'
   ],
   'makefile' : [
     'WIZNET=1','W5100=1', # Add WIZnet support - W5100 is the most common Arduino shield
     'DEFINES+=-DHAL_NFC_ENGINEERING_BC_FTPAN_WORKAROUND=1', # Looks like proper production nRF52s had this issue
#     'DEFINES+=-DCONFIG_GPIO_AS_PINRESET', # Allow the reset pin to work
     'DEFINES+=-DBLUETOOTH_NAME_PREFIX=\'"Pixl.js"\'',
     'DEFINES+=-DDUMP_IGNORE_VARIABLES=\'"g\\0"\'',
     'DFU_PRIVATE_KEY=targets/nrf5x_dfu/dfu_private_key.pem',
     'DFU_SETTINGS=--application-version 0xff --hw-version 52 --sd-req 0x8C',
     'INCLUDE += -I$(ROOT)/libs/pixljs',
     'WRAPPERSOURCES += libs/pixljs/jswrap_pixljs.c'
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
  'LED1' : { 'pin' : 'H0' }, # Pin negated in software
  'BTN1' : { 'pin' : 'H1', 'pinstate' : 'IN_PULLDOWN' }, # Pin negated in software
  'BTN2' : { 'pin' : 'H2', 'pinstate' : 'IN_PULLDOWN' }, # Pin negated in software
  'BTN3' : { 'pin' : 'H3', 'pinstate' : 'IN_PULLDOWN' }, # Pin negated in software
  'BTN4' : { 'pin' : 'H4', 'pinstate' : 'IN_PULLDOWN' }, # Pin negated in software
  'LCD' : {
            'width' : 128, 'height' : 64, 'bpp' : 1,
            'controller' : 'st7567',
            'pin_dc' : 'H5',
            'pin_cs' : 'H6',
            'pin_rst' : 'H7',
            'pin_sck' : 'H8',
            'pin_mosi' : 'H9',
            'pin_backlight' : 'H0',
          },
};

# left-right, or top-bottom order
board = {
};
board["_css"] = """
""";

def get_pins():
  pins = [
   { "name":"PA0", "sortingname":"A00", "port":"A", "num":"2", "functions":{ "ADC1_IN0":0 }, "csv":{} },
   { "name":"PA1", "sortingname":"A01", "port":"A", "num":"3", "functions":{ "ADC1_IN1":0 }, "csv":{} },
   { "name":"PA2", "sortingname":"A02", "port":"A", "num":"4", "functions":{ "ADC1_IN2":0 }, "csv":{} },
   { "name":"PA3", "sortingname":"A03", "port":"A", "num":"5", "functions":{ "ADC1_IN3":0 }, "csv":{} },
   { "name":"PA4", "sortingname":"A04", "port":"A", "num":"28", "functions":{ "ADC1_IN4":0 }, "csv":{} },
   { "name":"PA5", "sortingname":"A05", "port":"A", "num":"29", "functions":{ "ADC1_IN5":0 }, "csv":{} },
   
   { "name":"PD0",  "sortingname":"D00", "port":"D", "num":"25", "functions":{}, "csv":{} },
   { "name":"PD1",  "sortingname":"D01", "port":"D", "num":"26", "functions":{}, "csv":{} },
   { "name":"PD2",  "sortingname":"D02", "port":"D", "num":"27", "functions":{}, "csv":{} },
   { "name":"PD3",  "sortingname":"D03", "port":"D", "num":"30", "functions":{ "ADC1_IN6":0 }, "csv":{} },
   { "name":"PD4",  "sortingname":"D04", "port":"D", "num":"31", "functions":{ "ADC1_IN7":0 }, "csv":{} },
   { "name":"PD5",  "sortingname":"D05", "port":"D", "num":"0", "functions":{}, "csv":{} }, 
   { "name":"PD6",  "sortingname":"D06", "port":"D", "num":"1", "functions":{}, "csv":{} }, 
   { "name":"PD7",  "sortingname":"D07", "port":"D", "num":"6", "functions":{}, "csv":{} }, 
   { "name":"PD8",  "sortingname":"D08", "port":"D", "num":"7", "functions":{}, "csv":{} },
   { "name":"PD9",  "sortingname":"D09", "port":"D", "num":"8", "functions":{}, "csv":{} }, 
   { "name":"PD10", "sortingname":"D10", "port":"D", "num":"17", "functions":{}, "csv":{} },
   { "name":"PD11", "sortingname":"D11", "port":"D", "num":"18", "functions":{}, "csv":{} },
   { "name":"PD12", "sortingname":"D12", "port":"D", "num":"19", "functions":{}, "csv":{} },
   { "name":"PD13", "sortingname":"D13", "port":"D", "num":"20", "functions":{}, "csv":{} },

   { "name":"PH0", "sortingname":"H0", "port":"H", "num":"16", "functions":{}, "csv":{} }, # LED
   { "name":"PH1", "sortingname":"H1", "port":"H", "num":"23", "functions":{}, "csv":{} }, # BTN1
   { "name":"PH2", "sortingname":"H2", "port":"H", "num":"21", "functions":{}, "csv":{} }, # 2
   { "name":"PH3", "sortingname":"H3", "port":"H", "num":"22", "functions":{}, "csv":{} }, # 3
   { "name":"PH4", "sortingname":"H4", "port":"H", "num":"24", "functions":{}, "csv":{} }, # 4
   { "name":"PH5", "sortingname":"H5", "port":"H", "num":"13", "functions":{}, "csv":{} }, # LCD DC
   { "name":"PH6", "sortingname":"H6", "port":"H", "num":"12", "functions":{}, "csv":{} }, # LCD CS
   { "name":"PH7", "sortingname":"H7", "port":"H", "num":"11", "functions":{}, "csv":{} }, # LCD RST
   { "name":"PH8", "sortingname":"H8", "port":"H", "num":"14", "functions":{}, "csv":{} }, # LCD SCK
   { "name":"PH9", "sortingname":"H9", "port":"H", "num":"15", "functions":{}, "csv":{} }, # LCD MOSI
  ];
  # Make buttons and LEDs negated
  pinutils.findpin(pins, "PH0", True)["functions"]["NEGATED"]=0;
  pinutils.findpin(pins, "PH1", True)["functions"]["NEGATED"]=0;
  pinutils.findpin(pins, "PH2", True)["functions"]["NEGATED"]=0;
  pinutils.findpin(pins, "PH3", True)["functions"]["NEGATED"]=0;
  pinutils.findpin(pins, "PH4", True)["functions"]["NEGATED"]=0;
  # everything is non-5v tolerant
  for pin in pins:
    pin["functions"]["3.3"]=0;
  #The boot/reset button will function as a reset button in normal operation. Pin reset on PD21 needs to be enabled on the nRF52832 device for this to work.
  return pins
