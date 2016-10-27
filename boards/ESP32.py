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
 'espruino_page_link'       : 'EspruinoESP32',
 'default_console'          : "EV_SERIAL1",
 'default_console_baudrate' : "115200",
 'variables'                : 5000,
 'binary_name'              : 'espruino_%v_esp32',
 'build' : {
   'defines' : [
     'USE_NET'
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
  'usart'   : 2,
  'spi'     : 2,
  'i2c'     : 1,
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
board_esp12 = {
    'top' : ['D1', 'D3', 'D5', 'D4', 'D0', 'D2', 'D15', 'GND'],
    'bottom' : ['VCC', 'D13', 'D12', 'D14', 'B16', 'CH_EN', 'A0', 'RESET'],
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
board_esp01["_css"] = """
#board {
  width:  500px;
  height: 299px;
  left: 50px;
  top: 0px;
  background-image: url(img/ESP8266_01.jpg);
}
#boardcontainer {
  height: 300px;
}
#board #right {
  top: 30px;
  left: 200px;
}
#board #left {
  top: 65px;
  right: 80px;
}
#board #right  {
  top: 65px;
  left: 460px;
}
#board .leftpin {
  height: 48px;
}
#board .rightpin {
  height: 48px;
}
""";

boards = [ board_esp12, board_esp01 ];

def get_pins():

  pins = pinutils.generate_pins(0,19);

  pins.extend(pinutils.generate_pins(21,22));

  pins.extend(pinutils.generate_pins(25,27));

  pins.extend(pinutils.generate_pins(32,39));

  pinutils.findpin(pins, "PD36", True)["functions"]["ADC1_IN0"]=0;

  pinutils.findpin(pins, "PD37", True)["functions"]["ADC1_IN1"]=0;

  pinutils.findpin(pins, "PD38", True)["functions"]["ADC1_IN2"]=0;

  pinutils.findpin(pins, "PD39", True)["functions"]["ADC1_IN3"]=0;

  pinutils.findpin(pins, "PD32", True)["functions"]["ADC1_IN4"]=0;

  pinutils.findpin(pins, "PD33", True)["functions"]["ADC1_IN5"]=0;

  pinutils.findpin(pins, "PD34", True)["functions"]["ADC1_IN6"]=0;

  pinutils.findpin(pins, "PD35", True)["functions"]["ADC1_IN7"]=0;


  pinutils.findpin(pins, "PD4", True)["functions"]["ADC2_IN0"]=0;

  pinutils.findpin(pins, "PD0", True)["functions"]["ADC2_IN1"]=0;

  pinutils.findpin(pins, "PD2", True)["functions"]["ADC2_IN2"]=0;

  pinutils.findpin(pins, "PD15", True)["functions"]["ADC2_IN3"]=0;

  pinutils.findpin(pins, "PD13", True)["functions"]["ADC2_IN4"]=0;

  pinutils.findpin(pins, "PD12", True)["functions"]["ADC2_IN5"]=0;

  pinutils.findpin(pins, "PD14", True)["functions"]["ADC2_IN6"]=0;

  pinutils.findpin(pins, "PD27", True)["functions"]["ADC2_IN7"]=0;


  pinutils.findpin(pins, "PD0", True)["functions"]["LED_1"]=0;

  pinutils.findpin(pins, "PD10", True)["functions"]["USART0_TX"]=0;

  pinutils.findpin(pins, "PD17", True)["functions"]["USART2_TX"]=0;

  pinutils.findpin(pins, "PD32", True)["functions"]["USART0_RX"]=0;

  return pins