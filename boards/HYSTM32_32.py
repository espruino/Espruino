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
 'name' : "HY-MiniSTM32V 3.2 inch LCD Board (VCT6)",
 'link' : [ "http://www.hotmcu.com/hyministm32v-dev-board-32-tft-lcd-module-p-5.html" ],
 'variables' : 2000,
 'serial_bootloader' : True,
 'binary_name' : 'espruino_%v_hystm32_32_vc.bin',
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
      'DEFINES+=-DFSMC_BITBANG # software implementation because FSMC HW causes strange crashes'
    ]
  }
};

chip = {
  'part' : "STM32F103VC",
  'family' : "STM32F1",
  'package' : "LQFP100",
  'ram' : 48,
  'flash' : 256,
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
  'OSC_RTC' : { 'pin_1' :  'C14',
                'pin_2' : 'C15' },
  'LED1' : { 'pin' : 'B0' },
  'LED2' : { 'pin' : 'B1' },
  'BTN1' : { 'pin' : 'C13', "inverted":1 },
  'BTN2' : { 'pin' : 'B2' }, # TODO inverted?
  'USB' : { 'pin_disc' :  'B7',
            'pin_dm' : 'A11',
            'pin_dp' : 'A12' },
  'SD' :  { 'pin_cmd' :  'D2',
            'pin_cd' :  'D3',
            'pin_d0' :  'C8',
            'pin_d1' :  'C9',
            'pin_d2' :  'C10',
            'pin_d3' :  'C11',
            'pin_clk' :  'C12' },
  'TOUCHSCREEN' : {
            'pin_irq' : 'B6',
            'pin_cs' : 'A4',
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
  'bottom' : [ 'GND', 'E0','E2','E4','E6','C13','C15','C1','C3','GND','VDDA','A1','A3','A5','A7','C5','B1','E7','E9','E11','E13','E15','B11', 'GND' ],
  'bottom2' : [ '5V','E1','E3','E5','VBAT','C14','C0','C2','GND','VREF+','A0','A2','A4','A6','C4','B0','B2','E8','E10','E12','E14','B10','GND','3V3' ],
  'top' : [ 'GND', 'B12','B14','D8','D10','D12','D14','C6','C8','A8','A10','A12','A14','GND','C10','C12','D1','D3','D5','D7','B4','B6','B8','GND' ],
  'top2' : [ '5V', 'B13','B15','D9','D11','D13','D15','C7','C9','A9','A11','A13','A15','3V3','C11','D0','D2','D4','D6','B3','B5','B7','B9','3V3' ],
};
board["top"].reverse()
board["top2"].reverse()
board["_css"] = """
#board {
  width: 1025px;
  height: 837px;
  top: 240px;
  background-image: url(img/HYSTM32_32.jpg);
}
#boardcontainer {
  height: 1300px;
}
#top {
  top: 40px;
  left: 210px;
}
#top2 {
  top: 110px;
  left: 210px;
}
#bottom  {
  top: 770px;
  left: 210px;
}
#bottom2  {
  top: 700px;
  left: 210px;
}
#otherpins {
  display: none;
}

""";


def get_pins():
  pins = pinutils.scan_pin_file([], 'stm32f103xe.csv', 6, 10, 11)
  return pinutils.only_from_package(pinutils.fill_gaps_in_pin_list(pins), chip["package"])
