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
import os;
import pinutils;
info = {
 'name'            : "ESP8266",
 'espruino_page_link' : 'EspruinoESP8266',
 'default_console' : "EV_SERIAL1",
 'default_console_baudrate' : "115200",
 'variables'       : 1600,
 'binary_name'     : 'espruino_%v_esp8266_4mb',
 'build' : {
   'libraries' : [
     'NET',
     'TELNET',
     'GRAPHICS',
     'CRYPTO',
     'NEOPIXEL',
     #'FILESYSTEM',
     #'FLASHFS'
   ],
   'makefile' : [
     'FLASH_4MB=1',
     'ESP_FLASH_MAX=831488',
     'FLASH_BAUD=460800'    
    ]
 }
};
chip = {
  'part'    : "ESP8266",
  'family'  : "ESP8266",
  'package' : "",
  'ram'     : 80,
  'flash'   : 0,
  'speed'   : 160,
  'usart'   : 2,
  'spi'     : 1,
  'i2c'     : 1,
  'adc'     : 1,
  'dac'     : 0,
  'saved_code' : {
    # 0x000000 + 4096 * ( 256 -  48 save - 1 wifi  - 4 reserved ) 
    'address' :  0x0CB000, 
    'page_size' : 4096,
    'pages' : 48,
    'flash_available' : 812, # firmware can be up to this size
  },
};

devices = {
};

# left-right, or top-bottom order
board_esp12 = {
    'top' : ['D1', 'D3', 'D5', 'D4', 'D0', 'D2', 'D15', 'GND'],
    'bottom' : ['VCC', 'D13', 'D12', 'D14', 'D16', 'CH_EN', 'A0', 'RESET'],
    'right' : ['D11', 'D8', 'D9', 'D10', 'D7', 'D6'],
};
board_esp12["bottom"].reverse()
board_esp12["right"].reverse()
board_esp12["_css"] = """
#board {
  width:  600px;
  height: 384px;
  left: 50px;
  top: 100px;
  background-image: url(img/ESP8266_12.jpg);
}
#boardcontainer {
  height: 600px;
}
#board #right {
  top: 60px;
  left: 600px;
}
#board #top {
  bottom: 360px;
  left: 195px;
}
#board #bottom  {
  top: 365px;
  left: 195px;
}
#board .rightpin {
  height: 48px;
}
#board .toppin, #board .bottompin {
  width: 44px;
}
""";

# left-right, or top-bottom order
board_esp01 = {
    'left' : ['D3', 'D0', 'D2', 'GND'],
    'right' : ['VCC', 'RESET', 'CH_PD', 'D1'],
    '_hide_not_on_connectors' : True
};

boards = [ board_esp12 ];

def get_pins():
  # add pins D0 .. D16,A0
  pins = [
   { "name":"PD0",  "sortingname":"D00", "port":"D", "num":"0", "functions":{"LED_1":0}, "csv":{} },
   { "name":"PD1",  "sortingname":"D01", "port":"D", "num":"1", "functions":{"USART1_TX":0}, "csv":{} },
   { "name":"PD2",  "sortingname":"D02", "port":"D", "num":"2", "functions":{"USART2_TX":0}, "csv":{} },
   { "name":"PD3",  "sortingname":"D03", "port":"D", "num":"3", "functions":{"USART1_RX":0}, "csv":{} },
   { "name":"PD4",  "sortingname":"D04", "port":"D", "num":"4", "functions":{}, "csv":{} },
   { "name":"PD5",  "sortingname":"D05", "port":"D", "num":"5",  "functions":{}, "csv":{} }, 
   { "name":"PD6",  "sortingname":"D06", "port":"D", "num":"6",  "functions":{}, "csv":{} }, 
   { "name":"PD7",  "sortingname":"D07", "port":"D", "num":"7",  "functions":{}, "csv":{} }, 
   { "name":"PD8",  "sortingname":"D08", "port":"D", "num":"8",  "functions":{}, "csv":{} },
   { "name":"PD9",  "sortingname":"D09", "port":"D", "num":"9",  "functions":{}, "csv":{} }, 
   { "name":"PD10", "sortingname":"D10", "port":"D", "num":"10", "functions":{}, "csv":{} },
   { "name":"PD11", "sortingname":"D11", "port":"D", "num":"11", "functions":{}, "csv":{} },
   { "name":"PD12", "sortingname":"D12", "port":"D", "num":"12", "functions":{"SPI1_MISO":0}, "csv":{} },
   { "name":"PD13", "sortingname":"D13", "port":"D", "num":"13", "functions":{"SPI1_MOSI":0}, "csv":{} },
   { "name":"PD14", "sortingname":"D14", "port":"D", "num":"14", "functions":{"SPI1_SCK":0}, "csv":{} },
   { "name":"PD15", "sortingname":"D15", "port":"D", "num":"15", "functions":{}, "csv":{} },
   { "name":"PD16", "sortingname":"D16", "port":"D", "num":"16", "functions":{}, "csv":{} },
   { "name":"PA0", "sortingname":"A00",  "port":"A", "num":"17",   "functions":{ "ADC1_IN0":0 }, "csv":{} }
  ]
  return pins;
