#!/bin/false
# -*- coding: utf8 -*-
# This file is part of Espruino, a JavaScript interpreter for Microcontrollers
#
# Copyright (C) 2013 Gordon Williams <gw@pur3.co.uk>
# Copyright (C) 2017 ST for DiscoL496G specific lines of this file
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
 'name' : "STM32L496 Discovery",
 'link' :  [ "http://www.st.com/en/evaluation-tools/32l496gdiscovery.html" ],
 'espruino_page_link' : 'STM32L496GDISCOVERY',
 'default_console' : "EV_SERIAL2",
 'default_console_tx' : "A2",
 'default_console_rx' : "D6",
 'variables' :  19200, 	# variables computed from available RAM size : (256-16)*1024/16-1
 'binary_name' : 'espruino_%v_stm32l496gdiscovery.bin',
 'build' : {
   'optimizeflags' : '-O3',
   'libraries' : [
     'NET',
     'GRAPHICS',
     'NEOPIXEL',
     'CRYPTO',
     'TLS',
     'FILESYSTEM'
   ],
   'makefile' : [
     'WRAPPERSOURCES+=targets/nucleo/jswrap_nucleo.c',
     'DEFINES+=-DUSE_USB_OTG_FS=1',
     'STLIB=STM32L496xx',
     'PRECOMPILED_OBJS+=$(ROOT)/targetlibs/stm32l4/lib/CMSIS/Device/ST/STM32L4xx/Source/Templates/gcc/startup_stm32l496xx.o'
   ]
  }
};

chip = {
  'part' : "STM32L496RG",
  'family' : "STM32L4",
  'package' : "UFBGA169",
  'ram' : 320,
  'flash' : 1024,
  'speed' : 80,
  'usart' : 3,
  'spi' : 4,
  'i2c' : 3,
  'adc' : 3,
  'dac' : 2,
  'saved_code' : {
    # code size (with debug) : 448000 ~ 0x6D800 starts at 0x0800 0000 ends at 0x0806 D800
    # so we have some left room for Espruino firmware and no risk to clear it while saving
    'address' : 0x080D0000, # We have 0 -> 0xFFFFF (1MB), so this is at the end of flash
    'page_size' :  2048, # size of pages, 256 pages on bank1, 256 pages on bank2
    'pages' : 64, # count of pages we're using to save RAM to Flash
    'flash_available' : 832 # kb - quantity reserved to receive the Firmware
  },
};

devices = {
  'OSC' : { 'pin_1' : 'H0', 	# from LCD pwr on
            'pin_2' : 'H1' }, 	# to mic vdd
  'OSC_RTC' : { 'pin_1' : 'C14', # OSC32_IN (32kHz oscillator)
                'pin_2' : 'C15' }, # OSC32_OUT
  'BTN1' : { 'pin' : 'C13', 'pinstate' : 'IN_PULLDOWN' }, # joy center
#  'BTN2' : { 'pin' : 'I9', 'pinstate' : 'IN_PULLDOWN' }, # joy l
#  'BTN3' : { 'pin' : 'F11', 'pinstate' : 'IN_PULLDOWN' }, # joy r
#  'BTN4' : { 'pin' : 'I8', 'pinstate' : 'IN_PULLDOWN' }, # joy u
#  'BTN5' : { 'pin' : 'I10', 'pinstate' : 'IN_PULLDOWN' }, # joy d
  'LED2' : { 'pin' : 'B13' },
  'JTAG' : {
        'pin_MS' : 'A13', # TMS/SWDIO
        'pin_CK' : 'A14', # TCK/SWCLK
        'pin_DI' : 'A15'
          },
  'USB' : { 'pin_charge' :  'A10', # OTGFS_ID
            'pin_vsense' :  'A9', # OTGFS_VBUS
            'pin_dm' : 'A11',   # OTGFS_DM
            'pin_dp' : 'A12' }, # OTGFS_DP

  'NUCLEO_A' : [ 'C4','C1','C3','F10','A1','C0' ],
  'NUCLEO_D' : [ 'G8','G7','G13','H15','I11','B9','I6','G6','G15','H13','A15','B5','B4','A5','B7','B8' ],

  'SD' :  { 'pin_cmd' :  'D2',
            'pin_d0' :  'C8',
            'pin_d1' :  'C9',
            'pin_d2' :  'C10',
            'pin_d3' :  'C11',
            'pin_clk' :  'C12' },
};

# left-right, or top-bottom order
board = {
  # recto side
  # STMOD+ connector
  'top2' : [ 'G11', 'B6', 'G10', 'G12', 'GND','5V', 'B8', 'I3', 'D3', 'B7' ],
  'top' : [ 'H2', 'B2', 'A4', 'A0', '5V','GND', 'C7', 'C2', 'B12', 'C2' ],
  # PMOD+ connector
  'right' : [ 'H2','B2','NC','NC','GND','3V3' ],
  'right2' : [ 'G11','B6','G10','G12','GND','3V3' ],

  # verso side
  # ARDUINO connector
  'bottom2' : [ 'C4','C1','C3','F10','C1','C4','','VIN','GND','5V','3V3','RESET','VDD (IOREF)','' ],
  'bottom' : [ 'G8','G7','G13','H15','I11','B9','I6','G6', '', 'G15','H13','A15','B5','B4','A5','GND','AVDD','B7','B8' ],
};
board["_css"] = """
#board {
  width: 800px;
  height: 864px;
  top: 250px;
  background-image: url(img/STM32L496GDISCOVERY.jpg);
}
#boardcontainer {
  height: 1450px;
}
#top {
  bottom: 843px;
  left: 518px;
}
#top2 {
  top: 20px;
  left: 518px;
}

#right  {
  top: 55px;
  left: 782px;
}
#right2  {
  top: 55px;
  right: 20px
}
#bottom  {
  top: 796px;
  right: 221px;
}
#bottom2  {
  bottom: 360px;
  right: 283px;
}

.toppin { width: 15px; padding:0px; }
.top2pin { width: 15px; padding:0px; }
.rightpin { height: 15px; }
.right2pin { height: 15px;  }
.bottompin { width: 15px; padding:0px; }
.bottom2pin { width: 15px; padding:0px; }
""";

def get_pins():
  pins = pinutils.scan_pin_file([], 'stm32l496.csv', 6, 10, 11)
  pins = pinutils.scan_pin_af_file(pins, 'stm32l496_af.csv', 0, 1)
  return pinutils.only_from_package(pinutils.fill_gaps_in_pin_list(pins), chip["package"])
