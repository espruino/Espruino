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
# Reads board information from boards/BOARDNAME.py and uses it to generate a C file
# describing which peripherals are available on which pins
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
print "Script location "+scriptdir
if len(sys.argv)!=3:
  print "ERROR, USAGE: build_pininfo.py BOARD_NAME PININFO_FILENAME"
  exit(1)
boardname = sys.argv[1]
pininfoFilename = sys.argv[2] 
print "PININFO_FILENAME"+pininfoFilename
print "BOARD "+boardname
# import the board def

board = importlib.import_module(boardname)

# Call the included board_specific file - it sets up 'pins' and 'fill_gaps'
pins = board.get_pins()

#print(json.dumps(pins, sort_keys=True, indent=2))

# -----------------------------------------------------------------------------------------

pininfoFile = open(pininfoFilename, 'w')
def writepininfo(s): pininfoFile.write(s+"\n");

writepininfo("// auto-generated pin info file")
writepininfo("// for board "+boardname)
writepininfo("#include \"jshardware_pininfo.h\"")
writepininfo("")
writepininfo("const int pinInfoCount = "+str(len(pins))+";")
writepininfo("const JshPinInfo pinInfo[] = {")
for pin in pins:
  analog = "JSH_ANALOG_NONE";
  for function in pin["functions"]:
    if function.find("ADC")==0:
      inpos = function.find("_IN")
      adc = function[3:inpos]
      channel = function[inpos+3:]
      analog = "JSH_ANALOG"+adc+"|JSH_ANALOG_CH"+channel;

  functions = [ ]
  for afname in pin["functions"]:
    af = pin["functions"][afname]
    if afname in pinutils.ALLOWED_FUNCTIONS:
      functions.append("JSH_AF"+str(af)+"|"+pinutils.ALLOWED_FUNCTIONS[afname]);

  if len(functions)>pinutils.MAX_ALLOWED_FUNCTIONS:
    print "ERROR: Too many functions for pin "+pin["name"]+" ("+str(len(functions))+" functions)";
    exit(1)
  while len(functions)<pinutils.MAX_ALLOWED_FUNCTIONS: functions.append("0")

  writepininfo("/* "+pin["name"].ljust(4)+" */ { JSH_PORT"+pin["port"]+", JSH_PIN"+pin["num"]+", "+analog+", "+', '.join(functions)+" },")
writepininfo("};")

portinfo = {}

for port in pinutils.ALLOWED_PORTS:
  c=0
  o=-1
  for pin in pins:
    if pin["port"]==port:
      if int(pin["num"])>=c: c = int(pin["num"])+1
      if o<0: o=pins.index(pin)
  portinfo[port] = { 'count' : c, 'offset' : o };
# Olimexino hack as things have been renamed
if boardname=="OLIMEXINO_STM32":
  for port in pinutils.ALLOWED_PORTS:
    if port=="A": portinfo[port] = { 'count' : 16, 'offset' : 15 }
    elif port=="D": portinfo[port] = { 'count' : 39, 'offset' : 0 }
    else: portinfo[port] = { 'count' : 0, 'offset' : -1 }   

for port in pinutils.ALLOWED_PORTS:
  writepininfo("const int JSH_PORT"+port+"_COUNT = "+str(portinfo[port]['count'])+";")
for port in pinutils.ALLOWED_PORTS:
  writepininfo("const int JSH_PORT"+port+"_OFFSET = "+str(portinfo[port]['offset'])+";")


