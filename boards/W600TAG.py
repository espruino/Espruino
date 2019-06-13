#!/bin/false
# This file is part of Espruino, a JavaScript interpreter for Microcontrollers
#
# Copyright (C) 2019 WangJun <beanjs>
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
 'name' : "W600TAG",
 'link' :  [ "" ],
 'default_console' : "EV_SERIAL1",
 'default_console_baudrate' : "115200",
 'default_console_tx' : "A4",
 'default_console_rx' : "A5",
 'variables' : 1000,
 'binary_name' : 'espruino_%v_w600.bin',
 'build' : {
   'libraries' : [
     #'NET'
   ],
   'makefile' : [
   ]
 }
};

chip = {
  'part' : "W600",
  'family' : "W600",
  'package' : "",
  'ram' : 288,
  'flash' : 1024,
  'speed' : 80,
  'usart' : 2,
  'spi' : 2,
  'i2c' : 1,
  'adc' : 0 ,
  'dac' : 0,
  'saved_code' : {
    'address' :  0x80F0000, # wm_flash_map.h
    'page_size' : 4096,
    'pages' : 1,
    'flash_available':512
  },
};

#******************LAYOUT For 1M Flash**********************
#    *Reserved     	0x8000000-0x8010000	    64Kbyte
#    *Code   		0x8010100-0x808FFFF	    512Kbyte - 256 byte
#    *Update		0x8090000-0x80EFFFF	    384Kbyte
#    *User			0x80F0000-0x80FBFFF	    48Kbyte    
#    *Parameter	0x80FC000-0x80FFFFF          16Kbyte
#*********************************************************

#**************LAYOUT For 2M Flash as 1M LAYOUT**************
#    *Reserved     		0x8000000-0x8010000	    64Kbyte
#    *Code   			0x8010100-0x80FFFFF	    896Kbyte -256byte
#    *Old  User Area	0x80F0000-0x80FBFFF          48Kbyte
#    *Parameter		0x80FC000-0x80FFFFF          16Kbyte
#    *Update			0x8100000-0x81AFFFF	    704Kbyte
#    *EXT User			0x81B0000-0x81FFFFF	    320Kbyte    
#*********************************************************

#******************LAYOUT For 2M Flash*********************
#    *Reserved     	0x8000000-0x8010000	    64Kbyte
#    *Code   		0x8010100-0x80FFFFF	    960Kbyte -256byte
#    *Update		0x8100000-0x81BFFFF	    768Kbyte
#    *User			0x81C0000-0x81FBFFF	    240Kbyte    
#    *Parameter	0x81FC000-0x81FFFFF          16Kbyte
#*********************************************************


devices = {
#  'LED1' : { 'pin' : 'D22' },
#  'LED2' : { 'pin' : 'D21' },
#  'LED3' : { 'pin' : 'D23' }
};

def get_pins():
  pins = []
  pinutils.findpin(pins,"PA0",False)["functions"]["TIM1_CH1"]=0
  pinutils.findpin(pins,"PA1",False)["functions"]["TIM1_CH2"]=0
  pinutils.findpin(pins,"PA4",False)["functions"]["UART1_TX"]=0
  pinutils.findpin(pins,"PA5",False)["functions"]["UART1_RX"]=0

  pinutils.findpin(pins,"PB6",False)["functions"]["TIM1_CH4"]=0
  pinutils.findpin(pins,"PB7",False)
  pinutils.findpin(pins,"PB8",False)["functions"]["TIM1_CH5"]=0
  pinutils.findpin(pins,"PB9",False)
  pinutils.findpin(pins,"PB10",False)
  pinutils.findpin(pins,"PB11",False)["functions"]["UART2_RX"]=0
  pinutils.findpin(pins,"PB12",False)["functions"]["UART2_TX"]=0

  pinutils.findpin(pins,"PB13",False)["functions"]["I2C1_SCL"]=0
  pinutils.findpin(pins,"PB14",False)["functions"]["I2C1_SDA"]=0
  pinutils.findpin(pins,"PB15",False)
  pinutils.findpin(pins,"PB16",False)["functions"]["SPI1_SCK"]=0
  pinutils.findpin(pins,"PB17",False)["functions"]["SPI1_MISO"]=0
  pinutils.findpin(pins,"PB18",False)["functions"]["SPI1_MOSI"]=0



  # pins = pinutils.generate_pins(0,15,"A") + pinutils.generate_pins(0,31,"B");
  return pins
