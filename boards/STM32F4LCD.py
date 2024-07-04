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
 'name' : "STM32 F4 LCD board",
 'link' :  [ "https://www.aliexpress.com/item/1005006735000932.html" ], # https://github.com/wegi1/STM32F407VET6-BLACK-ILI9341-BENCHMARK/
  #https://stm32-base.org/assets/pdf/boards/original-schematic-STM32F407VET6-STM32_F4VE_V2.0.pdf
 'variables' : 2450,
 'binary_name' : 'espruino_%v_stm32f4lcd.bin',
 'default_console' : "EV_SERIAL2",
 'default_console_tx' : "A2",
 'default_console_rx' : "A3",
 'default_console_baudrate' : "9600",
 'build' : {
   'optimizeflags' : '-Os',
   'libraries' : [
#     'USB_HID',
     'GRAPHICS',
     'LCD_FSMC',
     'FILESYSTEM',
     'FILESYSTEM_SDIO',
   ],
   'makefile' : [
     'DEFINES+=-DUSE_USB_OTG_FS=1',
     'DEFINES+=-DUSE_RTC',
     'DEFINES+=-DUSE_FONT_6X8 -DGRAPHICS_PALETTED_IMAGES -DGRAPHICS_ANTIALIAS -DESPR_PBF_FONTS -DESPR_GRAPHICS_INTERNAL',
     'DEFINES+=-DLCD_ORIENTATION_LANDSCAPE',
     'STLIB=STM32F407xx',
     '-DHSE_VALUE=9000000',
#     'DEFINES+=-DFSMC_BITBANG',
     'PRECOMPILED_OBJS+=$(ROOT)/targetlibs/stm32f4/lib/startup_stm32f40_41xxx.o',
     'TARGETSOURCES+=targetlibs/stm32f4/lib/stm32f4xx_fsmc.c',
     'INCLUDE += -I$(ROOT)/libs/pipboy',
     'WRAPPERSOURCES += libs/pipboy/avi.c',
     'WRAPPERSOURCES += libs/pipboy/jswrap_pipboy.c',     
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
  'saved_code' : {
    'address' : 0x08080000,
    'page_size' : 65536, # size of pages
    'pages' : 1, # number of pages we're using
    'flash_available' : 512 # Saved code is after binary
  },
};

devices = {
  'OSC' : { 'pin_1' : 'H0',
            'pin_2' : 'H1' },
  'OSC_RTC' : { 'pin_1' : 'C14',
                'pin_2' : 'C15' },
  'LED1' : { 'pin' : 'A6' },
  'LED2' : { 'pin' : 'A7' },
  'BTN1' : { 'pin' : 'A0', 'pinstate' : 'IN_PULLDOWN' },
  'BTN2' : { 'pin' : 'E4', 'pinstate' : 'IN_PULLDOWN' },  
  'BTN3' : { 'pin' : 'E3', 'pinstate' : 'IN_PULLDOWN' },  
  
  'SD' :  { 'pin_cmd' :  'D2',
            'pin_d0' :  'C8',
            'pin_d1' :  'C9',
            'pin_d2' :  'C10',
            'pin_d3' :  'C11',
            'pin_clk' : 'C12' },
  'TOUCHSCREEN' : {
            'pin_irq' : 'C5',
            'pin_cs' : 'B12',
            'pin_sck' : 'B13',
            'pin_miso' : 'B14',
            'pin_mosi' : 'B15'
          },
  'LCD' : {
            'width' : 320, 'height' : 240, 'bpp' : 16, 'controller' : 'fsmc',
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
            'pin_rs' : 'D13',
            'pin_rd' : 'D4',
            'pin_wr' : 'D5',
            'pin_cs' : 'D7'
            # B1 = backlight
          },  
  # flash
  'USB' : { 'pin_vsense' :  'A9',
            'pin_dm' : 'A11',
            'pin_dp' : 'A12' },
  
  'JTAG' : {
        'pin_MS' : 'A13',
        'pin_CK' : 'A14',
        'pin_DI' : 'A15'
          },
};

# left-right, or top-bottom order
board = {
};
board["_css"] = """
""";

def get_pins():
  pins = pinutils.scan_pin_file([], 'stm32f40x.csv', 6, 9, 10)
  pins = pinutils.scan_pin_af_file(pins, 'stm32f401_af.csv', 0, 1) # FIXME: for wrong device!
#  print(pinutils.findpin(pins, "PA9", True));
  pins = pinutils.only_from_package(pinutils.fill_gaps_in_pin_list(pins), chip["package"])
  pinutils.findpin(pins, "PA6", True)["functions"]["NEGATED"]=0; # LED1
  pinutils.findpin(pins, "PA7", True)["functions"]["NEGATED"]=0; # LED2
  return pins
