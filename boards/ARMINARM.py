#!/bin/false
# This file is part of Espruino, a JavaScript interpreter for Microcontrollers
#
# Copyright (C) 2013 Gordon Williams <gw@pur3.co.uk>
# Adapted for ARMinARM board by Rik Teerling <on@onandoffables.com>
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
 'name' : "ARMinARM addon board for Raspberry Pi B+",
 'link' : [ "https://www.onandoffables.com/" ],
 'variables' : 3250,
 'binary_name' : 'espruino_%v_ARMinARM.bin',
 'build' : {
   'optimizeflags' : '-Os',
   'libraries' : [
     'NET',
     'GRAPHICS',
     'NEOPIXEL'
     'FILESYSTEM'
   ],
   'makefile' : [
     'STLIB=STM32F10X_HD',
     'PRECOMPILED_OBJS+=$(ROOT)/targetlibs/stm32f1/lib/startup_stm32f10x_hd.o'
   ]
  }
};
chip = {
  'part' : "STM32F103RET6",
  'family' : "STM32F1",
  'package' : "LQFP64",
  'ram' : 64,
  'flash' : 512,
  'speed' : 72,
  'usart' : 5,
  'spi' : 3,
  'i2c' : 2,
  'adc' : 3,
  'dac' : 2,
};
devices = {
  'OSC' : { 'pin_in' :  'D0',
            'pin_out' : 'D1' },
  'OSC_RTC' : { 'pin_in' :  'C14',
                'pin_out' : 'C15' },
  'LED1' : { 'pin' : 'B0' },
  'BTN1' : { 'pin' : 'A0' },
  'USB' : { 'pin_disc' :  'C13',
            'pin_dm' : 'A11',
            'pin_dp' : 'A12' },
  'SD' :  { 'pin_cs' :  'D2',
            'pin_di' :  'B15',
            'pin_do' :  'B14',
            'pin_clk' :  'B13' },
#  'BLUETOOTH' : { 'pin_tx' : 'A9',
#                  'pin_rx' : 'A10' },
};

def get_pins():
  pins = pinutils.scan_pin_file([], 'stm32f103xe.csv', 6, 10, 11)
  return pinutils.only_from_package(pinutils.fill_gaps_in_pin_list(pins), chip["package"])
