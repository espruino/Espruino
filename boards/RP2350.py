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
#
# Raspberry Pi Pico 2 (RP2350)
# ----------------------------------------------------------------------------------------

import pinutils;

info = {
 'name' : "Raspberry Pi Pico 2",
 'link' : [ "https://www.raspberrypi.com/products/raspberry-pi-pico-2/" ],
 'default_console'          : "EV_SERIAL1",
 'default_console_tx'       : "D0",
 'default_console_rx'       : "D1",
 'default_console_baudrate' : "115200",
 'variables'                : 16000,  # ~220KB of 512KB SRAM (14B/JsVar under the 16383 threshold), ~285KB left for heap
 'binary_name'              : 'espruino_%v_rp2350.uf2',
 'build' : {
   'optimizeflags' : '-Os',
   'libraries' : [
     'GRAPHICS',
     'FILESYSTEM',
     'JIT',
   ],
   'makefile' : [
     'DEFINES+=-DPIN_NAMES_DIRECT=1',  # Pins use direct GPIO numbering
     # ESPR_PACKED_SYMPTR can't be used - code runs from XIP flash at 0x10000000, beyond its 20 bit pointer range
     'PICO_BOARD?=pico2', # Pico SDK board header (boards/pico2.h)
   ]
 }
};

chip = {
  'part'    : "RP2350A",
  'family'  : "RP2350",
  'package' : "QFN60",
  'ram'   : 520,
  'flash' : 4096,  # 4MB external QSPI flash, memory-mapped (XIP) at 0x10000000
  'speed' : 150,
  'usart' : 2,
  'spi'   : 2,
  'i2c'   : 2,
  'adc'   : 1,
  'dac'   : 0,
  'saved_code' : {
    'address' : 0x10100000,  # After the 1MB firmware region (see linker FLASH LENGTH)
    'page_size' : 4096,
    'pages' : 768,           # 3MB Storage: fills 0x10100000..0x10400000 (end of the 4MB flash)
    'flash_available' : 4096 - 1024  # 4MB flash - 1MB firmware region
  },
};

devices = {
  'LED1' : { 'pin' : 'D25' },  # Onboard LED on GP25
};

def get_pins():
  # RP2350A (as on Pico 2) has GPIO 0-29
  pins = pinutils.generate_pins(0, 29)

  # UART0 pins
  pinutils.findpin(pins, "PD0", True)["functions"]["USART1_TX"]=0;
  pinutils.findpin(pins, "PD1", True)["functions"]["USART1_RX"]=0;
  pinutils.findpin(pins, "PD12", True)["functions"]["USART1_TX"]=0;
  pinutils.findpin(pins, "PD13", True)["functions"]["USART1_RX"]=0;
  pinutils.findpin(pins, "PD16", True)["functions"]["USART1_TX"]=0;
  pinutils.findpin(pins, "PD17", True)["functions"]["USART1_RX"]=0;

  # UART1 pins
  pinutils.findpin(pins, "PD4", True)["functions"]["USART2_TX"]=0;
  pinutils.findpin(pins, "PD5", True)["functions"]["USART2_RX"]=0;
  pinutils.findpin(pins, "PD8", True)["functions"]["USART2_TX"]=0;
  pinutils.findpin(pins, "PD9", True)["functions"]["USART2_RX"]=0;
  pinutils.findpin(pins, "PD20", True)["functions"]["USART2_TX"]=0;
  pinutils.findpin(pins, "PD21", True)["functions"]["USART2_RX"]=0;
  pinutils.findpin(pins, "PD24", True)["functions"]["USART2_TX"]=0;
  pinutils.findpin(pins, "PD25", True)["functions"]["USART2_RX"]=0;

  # SPI0 pins
  pinutils.findpin(pins, "PD2", True)["functions"]["SPI1_SCK"]=0;
  pinutils.findpin(pins, "PD3", True)["functions"]["SPI1_MOSI"]=0;
  pinutils.findpin(pins, "PD0", True)["functions"]["SPI1_MISO"]=0;
  pinutils.findpin(pins, "PD18", True)["functions"]["SPI1_SCK"]=0;
  pinutils.findpin(pins, "PD19", True)["functions"]["SPI1_MOSI"]=0;
  pinutils.findpin(pins, "PD16", True)["functions"]["SPI1_MISO"]=0;
  pinutils.findpin(pins, "PD6", True)["functions"]["SPI1_SCK"]=0;
  pinutils.findpin(pins, "PD7", True)["functions"]["SPI1_MOSI"]=0;
  pinutils.findpin(pins, "PD4", True)["functions"]["SPI1_MISO"]=0;

  # SPI1 pins
  pinutils.findpin(pins, "PD10", True)["functions"]["SPI2_SCK"]=0;
  pinutils.findpin(pins, "PD11", True)["functions"]["SPI2_MOSI"]=0;
  pinutils.findpin(pins, "PD8", True)["functions"]["SPI2_MISO"]=0;
  pinutils.findpin(pins, "PD14", True)["functions"]["SPI2_SCK"]=0;
  pinutils.findpin(pins, "PD15", True)["functions"]["SPI2_MOSI"]=0;
  pinutils.findpin(pins, "PD12", True)["functions"]["SPI2_MISO"]=0;

  # I2C0 pins
  pinutils.findpin(pins, "PD0", True)["functions"]["I2C1_SDA"]=0;
  pinutils.findpin(pins, "PD1", True)["functions"]["I2C1_SCL"]=0;
  pinutils.findpin(pins, "PD4", True)["functions"]["I2C1_SDA"]=0;
  pinutils.findpin(pins, "PD5", True)["functions"]["I2C1_SCL"]=0;
  pinutils.findpin(pins, "PD8", True)["functions"]["I2C1_SDA"]=0;
  pinutils.findpin(pins, "PD9", True)["functions"]["I2C1_SCL"]=0;
  pinutils.findpin(pins, "PD12", True)["functions"]["I2C1_SDA"]=0;
  pinutils.findpin(pins, "PD13", True)["functions"]["I2C1_SCL"]=0;
  pinutils.findpin(pins, "PD16", True)["functions"]["I2C1_SDA"]=0;
  pinutils.findpin(pins, "PD17", True)["functions"]["I2C1_SCL"]=0;
  pinutils.findpin(pins, "PD20", True)["functions"]["I2C1_SDA"]=0;
  pinutils.findpin(pins, "PD21", True)["functions"]["I2C1_SCL"]=0;
  pinutils.findpin(pins, "PD24", True)["functions"]["I2C1_SDA"]=0;
  pinutils.findpin(pins, "PD25", True)["functions"]["I2C1_SCL"]=0;

  # I2C1 pins
  pinutils.findpin(pins, "PD2", True)["functions"]["I2C2_SDA"]=0;
  pinutils.findpin(pins, "PD3", True)["functions"]["I2C2_SCL"]=0;
  pinutils.findpin(pins, "PD6", True)["functions"]["I2C2_SDA"]=0;
  pinutils.findpin(pins, "PD7", True)["functions"]["I2C2_SCL"]=0;
  pinutils.findpin(pins, "PD10", True)["functions"]["I2C2_SDA"]=0;
  pinutils.findpin(pins, "PD11", True)["functions"]["I2C2_SCL"]=0;
  pinutils.findpin(pins, "PD14", True)["functions"]["I2C2_SDA"]=0;
  pinutils.findpin(pins, "PD15", True)["functions"]["I2C2_SCL"]=0;
  pinutils.findpin(pins, "PD18", True)["functions"]["I2C2_SDA"]=0;
  pinutils.findpin(pins, "PD19", True)["functions"]["I2C2_SCL"]=0;
  pinutils.findpin(pins, "PD22", True)["functions"]["I2C2_SDA"]=0;
  pinutils.findpin(pins, "PD23", True)["functions"]["I2C2_SCL"]=0;

  # ADC channels
  pinutils.findpin(pins, "PD26", True)["functions"]["ADC1_IN0"]=0;
  pinutils.findpin(pins, "PD27", True)["functions"]["ADC1_IN1"]=0;
  pinutils.findpin(pins, "PD28", True)["functions"]["ADC1_IN2"]=0;
  pinutils.findpin(pins, "PD29", True)["functions"]["ADC1_IN3"]=0;

  # PWM channels (TIM1 maps to PWM slice 0, TIM2 to slice 1, etc.)
  # PWM slice 0: GP0(A), GP1(B), GP8(A), GP9(B), GP16(A), GP17(B), GP24(A), GP25(B)
  for p in [0,8,16,24]:
    pinutils.findpin(pins, "PD"+str(p), True)["functions"]["TIM1_CH1"]=0;
  for p in [1,9,17,25]:
    pinutils.findpin(pins, "PD"+str(p), True)["functions"]["TIM1_CH2"]=0;
  # PWM slice 1: GP2(A), GP3(B), GP10(A), GP11(B), GP18(A), GP19(B), GP26(A), GP27(B)
  for p in [2,10,18,26]:
    pinutils.findpin(pins, "PD"+str(p), True)["functions"]["TIM2_CH1"]=0;
  for p in [3,11,19,27]:
    pinutils.findpin(pins, "PD"+str(p), True)["functions"]["TIM2_CH2"]=0;
  # PWM slice 2: GP4(A), GP5(B), GP12(A), GP13(B), GP20(A), GP21(B), GP28(A)
  for p in [4,12,20,28]:
    pinutils.findpin(pins, "PD"+str(p), True)["functions"]["TIM3_CH1"]=0;
  for p in [5,13,21]:
    pinutils.findpin(pins, "PD"+str(p), True)["functions"]["TIM3_CH2"]=0;
  # PWM slice 3: GP6(A), GP7(B), GP14(A), GP15(B), GP22(A), GP23(B)
  for p in [6,14,22]:
    pinutils.findpin(pins, "PD"+str(p), True)["functions"]["TIM4_CH1"]=0;
  for p in [7,15,23]:
    pinutils.findpin(pins, "PD"+str(p), True)["functions"]["TIM4_CH2"]=0;

  pinutils.findpin(pins, "PD25", True)["functions"]["LED1"]=0;

  # All pins are 3.3V (NOT 5V tolerant)
  for pin in pins:
    pin["functions"]["3.3"]=0;

  return pins
