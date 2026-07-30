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
    #  'FILESYSTEM',
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
  'part'    : "RP2350",
  'family'  : "RP2XXX",
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
  "USB": {}
};

def get_pins():
  # RP2350A (as on Pico 2) has GPIO 0-29
  pins = pinutils.generate_pins(0, 29)

  # GPIO->function crossbar. Applied in peripheral order so pinInfo output
  # (functions sorted by usage, ties by insertion) stays stable.
  def af(func, gpios):
    for p in gpios:
      pinutils.findpin(pins, "PD"+str(p), True)["functions"][func]=0;

  # UART: hw uart0/1 -> USART1/2
  af("USART1_TX", [0,12,16]);   af("USART1_RX", [1,13,17])
  af("USART2_TX", [4,8,20,24]); af("USART2_RX", [5,9,21,25])

  # SPI: hw spi0/1 -> SPI1/2
  af("SPI1_SCK", [2,18,6]); af("SPI1_MOSI", [3,19,7]); af("SPI1_MISO", [0,16,4])
  af("SPI2_SCK", [10,14]);  af("SPI2_MOSI", [11,15]);  af("SPI2_MISO", [8,12])

  # I2C: hw i2c0/1 -> I2C1/2
  af("I2C1_SDA", [0,4,8,12,16,20,24]); af("I2C1_SCL", [1,5,9,13,17,21,25])
  af("I2C2_SDA", [2,6,10,14,18,22]);   af("I2C2_SCL", [3,7,11,15,19,23])

  # ADC: GP26-29 -> ch0-3
  af("ADC1_IN0", [26]); af("ADC1_IN1", [27]); af("ADC1_IN2", [28]); af("ADC1_IN3", [29])

  # PWM: SDK slice=(gpio>>1)&7, channel=gpio&1 -> TIM(slice+1)_CH(channel+1)
  for g in range(30):
    pinutils.findpin(pins, "PD"+str(g), True)["functions"]["TIM"+str(((g>>1)&7)+1)+"_CH"+str((g&1)+1)]=0;

  pinutils.findpin(pins, "PD25", True)["functions"]["LED1"]=0;

  # All pins are 3.3V (NOT 5V tolerant)
  for pin in pins:
    pin["functions"]["3.3"]=0;

  return pins
