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
 'name'                     : "ESP32",
 'espruino_page_link'       : 'ESP32',
 'default_console'          : "EV_SERIAL1",
 'default_console_baudrate' : "115200",
 'variables'                : 5000,
 'binary_name'              : 'espruino_%v_esp32.bin',
 'build' : {
   'optimizeflags' : '-Og',
   'libraries' : [
     'ESP32',
     'NET',
     'GRAPHICS',
     'CRYPTO',
     'TLS',
     'TELNET',
     'NEOPIXEL',
     'FILESYSTEM',
     'FLASHFS'	 
   ],
   'makefile' : [
     'DEFINES+=-DESP_PLATFORM -DESP32=1'
   ]
 }
};

chip = {
  'part'    : "ESP32",
  'family'  : "ESP32",
  'package' : "",
  'ram'     : 512,
  'flash'   : 0,
  'speed'   : 160,
  'usart'   : 3,
  'spi'     : 2,
  'i2c'     : 2,
  'adc'     : 1,
  'dac'     : 0,
  'saved_code' : {
    'address' : 0x100000,
    'page_size' : 4096,
    'pages' : 16,
    'flash_available' : 960, # firmware can be up to this size
  },
};
devices = {
};

# left-right, or top-bottom order
board_esp32 = {
   'top' : ['GND','D23','D22','D1','D3','D21','D20','D19','D18','D5','D17','D16','D4','D0'],
   'bottom' : ['D12','D14','D27','D26','D25','D33','D32','D35','D34','D39','D36','EN','3V3','GND'],
   'right' : [ 'GND','D13','D9','D10','D11','D6','D7','D8','D15','D2']
};
board_esp32["bottom"].reverse()
board_esp32["right"].reverse()
board_esp32["_css"] = """
#board {
  width:  600px;
  height: 435px;
  left: 50px;
  top: 170px;
  background-image: url(img/ESP32.jpg);
}
#boardcontainer {
  height: 700px;
}
#board #right {
  top: 80px;
  left: 600px;
}
#board #top {
  bottom: 440px;
  left: 155px;
}
#board #bottom  {
  top: 435px;
  left: 155px;
}
#board .rightpin {
  height: 28px;
}
#board .toppin, #board .bottompin {
  width: 24px;
}
""";

boards = [ board_esp32 ];

def get_pins():


#  { "name":"PD20", "sortingname":"D20", "port":"D", "num":"30", "functions":{ "I2C1_SDA":0 }, "csv":{} },


#  pins = pinutils.generate_pins(0,5);
##6-11 are used by Flash chip
#  pins.extend(pinutils.generate_pins(12,23));

# pins.extend(pinutils.generate_pins(25,27));
##32-33 are routed to rtc for xtal
#  pins.extend(pinutils.generate_pins(34,39));

#  pins = pinutils.fill_gaps_in_pin_list(pins);

  pins = pinutils.generate_pins(0,39) # 40 General Purpose I/O Pins.

  pinutils.findpin(pins, "PD36", True)["functions"]["ADC1_IN0"]=0;
  pinutils.findpin(pins, "PD37", True)["functions"]["ADC1_IN1"]=0;
  pinutils.findpin(pins, "PD38", True)["functions"]["ADC1_IN2"]=0;
  pinutils.findpin(pins, "PD39", True)["functions"]["ADC1_IN3"]=0;
  pinutils.findpin(pins, "PD32", True)["functions"]["ADC1_IN4"]=0;
  pinutils.findpin(pins, "PD33", True)["functions"]["ADC1_IN5"]=0;
  pinutils.findpin(pins, "PD34", True)["functions"]["ADC1_IN6"]=0;
  pinutils.findpin(pins, "PD35", True)["functions"]["ADC1_IN7"]=0;

#ADC2 not supported yet, waiting for driver from espressif
  pinutils.findpin(pins, "PD4", True)["functions"]["ADC2_IN0"]=0;
  pinutils.findpin(pins, "PD0", True)["functions"]["ADC2_IN1"]=0;
  pinutils.findpin(pins, "PD2", True)["functions"]["ADC2_IN2"]=0;
  pinutils.findpin(pins, "PD15", True)["functions"]["ADC2_IN3"]=0;
  pinutils.findpin(pins, "PD13", True)["functions"]["ADC2_IN4"]=0;
  pinutils.findpin(pins, "PD12", True)["functions"]["ADC2_IN5"]=0;
  pinutils.findpin(pins, "PD14", True)["functions"]["ADC2_IN6"]=0;
  pinutils.findpin(pins, "PD27", True)["functions"]["ADC2_IN7"]=0;

  pinutils.findpin(pins, "PD25", True)["functions"]["DAC_OUT1"]=0;

  pinutils.findpin(pins, "PD26", True)["functions"]["DAC_OUT2"]=0;

  pinutils.findpin(pins, "PD0", True)["functions"]["LED_1"]=0;

  pinutils.findpin(pins, "PD10", True)["functions"]["USART0_TX"]=0;
  pinutils.findpin(pins, "PD16", True)["functions"]["USART2_RX"]=0;
  pinutils.findpin(pins, "PD17", True)["functions"]["USART2_TX"]=0;
  pinutils.findpin(pins, "PD32", True)["functions"]["USART0_RX"]=0;

  # everything is non-5v tolerant
  #for pin in pins:
  #  pin["functions"]["3.3"]=0;
  return pins
