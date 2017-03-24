#!/bin/false
# -*- coding: utf8 -*-
# This file is part of Espruino, a JavaScript interpreter for Microcontrollers
#
# Copyright (C) 2013 Gordon Williams <gw@pur3.co.uk>
# Copyright (C) 2014 ST for NucleoL476RG specific lines of this file
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
 'name' : "ST NUCLEO-L476RG",
 'link' :  [ "http://www.st.com/content/st_com/en/products/evaluation-tools/product-evaluation-tools/mcu-eval-tools/stm32-mcu-eval-tools/stm32-mcu-nucleo/nucleo-l476rg.html"],
 'default_console' : "EV_SERIAL2", # USART2 by default, the Nucleo's USB is actually running on this too
 'default_console_tx' : "A2", # USART2_TX on PA2,
 'default_console_rx' : "A3", # USART2_RX on PA3
 'variables' :  5119, # (96-16)*1024/16-1
 'binary_name' : 'espruino_%v_nucleol476rg.bin',
 'build' : {
   'optimizeflags' : '-O3',
   'libraries' : [
     'NET',
     'GRAPHICS',
     'NEOPIXEL'
   ],
   'makefile' : [
     'NUCLEO=1',
     'DEFINES+=-DUSE_USB_OTG_FS=1',
     'STLIB=STM32L476xx',
     'PRECOMPILED_OBJS+=$(ROOT)/targetlibs/stm32l4/lib/CMSIS/Device/ST/STM32L4xx/Source/Templates/gcc/startup_stm32l476xx.o'
   ]
  }
};

chip = {
  'part' : "STM32L476RG",
  'family' : "STM32L4",
  'package' : "LQFP64",
  'ram' : 96, ###TO CHECK # 0x0001 8000 long, from 0x2000 0000 to 0x2001 7FFF
  'flash' : 1024, ###TO CHECK # 0x0008 0000 long, from 0x0800 0000 to 0x0807 FFFF
  'speed' : 80,
  'usart' : 3,
  'spi' : 4,
  'i2c' : 3,
  'adc' : 1,
  'dac' : 2,
  'saved_code' : {
    # code size 300000 = 0x493E0 starts at 0x0800 0000 ends at 0x0804 93E0
    # so we have some left room for Espruino firmware and no risk to clear it while saving
    'address' : 0x08050000, # flash_saved_code_start 0x080493E0 to 0x8100000
    # we have enough flash space in this single flash page to save all of the ram
    'page_size' :  2048, # size of pages : on STM32F411, last 2 pages are 128 Kbytes
    # we use the last flash page only, furthermore it persists after a firmware flash of the board
    'pages' : 352, # count of pages we're using to save RAM to Flash,
    'flash_available' : 704 # binary will have a hole in it, so we just want to test against full size
  },
};

devices = {
  'OSC' : { 'pin_1' : 'H0', # MCO from ST-LINK fixed at 8 Mhz, boards rev MB1136 C-02
            'pin_2' : 'H1' },
  'OSC_RTC' : { 'pin_1' : 'C14', # MB1136 C-02 corresponds to a board configured with on-board 32kHz oscillator
                'pin_2' : 'C15' },
  'LED1' : { 'pin' : 'A5' },
  'BTN1' : { 'pin' : 'C13',
             'inverted' : True, # 1 when unpressed, 0 when pressed! (Espruino board is 1 when pressed)
             'pinstate': 'IN_PULLUP', # to specify INPUT, OUPUT PULL_UP PULL_DOWN..
           },
  'JTAG' : {
        'pin_MS' : 'A13',
        'pin_CK' : 'A14',
        'pin_DI' : 'A15'
          },
#  'USB' : { 'pin_otg_pwr' : 'C0',
#            'pin_dm' : 'A11',
#            'pin_dp' : 'A12',
#            'pin_vbus' : 'A9',
#            'pin_id' : 'A10', },
  'NUCLEO_A' : [ 'A0','A1','A4','B0','C1','C0' ],
  'NUCLEO_D' : [ 'A3','A2','A10','B3','B5','B4','B10','A8','A9','C7','B6','A7','A6','A5','B9','B8' ],
};

# left-right, or top-bottom order
board = {
  'left' :   [ 'C10', 'C12', 'VDD', 'BOOT0', 'NC', 'NC', 'A13', 'A14', 'A15', 'GND', 'B7', 'C13', 'C14', 'C15', 'H0', 'H1', 'VBAT', 'C2', 'C3'],
  'left2' :  [ 'C11', 'D2', 'E5V', 'GND', 'NC', 'IOREF', 'RESET', '3V3', '5V', 'GND', 'GND', 'VIN', 'NC', 'A0', 'A1', 'A4', 'B0', 'C1', 'C0'],
  'right2' : [ 'C9', 'B8', 'B9', 'AVDD', 'GND', 'A5', 'A6', 'A7', 'B6','C7','A9','A8','B10','B4','B5','B3','A10','A2','A3'],
  'right' :  [ 'C8', 'C6', 'C5', 'U5V', 'NC', 'A12', 'A11', 'B12', 'NC', 'GND', 'B2', 'B1', 'B15', 'B14', 'B13', 'AGND', 'C4', 'NC', 'NC'],
};
board["_css"] = """
#board {
  width: 713px;
  height: 800px;
  left: 200px;
  background-image: url(img/NUCLEOL476RG.jpg);
}
#boardcontainer {
  height: 1020px;
}
#left {
  top: 310px;
  right: 640px;
}
#left2 {
  top: 310px;
  left: 85px;
}

#right  {
  top: 310px;
  left: 615px;
}
#right2  {
  top: 310px;
  right: 105px;
}
""";

def get_pins():
  pins = pinutils.scan_pin_file([], 'stm32l476.csv', 6, 10, 11)
  pins = pinutils.scan_pin_af_file(pins, 'stm32l476_af.csv', 0, 1)
  return pinutils.only_from_package(pinutils.fill_gaps_in_pin_list(pins), chip["package"])
