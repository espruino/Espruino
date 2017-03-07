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
import copy;
info = {
 'name' : "Embedded PI / COOCOX",
 'link' :  [ "http://coocox.org" ],
 'variables' : 715,
 'binary_name' : 'espruino_%v_embedded_pi.bin',
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
  'part' : "STM32F103RB", #T6
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
};

devices = {
#  'OSC' : { 'pin_1' :  'D0',
#            'pin_2' : 'D1' },
  'OSC_RTC' : { 'pin_1' :  'D22',
                'pin_2' : 'D23' },
  'LED1' : { 'pin' : 'D13' },
#  'LED2' : { 'pin' : 'D3' },
  'BTN1' : { 'pin' : 'D38' }, # 'C9'
  'USB' : { 'pin_disc' :  'D39',
            'pin_dm' : 'D40',
            'pin_bp' : 'D41'
          },
 # 'SD' :  { 'pin_cs' :  'D25',#'D2',
 #           'pin_di' :  'D34',#'B15',
 #           'pin_do' :  'D33',#'B14',
 #           'pin_clk' :  'D32'}, #'B13'
};

# left-right, or top-bottom order
board = {
  'top' : [ 'D15','D14', 'AREF','GND', 'D13', 'D12', 'D11','D10', 'D9', 'D8', '', 'D7', 'D6', 'D5', 'D4', 'D3', 'D2', 'D1', 'D0'],
  'top2' : [ 'D39','D38','D37', 'D36', 'D35', 'D34', 'D33','D32', 'D31', 'D30', '', 'D29', 'D28', 'D27', 'D26', 'D25', 'D24', 'D23', 'D22'],
  'bottom' : [ 'NC','DVCC','RST', '3.3', '5', 'GNDA', 'GND', 'VIN', '', 'D16', 'D17', 'D18', 'D19', 'D14', 'D15'],
  'bottom2' : [ 'BOOT0','BOOT1','RST', '3.3', 'NC', 'GND', 'D26', 'D28', '', 'D40', 'D41', 'D42', 'D43', 'D44', 'D45'],

  'right2' : [ 'NC','D13','D12' ],
  'right' : [ 'GND','D11','NC' ],

};
board["right"].reverse()
board["right2"].reverse()
board["_css"] = """
#board {
  width: 1052px;
  height: 506px;
  top: 300px;
  left: 200px;
  background-image: url(img/EMBEDDED_PI.jpg);
}
#boardcontainer {
  height: 850px;
}

#top {
  top: -20px;
  left: 540px;
}
#top2 {
  top: 60px;
  left: 540px;
}
#bottom  {
  top: 500px;
  left: 650px;
}
#bottom2  {
  top: 420px;
  left: 650px;
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
  top: 200px;
  left: 1330px;
}
#right2  {
  top: 200px;
  right: -270px;
}

""";



def get_pins():
  pins = pinutils.scan_pin_file([], 'stm32f103xb.csv', 6, 10, 11)
  # Embedded Pi Mapping
  pinmapping = {
    'D0' :'PC11',
    'D1' :'PC10',
    'D2' :'PC12',
    'D3' :'PC6',
    'D4' :'PC7',
    'D5' :'PC8',
    'D6' :'PC9',
    'D7' :'PD2',
    'D8' :'PA15',
    'D9' :'PA8',
    'D10':'PB12',
    'D11':'PB15',
    'D12':'PB14',
    'D13':'PB13',
    'D14':'PB7',
    'D15':'PB6',
    'D16':'PC0', # share with D40
    'D17':'PC1', # share with D41
    'D18':'PC2', # share with D42
    'D19':'PC3', # share with D43
    'D20':'PB7', # share with D14
    'D21':'PB6', # share with D15
    'D22':'PA3',
    'D23':'PA2',
    'D24':'PA1',
    'D25':'PA0',
    'D26':'PA9',
    'D27':'PB0',
    'D28':'PA10',
    'D29':'PB1',
    'D30':'PB8',
    'D31':'PB9',
    'D32':'PA4',
    'D33':'PA7',
    'D34':'PA6',
    'D35':'PA5',
    'D36':'PC13',
    'D37':'PB5',
    'D38':'PB11',
    'D39':'PB10',
    'D40':'PC0',
    'D41':'PC1',
    'D42':'PC2',
    'D43':'PC3',
    'D44':'PC4',
    'D45':'PC5',
  };
  newpins = []
  for newname in pinmapping:
    print newname+" => "+pinmapping[newname]
    pin = copy.deepcopy(pinutils.findpin(pins, pinmapping[newname], True))
    pin["name"] = "P"+newname
    pin["sortingname"] = newname[0] + newname[1:].rjust(2,'0')
    newpins.append(pin)
  # Because 'pinmapping' is NOT stored in order!!!
  newpins = sorted(newpins, key=lambda pin: pin["sortingname"])
#  print(json.dumps(newpins, sort_keys=True, indent=2))
  return newpins
