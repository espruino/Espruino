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
 'name' : "Nordic Thingy:52",
 'link' :  [ "https://www.nordicsemi.com/eng/Products/Nordic-Thingy-52" ],
 'espruino_page_link' : 'Thingy52',
 'default_console' : "EV_SERIAL1",
 'default_console_tx' : "D2",
 'default_console_rx' : "D3",
 'default_console_baudrate' : "9600",
 'variables' : 2500, # How many variables are allocated for Espruino to use. RAM will be overflowed if this number is too high and code won't compile.
 'bootloader' : 1,
 'binary_name' : 'espruino_%v_thingy52.bin',
 'build' : {
   'optimizeflags' : '-Os',
   'libraries' : [
     'BLUETOOTH',
#     'NET',
     'GRAPHICS',
     'NFC'
   ],
   'makefile' : [
     'DEFINES+=-DHAL_NFC_ENGINEERING_BC_FTPAN_WORKAROUND=1', # Looks like proper production nRF52s had this issue
     'DEFINES+=-DBLUETOOTH_NAME_PREFIX=\'"Thingy"\'',
     'DEFINES+=-DDUMP_IGNORE_VARIABLES=\'"Thingy\\0"\'',
     'DFU_PRIVATE_KEY=targets/nrf5x_dfu/dfu_private_key.pem',
     'DFU_SETTINGS=--application-version 0xff --hw-version 52 --sd-req 0x8C',
     'INCLUDE += -I$(ROOT)/libs/nordic_thingy',
     'WRAPPERSOURCES += libs/nordic_thingy/jswrap_thingy.c',
     'JSMODULESOURCES+=libs/nordic_thingy/LIS2DH12.min.js',
     'JSMODULESOURCES+=libs/nordic_thingy/LPS22HB.min.js',
     'JSMODULESOURCES+=libs/nordic_thingy/HTS221.min.js',
     'JSMODULESOURCES+=libs/nordic_thingy/CCS811.min.js',
     'JSMODULESOURCES+=libs/nordic_thingy/BH1745.min.js',
     'JSMODULESOURCES+=libs/nordic_thingy/Thingy.min.js'
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
  'spi' : 3,
  'i2c' : 2,
  'adc' : 1,
  'dac' : 0,
  'saved_code' : {
    'address' : ((118 - 10) * 4096), # Bootloader takes pages 120-127, FS takes 118-119
    'page_size' : 4096,
    'pages' : 10,
    'flash_available' : 512 - ((31 + 8 + 2 + 10)*4) # Softdevice uses 31 pages of flash, bootloader 8, FS 2, code 10. Each page is 4 kb.
  },
};

'''
P0.00	XL1	Low frequency crystal
P0.01	XL2	Low frequency crystal
P0.02	ANA/DIG0	Analog/Digital GPIO externally available
P0.03	ANA/DIG1	Analog/Digital GPIO externally available
P0.04	ANA/DIG2	Analog/Digital GPIO externally available
P0.05	SX_OSCIO	I/O expander oscillator input line
P0.06	MPU_INT	Motion sensor interrupt line
P0.07	SDA	I2C data line
P0.08	SCL	I2C clock line
P0.09	NFC1	Near field communication antenna
P0.10	NFC2	Near field communication antenna
P0.11	BUTTON	Button input
P0.12	LIS_INT1	Low power accelerometer interrupt line
P0.13	USB_DETECT	USB detect signal
P0.14	SDA_EXT	External and low power accelerometer I2C data line
P0.15	SCL_EXT	External and low power accelerometer I2C clock line
P0.16	SX_RESET	I/O expander reset line
P0.17	BAT_CHG_STAT	Battery charge status
P0.18	MOS_1	Gate of N-MOS transistor externally available
P0.19	MOS_2	Gate of N-MOS transistor externally available
P0.20	MOS_3	Gate of N-MOS transistor externally available
P0.21	MOS_4	Gate of N-MOS transistor externally available
P0.22	CCS_INT	Gas sensor interrupt line
P0.23	LPS_INT	Pressure sensor interrupt line
P0.24	HTS_INT	Humidity sensor interrupt line
P0.25	MIC_DOUT	Microphone PDM data
P0.26	MIC_CLK	Microphone PDM clock
P0.27	SPEAKER	Speaker PWM signal
P0.28	BATTERY	Battery monitoring input
P0.29	SPK_PWR_CTRL	Speaker amplifier power control
P0.30	VDD_PWD_CTRL	Power control for sensors, I/O expander, and LEDs
P0.31	BH_INT	Color sensor interrupt line

SXIO0	IOEXT0	Digital GPIO externally available
SXIO1	IOEXT1	Digital GPIO externally available
SXIO2	IOEXT2	Digital GPIO externally available
SXIO3	IOEXT3	Digital GPIO externally available
SXIO4	BAT_MON_EN	Battery monitoring enable
SXIO5	LIGHTWELL_G	Green color of the lightwell LEDs
SXIO6	LIGHTWELL_B	Blue color of the lightwell LEDs
SXIO7	LIGHTWELL_R	Red color of the lightwell LEDs
SXIO8	MPU_PWR_CTRL	Motion sensor power control
SXIO9	MIC_PWR_CTRL	Microphone power control
SXIO10	CCS_PWR_CTRL	Gas sensor power control
SXIO11	CCS_RESET	Gas sensor reset line
SXIO12	CCS_WAKE	Gas sensor wake line
SXIO13	SENSE_LED_R	Red color of the color sensor support LED
SXIO14	SENSE_LED_G	Green color of the color sensor support LED
SXIO15	SENSE_LED_B	Blue color of the color sensor support LED
'''

devices = {
  'BTN1' : { 'pin' : 'D11', 'pinstate' : 'IN_PULLDOWN' }, # Pin negated in software
  # 'V' pins are virtual
  'LED1' : { 'pin' : 'V7' }, # Pin negated in software
  'LED2' : { 'pin' : 'V5' }, # Pin negated in software
  'LED3' : { 'pin' : 'V6' }, # Pin negated in software
};

# left-right, or top-bottom order
board = {
  'left' : [ 'VDD', 'VDD', 'RESET', 'VDD','5V','GND','GND','PD3','PD4','PD28','PD29','PD30','PD31'],
  'right' : [ 'PD27', 'PD26', 'PD2', 'GND', 'PD25','PD24','PD23', 'PD22','PD20','PD19','PD18','PD17','PD16','PD15','PD14','PD13','PD12','PD11','PD10','PD9','PD8','PD7','PD6','PD5','PD21','PD1','PD0'],
};
board["_css"] = """
""";

def get_pins():
  # 32 General Purpose I/O Pins, 16 'virtual' Port Expanded pins
  pins = pinutils.generate_pins(0,31,"D") + pinutils.generate_pins(0,15,"V");
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
  pinutils.findpin(pins, "PD11", True)["functions"]["NEGATED"]=0;
  pinutils.findpin(pins, "PV5", True)["functions"]["NEGATED"]=0;
  pinutils.findpin(pins, "PV6", True)["functions"]["NEGATED"]=0;
  pinutils.findpin(pins, "PV7", True)["functions"]["NEGATED"]=0;


  # everything is non-5v tolerant
  for pin in pins:
    pin["functions"]["3.3"]=0;
  #The boot/reset button will function as a reset button in normal operation. Pin reset on PD21 needs to be enabled on the nRF52832 device for this to work.
  return pins
