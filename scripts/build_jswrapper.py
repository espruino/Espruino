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
import common
from inspect import currentframe, getframeinfo

# ------------------------------------------------------------------------------------------------------

def codeOut(s): 
#  print str(s)
  wrapperFile.write(s+"\n");

# Dump the current position in this file
def getCurrentFilePos():
  cf = currentframe()
  return "build_jswrapper.py:"+str(cf.f_back.f_lineno)

# ------------------------------------------------------------------------------------------------------

def toArgumentType(argName):
  if argName=="": return "JSWAT_VOID";
  if argName=="JsVar": return "JSWAT_JSVAR";
  if argName=="JsVarArray": return "JSWAT_ARGUMENT_ARRAY";
  if argName=="bool": return "JSWAT_BOOL";
  if argName=="pin": return "JSWAT_PIN";
  if argName=="int32": return "JSWAT_INT32";
  if argName=="int": return "JSWAT_INT32";
  if argName=="float": return "JSWAT_JSVARFLOAT";
  sys.stderr.write("ERROR: toArgumentType: Unknown argument name "+argName+"\n")
  exit(1)

def toCType(argName):
  if argName=="": return "void";
  if argName=="JsVar": return "JsVar*";
  if argName=="JsVarArray": return "JsVar*";
  if argName=="bool": return "bool";
  if argName=="pin": return "Pin";
  if argName=="int32": return "int";
  if argName=="int": return "JsVarInt";
  if argName=="float": return "JsVarFloat";
  sys.stderr.write("ERROR: toCType: Unknown argument name "+argName+"\n")
  exit(1)

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
  if jsondata["thisParam"]: s.append("JSWAT_THIS_ARG");
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
  return " | ".join(s);
  
def getCDeclaration(jsondata, name): 
  # name could be '(*)' for a C function pointer 
  params = getParams(jsondata)
  result = getResult(jsondata);
  s = [ ]
  if jsondata["thisParam"]: s.append("JsVar*");
  for param in params:
    s.append(toCType(param[1]));
  return toCType(result[0])+" "+name+"("+",".join(s)+")";

def codeOutSymbolTable(name, tab):
  codeName = name.replace(".","_")
  # sort by name
  tab["functions"] = sorted(tab["functions"], key=lambda n: n["name"]);
  # output tables
  listSymbols = []
  listChars = ""
  strLen = 0
  # TODO: what if we have no functions? Should save space by not having a table
  for sym in tab["functions"]:
    symName = sym["name"];

    if "generate" in sym:
      listSymbols.append("{"+", ".join([str(strLen), "(void (*)(void))"+sym["generate"], getArgumentSpecifier(sym)])+"}")
      listChars = listChars + symName + "\\0";
      strLen = strLen + len(symName) + 1
    else: 
      print (codeName + "." + symName+" not included in Symbol Table because no 'generate'")
      print(json.dumps(sym, sort_keys=True, indent=2))
  tab["symbolTableChars"] = "\""+listChars+"\"";
  tab["symbolTableCount"] = str(len(listSymbols));
  tab["symbolListName"] = "jswSymbols_"+codeName;
  if name in constructors: 
    tab["constructorPtr"]="(void (*)(void))"+constructors[name]["generate"]
    tab["constructorSpec"]=getArgumentSpecifier(constructors[name])
  else:
    tab["constructorPtr"]="0"
    tab["constructorSpec"]="0"
  codeOut("static const JswSymPtr "+tab["symbolListName"]+"[] = {\n  "+",\n  ".join(listSymbols)+"\n};");

def codeOutBuiltins(indent, builtin):
  codeOut(indent+"jswBinarySearch(&jswSymbolTables["+builtin["indexName"]+"], parent, name);");

# ------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------

jsondatas = common.get_jsondata(False)
includes = common.get_includes_from_jsondata(jsondatas)

# work out what we have actually got
classes = {}      # list of class names
constructors = {} 
objectChecks = {} # class name -> the check for this class Type

for jsondata in jsondatas:

  if not "type" in jsondata:
    print("ERROR: no type for "+json.dumps(jsondata, sort_keys=True, indent=2))
    exit(1)

  if jsondata["type"] in ["idle","kill","init","include"]: continue

  if not "name" in jsondata:
    print("WARNING: no name for "+json.dumps(jsondata, sort_keys=True, indent=2))

  if jsondata["type"]=="object":
    if "check" in jsondata:
      objectChecks[jsondata["name"]] = jsondata["check"]
    if jsondata["name"] in classes:
      print("ERROR: "+jsondata["name"]+" is defined twice");
      exit(1);
    classes[jsondata["name"]] = jsondata

  if jsondata["type"] in ["function","variable","constructor"]:
    if not "thisParam" in jsondata: jsondata["thisParam"] = False

  if not "instanceOf" in jsondata:
    print "WARNING: No instanceOf for "+jsondata["name"]

  if jsondata["type"]=="constructor":
    if not jsondata["name"] in constructors:
      constructors[jsondata["name"]] = jsondata
    else:
      print("ERROR: duplicate constructor "+jsondata["name"])
      exit(1)

  if jsondata["type"]=="object":
    if "instanceOf" in jsondata:
      jsondatas.append({ 
        "autogenerated":getCurrentFilePos(),
        "type":"variable", 
        "thisParam":False,
        "name":"__proto__", 
        "memberOf":jsondata["name"],
        "generate_full" : "jswCreateFromSymbolTable(jswSymbolIndex_"+jsondata["instanceOf"].replace(".","_")+"_prototype)",
        "return" : ["JsVar"],
        "filename" : jsondata["filename"]
      });
    if not "generate" in jsondata and not "generate_full" in jsondata:
      jsondata["autogenerated"] = getCurrentFilePos()
      jsondata["thisParam"] = False
      jsondata["return"] = ["JsVar"]
      jsondata["generate_full"] = "jswCreateFromSymbolTable(jswSymbolIndex_"+jsondata["name"].replace(".","_")+")"
    
# Add basic classes if we have prototypes for classes, but not the class itself
for className in classes:  
  if className[-10:]==".prototype" and not className[:-10] in classes:
    print("Auto-creating class for "+className[:-10]);
    classes[className[:-10]] = {
        "autogenerated":getCurrentFilePos(),
        "type":"object", "name": className[:-10], 
        "filename" : classes[className]["filename"],
      };


print("Finding Libraries")
libraries = []
for jsondata in jsondatas:
  if jsondata["type"]=="library":
    print(" - Found library "+jsondata["name"])
    libraries.append(jsondata["name"])


print("Creating Symbol Tables")
symbolTables = {}
# Add main types
for className in classes:
  symbolTables[className] = { "autogenerated":getCurrentFilePos(), "type" : "object", "name" : className, "functions" : [] }
for libName in libraries:
#  if libName in symbolTables: 
#    print("ERROR: Name conflict for "+libName+" while adding Library");
#    print("Existing: "+json.dumps(symbolTables[libName], sort_keys=True, indent=2))
#    exit(1);
  # ... we know there will already be a 'class' for the library name...
  symbolTables[libName] = { "autogenerated":getCurrentFilePos(), "type" : "library", "name" : libName, "functions" : [] }

# Now populate with prototypes
for className in classes:
  if className[-10:]==".prototype":
    jsondatas.append({ 
        "autogenerated":getCurrentFilePos(),
        "type":"variable", 
        "thisParam":False,
        "name":"prototype", 
        "memberOf":className[:-10],
        "generate_full" : "jswCreateFromSymbolTable(jswSymbolIndex_"+className.replace(".","_")+")",
        "return" : ["JsVar"],
        "filename" : classes[className]["filename"]
    })
    if "memberOf" in classes[className]:
      print("ERROR: Class "+className+" CAN'T BE a member of anything because it's a prototype ("+classes[className]["filename"]+")");  
      exit(1);
  elif "memberOf" in classes[className]:
    if not className in classes:
      j = { 
        "autogenerated":getCurrentFilePos(),
        "type":"variable", 
        "thisParam":False,
        "name":className, 
        "memberOf":classes[className]["memberOf"],
        "generate_full" : "jswCreateFromSymbolTable(jswSymbolIndex_"+className+")",
        "return" : ["JsVar"],
        "filename" : classes[className]["filename"]
      } 
      classes[className] = j
      jsondatas.append(j)
  else:
    print("WARNING: Class "+className+" is not a member of anything ("+classes[className]["filename"]+")");

try:
  for j in jsondatas:
    if "memberOf" in j:
      if not j["memberOf"] in symbolTables:
        symbolTables[j["memberOf"]] = { "autogenerated":getCurrentFilePos(), "type" : "object", "name" : j["memberOf"], "functions" : [] }
      symbolTables[j["memberOf"]]["functions"].append(j);
    else: print("no member: "+json.dumps(j, sort_keys=True, indent=2))
except:
  print("Unexpected error:", sys.exc_info())
  print(json.dumps(j, sort_keys=True, indent=2))
  exit(1)

# If we have a 'prototype', make sure it's linked back
# into the original class
for sym in symbolTables:
  if sym[-10:]==".prototype":
    className = sym[:-10]
    if not "prototype" in symbolTables[className]["functions"]:      
      j = { 
        "autogenerated":getCurrentFilePos(),
        "type":"variable", 
        "thisParam":False,
        "name":"prototype", 
        "memberOf":className,
        "generate_full" : "jswCreateFromSymbolTable(jswSymbolIndex_"+className+"_prototype)",
        "return" : ["JsVar"],
        "filename" : classes[className]["filename"]
      } 
      symbolTables[className]["functions"].append(j)
      jsondatas.append(j)
 

#print(json.dumps(symbolTables, sort_keys=True, indent=2))
#exit(1)

# ------------------------------------------------------------------------------------------------------
#print json.dumps(tree, sort_keys=True, indent=2)
# ------------------------------------------------------------------------------------------------------    

wrapperFile = open('gen/jswrapper.c', 'w')

codeOut('// Automatically generated wrapper file ')
codeOut('// Generated by scripts/build_jswrapper.py');
codeOut('');
codeOut('#include "jswrapper.h"');
codeOut('#include "jsnative.h"');
for include in includes:
  codeOut('#include "'+include+'"');
codeOut('');
#codeOut('// -----------------------------------------------------------------------------------------');
#codeOut('');
codeOut('');
codeOut('// -----------------------------------------------------------------------------------------');
codeOut('// --------------------------------------------------------------- SYMBOL TABLE INDICES    ');
codeOut('// -----------------------------------------------------------------------------------------');
codeOut('');

idx = 0
for name in symbolTables:
  symbolTables[name]["indexName"] = "jswSymbolIndex_"+name.replace(".","_");
  symbolTables[name]["index"] = str(idx);
  codeOut("const unsigned char "+symbolTables[name]["indexName"]+" = "+str(idx)+";");
  idx = idx + 1

codeOut('');
codeOut('// -----------------------------------------------------------------------------------------');
codeOut('// ----------------------------------------------------------------- AUTO-GENERATED WRAPPERS');
codeOut('// -----------------------------------------------------------------------------------------');
codeOut('');

for jsondata in jsondatas:
  if ("generate_full" in jsondata):
    gen_name = "gen_jswrap"
    if "memberOf" in jsondata: gen_name = gen_name + "_" + jsondata["memberOf"].replace(".","_");
    gen_name = gen_name + "_" + jsondata["name"].replace(".","_");

    jsondata["generate"] = gen_name
    s = [ ]

    params = getParams(jsondata)
    result = getResult(jsondata);
    if jsondata["thisParam"]: s.append("JsVar *parent");
    for param in params:
      s.append(toCType(param[1])+" "+param[0]);
     
    codeOut("/* "+json.dumps(jsondata, sort_keys=True, indent=2).replace("*/","").replace("/*","")+" */")
    codeOut("static "+toCType(result[0])+" "+jsondata["generate"]+"("+", ".join(s)+") {");
    if result[0]:
      codeOut("  return "+jsondata["generate_full"]+";");
    else:
      codeOut("  "+jsondata["generate_full"]+";");  
    codeOut("}");
    codeOut('');        





codeOut('// -----------------------------------------------------------------------------------------');
codeOut('// -----------------------------------------------------------------------------------------');
codeOut('// -----------------------------------------------------------------------------------------');
codeOut('');
codeOut('');

print("Outputting Symbol Tables")
for name in symbolTables:
  codeOutSymbolTable(name, symbolTables[name]);
codeOut('');
codeOut('');

codeOut('const JswSymList jswSymbolTables[] = {');
for name in symbolTables:
  tab = symbolTables[name]
  codeOut("  {"+", ".join([tab["symbolListName"], tab["symbolTableCount"], tab["symbolTableChars"], tab["constructorPtr"], tab["constructorSpec"]])+"}, // "+tab["indexName"]+", "+name);
codeOut('};');
codeOut('');
codeOut('// -----------------------------------------------------------------------------------------');
codeOut('// ------------------------------------------------------------------ symbols for debugging ');
codeOut('');
for name in symbolTables:
  tab = symbolTables[name]
  codeOut("  const JswSymList *jswSymbolTable_"+name.replace(".","_")+" = &jswSymbolTables["+tab["index"]+"]; // "+tab["indexName"]);
codeOut('');
codeOut('// -----------------------------------------------------------------------------------------');
codeOut('// -----------------------------------------------------------------------------------------');

codeOut("""
JsVar *jswCreateFromSymbolTable(int tableIndex) {
  JsVar *v = jsvNewWithFlags(JSV_OBJECT | JSV_NATIVE);
  if (v) v->varData.nativeObject = &jswSymbolTables[tableIndex];
  return v;
}


JsVar *jswBinarySearch(const JswSymList *symbolsPtr, JsVar *parent, const char *name) {
  int searchMin = 0;
  int searchMax = symbolsPtr->symbolCount-1;
  while (searchMin <= searchMax) {
    int idx = (searchMin+searchMax) >> 1;
    const JswSymPtr *sym = &symbolsPtr->symbols[idx];
    int cmp = strcmp(name, &symbolsPtr->symbolChars[sym->strOffset]);
    if (cmp==0) {
      if ((sym->functionSpec & JSWAT_EXECUTE_IMMEDIATELY_MASK) == JSWAT_EXECUTE_IMMEDIATELY)
        return jsnCallFunction(sym->functionPtr, sym->functionSpec, parent, 0, 0);
      return jsvNewNativeFunction(sym->functionPtr, sym->functionSpec);
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

codeOut('')
codeOut('')

codeOut('int jswGetSymbolIndexForObject(JsVar *var) {') 
codeOut('  if (jsvIsRoot(var)) {');
codeOut('    return jswSymbolIndex_global;');
codeOut('  }');
codeOut('  if (jsvIsNativeObject(var)) {');
codeOut('    assert(var->varData.nativeObject);');
codeOut('    return (int)(var->varData.nativeObject-jswSymbolTables);'); # fixme - why not store int??
codeOut('  }');
for className in objectChecks.keys():
  if className!="Function": # FIXME: This breaks `function Foo();Foo.prototype.toString=something;`
    codeOut("  if ("+objectChecks[className]+") return jswSymbolIndex_"+className+";")
codeOut("  return -1;")
codeOut('}')

codeOut('int jswGetSymbolIndexForObjectProto(JsVar *var) {') 
codeOut('  // Instantiated objects, so we should point to the prototypes of the object itself');
codeOut(' //FIXME - see build_jswrapper.py')
for className in objectChecks.keys():
  if not className=="global": # we did 'global' above
    if (className+".prototype") in symbolTables:
      codeOut("  if ("+objectChecks[className]+") return jswSymbolIndex_"+className+"_prototype;")
codeOut("  return -1;")
codeOut('}')

codeOut("""


const JswSymList *jswGetSymbolListForObject(JsVar *var) {
  int symIdx = jswGetSymbolIndexForObject(var);
  return (symIdx>=0) ? &jswSymbolTables[symIdx] : 0;
}


const JswSymList *jswGetSymbolListForObjectProto(JsVar *var) {
  int symIdx = jswGetSymbolIndexForObjectProto(var);
  return (symIdx>=0) ? &jswSymbolTables[symIdx] : 0;
}

// For instances of builtins like Pin, String, etc, search in X.prototype
JsVar *jswFindInObjectProto(JsVar *parent, const char *name) { 
  int symIdx = jswGetSymbolIndexForObjectProto(parent);
  if (symIdx>=0) return jswBinarySearch(&jswSymbolTables[symIdx], parent, name);
  return 0;
}

JsVar *jswFindBuiltIn(JsVar *parentInstance, JsVar *parent, const char *name) {
  if (jsvIsRoot(parent)) {
    // Check to see whether we're referencing a pin? Should really be in symbol table...
    Pin pin = jshGetPinFromString(name);
    if (pin != PIN_UNDEFINED) {
      return jsvNewFromPin(pin);
    }
  }
  int symIdx = jswGetSymbolIndexForObject(parent);
  if (symIdx>=0) return jswBinarySearch(&jswSymbolTables[symIdx], parentInstance, name);
  return 0;
}

""");




builtinChecks = []
for jsondata in jsondatas:
  if "memberOf" in jsondata:
    if not jsondata["memberOf"] in libraries and jsondata["memberOf"].find(".")<0:
      check = 'strcmp(name, "'+jsondata["memberOf"]+'")==0';
      if not check in builtinChecks :
        builtinChecks.append(check)


codeOut('bool jswIsBuiltInObject(const char *name) {') 
codeOut('  return\n'+" ||\n    ".join(builtinChecks)+';')
codeOut('}')

codeOut('')
codeOut('')


codeOut('JsVar *jswGetBuiltInLibrary(const char *name) {') 
for lib in libraries:
  codeOut('  if (strcmp(name, "'+lib+'")==0) return jswCreateFromSymbolTable(jswSymbolIndex_'+lib+');');
codeOut('  return 0;')
codeOut('}')

codeOut('')
codeOut('')


codeOut('/** Given a variable, return the basic object name of it */')
codeOut('const char *jswGetBasicObjectName(JsVar *var) {') 
for className in objectChecks.keys():
  codeOut("  if ("+objectChecks[className]+") return \""+className+"\";")
codeOut('  return 0;')
codeOut('}')

codeOut('')
codeOut('')


codeOut("/** Given the name of a Basic Object, eg, Uint8Array, String, etc. Return the prototype object's name - or 0. */")
codeOut('const char *jswGetBasicObjectPrototypeName(const char *objectName) {') 
for jsondata in jsondatas:
  if "type" in jsondata and jsondata["type"]=="object":
    if "prototype" in jsondata:
      #print json.dumps(jsondata, sort_keys=True, indent=2)
      codeOut("  if (!strcmp(objectName, \""+jsondata["name"]+"\")) return \""+jsondata["prototype"]+"\";")
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

codeOut("/** Tasks to run on Initialisation */")
codeOut('void jswInit() {')
for jsondata in jsondatas:
  if "type" in jsondata and jsondata["type"]=="init":
    codeOut("  "+jsondata["generate"]+"();")
codeOut('}')

codeOut('')
codeOut('')

codeOut("/** Tasks to run on Deinitialisation */")
codeOut('void jswKill() {')
for jsondata in jsondatas:
  if "type" in jsondata and jsondata["type"]=="kill":
    codeOut("  "+jsondata["generate"]+"();")
codeOut('}')

codeOut('')
codeOut('')



