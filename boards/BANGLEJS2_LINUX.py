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
 'name' : "Bangle.js 2 test build for Linux",
 'link' :  [ "https://www.espruino.com/Bangle.js2" ],
 'espruino_page_link' : 'SMAQ3',
 'default_console' : "EV_USBSERIAL",
 #'default_console' : "EV_SERIAL1",
# 'default_console_tx' : "D6",
# 'default_console_rx' : "D8",
# 'default_console_baudrate' : "9600",
 'variables' : 12000, # How many variables are allocated for Espruino to use. RAM will be overflowed if this number is too high and code won't compile.
 'bootloader' : 1,
 'binary_name' : 'espruino_banglejs2',
 'build' : {
   'optimizeflags' : '-Os',
   'libraries' : [
     'TERMINAL',
     'GRAPHICS',
     'FILESYSTEM', # for writing screenshots/etc
     'LCD_MEMLCD',
#     'TENSORFLOW'  
   ],
   'makefile' : [
     'LINUX=1',
     'DEFINES += -DESPR_HWVERSION=2 -DBANGLEJS -DBANGLEJS_Q3 -DEMULATED',
     'DEFINES += -DBLUETOOTH_NAME_PREFIX=\'"Bangle.js"\'',
     'DEFINES += -DDUMP_IGNORE_VARIABLES=\'"g\\0"\'',
     'DEFINES+=-DESPR_GRAPHICS_INTERNAL=1',
     'DEFINES += -DUSE_FONT_6X8 -DGRAPHICS_PALETTED_IMAGES -DESPR_GRAPHICS_3BIT',
     'DEFINES += -DNO_DUMP_HARDWARE_INITIALISATION', # don't dump hardware init - not used and saves 1k of flash
     'DEFINES += -DESPR_NO_LINE_NUMBERS=1', # we execute mainly from flash, so line numbers can be worked out
     'INCLUDE += -I$(ROOT)/libs/banglejs -I$(ROOT)/libs/misc',
     'WRAPPERSOURCES += libs/banglejs/jswrap_bangle.c',
     'WRAPPERSOURCES += libs/graphics/jswrap_font_6x15.c',
     'SOURCES += libs/misc/nmea.c',
     'SOURCES += libs/misc/stepcount.c',
     'SOURCES += libs/misc/heartrate.c',
     'SOURCES += libs/misc/hrm_emulated.c',
     'SOURCES += libs/banglejs/banglejs2_storage_default.c',
     'DEFINES += -DESPR_STORAGE_INTITIAL_CONTENTS=1', #
     'JSMODULESOURCES += libs/js/banglejs/locale.min.js',
   ]
 }
};


chip = {
  'part' : "LINUX",
  'family' : "LINUX",
  'package' : "",
  'ram' : 256,
  'flash' : 1024,
  'speed' : 64,
  'usart' : 2,
  'spi' : 1,
  'i2c' : 1,
  'adc' : 1,
  'dac' : 0
};

devices = {
  'USB' : {}, # to convince code that we have a USB port (it's used for the console ion Linux)
  'BTN1' : { 'pin' : 'D17', 'pinstate' : 'IN_PULLDOWN' }, # Pin negated in software
  'LED1' : { 'pin' : 'D8', 'novariable':True }, # Backlight flash for low level debug - but in code we just use 'fake' LEDs
  'LCD' : {
            'width' : 176, 'height' : 176, 
            'bpp' : 3,
            'controller' : 'LPM013M126', # LPM013M126C
            'pin_cs' : 'D5',
            'pin_extcomin' : 'D6',
            'pin_disp' : 'D7',
            'pin_sck' : 'D26',
            'pin_mosi' : 'D27',
            'pin_bl' : 'D8',
          },
  'TOUCH' : {
            'device' : 'EMULATED', 'addr' : 0x15,
            'pin_sda' : 'D33',
            'pin_scl' : 'D34',
            'pin_rst' : 'D35',
            'pin_irq' : 'D36'
          },
  'VIBRATE' : { 'pin' : 'D19' },
  'BAT' : {
            'pin_charging' : 'D23', # active low
            'pin_voltage' : 'D3'
          }
};

# left-right, or top-bottom order
board = {
};
board["_css"] = """
#board {
  width: 528px;
  height: 800px;
  top: 0px;
  left : 200px;
  background-image: url(img/NRF528DK.jpg);
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
  pins = pinutils.generate_pins(0,47) # 48 General Purpose I/O Pins.
  pinutils.findpin(pins, "PD0", True)["functions"]["XL1"]=0;
  pinutils.findpin(pins, "PD1", True)["functions"]["XL2"]=0;
  pinutils.findpin(pins, "PD9", True)["functions"]["NFC1"]=0;
  pinutils.findpin(pins, "PD10", True)["functions"]["NFC2"]=0;
  pinutils.findpin(pins, "PD2", True)["functions"]["ADC1_IN0"]=0;
  pinutils.findpin(pins, "PD3", True)["functions"]["ADC1_IN1"]=0;
  pinutils.findpin(pins, "PD4", True)["functions"]["ADC1_IN2"]=0;
  pinutils.findpin(pins, "PD5", True)["functions"]["ADC1_IN3"]=0;
  pinutils.findpin(pins, "PD28", True)["functions"]["ADC1_IN4"]=0;
  pinutils.findpin(pins, "PD29", True)["functions"]["ADC1_IN5"]=0;
  pinutils.findpin(pins, "PD30", True)["functions"]["ADC1_IN6"]=0;
  pinutils.findpin(pins, "PD31", True)["functions"]["ADC1_IN7"]=0;
  # Make buttons and LEDs negated
  pinutils.findpin(pins, "PD17", True)["functions"]["NEGATED"]=0; # button

  # everything is non-5v tolerant
  for pin in pins:
    pin["functions"]["3.3"]=0;
  #The boot/reset button will function as a reset button in normal operation. Pin reset on PD21 needs to be enabled on the nRF52832 device for this to work.
  return pins
