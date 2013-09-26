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
# Simple script to extract the user-friendly name of the board from boards/BOARDNAME.py
# Used when auto-generating the website
# ----------------------------------------------------------------------------------------

import subprocess;
import re;
import json;
import sys;
import os;
import importlib;

scriptdir = os.path.dirname(os.path.realpath(__file__))
basedir = scriptdir+"/../"
sys.path.append(basedir+"scripts");
sys.path.append(basedir+"boards");

import pinutils;

# -----------------------------------------------------------------------------------------

# Now scan AF file
if len(sys.argv)!=3:
  print "ERROR, USAGE: check_flash_usage.py BOARD_NAME espruino_XXX.bin"
  exit(1)
boardname = sys.argv[1]
flashfile = sys.argv[2]
# import the board def
board = importlib.import_module(boardname)

variables = board.info["variables"]
var_size = 16 if variables<255 else 20
var_cache_size = var_size*variables
flash_needed = var_cache_size + 4 # for magic number
flash_page_size = 1024 # massive guess (boards with bigger page sizes generally have enough flash)
flash_pages = (flash_needed+flash_page_size-1)/flash_page_size
total_flash = board.chip["flash"]*1024
remaining_flash = total_flash - flash_pages*flash_page_size
file_size = os.path.getsize(flashfile)

print "Variables = "+str(variables)
print "JsVar size = "+str(var_size)
print "VarCache size = "+str(var_cache_size)
print "Flash pages = "+str(flash_pages)
print "Total flash = "+str(total_flash)
print "Remaining flash = "+str(remaining_flash)
print "File size = "+str(file_size)

if file_size > remaining_flash:
  print "FAIL - Too bit to fit in flash"
  exit(1)

print "PASS"

