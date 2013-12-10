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

# Now scan AF file
print "Script location "+scriptdir
if len(sys.argv)!=2:
  print "ERROR, USAGE: build_board_json.py BOARD_NAME"
  exit(1)
boardname = sys.argv[1]
jsonFilename = "boards/"+boardname+".json"
print "JSON_FILENAME "+jsonFilename
print "BOARD "+boardname
# import the board def
board = importlib.import_module(boardname)
# Call the included board_specific file - it sets up 'pins' and 'fill_gaps'
pins = board.get_pins()
pins = pinutils.append_devices_to_pin_list(pins, board)

# -----------------------------------------------------------------------------------------
board.info["image_url"] = "http://www.espruino.com/img/"+boardname+".jpg"
board.info["thumb_url"] = "http://www.espruino.com/img/"+boardname+"_thumb.jpg"
board.info["binary_version"] = common.get_version();
board.info["binary_url"] = "http://www.espruino.com/binaries/"+common.get_board_binary_name(board)
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
 "layout" : board.board,
 "devices" : board.devices,
 "pins" : pins,
 "peripherals" : pinperipherals,
};

jsonFile = open(jsonFilename, 'w')
jsonFile.write(json.dumps(boarddata, indent=1));
jsonFile.close();

