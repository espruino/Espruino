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

# Schematic http://docs.rakwireless.com/en/RAK8211/Hardware%20Design/RAK8211_iTRACKER_M35_V20_20171203.pdf
# GPS HW    https://www.quectel.com/UploadImage/Downlad/L70_Hardware_Design_V2.0.pdf
# GPS SW    http://docs-europe.electrocomponents.com/webdocs/147d/0900766b8147dbdd.pdf

info = {
 'name' : "iTracker RAK8211",
 #https://github.com/RAKWireless/ITRACKER-Arduino-SDK
 'link' :  [ "http://docs.rakwireless.com/en/RAK8211/Hardware%20Design/RAK8211-G%20Datasheet%20V1.0.pdf" ],
 'espruino_page_link' : 'RAK8211',
 'default_console' : "EV_SERIAL1",
 'default_console_tx' : "D29",
 'default_console_rx' : "D28",
 'default_console_baudrate' : "9600",
 'variables' : 2500, # How many variables are allocated for Espruino to use. RAM will be overflowed if this number is too high and code won't compile.
 'bootloader' : 1,
 'binary_name' : 'espruino_%v_RAK8211.hex',
 'build' : {
   'optimizeflags' : '-Os',
   'libraries' : [
     'BLUETOOTH',
     'NET'
   ],
   'makefile' : [
     'DEFINES+=-DCONFIG_GPIO_AS_PINRESET', # Allow the reset pin to work
     'DEFINES+=-DCONFIG_NFCT_PINS_AS_GPIOS', # Don't use NFC - the pins are used for GPS
     'DEFINES+=-DHAL_NFC_ENGINEERING_BC_FTPAN_WORKAROUND=1', # Looks like proper production nRF52s had this issue
     'DEFINES+=-DBLUETOOTH_NAME_PREFIX=\'"iTracker"\'',
     'DFU_PRIVATE_KEY=targets/nrf5x_dfu/dfu_private_key.pem',
     'DFU_SETTINGS=--application-version 0xff --hw-version 52 --sd-req 0x8C',
     'JSMODULESOURCES += libs/js/AT.min.js',
     'JSMODULESOURCES += libs/js/GPS.min.js',
     'JSMODULESOURCES += libs/js/BME280.min.js',
     'JSMODULESOURCES += libs/js/LIS2MDL.min.js',
     'JSMODULESOURCES += libs/js/LIS3DH.min.js',
     'JSMODULESOURCES += libs/js/OPT3001.min.js',
#     'JSMODULESOURCES += libs/js/ATSMS.min.js',
#     'JSMODULESOURCES += libs/js/QuectelM35.min.js',
     'JSMODULESOURCES += iTracker:libs/js/rak/RAK8211.min.js'
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
  'usart' : 3,
  'spi' : 3,
  'i2c' : 2,
  'adc' : 1,
  'dac' : 0,
  'saved_code' : {
    'address' : ((118 - 3) * 4096), # Bootloader takes pages 120-127, FS takes 118-119
    'page_size' : 4096,
    'pages' : 3,
    'flash_available' : 512 - ((31 + 8 + 1 + 3)*4) # Softdevice uses 31 pages of flash, bootloader 8, FS 1, code 3. Each page is 4 kb.
  },
};

devices = {
  'BTN1' : { 'pin' : 'D30', 'pinstate' : 'IN_PULLDOWN' },
  'GPRS' : {'pin_tx' : 'D12', 'pin_rx' : 'D20', 'pin_reset' : 'D14', 'pin_pwrkey' : 'D15', 'pin_pwron' : 'D6'},
  'GPS' : {'pin_tx' : 'D9', 'pin_rx' : 'D8', 'pin_standby' : 'D7', 'pin_pwron' : 'D10', 'pin_reset' : 'D31'},
  'BME' : {'pin_cs' : 'D2', 'pin_sdi' : 'D3', 'pin_sck': 'D4', 'pin_sdo' : 'D5'},
  'LIS2MDL' : {'pin_scl' : 'D11', 'pin_sda': 'D13', 'pin_int' : 'D16'},
  'LIS3DH' : {'pin_scl' : 'D18', 'pin_sda' : 'D19', 'pin_int1' : 'D25', 'pin_res' : 'D26', 'pin_int2' : 'D27'},
  'OPT'  : {'pin_sda' : '21', 'pin_scl' : 'D23', 'pin_int' : 'D22'}
  # Pin D22 is used for clock when driving neopixels - as not specifying a pin seems to break things
};

# left-right, or top-bottom order
board = {
  'right' : [ 'D21', '3V', 'GND', 'D30', '',
              'GND', '3V', 'D29', 'D28', '',
              '3V', 'SWDIO', 'SWDCLK', 'GND' ],
  '_hide_not_on_connectors' : True,
  '_notes' : {
    'D21' : "Also RESET if configured",
    'D30' : "Labelled TILT_DOUT",
    'D29' : "Labelled SENSOR_DOUT2",
    'D28' : "Labelled SENSOR_DOUT1"
  }
};

board["_css"] = """
#board {
  width: 400px;
  height: 506px;
  top: 0px;
  left : 0px;
  background-image: url(img/RAK8211.png);
}
#boardcontainer {
  height: 506px;
}
#right {
  top: 73px;
  left: 405px;
}
.rightpin {
  height: 18px;
}
""";

def get_pins():
  pins = pinutils.generate_pins(0,31) # 32 General Purpose I/O Pins.
  pinutils.findpin(pins, "PD0", True)["functions"]["XL1"]=0;
  pinutils.findpin(pins, "PD1", True)["functions"]["XL2"]=0;
  pinutils.findpin(pins, "PD28", True)["functions"]["ADC1_IN4"]=0;
  pinutils.findpin(pins, "PD28", True)["functions"]["USART1_TX"]=0;
  pinutils.findpin(pins, "PD29", True)["functions"]["USART1_RX"]=0;
  pinutils.findpin(pins, "PD12", True)["functions"]["GPRS_TX"]=0;
  pinutils.findpin(pins, "PD20", True)["functions"]["GPRS_RX"]=0;
  pinutils.findpin(pins, "PD14", True)["functions"]["GPRS_RESET"]=0;
  pinutils.findpin(pins, "PD15", True)["functions"]["GPRS_PWRKEY"]=0;
  pinutils.findpin(pins, "PD6", True)["functions"]["GPRS_PWN_ON"]=0;
  pinutils.findpin(pins, "PD9", True)["functions"]["GPS_TX"]=0;
  pinutils.findpin(pins, "PD8", True)["functions"]["GPS_RX"]=0;
  pinutils.findpin(pins, "PD7", True)["functions"]["GPS_STANDBY"]=0;
  pinutils.findpin(pins, "PD10", True)["functions"]["GPS_PWR_ON"]=0;
  pinutils.findpin(pins, "PD31", True)["functions"]["GPS_RESET"]=0;
  pinutils.findpin(pins, "PD2", True)["functions"]["BME_CS"]=0;
  pinutils.findpin(pins, "PD3", True)["functions"]["BME_SDI"]=0;
  pinutils.findpin(pins, "PD4", True)["functions"]["BME_SCK"]=0;
  pinutils.findpin(pins, "PD5", True)["functions"]["BME_SDO"]=0;
  pinutils.findpin(pins, "PD11", True)["functions"]["LIS2MDL_SCL"]=0;
  pinutils.findpin(pins, "PD13", True)["functions"]["LIS2MDL_SDA"]=0;
  pinutils.findpin(pins, "PD16", True)["functions"]["LIS2MDL_INT"]=0;
  pinutils.findpin(pins, "PD18", True)["functions"]["LIS3DH_SCL"]=0;
  pinutils.findpin(pins, "PD19", True)["functions"]["LIS3DH_SDA"]=0;
  pinutils.findpin(pins, "PD25", True)["functions"]["LIS3DH_INT1"]=0;
  pinutils.findpin(pins, "PD27", True)["functions"]["LIS3DH_INT2"]=0;
  pinutils.findpin(pins, "PD26", True)["functions"]["LIS3DH_RES"]=0;
  pinutils.findpin(pins, "PD21", True)["functions"]["OPT_SDA"]=0;
  pinutils.findpin(pins, "PD23", True)["functions"]["OPT_SCL"]=0;
  pinutils.findpin(pins, "PD22", True)["functions"]["OPT_INT"]=0;


  # everything is non-5v tolerant
  for pin in pins:
    pin["functions"]["3.3"]=0;

  #The boot/reset button will function as a reset button in normal operation. Pin reset on PD21 needs to be enabled on the nRF52832 device for this to work.
  return pins
