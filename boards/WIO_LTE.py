
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
 'name' : "WIO TRACKER LTE",
 'link' :  [ "http://www.espruino.com/WioLTE" ],
 'espruino_page_link' : 'WioLTE', 
 'default_console' : "EV_SERIAL2", # FIXME: This was S2 because of pin conflict. Not sure if it's really an issue?
 'variables' : 5450,
 'binary_name' : 'espruino_%v_Wio_LTE.bin',
 'build' : {
   'optimizeflags' : '-O3',
   'libraries' : [
     'NET',
     'NEOPIXEL',
     'FILESYSTEM',  # Add FILESYSTEM will force javascript module to load from SD card, remain to be seen.
     'WIO_LTE'
   ],
   'makefile' : [
     'DEFINES+=-DUSE_USB_OTG_FS=1',
     'DEFINES+=-DPIN_NAMES_DIRECT=1', # Package skips out some pins, so we can't assume each port starts from 0
     'DEFINES+=-DWIO_LTE',
     'USE_DFU=1',
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
  'USB' : { 'pin_otg_pwr' : 'C0',
            'pin_dm' : 'A11',
            'pin_dp' : 'A12',
            'pin_vbus' : 'A9',
            'pin_id' : 'A10', },
  'JTAG' : {
        'pin_MS' : 'A13',
        'pin_CK' : 'A14',
        #'pin_DI' : 'A15'
          },
  'PWR' : { 
            'pin_dtr' : 'A1',
            'pin_neopixel_pwr' : 'A8',
            'pin_sdcard_pwr' : 'A15',
            'pin_battery_read' : 'B0',
            'pin_lte_pwr' : 'B5',
            'pin_vccb' : 'B10',
            'pin_ant_pwr' : 'B12',
            'pin_lte_status' : 'B15',
            'pin_wakeup_in' : 'C0',
            'pin_ap_ready' : 'C1',
            'pin_wakeup_disable' : 'C2',
            'pin_lte_reset' : 'C3',
            'pin_pwr_key' : 'C4',
            'pin_codec_i2c_pwr' : 'C5' },              
};

# left-right, or top-bottom order
board = {
  'left' : [ 'GND', '3V3', 'A5', 'A4', '', '', 'GND', '3V3', 'B6', 'B7', '', '', 'GND', '3V3', 'B9', 'B8'],
  'right' : [ 'A6', 'A7', '3V3', 'GND', '', '', 'B4', 'B3', '3V3', 'GND', '', '', 'C6', 'C7', '3V3', 'GND'],
};
board["_css"] = """
#board {
  width: 457px;
  height: 480px;
  left: 200px;
  background-image: url(img/WIO_LTE.jpg);
}
#boardcontainer {
  height: 468px;
}
#left {
  top: 98px;
  right: 352px;
}
#right  {
  top: 98px;
  left: 352px;
}
.leftpin { height: 18px; padding:0px; }
.rightpin { height: 18px; padding:0px; }
""";

def get_pins():
  pins = pinutils.scan_pin_file([], 'stm32f40x.csv', 6, 9, 10)
  return pinutils.only_from_package(pinutils.fill_gaps_in_pin_list(pins), chip["package"])
