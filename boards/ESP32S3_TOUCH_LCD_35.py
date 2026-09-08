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
# FIXME:
#   LCD speed (currently 8mhz?)
#   LCD reflected l/r
#   No IO expander (need virtual pins?)
#   No touch/SD


import pinutils;
info = {
 'name'                     : "Waveshare ESP32-S3-Touch-LCD-3.5",
 #https://docs.waveshare.com/ESP32-S3-Touch-LCD-3.5/Resources-And-Documents
 #https://files.waveshare.com/wiki/ESP32-S3-Touch-LCD-3.5/ESP32-S3-Touch-LCD-3.5-Schematic.pdf
 'espruino_page_link'       : 'ESP32',
 'default_console'          : "EV_SERIAL1", # USB + Serial1 - see ESPR_USE_USB_SERIAL_JTAG
 'default_console_baudrate' : "115200",
 'variables'                : 4095, 
 'io_buffer_size'           : 4096, # How big is the input buffer (in bytes). Default on nRF52 is 1024
 'binary_name'              : 'espruino_%v_esp32s3_touch_lcd35.bin',
 'build' : {
   'optimizeflags' : '-Og',
   'libraries' : [
     'ESP32',
     'NET',
     'GRAPHICS',
     'LCD_SPI_UNBUF',
     'CRYPTO','SHA256', 'SHA512',
     'TLS',
     'TELNET',
     'NEOPIXEL',
     'FILESYSTEM',
     'BLUETOOTH'
   ],
   'makefile' : [
     'DEFINES+=-DESP_PLATFORM -DESP32=1',
     'DEFINES+=-DESP_STACK_SIZE=25000',
     'DEFINES+=-DJSVAR_MALLOC', # Allocate space for variables at jsvInit time
     'DEFINES+=-DESPR_GRAPHICS_INTERNAL -DESPR_GRAPHICS_SELF_INIT', # ensure graphics instantiates itself
     'DEFINES+=-DUSE_FONT_6X8 -DSPISENDMANY_BUFFER_SIZE=1600',
     'DEFINES+=-DESPR_USE_USB_SERIAL_JTAG -DUSB', # Use on-chip USB. See ESPR_USE_USB_SERIAL_JTAG in README_BuildProcess.md
     'ESP32_FLASH_MAX=1572864'
   ]
 }
};

chip = {
  'part'    : "ESP32S3",
  'family'  : "ESP32_IDF5",
  'package' : "",
  'ram'     : 512,
  'flash'   : 0,
  'speed'   : 240,
  'usart'   : 3,
  'spi'     : 2,
  'i2c'     : 2,
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
  'BTN1' : { 'pin' : 'D0' },
#  'SD' :  { #'pin_cs' :  'E3',# EXIO3
#            'pin_di' :  'D10',
#            'pin_do' :  'D9',
#            'pin_clk' : 'D11' },  
  'TOUCH' : {
            'device' : 'XPT2046',
#            'pin_irq' : 'E2', # EXIO2
            'pin_sda' : 'D8',
            'pin_scl' : 'D7'
          },
  'LCD' : {
            'width' : 320, 'height' : 480, 'bpp' : 16, 'controller' : 'ST7796',
            'pin_dc' : 'D3',
            'pin_cs' : 'D4', # this may not be connected. schematic doesn't show it but D4 is free anyway
            'pin_sck' : 'D5',
            'pin_mosi' : 'D1',
            'pin_miso' : 'D2',
            'pin_bl' : 'D6',
#           'pin_rst' : 'E1',# EXIO1
            'spi_device' : 'EV_SPI1'
          },
  'MISC' : {
    'pin_ioexp_sda' : 'D8',
    'pin_ioexp_scl' : 'D7',
  }
};

boards = [ ];

def get_pins():
  # ESP32-S3 has 45 Physical GPIO pins Numbered 0->21 and 26->48
  # see https://www.espressif.com/sites/default/files/documentation/esp32-s3_technical_reference_manual_en.pdf
  pins = pinutils.generate_pins(0,48)
  # TODO: we could delete 22..25 as ESP32-S3 doesn't seem to have those

  pinutils.findpin(pins, "PD0", True)["functions"]["NEGATED"]=0; # button negated

  # I2C added for issue #2589 - all decided by user (not defined in specs)
  pinutils.findpin(pins, "PD8", True)["functions"]["I2C1_SDA"]=0;
  pinutils.findpin(pins, "PD9", True)["functions"]["I2C1_SCL"]=0;
  pinutils.findpin(pins, "PD18", True)["functions"]["I2C2_SDA"]=0;
  pinutils.findpin(pins, "PD19", True)["functions"]["I2C2_SCL"]=0;

  # SPI added for issue #2601
  #  - for SPI1 use pins that will bypass GPIO matrix (So Quicker) see esp-idf-4 /components/soc/esp32s3/include/soc/spi_pins.h
  pinutils.findpin(pins, "PD12", True)["functions"]["SPI1_SCK"]=0;
  pinutils.findpin(pins, "PD13", True)["functions"]["SPI1_MISO"]=0;
  pinutils.findpin(pins, "PD11", True)["functions"]["SPI1_MOSI"]=0;
  #  - SPI2 is decided by user
  pinutils.findpin(pins, "PD4", True)["functions"]["SPI2_SCK"]=0;
  pinutils.findpin(pins, "PD6", True)["functions"]["SPI2_MISO"]=0;
  pinutils.findpin(pins, "PD7", True)["functions"]["SPI2_MOSI"]=0;

  pinutils.findpin(pins, "PD43", True)["functions"]["USART1_TX"]=0;
  pinutils.findpin(pins, "PD44", True)["functions"]["USART1_RX"]=0;
  pinutils.findpin(pins, "PD17", True)["functions"]["USART2_TX"]=0;
  pinutils.findpin(pins, "PD18", True)["functions"]["USART2_RX"]=0;

  # everything is non-5v tolerant
  #for pin in pins:
  #  pin["functions"]["3.3"]=0;
  return pins
