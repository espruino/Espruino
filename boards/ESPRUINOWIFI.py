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
 'name' : "Espruino WiFi",
 'link' : [ "http://www.espruino.com/EspruinoWiFi" ],
 'espruino_page_link' : 'EspruinoWiFi',
 'default_console' : "EV_SERIAL1",
 'default_console_tx' : "B6",
 'default_console_rx' : "B7",
 'variables' : 5100,
 'bootloader' : 1,
 'binary_name' : 'espruino_%v_pico_1r3.bin',
 'binaries' : [
  { 'filename' : 'espruino_%v_pico_1r3.bin', 'description' : "Normal Espruino WiFi build"},
 ],
 'build' : {
   'defines' : [
     'USE_USB_HID',
     'USE_NET',
     'USE_GRAPHICS',
     'USE_TV',
     'USE_HASHLIB',
     'USE_FILESYSTEM',
     'USE_CRYPTO',
     'USE_TLS'
   ]
 }
};
chip = {
  'part' : "STM32F411CEU6",
  'family' : "STM32F4",
  'package' : "UQFN48", 
  'ram' : 96,
  'flash' : 512,
  'speed' : 100,
  'usart' : 6,
  'spi' : 3,
  'i2c' : 3,
  'adc' : 1,
  'dac' : 0,
  'saved_code' : {
    'address' : 0x08010000,
    'page_size' : 65536, # size of pages
    'pages' : 1, # number of pages we're using
    'flash_available' : 512-128 # Saved code is before binary, test against full size minus offset
  },
  'place_text_section' : 0x00020000, # note flash_available above
};

devices = {
  'OSC' : { 'pin_in' :  'H0', # checked
            'pin_out' : 'H1' }, # checked
  'OSC_RTC' : { 'pin_in' :  'C14', # checked
                'pin_out' : 'C15' }, # checked
  'BTN1' : { 'pin' : 'C13', 'pinstate' : 'IN_PULLDOWN' }, 
  'LED1' : { 'pin' : 'B2' }, 
  'LED2' : { 'pin' : 'B12' },
  'USB' : { 'pin_vsense' :  'A9',
            'pin_dm' : 'A11',   # checked
            'pin_dp' : 'A12' }, # checked
  'JTAG' : {
        'pin_MS' : 'A13',
        'pin_CK' : 'A14', 
#        'pin_DI' : 'A15' 
          }
};

# left-right, or top-bottom order
board = {
  'top' : [ 'B15', 'B14', 'B13', 'B10', 'B1', 'A7', 'A6', 'A5', 'A4', 'A1', 'A0' ], 
  'bottom' : [ 'GND', 'VBAT', '3.3', 'B3', 'B4', 'B5', 'B6', 'B7','B8', 'B9', 'B0'],

  'right' : ['A10','A8'],
  '_notes' : {
    'A2' : 'ESP8266 RX',
    'A3' : 'ESP8266 TX',
    'A15' : 'ESP8266 CTS', # ESP8266 Clear to send (when enabled with AT+UART_DEF)
    'A14' : 'ESP8266 CH_PD', # ESP8266 Power Down, also ARM SWCLK (pull down by default)
    'A13' : 'ESP8266 GPIO0', # ESP8266 bootloader mode, also ARM SWDIO (pull up by default)

  }
};
board["_css"] = """
#board {
  width: 550px;
  height: 272px;
  top: 300px;
  left : 100px;
  background-image: url(img/ESPRUINOWIFI.png);
}
#boardcontainer {
  height: 800px;
}
#top {
  bottom: 253px;
  left: 194px;
}
#bottom {
  top: 255px;
  left: 192px;
}

#right  {
  top: 60px;
  left: 540px;
}
.toppin { width: 32px; }
.bottompin { width: 32px; }

.rightpin { height: 17px; }

""";

def get_pins():
  pins = pinutils.scan_pin_file([], 'stm32f401.csv', 5, 8, 9)
  pins = pinutils.scan_pin_af_file(pins, 'stm32f401_af.csv', 0, 1)
  return pinutils.only_from_package(pinutils.fill_gaps_in_pin_list(pins), chip["package"])
