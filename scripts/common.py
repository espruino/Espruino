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

# Global
import subprocess;
import re;
import json;
import sys;
import os;
import importlib;
# Local
import pinutils;

silent = os.getenv("SILENT");
if silent:
  class Discarder(object):
    def write(self, text):
        pass # do nothing
  # now discard everything coming out of stdout
  sys.stdout = Discarder()

# http://stackoverflow.com/questions/4814970/subprocess-check-output-doesnt-seem-to-exist-python-2-6-5
if "check_output" not in dir( subprocess ):
    def f(*popenargs, **kwargs):
        if 'stdout' in kwargs:
            raise ValueError('stdout argument not allowed, it will be overridden.')
        process = subprocess.Popen(stdout=subprocess.PIPE, *popenargs, **kwargs)
        output, unused_err = process.communicate()
        retcode = process.poll()
        if retcode:
            cmd = kwargs.get("args")
            if cmd is None:
                cmd = popenargs[0]
            raise subprocess.CalledProcessError(retcode, cmd)
        return output
    subprocess.check_output = f


# Scans files for comments of the form /*JSON......*/
#
# Comments look like:
#
#/*JSON{ "type":"staticmethod|staticproperty|constructor|method|property|function|variable|class|library|idle|init|kill",
#                      // class = built-in class that does not require instantiation
#                      // library = built-in class that needs require('classname')
#                      // idle = function to run on idle regardless
#                      // init = function to run on initialisation
#                      // kill = function to run on deinitialisation
#         "class" : "Double", "name" : "doubleToIntBits",
#         "needs_parentName":true,           // optional - if for a method, this makes the first 2 args parent+parentName (not just parent)
#         "generate_full|generate|wrap" : "*(JsVarInt*)&x",
#         "description" : " Convert the floating point value given into an integer representing the bits contained in it",
#         "params" : [ [ "x" , "float|int|int32|bool|pin|JsVar|JsVarName|JsVarArray", "A floating point number"] ],
#                               // float - parses into a JsVarFloat which is passed to the function
#                               // int - parses into a JsVarInt which is passed to the function
#                               // int32 - parses into a 32 bit int
#                               // bool - parses into a boolean
#                               // pin - parses into a pin
#                               // JsVar - passes a JsVar* to the function (after skipping names)
#                               // JsVarArray - parses this AND ANY SUBSEQUENT ARGUMENTS into a JsVar of type JSV_ARRAY. THIS IS ALWAYS DEFINED, EVEN IF ZERO LENGTH. Currently it must be the only parameter
#         "return" : ["int|float|JsVar", "The integer representation of x"],
#         "return_object" : "ObjectName", // optional - used for tern's code analysis - so for example we can do hints for openFile(...).yyy
#         "no_create_links":1                // optional - if this is set then hyperlinks are not created when this name is mentioned (good example = bit() )
#         "not_real_object" : "anything",    // optional - for classes, this means we shouldn't treat this as a built-in object, as internally it isn't stored in a JSV_OBJECT
#         "prototype" : "Object",    // optional - for classes, this is what their prototype is. It's particlarly helpful if not_real_object, because there is no prototype var in that case
#         "check" : "jsvIsFoo(var)", // for classes - this is code that returns true if 'var' is of the given type
#         "ifndef" : "SAVE_ON_FLASH", // if the given preprocessor macro is defined, don't implement this
#         "ifdef" : "USE_LCD_FOO", // if the given preprocessor macro isn't defined, don't implement this
#         "#if" : "A>2", // add a #if statement in the generated C file (ONLY if type==object)
#}*/
#
# description can be an array of strings as well as a simple string (in which case each element is separated by a newline),
# and adding ```sometext``` in the description surrounds it with HTML code tags
#


def get_jsondata(is_for_document, parseArgs = True, board = False):
    scriptdir = os.path.dirname	(os.path.realpath(__file__))
    print("Script location "+scriptdir)
    os.chdir(scriptdir+"/..")

    jswraps = []
    defines = []

    if board and ("build" in board.info)  and ("defines" in board.info["build"]):
        for i in board.info["build"]["defines"]:
          print("Got define from board: " + i);
          defines.append(i)

    if parseArgs and len(sys.argv)>1:
      print("Using files from command line")
      for i in range(1,len(sys.argv)):
        arg = sys.argv[i]
        if arg[0]=="-":
          if arg[1]=="D":
            defines.append(arg[2:])
          elif arg[1]=="B":
            board = importlib.import_module(arg[2:])
            if "usart" in board.chip: defines.append("USART_COUNT="+str(board.chip["usart"]));
            if "spi" in board.chip: defines.append("SPI_COUNT="+str(board.chip["spi"]));
            if "i2c" in board.chip: defines.append("I2C_COUNT="+str(board.chip["i2c"]));
            if "USB" in board.devices: defines.append("defined(USB)=True");
            else: defines.append("defined(USB)=False");
          elif arg[1]=="F":
            "" # -Fxxx.yy in args is filename xxx.yy, which is mandatory for build_jswrapper.py
          else:
            print("Unknown command-line option")
            exit(1)
        else:
          jswraps.append(arg)
    else:
      print("Scanning for jswrap.c files")
      jswraps = subprocess.check_output(["find", ".", "-name", "jswrap*.c"]).strip().split("\n")

    if len(defines)>1:
      print("Got #DEFINES:")
      for d in defines: print("   "+d)

    jsondatas = []
    for jswrap in jswraps:
      # ignore anything from archives
      if jswrap.startswith("./archives/"): continue

      # now scan
      print("Scanning "+jswrap)
      code = open(jswrap, "r").read()

      if is_for_document and "DO_NOT_INCLUDE_IN_DOCS" in code:
        print("FOUND 'DO_NOT_INCLUDE_IN_DOCS' IN FILE "+jswrap)
        continue

      for comment in re.findall(r"/\*JSON.*?\*/", code, re.VERBOSE | re.MULTILINE | re.DOTALL):
        charnumber = code.find(comment)
        linenumber = 1+code.count("\n", 0, charnumber)
        # Strip off /*JSON .. */ bit
        comment = comment[6:-2]

        endOfJson = comment.find("\n}")+2;
        jsonstring = comment[0:endOfJson];
        description =  comment[endOfJson:].strip();
#            print("Parsing "+jsonstring)
        try:
          jsondata = json.loads(jsonstring)
          if len(description): jsondata["description"] = description;
          jsondata["filename"] = jswrap
          jsondata["include"] = jswrap[:-2]+".h"
          jsondata["githublink"] = "https://github.com/espruino/Espruino/blob/master/"+jswrap+"#L"+str(linenumber)

          dropped_prefix = "Dropped "
          if "name" in jsondata: dropped_prefix += jsondata["name"]+" "
          elif "class" in jsondata: dropped_prefix += jsondata["class"]+" "
          drop = False
          if not is_for_document:
            if ("ifndef" in jsondata) and (jsondata["ifndef"] in defines):
              print(dropped_prefix+" because of #ifndef "+jsondata["ifndef"])
              drop = True
            if ("ifdef" in jsondata) and not (jsondata["ifdef"] in defines):
              print(dropped_prefix+" because of #ifdef "+jsondata["ifdef"])
              drop = True
            if ("#ifdef" in jsondata) or ("#ifndef" in jsondata):
              sys.stderr.write( "'#ifdef' where 'ifdef' should be used in " + jsonstring + " - "+str(sys.exc_info()[0]) + "\n" )
              exit(1)
            if ("#if" in jsondata):
              expr = jsondata["#if"]
              for defn in defines:
                if defn.find('=')!=-1:
                  dname = defn[:defn.find('=')]
                  dkey = defn[defn.find('=')+1:]
                  expr = expr.replace(dname, dkey);
              try:
                r = eval(expr)
              except:
                print("WARNING: error evaluating '"+expr+"' - from '"+jsondata["#if"]+"'")
                r = True
              if not r:
                print(dropped_prefix+" because of #if "+jsondata["#if"]+ " -> "+expr)
                drop = True
          if not drop:
            jsondatas.append(jsondata)
        except ValueError as e:
          sys.stderr.write( "JSON PARSE FAILED for " +  jsonstring + " - "+ str(e) + "\n")
          exit(1)
        except:
          sys.stderr.write( "JSON PARSE FAILED for " + jsonstring + " - "+str(sys.exc_info()[0]) + "\n" )
          exit(1)
    print("Scanning finished.")

    if board:
      for device in pinutils.SIMPLE_DEVICES:
        if device in board.devices:
          jsondatas.append({
            "type" : "variable",
            "name" : device,
            "generate_full" : device+"_PININDEX",
            "return" : ["pin", device],
            "filename" : "BOARD.py",
            "include" : "platform_config.h"
          })
      if "LED1" in board.devices:
        jsondatas.append({
          "type" : "variable",
          "name" : "LED",
          "generate_full" : "LED1_PININDEX",
          "return" : ["pin", "LED1"],
          "filename" : "BOARD.py",
          "include" : "platform_config.h"
        })
      if "BTN1" in board.devices:
        jsondatas.append({
          "type" : "variable",
          "name" : "BTN",
          "generate_full" : "BTN1_PININDEX",
          "return" : ["pin", "Button 1"],
          "filename" : "BOARD.py",
          "include" : "platform_config.h"
        })

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
  return jsondata["type"]=="property" or jsondata["type"]=="staticproperty" or jsondata["type"]=="variable"

def is_function(jsondata):
  return jsondata["type"]=="function" or jsondata["type"]=="method"

def get_prefix_name(jsondata):
  if jsondata["type"]=="event": return "event"
  if jsondata["type"]=="constructor": return "constructor"
  if jsondata["type"]=="function": return "function"
  if jsondata["type"]=="method": return "function"
  if jsondata["type"]=="variable": return "variable"
  if jsondata["type"]=="property": return "property"
  return ""

def get_ifdef_description(d):
  if d=="SAVE_ON_FLASH": return "devices with low flash memory"
  if d=="STM32F1": return "STM32F1 devices (including Original Espruino Board)"
  if d=="NRF52": return "NRF52 devices (like Puck.js)"
  if d=="USE_LCD_SDL": return "Linux with SDL support compiled in"
  if d=="USE_TLS": return "devices with TLS and SSL support (Espruino Pico and Espruino WiFi only)"
  if d=="RELEASE": return "release builds"
  if d=="LINUX": return "Linux-based builds"
  if d=="BLUETOOTH": return "devices with Bluetooth LE capability"
  if d=="USB": return "devices with USB"
  if d=="USE_USB_HID": return "devices that support USB HID (Espruino Pico and Espruino WiFi)"
  if d=="USE_AES": return "devices that support AES (Espruino Pico, Espruino WiFi or Linux)"
  if d=="USE_CRYPTO": return "devices that support Crypto Functionality (Espruino Pico, Espruino WiFi, Linux or ESP8266)"
  if d=="USE_FLASHFS": return "devices with filesystem in Flash support enabled (ESP32 only)"
  print("WARNING: Unknown ifdef '"+d+"' in common.get_ifdef_description")
  return d

def get_script_dir():
        return os.path.dirname(os.path.realpath(__file__))

def get_version():
        # Warning: the same release label derivation is also in the Makefile
        scriptdir = get_script_dir()
        jsutils = scriptdir+"/../src/jsutils.h"
        version = re.compile("^.*JS_VERSION.*\"(.*)\"");
        alt_release = os.getenv("ALT_RELEASE")
        if alt_release == None:
          # Default release labeling based on commits since last release tag
          latest_release = subprocess.check_output('git tag | grep RELEASE_ | sort | tail -1', shell=True).strip()
          commits_since_release = subprocess.check_output('git log --oneline '+latest_release.decode("utf-8")+'..HEAD | wc -l', shell=True).decode("utf-8").strip()
        else:
          # Alternate release labeling with fork name (in ALT_RELEASE env var) plus branch
          # name plus commit SHA
          sha = subprocess.check_output('git rev-parse --short HEAD', shell=True).strip()
          branch = subprocess.check_output('git name-rev --name-only HEAD', shell=True).strip()
          commits_since_release = alt_release + '_' + branch + '_' + sha
        for line in open(jsutils):
            match = version.search(line);
            if (match != None):
                v = match.group(1);
                if commits_since_release=="0": return v
                else: return v+"."+commits_since_release
        return "UNKNOWN"


def get_name_or_space(jsondata):
        if "name" in jsondata: return jsondata["name"]
        return ""

def get_bootloader_size(board):
        if board.chip["family"]=="STM32F4": return 16*1024; # 16kb Pages, so we have no choice
        return 10*1024;

# On normal chips this is 0x00000000
# On boards with bootloaders it's generally + 10240
# On F401, because of the setup of pages we put the bootloader in the first 16k, then in the 16+16+16 we put the saved code, and then finally we but the binary somewhere else
def get_espruino_binary_address(board):
        if "place_text_section" in board.chip:
          return board.chip["place_text_section"]
        if "bootloader" in board.info and board.info["bootloader"]==1:
          return get_bootloader_size(board);
        return 0;

def get_board_binary_name(board):
        return board.info["binary_name"].replace("%v", get_version());
