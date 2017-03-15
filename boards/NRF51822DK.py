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
 'name' : "nRF51 Development Kit",
 'link' :  [ "https://www.nordicsemi.com/Products/Bluetooth-Smart-Bluetooth-low-energy/nRF51822" ],
  # This is the PCA10028
 'default_console' : "EV_SERIAL1",
 'default_console_tx' : "D9",
 'default_console_rx' : "D11",
 'default_console_baudrate' : "9600",
 'variables' : 1050,
 'binary_name' : 'espruino_%v_nrf51822.bin',
 'build' : {
   'optimizeflags' : '-Os',
   'libraries' : [
     'BLUETOOTH',
     'GRAPHICS',
   ],
   'makefile' : [
     'SAVE_ON_FLASH=1',
     'DEFINES += -DUSE_DEBUGGER -DUSE_TAB_COMPLETE',
     'DEFINES += -DBOARD_PCA10028'
   ]
 }
};

chip = {
  'part' : "NRF51822",
  'family' : "NRF51",
  'package' : "QFN48",
  'ram' : 32,
  'flash' : 256,
  'speed' : 16,
  'usart' : 1,
  'spi' : 1,
  'i2c' : 1,
  'adc' : 0,
  'dac' : 0,
   # If using DFU bootloader, it sits at 0x3C000 - 0x40000 (0x40000 is end of flash)
   # Might want to change 256 -> 240 in the code below
  'saved_code' : {
    'address' : ((256 - 3) * 1024),
    'page_size' : 1024,
    'pages' : 3,
    'flash_available' : (256 - 108 - 3)
  }
};

devices = {
  'LED1' : { 'pin' : 'D21', 'inverted' : True },
  'LED2' : { 'pin' : 'D22', 'inverted' : True },
  'LED3' : { 'pin' : 'D23', 'inverted' : True },
  'LED4' : { 'pin' : 'D24', 'inverted' : True },
  'BTN1' : { 'pin' : 'D17', 'inverted' : True, 'pinstate' : 'IN_PULLUP'},
  'BTN2' : { 'pin' : 'D18', 'inverted' : True, 'pinstate' : 'IN_PULLUP'},
  'BTN3' : { 'pin' : 'D19', 'inverted' : True, 'pinstate' : 'IN_PULLUP'},
  'BTN4' : { 'pin' : 'D20', 'inverted' : True, 'pinstate' : 'IN_PULLUP'},
  'RX_PIN_NUMBER' : { 'pin' : 'D11'},
  'TX_PIN_NUMBER' : { 'pin' : 'D9'},
  'CTS_PIN_NUMBER' : { 'pin' : 'D10'},
  'RTS_PIN_NUMBER' : { 'pin' : 'D8'},
};

# left-right, or top-bottom order THIS IS INCORRECT!!!!!
board = {
  'left' : [ 'VDD', 'VDD', 'RESET', 'VDD','5V','GND','GND','PD3','PD4','PD28','PD29','PD30','PD31'],
  'right' : [ 'PD27', 'PD26', 'PD2', 'GND', 'PD25','PD24','PD23', 'PD22','PD20','PD19','PD18','PD17','PD16','PD15','PD14','PD13','PD12','PD11','PD10','PD9','PD8','PD7','PD6','PD5','PD21','PD1','PD0'],
};
board["_css"] = """
""";

def get_pins():
  pins = pinutils.generate_pins(0,31) # 32 General Purpose I/O Pins.
  pinutils.findpin(pins, "PD27", True)["functions"]["XL1"]=0;
  pinutils.findpin(pins, "PD26", True)["functions"]["XL2"]=0;
  pinutils.findpin(pins, "PD8", True)["functions"]["RTS"]=0;
  pinutils.findpin(pins, "PD9", True)["functions"]["TXD"]=0;
  pinutils.findpin(pins, "PD10", True)["functions"]["CTS"]=0;
  pinutils.findpin(pins, "PD11", True)["functions"]["RXD"]=0;
  pinutils.findpin(pins, "PD17", True)["functions"]["Button_1"]=0;
  pinutils.findpin(pins, "PD18", True)["functions"]["Button_2"]=0;
  pinutils.findpin(pins, "PD19", True)["functions"]["Button_3"]=0;
  pinutils.findpin(pins, "PD20", True)["functions"]["Button_4"]=0;
  pinutils.findpin(pins, "PD21", True)["functions"]["LED_1"]=0;
  pinutils.findpin(pins, "PD22", True)["functions"]["LED_2"]=0;
  pinutils.findpin(pins, "PD23", True)["functions"]["LED_3"]=0;
  pinutils.findpin(pins, "PD24", True)["functions"]["LED_4"]=0;

  pinutils.findpin(pins, "PD0", True)["functions"]["ADC1_IN1"]=0;
  pinutils.findpin(pins, "PD1", True)["functions"]["ADC1_IN2"]=0;
  pinutils.findpin(pins, "PD2", True)["functions"]["ADC1_IN3"]=0;
  pinutils.findpin(pins, "PD3", True)["functions"]["ADC1_IN4"]=0;
  pinutils.findpin(pins, "PD4", True)["functions"]["ADC1_IN5"]=0;
  pinutils.findpin(pins, "PD5", True)["functions"]["ADC1_IN6"]=0;
  pinutils.findpin(pins, "PD6", True)["functions"]["ADC1_IN7"]=0;
  # everything is non-5v tolerant
  for pin in pins:
    pin["functions"]["3.3"]=0;

  #The boot/reset button will function as a reset button in normal operation. Pin reset on PD21 needs to be enabled on the nRF52832 device for this to work.
  return pins
