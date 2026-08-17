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
 'name'                     : "Espruino ePaper Badge",
 'espruino_page_link'       : 'Badge',
 'default_console'          : "EV_USBSERIAL", # USB only no serial, see ESPR_USE_USB_SERIAL_JTAG
 'default_console_baudrate' : "115200",
 'variables'                : 4095,
 'io_buffer_size'           : 2048, # How big is the input buffer (in bytes). Default on nRF52 is 1024
 'binary_name'              : 'espruino_%v_epaper_badge.bin',
 'build' : {
   'optimizeflags' : '-Og',
   'libraries' : [
     'ESP32',
     'NET',
     'GRAPHICS',
     'CRYPTO','SHA256','SHA512',
     'TLS',
#     'TELNET',
#     'FILESYSTEM',
     'BLUETOOTH',
     'NEOPIXEL'
   ],
   'makefile' : [
     'DEFINES+=-DBLUETOOTH_NAME_PREFIX="Espruino"',
     'DEFINES+=-DESP_PLATFORM -DESP32=1 -DESP32C3 -DESP32C3_IDF4',
     'DEFINES+=-DESP_STACK_SIZE=15000',
     'DEFINES+=-DESP_HEAP_SIZE=70000', # enough for HTTPS
     'DEFINES+=-DJSVAR_MALLOC', # Allocate space for variables at jsvInit time
     'DEFINES+=-DUSE_FONT_6X8',
     'DEFINES+=-DESPR_USE_USB_SERIAL_JTAG -DUSB', # Use on-chip USB. See ESPR_USE_USB_SERIAL_JTAG in README_BuildProcess.md
     'ESP32_FLASH_MAX=1572864',
     'JSMODULESOURCES += _:libs/js/misc/epaper_badge.js'
   ]
 }
};

chip = {
  'part'    : "ESP32C3",
  'family'  : "ESP32_IDF4",
  'package' : "",
  'ram'     : 400,
  'flash'   : 0,
  'speed'   : 160,
  'usart'   : 2,
  'spi'     : 1,
  'i2c'     : 1,
  'adc'     : 2,
  'dac'     : 0,
  'saved_code' : {
    'address' : 0x320000,
    'page_size' : 4096,
    'pages' : 224, # 896kb - see partitions_espruino.csv
    'flash_available' : 1344, # firmware can be up to this size - see partitions_espruino.csv
  },
};
devices = {
  'BTN1' : { 'pin' : 'D3' },
  'BTN2' : { 'pin' : 'D2' },
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
  background-image: url(img/ESP32C3.jpg);
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
  pins = pinutils.generate_pins(0,21) # 22 General Purpose I/O Pins.

  pinutils.findpin(pins, "PD0", True)["functions"]["ADC1_IN0"]=0;
  pinutils.findpin(pins, "PD1", True)["functions"]["ADC1_IN1"]=0;
  pinutils.findpin(pins, "PD2", True)["functions"]["ADC1_IN2"]=0;
  pinutils.findpin(pins, "PD3", True)["functions"]["ADC1_IN3"]=0;
  pinutils.findpin(pins, "PD4", True)["functions"]["ADC1_IN4"]=0;
  # pinutils.findpin(pins, "PD5", True)["functions"]["ADC2_IN0"]=0;
  # On supermini D8 is (inverted) LED
  # On supermini D9 is (inverted) Button
  # D12-D17 are SPI (internal SPI) - not sure they should even be exposed??

  pinutils.findpin(pins, "PD18", True)["functions"]["USB"]=0; # D-
  pinutils.findpin(pins, "PD19", True)["functions"]["USB"]=0; # D+
  pinutils.findpin(pins, "PD20", True)["functions"]["USART1_RX"]=0;
  pinutils.findpin(pins, "PD21", True)["functions"]["USART1_TX"]=0;
  pinutils.findpin(pins, "PD9", True)["functions"]["I2C1_SCL"]=0; # added for issue #2589 fix
  pinutils.findpin(pins, "PD8", True)["functions"]["I2C1_SDA"]=0; # added for issue #2589 fix

  # SPI added for issue #2601
  # See esp-idf-4 /components/soc/esp32c3/include/soc/soc_caps.h
  pinutils.findpin(pins, "PD6", True)["functions"]["SPI1_SCK"]=0;
  pinutils.findpin(pins, "PD2", True)["functions"]["SPI1_MISO"]=0;
  pinutils.findpin(pins, "PD7", True)["functions"]["SPI1_MOSI"]=0;

  pinutils.findpin(pins, "PD3", True)["functions"]["NEGATED"]=0; # BTN1 negate
  pinutils.findpin(pins, "PD2", True)["functions"]["NEGATED"]=0; # BTN2 negate

  # everything is non-5v tolerant
  for pin in pins:
    pin["functions"]["3.3"]=0;
  return pins
