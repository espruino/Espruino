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
 'variables' : 7148,
 'bootloader' : 1,
 'binary_name' : 'espruino_%v_wifi.bin',
 'binaries' : [
  { 'filename' : 'espruino_%v_wifi.bin', 'description' : "Normal Espruino WiFi build"},
 ],
 'build' : {
   'optimizeflags' : '-Os',
   'libraries' : [
     'USB_HID',
     'NET',
     'GRAPHICS',
     'TV',
     'HASHLIB',
     'FILESYSTEM',
     'CRYPTO',
     'TLS',
     'NEOPIXEL'
   ],
   'makefile' : [
     'DEFINES+=-DUSE_USB_OTG_FS=1 -DESPRUINOWIFI',
     'STLIB=STM32F411xE',
     'PRECOMPILED_OBJS+=$(ROOT)/targetlibs/stm32f4/lib/startup_stm32f401xx.o'
   ]
  }
};

chip = {
  'part' : "STM32F411CEU6",
  'family' : "STM32F4",
  'package' : "UQFN48",
  'ram' : 128,
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
          },
  'ESP8266' : { 'pin_rx' :  'A2',
                'pin_tx' : 'A3',
                'pin_cts' : 'A15',
                'pin_ch_pd' : 'A14',
                'pin_gpio0' : 'A13',
   },
};

def rev(x):
    x.reverse();
    return x;

# left-right, or top-bottom order
board = {
  'left' : rev([ 'B15', 'B14', 'B13', 'B10', 'B1', 'A7', 'A6', 'A5', 'A4', 'A1', 'A0' ]),
  'right' : rev([ 'GND', 'VUSB', '3.3', 'B3', 'B4', 'B5', 'B6', 'B7','B8', 'B9', 'B0']),

  'left2' : ['A10','A8'],
  '_notes' : {
    'VUSB' : "This pin is connected directly to USB 5V - as such you should not use it to power Espruino WiFi *unless* the Micro USB port is unplugged",
    'B6' : "Serial Console TX when USB disconnected, use `USB.setConsole(true)` to avoid",
    'B7' : "Serial Console RX when USB disconnected, use `USB.setConsole(true)` to avoid",
    'A2' : 'ESP8266 RX',
    'A3' : 'ESP8266 TX',
    'A15' : 'ESP8266 CTS', # ESP8266 Clear to send (when enabled with AT+UART_DEF)
    'A14' : 'ESP8266 CH_PD', # ESP8266 Power Down, also ARM SWCLK (pull down by default)
    'A13' : 'ESP8266 GPIO0', # ESP8266 bootloader mode, also ARM SWDIO (pull up by default)
  }
};
board["_css"] = """
#board {
  width: 371px;
  height: 450px;
  top: 25px;
  left : 200px;
  background-image: url(img/ESPRUINOWIFI.png);
}
#boardcontainer {
  height: 500px;
}
#left {
  top: 10px;
  right: 370px;
}
#right {
  top: 10px;
  left: 370px;
}
#left2  {
  top: 380px;
  left: 60px;
}
.leftpin { height: 39px; }
.rightpin { height: 39px; }
.left2pin { height: 39px; }

""";

def get_pins():
  pins = pinutils.scan_pin_file([], 'stm32f401.csv', 5, 8, 9)
  pins = pinutils.scan_pin_af_file(pins, 'stm32f401_af.csv', 0, 1)
  return pinutils.only_from_package(pinutils.fill_gaps_in_pin_list(pins), chip["package"])
