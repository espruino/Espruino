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

import common;
import pinutils;

# -----------------------------------------------------------------------------------------

# Now scan AF file
if len(sys.argv)!=2:
  print "ERROR, USAGE: get_board_name.py BOARD_NAME"
  exit(1)
boardname = sys.argv[1]
# import the board def
board = importlib.import_module(boardname)
print common.get_board_binary_name(board)
