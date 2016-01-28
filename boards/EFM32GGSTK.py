#!/bin/false

import pinutils;
import json;
info = {
  'name': "EFM32 GG STK",
  'link': [ "https://www.silabs.com/products/mcu/lowpower/Pages/efm32gg-stk3700.aspx" ],
  'variables': 1720,
  'binary_name': 'espruino_%v_efm32ggstk.bin',
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
  'BTN1' : { 'pin' : 'B9',  'pinstate' : 'IN' },
  'BTN2' : { 'pin' : 'B10', 'pinstate' : 'IN' },
  'LED1' : { 'pin' : 'E2' }, 
  'LED2' : { 'pin' : 'E3' },
}

def get_pins():
  pins = pinutils.scan_pin_file([], 'efm32ggstk.csv', 1, 4, 5)
  return pinutils.fill_gaps_in_pin_list(pins)
