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
 'name' : "Espruino Bangle.js",
 'link' :  [ "http://www.espruino.com/Bangle.js" ],
 'espruino_page_link' : 'Bangle.js',
 'default_console' : "EV_BLUETOOTH",
 'variables' : 2100, # How many variables are allocated for Espruino to use. RAM will be overflowed if this number is too high and code won't compile.
 'bootloader' : 1,
 'binary_name' : 'espruino_%v_banglef5.hex',
 'build' : {
   'optimizeflags' : '-Os',
   'libraries' : [
     'BLUETOOTH',
     'TERMINAL',
     'GRAPHICS', 
     'LCD_SPI',
     'TENSORFLOW'     
   ],
   'makefile' : [
     'DEFINES += -DCONFIG_NFCT_PINS_AS_GPIOS', # Allow the reset pin to work
     'DEFINES += -DBUTTONPRESS_TO_REBOOT_BOOTLOADER',
     'DEFINES+=-DBLUETOOTH_NAME_PREFIX=\'"Bangle.js"\'',
     'DEFINES+=-DDUMP_IGNORE_VARIABLES=\'"g\\0"\'',
     'DEFINES+=-DUSE_FONT_6X8 -DGRAPHICS_PALETTED_IMAGES',
     'DFU_PRIVATE_KEY=targets/nrf5x_dfu/dfu_private_key.pem',
     'DFU_SETTINGS=--application-version 0xff --hw-version 52 --sd-req 0x8C',
     'INCLUDE += -I$(ROOT)/libs/banglejs -I$(ROOT)/libs/misc',
     'WRAPPERSOURCES += libs/banglejs/jswrap_banglef5.c',
     'SOURCES += libs/misc/nmea.c',
     'JSMODULESOURCES += libs/js/graphical_menu.min.js',
     'NRF_BL_DFU_INSECURE=1',
     'LINKER_BOOTLOADER=targetlibs/nrf5x_12/nrf5x_linkers/banglejs_dfu.ld',
     'LINKER_ESPRUINO=targetlibs/nrf5x_12/nrf5x_linkers/banglejs_espruino.ld'
   ]
 }
};


chip = {
  'part' : "NRF52832",
  'family' : "NRF52",
  'package' : "QFN48",
  'ram' : 64,
  'flash' : 512,
  'speed' : 64,
  'usart' : 1,
  'spi' : 1,
  'i2c' : 1,
  'adc' : 1,
  'dac' : 0,
  'saved_code' : {
    'address' : 0x40000000, # put this in external flash
    'page_size' : 4096,
    'pages' : 64, # 256kb - still loads left
    'flash_available' : 512 - ((31 + 8 + 2)*4) # Softdevice uses 31 pages of flash, bootloader 8, FS 2. Each page is 4 kb.
  },
};

devices = {

  'BTN1' : { 'pin' : 'D12', 'pinstate' : 'IN_PULLDOWN' }, # Top right -  Pin negated in software
  'BTN2' : { 'pin' : 'D13', 'pinstate' : 'IN_PULLDOWN' }, # Bottom right -  Pin negated in software
  'BTN3' : { 'pin' : 'D16' }, # Touch
#  'BTN4' : { 'pin' : 'D16', 'pinstate' : 'IN_PULLDOWN' }, # Pin negated in software
  'LED1' : { 'pin' : 'D14' }, # Pin negated in software
#  'LED2' : { 'pin' : 'D18' }, # Pin negated in software
#  'LED3' : { 'pin' : 'D19' }, # Pin negated in software
#  'LED4' : { 'pin' : 'D20' }, # Pin negated in software
  'VIBRATE' : { 'pin' : 'D11' }, # Pin negated in software
  'LCD' : {
            'width' : 128, 'height' : 96, 'bpp' : 4,
            'controller' : 'st7735',
            'pin_dc' : 'D22',
            'pin_cs' : 'D10',
            'pin_rst' : 'D23',
            'pin_sck' : 'D9',
            'pin_mosi' : 'D8',
            'pin_bl' : 'D21',
          },
  'GPS' : {
            'device' : 'M8130-KT',
            'pin_en' : 'D0', # inverted
            'pin_rx' : 'D5',
            'pin_tx' : 'D6'
          },
  'BAT' : {
            'pin_charging' : 'D7', # inverted
            'pin_voltage' : 'D4'
          },
  'HEARTRATE' : {
            'pin_led' : 'D14',
            'pin_analog' : 'D3'
          },
  'ACCEL' : {
            'device' : 'KX023', 'addr' : 0x1e,
            'pin_sda' : 'D1',
            'pin_scl' : 'D2'
          },
  'SPIFLASH' : {
            'pin_cs' : 'D18',
            'pin_sck' : 'D19',
            'pin_mosi' : 'D20',
            'pin_miso' : 'D17',
            'size' : 2097152
          },
  'PRESSURE' : {
            'device' : 'HP203', 'addr' : 0x76,
            'pin_sda' : 'D1',
            'pin_scl' : 'D2'
          },
};

# left-right, or top-bottom order
board = {
  'left' : [],
  'right' : [],
  '_notes' : {
  }
};
board["_css"] = """
#board {
  width: 528px;
  height: 800px;
  top: 0px;
  left : 200px;
  background-image: url(img/BANGLEF5.jpg);
}
#boardcontainer {
  height: 900px;
}

#left {
    top: 219px;
    right: 466px;
}
#right {
    top: 150px;
    left: 466px;
}

.leftpin { height: 17px; }
.rightpin { height: 17px; }
""";

def get_pins():
  pins = pinutils.generate_pins(0,31) # 32 General Purpose I/O Pins.
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
  # negate buttons
  pinutils.findpin(pins, "PD12", True)["functions"]["NEGATED"]=0; # btn1
  pinutils.findpin(pins, "PD13", True)["functions"]["NEGATED"]=0; # btn2
  pinutils.findpin(pins, "PD11", True)["functions"]["NEGATED"]=0; # vibrate
  pinutils.findpin(pins, "PD14", True)["functions"]["NEGATED"]=0; # HRM LED


  # everything is non-5v tolerant
  for pin in pins:
    pin["functions"]["3.3"]=0;
  #The boot/reset button will function as a reset button in normal operation. Pin reset on PD21 needs to be enabled on the nRF52832 device for this to work.
  return pins
