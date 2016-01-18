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

}

def get_pins():
  pins = pinutils.scan_pin_file([], 'efm32ggstk.csv', 0, 1, 2)
  pins = pinutils.scan_pin_af_file(pins, 'efm32ggstk_af.csv', 0, 1)
  return pinutils.fill_gaps_in_pin_list(pins)
