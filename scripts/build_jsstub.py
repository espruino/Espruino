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
# Reads JSON schemas from jswrap files and uses it to generate a JS stub
# which is useful for autocomplete
#
# To generate the stubs for Node.js use `build_jssstub.py node`
# ----------------------------------------------------------------------------------------

import sys;
import json;
import common;

# encapsulate output in a comment
print("/*");
schemas = common.get_jsondata(True, False)
print("*/");

# get structured data for further parsing
context = common.get_struct_from_jsondata(schemas)
#print(json.dumps(context, sort_keys=True, indent=2))

def println(line):
  print ("\n" + line)

def getDescComment(obj):
  if not "desc" in obj or len(obj["desc"]) == 0:
    return ''
  val = "/**"
  desc = obj["desc"]
  if not isinstance(desc, list):
    desc = [desc]
  for line in desc:
    val += "\n * " + line
  val += "\n */"
  return val

def getAssignment(objName, funcName, details, isProto = False, isProp = False):
  proto = "prototype." if isProto else ""
  val = "{}" if isProp else "function (){}"
  return objName + ("." if objName else "") + proto + funcName + " = " + val + ";"

def looper(obj, objName, key, isProto = False, isProp = False):
  if key in obj:
    for name in obj[key].keys():
      details = obj[key][name]
      # handle description comment
      desc = getDescComment(details)
      if len(desc) != 0: print(desc)
      # assign without clobbering existing members
      proto = ".prototype" if isProto else ""
      print("if (!('" + name + "' in " + objName + proto + "))")
      print(getAssignment(objName, name, details, isProto, isProp))

def buildJsStubs(forNode = False):
  def export(name):
    if forNode:
      print("module.exports." + name + ' = ' + name + ';')
  ns = "global" if forNode else "window"
  instances = dict()
  for objName in context.keys():
    obj = context[objName]
    if "methods" in obj:
      println(getDescComment(obj))
      # ensure object or constructor is defined w/o clobbering     
      print("if (!('" + objName + "' in " + ns + ")) {")
      if len(obj["methods"]) or len(obj["props"]):
        if "constructor" in obj:
          print(getDescComment(obj["constructor"]))
        print("  " + ns + "." + objName + " = function (){};")
        print("  " + ns + "." + objName + ".prototype = {};")
      else:
        print("  " + ns + "." + objName + " = {};")
      print("}")
      # handle methods and props
      looper(obj, objName, "methods", True)
      looper(obj, objName, "props", True, True)
      looper(obj, objName, "staticmethods", False)
      looper(obj, objName, "staticprops", False)
      export(objName)
    elif "return" in obj:
      println(getDescComment(obj))
      print("if (!('" + objName + "' in " + ns + "))")
      print(ns + "." + objName + " = function (){};")
      export(objName)
    elif "instanceof" in obj:
      instances[objName] = obj

  # make sure instances are defined after all interfaces are present
  for instance in instances.keys():
      println(instance + " = new " + instances[instance]["instanceof"] + "();")
      export(instance)

# determine if output should be for node
try:
  sys.argv.index('node');
except:
  forNode = False
else:
  forNode = True

buildJsStubs(forNode)

