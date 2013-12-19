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
 'name' : "Espruino Board rev 1.0",
 'link' : [ "http://www.espruino.com/kick" ],
 'variables' : 2000,
 'bootloader' : 0,
 'serial_bootloader' : False,
 'binary_name' : 'espruino_%v_espruino_1r0.bin',
};
chip = {
  'part' : "STM32F103RGT6",
  'family' : "STM32F1",
  'package' : "LQFP64",
  'ram' : 96,
  'flash' : 512, # 1024, but in 2 banks
  'speed' : 72,
  'usart' : 5,
  'spi' : 3,
  'i2c' : 2,
  'adc' : 3,
  'dac' : 2,
};
# left-right, or top-bottom order
board = {
  'top' : [ 'D2', 'C11', 'C10', 'A14', 'A14', 'A13', 'A10', 'A9', 'A8', 'C9', 'C8', 'C7', 'C6', 'B15', 'B14', 'B13', 'B12', '3.3', 'VBAT', 'GND' ],
  'bottom' : [ 'B3', 'B4', 'B5', 'B6', 'B7', 'B8', 'B9', 'C13', 'C14', 'C15', 'C0', 'C1', 'C2', 'C3', 'A0', 'A1', 'A2', '3.3', 'VBAT', 'GND' ],
  'mid' : ['B2', 'B1', 'B0', 'C5', 'C4', 'A7', 'A6', 'A5', 'A4', 'A3' ]
};

devices = {
  'OSC' : { 'pin_in' :  'D0',
            'pin_out' : 'D1' },
  'LED1' : { 'pin' : 'B7' },
  'LED2' : { 'pin' : 'B8' },
  'LED3' : { 'pin' : 'B9' },
  'BTN1' : { 'pin' : 'A3' },
  'USB' : { 'pin_disc' :  'C12',
            'pin_dm' : 'A11',
            'pin_dp' : 'A12' },
  'SD' :  { 'pin_cs' :  'D2',
            'pin_di' :  'B15',
            'pin_do' :  'B14',
            'pin_clk' :  'B13' },
  'BLUETOOTH' : { 'pin_tx' : 'A9',
                  'pin_rx' : 'A10' },
};

board_css = """
#board {
  width: 585px;
  height: 431px;
  top: 280px;
  background-image: url(img/ESPRUINOBOARD_R1_0.jpg);
}
#boardcontainer {
  height: 1000px;
}
#top {
  top: -20px;
  left: 43px;
}

#bottom  {
  top: 431px;
  left: 43px;
}
#mid {
  top: 48px;
  left: 280px;
}
.midpin {
  padding: 3px;
}
""";

def get_pins():
  pins = pinutils.scan_pin_file([], 'stm32f103xe.csv', 6, 10, 11)
  return pinutils.only_from_package(pinutils.fill_gaps_in_pin_list(pins), chip["package"])
