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
 'name' : "STM32 F429 Discovery",
 'link' :  [ "http://www.st.com/web/catalog/tools/FM116/SC959/SS1532/LN1199/PF259090" ],
 'default_console' : "EV_SERIAL1",
 'variables' : 5450,
 'binary_name' : 'espruino_%v_stm32f429idiscovery.bin',
};
chip = {
  'part' : "STM32F429ZIT6",
  'family' : "STM32F4",
  'package' : "LQFP144",
  'ram' : 128,#256,
  'flash' : 512, #2048,
  'speed' : 168,
  'usart' : 6,
  'spi' : 3,
  'i2c' : 3,
  'adc' : 3,
  'dac' : 2,
};
# left-right, or top-bottom order
board = {
  'left' : [ ], # fixme
  'left2' : [ ],
  'right2' : [ ],
  'right' : [ ],
};
devices = {
  'OSC' : { 'pin_1' : 'H0',
            'pin_2' : 'H1' },
  'OSC_RTC' : { 'pin_1' : 'C14',
                'pin_2' : 'C15' },
  'LED1' : { 'pin' : 'G13' }, # green
  'LED2' : { 'pin' : 'G14' }, # red
  'BTN1' : { 'pin' : 'A0' },
  'USB' : { 'pin_dm' : 'B14',
            'pin_dp' : 'B15',
            'pin_vbus' : 'B13',
            'pin_id' : 'B12', 
            'pin_pso' : 'C4',  # Power supply enable
            'pin_oc' : 'C5',   # Overcurrent
           },
  'MEMS' :  {  'device' : 'L3GD20',
            'pin_cs' :  'C1',
            'pin_int1' :  'A1',
            'pin_int2' :  'A2',
            'pin_mosi' :  'F9',
            'pin_miso' :  'F8',
            'pin_sck' :  'F7' },
  'TOUCHSCREEN' : {
            'pin_irq' : 'A15',
            'pin_cs' : '',
            'pin_scl' : 'A8',
            'pin_sda' : 'C9',
          },
  'LCD' : {
            'width' : 320, 'height' : 240, 'bpp' : 16, 'controller' : 'fsmc', 'controller2' : 'ili9341',
            'pin_d0' : 'D6',
            'pin_d1' : 'G11',
            'pin_d2' : 'G12',
            'pin_d3' : 'A3',
            'pin_d4' : 'B8',
            'pin_d5' : 'B9',
            'pin_d6' : 'A6',
            'pin_d7' : 'G10',
            'pin_d8' : 'B10',
            'pin_d9' : 'B11',
            'pin_d10' : 'C7',
            'pin_d11' : 'D3',
            'pin_d12' : 'C10',
            'pin_d13' : 'B0',
            'pin_d14' : 'A11',
            'pin_d15' : 'A12',
            'pin_d16' : 'B1',
            'pin_d16' : 'G6',
            'pin_rd' : 'D12', # RDX
            'pin_wr' : 'D13',# WRQ
            'pin_cs' : 'C2', # CSX
            'pin_en' : 'F10',
            'pin_vsync' : 'A4',
            'pin_hsync' : 'C6',
            'pin_dotlck' : 'G7',
            'pin_dc' : 'F7', # DCX
            'pin_sda' : 'F9',
            'pin_im0' : 'D2', # pulled to 0
            'pin_im1' : 'D4', # pulled to 1
            'pin_im2' : 'D5', # pulled to 1
            'pin_im3' : 'D7', # pulled to 0
          },
  'SDRAM' : {
            'pin_sdcke1' : 'B5', 
            'pin_sdne1' : 'B6',
            'pin_sdnwe' : 'C0',
            'pin_d2' : 'D0',
            'pin_d3' : 'D1',
            'pin_d13' : 'D8',
            'pin_d14' : 'D9',
            'pin_d15' : 'D10',
            'pin_d0' : 'D14',
            'pin_d1' : 'D15',
            'pin_nbl0' : 'E0',
            'pin_nbl1' : 'E1',
            'pin_d4' : 'E7',
            'pin_d5' : 'E8',
            'pin_d6' : 'E9',
            'pin_d7' : 'E10',
            'pin_d8' : 'E11',
            'pin_d9' : 'E12',
            'pin_d10' : 'E13',
            'pin_d11' : 'E14',
            'pin_d12' : 'E15',
            'pin_a0' : 'F0',
            'pin_a1' : 'F1',
            'pin_a2' : 'F2',
            'pin_a3' : 'F3',
            'pin_a4' : 'F4',
            'pin_a5' : 'F5',
            'pin_sdnras' : 'F11',
            'pin_a6' : 'F12',
            'pin_a7' : 'F13',
            'pin_a8' : 'F14',
            'pin_a9' : 'F15',
            'pin_a10' : 'G0',
            'pin_a11' : 'G1',
            'pin_ba0' : 'G4',
            'pin_ba1' : 'G5',
            'pin_sdclk' : 'G8',
            'pin_sdncas' : 'G15',
  },
};


board_css = """
#board {
  width: 680px;
  height: 1020px;
  left: 200px;
  background-image: url(img/STM32F429IDISCOVERY.jpg);
}
#boardcontainer {
  height: 1020px;
}
#left {
  top: 375px;
  right: 590px;
}
#left2 {
  top: 375px;
  left: 105px;
}

#right  {
  top: 375px;
  left: 550px;
}
#right2  {
  top: 375px;
  right: 145px;
}
""";

def get_pins():
  pins = pinutils.scan_pin_file([], 'stm32f40x.csv', 6, 9, 10)
  return pinutils.only_from_package(pinutils.fill_gaps_in_pin_list(pins), chip["package"])
