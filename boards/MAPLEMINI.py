#!/bin/false
# This file is part of Espruino, a JavaScript interpreter for Microcontrollers
#
# Copyright (C) 2013 Gordon Williams <gw@pur3.co.uk>
# Copyright (C) 2015 Anton Eltchaninov <anton.eltchaninov@gmail.com>
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
import json;
info = {
 'name' : "Leaflabs Maple Mini",
 'link' :  [ "http://leaflabs.com/docs/hardware/maple-mini.html" ],
 'variables' : 715,
 'binary_name' : 'espruino_%v_maplemini.bin',
 'build' : {
   'optimizeflags' : '-Os',
   'libraries' : [
     'FILESYSTEM'
   ],
   'makefile' : [
     'SAVE_ON_FLASH=1',
     'STLIB=STM32F10X_MD',
     'PRECOMPILED_OBJS+=$(ROOT)/targetlibs/stm32f1/lib/startup_stm32f10x_md.o'
   ]
 }
};

chip = {
  'part' : "STM32F103CB", #T6
  'family' : "STM32F1",
  'package' : "LQFP48",
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
#  'OSC' : { 'pin_1' :  'D0',
#            'pin_2' : 'D1' },
  'LED1' : { 'pin' : 'D33' },
  'BTN1' : { 'pin' : 'D32' },
  'USB' : { 'pin_disc' :  'D34',
            'pin_dm' : 'D24',
            'pin_dp' : 'D23'
          },
};

# left-right, or top-bottom order
board = {
  'top' : [ 'AV+', 'AV-', 'VBAT', 'D14', 'D13', 'D12', 'RST', 'A0', 'A1', 'A2', 'A3', 'A4', 'A5', 'A6', 'A7', 'A8', 'D2', 'D1', 'D0', 'VIN' ],
  'bottom' : [ 'VCC', 'GND', 'BUT', 'D15', 'D16', 'D17', 'D18', 'D19', 'D20', 'D21', 'D22', 'D23', 'D24', 'D25', 'D26', 'D27', 'D28', 'D29', 'D30', 'D31'],
  '_pinmap' : { 'A0':'D11', 'A1':'D10', 'A2':'D9', 'A3':'D8', 'A4':'D7', 'A5':'D6', 'A6':'D5', 'A7':'D4', 'A8':'D3'}
};

def get_pins():
  pins = pinutils.scan_pin_file([], 'stm32f103xb.csv', 6, 10, 11)
  # Olimexino/Maple pins have stupid names
  pinmapping = {
    'D0' :'PB11',
    'D1' :'PB10',
    'D2' :'PB2',
    'D3' :'PB0',
    'D4' :'PA7',
    'D5' :'PA6',
    'D6' :'PA5',
    'D7' :'PA4',
    'D8' :'PA3',
    'D9' :'PA2',
    'D10':'PA1',
    'D11':'PA0',
    'D12':'PC15',
    'D13':'PC14',
    'D14':'PC13',
    'D15':'PB7',
    'D16':'PB6',
    'D17':'PB5',
    'D18':'PB4',
    'D19':'PB3',
    'D20':'PA15',
    'D21':'PA14',
    'D22':'PA13',
    'D23':'PA12',# for USB dp
    'D24':'PA11',# for USB dm
    'D25':'PA10',
    'D26':'PA9',
    'D27':'PA8',
    'D28':'PB15',
    'D29':'PB14',
    'D30':'PB13',
    'D31':'PB12',
    'D32':'PB8',# for button
    'D33':'PB1',
    'D34':'PB9',# for USB disc
  };
  newpins = []
  for newname in pinmapping:
#    print newname
    pin = pinutils.findpin(pins, pinmapping[newname], True)
    pin["name"] = "P"+newname
    pin["sortingname"] = newname[0] + newname[1:].rjust(2,'0')
    newpins.append(pin)
  # Because 'pinmapping' is NOT stored in order!!!
  newpins = sorted(newpins, key=lambda pin: pin["sortingname"])
#  print(json.dumps(newpins, sort_keys=True, indent=2))
  return newpins
