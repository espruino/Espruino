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
import common;
import copy;

scriptdir = os.path.dirname(os.path.realpath(__file__))
basedir = scriptdir+"/../"
sys.path.append(basedir + "scripts");
sys.path.append(basedir + "boards");

import pinutils;

# -----------------------------------------------------------------------------------------

# Now scan AF file
print("Script location " + scriptdir)
if len(sys.argv)!=4:
  print("ERROR, USAGE: build_pininfo.py BOARD_NAME jspininfo.c jspininfo.h")
  exit(1)
boardname = sys.argv[1]
pininfoSourceFilename = sys.argv[2]
pininfoHeaderFilename = sys.argv[3]  
print("PININFO_SOURCE_FILENAME" + pininfoSourceFilename)
print("PININFO_HEADER_FILENAME" + pininfoHeaderFilename)
print("BOARD " + boardname)
# import the board def

board = importlib.import_module(boardname)

# Call the included board_specific file - it sets up 'pins' and 'fill_gaps'
pins = board.get_pins()
pins = pinutils.remove_used_pinfunctions(pins, board)

#print(json.dumps(pins, sort_keys=True, indent=2))

# -----------------------------------------------------------------------------------------

pininfoSourceFile = open(pininfoSourceFilename, 'w')
pininfoHeaderFile = open(pininfoHeaderFilename, 'w')
def writesource(s): pininfoSourceFile.write(s+"\n");
def writeheader(s): pininfoHeaderFile.write(s+"\n");

# -----------------------------------------------------------------------------------------

pinInfoFunctionCount = 0
# Gather some info on available pin functions
usageCount = {}
for pin in pins:
  for afname in pin["functions"]:
    if not afname in usageCount: usageCount[afname] = 0
    usageCount[afname] = usageCount[afname] + 1
# Now treat TIMx_CHy and TIMx_CHyN as the same
for used in usageCount:
  usedNegated = used+"N"
  if usedNegated in usageCount:
    c = usageCount[used] + usageCount[usedNegated]
    usageCount[used] = c; 
    usageCount[usedNegated] = c;

#print(json.dumps(usageCount, sort_keys=True, indent=2))

# work out pin functions (and how many we need)
for pin in pins:
  # Sort pin functions by:
  # How many times they're used (least used go first) and alternate function (functions on the default function get used first)
  sortedFunctions = [];
  for afname in pin["functions"]: sortedFunctions.append(afname)
  sortedFunctions = sorted(sortedFunctions, key=lambda afname: usageCount[afname]*100+pin["functions"][afname]);
  #
  functions = [ ]
  for afname in sortedFunctions:
    af = pin["functions"][afname]
    if afname in pinutils.ALLOWED_FUNCTIONS:
      functions.append("JSH_AF"+str(af)+"|"+pinutils.ALLOWED_FUNCTIONS[afname]+"/* "+str(usageCount[afname])+" Uses */");
  pin["jshFunctions"] = functions
  if len(functions)>pinInfoFunctionCount: pinInfoFunctionCount = len(functions)


writesource("// auto-generated pin info file")
writesource("// for board "+boardname)
writesource("#include \"jspininfo.h\"")
writesource("")
writesource("const JshPinInfo pinInfo[JSH_PIN_COUNT] = {")

for pin in pins:
  analog = "JSH_ANALOG_NONE";
  for function in pin["functions"]:
    if function.find("ADC")==0:
      inpos = function.find("_IN")
      adc = function[3:inpos]
      channel = function[inpos+3:]
      analog = "JSH_ANALOG"+adc+"|JSH_ANALOG_CH"+channel;
  functions = pin["jshFunctions"]
  while len(functions)<pinInfoFunctionCount: functions.append("0")

  writesource("/* "+pin["name"].ljust(4)+" */ { JSH_PORT"+pin["port"]+", JSH_PIN0+"+pin["num"]+", "+analog+", { "+', '.join(functions)+" } },")
writesource("};")

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
if boardname in ["OLIMEXINO_STM32", "OLIMEXINO_STM32_RE", "MAPLERET6_STM32"]:
  for port in pinutils.ALLOWED_PORTS:
    if port=="A": portinfo[port] = { 'count' : 16, 'offset' : 15 }
    elif port=="D": portinfo[port] = { 'count' : 39, 'offset' : 0 }
    else: portinfo[port] = { 'count' : 0, 'offset' : -1 }   

writesource("")


writeheader("// auto-generated pin info file")
writeheader("// for board "+boardname)
writeheader("")
writeheader("#ifndef JSPININFO_H")
writeheader("#define JSPININFO_H")
writeheader("")
writeheader("#include \"jspin.h\"")
writeheader("")
writeheader("#define JSH_PIN_COUNT "+str(len(pins)));
writeheader("")
for port in pinutils.ALLOWED_PORTS:
  writeheader("#define JSH_PORT"+port+"_COUNT "+str(portinfo[port]['count']))
for port in pinutils.ALLOWED_PORTS:
  writeheader("#define JSH_PORT"+port+"_OFFSET "+str(portinfo[port]['offset']))
writeheader("")
writeheader("#define JSH_PININFO_FUNCTIONS "+str(pinInfoFunctionCount))
writeheader("")
writeheader("typedef struct JshPinInfo {")
writeheader("  JsvPinInfoPort port;")
writeheader("  JsvPinInfoPin pin;")
writeheader("  JsvPinInfoAnalog analog; // TODO: maybe we don't need to store analogs separately")
writeheader("  JshPinFunction functions[JSH_PININFO_FUNCTIONS];")
writeheader("} PACKED_FLAGS JshPinInfo;")
writeheader("")
writeheader("extern const JshPinInfo pinInfo[JSH_PIN_COUNT];");
writeheader("")
writeheader("#endif // JSPININFO_H")
