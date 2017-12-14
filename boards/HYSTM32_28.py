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
 'name' : "STM32 2.8 inch LCD Board (RBT6)",
 'variables' : 700,
 'serial_bootloader' : True,
 'binary_name' : 'espruino_%v_hystm32_28_rb.bin',
 'build' : {
    'optimizeflags' : '-Os',
    'libraries' : [
      'GRAPHICS',
      'LCD_FSMC',
      'NEOPIXEL'
    ],
    'makefile' : [
      'SAVE_ON_FLASH=1',
      'STLIB=STM32F10X_MD',
      'PRECOMPILED_OBJS+=$(ROOT)/targetlibs/stm32f1/lib/startup_stm32f10x_md.o',
      'DEFINES+=-DFSMC_BITBANG # software implementation because FSMC HW causes strange crashes'
    ]
  }
};
chip = {
  'part' : "STM32F103RB", #T6
  'family' : "STM32F1",
  'package' : "LQFP64",
  'ram' : 20,
  'flash' : 128,
  'speed' : 72,
  'usart' : 3,
  'spi' : 2,
  'i2c' : 2,
  'adc' : 3,
  'dac' : 0,
  'saved_code' : {
    'address' : 0x08000000 + ((128-3)*1024),
    'page_size' : 1024, # size of pages
    'pages' : 3, # number of pages we're using
    'flash_available' : 128-3 # 3 used for code
  },
};

devices = {
  'OSC' : { 'pin_1' : 'D0',
            'pin_2' : 'D1' },
  'OSC_RTC' : { 'pin_1' :  'C14',
                'pin_2' : 'C15' },
  'LED1' : { 'pin' : 'A2' },
  'LED2' : { 'pin' : 'A3' },
  'BTN1' : { 'pin' : 'A0', "inverted":1 },
  'BTN2' : { 'pin' : 'A1' },# TODO inverted?
  'POT1' : { 'pin' : 'B0' },
  'USB' : { 'pin_disc' :  'D2',
            'pin_dm' : 'A11',
            'pin_dp' : 'A12' },
  'SD' :  { 'pin_cs' :  'B7',
            'pin_sck' : 'A5',
            'pin_miso' : 'A6',
            'pin_mosi' : 'A7'
          },
  'TOUCHSCREEN' : {
            'pin_irq' : 'C13',
            'pin_cs' : 'A4',
            'pin_sck' : 'A5',
            'pin_miso' : 'A6',
            'pin_mosi' : 'A7'
          },
  'LCD' : {
            'width' : 320, 'height' : 240, 'bpp' : 16, 'controller' : 'fsmc',
            'pin_d0' : 'C0',
            'pin_d1' : 'C1',
            'pin_d2' : 'C2',
            'pin_d3' : 'C3',
            'pin_d4' : 'C4',
            'pin_d5' : 'C5',
            'pin_d6' : 'C6',
            'pin_d7' : 'C7',
            'pin_d8' : 'B8',
            'pin_d9' : 'B9',
            'pin_d10' : 'B10',
            'pin_d11' : 'B11',
            'pin_d12' : 'B12',
            'pin_d13' : 'B13',
            'pin_d14' : 'B14',
            'pin_d15' : 'B15',
            'pin_rd' : 'C11',
            'pin_wr' : 'C10',
            'pin_cs' : 'C8',
            'pin_rs' : 'C9',
            'pin_backlight' : 'C12',
          },
  'JTAG' : {
        'pin_MS' : 'A13',
        'pin_CK' : 'A14',
        'pin_DI' : 'A15'
          }
};

# left-right, or top-bottom order
board = {
  'top' : [ '5V','A8','A10','A12','A14','B0','B2','B4','B6','C8','C10','C12','C14','3V3', ],
  'top2' : [ 'GND','A9','A11','A13','A15','B1','B3','B5','B7','C9','C11','C13','C15','GND' ],
  'bottom2' : [ 'GND','C1','C3','C5','C7','A1','A3','A5','A7','B9','B11','B13','B15','GND' ],
  'bottom' : [ '5V','C0','C2','C4','C6','A0','A2','A4','A6','B8','B10','B12','B14','3V3' ],
  'right' : [ '3V3','B4','A15','A13','A14','RTCK','B3','NRST','NC','5V' ],
};
board["top"].reverse()
board["top2"].reverse()
board["_css"] = """
#board {
  width: 980px;
  height: 770px;
  top: 220px;
  background-image: url(img/HYSTM32_28.jpg);
}
#boardcontainer {
  height: 1300px;
}
#top {
  top: 40px;
  left: 320px;
}
#top2 {
  top: 80px;
  left: 320px;
}
#bottom  {
  top: 720px;
  left: 320px;
}
#bottom2  {
  top: 680px;
  left: 320px;
}
#right  {
  top: 350px;
  left: 960px;
}

""";


def get_pins():
  pins = pinutils.scan_pin_file([], 'stm32f103xb.csv', 6, 10, 11)
  return pinutils.only_from_package(pinutils.fill_gaps_in_pin_list(pins), chip["package"])
