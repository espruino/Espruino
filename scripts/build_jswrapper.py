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

# ------------------------------------------------------------------------------------------------------

def codeOut(s): 
#  print str(s)
  wrapperFile.write(s+"\n");

# ------------------------------------------------------------------------------------------------------

def getTestFor(className):
  if className=="String.prototype": return "jsvIsString(parent)"
  if className=="Pin.prototype": return "jsvIsPin(parent)"
  if className=="Integer.prototype": return "jsvIsInt(parent)"
  if className=="Double.prototype": return "jsvIsFloat(parent)"
  if className=="Number.prototype": return "jsvIsInt(parent) || jsvIsFloat(parent)"
  if className=="Object.prototype": return "parent" # we assume all are objects
  if className=="Array.prototype": return "jsvIsArray(parent)"
  if className=="ArrayBufferView.prototype": return "jsvIsArrayBuffer(parent) && parent->varData.arraybuffer.type!=ARRAYBUFFERVIEW_ARRAYBUFFER"
  if className=="Function.prototype": return "jsvIsFunction(parent)"
  return "";

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
  if not jsondata["static"]: s.append("JSWAT_THIS_ARG");
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
  if not jsondata["static"]: s.append("JsVar*");
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
  tab["symbolTableChars"] = "\""+listChars+"\"";
  tab["symbolTableCount"] = str(len(listSymbols));
  tab["symbolListName"] = "jswSymbols_"+codeName;
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
    print "ERROR: no type for "+json.dumps(jsondata, sort_keys=True, indent=2)
    exit(1)

  if jsondata["type"] in ["idle","kill","init","include"]: continue

  if not "name" in jsondata:
    print "WARNING: no name for "+json.dumps(jsondata, sort_keys=True, indent=2)
    jsondata["name"] = jsondata["class"]
    del jsondata["class"]


  jsondata["static"]=True
  if jsondata["type"]=="staticmethod":
#    print(jsondata["class"]+"."+jsondata["name"]+" needs updating")
    jsondata["type"]="function"   
  if jsondata["type"]=="staticproperty":
    jsondata["type"]="variable"
#    print(jsondata["class"]+"."+jsondata["name"]+" needs updating")
  if jsondata["type"]=="method":
    jsondata["type"]="function"
    jsondata["static"]=False
    jsondata["class"]=jsondata["class"]+".prototype"
#    print(jsondata["class"]+"."+jsondata["name"]+" needs updating")
  if jsondata["type"]=="property":
    jsondata["type"]="variable"
    jsondata["static"]=False
    jsondata["class"]=jsondata["class"]+".prototype"
#    print(jsondata["class"]+"."+jsondata["name"]+" needs updating")

  if jsondata["type"]=="class":
    if "check" in jsondata:
      objectChecks[jsondata["name"]] = jsondata["check"]
    classes[jsondata["name"]] = jsondata
    # Also create a fake variable for the class prototype to make sure it'll exist
    jsondatas.append({ 
        "type":"variable", 
        "static":True,
        "name":"i_am_just_a_placeholder",         
        "class":jsondata["name"]+".prototype",
        "filename":"jswrapper.c"
      })

  if not "class" in jsondata:
    print "No class for "+jsondata["name"]+" - adding to Global"
    jsondata["class"] = "global";

  if jsondata["type"]=="constructor":
    if not jsondata["name"] in constructors:
      constructors[jsondata["name"]] = jsondata

  if jsondata["type"]=="class" or jsondata["type"]=="object":
    if "instanceof" in jsondata:
      jsondatas.append({ 
        "type":"variable", 
        "static":True,
        "name":"__proto__", 
        "class":jsondata["name"],
        "generate_full" : "jswFindFromSymbolTable(jswSymbolIndex_"+jsondata["instanceof"].replace(".","_")+"_prototype)",
        "filename" : jsondata["filename"]
      })
    if "prototype" in jsondata:
      jsondatas.append({ 
        "type":"variable", 
        "static":True,
        "name":"__proto__",         
        "class":jsondata["name"]+".prototype",
        "generate_full" : "jswFindFromSymbolTable(jswSymbolIndex_"+jsondata["prototype"].replace(".","_")+"_prototype)",
        "filename" : jsondata["filename"]
      })

# Make sure we have any classes that are referenced
for jsondata in jsondatas:
  if "class" in jsondata:
    className = jsondata["class"]
    if not className in classes:
      print("Auto-creating class for "+className);
      classes[className] = {
        "type":"class", "class": className,  "name": className,
        "filename" : jsondata["filename"],
      };

# Add basic classes if we have prototypes for classes, but not the class itself
for className in classes:  
  if className[-10:]==".prototype" and not className[:-10] in classes:
    print("Auto-creating class for "+className[:-10]);
    classes[className[:-10]] = {
        "type":"class", "class": className[:-10],  "name": className[:-10],
        "filename" : classes[className]["filename"],
      };

# Add constructors if we need them
# TODO: Do we need this???
for className in classes:
  if not className in constructors:
    print("Added constructor for "+className);
    jsondata = {
        "type":"constructor", "class": className,  "name": className,
        "generate_full" : "jswFindFromSymbolTable(jswSymbolIndex_"+className.replace(".","_")+")",
        "filename" : "jswrapper.c",
        "static" : "True",
        "return" : [ "JsVar", "" ]
    };
    constructors[className] = jsondata;
    jsondatas.append(jsondata);

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
  symbolTables[className] = { "type" : "class", "functions" : [] }
for libName in libraries:
#  if libName in symbolTables: 
#    print("ERROR: Name conflict for "+libName+" while adding Library");
#    print("Existing: "+json.dumps(symbolTables[libName], sort_keys=True, indent=2))
#    exit(1);
  # ... we know there will already be a 'class' for the library name...
  symbolTables[libName] = { "type" : "library", "functions" : [] }

# Now populate with prototypes
for className in classes:
  if className[-10:]==".prototype":
    jsondatas.append({ 
        "type":"variable", 
        "static":True,
        "name":"prototype", 
        "class":className[:-10],
        "generate_full" : "jswFindFromSymbolTable(jswSymbolIndex_"+className.replace(".","_")+")",
        "filename" : classes[className]["filename"]
    })
    jsondatas.append({ 
        "type":"variable", 
        "static":True,
        "name":"constructor", 
        "class":className,
        "generate_full" : "jswFindFromSymbolTable(jswSymbolIndex_"+className[:-10].replace(".","_")+")",
        "filename" : classes[className]["filename"]
    })


try:
  for j in jsondatas:
    if j["type"]=="function" or j["type"]=="variable":
      symbolTables[j["class"]]["functions"].append(j);
except:
  print("Unexpected error:", sys.exc_info())
  print(json.dumps(j, sort_keys=True, indent=2))
  exit(1)

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
codeOut('// -----------------------------------------------------------------------------------------');
codeOut("""
const JswSymList jswSymbolTables[]; // forward decl

JsVar *jswFindFromSymbolTable(int tableIndex) {
  // TODO: first see if one is actually defined
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

codeOut('// -----------------------------------------------------------------------------------------');
codeOut('// --------------------------------------------------------------- SYMBOL TABLE INDICES    ');
codeOut('// -----------------------------------------------------------------------------------------');
codeOut('');

idx = 0
for name in symbolTables:
  symbolTables[name]["indexName"] = "jswSymbolIndex_"+name.replace(".","_");
  codeOut("static const unsigned char "+symbolTables[name]["indexName"]+" = "+str(idx)+";");
  idx = idx + 1

codeOut('');
codeOut('// -----------------------------------------------------------------------------------------');
codeOut('// ----------------------------------------------------------------- AUTO-GENERATED WRAPPERS');
codeOut('// -----------------------------------------------------------------------------------------');
codeOut('');

for jsondata in jsondatas:
  if ("generate_full" in jsondata) or (jsondata["type"]=="object"):
    gen_name = "gen_jswrap"
    if "class" in jsondata: gen_name = gen_name + "_" + jsondata["class"].replace(".","_");
    gen_name = gen_name + "_" + jsondata["name"].replace(".","_");

    jsondata["generate"] = gen_name
    s = [ ]

    if jsondata["type"]=="object": # TODO: this isn't right now...
      jsondata["generate_full"] = "jspNewObject(\""+jsondata["name"]+"\", \""+jsondata["instanceof"]+"\") /* needs JSWAT_EXECUTE_IMMEDIATELY */";
      params = []
      result = ["JsVar"]
    else:
      params = getParams(jsondata)
      result = getResult(jsondata);
      if not jsondata["static"]: s.append("JsVar *parent");
      for param in params:
        s.append(toCType(param[1])+" "+param[0]);
     
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
  codeOut("  {"+", ".join([tab["symbolListName"], tab["symbolTableCount"], tab["symbolTableChars"]])+"}, // "+name);
codeOut('};');

codeOut('')
codeOut('')

codeOut('const JswSymList *jswGetSymbolListForObject(JsVar *var) {') 
codeOut('  if (jsvIsRoot(var)) {');
codeOut('    return &jswSymbolTables[jswSymbolIndex_global];');
codeOut('  }');
codeOut('  if (jsvIsNativeObject(var)) {');
codeOut('    assert(var->varData.nativeObject);');
codeOut('    return var->varData.nativeObject;');
codeOut('  }');
codeOut('  // Instantiated objects, so we should point to the prototypes');
for className in objectChecks.keys():
  if not className=="global": # we did 'global' above
    codeOut("  if ("+objectChecks[className]+") return &jswSymbolTables[jswSymbolIndex_"+className+"_prototype];")
codeOut("  return 0;")
codeOut('}')

codeOut('');
codeOut('');


codeOut('JsVar *jswFindBuiltIn(JsVar *parent, const char *name) {');
codeOut('  if (jsvIsRoot(parent)) {');
codeOut('    Pin pin = jshGetPinFromString(name);');
codeOut('    if (pin != PIN_UNDEFINED) {');
codeOut('      return jsvNewFromPin(pin);');
codeOut('    }');
codeOut('  }');
codeOut('  const JswSymList *symList = jswGetSymbolListForObject(parent);');
codeOut('  if (symList) return jswBinarySearch(symList, parent, name);');
codeOut('  return 0;')
codeOut('}')



codeOut('')
codeOut('')

builtinChecks = []
for jsondata in jsondatas:
  if "class" in jsondata:
    if not jsondata["class"] in libraries and jsondata['class'].find(".")<0:
      check = 'strcmp(name, "'+jsondata["class"]+'")==0';
      if not check in builtinChecks :
        builtinChecks.append(check)


codeOut('bool jswIsBuiltInObject(const char *name) {') 
codeOut('  return\n'+" ||\n    ".join(builtinChecks)+';')
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
  if "type" in jsondata and jsondata["type"]=="class":
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



