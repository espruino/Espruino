#!/bin/false
# This file is part of Espruino, a JavaScript interpreter for Microcontrollers
#
# Copyright (C) 2013 Gordon Williams <gw@pur3.co.uk>
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

import pinutils

info = {
  "name": "Raspberry Pi Pico (RP2040)",
  "link": [ "https://espruino.com/Pico" ],
  "espruino_page_link": "Pico",
  "boardname": "ESPR_RP2040_PICO",
  "default_console": "EV_USBSERIAL",
  "variables": 12000,
  "io_buffer_size": 4096,
  "binary_name": "espruino_%v_rp2040_pico.uf2",
  "build": {
    "optimizeflags": "-Og",
    "libraries": [
      "GRAPHICS"
    ],
    "makefile": [
      "DEFINES+=-DRP2040=1",
      "DEFINES+=-DUSE_FONT_6X8"
    ]
  }
}

chip = {
  "part": "RP2040",
  "family": "RP2040",
  "package": "QFN56",
  "ram": 264,
  "flash": 2048,
  "speed": 133,
  "usart": 2,
  "spi": 2,
  "i2c": 2,
  "adc": 1,
  "dac": 0,
  "saved_code": {
    "address": 0x081C0000,
    "page_size": 4096,
    "pages": 64,
    "flash_available": 1792,
  },
}

devices = {
  "LED1": { "pin": "D25" },
  "USB": {}
}


board = {
  "left": [
    "VBUS", "VSYS", "GND", "3V3_EN", "3V3", "ADC_VREF",
    "D28", "AGND", "D27", "D26", "RUN",
    "D22", "GND", "D21", "D20", "D19", "D18", "GND", "D17", "D16"
  ],
  "right": [
    "D0", "D1", "GND", "D2", "D3", "D4", "D5", "GND", "D6", "D7",
    "D8", "D9", "GND", "D10", "D11", "D12", "D13", "GND", "D14", "D15"
  ],
  "_notes": {
    "D25": "Onboard LED on Raspberry Pi Pico",
    "D26": "ADC0",
    "D27": "ADC1",
    "D28": "ADC2",
    "RUN": "Pull low to reset the RP2040",
    "D0": "Common debug UART TX during bring-up when early logging is enabled",
    "D1": "Common debug UART RX during bring-up when early logging is enabled"
  }
}


def add_pin_function(pins, pin_name, function_name, af=0):
  pinutils.findpin(pins, pin_name, True)["functions"][function_name] = af


def get_pins():
  # Pico exposes GPIO 0..29.
  pins = pinutils.generate_pins(0, 29)

  # ADC channels on GPIO 26..29.
  add_pin_function(pins, "PD26", "ADC1_IN0")
  add_pin_function(pins, "PD27", "ADC1_IN1")
  add_pin_function(pins, "PD28", "ADC1_IN2")
  add_pin_function(pins, "PD29", "ADC1_IN3")

  # The Pico's onboard LED is connected to GPIO25.
  add_pin_function(pins, "PD25", "LED1")

  # Common UART pin pairs exposed on Raspberry Pi Pico headers.
  add_pin_function(pins, "PD0", "USART1_TX")
  add_pin_function(pins, "PD1", "USART1_RX")
  add_pin_function(pins, "PD4", "USART2_TX")
  add_pin_function(pins, "PD5", "USART2_RX")
  add_pin_function(pins, "PD8", "USART2_TX")
  add_pin_function(pins, "PD9", "USART2_RX")
  add_pin_function(pins, "PD12", "USART1_TX")
  add_pin_function(pins, "PD13", "USART1_RX")
  add_pin_function(pins, "PD16", "USART1_TX")
  add_pin_function(pins, "PD17", "USART1_RX")
  add_pin_function(pins, "PD20", "USART2_TX")
  add_pin_function(pins, "PD21", "USART2_RX")
  add_pin_function(pins, "PD24", "USART2_TX")
  add_pin_function(pins, "PD25", "USART2_RX")
  add_pin_function(pins, "PD28", "USART1_TX")
  add_pin_function(pins, "PD29", "USART1_RX")

  # Common I2C pin pairs exposed on Raspberry Pi Pico headers.
  # Use AF0 for the preferred/default pairs so bare I2C1.setup()/I2C2.setup()
  # resolve to the bench harness without removing alternate RP2040-capable pins.
  add_pin_function(pins, "PD0", "I2C1_SDA", 1)
  add_pin_function(pins, "PD1", "I2C1_SCL", 1)
  add_pin_function(pins, "PD2", "I2C2_SDA", 1)
  add_pin_function(pins, "PD3", "I2C2_SCL", 1)
  add_pin_function(pins, "PD4", "I2C1_SDA", 1)
  add_pin_function(pins, "PD5", "I2C1_SCL", 1)
  add_pin_function(pins, "PD6", "I2C2_SDA", 1)
  add_pin_function(pins, "PD7", "I2C2_SCL", 1)
  add_pin_function(pins, "PD8", "I2C1_SDA", 0)
  add_pin_function(pins, "PD9", "I2C1_SCL", 0)
  add_pin_function(pins, "PD10", "I2C2_SDA", 0)
  add_pin_function(pins, "PD11", "I2C2_SCL", 0)
  add_pin_function(pins, "PD12", "I2C1_SDA", 1)
  add_pin_function(pins, "PD13", "I2C1_SCL", 1)
  add_pin_function(pins, "PD14", "I2C2_SDA", 1)
  add_pin_function(pins, "PD15", "I2C2_SCL", 1)
  add_pin_function(pins, "PD16", "I2C1_SDA", 1)
  add_pin_function(pins, "PD17", "I2C1_SCL", 1)
  add_pin_function(pins, "PD18", "I2C2_SDA", 1)
  add_pin_function(pins, "PD19", "I2C2_SCL", 1)
  add_pin_function(pins, "PD20", "I2C1_SDA", 1)
  add_pin_function(pins, "PD21", "I2C1_SCL", 1)
  add_pin_function(pins, "PD22", "I2C2_SDA", 1)
  add_pin_function(pins, "PD23", "I2C2_SCL", 1)
  add_pin_function(pins, "PD24", "I2C1_SDA", 1)
  add_pin_function(pins, "PD25", "I2C1_SCL", 1)
  add_pin_function(pins, "PD26", "I2C2_SDA", 1)
  add_pin_function(pins, "PD27", "I2C2_SCL", 1)
  add_pin_function(pins, "PD28", "I2C1_SDA", 1)
  add_pin_function(pins, "PD29", "I2C1_SCL", 1)

  # Common SPI pin groups exposed on Raspberry Pi Pico headers.
  # Use AF0 for the preferred/default SPI1 harness pins so bare SPI1.setup()
  # resolves to D16/D18/D19 without removing alternate RP2040-capable pins.
  add_pin_function(pins, "PD0", "SPI1_MISO", 1)
  add_pin_function(pins, "PD2", "SPI1_SCK", 1)
  add_pin_function(pins, "PD3", "SPI1_MOSI", 1)
  add_pin_function(pins, "PD4", "SPI1_MISO", 1)
  add_pin_function(pins, "PD6", "SPI1_SCK", 1)
  add_pin_function(pins, "PD7", "SPI1_MOSI", 1)
  add_pin_function(pins, "PD8", "SPI2_MISO")
  add_pin_function(pins, "PD10", "SPI2_SCK")
  add_pin_function(pins, "PD11", "SPI2_MOSI")
  add_pin_function(pins, "PD12", "SPI2_MISO")
  add_pin_function(pins, "PD14", "SPI2_SCK")
  add_pin_function(pins, "PD15", "SPI2_MOSI")
  add_pin_function(pins, "PD16", "SPI1_MISO", 0)
  add_pin_function(pins, "PD18", "SPI1_SCK", 0)
  add_pin_function(pins, "PD19", "SPI1_MOSI", 0)
  add_pin_function(pins, "PD20", "SPI1_MISO", 1)
  add_pin_function(pins, "PD22", "SPI1_SCK", 1)
  add_pin_function(pins, "PD23", "SPI1_MOSI", 1)
  add_pin_function(pins, "PD24", "SPI2_MISO")
  add_pin_function(pins, "PD26", "SPI2_SCK")
  add_pin_function(pins, "PD27", "SPI2_MOSI")
  add_pin_function(pins, "PD28", "SPI2_MISO")

  for pin in pins:
    pin["functions"]["3.3"] = 0

  return pins
