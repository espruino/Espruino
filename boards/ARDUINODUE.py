#!/bin/false

import pinutils;
import json;

info = {
 'name': "Arduino Due",
 'link': [ "https://store.arduino.cc/usa/arduino-due" ],
 'variables': 1720,
 'binary_name': 'espruino_%v_arduinodue.bin',
 'default_console' : "EV_SERIAL1",
 'default_console_tx' : "D1",
 'default_console_rx' : "D0",
 'default_console_baudrate' : "9600",
 'build' : {
  'optimizeflags' : '-Os',
  'libraries' : [
  ],
  'makefile' : [
  ]
 }
};
chip = {
 'part': "ATSAM3X8E",
 'family': "SAMD",
 'package': "LQFP144",
 'ram': 96,
 'flash': 512,
 'speed': 84,
 'usart': 4,
 'spi': 4,
 'i2c': 2,
 'adc': 1,
 'dac': 1,
 'saved_code' : {
   'address' : 0x0008000,
   'page_size' : 256,
   'pages' : 2048,
   'flash_available' : 312,	# Firmware is roughly 800 Pages 200 KB, leaving around 312 KB unused 
 },
};

devices = {
 'LED1' : { 'pin' : 'D13' },
 'RX_PIN_NUMBER' : { 'pin' : 'D0'},
 'TX_PIN_NUMBER' : { 'pin' : 'D1'}, 
}

# left-right, from bottom to top - up is where the power jack is
board = {
  'left' : [],
  'right' : ['D0', 'D1', 'D13'],
};

def get_pins():
  pins = pinutils.generate_pins(0,53)
  pinutils.findpin(pins, "PD0", True)["functions"]["RXD"]=0;
  pinutils.findpin(pins, "PD1", True)["functions"]["TXD"]=0;
  pinutils.findpin(pins, "PD13", True)["functions"]["LED"]=0;
  # everything is non-5v tolerant
  for pin in pins:
    pin["functions"]["3.3"]=0;
  return pins
