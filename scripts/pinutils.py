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
# Used by boards/BOARDNAME.py to read csv files describing which functions are
# available on which pins in which packages
# ----------------------------------------------------------------------------------------

import subprocess;
import re;
import json;
import sys;
import os;

ALLOWED_PORTS = "ABCDEFGH";
ALLOWED_FUNCTIONS = {}
CLASSES = {}
NAMES = {}
for i in range(1, 3):
  ALLOWED_FUNCTIONS["DAC"+str(i)+"_OUT"] = "JSH_DAC|JSH_DAC_CH"+str(i);
  ALLOWED_FUNCTIONS["DAC_OUT"+str(i)] = "JSH_DAC|JSH_DAC_CH"+str(i);
  ALLOWED_FUNCTIONS["DAC1_OUT"+str(i)] = "JSH_DAC|JSH_DAC_CH"+str(i);
  ALLOWED_FUNCTIONS["DAC2_OUT"+str(i)] = "JSH_DAC|JSH_DAC_CH"+str(i);
for i in range(1, 7):
  ALLOWED_FUNCTIONS["USART"+str(i)+"_TX"] = "JSH_USART"+str(i)+"|JSH_USART_TX";
  ALLOWED_FUNCTIONS["USART"+str(i)+"_RX"] = "JSH_USART"+str(i)+"|JSH_USART_RX";
  ALLOWED_FUNCTIONS["UART"+str(i)+"_TX"] = "JSH_USART"+str(i)+"|JSH_USART_TX";
  ALLOWED_FUNCTIONS["UART"+str(i)+"_RX"] = "JSH_USART"+str(i)+"|JSH_USART_RX";
for i in range(1, 5):
  ALLOWED_FUNCTIONS["SPI"+str(i)+"_SCK"] = "JSH_SPI"+str(i)+"|JSH_SPI_SCK";
  ALLOWED_FUNCTIONS["SPI"+str(i)+"_MISO"] = "JSH_SPI"+str(i)+"|JSH_SPI_MISO";
  ALLOWED_FUNCTIONS["SPI"+str(i)+"_MOSI"] = "JSH_SPI"+str(i)+"|JSH_SPI_MOSI";
for i in range(1, 5):
  ALLOWED_FUNCTIONS["I2C"+str(i)+"_SCL"] = "JSH_I2C"+str(i)+"|JSH_I2C_SCL";
  ALLOWED_FUNCTIONS["I2C"+str(i)+"_SDA"] = "JSH_I2C"+str(i)+"|JSH_I2C_SDA";
#I2C_SMBA?
for i in range(1, 19):
  for ch in range(1, 5):
    ALLOWED_FUNCTIONS["TIM"+str(i)+"_CH"+str(ch)] = "JSH_TIMER"+str(i)+"|JSH_TIMER_CH"+str(ch);
    ALLOWED_FUNCTIONS["TIM"+str(i)+"_CH"+str(ch)+"N"] = "JSH_TIMER"+str(i)+"|JSH_TIMER_CH"+str(ch)+"|JSH_TIMER_NEGATED";

CLASSES["CANRX"]="CAN"
CLASSES["CANTX"]="CAN"
CLASSES["CAN_RX"]="CAN"
CLASSES["CAN_TX"]="CAN"
for i in range(0,17):
  CLASSES["ADC1_IN"+str(i)]="ADC"
  CLASSES["ADC2_IN"+str(i)]="ADC"
  CLASSES["ADC3_IN"+str(i)]="ADC"
  CLASSES["ADC12_IN"+str(i)]="ADC"
  CLASSES["ADC123_IN"+str(i)]="ADC"
for fn in ALLOWED_FUNCTIONS:
  defn = ALLOWED_FUNCTIONS[fn];
  if defn.startswith("JSH_DAC"): CLASSES[fn]="DAC"
  if defn.startswith("JSH_USART"): CLASSES[fn]="USART"
  if defn.startswith("JSH_SPI"): CLASSES[fn]="SPI"
  if defn.startswith("JSH_I2C"): CLASSES[fn]="I2C"
  if defn.startswith("JSH_TIMER"): CLASSES[fn]="PWM"


DEVICES = {
 "OSC":"OSC",
 "OSC_RTC":"OSC RTC",
 "BOOT0":"BOOT0",
 "BOOT1":"BOOT1",
 "SD":"SD CARD",
 "USB":"USB",
 "BLUETOOTH":"BLUETOOTH",
 "TOUCHSCREEN":"TOUCH",
 "LCD":"LCD",
 "MIC":"MIC",
 "AUDIO":"AUDIO",
 "MEMS":"MEMS",
 "GYRO":"GYRO",
};

for i in range(0,7):
 DEVICES["LED"+str(i)]="LED"+str(i)
 DEVICES["BTN"+str(i)]="BTN"+str(i)
 DEVICES["POT"+str(i)]="POT"+str(i)


for D in DEVICES:
 CLASSES[D]="DEVICE"
 NAMES[D]=DEVICES[D]

# is a pin name valid
def isvalidpin(pinname):
  pinport = pinname[1:2]
  if pinname[:1]=="P" and ALLOWED_PORTS.find(pinport)!=-1:
    if pinname.find('-')!=-1: pinname = pinname[:pinname.find('-')]
    pinnum = pinname[2:]
    return pinnum.isdigit()
  return False
    

# Find/populate a pin
def haspin(pins, pinname):
  for pin in pins:
    if pin["name"]==pinname: 
      return True
  return False

# Find/populate a pin
def findpin(pins, pinname, force):
  if pinname.find('-')!=-1: pinname = pinname[:pinname.find('-')]
  for pin in pins:
    if pin["name"]==pinname: 
      return pin
  if force:
    print "ERROR: pin "+pinname+" not found"
    exit(1);
  pin = {}
  pin["name"] = pinname
  pin["port"] = pinname[1:2]
  pin["num"] = pinname[2:]
  pin["sortingname"] = pin["port"]+pin["num"].rjust(2,'0')
  pin["functions"] = {}
  pin["csv"] = {}
  pins.append(pin)
  return pin

# Code for scanning AF file
def scan_pin_af_file(pins, filename, nameoffset, afoffset):
  f = open(os.path.dirname(os.path.realpath(__file__))+'/../boards/pins/'+filename)
  lines = f.readlines()
  f.close()
  for line in lines:
    pindata = line.split(",")
    pinname = pindata[nameoffset].strip()
    if pinname.find('(')>0: pinname = pinname[:pinname.find('(')]
    if not isvalidpin(pinname): continue
    pin = findpin(pins, pinname, False)    
    #print(json.dumps(pin, sort_keys=True, indent=2))  
    for af in range(0, len(pindata)-afoffset):
      fname = pindata[af+afoffset].strip()
      pin["functions"][fname] = af
      #print pinname+" --- "+fname+" : "+str(af)
  return pins

# Code for scanning normal file
def scan_pin_file(pins, filename, nameoffset, functionoffset, altfunctionoffset): 
  f = open(os.path.dirname(os.path.realpath(__file__))+'/../boards/pins/'+filename)
  lines = f.readlines()  
  f.close()
  headings = lines[0].split(",")
  for line in lines:
    pindata = line.split(",")
    pinname = pindata[nameoffset].strip()

    extrafunction = "" 
    if "BOOT1" in line: extrafunction="BOOT1"                            
    if pinname.find('(')>0: pinname = pinname[:pinname.find('(')]
    if not isvalidpin(pinname): continue
    pin = findpin(pins, pinname, False)
    for i,head in enumerate(headings):
      pin["csv"][head] = pindata[i].strip()      
    if extrafunction!="":
      pin["functions"][extrafunction] = 0
    for fn in pindata[functionoffset].strip().split("/"): 
      fname = fn.strip()
      pin["functions"][fname] = 0      
    if altfunctionoffset>=0:
      for fn in pindata[altfunctionoffset].strip().split("/"): 
        fname = fn.strip()
        pin["functions"][fname] = 1
#    print pin["name"]+" : "+', '.join(pin["functions"])
  return pins

# Create a simple list of pins
def generate_pins(min_pin, max_pin):
  pins = []
  for n in range(min_pin, max_pin+1):
    findpin(pins, "PD"+str(n), False)
  return pins

# fill in gaps - eg. put A2 in A0,A1,A3,A4
def fill_gaps_in_pin_list(pins):
  # first sort
  pins = sorted(pins, key=lambda pin: pin["sortingname"])
  # then fill in
  prevpin = False
  newpins = []
  for pin in pins:
    if prevpin!=False:
      if prevpin["port"]==pin["port"]:
        for num in range(int(prevpin["num"])+1,int(pin["num"])):
          newpin = {}
          newpin["name"] = "P"+pin["port"]+str(num)
          newpin["port"] = "_NONE"
          newpin["num"] = str(num)
          newpin["sortingname"] = pin["port"]+newpin["num"].rjust(2,'0')
          newpin["functions"] = {}
          newpins.append(newpin)
          print "Added fake pin "+newpin["name"]
    newpins.append(pin)
    prevpin = pin
  return newpins

# Only return the pins for the specified package
def only_from_package(pins, package):
  newpins = []
  for pin in pins:
#    print json.dumps(pin)
    pinnumber =  pin["csv"][package]
    if pinnumber!="" and pinnumber!="0":
      newpins.append(pin)
  return newpins

def get_device_pins(board):
  pins = {}
  for devicename in board.devices:
    for deviceinfo in board.devices[devicename]:
      if deviceinfo[:3]=="pin":
        pinname = board.devices[devicename][deviceinfo]
        pins["P"+pinname] = { "device":devicename, "function": deviceinfo };
#  print(json.dumps(pins))
  return pins

# If devices are used by a board, fill in their details for each pin
def append_devices_to_pin_list(pins, board):
  devicepins = get_device_pins(board)
  
  for i,pin in enumerate(pins):
    if pin["name"] in devicepins:
      pins[i]["functions"][devicepins[pin["name"]]["device"]] = devicepins[pin["name"]]["function"]
#      print pins[i]["functions"][devicepins[pin["name"]]["device"]] 
  return pins

