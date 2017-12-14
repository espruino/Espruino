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
# Script to extract the correct decls from boards/BOARDNAME.py
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
  print("ERROR, USAGE: get_makefile_decls.py BOARD_NAME")
  exit(1)
boardname = sys.argv[1]
# import the board def
board = importlib.import_module(boardname)
#print(json.dumps(board.info))

print("# Generated with scripts/get_makefile_decls.py "+boardname);
print("BOARD="+boardname)

binaryName=common.get_board_binary_name(board)
if binaryName.find('.bin')>=0:
    binaryName = binaryName[:binaryName.find('.bin')]
if binaryName.find('.hex')>=0:
    binaryName = binaryName[:binaryName.find('.hex')]
print("PROJ_NAME="+binaryName)

if board.chip["family"]!="LINUX":
    print("EMBEDDED=1")
print("FAMILY="+board.chip['family'])
print("CHIP="+board.chip['part'])

if "optimizeflags" in board.info["build"]:
    if board.info["build"]["optimizeflags"]:
        print("OPTIMIZEFLAGS+="+board.info["build"]["optimizeflags"])

for lib in board.info["build"]["libraries"]:
    print("USE_"+lib+"=1")

if "makefile" in board.info["build"]:
    for mfLine in board.info["build"]["makefile"]:
        print(mfLine)

if 'USB' in board.devices:
    print("USB:=1")

if 'bootloader' in board.info and board.info['bootloader']==1:
    print("USE_BOOTLOADER:=1")
    print("BOOTLOADER_PROJ_NAME:=bootloader_$(PROJ_NAME)")
    print("DEFINES+=-DUSE_BOOTLOADER")
