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
 'name' : "Fallout TV series Pip-Boy",
 'boardname' : "PIPBOY",
 'link' :  [ "https://www.thewandcompany.com/fallout-pip-boy" ],
 'variables' : 5000, # 5000 -> 65k for vars
 'binary_name' : 'espruino_%v_pipboy.bin',
 'default_console' : "EV_SERIAL3",
 'default_console_tx' : "B10",
 'default_console_rx' : "B11",
 'default_console_baudrate' : "115200",
 'io_buffer_size' : 512,
 'xoff_thresh' : 3,
 'xon_thresh' : 2,
 'build' : {
   'optimizeflags' : '-Os',
   'libraries' : [
#     'USB_HID',
     'GRAPHICS',
     'LCD_FSMC',
     'FILESYSTEM',
     'FILESYSTEM_SDIO',
     'JIT'
   ],
   'makefile' : [
     'DEFINES+=-DUSE_USB_OTG_FS=1',
     'DEFINES+=-DUSE_RTC',
     'DEFINES+=-DUSE_FONT_6X8 -DGRAPHICS_PALETTED_IMAGES -DGRAPHICS_ANTIALIAS -DESPR_PBF_FONTS -DESPR_GRAPHICS_INTERNAL -DESPR_GRAPHICS_NO_SPLASH',
     'DEFINES+=-DESPR_SDIO_FAST_UNALIGNED',  # see sdio_diskio.c - this is a nasty hack to increase unaligned read speed
     'DEFINES+=-DLCD_ORIENTATION_LANDSCAPE',
#     'DEFINES+=-DLCD_CRT_EFFECT',
     'DEFINES+=-DUSE_AUDIO_CODEC',
     'DEFINES+=-DESPR_DELAY_MULTIPLIER=28672', # don't work out what to use for jshDelayMicroseconds at boot, just hard-code it
     'STLIB=STM32F407xx',
     '-DHSE_VALUE=9000000',
#     'DEFINES+=-DFSMC_BITBANG',
     'PRECOMPILED_OBJS+=$(ROOT)/targetlibs/stm32f4/lib/startup_stm32f40_41xxx.o',
     'TARGETSOURCES+=targetlibs/stm32f4/lib/stm32f4xx_fsmc.c',
     'INCLUDE += -I$(ROOT)/libs/pipboy',
     'WRAPPERSOURCES += libs/pipboy/avi.c libs/pipboy/stm32_i2s.c',
     'WRAPPERSOURCES += libs/pipboy/jswrap_pipboy.c',
     'WRAPPERSOURCES += libs/graphics/jswrap_font_monofonto_120.c',
     'WRAPPERSOURCES += libs/graphics/jswrap_font_monofonto_96.c',
     'WRAPPERSOURCES += libs/graphics/jswrap_font_monofonto_28.c',
     'WRAPPERSOURCES += libs/graphics/jswrap_font_monofonto_23.c',
     'WRAPPERSOURCES += libs/graphics/jswrap_font_monofonto_18.c',
     'WRAPPERSOURCES += libs/graphics/jswrap_font_monofonto_16.c',
#     'USE_DFU=1'
   ]
  }
};

chip = {
  'part' : "STM32F407VE",
  'family' : "STM32F4",
  'package' : "LQFP100",
  'ram' : 192,
  'flash' : 512,
#  'flash_base' : 0x08000000,
  'speed' : 168,
  'usart' : 6,
  'spi' : 3,
  'i2c' : 3,
  'adc' : 3,
  'dac' : 2,
  'saved_code' : { # last page
    'address' : 0x08060000,
    'page_size' : 131072, # size of pages
    'pages' : 1, # number of pages we're using
    'flash_available' : 384 # Saved code is after binary
  },
};

devices = {
  'OSC' : { 'pin_1' : 'H0',
            'pin_2' : 'H1' },
  'OSC_RTC' : { 'pin_1' : 'C14',
                'pin_2' : 'C15' },
  'LED1' : { 'pin' : 'E4', 'pinstate' : 'OUT_OPENDRAIN' }, # Red element of RGB LED - needs to be open-drain otherwise we can't turn it off completely
  'LED2' : { 'pin' : 'E5' }, # Green element of RGB LED
  'LED3' : { 'pin' : 'E6' }, # Blue element of RGB LED
  'LED4' : { 'pin' : 'E3' }, # Radio tuning indicator LED
  'BTN1' : { 'pin' : 'A1' }, # "Play" button
  'BTN2' : { 'pin' : 'E1' }, # "Up" button
  'BTN3' : { 'pin' : 'E2' }, # "Down" button
  'BTN4' : { 'pin' : 'A2' }, # "Flashlight" button
  'BTN5' : { 'pin' : 'A10' }, # Thumbwheel encoder A - PA9 for v0.3, PA10 for v0.5
  'BTN6' : { 'pin' : 'A8' }, # Thumbwheel encoder B
  'BTN7' : { 'pin' : 'A3' }, # Clock "select" button
  'BTN8' : { 'pin' : 'B1' }, # Clock encoder A
  'BTN9' : { 'pin' : 'B0' }, # Clock encoder B
  'BTN10' : { 'pin' : 'A0' }, # "Power" button
 
  'BAT' : {
            'pin_sense_en' : 'C4', 
            'pin_voltage' : 'A6',
            'pin_charging' : 'C5',
          },
  'SD' :  { 'pin_cmd' :  'D2',
            'pin_d0' :  'C8',
            'pin_d1' :  'C9',
            'pin_d2' :  'C10',
            'pin_d3' :  'C11',
            'pin_clk' : 'C12',
            'pin_cd' :  'A15', # SD card detect switch - doesn't work on hardware v0.3
            'pin_pwr' : 'D3'}, # SD card power supply enable (also switches power to the ES8388 audio codec)
  'LCD' : {
            'width' : 480, 'height' : 320, 'bpp' : 16, 'controller' : 'fsmc',
            'pin_d0' : 'D14',
            'pin_d1' : 'D15',
            'pin_d2' : 'D0',
            'pin_d3' : 'D1',
            'pin_d4' : 'E7',
            'pin_d5' : 'E8',
            'pin_d6' : 'E9',
            'pin_d7' : 'E10',
            'pin_d8' : 'E11',
            'pin_d9' : 'E12',
            'pin_d10' : 'E13',
            'pin_d11' : 'E14',
            'pin_d12' : 'E15',
            'pin_d13' : 'D8',
            'pin_d14' : 'D9',
            'pin_d15' : 'D10',
            'pin_rs' : 'D11',
            'pin_rd' : 'D4',
            'pin_wr' : 'D5',
            'pin_cs' : 'D7', # CS / NE1
            'pin_bl' : 'B15' # backlight
          },  
  'SPIFLASH' : {
            'pin_cs' : 'B14',
            'pin_sck' : 'B3',
            'pin_mosi' : 'B5', 
            'pin_miso' : 'B4',
            'size' : 4096*64, 
            'memmap_base' : 0x60000000 # map into the address space (in software) - FIXME: what should this address be?
          },
  'USB' : { 'pin_vsense' :  'A5', # PA5 for v0.3, PA9 for v0.5 (which should be connected but doesn't seem to work)
            'pin_dm' : 'A11',
            'pin_dp' : 'A12' },
  
  'JTAG' : {
        'pin_MS' : 'A13',
        'pin_CK' : 'A14',
          },
};

# left-right, or top-bottom order
board = {
};
board["_css"] = """
""";

def get_pins():
  pins = pinutils.scan_pin_file([], 'stm32f40x.csv', 6, 9, 10)
  pins = pinutils.scan_pin_af_file(pins, 'stm32f405_af.csv', 0, 1) # F405 is close enough to the F407
  pins = pinutils.only_from_package(pinutils.fill_gaps_in_pin_list(pins), chip["package"])
  pinutils.findpin(pins, "PE4", True)["functions"]["NEGATED"]=0; # LED1
  pinutils.findpin(pins, "PE5", True)["functions"]["NEGATED"]=0; # LED2
  pinutils.findpin(pins, "PE6", True)["functions"]["NEGATED"]=0; # LED3
  pinutils.findpin(pins, "PE3", True)["functions"]["NEGATED"]=0; # LED4
  pinutils.findpin(pins, "PA1", True)["functions"]["NEGATED"]=0; # BTN1
  pinutils.findpin(pins, "PE1", True)["functions"]["NEGATED"]=0; # BTN2
  pinutils.findpin(pins, "PE2", True)["functions"]["NEGATED"]=0; # BTN3
  pinutils.findpin(pins, "PA2", True)["functions"]["NEGATED"]=0; # BTN4
  pinutils.findpin(pins, "PA10", True)["functions"]["NEGATED"]=0; # BTN5 - PA9 for v0.3, PA10 for v0.5
  pinutils.findpin(pins, "PA8", True)["functions"]["NEGATED"]=0; # BTN6
  pinutils.findpin(pins, "PA3", True)["functions"]["NEGATED"]=0; # BTN7
  pinutils.findpin(pins, "PB1", True)["functions"]["NEGATED"]=0; # BTN8
  pinutils.findpin(pins, "PB0", True)["functions"]["NEGATED"]=0; # BTN9
  pinutils.findpin(pins, "PA0", True)["functions"]["NEGATED"]=0; # BTN10
  return pins
