#!/bin/false

import pinutils;
import json;

info = {
 'name': "Arduino Due",
 'link': [ "https://store.arduino.cc/usa/arduino-due" ],
 'variables': 1720,
 'binary_name': 'espruino_%v_arduinodue.bin',
 'default_console' : "EV_SERIAL4",
 'default_console_tx' : "1",
 'default_console_rx' : "0",
 'default_console_baudrate' : "115200",
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
 'family': "samd",
 'package': "LQFP144",
 'ram': 96,
 'flash': 512,
 'speed': 84,
 'usart': 4,
 'spi': 4,
 'i2c': 2,
 'adc': 1,
 'dac': 1,
};

devices = {
 'LED1' : { 'pin' : '13' },
 'RX_PIN_NUMBER' : { 'pin' : '0'},
 'TX_PIN_NUMBER' : { 'pin' : '1'}, 
}

# left-right, from bottom to top - up is where the power jack is
board = {
  'left' : [],
  'right' : ['0', '1', '13'],
};

def get_pins():
  pins = pinutils.generate_pins(0,2)
  pinutils.findpin(pins, "0", True)["functions"]["RXD"]=0;
  pinutils.findpin(pins, "1", True)["functions"]["TXD"]=0;
  pinutils.findpin(pins, "13", True)["functions"]["LED"]=0;
  # everything is non-5v tolerant
  for pin in pins:
    pin["functions"]["3.3"]=0;
  return pins
