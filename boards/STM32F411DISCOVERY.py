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
 'name' : "STM32 F411 Discovery",
 'link' :  [ "http://www.st.com/content/st_com/en/products/evaluation-tools/product-evaluation-tools/mcu-eval-tools/stm32-mcu-eval-tools/stm32-mcu-discovery-kits/32f411ediscovery.html" ],
 'default_console' : "EV_SERIAL2", # FIXME: This was S2 because of pin conflict. Not sure if it's really an issue?
 'variables' : 5450,
 'binary_name' : 'espruino_%v_stm32f411discovery.bin',
 'build' : {
   'optimizeflags' : '-O3',
   'libraries' : [
     'NET',
     'GRAPHICS',
     'NEOPIXEL'
   ],
   'makefile' : [
     'DEFINES+=-DUSE_USB_OTG_FS=1',
     'STLIB=STM32F411xE',
     'PRECOMPILED_OBJS+=$(ROOT)/targetlibs/stm32f4/lib/startup_stm32f401xx.o'
   ]
  }
};
chip = {
  'part' : "STM32F411RET6",
  'family' : "STM32F4",
  'package' : "LQFP100",
  'ram' : 128, # 0x0001 8000 long, from 0x2000 0000 to 0x2001 7FFF
  'flash' : 512, # 0x0008 0000 long, from 0x0800 0000 to 0x0807 FFFF
  'speed' : 100,
  'usart' : 3,
  'spi' : 5,
  'i2c' : 3,
  'adc' : 1,
  'dac' : 0,
  'saved_code' : {
    # code size 225248 = 0x36FE0 starts at 0x0800 0000 ends at 0x0803 6FE0
    # so we have some left room for Espruino firmware and no risk to clear it while saving
    'address' : 0x08060000, # flash_saved_code_start 0x0806 0000 to 0x807 5000
    # we have enough flash space in this single flash page to save all of the ram
    'page_size' :  131072, # size of pages : on STM32F411, last 2 pages are 128 Kbytes
    # we use the last flash page only, furthermore it persists after a firmware flash of the board
    'pages' : 1, # count of pages we're using to save RAM to Flash,
    'flash_available' : 512 # binary will have a hole in it, so we just want to test against full size
  },
};

devices = {
  'OSC' : { 'pin_1' : 'H0',
            'pin_2' : 'H1' },
  'OSC_RTC' : { 'pin_1' : 'C14',
                'pin_2' : 'C15' },
  'LED1' : { 'pin' : 'D13' },
  'LED2' : { 'pin' : 'D12' },
  'LED3' : { 'pin' : 'D14' },
  'LED4' : { 'pin' : 'D15' },
  'BTN1' : { 'pin' : 'A0', 'pinstate' : 'IN_PULLDOWN' },
  'USB' : { 'pin_otg_pwr' : 'C0',
            'pin_dm' : 'A11',
            'pin_dp' : 'A12',
            'pin_vbus' : 'A9',
            'pin_id' : 'A10', },
  'MEMS' :  {  'device' : 'LIS302DL',
            'pin_cs' :  'E3',
            'pin_int1' :  'E0',
            'pin_int2' :  'E1',
            'pin_mosi' :  'A7',
            'pin_miso' :  'A6',
            'pin_sck' :  'A5' },
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
  'left' : [ 'GND', 'VDD', 'GND', 'C1','C3','A1','A3','A5','A7','C5','B1','GND','E7','E9','E11','E13','E15','B11','B13','B15','D9','D11','D13','D15','GND'],
  'left2' : [ 'GND', 'VDD', 'NRST', 'C0','C2','A0','A2','A4','A6','C4','B0','B2','E8','E10','E12','E14','B10','B12','B14','D8','D10','D12','D14','NC','GND'],
  'right2' : [ 'GND', '5V', '3V3', 'H0', 'C14','E6','E4','E2','E0','B8','BOOT0','B6','B4','D7','D5','D3','D1','C12','C10','A14','A10','A8','C8','C6','GND'],
  'right' : [ 'GND', '5V', '3V3', 'H1', 'C15','C13','E5','E3','E1','B9','VDD','B7','B5','B3','D6','D4','D2','D0','C11','A15','A13','A9','C9','C7','GND'],
};
board["_css"] = """
#board {
  width: 680px;
  height: 1020px;
  left: 200px;
  background-image: url(img/STM32F411DISCOVERY.jpg);
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
