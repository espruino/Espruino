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
import json;
info = {
 'name' : "Olimexino STM32 / Leaflabs Maple",
 'link' :  [ "https://www.olimex.com/Products/Duino/STM32/OLIMEXINO-STM32/", "http://leaflabs.com/devices/maple/" ],
 'variables' : 1020,
 'binary_name' : 'espruino_%v_olimexino_stm32.bin',
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
  'part' : "STM32F103RBT6",
  'family' : "STM32F1",
  'package' : "LQFP64",
  'ram' : 20,
  'flash' : 128,
  'speed' : 72,
  'usart' : 3,
  'spi' : 2,
  'i2c' : 2,
  'adc' : 3,
  'dac' : 0,
  'saved_code' : {
    'address' : 0x08000000 + ((128-4)*1024),
    'page_size' : 1024, # size of pages
    'pages' : 4, # number of pages we're using
    'flash_available' : 128-4 # 4 used for code
  },
};

devices = {
#  'OSC' : { 'pin_1' :  'D0',
#            'pin_2' : 'D1' },
  'OSC_RTC' : { 'pin_1' :  'D22',
                'pin_2' : 'D23' },
  'LED1' : { 'pin' : 'D13' },
  'LED2' : { 'pin' : 'D3' },
  'BTN1' : { 'pin' : 'D38' }, # 'C9'
  'USB' : { 'pin_disc' :  'D39',
            'pin_dm' : 'D40',
            'pin_dp' : 'D41'
          },
  'SD' :  { 'pin_cs' :  'D25',#'D2',
            'pin_di' :  'D34',#'B15',
            'pin_do' :  'D33',#'B14',
            'pin_clk' :  'D32'}, #'B13'
};

# left-right, or top-bottom order
board = {
  'top' : [ 'D14', 'GND', 'D13', 'D12', 'D11','D10', 'D9', 'D8', '', 'D7', 'D6', 'D5', 'D4', 'D3', 'D2', 'D1', 'D0'],
  'bottom' : [ 'RST', '3.3', '3.3A', 'GNDA', 'GND', 'VIN', '', 'A0', 'A1', 'A2', 'A3', 'A4', 'A5'],
  'right' : [ 'D23','D25','D27','D29','D31','D33','D35','D37' ],
  'right2' : [ 'D24','D26','D28','D30','D32','D34','D36','GND' ],

  'left' : [ '3V3','D7','D29','D12','D13' ],
  'left2' : [ 'GND','D8','D20','D11','D4' ],

  '_pinmap' : { 'A0':'D15', 'A1':'D16', 'A2':'D17', 'A3':'D18', 'A4':'D19', 'A5':'D20' }
};
board["left"].reverse()
board["left2"].reverse()
board["right"].reverse()
board["right2"].reverse()
board["_css"] = """
#board {
  width: 540px;
  height: 418px;
  top: 300px;
  left: 200px;
  background-image: url(img/OLIMEXINO_STM32.jpg);
}
#boardcontainer {
  height: 850px;
}

#top {
  top: -20px;
  left: 140px;
}
#bottom  {
  top: 431px;
  left: 220px;
}

#left {
  top: 155px;
  right: 520px;

}
#left2  {
  top:155px;
  left: 20px;
}

#right {
  top: 155px;
  left: 520px;
}
#right2  {
  top: 155px;
  right: 20px;
}

""";



def get_pins():
  pins = pinutils.scan_pin_file([], 'stm32f103xb.csv', 6, 10, 11)
  # Olimexino/Maple pins have stupid names
  pinmapping = {
    'D0' :'PA3',
    'D1' :'PA2',
    'D2' :'PA0',
    'D3' :'PA1',
    'D4' :'PB5',
    'D5' :'PB6',
    'D6' :'PA8',
    'D7' :'PA9',
    'D8' :'PA10',
    'D9' :'PB7',
    'D10':'PA4',
    'D11':'PA7',
    'D12':'PA6',
    'D13':'PA5',
    'D14':'PB8',
    'D15':'PC0', # shared with A0-A15
    'D16':'PC1',
    'D17':'PC2',
    'D18':'PC3',
    'D19':'PC4',
    'D20':'PC5',
    'D21':'PC13',
    'D22':'PC14',
    'D23':'PC15',
    'D24':'PB9',
    'D25':'PD2',
    'D26':'PC10',
    'D27':'PB0',
    'D28':'PB1',
    'D29':'PB10',
    'D30':'PB11',
    'D31':'PB12',
    'D32':'PB13',
    'D33':'PB14',
    'D34':'PB15',
    'D35':'PC6',
    'D36':'PC7',
    'D37':'PC8',
    'D38':'PC9', # for button
    'D39':'PC12', # for USB disc
    'D40':'PA11', # for USB dm
    'D41':'PA12', # for USB dp
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
