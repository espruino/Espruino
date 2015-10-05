#!/bin/false
# -*- coding: utf8 -*-
# This file is part of Espruino, a JavaScript interpreter for Microcontrollers
#
# Copyright (C) 2013 Gordon Williams <gw@pur3.co.uk>
# Copyright (C) 2014 Alain Sézille for NucleoF411RE specific lines of this file
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
  'name' : "EMW3165",
  'link' :  [ "http://www.mxchip.com/wireless/downloadcenter/wifi/32.html"],
  'default_console' : "EV_SERIAL2", # USART2 by default, the Nucleo's USB is actually running on this too
  'default_console_tx' : "A2", # USART2_TX on PA2,
  'default_console_rx' : "A3", # USART2_RX on PA3
  'variables' :  2048, # was 7423: (128-12)*1024/16-1
  'binary_name' : 'espruino_%v_emw3165.bin',
};
chip = {
  'part' : "STM32F411CE",
  'family' : "STM32F4",
  'package' : "UQFN48",
  'ram' : 128, # 0x0001 8000 long, from 0x2000 0000 to 0x2001 7FFF
  'flash' : 512, # 0x0008 0000 long, from 0x0800 0000 to 0x0807 FFFF
  'speed' : 100,
  'usart' : 3,
  'spi' : 4,
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
  #'place_text_section' : 0x08010000, # note flash_available above # TODO USELESS
};
# left-right, or top-bottom order
board = {
  'top' :   [ 'NC', 'PB2', 'NC', 'PA7', 'PA15', 'PB3', 'PB4', 'PA2', 'PA1', 'VBAT', 'NC', 'PA3', 'NRST', 'PA0', 'NC', 'PC13', 'PB10', 'PB9', 'PB12', 'GND' ],
  'bottom' :  [ 'GND', 'NC', 'NC', 'NC', 'PA14', 'PA13', 'PA12', 'NC', 'PA10', 'PB6', 'PB8', 'NC', 'PB13', 'PA5', 'PA11', 'PB1', 'PB0', 'PA4', 'VDD', 'VDD', 'ANT' ],
};
devices = {
  #?? 'OSC' : { 'pin_1' : 'H0', # MCO from ST-LINK fixed at 8 Mhz, boards rev MB1136 C-02
  #??           'pin_2' : 'H1' },
  #?? 'OSC_RTC' : { 'pin_1' : 'C14', # MB1136 C-02 corresponds to a board configured with on-board 32kHz oscillator
  #??               'pin_2' : 'C15' },
  #?? 'NUCLEO_A' : [ 'A0','A1','A4','B0','C1','C0' ],
  #?? 'NUCLEO_D' : [ 'A3','A2','A10','B3','B5','B4','B10','A8','A9','C7','B6','A7','A6','A5','B9','B8' ],
};


board_css = """
""";

def get_pins():
  pins = pinutils.scan_pin_file([], 'stm32f401.csv', 5, 8, 9)
  pins = pinutils.scan_pin_af_file(pins, 'stm32f401_af.csv', 0, 1)
  return pinutils.only_from_package(pinutils.fill_gaps_in_pin_list(pins), chip["package"])
