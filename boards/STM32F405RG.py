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
# Can be run in QEMU with:
#
#   qemu-system-arm -M olimex-stm32-h405 -nographic -kernel bin/espruino_2v29.29_STM32F405RG.bin
#
#   and for debug:
#       -gdb tcp::4242 -S 
#
# Press Ctrl-a then x to exit QEMU.

import pinutils;
info = {
 'name' : "STM32F405RG",
 'default_console' : "EV_SERIAL1",
 'variables' : 5450,
 'binary_name' : 'espruino_%v_STM32F405RG.bin',
 'build' : {
   'optimizeflags' : '-O3',
   'libraries' : [
     'JIT'
   ],
   'makefile' : [
     'DEFINES+=-DQEMU=1', # need to tell this not to use ADC as QEMU doesn't support it
     'DEFINES+=-DPIN_NAMES_DIRECT=1', # Package skips out some pins, so we can't assume each port starts from 0
     'STLIB=STM32F405xx',
     'PRECOMPILED_OBJS+=$(ROOT)/targetlibs/stm32f4/lib/startup_stm32f40_41xxx.o'
   ]
  }
};

chip = {
  'part' : "STM32F405RGT6",
  'family' : "STM32F4",
  'package' : "LQFP64",
  'ram' : 192,
  'flash' : 1024,
  'speed' : 168,
  'usart' : 6,
  'spi' : 3,
  'i2c' : 3,
  'adc' : 3,
  'dac' : 2,
};

devices = {
  'OSC' : { 'pin_1' : 'H0',
            'pin_2' : 'H1' },
  'OSC_RTC' : { 'pin_1' : 'C14',
                'pin_2' : 'C15' },
};

# left-right, or top-bottom order
board = {
};
board["_css"] = """
""";

def get_pins():
  pins = pinutils.scan_pin_file([], 'stm32f40x.csv', 6, 9, 10)
  return pinutils.only_from_package(pinutils.fill_gaps_in_pin_list(pins), chip["package"])
