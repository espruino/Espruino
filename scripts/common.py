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
# Reads board information from boards/BOARDNAME.py - used by build_board_docs, 
# build_pininfo, and build_platform_config
# ----------------------------------------------------------------------------------------

import subprocess;
import re;
import json;
import sys;
import os;

silent = os.getenv("SILENT");
if silent:
  class Discarder(object):
    def write(self, text):
        pass # do nothing
  # now discard everything coming out of stdout
  sys.stdout = Discarder()

# Scans files for comments of the form /*JSON......*/ 
# 
# Comments look like:
#
#/*JSON{ "type":"staticmethod|staticproperty|constructor|method|property|function|variable|class|library",
#                      // class = built-in class that does not require instantiation
#                      // library = built-in class that needs require('classname')
#         "class" : "Double", "name" : "doubleToIntBits",
#         "needs_parentName":true,           // optional - if for a method, this makes the first 2 args parent+parentName (not just parent)
#         "generate_full|generate|wrap" : "*(JsVarInt*)&x",
#         "description" : " Convert the floating point value given into an integer representing the bits contained in it",
#         "params" : [ [ "x" , "float|int|JsVar|JsVarName|JsVarArray", "A floating point number"] ],
#                               // float - parses into a JsVarFloat which is passed to the function
#                               // int - parses into a JsVarInt which is passed to the function
#                               // JsVar - passes a JsVar* to the function (after skipping names)
#                               // JsVarName - passes a JsVar* to the function (WITHOUT skipping names)
#                               // JsVarArray - parses this AND ANY SUBSEQUENT ARGUMENTS into a JsVar of type JSV_ARRAY. THIS IS ALWAYS DEFINED, EVEN IF ZERO LENGTH. Currently it must be the only parameter
#         "return" : ["int|float", "The integer representation of x"],
#         "no_create_links":1                // optional - if this is set then hyperlinks are not created when this name is mentioned (good example = bit() )
#         "not_real_object" : "anything",    // optional - for classes, this means we shouldn't treat this as a built-in object, as internally it isn't stored in a JSV_OBJECT
#         "prototype" : "Object",    // optional - for classes, this is what their prototype is. It's particlarly helpful if not_real_object, because there is no prototype var in that case
#         "check" : "jsvIsFoo(var)", // for classes - this is code that returns true if 'var' is of the given type
#         "ifndef" : "SAVE_ON_FLASH", // if the given preprocessor macro is defined, don't implement this
#         "ifdef" : "USE_LCD_FOO", // if the given preprocessor macro isn't defined, don't implement this
#}*/
#
# description can be an array of strings as well as a simple string (in which case each element is separated by a newline),
# and adding ```sometext``` in the description surrounds it with HTML code tags
#


def get_jsondata(is_for_document, parseArgs = True):
        scriptdir = os.path.dirname	(os.path.realpath(__file__))
        print "Script location "+scriptdir
        os.chdir(scriptdir+"/..")

        jswraps = []
        defines = []

        if parseArgs and len(sys.argv)>1:
          print "Using files from command line"
          for i in range(1,len(sys.argv)):
            arg = sys.argv[i]
            if arg[0]=="-":
              if arg[1]=="D": 
                defines.append(arg[2:])
              else:
                print "Unknown command-line option"
                exit(1)
            else:
              jswraps.append(arg)
        else:
          print "Scanning for jswrap.c files"
          jswraps = subprocess.check_output(["find", ".", "-name", "jswrap*.c"]).strip().split("\n")

        if len(defines)>1:
          print "Got #DEFINES:"
          for d in defines: print d

        jsondatas = []
        for jswrap in jswraps:
          # ignore anything from archives
          if jswrap.startswith("./archives/"): continue

          # now scan
          print "Scanning "+jswrap
          code = open(jswrap, "r").read()

          if is_for_document and "DO_NOT_INCLUDE_IN_DOCS" in code: 
            print "FOUND 'DO_NOT_INCLUDE_IN_DOCS' IN FILE "+jswrap
            continue

          for comment in re.findall(r"/\*JSON.*?\*/", code, re.VERBOSE | re.MULTILINE | re.DOTALL):
            jsonstring = comment[6:-2]
#            print "Parsing "+jsonstring
            try:
              jsondata = json.loads(jsonstring)
              jsondata["filename"] = jswrap
              jsondata["include"] = jswrap[:-2]+".h"
              if ("ifndef" in jsondata) and (jsondata["ifndef"] in defines):
                print "Dropped because of #ifndef "+jsondata["ifndef"]
              elif ("ifdef" in jsondata) and not (jsondata["ifdef"] in defines):
                print "Dropped because of #ifdef "+jsondata["ifdef"]
              else:
                jsondatas.append(jsondata)
            except ValueError as e:
              sys.stderr.write( "JSON PARSE FAILED -",  e , "\n")
              exit(1)
            except:
              sys.stderr.write( "JSON PARSE FAILED",  sys.exc_info()[0], "\n" )
              exit(1)
        print "Scanning finished."
        return jsondatas

# Takes the data from get_jsondata and restructures it in prepartion for output as JS
# 
# Results look like:, 
#{
#  "Pin": {
#    "desc": [
#      "This is the built-in class for Pins, such as D0,D1,LED1, or BTN", 
#      "You can call the methods on Pin, or you can use Wiring-style functions such as digitalWrite"
#    ], 
#    "methods": {
#      "read": {
#        "desc": "Returns the input state of the pin as a boolean", 
#        "params": [], 
#        "return": [
#          "bool", 
#          "Whether pin is a logical 1 or 0"
#        ]
#      }, 
#      "reset": {
#        "desc": "Sets the output state of the pin to a 0", 
#        "params": [], 
#        "return": []
#      }, 
#      ...
#    }, 
#    "props": {}, 
#    "staticmethods": {}, 
#    "staticprops": {}
#  },
#  "print": {
#    "desc": "Print the supplied string", 
#    "return": []
#  },
#  ...
#}
#

def get_struct_from_jsondata(jsondata):
  context = {"modules": {}}

  def checkClass(details):
    cl = details["class"]
    if not cl in context:
      context[cl] = {"type": "class", "methods": {}, "props": {}, "staticmethods": {}, "staticprops": {}, "desc": details.get("description", "")}
    return cl

  def addConstructor(details):
    cl = checkClass(details)
    context[cl]["constructor"] = {"params": details.get("params", []), "return": details.get("return", []), "desc": details.get("description", "")}

  def addMethod(details, type = ""):
    cl = checkClass(details)
    context[cl][type + "methods"][details["name"]] = {"params": details.get("params", []), "return": details.get("return", []), "desc": details.get("description", "")}

  def addProp(details, type = ""):
    cl = checkClass(details)
    context[cl][type + "props"][details["name"]] = {"return": details.get("return", []), "desc": details.get("description", "")}

  def addFunc(details):
    context[details["name"]] = {"type": "function", "return": details.get("return", []), "desc": details.get("description", "")}

  def addObj(details):
    context[details["name"]] = {"type": "object", "instanceof": details.get("instanceof", ""), "desc": details.get("description", "")}

  def addLib(details):
    context["modules"][details["class"]] = {"desc": details.get("description", "")}

  def addVar(details):
    return

  for data in jsondata:
    type = data["type"]
    if type=="class":
      checkClass(data)
    elif type=="constructor":
      addConstructor(data)
    elif type=="method":
      addMethod(data)
    elif type=="property":
      addProp(data)
    elif type=="staticmethod":
      addMethod(data, "static")
    elif type=="staticproperty":
      addProp(data, "static")
    elif type=="function":
      addFunc(data)
    elif type=="object":
      addObj(data)
    elif type=="library":
      addLib(data)
    elif type=="variable":
      addVar(data)
    else:
      print(json.dumps(data, sort_keys=True, indent=2))

  return context

def get_includes_from_jsondata(jsondatas):
        includes = []
        for jsondata in jsondatas:
          include = jsondata["include"]
          if not include in includes:
                includes.append(include)
        return includes

def is_property(jsondata):
  return jsondata["type"]=="property" or jsondata["type"]=="staticproperty"

def get_script_dir():
        return os.path.dirname(os.path.realpath(__file__))
def get_version():
        scriptdir = get_script_dir()
        jsutils = scriptdir+"/../src/jsutils.h"
        return subprocess.check_output(["sed", "-ne", "s/^.*JS_VERSION.*\"\(.*\)\"/\\1/p", jsutils]).strip()

def get_name_or_space(jsondata):
        if "name" in jsondata: return jsondata["name"]
        return ""

def get_bootloader_size():
        return 10*1024;

def get_board_binary_name(board):
        return board.info["binary_name"].replace("%v", get_version());
