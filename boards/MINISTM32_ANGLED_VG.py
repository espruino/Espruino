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
 'name' : "Mini STM32 angled 7 inch LCD Board (VGT6)",
 #'variables' : 2800,
 'variables' : 5376, # (96-12)*1024/16-1
 'serial_bootloader' : True,
 'binary_name' : 'espruino_%v_mini_stm32_vg.bin',
};
chip = {
  'part' : "STM32F103VG", #T6
  'family' : "STM32F1",
  'package' : "LQFP100",
  'ram' : 96,
  'flash' : 1024,
  'speed' : 72,
  'usart' : 3,
  'spi' : 2,
  'i2c' : 2,
  'adc' : 3,
  'dac' : 2,
};
# left-right, or top-bottom order
board = {
  'bottom' : [ 'A3', 'A1', 'C0', 'C2', 'C4', 'B0', '', '', 'E5', 'E2', 'E0', 'B8', 'B10', 'B12', 'B14', 'D12', 'C7', 'D6', 'GND', '3V' ],
  'bottom2' : [ 'A2', 'A0', 'C1', 'C3', 'C5', 'B1', '', 'E6', 'E4', 'E3', 'B9', '', '', 'B11', 'B13', 'B15', 'C6', 'A8', 'GND', '3V' ],
  'lcd' : [ '3V', 'GND', 'RESET', 'RD', 'WR', 'CS', 'RS', 'E10', 'E9', 'D1', 'E8', 'E7', '3V', 'D0', 'D15', 'D14', 'GND', 'GND', '5V', '5V' ],
  'lcd2' : [ 'A5', 'B7', 'A7', 'A6', 'B6', '', '', '', '', 'D8', 'E15', 'G', 'E14', 'E13', 'E12', 'D9', 'E11', 'G', 'D13', 'D10' ],
};
#board["top"].reverse()
#board["top2"].reverse()
devices = {
  'OSC' : { 'pin_1' : 'D0',
            'pin_2' : 'D1' },
  'OSC_RTC' : { 'pin_1' : 'C14',
                'pin_2' : 'C15' },
  'LED1' : { 'pin' : 'B5' },
  'BTN1' : { 'pin' : 'B15',
             'inverted' : True, # 1 when unpressed, 0 when pressed! (Espruino board is 1 when pressed)
             'pinstate': 'IN_PULLUP', # to specify INPUT, OUPUT PULL_UP PULL_DOWN..
           },
  'USB' : { 'pin_disc' :  'C13',
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


board_css = """
#board {
  width: 1170px;
  height: 834px;
  left: 100px;
  top: 200px;
  background-image: url(img/MINISTM32_ANGLED.jpg);
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


