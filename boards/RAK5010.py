#!/bin/false
# This file is part of Espruino, a JavaScript interpreter for Microcontrollers
#
# Copyright (C) 2020 Gordon Williams <gw@pur3.co.uk>
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



# Schematic https://doc.rakwireless.com/datasheet/rakproducts/schematic-diagram---rak5010-wistrio

info = {
 'name' : "RAK5010 WisTrio NB-IoT Tracker",
 'link' :  [ "https://doc.rakwireless.com/datasheet/rakproducts/interfaces---rak5010-wistrio-nb-iot" ],
 'espruino_page_link' : 'RAK5010',
 'variables' : 2500, # How many variables are allocated for Espruino to use. RAM will be overflowed if this number is too high and code won't compile.
# 'bootloader' : 1,
# 'default_console' : "EV_SERIAL1",
# 'default_console_tx' : "D34", # IO3
# 'default_console_rx' : "D33", # IO4
# 'default_console_baudrate' : "9600",
 'binary_name' : 'espruino_%v_RAK5010.hex',
 'build' : {
   'optimizeflags' : '-Os',
   'libraries' : [
     'BLUETOOTH',
     'NET'
   ],
   'makefile' : [
     'DEFINES+=-DCONFIG_GPIO_AS_PINRESET', # Allow the reset pin to work
     'DEFINES+=-DCONFIG_NFCT_PINS_AS_GPIOS', # Don't use NFC - the pins are used for GPS
#     'DEFINES += -DNRF_USB=1 -DUSB', # USB appears to think it is connected all the time
     'NRF_SDK15=1'
     'DEFINES+=-DBLUETOOTH_NAME_PREFIX=\'"iTracker"\'',
     'DFU_PRIVATE_KEY=targets/nrf5x_dfu/dfu_private_key.pem',
     'DFU_SETTINGS=--application-version 0xff --hw-version 52 --sd-req 0x8C',
     'JSMODULESOURCES += libs/js/AT.min.js',
     'JSMODULESOURCES += libs/js/GPS.min.js',
     'JSMODULESOURCES += libs/js/LIS3DH.min.js',
     'JSMODULESOURCES += libs/js/OPT3001.min.js',
     'JSMODULESOURCES += libs/js/ATSMS.min.js',
     'JSMODULESOURCES += libs/js/QuectelBG96.min.js',
     'JSMODULESOURCES += iTracker:libs/js/rak/RAK5010.js' # FIXME - use minified
   ]
 }
};

chip = {
  'part' : "NRF52840",
  'family' : "NRF52",
  'package' : "AQFN73",
  'ram' : 256,
  'flash' : 1024,
  'speed' : 64,
  'usart' : 2,
  'spi' : 3,
  'i2c' : 2,
  'adc' : 1,
  'dac' : 0,
  'saved_code' : {
    'address' : ((246 - 10) * 4096), # Bootloader takes pages 248-255, FS takes 246-247
    'page_size' : 4096,
    'pages' : 10,
    'flash_available' : 1024 - ((31 + 8 + 2 + 10)*4) # Softdevice uses 31 pages of flash, bootloader 8, FS 2, code 10. Each page is 4 kb.
  },
};

devices = {
  'LED1' : { 'pin' : 'D12' },
#  'BTN1' : { 'pin' : '', 'pinstate' : 'IN_PULLDOWN' },
# IS25WP064A QSPI flash
#  d0 D3 P0.03
#  d1    P1.15
#  d2    P1.14 
#  d3    P1.13
#  clk   P1.11
#  cs    P1.10
};

# left-right, or top-bottom order
board = {
 'right' : [ 'GND', 'VBAT', 'D5', 'D19', '',
             'VREF', 'D20', 'D34', 'D33', '',
             '3V', 'SWDIO', 'SWDCLK', 'GND' ],
  '_hide_not_on_connectors' : True,
  '_notes' : {
    'D21' : "Also RESET if configured",
    'D33' : "Labelled IO4 (requires VREF to be connected to 3V)",
    'D34' : "Labelled IO3 (requires VREF to be connected to 3V)",
    'D20' : "Labelled IO2 (requires VREF to be connected to 3V)",
    'D19' : "Labelled IO1 (requires VREF to be connected to 3V)",
    'D5' : "Labelled AIN"
  }
};

board["_css"] = """
#board {
  width: 480px;
  height: 526px;
  top: 0px;
  left : 0px;
  background-image: url(img/RAK5010.png);
}
#boardcontainer {
  height: 506px;
}
#right {
  top: 59px;
  left: 480px;
}
.rightpin {
  height: 26px;
}
""";

def get_pins():
  pins = pinutils.generate_pins(0,47) # 48 General Purpose I/O Pins.
  # Add pin functions
  pinutils.findpin(pins, "PD0", True)["functions"]["XL1"]=0;
  pinutils.findpin(pins, "PD1", True)["functions"]["XL2"]=0;
  pinutils.findpin(pins, "PD2", True)["functions"]["ADC1_IN0"]=0;
  pinutils.findpin(pins, "PD3", True)["functions"]["ADC1_IN1"]=0;
  pinutils.findpin(pins, "PD4", True)["functions"]["ADC1_IN2"]=0;
  pinutils.findpin(pins, "PD5", True)["functions"]["ADC1_IN3"]=0;
  pinutils.findpin(pins, "PD28", True)["functions"]["ADC1_IN4"]=0;
  pinutils.findpin(pins, "PD29", True)["functions"]["ADC1_IN5"]=0;
  pinutils.findpin(pins, "PD30", True)["functions"]["ADC1_IN6"]=0;
  pinutils.findpin(pins, "PD31", True)["functions"]["ADC1_IN7"]=0;

  # everything is non-5v tolerant
  for pin in pins:
    pin["functions"]["3.3"]=0;
  #The boot/reset button will function as a reset button in normal operation. Pin reset on PD21 needs to be enabled on the nRF52832 device for this to work.
  return pins
