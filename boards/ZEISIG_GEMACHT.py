
# This file is part of Espruino, a JavaScript interpreter for Microcontrollers
#
# Copyright (C) 2013 Gordon Williams <gw@pur3.co.uk>
# Adapted for the Zeisig Gemacht by Raman Gopalan <ramangopalan@gmail.com>
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
 'name' : "Espruino - Zeisig Gemacht - for Raman's 4 kilowatt toy",
 'link' : [ "" ],
 'variables' : 2000,
 'serial_bootloader' : True,
 'binary_name' : 'espruino_%v_zeisig_gemacht.bin',
 'build' : {
    'optimizeflags' : '-Os',
    'libraries' : [
      'FILESYSTEM',
    ],
    'makefile' : [
      'STLIB=STM32F10X_HD',
      'PRECOMPILED_OBJS+=$(ROOT)/targetlibs/stm32f1/lib/startup_stm32f10x_hd.o',
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
  'SD' :  { 'pin_cs' : 'A4',
            'pin_di' : 'A7',
            'pin_do' : 'A6',
            'pin_clk' : 'A5' }
};

# left-right, or top-bottom order
board = {
};
board["_css"] = """
""";

def get_pins():
  pins = pinutils.scan_pin_file([], 'stm32f103xe.csv', 6, 10, 11)
  return pinutils.only_from_package(pinutils.fill_gaps_in_pin_list(pins), chip["package"])
