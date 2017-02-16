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
 'link' : [ "https://en.wikipedia.org/wiki/Micro_Bit" ],
 'espruino_page_link' : 'MicroBit',
 'default_console' : "EV_SERIAL1",
 'default_console_tx' : "H0", # pin 24
 'default_console_rx' : "H1", # pin 25
 'default_console_baudrate' : "9600",
 'variables' : 300,
 'binary_name' : 'espruino_%v_microbit.hex',
 'build' : {
  'defines' : [
     'USE_GRAPHICS',
     'USE_BLUETOOTH',
   ]
 }
};
chip = {
  'part' : "NRF51822",
  'family' : "NRF51",
  'package' : "QFN48",
  'ram' : 16,
  'flash' : 256,
  'speed' : 16,
  'usart' : 1,
  'spi' : 1,
  'i2c' : 1,
  'adc' : 0,
  'dac' : 0,
   # If using DFU bootloader, it sits at 0x3C000 - 0x40000 (0x40000 is end of flash)
   # Might want to change 256 -> 240 in the code below
  'saved_code' : {
    'address' : ((256 - 3) * 1024),
    'page_size' : 1024,
    'pages' : 3,
    'flash_available' : (256 - 108 - 3) # total flash pages - softdevice - saved code
  }
};

devices = {
  'BTN1' : { 'pin' : 'D5', 'inverted' : True, 'pinstate' : 'IN_PULLUP' }, # 'P0_17'
  'BTN2' : { 'pin' : 'D11', 'inverted' : True, 'pinstate' : 'IN_PULLUP' }, # 'P0_26'
};

# left-right, or top-bottom order
board = {
  'bottom' : [ 'D3', '','D0','','D4','D5','D6','D7','','D1','','D8','D9','D10','D11','D12','','D2','',
               'D13','D14','D15','D16','3.3','','3.3','','3.3','D19','D20','GND','','GND','','GND' ],
  '_hide_not_on_connectors' : True,
  '_notes' : {
    'D3'  : "LED Col 1",
    'D4'  : "LED Col 2",
    'D6'  : "LED Col 9",
    'D7'  : "LED Col 8",
    'D9'  : "LED Col 7",
    'D10'  : "LED  Col 3"
  }
};
board["_css"] = """
#board {
  width: 659px;
  height: 562px;
  top: 0px;
  left : 0px;
  background-image: url(img/MICROBIT.jpg);
}
#boardcontainer {
  height: 700px;
}

#bottom {
  top: 490px;
  left: 52px;
}

.bottompin { width: 10px; }
""";

# Display is on:
# real NRF pins 4,5,6,7,8,9,10,11,12 (column pull down)
# real NRF pins 13,14,15 (row pull up)

def get_pins():
  pins = [
   { "name":"PD0", "sortingname":"D00", "port":"D", "num":"1", "functions":{ "ADC1_IN4":0 }, "csv":{} },
   { "name":"PD1", "sortingname":"D01", "port":"D", "num":"2", "functions":{ "ADC1_IN3":0 }, "csv":{} },
   { "name":"PD2", "sortingname":"D02", "port":"D", "num":"3", "functions":{ "ADC1_IN2":0 }, "csv":{} },
   { "name":"PD3", "sortingname":"D03", "port":"D", "num":"4", "functions":{ "ADC1_IN5":0 }, "csv":{} },   # LED col 1
   { "name":"PD4", "sortingname":"D04", "port":"D", "num":"5", "functions":{}, "csv":{} },  # BTNA
   { "name":"PD5", "sortingname":"D05", "port":"D", "num":"17", "functions":{ "ADC1_IN6":0 }, "csv":{} },   # LED col 2
   { "name":"PD6", "sortingname":"D06", "port":"D", "num":"12", "functions":{}, "csv":{} },  # LED row 2
   { "name":"PD7", "sortingname":"D07", "port":"D", "num":"11", "functions":{}, "csv":{} },  # LED row 1
   { "name":"PD8", "sortingname":"D08", "port":"D", "num":"18", "functions":{}, "csv":{} },
   { "name":"PD9", "sortingname":"D09", "port":"D", "num":"10", "functions":{}, "csv":{} },  # LED row 3
   { "name":"PD10", "sortingname":"D10", "port":"D", "num":"6", "functions":{ "ADC1_IN7":0 }, "csv":{} },  # LED col 3
   { "name":"PD11", "sortingname":"D11", "port":"D", "num":"26", "functions":{}, "csv":{} }, # BTNB
   { "name":"PD12", "sortingname":"D12", "port":"D", "num":"20", "functions":{}, "csv":{} },
   { "name":"PD13", "sortingname":"D13", "port":"D", "num":"23", "functions":{ "SPI1_SCK":0 }, "csv":{} },
   { "name":"PD14", "sortingname":"D14", "port":"D", "num":"22", "functions":{ "SPI1_MISO":0 }, "csv":{} },
   { "name":"PD15", "sortingname":"D15", "port":"D", "num":"21", "functions":{ "SPI1_MOSI":0 }, "csv":{} },
   { "name":"PD16", "sortingname":"D16", "port":"D", "num":"16", "functions":{}, "csv":{} },
   { "name":"PD17", "sortingname":"D17", "port":"D", "num":"31", "functions":{}, "csv":{} }, # FIXME 3.3v
   { "name":"PD18", "sortingname":"D18", "port":"D", "num":"31", "functions":{}, "csv":{} }, # FIXME 3.3v
   { "name":"PD19", "sortingname":"D19", "port":"D", "num":"0", "functions":{ "I2C1_SCL":0, "ADC1_IN0":0 }, "csv":{} },
   { "name":"PD20", "sortingname":"D20", "port":"D", "num":"30", "functions":{ "I2C1_SDA":0 }, "csv":{} },
   { "name":"PH0", "sortingname":"H0", "port":"D", "num":"24", "functions":{}, "csv":{} },
   { "name":"PH1", "sortingname":"H1", "port":"D", "num":"25", "functions":{}, "csv":{} }
  ];
  # everything is non-5v tolerant 
  for pin in pins:
    pin["functions"]["3.3"]=0;
  return pins
