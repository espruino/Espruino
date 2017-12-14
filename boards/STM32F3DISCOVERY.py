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
 'name' : "STM32 F3 Discovery",
 'link' :  [ "http://www.st.com/web/catalog/tools/FM116/SC959/SS1532/PF254044" ],
 'variables' : 1720,
 'binary_name' : 'espruino_%v_stm32f3discovery.bin',
 'build' : {
   'optimizeflags' : '-Os',
   'libraries' : [
     'NET',
     'GRAPHICS',
     'NEOPIXEL'
   ],
   'makefile' : [
     'STLIB=STM32F3XX',
     'PRECOMPILED_OBJS+=$(ROOT)/targetlibs/stm32f3/lib/startup_stm32f30x.o'
   ]
  }
};
chip = {
  'part' : "STM32F303VCT6",
  'family' : "STM32F3",
  'package' : "LQFP100",
  'ram' : 40,
  'flash' : 256,
  'speed' : 72,
  'usart' : 3,
  'spi' : 2,
  'i2c' : 2,
  'adc' : 4,
  'dac' : 2,
};

devices = {
  'OSC' : { 'pin_1' : 'F0',
            'pin_2' : 'F1' },
  'OSC_RTC' : { 'pin_1' : 'C14',
                'pin_2' : 'C15' },
  'LED1' : { 'pin' : 'E9' },
  'LED2' : { 'pin' : 'E8' },
  'LED3' : { 'pin' : 'E10' },
  'LED4' : { 'pin' : 'E15' },
  'LED5' : { 'pin' : 'E11' },
  'LED6' : { 'pin' : 'E14' },
  'LED7' : { 'pin' : 'E12' },
  'LED8' : { 'pin' : 'E13' },
  'BTN1' : { 'pin' : 'A0' },

  'USB' : { 'pin_otg_pwr' : 'C0',#
            'pin_dm' : 'A11',
            'pin_dp' : 'A12',
            'pin_vbus' : 'A9',#
            'pin_id' : 'A10', },#
  'GYRO' : { 'device' : 'L3GD20',
             'pin_cs' : 'E3',
             'pin_sck' : 'A5',
             'pin_miso' : 'A6',
             'pin_mosi' : 'A7',
             'pin_int1' : 'E0',
             'pin_int2' : 'E1',
           },
  'MEMS' :  {  'device' : 'LSM303DLHC)',
            'pin_sda' :  'B7',
            'pin_scl' :  'B6',
            'pin_drdy' :  'E2',
            'pin_int1' :  'E4',
            'pin_int2' :  'E5',
   },
  'MIC' :  { 'device' : 'MP45DT02',
             'pin_clk' :  'C3',
             'pin_dout' :  'B10', },
  'AUDIO' :  { 'device' : 'CS43L22',
               'pin_sda' :  'B9',
               'pin_scl' :  'B6',
               'pin_mclk' :  'C7',
               'pin_sclk' :  'C10',
               'pin_sdin' :  'C12',
               'pin_lrck' :  'A4',
               'pin_nrst' :  'D4',
                },
  'JTAG' : {
        'pin_MS' : 'A13',
        'pin_CK' : 'A14',
        'pin_DI' : 'A15'
          },
};

# left-right, or top-bottom order
board = {
  'left' : [ '3V','GND','C1','C3','A1','A3','F4','A5','A7','C5','B1','E7','E9','E11','E13','E15','B11','B13','B15','D9','D11','D13','D15','C6','GND' ],
  'left2' : [ '3V','NRST','C0','C2','F2','A0','A2','A4','A6','C4','B0','B2','E8','E10','E12','E14','B10','B12','B14','D8','D10','D12','D14','C7','GND' ],
  'right2' : [ '5V','F9','D0','C14','E6','E4','E2','E0','B8','BOOT0','B6','B4','D7','D5','D3','D1','C12','C10','A14','F6','A12','A10','A8','C8','GND' ],
  'right' : [ '5V','F10','F1','C15','C13','E5','E3','E1','B9','VDD','B7','B5','B3','D6','D4','D2','D0','C11','A15','A13','A11','A9','C9','NC','GND' ],
};
board["_css"] = """
#board {
  width: 598px;
  height: 891px;
  left: 200px;
  background-image: url(img/STM32F3DISCOVERY.jpg);
}
#boardcontainer {
  height: 891px;
}
#left {
  top: 320px;
  right: 530px;
}
#left2 {
  top: 320px;
  left: 85px;
}
#right2  {
  top: 320px;
  right: 80px;
}
#right  {
  top: 320px;
  left: 530px;
}
""";

def get_pins():
  pins = pinutils.scan_pin_file([], 'stm32f303.csv', 3, 6, 7)
  pins = pinutils.scan_pin_af_file(pins, 'stm32f303_af.csv', 1, 2)
#  print(json.dumps(pins, sort_keys=True, indent=2))
#  return pinutils.only_from_package(pinutils.fill_gaps_in_pin_list(pins), chip["package"])
  return pinutils.fill_gaps_in_pin_list(pins)
