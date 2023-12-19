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

# TODO:
# Move to SDK17 with proper nRF52833 support
#   - expand RAM to 128k
# Proper event handling for accelerometer/etc
# Functions for sound

info = {
 'name' : "BBC micro:bit 2",
 'link' :  [ "https://en.wikipedia.org/wiki/Micro_Bit" ],
 'espruino_page_link' : 'MicroBit',
  # This is the PCA10036
 'default_console' : "EV_SERIAL1",
 'default_console_tx' : "H1",
 'default_console_rx' : "H0",
 'default_console_baudrate' : "9600",
 'variables' : 6000, # How many variables are allocated for Espruino to use. RAM will be overflowed if this number is too high and code won't compile.
 'binary_name' : 'espruino_%v_microbit2.hex',
 'build' : {
   'optimizeflags' : '-Os',
   'libraries' : [
     'BLUETOOTH',
#     'NET',
     'GRAPHICS',
     'NEOPIXEL',
     'TENSORFLOW'
   ],
   'makefile' : [
     'DEFINES += -DCONFIG_GPIO_AS_PINRESET', # Allow the reset pin to work
     'DEFINES += -DNEOPIXEL_SCK_PIN=27 -DNEOPIXEL_LRCK_PIN=18', # SCK pin needs defining as something unused for neopixel (HW errata means they can't be disabled) 
     'DEFINES += -DGPIO_COUNT=2 -DP1_PIN_NUM=16 -DNRF_P1_BASE=0x50000300UL "-DNRF_P1=((NRF_GPIO_Type*)NRF_P1_BASE)"', # Hack for 52833 on SDK12
     'DEFINES += -DMICROBIT', # enable microbit-specific stuff
     'INCLUDE += -I$(ROOT)/libs/microbit',
     'WRAPPERSOURCES += libs/microbit/jswrap_microbit.c',
     'DEFINES+=-DNEOPIXEL_SCK_PIN=27', # an unused pin
     'LDFLAGS += -Xlinker --defsym=LD_SRAM_SIZE=0x20000' #
   ]
 }
};


chip = {
  'part' : "NRF52832", # actually 52833 but we're using SDK12 for this at the moment, and it doesn't support it
  'family' : "NRF52",
  'package' : "QFN48",
  'ram' : 64,
  'flash' : 512,
  'speed' : 64, # TODO: actually 128k
  'usart' : 1,
  'spi' : 1,
  'i2c' : 1,
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
  'BTN1' : { 'pin' : 'D5', 'pinstate' : 'IN_PULLDOWN' }, # Pin negated in software
  'BTN2' : { 'pin' : 'D11', 'pinstate' : 'IN_PULLDOWN' }, # Pin negated in software
#  'BTN3' : { 'pin' : 'H2', 'pinstate' : 'IN' }, # Pin negated in software
  # 'V' pins are virtual
  'LED1' : { 'pin' : 'V0' },
};

# left-right, or top-bottom order
board = {
  'bottom' : [ 'D3', '','D0','','D4','D5','D6','D7','','D1','','D8','D9','D10','D11','D12','','D2','',
               'D13','D14','D15','D16','3.3','','3.3','','3.3','D19','D20','GND','','GND','','GND' ],
  '_hide_not_on_connectors' : True,
  '_notes' : {
    'D3'  : "LED Row 3",
    'D4'  : "LED Row 1",
    'D5'  : "BTN1",
    'D6'  : "LED Row 4",
    'D7'  : "LED Row 2",
    'D10'  : "LED  Row 5",
    'D11'  : "BTN2",
  }
};
board["_css"] = """
#board {
  width: 659px;
  height: 562px;
  top: 0px;
  left : 0px;
  background-image: url(img/MICROBIT2.jpg);
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

def get_pins():
  pins = [
   { "name":"PD0", "sortingname":"D00", "port":"D", "num":"2", "functions":{ "ADC1_IN0":0 }, "csv":{} }, # RING0
   { "name":"PD1", "sortingname":"D01", "port":"D", "num":"3", "functions":{ "ADC1_IN1":0 }, "csv":{} }, # RING1
   { "name":"PD2", "sortingname":"D02", "port":"D", "num":"4", "functions":{ "ADC1_IN2":0 }, "csv":{} }, # RING2
   { "name":"PD3", "sortingname":"D03", "port":"D", "num":"31", "functions":{  }, "csv":{} },   # COLR3
   { "name":"PD4", "sortingname":"D04", "port":"D", "num":"28", "functions":{  }, "csv":{} },  # COLR1
   { "name":"PD5", "sortingname":"D05", "port":"D", "num":"14", "functions":{}, "csv":{} },   # BTN1
   { "name":"PD6", "sortingname":"D06", "port":"D", "num":"37", "functions":{}, "csv":{} },  # COLR4
   { "name":"PD7", "sortingname":"D07", "port":"D", "num":"11", "functions":{}, "csv":{} },  # COLR2
   { "name":"PD8", "sortingname":"D08", "port":"D", "num":"10", "functions":{}, "csv":{} }, # GPIO1
   { "name":"PD9", "sortingname":"D09", "port":"D", "num":"9", "functions":{}, "csv":{} },  # GPIO2
   { "name":"PD10", "sortingname":"D10", "port":"D", "num":"30", "functions":{  }, "csv":{} },  # COLR5
   { "name":"PD11", "sortingname":"D11", "port":"D", "num":"23", "functions":{  }, "csv":{} }, # BTN2
   { "name":"PD12", "sortingname":"D12", "port":"D", "num":"12", "functions":{}, "csv":{} }, # GPIO4
   { "name":"PD13", "sortingname":"D13", "port":"D", "num":"17", "functions":{ "SPI1_SCK":0 }, "csv":{} },
   { "name":"PD14", "sortingname":"D14", "port":"D", "num":"1", "functions":{ "SPI1_MISO":0 }, "csv":{} },
   { "name":"PD15", "sortingname":"D15", "port":"D", "num":"13", "functions":{ "SPI1_MOSI":0 }, "csv":{} },
   { "name":"PD16", "sortingname":"D16", "port":"D", "num":"34", "functions":{}, "csv":{} }, # GPIO3
   { "name":"PD17", "sortingname":"D17", "port":"D", "num":"27", "functions":{}, "csv":{} }, # FIXME 3.3v - using an unused pin
   { "name":"PD18", "sortingname":"D18", "port":"D", "num":"27", "functions":{}, "csv":{} }, # FIXME 3.3v - using an unused pin
   { "name":"PD19", "sortingname":"D19", "port":"D", "num":"26", "functions":{ "I2C1_SCL":0 }, "csv":{} },
   { "name":"PD20", "sortingname":"D20", "port":"D", "num":"32", "functions":{ "I2C1_SDA":0 }, "csv":{} },
   { "name":"PH0", "sortingname":"H00", "port":"H", "num":"40", "functions":{"USART1_RX" : 0}, "csv":{} }, # UART_RX - receive into Espruino
   { "name":"PH1", "sortingname":"H01", "port":"H", "num":"6", "functions":{"USART1_TX" : 0}, "csv":{} }, # UART_TX
   { "name":"PH2", "sortingname":"H02", "port":"H", "num":"36", "functions":{}, "csv":{} }, # face touch
   { "name":"PH3", "sortingname":"H03", "port":"H", "num":"0", "functions":{}, "csv":{} }, # speaker
   { "name":"PH4", "sortingname":"H04", "port":"H", "num":"5", "functions":{ "ADC1_IN3":0 }, "csv":{} }, # mic_in
   { "name":"PH5", "sortingname":"H05", "port":"H", "num":"20", "functions":{}, "csv":{} }, # run_mic
   { "name":"PH6", "sortingname":"H06", "port":"H", "num":"16", "functions":{}, "csv":{} }, # INT_SDA
   { "name":"PH7", "sortingname":"H07", "port":"H", "num":"8", "functions":{}, "csv":{} }, # INT_SCL
   { "name":"PH8", "sortingname":"H08", "port":"H", "num":"25", "functions":{}, "csv":{} }, # INT
   { "name":"PV0", "sortingname":"V0", "port":"V", "num":"0", "functions":{}, "csv":{} } # LED virtual pin
  ];
  #21 # ROW1
  #22 # ROW2
  #15 # ROW3
  #24 # ROW4
  #19 # ROW5

  # Make buttons negated
  pinutils.findpin(pins, "PD5", True)["functions"]["NEGATED"]=0;
  pinutils.findpin(pins, "PD11", True)["functions"]["NEGATED"]=0;

  # everything is non-5v tolerant
  for pin in pins:
    pin["functions"]["3.3"]=0;
  #The boot/reset button will function as a reset button in normal operation. Pin reset on PD21 needs to be enabled on the nRF52832 device for this to work.
  return pins
