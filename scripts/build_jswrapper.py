#!/usr/bin/env python3

# This file is part of Espruino, a JavaScript interpreter for Microcontrollers
#
# Copyright (C) 2013 Gordon Williams <gw@pur3.co.uk>
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# ----------------------------------------------------------------------------------------
# Scans files for comments of the form /*JSON......*/ and then builds a tree structure of ifs to
# efficiently detect the symbols without using RAM. See common.py for formatting
# ----------------------------------------------------------------------------------------

import subprocess;
import re;
import json;
import sys;
import os;
scriptdir = os.path.dirname(os.path.realpath(__file__))
basedir = scriptdir+"/../"
sys.path.append(basedir+"scripts");
sys.path.append(basedir+"boards");
import importlib;
import common;
from collections import OrderedDict;

if len(sys.argv)<2 or sys.argv[len(sys.argv)-2][:2]!="-B" or sys.argv[len(sys.argv)-1][:2]!="-F":
	print("USAGE: build_jswrapper.py ... -BBOARD -Fwrapperfile.c modulename:path/to/modulesource.js")
	print("")
	print("           -Fwrapperfile.c                     ; include a jswrap file in the build ")
	print("")
	print("           path/to/modulename.js               ; include a JS module called modulename")
	print("           modulename:path/to/modulesource.js  ; include a JS module called modulename")
	print("           _:bootcode                          ; JS code to be executed at boot time")
	print("             ; These can be specified in the JSMODULESOURCES environment variable")
	exit(1)

boardName = sys.argv[len(sys.argv)-2]
boardName = boardName[2:]

wrapperFileName = sys.argv[len(sys.argv)-1]
wrapperFileName = wrapperFileName[2:]

# Load any JS modules specified on command-line
jsmodules = {}     # JS modules to be included
jsbootcode = False # if set, JS code to be run at boot time
for i in range(1,len(sys.argv)):
  arg = sys.argv[i]
  if arg[0]!="-" and arg[-3:]==".js":
    if arg.find(":")>=0:
      colon = arg.find(":")
      modulename = arg[:colon]
      arg = arg[colon+1:]
    else:
      modulename = arg.rsplit('/',1)[1][:-3]
      if modulename[-4:]==".min": modulename=modulename[:-4]
    print("Loading JS module: "+arg+" -> "+modulename)
    jscode = open(arg, "r").read()
    if modulename=="_":
      jsbootcode = jscode
    else:
      jsmodules[modulename] = jscode

# List of argument specifiers (JSWAT...) that have been used
argSpecs = []
# for each argument specifier, the code required
argSpecCalls = []

# ------------------------------------------------------------------------------------------------------

def codeOut(s):
#  print str(s)
  wrapperFile.write(s+"\n");

def FATAL_ERROR(s):
  sys.stderr.write("ERROR: "+s)
  exit(1)

# ------------------------------------------------------------------------------------------------------

def getConstructorTestFor(className, variableName):
    # IMPORTANT - we expect built-in objects to be native functions with a pointer to
    # their constructor function inside
    for jsondata in jsondatas:
      if jsondata["type"]=="constructor" and jsondata["name"]==className:
        if variableName=="constructorPtr": # jsvIsNativeFunction/etc has already been done
          return "constructorPtr==(void*)"+jsondata["generate"];
        else:
          return "jsvIsNativeFunction("+variableName+") && (void*)"+variableName+"->varData.native.ptr==(void*)"+jsondata["generate"];
    print("No constructor found for "+className)
    exit(1)

def getTestFor(className, static):
  if static:
    return getConstructorTestFor(className, "parent");
  else:
    if className=="String": return "jsvIsString(parent)"
    if className=="Pin": return "jsvIsPin(parent)"
    if className=="Integer": return "jsvIsInt(parent)"
    if className=="Double": return "jsvIsFloat(parent)"
    if className=="Number": return "jsvIsNumeric(parent)"
    if className=="Object": return "parent" # we assume all are objects
    if className=="Array": return "jsvIsArray(parent)"
    if className=="ArrayBuffer": return "jsvIsArrayBuffer(parent) && parent->varData.arraybuffer.type==ARRAYBUFFERVIEW_ARRAYBUFFER"
    if className=="ArrayBufferView": return "jsvIsArrayBuffer(parent) && parent->varData.arraybuffer.type!=ARRAYBUFFERVIEW_ARRAYBUFFER"
    if className=="Function": return "jsvIsFunction(parent)"
    return getConstructorTestFor(className, "constructorPtr");

def toArgumentType(argName):
  if argName=="": return "JSWAT_VOID";
  if argName=="JsVar": return "JSWAT_JSVAR";
  if argName=="JsVarArray": return "JSWAT_ARGUMENT_ARRAY";
  if argName=="bool": return "JSWAT_BOOL";
  if argName=="pin": return "JSWAT_PIN";
  if argName=="int32": return "JSWAT_INT32";
  if argName=="int": return "JSWAT_INT32";
  if argName=="float": return "JSWAT_JSVARFLOAT";
  FATAL_ERROR("toArgumentType: Unknown argument name "+argName+"\n")

def toCType(argName):
  if argName=="": return "void";
  if argName=="JsVar": return "JsVar*";
  if argName=="JsVarArray": return "JsVar*";
  if argName=="bool": return "bool";
  if argName=="pin": return "Pin";
  if argName=="int32": return "int";
  if argName=="int": return "JsVarInt";
  if argName=="float": return "JsVarFloat";
  FATAL_ERROR("toCType: Unknown argument name "+argName+"\n")

def toCBox(argName):
  if argName=="JsVar": return "";
  if argName=="bool": return "jsvNewFromBool";
  if argName=="pin": return "jsvNewFromPin";
  if argName=="int32": return "jsvNewFromInteger";
  if argName=="int": return "jsvNewFromInteger";
  if argName=="float": return "jsvNewFromFloat";
  FATAL_ERROR("toCBox: Unknown argument name "+argName+"\n")

def toCUnbox(argName):
  if argName=="JsVar": return "";
  if argName=="bool": return "jsvGetBool";
  if argName=="pin": return "jshGetPinFromVar";
  if argName=="int32": return "jsvGetInteger";
  if argName=="int": return "jsvGetInteger";
  if argName=="float": return "jsvGetFloat";
  FATAL_ERROR("toCUnbox: Unknown argument name "+argName+"\n")


def hasThis(func):
  return func["type"]=="property" or func["type"]=="method"

def getParams(func):
  params = []
  if "params" in func:
    for param in func["params"]:
      params.append(param)
  return params

def getResult(func):
  result = [ "", "Description" ]
  if "return" in func: result = func["return"]
  return result

def getArgumentSpecifier(jsondata):
  params = getParams(jsondata)
  result = getResult(jsondata);
  s = [ toArgumentType(result[0]) ]
  if hasThis(jsondata): s.append("JSWAT_THIS_ARG");
  # Either it's a variable/property, in which case we need to execute the native function in order to return the correct info
  if jsondata["type"]=="variable" or common.is_property(jsondata):
    s.append("JSWAT_EXECUTE_IMMEDIATELY")
  # Or it's an object, in which case the native function contains code that creates it - and that must be executed too.
  # It also returns JsVar
  if jsondata["type"]=="object":
    s = [ toArgumentType("JsVar"), "JSWAT_EXECUTE_IMMEDIATELY" ]

  n = 1
  for param in params:
    s.append("("+toArgumentType(param[1])+" << (JSWAT_BITS*"+str(n)+"))");
    n=n+1
  if n>5:
    sys.stderr.write(json.dumps(jsondata, sort_keys=True, indent=2)+"\n")
    FATAL_ERROR("getArgumentSpecifier: Too many arguments to fit in type specifier, Use JsVarArray\n")

  argSpec = " | ".join(s);
  return argSpec

def getCDeclaration(jsondata, name):
  # name could be '(*)' for a C function pointer
  params = getParams(jsondata)
  result = getResult(jsondata);
  s = [ ]
  if hasThis(jsondata): s.append("JsVar*");
  for param in params:
    s.append(toCType(param[1]));
  return toCType(result[0])+" "+name+"("+",".join(s)+")";

def codeOutSymbolTable(builtin):
  codeName = builtin["name"]
  # sort by name
  builtin["functions"] = sorted(builtin["functions"], key=lambda n: n["name"]);
  # output tables
  listSymbols = []
  listChars = ""
  strLen = 0
  for sym in builtin["functions"]:
    symName = sym["name"];

    if builtin["name"]=="global" and symName in libraries:
      continue # don't include libraries on global namespace
    if "generate" in sym:
      listSymbols.append("{"+", ".join([str(strLen), getArgumentSpecifier(sym), "(void (*)(void))"+sym["generate"]])+"}")
      listChars = listChars + symName + "\\0";
      strLen = strLen + len(symName) + 1
    else:
      print (codeName + "." + symName+" not included in Symbol Table because no 'generate'")
  builtin["symbolTableChars"] = "\""+listChars+"\"";
  builtin["symbolTableCount"] = str(len(listSymbols));
  codeOut("static const JswSymPtr jswSymbols_"+codeName+"[] FLASH_SECT = {\n  "+",\n  ".join(listSymbols)+"\n};");

def codeOutBuiltins(indent, builtin):
  codeOut(indent+"jswBinarySearch(&jswSymbolTables["+builtin["indexName"]+"], parent, name);");

#================== to remove JS-definitions given by blacklist==============
def delete_by_indices(lst, indices):
    indices_as_set = set(indices)
    return [ lst[i] for i in range(len(lst)) if i not in indices_as_set ]

def removeBlacklistForWrapper(blacklistfile,datas):
	json_File = open(blacklistfile,'r')
	blacklist = json.load(json_File)
	toremove = []
	for idx,jsondata in enumerate(datas):
		if "class" in jsondata:
			if "name" in jsondata:
				for black in blacklist:
					if jsondata["class"] == black["class"]:
						if(jsondata["name"] == black["name"] or black["name"] == "*"):
							toremove.append(idx)
							print("Removing "+black["class"]+"."+black["name"]+" due to blacklist wildcard")
# extension by jumjum
		else:
			if "name" in jsondata:
				for black in blacklist:
					if black["class"] == "__":
						if jsondata["name"] == black["name"]:
						  toremove.append(idx)
						  print("Removing global."+black["name"]+" due to blacklist wildcard")
		if "type" in jsondata:
			if "class" in jsondata:
				for black in blacklist:
					if jsondata["class"] == black["class"]:
						if black["name"] == "*":
						  toremove.append(idx)
						  print("Removing "+black["class"]+" due to blacklist wildcard")
			if "instanceof" in jsondata:
				for black in blacklist:
					if jsondata["instanceof"] == black["class"]:
						if black["name"] == "*":
						  toremove.append(idx)
						  print("Removing "+black["class"]+" due to blacklist wildcard")

#  end extension by jumjum
	return delete_by_indices( datas, toremove)
# ------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------

print("BOARD "+boardName)
board = importlib.import_module(boardName)

jsondatas = common.get_jsondata(is_for_document = False, parseArgs = True, boardObject = board)
if 'BLACKLIST' in os.environ:
	jsondatas = removeBlacklistForWrapper(os.environ['BLACKLIST'],jsondatas)

includes = common.get_includes_from_jsondata(jsondatas)

# work out what we have actually got
classes = []
constructors = []
for jsondata in jsondatas:
  if "class" in jsondata :
    if not jsondata["class"] in classes:
      classes.append(jsondata["class"])
  if jsondata["type"]=="constructor":
    if not jsondata["name"] in constructors:
      constructors.append(jsondata["name"])
    if not "return" in jsondata:
      FATAL_ERROR("Constructors MUST return something: "+jsondata["name"]+":"+jsondata["filename"]+"\n");

# Add constructors if we need them
for className in classes:
  if not className in constructors:
    jsondatas.append({
        "type":"constructor", "class": className,  "name": className,
        "generate_full" : "NULL", # return undefined means 'new X' uses the object that was returned, which is correct
        "filename" : "jswrapper.c",
        "return" : [ "JsVar", "" ]
    });

# ------------------------------------------------------------------------------------------------------
#print(json.dumps(tree, sort_keys=True, indent=2))
# ------------------------------------------------------------------------------------------------------

wrapperFile = open(wrapperFileName,'w')

codeOut('// Automatically generated wrapper file ')
codeOut('// Generated by scripts/build_jswrapper.py');
codeOut('');
codeOut('#include "jswrapper.h"');
codeOut('#include "jsnative.h"');
codeOut('#include "jsparse.h"');
for include in includes:
  codeOut('#include "'+include+'"');
codeOut('');
codeOut('');

codeOut('// -----------------------------------------------------------------------------------------');
codeOut('// ----------------------------------------------------------------- AUTO-GENERATED WRAPPERS');
codeOut('// -----------------------------------------------------------------------------------------');
codeOut('');

for jsondata in jsondatas:
  # Include 'inline' C declarations
  if ("generate_full" in jsondata) or (jsondata["type"]=="object"):
    gen_name = "gen_jswrap"
    if "class" in jsondata: gen_name = gen_name + "_" + jsondata["class"];
    gen_name = gen_name + "_" + jsondata["name"];
    jsondata["generate"] = gen_name

    s = [ ]
    if jsondata["type"]=="object":
      jsondata["generate_full"] = "jspNewObject(\""+jsondata["name"]+"\", \""+jsondata["instanceof"]+"\") /* needs JSWAT_EXECUTE_IMMEDIATELY */";
      params = []
      result = ["JsVar"]
    else:
      params = getParams(jsondata)
      result = getResult(jsondata);
      if hasThis(jsondata): s.append("JsVar *parent");
      for param in params:
        s.append(toCType(param[1])+" "+param[0]);

    codeOut("static "+toCType(result[0])+" "+jsondata["generate"]+"("+", ".join(s)+") {");
    if result[0]:
      codeOut("  return "+jsondata["generate_full"]+";");
    else:
      codeOut("  "+jsondata["generate_full"]+";");
    codeOut("}");
    codeOut('');
  # Include JavaScript functions
  if ("generate_js" in jsondata):
    gen_name = "gen_jswrap"
    if "class" in jsondata: gen_name = gen_name + "_" + jsondata["class"];
    gen_name = gen_name + "_" + jsondata["name"];
    jsondata["generate"] = gen_name

    s = [ ]
    params = getParams(jsondata)
    result = getResult(jsondata)
    if hasThis(jsondata): s.append("JsVar *parent")
    for param in params:
      if param[1]!="JsVar": FATAL_ERROR("All arguments to generate_js must be JsVars");
      s.append(toCType(param[1])+" "+param[0]);

    js = "";
    with open(basedir+jsondata["generate_js"], 'r') as file:
      js = file.read().strip()
    statement = "jspExecuteJSFunction("+json.dumps(js)
    if hasThis(jsondata): statement = statement + ", parent"
    else: statement = statement + ", NULL"

    codeOut("static "+toCType(result[0])+" "+jsondata["generate"]+"("+", ".join(s)+") {")
    if len(params):
      codeOut("  JsVar *args[] = {");
      for param in params:
        codeOut("    "+param[0]+",")
      codeOut("  };")
      statement = statement + ","+str(len(params))+", args)"
    else: # no args
      statement = statement + ",0,NULL)"

    if result[0]:
      if result[0]!="JsVar": FATAL_ERROR("All arguments to generate_js must be JsVars");
      codeOut("  return "+statement+";")
    else:
      codeOut("  jsvUnLock("+statement+");")
    codeOut("}");
    codeOut('');

codeOut('// -----------------------------------------------------------------------------------------');
codeOut('// -----------------------------------------------------------------------------------------');
codeOut('// -----------------------------------------------------------------------------------------');
codeOut('');


# In jswBinarySearch we used to use READ_FLASH_UINT16 for sym->strOffset and sym->functionSpec for ESP8266
# (where unaligned reads broke) but despite being packed, the structure JswSymPtr is still always an multiple
# of 2 in length so they will always be halfword aligned.
codeOut("""
// Binary search coded to allow for JswSyms to be in flash on the esp8266 where they require
// word accesses
JsVar *jswBinarySearch(const JswSymList *symbolsPtr, JsVar *parent, const char *name) {
  uint8_t symbolCount = READ_FLASH_UINT8(&symbolsPtr->symbolCount);
  int searchMin = 0;
  int searchMax = symbolCount - 1;
  while (searchMin <= searchMax) {
    int idx = (searchMin+searchMax) >> 1;
    const JswSymPtr *sym = &symbolsPtr->symbols[idx];
    int cmp = FLASH_STRCMP(name, &symbolsPtr->symbolChars[sym->strOffset]);
    if (cmp==0) {
      unsigned short functionSpec = sym->functionSpec;
      if ((functionSpec & JSWAT_EXECUTE_IMMEDIATELY_MASK) == JSWAT_EXECUTE_IMMEDIATELY)
        return jsnCallFunction(sym->functionPtr, functionSpec, parent, 0, 0);
      return jsvNewNativeFunction(sym->functionPtr, functionSpec);
    } else {
      if (cmp<0) {
        // searchMin is the same
        searchMax = idx-1;
      } else {
        searchMin = idx+1;
        // searchMax is the same
      }
    }
  }
  return 0;
}

""");

codeOut('// -----------------------------------------------------------------------------------------');
codeOut('// -----------------------------------------------------------------------------------------');
codeOut('// -----------------------------------------------------------------------------------------');
codeOut('');
codeOut('');

print("Finding Libraries")
libraries = []
for jsondata in jsondatas:
  if jsondata["type"]=="library":
    print("Found library "+jsondata["class"])
    libraries.append(jsondata["class"])

print("Classifying Functions")
builtins = OrderedDict()
for jsondata in jsondatas:
  if "name" in jsondata:
    #print(json.dumps(jsondata, sort_keys=True, indent=2))
    jsondata["static"] = not (jsondata["type"]=="property" or jsondata["type"]=="method")

    testCode = "!parent"
    builtinName = "global"
    className = "global"
    isProto = False
    if not jsondata["type"]=="constructor":
      if "class" in jsondata:
        testCode = getTestFor(jsondata["class"], jsondata["static"])
        className = jsondata["class"]
        builtinName = className
        if not jsondata["static"]:
          isProto = True
          builtinName = builtinName+"_proto";

    if not testCode in builtins:
      print("Adding "+testCode+" to builtins ("+className+")")
      builtins[testCode] = { "name" : builtinName, "className" : className, "isProto" : isProto, "functions" : [] }
    builtins[testCode]["functions"].append(jsondata);

# For the ESP8266 we want to put the structures into flash, we need a fresh section 'cause the
# .irom.literal section used elsewhere has different readability attributes, sigh
codeOut("#ifdef ESP8266\n#define FLASH_SECT __attribute__((section(\".irom.literal2\"))) __attribute__((aligned(4)))");
codeOut("#else\n#define FLASH_SECT\n#endif\n");

print("Outputting Symbol Tables")
idx = 0
for b in builtins:
  builtin = builtins[b]
  codeOutSymbolTable(builtin);
  builtins[b]["indexName"] = "jswSymbolIndex_"+builtin["name"];
  codeOut("static const unsigned char "+builtin["indexName"]+" = "+str(idx)+";");
  idx = idx + 1
codeOut('');
codeOut('');

# output the strings, possibly with __attribute__ to put them into flash
for b in builtins:
  builtin = builtins[b]
  codeOut("FLASH_STR(jswSymbols_"+builtin["name"]+"_str, " + builtin["symbolTableChars"] +");");
codeOut('');
# output the symbol table array referencing the above strings
codeOut('const JswSymList jswSymbolTables[] FLASH_SECT = {');
for b in builtins:
  builtin = builtins[b]
  codeOut("  {"+", ".join(["jswSymbols_"+builtin["name"], "jswSymbols_"+builtin["name"]+"_str", builtin["symbolTableCount"]])+"},");
codeOut('};');

codeOut('');
codeOut('');

codeOut('const JswSymList *jswGetSymbolListForConstructorProto(JsVar *constructor) {')
codeOut('  void *constructorPtr = constructor->varData.native.ptr;')
for className in builtins:
  builtin = builtins[className]
  if builtin["isProto"] and "constructorPtr" in className:
    codeOut("  if ("+className+") return &jswSymbolTables["+builtin["indexName"]+"];");
codeOut('  return 0;')
codeOut('}')

codeOut('')
codeOut('')


codeOut('JsVar *jswFindBuiltInFunction(JsVar *parent, const char *name) {')
codeOut('  JsVar *v;')
codeOut('  if (parent && !jsvIsRoot(parent)) {')

codeOut('    // ------------------------------------------ INSTANCE + STATIC METHODS')
nativeCheck = "jsvIsNativeFunction(parent) && "
codeOut('    if (jsvIsNativeFunction(parent)) {')
codeOut('      const JswSymList *l = jswGetSymbolListForObject(parent);')
codeOut('      if (l) {');
codeOut('        v = jswBinarySearch(l, parent, name);')
codeOut('        if (v) return v;');
codeOut('      }')
codeOut('    }')
for className in builtins:
  if className!="parent" and  className!="!parent" and not "constructorPtr" in className and not className.startswith(nativeCheck):
    codeOut('    if ('+className+') {')
    codeOutBuiltins("      v = ", builtins[className])
    codeOut('      if (v) return v;');
    codeOut("    }")
codeOut('    // ------------------------------------------ INSTANCE METHODS WE MUST CHECK CONSTRUCTOR FOR')
codeOut('    JsVar *proto = jsvIsObject(parent)?jsvSkipNameAndUnLock(jsvFindChildFromString(parent, JSPARSE_INHERITS_VAR)):0;')
codeOut('    JsVar *constructor = jsvIsObject(proto)?jsvSkipNameAndUnLock(jsvFindChildFromString(proto, JSPARSE_CONSTRUCTOR_VAR)):0;')
codeOut('    jsvUnLock(proto);')
codeOut('    if (constructor && jsvIsNativeFunction(constructor)) {')
codeOut('      const JswSymList *l = jswGetSymbolListForConstructorProto(constructor);')
codeOut('      jsvUnLock(constructor);')
codeOut('      if (l) {');
codeOut('        v = jswBinarySearch(l, parent, name);')
codeOut('        if (v) return v;');
codeOut('      }')
codeOut('    } else {')
codeOut('      jsvUnLock(constructor);')
codeOut('    }')
codeOut('    // ------------------------------------------ METHODS ON OBJECT')
if "parent" in builtins:
  codeOutBuiltins("    v = ", builtins["parent"])
  codeOut('    if (v) return v;');
codeOut('  } else { /* if (!parent) */')
codeOut('    // ------------------------------------------ FUNCTIONS')
codeOut('    #ifndef ESPR_EMBED')
codeOut('    // Handle pin names - eg LED1 or D5 (this is hardcoded in build_jsfunctions.py)')
codeOut('    Pin pin = jshGetPinFromString(name);')
codeOut('    if (pin != PIN_UNDEFINED) ')
codeOut('      return jsvNewFromPin(pin);')
codeOut('    #endif')
if "!parent" in builtins:
  codeOutBuiltins("    return ", builtins["!parent"])
codeOut('  }');

codeOut('  return 0;')
codeOut('}')

codeOut('')
codeOut('')

codeOut('const JswSymList *jswGetSymbolListForObject(JsVar *parent) {')
nativeTestStr = "jsvIsNativeFunction(parent) && ";
codeOut("  if (jsvIsNativeFunction(parent)) {");
for className in builtins:
  builtin = builtins[className]
  if not className in ["parent","!parent"] and not builtin["isProto"] and className.startswith(nativeTestStr):
    codeOut("    if ("+className[len(nativeTestStr):]+") return &jswSymbolTables["+builtin["indexName"]+"];");
codeOut("  }");
for className in builtins:
  builtin = builtins[className]
  if not className in ["parent","!parent"] and not builtin["isProto"] and not className.startswith(nativeTestStr):
    codeOut("  if ("+className+") return &jswSymbolTables["+builtin["indexName"]+"];");
codeOut("  if (parent==execInfo.root) return &jswSymbolTables[jswSymbolIndex_global];");
codeOut("  return 0;")
codeOut('}')

codeOut('')
codeOut('')

codeOut('const JswSymList *jswGetSymbolListForObjectProto(JsVar *parent) {')
codeOut('  if (jsvIsNativeFunction(parent)) {')
for className in builtins:
  builtin = builtins[className]
  if builtin["isProto"] and not "constructorPtr" in className and not className in ["parent","!parent"] :
    check = className
    for jsondata in jsondatas:
      if jsondata["type"]=="constructor" and jsondata["name"]==builtin["className"]:
        check = "(void*)parent->varData.native.ptr==(void*)"+jsondata["generate"]

    codeOut("    if ("+check+") return &jswSymbolTables["+builtin["indexName"]+"];");
codeOut('  }')
codeOut('  JsVar *constructor = jsvIsObject(parent)?jsvSkipNameAndUnLock(jsvFindChildFromString(parent, JSPARSE_CONSTRUCTOR_VAR)):0;')
codeOut('  if (constructor && jsvIsNativeFunction(constructor)) {')
codeOut('    const JswSymList *l = jswGetSymbolListForConstructorProto(constructor);')
codeOut('    jsvUnLock(constructor);')
codeOut('    if (l) return l;')
codeOut('  }')
nativeCheck = "jsvIsNativeFunction(parent) && "
for className in builtins:
  if className!="parent" and  className!="!parent" and not "constructorPtr" in className and not className.startswith(nativeCheck):
    codeOut('  if ('+className+") return &jswSymbolTables["+builtins[className]["indexName"]+"];");
codeOut("  return &jswSymbolTables["+builtins["parent"]["indexName"]+"];")
codeOut('}')

codeOut('')
codeOut('')

builtinObjectNames = []
for jsondata in jsondatas:
  if "class" in jsondata:
    builtinObjectName = jsondata["class"];
    if not builtinObjectName in libraries:
      if not builtinObjectName in builtinObjectNames:
        builtinObjectNames.append(builtinObjectName)


codeOut('bool jswIsBuiltInObject(const char *name) {')
codeOut('  const char *objNames = "'+"\\0".join(builtinObjectNames)+'\\0";')
codeOut('  const char *s = objNames;')
codeOut('  while (*s) {')
codeOut('    if (strcmp(s, name)==0) return true;') # OPT: I bet we could do this faster if not using strcmp, but it probably doesn't matter
codeOut('    s+=strlen(s)+1;')
codeOut('  }')
codeOut('  return false;')
codeOut('}')

codeOut('')
codeOut('')


codeOut('void *jswGetBuiltInLibrary(const char *name) {')
for lib in libraries:
  codeOut('  if (strcmp(name, "'+lib+'")==0) return (void*)gen_jswrap_'+lib+'_'+lib+';');
codeOut('  return 0;')
codeOut('}')

codeOut('')
codeOut('')

objectChecks = {}
for jsondata in jsondatas:
  if "type" in jsondata and jsondata["type"]=="class":
    if "check" in jsondata:
      objectChecks[jsondata["class"]] = jsondata["check"]

codeOut('/** Given a variable, return the basic object name of it */')
codeOut('const char *jswGetBasicObjectName(JsVar *var) {')
codeOut('  if (jsvIsArrayBuffer(var)) {')
for className in objectChecks.keys():
  if objectChecks[className].startswith("jsvIsArrayBuffer(var) && "):
    codeOut("    if ("+objectChecks[className][25:]+") return \""+className+"\";")
codeOut('  }')
for className in objectChecks.keys():
  if not objectChecks[className].startswith("jsvIsArrayBuffer(var) && "):
    codeOut("  if ("+objectChecks[className]+") return \""+className+"\";")
codeOut('  return 0;')
codeOut('}')

codeOut('')
codeOut('')


codeOut("/** Given the name of a Basic Object, eg, Uint8Array, String, etc. Return the prototype object's name - or 0. */")
codeOut('const char *jswGetBasicObjectPrototypeName(const char *objectName) {')
for jsondata in jsondatas:
  if "type" in jsondata and jsondata["type"]=="class":
    if "prototype" in jsondata:
      #print json.dumps(jsondata, sort_keys=True, indent=2)
      codeOut("  if (!strcmp(objectName, \""+jsondata["class"]+"\")) return \""+jsondata["prototype"]+"\";")
codeOut('  return strcmp(objectName,"Object") ? "Object" : 0;')
codeOut('}')

codeOut('')
codeOut('')

codeOut("/** Tasks to run on Idle. Returns true if either one of the tasks returned true (eg. they're doing something and want to avoid sleeping) */")
codeOut('bool jswIdle() {')
codeOut('  bool wasBusy = false;')
for jsondata in jsondatas:
  if "type" in jsondata and jsondata["type"]=="idle":
    codeOut("  if ("+jsondata["generate"]+"()) wasBusy = true;")
codeOut('  return wasBusy;')
codeOut('}')

codeOut('')
codeOut('')

codeOut("/** Tasks to run on Hardware Initialisation (called once at boot time, after jshInit, before jsvInit/etc) */")
codeOut('void jswHWInit() {')
for jsondata in jsondatas:
  if "type" in jsondata and jsondata["type"]=="hwinit":
    codeOut("  "+jsondata["generate"]+"();")
codeOut('}')

codeOut('')
codeOut('')

codeOut("/** Tasks to run on Initialisation (eg boot/load/reset/after save/etc) */")
codeOut('void jswInit() {')
if jsbootcode!=False:
  codeOut('  jsvUnLock(jspEvaluate('+json.dumps(jsbootcode)+', true/*static*/));')
for jsondata in jsondatas:
  if "type" in jsondata and jsondata["type"]=="init":
    codeOut("  "+jsondata["generate"]+"();")
codeOut('}')

codeOut('')
codeOut('')

codeOut("/** Tasks to run on Deinitialisation (eg before save/reset/etc) */")
codeOut('void jswKill() {')
for jsondata in jsondatas:
  if "type" in jsondata and jsondata["type"]=="kill":
    codeOut("  "+jsondata["generate"]+"();")
codeOut('}')

codeOut("/** Tasks to run when a character event is received */")
codeOut('bool jswOnCharEvent(IOEventFlags channel, char charData) {')
codeOut('  NOT_USED(channel);')
codeOut('  NOT_USED(charData);')
for jsondata in jsondatas:
  if "type" in jsondata and jsondata["type"].startswith("EV_"):
    codeOut("  if (channel=="+jsondata["type"]+") return "+jsondata["generate"]+"(charData);")
codeOut('  return false;')
codeOut('}')

codeOut("/** If we have a built-in module with the given name, return the module's contents - or 0 */")
codeOut('const char *jswGetBuiltInJSLibrary(const char *name) {')
codeOut('  NOT_USED(name);')
for modulename in jsmodules:
  codeOut("  if (!strcmp(name,\""+modulename+"\")) return "+json.dumps(jsmodules[modulename])+";")
codeOut('  return 0;')
codeOut('}')

codeOut('')
codeOut('')

codeOut('const char *jswGetBuiltInLibraryNames() {')
librarynames = []
for lib in libraries:
  librarynames.append(lib);
for lib in jsmodules:
  librarynames.append(lib);
codeOut('  return "'+','.join(librarynames)+'";')
codeOut('}')

if "USE_CALLFUNCTION_HACK" in board.defines:
  codeOut('// on Emscripten and i386 we cant easily hack around function calls with floats/etc, plus we have enough')
  codeOut('// resources, so just brute-force by handling every call pattern we use in a switch')
  codeOut('JsVar *jswCallFunctionHack(void *function, JsnArgumentType argumentSpecifier, JsVar *thisParam, JsVar **paramData, int paramCount) {')
  codeOut('  switch((int)argumentSpecifier) {')
  #for argSpec in argSpecs:
  #  codeOut('  case '+argSpec+":")
  argSpecs = []
  for jsondata in jsondatas:
    if "generate" in jsondata:
      argSpec = getArgumentSpecifier(jsondata)
      if not argSpec in argSpecs:
        argSpecs.append(argSpec)
        params = getParams(jsondata)
        result = getResult(jsondata);
        pTypes = []
        pValues = []
        if hasThis(jsondata):
          pTypes.append("JsVar*")
          pValues.append("thisParam")
        cmd = "";
        cmdstart = "";
        cmdend = "";
        n = 0
        for param in params:
          pTypes.append(toCType(param[1]));
          if param[1]=="JsVarArray":
            cmdstart =  "      JsVar *argArray = (paramCount>"+str(n)+")?jsvNewArray(&paramData["+str(n)+"],paramCount-"+str(n)+"):jsvNewEmptyArray();\n";
            pValues.append("argArray");
            cmdend = "      jsvUnLock(argArray);\n\n";
          else:
            pValues.append(toCUnbox(param[1])+"((paramCount>"+str(n)+")?paramData["+str(n)+"]:0)");
          n = n+1

        codeOut("    case "+argSpec+": {");
        codeOut("      JsVar *result = 0;");
        if cmdstart:  codeOut(cmdstart);
        cmd = "(("+toCType(result[0])+"(*)("+",".join(pTypes)+"))function)("+",".join(pValues)+")";
        if result[0]: codeOut("      result = "+toCBox(result[0])+"("+cmd+");");
        else: codeOut("      "+cmd+";");
        if cmdend:  codeOut(cmdend);
        codeOut("      return result;");
        codeOut("    }");
  #((uint32_t (*)(size_t,size_t,size_t,size_t))function)(argData[0],argData[1],argData[2],argData[3]);
  codeOut('  default: jsExceptionHere(JSET_ERROR,"Unknown argspec %d",argumentSpecifier);')
  codeOut('  }')
  codeOut('  return 0;')
  codeOut('}')

codeOut('')
codeOut('')
