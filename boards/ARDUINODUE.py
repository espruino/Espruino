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
   'address' : 0x000C0000,
   'page_size' : 256,
   'pages' : 1024,		# Total flash is 2048 Pages in two banks
   'flash_available' : 1024,	# We have 2 banks of flash with 1024 pages each, Firmware is roughly 800 Pages but we only use bank 1 for code and bank 2 for saving Javascript 
 },
};

devices = {
 'LED1' : { 'pin' : 'D13' },
 'RX_PIN_NUMBER' : { 'pin' : 'D0'},
 'TX_PIN_NUMBER' : { 'pin' : 'D1'}, 
}

# left-right, from bottom to top - up is where the power jack is
# bottom-middle: from left to right
board = {
  'left' : ['CANTX', 'CANRX', 'DAC1', 'DAC0', 'A11', 'A10', 'A9', 'A8', 'A7', 'A6', 'A5', 'A4', 'A3', 'A2', 'A1', 'A0', 'VIN', 'GND', 'GND', '5V', '3V3', 'RESET', 'IOREF'],
  'right' : ['SCL', 'SDA', 'RX1', 'TX1', 'RX2', 'TX2', 'RX3', 'TX3', 'RX0', 'RX1', 'D2', 'D3', 'D4', 'D5', 'D6', 'D7', 'D8', 'D9', 'D10', 'D11', 'D12', 'D13', 'GND', 'AREF', 'SDA1', 'SCL1'],
  'middle' : ['GND', 'RESET', 'MOSI', 'SCK', '5V', 'MISO'],
  'bottom' : ['GND', 'GND', 'D53', 'D52', 'D51', 'D50', 'D49', 'D48', 'D47', 'D46', 'D45', 'D44', 'D43', 'D42', 'D41', 'D40', 'D39', 'D38', 'D37', 'D36', 'D35', 'D34', 'D33', 'D32', 'D31', 'D30', 'D29', 'D28', 'D27', 'D26', 'D25', 'D24', '5V', '5V'],
};

def get_pins():
  pins = pinutils.generate_pins(0,69)
  pinutils.findpin(pins, "PD0", True)["functions"]["RXD"]=0;
  pinutils.findpin(pins, "PD1", True)["functions"]["TXD"]=0;
  pinutils.findpin(pins, "PD13", True)["functions"]["LED"]=0;
  # everything is non-5v tolerant
  for pin in pins:
    pin["functions"]["3.3"]=0;
  return pins
