#!/bin/false
# -*- coding: utf8 -*-
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
 'name' : "ST NUCLEO-F767ZI",
 'link' :  [ "https://www.st.com/en/evaluation-tools/nucleo-f767zi.html"],
 'default_console' : "EV_SERIAL3", # USART3 connected to ST-LINK Virtual Com Port by default without changing solder bridges
 'default_console_tx' : "D8", # USART3_TX on PD8
 'default_console_rx' : "D9", # USART3_RX on PD9
 'variables' :  31999, # (512-12)*1024/16-1
 'binary_name' : 'espruino_%v_nucleof767zi.bin',
 'build' : {
   'optimizeflags' : '-Os',
   'libraries' : [
     'GRAPHICS',
     'NET'
   ],
   'makefile' : [
     #'DEFINES+=-DSAVE_ON_FLASH_MATH', 
     #'DEFINES+=-DESPR_PACKED_SYMPTR', # Pack builtin symbols' offset into pointer to save 2 bytes/symbol
     'WRAPPERSOURCES+=targets/nucleo/jswrap_nucleo.c',
     'DEFINES+=-DUSE_USB_OTG_FS=1',
     'DEFINES+=-DPIN_NAMES_DIRECT=1', # Package skips out some pins, so we can't assume each port starts from 0
     'STLIB=STM32F767xx',
     'PRECOMPILED_OBJS+=$(ROOT)/targetlibs/stm32f7/lib/startup_stm32f767xx.o'
   ]
  }
};
chip = {
  'part' : "STM32F767ZIT6",
  'family' : "STM32F7",
  'package' : "LQFP144",
  'ram' : 512,
  'flash' : 2048,
  'flash_base' : 0x08000000,
  'speed' : 216,
  'usart' : 8,
  'spi' : 6,
  'i2c' : 4,
  'adc' : 3,
  'dac' : 2,
  'saved_code' : {
    # F767 flash sectors (single-bank, the ST factory default): 4x32KB + 1x128KB + 7x256KB = 2048KB
    # Reserve the last 256KB sector (sector 11, 0x081C_0000 to 0x0820_0000) for saved code
    'address' : 0x081C0000,
    'page_size' :  262144, # 256KB sector
    'pages' : 1,
    'flash_available' : 1792 # 2048 - 256 = 1792KB available for firmware
  },
};

devices = {
  'OSC' : { 'pin_1' : 'H0', # MCO from ST-LINK fixed at 8 Mhz, boards rev MB1136 C-02
            'pin_2' : 'H1' },
  'OSC_RTC' : { 'pin_1' : 'C14', # MB1136 C-02 corresponds to a board configured with on-board 32kHz oscillator
                'pin_2' : 'C15' },
  'LED1' : { 'pin' : 'B0' },
  'LED2' : { 'pin' : 'B7' },
  'LED3' : { 'pin' : 'B14' },
  'BTN1' : { 'pin' : 'C13', # TODO: Fix, this seems non functional
             'inverted' : True, # 1 when unpressed, 0 when pressed! (Espruino board is 1 when pressed)
             'pinstate': 'IN_PULLUP', # to specify INPUT, OUPUT PULL_UP PULL_DOWN..
           },
  'JTAG' : {
        'pin_MS' : 'A13',
        'pin_CK' : 'A14',
        'pin_DI' : 'A15'
          },
  'USB' : { 'pin_dm' : 'A11',
            'pin_dp' : 'A12',
            'pin_vbus' : 'A9',
            'pin_id' : 'A10', },
  # TODO: NUCLEO_A and NUCLEO_D Arduino header mappings for Nucleo-144
  # These are temporary values from Nucleo-64 to allow compilation
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
  background-image: url(img/NUCLEOF411RE.jpg);
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
  pins = pinutils.scan_pin_file([], 'stm32f767.csv', 6, 9, 10)
  pins = pinutils.scan_pin_af_file(pins, 'stm32f767_af.csv', 0, 1)
  return pinutils.only_from_package(pinutils.fill_gaps_in_pin_list(pins), chip["package"])
