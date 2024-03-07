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
     'DEFINES += -DESPR_USE_STEPPER_TIMER=1', # Build in the code for stepping using the timer
     'DEFINES += -DNRF_SDH_BLE_GATT_MAX_MTU_SIZE=131', # 23+x*27 rule as per https://devzone.nordicsemi.com/f/nordic-q-a/44825/ios-mtu-size-why-only-185-bytes
     'DEFINES += -DCENTRAL_LINK_COUNT=2 -DNRF_SDH_BLE_CENTRAL_LINK_COUNT=2', # allow two outgoing connections at once
     'LDFLAGS += -Xlinker --defsym=LD_APP_RAM_BASE=0x3660', # set RAM base to match MTU=131 + CENTRAL_LINK_COUNT=2
     'DEFINES += -DAPP_TIMER_OP_QUEUE_SIZE=6',
     'DEFINES+= -DBLUETOOTH_NAME_PREFIX=\'"Jolt.js"\'',
#    see targets/nrf5x/app_config.h for USB descriptors
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
  'QWIIC0' : {
    'pin_sda' : 'D3',
    'pin_scl' : 'D29',
    'pin_fet' : 'D7',
  },
  'QWIIC1' : {
    'pin_sda' : 'D2',
    'pin_scl' : 'D31',
    'pin_fet' : 'D27',
  },
  'QWIIC2' : {
    'pin_sda' : 'D44', # P1.12
    'pin_scl' : 'D45', # P1.13
    'pin_gnd' : 'D36', # P1.04
    'pin_vcc' : 'D43', # P1.11
  },
  'QWIIC3' : {
    'pin_sda' : 'D39', # P1.07
    'pin_scl' : 'D38', # P1.06
    'pin_gnd' : 'D37', # P1.05
    'pin_vcc' : 'D42', # P1.10
  },
  'DRIVER0' : {
    'pin_nsleep' : 'D21',
    'pin_nfault' : 'D19',
    'pin_mode' : 'D16',
    'pin_trq' : 'D12',
    'pin_d0' : 'D17',
    'pin_d1' : 'D15',
    'pin_d2' : 'D13',
    'pin_d3' : 'D14'
  },
  'DRIVER1' : {
    'pin_nsleep' : 'D23',
    'pin_nfault' : 'D20',
    'pin_mode' : 'D24',
    'pin_trq' : 'D35',
    'pin_d0' : 'D22',
    'pin_d1' : 'D32',
    'pin_d2' : 'D25',
    'pin_d3' : 'D34'  
  }
};

# left-right, or top-bottom order
board = {
  'bottom' : [ 'H0', 'H1', 'H2', 'H3', 'H4', 'H5', 'H6', 'H7' ],
  'top' : [ 'Q0.scl', 'Q0.sda', 'VCC', 'Q0.fet',#'D29', 'D3', 'VCC', 'D7', # Q0
            'Q1.scl', 'Q1.sda', 'VCC', 'Q1.fet',#'D31', 'D2', 'VCC', 'D27', # Q1
            'Q2.scl', 'Q2.sda', 'Q2.vcc', 'Q2.gnd',#'D45', 'D44', 'D43', 'D36', # Q2
            'Q3.scl', 'Q3.sda', 'Q3.vcc', 'Q3.gnd',#'D38', 'D39', 'D42', 'D37', # Q3
   ],
  'right' : ['NFC','NFC'],
  '_hide_not_on_connectors' : True,
  '_notes' : {
    'H0' : "Motor driver 0, output 0. This pin is also connected to an analog input via a 39k/220k potential divider", # D17
    'H1' : "Motor driver 0, output 1.", # D15
    'H2' : "Motor driver 0, output 2. This pin is also connected to an analog input via a 39k/220k potential divider", # D13
    'H3' : "Motor driver 0, output 3.", # D14
    'H4' : "Motor driver 1, output 0. This pin is also connected to an analog input via a 39k/220k potential divider", # D22
    'H5' : "Motor driver 1, output 1.", # D32
    'H6' : "Motor driver 1, output 2. This pin is also connected to an analog input via a 39k/220k potential divider", # D35
    'H7' : "Motor driver 1, output 3.", # D34
    
    'Q0.fet' : "INVERTED. Connected to 500mA FET. When 1, GND on Q0 is pulled low. When 0, GND on Q0 is open circuit",
    'Q1.fet' : "INVERTED. Connected to 500mA FET. When 1, GND on Q1 is pulled low. When 0, GND on Q1 is open circuit",
  },
};
board["_css"] = """
#board {
  width: 628px;
  height: 800px;
  top: 0px;
  left : 200px;
  background-image: url(img/JOLTJS.jpg);
}
#boardcontainer {
  height: 900px;
}

#top {
    top: 100px;
    left: 50px;
}
#bottom {
    top: 400px;
    left: 0px;
}
#right {
    top: 350px;
    left: 400px;
}

.toppin { width: 15px; }
.leftpin { height: 17px; }
.bottompin { width: 40px; }
""";

def get_pins():
  pins = pinutils.generate_pins(0,47,"D") + pinutils.generate_pins(0,7,"H"); # 48 General Purpose I/O Pins, 8 High power pins
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
  # High power analogs
  pinutils.findpin(pins, "PH0", True)["functions"]["ADC1_IN2"]=0;
  pinutils.findpin(pins, "PH2", True)["functions"]["ADC1_IN3"]=0;
  pinutils.findpin(pins, "PH4", True)["functions"]["ADC1_IN6"]=0;
  pinutils.findpin(pins, "PH6", True)["functions"]["ADC1_IN4"]=0;
  # renumber high power pins - the idea is despite being 'H' the pins still point to the correct 'real' pin number
  pinutils.findpin(pins, "PH0", True)["num"] = '17';
  pinutils.findpin(pins, "PH1", True)["num"] = '15';  
  pinutils.findpin(pins, "PH2", True)["num"] = '13';
  pinutils.findpin(pins, "PH3", True)["num"] = '14';
  pinutils.findpin(pins, "PH4", True)["num"] = '22';  
  pinutils.findpin(pins, "PH5", True)["num"] = '32';
  pinutils.findpin(pins, "PH6", True)["num"] = '25';
  pinutils.findpin(pins, "PH7", True)["num"] = '34';  
  

  # everything is non-5v tolerant
  for pin in pins:
    if pin["port"]!="H":
      pin["functions"]["3.3"]=0;
  #The boot/reset button will function as a reset button in normal operation. Pin reset on PD21 needs to be enabled on the nRF52832 device for this to work.
  return pins
