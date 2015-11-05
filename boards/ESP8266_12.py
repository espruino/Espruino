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

# The ESP8266_12 represents esp8266 esp-12 modules with 1MB or more flash and using the OTA
# (over the air update) flash layout with two 512KB partitions in the first MB and the
# v1.4 or later bootloader at 0x0. Modules with >1MB flash will have a SPIFFS filesystem
# in the flash beyond the first 1MB. The pin layout described here corresponds to esp-12
# modules but this board can also be used for other modules that have 1MB of flash or more.

import pinutils;
info = {
 'name'            : "ESP8266-12",
 'espruino_page_link' : 'ESP8266_12',
 'default_console' : "EV_SERIAL1",
 'variables'       : 1023,
 'binary_name'     : 'espruino_esp8266_12',
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
  'i2c'     : 1,
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
    'top' : ['D1', 'D3', 'D5', 'D4', 'D0', 'D2', 'D15', 'GND'],
    'bottom' : ['VCC', 'D13', 'D12', 'D14', 'B16', 'CH_EN', 'A0', 'RESET'],
    'right' : ['D11', 'D8', 'D9', 'D10', 'D7', 'D6'],
};
board["bottom"].reverse()
board["right"].reverse()
devices = {
    'LED1' : { 'pin': 'D0' },
};

board_css = """
#board {
  width:  600px;
  height: 384px;
  left: 50px;
  top: 200px;
  background-image: url(img/ESP8266_12.jpg);
}
#boardcontainer {
  height: 800px;
}
#right {
  top: 30px;
  left: 600px;
}
#top {
  top: 10px;
  left: 197px;
}
#bottom  {
  top: 360px;
  left: 197px;
}
.rightpin {
  margin: 28px 0px;
}
.toppin, .bottompin {
  margin: 0px 12px;
}
""";

def get_pins():
  pins = pinutils.generate_pins(0,15)
  pinutils.findpin(pins, "PD0", True)["functions"]["LED_1"]=0;
  # PA0 is the analog input pin
  #pinutils.findpin(pins, "PA0", False)["functions"]["ADC"]=0;
  # PB16 is the RTC GPIO pin, also called GPIO16
  #pinutils.findpin(pins, "PB16", False)

  #for pin in pins:
  #  pin["functions"] = {
  #    "I2C1_SCL": "JSH_I2C1_SCL",
  #    "I2C1_SDA": "JSH_I2C1_SDA",
  #    "ADC": 0,
  #  }

  return pins
