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
 'name' : "Jolt.js",
 'link' :  [ "https://www.espruino.com/Jolt.js" ],
 'espruino_page_link' : 'Jolt.js',
# 'default_console' : "EV_SERIAL1",
# 'default_console_tx' : "D6",
# 'default_console_rx' : "D8",
# 'default_console_baudrate' : "9600",
 'variables' : 12000, # How many variables are allocated for Espruino to use. RAM will be overflowed if this number is too high and code won't compile.
 'bootloader' : 1,
 'binary_name' : 'espruino_%v_joltjs.hex',
 'build' : {
   'optimizeflags' : '-Os',
   'libraries' : [
     'BLUETOOTH',
#     'NET',
     'GRAPHICS',
#     'NFC',
     'NEOPIXEL',
     'JIT' # JIT compiler enabled
   ],
   'makefile' : [
#     'DEFINES += -DCONFIG_GPIO_AS_PINRESET', # Allow the reset pin to work
     'DEFINES += -DNRF_USB=1 -DUSB',
     'DEFINES += -DNEOPIXEL_SCK_PIN=1 -DNEOPIXEL_LRCK_PIN=26', # nRF52840 needs LRCK pin defined for neopixel
     'DEFINES += -DNRF_SDH_BLE_GATT_MAX_MTU_SIZE=131', # 23+x*27 rule as per https://devzone.nordicsemi.com/f/nordic-q-a/44825/ios-mtu-size-why-only-185-bytes
     'DEFINES += -DCENTRAL_LINK_COUNT=2 -DNRF_SDH_BLE_CENTRAL_LINK_COUNT=2', # allow two outgoing connections at once
     'LDFLAGS += -Xlinker --defsym=LD_APP_RAM_BASE=0x3660', # set RAM base to match MTU=131 + CENTRAL_LINK_COUNT=2
     'DEFINES += -DAPP_TIMER_OP_QUEUE_SIZE=6', 
     'DEFINES+=-DBLUETOOTH_NAME_PREFIX=\'"Jolt.js"\'',     
     'DFU_SETTINGS=--application-version 0xff --hw-version 52 --sd-req 0xa9,0xae,0xb6',
     'DFU_PRIVATE_KEY=targets/nrf5x_dfu/dfu_private_key.pem',
     'DEFINES += -DNRF_BOOTLOADER_NO_WRITE_PROTECT=1', # By default the bootloader protects flash. Avoid this (a patch for NRF_BOOTLOADER_NO_WRITE_PROTECT must be applied first)
     #'DEFINES += -DBUTTONPRESS_TO_REBOOT_BOOTLOADER', # not enabled so watchdog isn't started
     'BOOTLOADER_SETTINGS_FAMILY=NRF52840',
     'INCLUDE += -I$(ROOT)/libs/jolt.js',
     'WRAPPERSOURCES += libs/joltjs/jswrap_jolt.c libs/joltjs/jswrap_qwiic.c',
     'NRF_SDK15=1',
   ]
 }
};


chip = {
  'part' : "NRF52840",
  'family' : "NRF52",
  'package' : "AQFN73",
  'ram' : 256,
  'flash' : 1024,
  'speed' : 64,
  'usart' : 2,
  'spi' : 3,
  'i2c' : 2,
  'adc' : 1,
  'dac' : 0,
  'saved_code' : {
    'address' : ((246 - 10) * 4096), # Bootloader takes pages 248-255, FS takes 246-247
    'page_size' : 4096,
    'pages' : 10,
    'flash_available' : 1024 - ((31 + 8 + 2 + 10)*4) # Softdevice uses 31 pages of flash, bootloader 8, FS 2, code 10. Each page is 4 kb.
  },
};

devices = {
  'BTN1' : { 'pin' : 'D0', 'pinstate' : 'IN_PULLDOWN' },
  'LED1' : { 'pin' : 'D6' }, # Pin negated in software
  'LED2' : { 'pin' : 'D8' }, # Pin negated in software
  'LED3' : { 'pin' : 'D41' }, # Pin negated in software
  # See Espruino/libs/joltjs/jswrap_jolt.c for other pins
  'QWIIC1' : {
    'pin_sda' : 'D3',
    'pin_scl' : 'D29',
    'pin_fet' : 'D7',
  },
  'QWIIC2' : {
    'pin_sda' : 'D2',
    'pin_scl' : 'D31',
    'pin_fet' : 'D27',    
  },
  'QWIIC3' : {
    'pin_sda' : 'D44', # P1.12
    'pin_scl' : 'D45', # P1.13
    'pin_gnd' : 'D36', # P1.04
    'pin_vcc' : 'D43', # P1.11    
  },
  'QWIIC4' : {
    'pin_sda' : 'D39', # P1.07
    'pin_scl' : 'D38', # P1.06
    'pin_gnd' : 'D37', # P1.05
    'pin_vcc' : 'D42', # P1.10
  },

};

# left-right, or top-bottom order
board = {
  '_hide_not_on_connectors' : True,    
  '_notes' : {
    'V0' : "Motor driver 0, output 0. This pin is also connected to an analog input via a 39k/220k potential divider",
    'V1' : "Motor driver 0, output 1. This pin is also connected to an analog input via a 39k/220k potential divider",
    'V2' : "Motor driver 0, output 2. This pin is also connected to an analog input via a 39k/220k potential divider",
    'V3' : "Motor driver 0, output 3. This pin is also connected to an analog input via a 39k/220k potential divider",
    'V4' : "Motor driver 1, output 0. This pin is also connected to an analog input via a 39k/220k potential divider",
    'V5' : "Motor driver 1, output 1. This pin is also connected to an analog input via a 39k/220k potential divider",
    'V6' : "Motor driver 1, output 2. This pin is also connected to an analog input via a 39k/220k potential divider",
    'V7' : "Motor driver 1, output 3. This pin is also connected to an analog input via a 39k/220k potential divider",
  },
};
board["_css"] = """
#board {
  width: 528px;
  height: 800px;
  top: 0px;
  left : 200px;
  background-image: url(img/JOLTJS.jpg);
}
#boardcontainer {
  height: 900px;
}

#left {
    top: 219px;
    right: 466px;
}
#right {
    top: 150px;
    left: 466px;
}

.leftpin { height: 17px; }
.rightpin { height: 17px; }
""";

def get_pins():
  pins = pinutils.generate_pins(0,47,"D") + pinutils.generate_pins(0,7,"V"); # 48 General Purpose I/O Pins, 8 virtual pins for IO
  pinutils.findpin(pins, "PD0", True)["functions"]["XL1"]=0;
  pinutils.findpin(pins, "PD1", True)["functions"]["XL2"]=0;
  pinutils.findpin(pins, "PD5", True)["functions"]["RTS"]=0;
  pinutils.findpin(pins, "PD6", True)["functions"]["TXD"]=0;
  pinutils.findpin(pins, "PD7", True)["functions"]["CTS"]=0;
  pinutils.findpin(pins, "PD8", True)["functions"]["RXD"]=0;
  pinutils.findpin(pins, "PD9", True)["functions"]["NFC1"]=0;
  pinutils.findpin(pins, "PD10", True)["functions"]["NFC2"]=0;
  pinutils.findpin(pins, "PD2", True)["functions"]["ADC1_IN0"]=0;
  pinutils.findpin(pins, "PD3", True)["functions"]["ADC1_IN1"]=0;
  pinutils.findpin(pins, "PD4", True)["functions"]["ADC1_IN2"]=0;
  pinutils.findpin(pins, "PD5", True)["functions"]["ADC1_IN3"]=0;
  pinutils.findpin(pins, "PD28", True)["functions"]["ADC1_IN4"]=0;
  pinutils.findpin(pins, "PD29", True)["functions"]["ADC1_IN5"]=0;
  pinutils.findpin(pins, "PD30", True)["functions"]["ADC1_IN6"]=0;
  pinutils.findpin(pins, "PD31", True)["functions"]["ADC1_IN7"]=0;
  # Make buttons and LEDs negated
  pinutils.findpin(pins, "PD6", True)["functions"]["NEGATED"]=0;
  pinutils.findpin(pins, "PD8", True)["functions"]["NEGATED"]=0;
  pinutils.findpin(pins, "PD41", True)["functions"]["NEGATED"]=0;

  # everything is non-5v tolerant
  for pin in pins:
    pin["functions"]["3.3"]=0;
  #The boot/reset button will function as a reset button in normal operation. Pin reset on PD21 needs to be enabled on the nRF52832 device for this to work.
  return pins
