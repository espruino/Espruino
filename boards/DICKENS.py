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

# NOTES FROM REVERSE ENGINEERING
#====================================
# Flash init:
# w(0x6), while (r(5)&1);

# D2 and D8 and used together. Both outputs
# D8 set/cleared then 10ms delay

# D2 set then I2C device 0x70[0x38] contacted

# D0/D1 will be 32kHz crystal

# Button flex header
#   o GND       D28(btn)
#     VCC       D46(btn)
#     D3        D9
#     D29(btn)  D10
#     D42(btn)  GND

# unfitted big flash-ish chip
# o  D14    NC?
#    D15    D17
#    D2     D19
#    GND    D18      
# 

# unfitted header (assuming same dir as Button flex header)
#     VCC D10
#     VCC D9
#     FET  GND
#     GND D31
#      ?  D30
# ... there's an unpopulated footprint - likely a FET (not checked to see what IO this is connected to)

# Unknown pins:
# D8,D16,D17,D26,D32,D34,D35,D36,
# D37,D38,D39,D41,D44,D45,D47

#====================================

info = {
 'name' : "DICKENS",
 'link' :  [ "" ],
 'espruino_page_link' : '',
 'default_console' : "EV_BLUETOOTH",
 'variables' : 5000, # How many variables are allocated for Espruino to use. RAM will be overflowed if this number is too high and code won't compile.
 'io_buffer_size' : 512, 
 'bootloader' : 1,
 'binary_name' : 'espruino_%v_dickens.hex',
 'build' : {
   'optimizeflags' : '-Os',
   'libraries' : [
     'BLUETOOTH',
     'GRAPHICS',
     'LCD_SPI'
   ],
   'makefile' : [
#     'DEFINES += -DNRF_LOG_ENABLED=1 -DNRF_LOG_FILTERS_ENABLED=0',
     'DEFINES += -DCONFIG_NFCT_PINS_AS_GPIOS', # Allow the reset pin to work
     'DEFINES += -DNRF_BL_DFU_ENTER_METHOD_BUTTON=1 -DNRF_BL_DFU_ENTER_METHOD_BUTTON_PIN=29',
     'DEFINES += -DBUTTONPRESS_TO_REBOOT_BOOTLOADER',
     'DEFINES += -DESPR_BOOTLOADER_SPIFLASH', # Allow bootloader to flash direct from SPI flash

     'BOOTLOADER_SETTINGS_FAMILY = NRF52840',
     'DFU_PRIVATE_KEY=targets/nrf5x_dfu/dfu_private_key.pem',
     'DFU_SETTINGS=--application-version 0xff --hw-version 52 --sd-req 0xA9', # SD 6.0.0

     'DEFINES+=-DBLUETOOTH_NAME_PREFIX=\'"Dickens"\'',
     'DEFINES+=-DCUSTOM_GETBATTERY=jswrap_banglejs_getBattery',
     'DEFINES+=-DDUMP_IGNORE_VARIABLES=\'"g\\0"\'',
     'DEFINES+=-DUSE_FONT_6X8 -DGRAPHICS_PALETTED_IMAGES -DESPR_GRAPHICS_12BIT -DGRAPHICS_ANTIALIAS',
     'DEFINES+=-DNO_DUMP_HARDWARE_INITIALISATION', # don't dump hardware init - not used and saves 1k of flash
     'INCLUDE += -I$(ROOT)/libs/banglejs -I$(ROOT)/libs/misc',
     'WRAPPERSOURCES += libs/banglejs/jswrap_bangle.c',
     'WRAPPERSOURCES += libs/graphics/jswrap_font_architekt10.c',
     'WRAPPERSOURCES += libs/graphics/jswrap_font_architekt15.c',
     'WRAPPERSOURCES += libs/graphics/jswrap_font_architekt35.c',
     'WRAPPERSOURCES += libs/graphics/jswrap_font_grotesk14.c',
     'JSMODULESOURCES += libs/js/banglejs/locale.min.js',
     'DEFINES += -DBANGLEJS',

     'NRF_SDK15=1'
   ]
 }
};


chip = {
  'part' : "NRF52840",
  'family' : "NRF52",
  'package' : "QFN48",
  'ram' : 256,
  'flash' : 1024,
  'speed' : 64,
  'usart' : 2,
  'spi' : 1,
  'i2c' : 1,
  'adc' : 1,
  'dac' : 0,
  'saved_code' : {
#    'address' : ((246 - 10) * 4096), # Bootloader takes pages 248-255, FS takes 246-247
#    'page_size' : 4096,
#    'pages' : 10,
#    'flash_available' : 1024 - ((31 + 8 + 2 + 10)*4) # Softdevice uses 31 pages of flash, bootloader 8, FS 2, code 10. Each page is 4 kb.
    'address' : 0x60000000, # put this in external spiflash (see below)
    'page_size' : 4096,
    'pages' : 768, # 3MB of 4MB flash
    'flash_available' : 1024 - ((31 + 8 + 2)*4) # Softdevice uses 31 pages of flash, bootloader 8, FS 2. Each page is 4 kb.
  },
};

devices = {
  'BTN1' : { 'pin' : 'D46', 'pinstate' : 'IN_PULLDOWN' }, # BL ATMOS Pin negated in software
  'BTN2' : { 'pin' : 'D28', 'pinstate' : 'IN_PULLDOWN' }, # TL Pin negated in software
  'BTN3' : { 'pin' : 'D29', 'pinstate' : 'IN_PULLDOWN' }, # TR STATUM Pin negated in software
  'BTN4' : { 'pin' : 'D42', 'pinstate' : 'IN_PULLDOWN' }, # BR Pin negated in software

  'VIBRATE' : { 'pin' : 'D6' }, # Pin negated in software
  'LCD' : {
            'width' : 240, 'height' : 240, 'bpp' : 16, # 16 normal, 12 bit is possible
            'controller' : 'gc9a01',
            'pin_dc' : 'D7',
            'pin_cs' : 'D11',
            'pin_rst' : 'D40',
            'pin_sck' : 'D12',
            'pin_mosi' : 'D5',
            'pin_miso' : 'D27',
            'pin_en' : 'D43', 
            'pin_bl' : 'D33', # TESTED!
            'bitrate' : 32000000
          },
  'BAT' : {
            'pin_charging' : 'D13', 
            'pin_voltage' : 'D4'
          },
  'MAG' : { # Magnetometer/compass
           'device' : 'GMC303', 
           'addr' : 0x0C,
           'pin_sda' : 'D9',
           'pin_scl' : 'D10'
         },
  'ACCEL' : {
            'device' : 'KXTJ3_1057', 'addr' : 0x0e,
            'pin_sda' : 'D9',
            'pin_scl' : 'D10',
#           'pin_int' : '' # unknown
         },
 'PRESSURE' : {
           'device' : 'SPL06_007', 'addr' : 0x76,
           'pin_sda' : 'D9',
           'pin_scl' : 'D10',
        },
  # KX126-1063
  'SPIFLASH' : {
            'pin_cs' : 'D20',
            'pin_sck' : 'D25',
            'pin_mosi' : 'D22', # D0
            'pin_miso' : 'D23', # D1
            'pin_wp' : 'D21', # D2
            'pin_rst' : 'D24', # D3
            'size' : 4096*1024, 
            'memmap_base' : 0x60000000 # map into the address space (in software)
          }
};

# left-right, or top-bottom order
board = {
};
board["_css"] = """
""";

def get_pins():
  pins = pinutils.generate_pins(0,47) # 48 General Purpose I/O Pins.
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
  # Make buttons and LEDs negated
  pinutils.findpin(pins, "PD29", True)["functions"]["NEGATED"]=0; # ok
  pinutils.findpin(pins, "PD46", True)["functions"]["NEGATED"]=0; # ok
  pinutils.findpin(pins, "PD42", True)["functions"]["NEGATED"]=0; # ok
  pinutils.findpin(pins, "PD28", True)["functions"]["NEGATED"]=0; # ok
  

  # everything is non-5v tlerant
  for pin in pins:
    pin["functions"]["3.3"]=0;
  #The boot/reset button will function as a reset button in normal operation. Pin reset on PD21 needs to be enabled on the nRF52832 device for this to work.
  return pins
