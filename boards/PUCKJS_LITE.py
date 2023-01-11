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
 'name' : "Puck.js Lite",
  # The Puck.js lite can use the same firmware as normal, but the bootloader needs to know
  # there's no LED3, so it can light up red instead of blue when connected
 'boardname' : "PUCKJS",
 'link' :  [ "https://espruino.com/Puck.js" ],
 'espruino_page_link' : 'Puck.js',
 'default_console' : "EV_SERIAL1",
 'default_console_tx' : "D28",
 'default_console_rx' : "D29",
 'default_console_baudrate' : "9600",
 'variables' : 2630, # How many variables are allocated for Espruino to use. RAM will be overflowed if this number is too high and code won't compile.
 'bootloader' : 1,
 'binary_name' : 'espruino_%v_puckjs_lite.hex',
 'build' : {
   'optimizeflags' : '-Os',
   'libraries' : [
     'BLUETOOTH',
     'NET',
     'GRAPHICS',
     'CRYPTO','SHA256',#'SHA512',
     'AES',
     'NEOPIXEL'
   ],
   'makefile' : [
     'DEFINES+=-DPUCKJS_LITE', 
     'DEFINES+=-DHAL_NFC_ENGINEERING_BC_FTPAN_WORKAROUND=1', # Looks like proper production nRF52s had this issue
     # 'DEFINES+=-DCONFIG_GPIO_AS_PINRESET', # reset isn't being used, so let's just have an extra IO (needed for Puck.js V2)
     'DEFINES+=-DESPR_DCDC_ENABLE', # Ensure DCDC converter is enabled
     'DEFINES += -DNEOPIXEL_SCK_PIN=22 -DNEOPIXEL_LRCK_PIN=16', # SCK pin needs defining as something unused for neopixel (HW errata means they can't be disabled) see https://github.com/espruino/Espruino/issues/2071
     'DEFINES += -DNRF_BLE_GATT_MAX_MTU_SIZE=53 -DNRF_BLE_MAX_MTU_SIZE=53', # increase MTU from default of 23
     'DEFINES += -DCENTRAL_LINK_COUNT=2 -DNRF_SDH_BLE_CENTRAL_LINK_COUNT=2', # allow two outgoing connections at once     
     'LDFLAGS += -Xlinker --defsym=LD_APP_RAM_BASE=0x3290', # set RAM base to match MTU=53 + CENTRAL_LINK_COUNT=2
     'DEFINES+=-DBLUETOOTH_NAME_PREFIX=\'"Puck.js"\'',
     'DEFINES+=-DCUSTOM_GETBATTERY=jswrap_puck_getBattery',
     'DEFINES+=-DNFC_DEFAULT_URL=\'"https://puck-js.com/go"\'',
     'DEFINES+=-DAPP_TIMER_OP_QUEUE_SIZE=3', # Puck.js magnetometer poll
     'DFU_PRIVATE_KEY=targets/nrf5x_dfu/dfu_private_key.pem',
     'DFU_SETTINGS=--application-version 0xff --hw-version 52 --sd-req 0x8C,0x91',
     'INCLUDE += -I$(ROOT)/libs/puckjs',
     'WRAPPERSOURCES += libs/puckjs/jswrap_puck.c'
   ]
 }
};

chip = {
  'part' : "NRF52832",
  'family' : "NRF52",
  'package' : "QFN48",
  'ram' : 64,
  'flash' : 512,
  'speed' : 64,
  'usart' : 1,
  'spi' : 1,
  'i2c' : 1,
  'adc' : 1,
  'dac' : 0,
  'saved_code' : {
    'address' : ((118 - 10) * 4096), # Bootloader takes pages 120-127, FS takes 118-119
    'page_size' : 4096,
    'pages' : 10,
    'flash_available' : 512 - ((31 + 8 + 2 + 10)*4) # Softdevice uses 31 pages of flash, bootloader 8, FS 2, code 10. Each page is 4 kb.
  },
};

devices = {
  'LED1' : { 'pin' : 'D5' },
  'LED2' : { 'pin' : 'D4' },
#  'LED3' : { 'pin' : 'D3' }, # Disabled on Puck.js Lite
  'IR'   : { 'pin_anode' : 'D25',   # on v2 this just goes to a FET
             'pin_cathode' : 'D26'  # on v2 this is the powered output named 'FET'
           },
  'BTN1' : { 'pin' : 'D0', 'pinstate' : 'IN_PULLDOWN' },
  'CAPSENSE' : { 'pin_rx' : 'D11', 'pin_tx' : 'D12' },
  'NFC': { 'pin_a':'D9', 'pin_b':'D10' },
  'MAG': { 'device': 'LIS3MDL', 'addr' : 30, # v2.0
           'pin_pwr':'D18',
           'pin_int':'D17',
           'pin_sda':'D20',
           'pin_scl':'D19',
           'pin_drdy':'D21',
           },
  'ACCEL': { 'device': 'LSM6DS3TR', 'addr' : 106, # v2.0
#           'pin_pwr':'D16', # can't actually power this from an IO pin due to undocumented, massive power draw on startup
           'pin_int':'D13',
           'pin_sda':'D14',
           'pin_scl':'D15' },
  'TEMP': { 'device': 'PCT2075TP', 'addr' : 78, # v2.0
           'pin_pwr':'D8',
           'pin_sda':'D7',
           'pin_scl':'D6' }

  # Pin D22 is used for clock when driving neopixels - as not specifying a pin seems to break things
};

# left-right, or top-bottom order
board = {
  'bottom' : [ 'D28', 'D29', 'D30', 'D31'],
  'right' : [ 'GND', '3V', 'D2', 'D1' ],
  'left2' : [ 'D6','D7','D8','D11','D13','D14','D16','D23','D24','D27' ],
  'right2' : [ 'D15' ],
  '_notes' : {
    'D11' : "Capacitive sense. D12 is connected to this pin via a 1 MOhm resistor",
    'D28' : "If pulled up to 1 on startup, D28 and D29 become Serial1",
    'D22' : "This is used as SCK when driving Neopixels, and will output a signal when 'require('neopixel').write' is called",
    'D16' : "This is used as LRCK when driving Neopixels, and will output a signal when 'require('neopixel').write' is called"
  }
};

board["_css"] = """
#board {
  width: 800px;
  height: 800px;
  top: 0px;
  left : 0px;
  background-image: url(img/PUCKJS_.jpg);
}
#boardcontainer {
  height: 900px;
}
#bottom {
    top: 639px;
    left: 291px;
}
#right {
    top: 304px;
    left: 640px;
}

.bottompin { width: 46px; }
.rightpin { height: 51px; }
.pinD6 { position:absolute; left: 560px; top: 419px;}
.pinD7 { position:absolute; left: 548px; top: 369px;}
.pinD8 { position:absolute; left: 512px; top: 398px;}
.pinD11 { position:absolute; left: 586px; top: 236px;}
.pinD13 { position:absolute; left: 500px; top: 293px;}
.pinD14 { position:absolute; left: 523px; top: 270px;}
.pinD15 { position:absolute; right: -483px; top: 268px;}
.pinD16 { position:absolute; left: 499px; top: 244px;}
.pinD23 { position:absolute; left: 157px; top: 438px;}
.pinD24 { position:absolute; left: 157px; top: 382px;}
.pinD27 { position:absolute; left: 244px; top: 581px;}
""";

def get_pins():
  pins = pinutils.generate_pins(0,31) # 32 General Purpose I/O Pins.
  pinutils.findpin(pins, "PD0", True)["functions"]["XL1"]=0;
  pinutils.findpin(pins, "PD1", True)["functions"]["XL2"]=0;
  pinutils.findpin(pins, "PD9", True)["functions"]["NFC1"]=0;
  pinutils.findpin(pins, "PD10", True)["functions"]["NFC2"]=0;
  pinutils.findpin(pins, "PD2", True)["functions"]["ADC1_IN0"]=0;
  pinutils.findpin(pins, "PD3", True)["functions"]["ADC1_IN1"]=0;
  pinutils.findpin(pins, "PD4", True)["functions"]["ADC1_IN2"]=0;
  pinutils.findpin(pins, "PD5", True)["functions"]["ADC1_IN3"]=0;
  pinutils.findpin(pins, "PD28", True)["functions"]["ADC1_IN4"]=0;
  pinutils.findpin(pins, "PD28", True)["functions"]["USART1_TX"]=0;
  pinutils.findpin(pins, "PD29", True)["functions"]["USART1_RX"]=0;
  pinutils.findpin(pins, "PD29", True)["functions"]["ADC1_IN5"]=0;
  pinutils.findpin(pins, "PD30", True)["functions"]["ADC1_IN6"]=0;
  pinutils.findpin(pins, "PD31", True)["functions"]["ADC1_IN7"]=0;
  # everything is non-5v tolerant
  for pin in pins:
    pin["functions"]["3.3"]=0;

  #The boot/reset button will function as a reset button in normal operation. Pin reset on PD21 needs to be enabled on the nRF52832 device for this to work.
  return pins
