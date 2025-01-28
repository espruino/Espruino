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

# To build and flash run:
"""
source ./scripts/provision.sh XIAOBLE
make clean && BOARD=XIAOBLE RELEASE=1 make
"""
# Then connect the board to your computer and press the reset button twice.
# You should now see a USB storage device called "XIAO-SENSE" (no matter which version you have).
# Copy the "espruino_*_xiaoble.uf2" file from the "bin" folder to that drive.
# The board should automatically disconnect after copying is finished, and reboot into Espruino,
# which turns on the red led for a short time after starting up.

# If you accidentally put some stuff in .boot0 that prevents you from interacting with Espruino:
# Pull pin D1 to 3.3 V / high (important: make sure you use 3.3 V; higher voltages may cause damage) and reset the board,
# by either pressing the reset button or cutting the power and powering it up again.
# This behaviour is archived by configuring pin D1 as BTN1.

import pinutils
info = {
    "name": "Seeed Xiao BLE",
    "link": ["https://www.seeedstudio.com/Seeed-XIAO-BLE-nRF52840-p-5201.html"],
    "default_console": "EV_USBSERIAL",
    "variables": 14000,  # How many variables are allocated for Espruino to use. RAM will be overflowed if this number is too high and code won't compile.
    "binary_name": "espruino_%v_xiaoble.uf2",
    "build": {
        "optimizeflags": "-Os",
        "libraries": [
            "BLUETOOTH",
            # "GRAPHICS",
            "CRYPTO","SHA256","SHA512",
            "AES_CCM",
            # "NFC",
            # "TENSORFLOW",
            "NEOPIXEL",
            "JIT",
        ],
        "makefile": [
            "DEFINES += -DESPR_LSE_ENABLE",  # Ensure low speed external osc enabled
            "DEFINES += -DCONFIG_GPIO_AS_PINRESET",  # Allow the reset pin to work
            "DEFINES += -DNRF_USB=1 -DUSB",
            "DEFINES += -DNEOPIXEL_SCK_PIN=33 -DNEOPIXEL_LRCK_PIN=34",  # nRF52840 needs LRCK pin defined for neopixel
            "DEFINES += -DBLUETOOTH_NAME_PREFIX='\"XIAOBLE\"'",
            "DEFINES += -DSPIFLASH_READ2X",  # Read SPI flash at 2x speed using MISO and MOSI for IO
            "DEFINES += -DESPR_UNICODE_SUPPORT=1",
            "DEFINES += -DNRF_SDH_BLE_GATT_MAX_MTU_SIZE=131",  # 23+x*27 rule as per https://devzone.nordicsemi.com/f/nordic-q-a/44825/ios-mtu-size-why-only-185-bytes
            # 'DEFINES += -DPIN_NAMES_DIRECT=1', # Package skips out some pins, so we can't assume each port starts from 0
            "LDFLAGS += -Xlinker --defsym=LD_APP_RAM_BASE=0x2ec0",  # set RAM base to match MTU
            'DEFINES += -DESPR_BLE_PRIVATE_ADDRESS_SUPPORT',
            'DEFINES += -DESPR_HAS_BOOTLOADER_UF2',
            'NRF_SDK15=1',
        ],
    },
}

chip = {
    "part": "NRF52840",
    "family": "NRF52",
    "package": "AQFN73",
    "ram": 256,
    "flash": 1024,
    "speed": 64,
    "usart": 1,
    "spi": 1,
    "i2c": 2,
    "adc": 1,
    "dac": 0,
    "saved_code": {
        "address": 0x60000000, # put this in external spiflash (see below)
        "page_size": 4096,
        "pages": 512, # Entire 2 MB of external flash
        "flash_available": 1024 - ((38 + 7 + 12 + 1) * 4) - 780,
        # Softdevice 140 uses 38 pages of flash, user data 7, bootloader 12, plus 1 page of padding (because I'm paranoid and really don't want to overwrite the bootloader).
        # Each page is 4 KB.
    },
}

devices = {
    "LED1": {"pin": "D11"},
    "LED2": {"pin": "D13"},
    "LED3": {"pin": "D12"},
    "BTN1": {"pin": "D1", "pinstate" : "IN_PULLDOWN"},
    "BAT": {
        "pin_charging": "D23",
        "pin_voltage": "D32",
    },
    "NFC": {"pin_a": "D30", "pin_b": "D31"},
    # accelerometer is only present on the "sense" variant (Seeed XIAO BLE nRF52840 Sense)
    #"ACCEL" : {
    #    # device is actually a LSM6DS3TR-C, not sure if this would even work
    #    "device" : "LSM6DSL", "addr" : 0x6b,
    #    "pin_sda" : "D17",
    #    "pin_scl" : "D16"
    #},
    "SPIFLASH": {
        "pin_cs": "D25",
        "pin_sck": "D24",
        "pin_mosi": "D26",
        "pin_miso": "D27",
        "pin_wp": "D28",
        "pin_rst": "D29",
        "size": 4096 * 512,  # 2 MB
        "memmap_base": 0x60000000,  # map into the address space (in software)
    },
}

# left-right, or top-bottom order
board = {}

# schematic at https://files.seeedstudio.com/wiki/XIAO-BLE/Seeed-Studio-XIAO-nRF52840-Sense-v1.1.pdf
# pinout sheet https://files.seeedstudio.com/wiki/XIAO-BLE/XIAO-nRF52840-pinout_sheet.xlsx
# see also https://github.com/Seeed-Studio/Adafruit_nRF52_Arduino/blob/master/variants/Seeed_XIAO_nRF52840/variant.h
# and https://github.com/Seeed-Studio/Adafruit_nRF52_Arduino/blob/master/variants/Seeed_XIAO_nRF52840/variant.cpp
def get_pins():
    pins = [
      { "name": "PD0", "sortingname": "D00", "port": "D", "num": "2", "functions": { "ADC1_IN0": 0 } },
      { "name": "PD1", "sortingname": "D01", "port": "D", "num": "3", "functions": { "ADC1_IN1": 0 } },
      { "name": "PD2", "sortingname": "D02", "port": "D", "num": "28", "functions": { "ADC1_IN4": 0 } },
      { "name": "PD3", "sortingname": "D03", "port": "D", "num": "29", "functions": { "ADC1_IN5": 0 } },
      { "name": "PD4", "sortingname": "D04", "port": "D", "num": "4", "functions": { "ADC1_IN2": 0 } },
      { "name": "PD5", "sortingname": "D05", "port": "D", "num": "5", "functions": { "ADC1_IN3": 0 } },
      { "name": "PD6", "sortingname": "D06", "port": "D", "num": "43", "functions": {} },
      { "name": "PD7", "sortingname": "D07", "port": "D", "num": "44", "functions": {} },
      { "name": "PD8", "sortingname": "D08", "port": "D", "num": "45", "functions": {} },
      { "name": "PD9", "sortingname": "D09", "port": "D", "num": "46", "functions": {} },
      { "name": "PD10", "sortingname": "D10", "port": "D", "num": "47", "functions": {} },
      { "name": "PD11", "sortingname": "D11", "port": "D", "num": "26", "functions": { "NEGATED": 0, "LED_RED": 0 } },
      { "name": "PD12", "sortingname": "D12", "port": "D", "num": "6", "functions": { "NEGATED": 0, "LED_BLUE": 0 } },
      { "name": "PD13", "sortingname": "D13", "port": "D", "num": "30", "functions": { "ADC1_IN6": 0, "NEGATED": 0, "LED_GREEN": 0 } },
      { "name": "PD14", "sortingname": "D14", "port": "D", "num": "14", "functions": { "NEGATED": 0, "VBAT_ENABLE": 0 } },
      { "name": "PD15", "sortingname": "D15", "port": "D", "num": "40", "functions": { "6D_PWR": 0 } },
      { "name": "PD16", "sortingname": "D16", "port": "D", "num": "27", "functions": { "6D_I2C_SCL": 0 } },
      { "name": "PD17", "sortingname": "D17", "port": "D", "num": "7", "functions": { "6D_I2C_SDA": 0 } },
      { "name": "PD18", "sortingname": "D18", "port": "D", "num": "11", "functions": { "6D_INT1": 0 } },
      { "name": "PD19", "sortingname": "D19", "port": "D", "num": "42", "functions": { "MIC_PWR": 0 } },
      { "name": "PD20", "sortingname": "D20", "port": "D", "num": "32", "functions": { "PDM_CLK": 0 } },
      { "name": "PD21", "sortingname": "D21", "port": "D", "num": "16", "functions": { "PDM_DATA": 0 } },
      { "name": "PD22", "sortingname": "D22", "port": "D", "num": "13", "functions": { "HICHG": 0 } },
      { "name": "PD23", "sortingname": "D23", "port": "D", "num": "17", "functions": { "NEGATED": 0, "CHG": 0 } },
      { "name": "PD24", "sortingname": "D24", "port": "D", "num": "21", "functions": { "QSPI_SCK": 0 } },
      { "name": "PD25", "sortingname": "D25", "port": "D", "num": "25", "functions": { "QSPI_CSN": 0 } },
      { "name": "PD26", "sortingname": "D26", "port": "D", "num": "20", "functions": { "QSPI_DI": 0 } },
      { "name": "PD27", "sortingname": "D27", "port": "D", "num": "24", "functions": { "QSPI_DO": 0 } },
      { "name": "PD28", "sortingname": "D28", "port": "D", "num": "22", "functions": { "QSPI_WP": 0 } },
      { "name": "PD29", "sortingname": "D29", "port": "D", "num": "23", "functions": { "QSPI_HOLD": 0 } },
      { "name": "PD30", "sortingname": "D30", "port": "D", "num": "9", "functions": { "NFC1": 0 } },
      { "name": "PD31", "sortingname": "D31", "port": "D", "num": "10", "functions": { "NFC2": 0 } },
      { "name": "PD32", "sortingname": "D32", "port": "D", "num": "31", "functions": { "ADC1_IN7": 0, "VBAT": 0 } },
    ]
    # D23 CHG
    # get charge state via digitalRead(D23)
    # HIGH: charging, LOW: not charging
    # D22 HICHG / PIN_CHARGING_CURRENT
    # battery charging current see https://wiki.seeedstudio.com/XIAO_BLE/#battery-charging-current
    # HIGH: 100mA, LOW: 50mA
    # D14 VBAT_ENABLE
    # enable battery voltage reading by digitalWrite(D14, true)
    # D32 VBAT
    # battery voltage; remember to enable voltage reading if you want to use this

    # everything is non-5v tolerant
    for pin in pins:
        pin["functions"]["3.3"] = 0
    return pins
