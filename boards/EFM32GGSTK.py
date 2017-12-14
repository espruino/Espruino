#!/bin/false

import pinutils;
import json;
info = {
  'name': "EFM32 GG STK",
  'link': [ "https://www.silabs.com/products/mcu/lowpower/Pages/efm32gg-stk3700.aspx" ],
  'variables': 1720,
  'binary_name': 'espruino_%v_efm32ggstk.bin',
  'default_console' : "EV_SERIAL4",
  'default_console_tx' : "E0",
  'default_console_rx' : "E1",
  'default_console_baudrate' : "115200",
  'build' : {
   'optimizeflags' : '-Os',
   'libraries' : [
   ],
   'makefile' : [
     'DEFINES+= -DEFM32GG890F1024=1 # This should be EFM32GG990F1024, temporary hack to avoid the #USB on line 772 in jsinteractive.c'
   ]
  }
};
chip = {
  'part': "EFM32GG990F1024",
  'family': "EFM32GG",
  'package': "BGA112",
  'ram': 128,
  'flash': 1024,
  'speed': 48,
  'usart': 3,
  'spi': 3,
  'i2c': 2,
  'adc': 1,
  'dac': 1,
};

devices = {
  'BTN1' : { 'pin' : 'B9',  'pinstate' : 'IN', 'inverted' : 'true' },
  'BTN2' : { 'pin' : 'B10', 'pinstate' : 'IN' },
  'LED1' : { 'pin' : 'E2' },
  'LED2' : { 'pin' : 'E3' },
}

def get_pins():
  pins = pinutils.scan_pin_file([], 'efm32ggstk.csv', 1, 4, 5)
  return pinutils.fill_gaps_in_pin_list(pins)
