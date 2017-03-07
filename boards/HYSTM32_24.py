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
 'name' : "STM32 2.4 inch LCD Board (VET6)",
 'variables' : 2800,
 'serial_bootloader' : True,
 'binary_name' : 'espruino_%v_hystm32_24_ve.bin',
 'build' : {
    'optimizeflags' : '-Os',
    'libraries' : [
      'GRAPHICS',
      'LCD_FSMC',
      'FILESYSTEM',
      'FILESYSTEM_SDIO',
      'NEOPIXEL'
    ],
    'makefile' : [
      'STLIB=STM32F10X_HD',
      'PRECOMPILED_OBJS+=$(ROOT)/targetlibs/stm32f1/lib/startup_stm32f10x_hd.o',
      'DEFINES+=-DFSMC_BITBANG # software implementation because FSMC HW causes strange crashes',
      'DEFINES+=-DUSE_RTC'
    ]
  }
};
chip = {
  'part' : "STM32F103VE", #T6
  'family' : "STM32F1",
  'package' : "LQFP100",
  'ram' : 64,
  'flash' : 512,
  'speed' : 72,
  'usart' : 3,
  'spi' : 2,
  'i2c' : 2,
  'adc' : 3,
  'dac' : 2,
};

devices = {
  'OSC' : { 'pin_1' : 'D0',
            'pin_2' : 'D1' },
  'OSC_RTC' : { 'pin_1' : 'C14',
                'pin_2' : 'C15' },
  'LED1' : { 'pin' : 'C6' },
  'LED2' : { 'pin' : 'C7' },
  'LED3' : { 'pin' : 'D13' },
  'LED4' : { 'pin' : 'D6' },
  'BTN1' : { 'pin' : 'E5', "inverted":1 },
  'BTN2' : { 'pin' : 'E4' },# TODO inverted?
  'BTN3' : { 'pin' : 'E3' },# TODO inverted?
  'BTN4' : { 'pin' : 'E4' },# TODO inverted?
  'POT1' : { 'pin' : 'C0' },
  'POT2' : { 'pin' : 'C1' },
  'USB' : { 'pin_disc' :  'B7',
            'pin_dm' : 'A11',
            'pin_dp' : 'A12' },
  'SD' :  { 'pin_cmd' :  'D2',
            'pin_d0' :  'C8',
            'pin_d1' :  'C9',
            'pin_d2' :  'C10',
            'pin_d3' :  'C11',
            'pin_clk' :  'C12' },
  'TOUCHSCREEN' : {
            'pin_irq' : 'B6',
            'pin_cs' : 'B7',
            'pin_sck' : 'A5',
            'pin_miso' : 'A6',
            'pin_mosi' : 'A7'
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
            'pin_rd' : 'D4',
            'pin_wr' : 'D5',
            'pin_cs' : 'D7',
            'pin_rs' : 'D11'
          },
  'JTAG' : {
        'pin_MS' : 'A13',
        'pin_CK' : 'A14',
        'pin_DI' : 'A15'
          }
};

# left-right, or top-bottom order
board = {
  'top' : [ 'GND','3V3','A2','A12','A9','C8','D13','B14','B12','C11','D2','B5','B1','E2','E4','E6','C3','A1','E0','3V3','','','3V3','B4','A15','A13','A14','JRTCK','B3','NRST','DBGGRQ','DBGACK' ],
  'top2' : [ 'GND','3V3','A3','A11','A10','C9','C7','B15','B13','C10','C12','D3','B2','B0','E3','E5','C2','A0','A4','3V3','','','3V3','GND','GND','GND','GND','GND','GND','GND','GND','GND' ],
  'bottom2' : [ 'A6','A5','A7','D6','C4','C5','E10','E9','E8','E7','D1','D0','D15','D14','B6','A6','C6','A7','B7','A5', ],
  'bottom' : [ 'GND','3V3','B11','E1','B10','D7','D10','D9','D8','E15','E14','E13','E12','E11','D4','D5','D11','C13','3V3','GND' ],
  'left' : [ 'B8','B9','D12','A8','C0','C1','GND' ],
};
board["top"].reverse()
board["top2"].reverse()
board["_css"] = """
#board {
  width: 1170px;
  height: 834px;
  left: 100px;
  top: 200px;
  background-image: url(img/HYSTM32_24.jpg);
}
#boardcontainer {
  height: 1250px;
}
#left {
  top: 130px;
  right: 1120px;
}
#top {
  top: 40px;
  left: 120px;
}
#top2 {
  top: 110px;
  left: 120px;
}
#bottom  {
  top: 760px;
  left: 330px;
}
#bottom2  {
  top: 690px;
  left: 330px;
}
""";


def get_pins():
  pins = pinutils.scan_pin_file([], 'stm32f103xe.csv', 6, 10, 11)
  return pinutils.only_from_package(pinutils.fill_gaps_in_pin_list(pins), chip["package"])
