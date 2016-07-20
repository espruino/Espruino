#!/usr/bin/python

# This file is part of Espruino, a JavaScript interpreter for Microcontrollers
#
# Copyright (C) 2013 Gordon Williams <gw@pur3.co.uk>
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# ----------------------------------------------------------------------------------------
# Builds JS file with info for the board
# ----------------------------------------------------------------------------------------

import subprocess;
import re;
import json;
import sys;
import os;
import importlib;
import string;


scriptdir = os.path.dirname(os.path.realpath(__file__))
basedir = scriptdir+"/../"
sys.path.append(basedir+"scripts");
sys.path.append(basedir+"boards");

import common;
import pinutils;

# -----------------------------------------------------------------------------------------
boardname = ""
for i in range(1,len(sys.argv)):
  arg = sys.argv[i]
  if arg[0]=="-" and arg[1]=="B": 
    boardname = arg[2:]

# Now scan AF file
if boardname=="":
  print("ERROR, USAGE: build_board_json.py -Ddefine=1 -BBOARD_NAME")
  print("")
  print("It's much easier to run this from the Makefile with:")
  print("   BOARDNAME=1 make boardjson")
  exit(1)

print("Script location "+scriptdir)
jsonFilename = "boards/"+boardname+".json"
print("JSON_FILENAME "+jsonFilename)
print("BOARD "+boardname)
# import the board def
board = importlib.import_module(boardname)
# Call the included board_specific file - it sets up 'pins' and 'fill_gaps'
pins = board.get_pins()
pins = pinutils.append_devices_to_pin_list(pins, board)
# -----------------------------------------------------------------------------------------
# Documentation/functions
jsondatas = common.get_jsondata(False) # use command-line args
# -----------------------------------------------------------------------------------------
board.info["image_url"] = "http://www.espruino.com/img/"+boardname+".jpg"
board.info["thumb_url"] = "http://www.espruino.com/img/"+boardname+"_thumb.jpg"
board.info["binary_version"] = common.get_version();
board.info["binary_url"] = "http://www.espruino.com/binaries/"+common.get_board_binary_name(board)
# -----------------------------------------------------------------------------------------
# Built-in modules

builtinModules = []
for jsondata in jsondatas:
  if jsondata["type"]=="library":
    builtinModules.append(jsondata["class"])
#    print (json.dumps(jsondata, indent=1));

board.info["builtin_modules"] = builtinModules
# -----------------------------------------------------------------------------------------
pinperipherals = {}

for pin in pins:
  if pin["name"][0] == 'P':
    pin["name"] = pin["name"][1:];
  pin["simplefunctions"] = {};
  pinfuncs = pin["simplefunctions"]
  for func in sorted(pin["functions"]):
    if func in pinutils.CLASSES:       
      name = pinutils.CLASSES[func]
   
      # list for individual pin
      if name in pinfuncs:
        pinfuncs[name].append(func)
      else:
        pinfuncs[name] = [ func ];
   
      # now handle more detailed
      if name=="SPI" or name=="I2C" or name=="USART":
        fs = pinutils.ALLOWED_FUNCTIONS[func].split("|")        
        periph = fs[0][4:]
        periphpin = fs[1][fs[1].rfind("_")+1:]
        if not periph in pinperipherals: pinperipherals[periph]={}
        if not periphpin in pinperipherals[periph]: pinperipherals[periph][periphpin]=[]
        pinperipherals[periph][periphpin].append(pin["name"])
      else:
        if not name in pinperipherals: pinperipherals[name]={}
        if not "" in pinperipherals[name]: pinperipherals[name][""]=[]
        pinperipherals[name][""].append(pin["name"]);
  
boarddata = {
 "info" : board.info,
 "chip" : board.chip,
# "layout" : board.board,
 "devices" : board.devices,
 "pins" : pins,
 "peripherals" : pinperipherals,
};

jsonFile = open(jsonFilename, 'w')
jsonFile.write(json.dumps(boarddata, indent=1));
jsonFile.close();
print("JSON written to "+jsonFilename);
