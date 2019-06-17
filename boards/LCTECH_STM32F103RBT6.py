#!/bin/false
# This file is part of Espruino, a JavaScript interpreter for Microcontrollers
#
# Copyright (C) 2013 Nic Marcondes <nic@upnic.net>
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# ----------------------------------------------------------------------------------------
# This file contains information for a specific board - the available pins, and where LEDs,
# Buttons, and other in-built peripherals are. It is used to build documentation as well
# as various source and header files for Espruino.
# It's based on Gordon Willians original script
# ----------------------------------------------------------------------------------------

import pinutils;
import json;
info = {
 'name' : "LC Technology STM32F103RBT6 / ebay board",
 'link' :  [ "http://www.lctech-inc.com/Hardware/Detail.aspx?id=4ae8ef7e-9bfe-48a1-9540-fa66ad4645b4"],
 'variables' : 715,
 'binary_name' : 'espruino_%v_lctech_stm32f103rbt6.bin',
 'build' : {
   'optimizeflags' : '-Os',
   'libraries' : [
   ],
   'makefile' : [
     'SAVE_ON_FLASH=1',
     'STLIB=STM32F10X_MD',
     'PRECOMPILED_OBJS+=$(ROOT)/targetlibs/stm32f1/lib/startup_stm32f10x_md.o'
   ]
  }
};
chip = {
  'part' : "STM32F103RBT6", #T6
  'family' : "STM32F1",
  'subfamily' : "MD",
  'package' : "LQFP64",
  'ram' : 20,
  'flash' : 128,
  'speed' : 72,
  'usart' : 3,
  'spi' : 2,
  'i2c' : 2,
  'adc' : 3,
  'dac' : 0,
};

devices = {
  'OSC' : { 'pin_1' : 'D0',
            'pin_2' : 'D1' },
  'OSC_RTC' : { 'pin_1' :  'C14',
                'pin_2' : 'C15' },
  'LED1' : { 'pin' : 'B8'}, #TODO: Make inverted
  'LED2' : { 'pin' : 'B9'}, #TODO: Make inverted
  'BTN1' : { 'pin' : 'C0' }, #TODO: Make pinMode(BTN1, "input_pullup");
  'BTN2' : { 'pin' : 'C1' }, #TODO: Make pinMode(BTN2, "input_pullup");
  'USB' : { 'pin_disc' :  'D2',
            'pin_dm' : 'A11',
            'pin_dp' : 'A12' },
};

board = {
  'top' : [ 'GND', 'D1', 'C14', 'B9', 'B7', 'B5', 'B3', 'C12', 'C10', 'A14', 'A12', 'A10', 'A8', 'C8', 'GND'],
  'top2' : [ '3V3', 'D0', 'C15', 'C13', 'B8', 'B6', 'B4', 'D2', 'C11', 'A15', 'A13', 'A11', 'A9', 'C9', '5V'],
  'bottom' : [ 'GND', 'C0', 'C2', 'A0', 'A2', 'A4', 'A6', 'C4', 'B0', 'B2', 'B11', 'B13', 'B15', 'C7', 'GND'],
  'bottom2' : [ '3V3', 'C1', 'C3', 'A1', 'A3', 'A5', 'A7', 'C5', 'B1', 'B10', 'B12', 'B14', 'C6', 'NC', '5V']
};
#TODO: Calculate this numbers..
board["_css"] = """
#board {
  width: 809px;
  height: 584px;
  top: 300px;
  left: 200px;
  background-image: url(img/LCTECH_STM32F103RBT6.jpg);
}
#boardcontainer {
  height: 850px;
}

#top {
  top: -1px;
  left: -1px;
}
#top2 {
  top: -10px;
  left: -10px;
}
#bottom  {
  top: 1px;
  left: 1px;
}
#bottom2  {
  top: 10px;
  left: 10px;
}

""";


def get_pins():
  pins = pinutils.scan_pin_file([], 'stm32f103xb.csv', 6, 10, 11)
  return pinutils.only_from_package(pinutils.fill_gaps_in_pin_list(pins), chip["package"])
