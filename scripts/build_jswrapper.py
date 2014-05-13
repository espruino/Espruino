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
sys.path.append(".");
import common

# ------------------------------------------------------------------------------------------------------

def codeOut(s): 
#  print str(s)
  wrapperFile.write(s+"\n");

def treewalk(tree, name, jsondata):
  if len(name)==0:
    tree[""] = jsondata
  else:	
    firstchar = name[:1]
    if not firstchar in tree: tree[firstchar] = {}
    treewalk(tree[firstchar], name[1:], jsondata)

# ------------------------------------------------------------------------------------------------------
# Creates something like 'name[0]=='s' && name[1]=='e' && name[2]=='t' && name[3]==0'
def createStringCompare(varName, checkOffsets, checkCharacters):
  checks = []
  # if we're doing multiple checks, batch up into int compare
  # NOTE: batching up into 64 bit compare doesn't help here
  # 4 byte check
  while len(checkOffsets)>3:    
    checkOffset = checkOffsets.pop(0)
    checkOffsets.pop(0)
    checkOffsets.pop(0)
    checkOffsets.pop(0)
    checkWC = [checkCharacters.pop(0),checkCharacters.pop(0),checkCharacters.pop(0),checkCharacters.pop(0)]
    checks.append("CMP4("+varName+"["+str(checkOffset)+"],'"+("','".join(checkWC))+"')")
  # 3 byte check
  while len(checkOffsets)>2:    
    checkOffset = checkOffsets.pop(0)
    checkOffsets.pop(0)
    checkOffsets.pop(0)
    checkWC = [checkCharacters.pop(0),checkCharacters.pop(0),checkCharacters.pop(0)]
    checks.append("CMP3("+varName+"["+str(checkOffset)+"],'"+("','".join(checkWC))+"')")  
  # 2 byte check
  while len(checkOffsets)>1:
    checkOffset = checkOffsets.pop(0)
    checkOffsets.pop(0)
    checkWC = [checkCharacters.pop(0),checkCharacters.pop(0)]
    checks.append("CMP2("+varName+"["+str(checkOffset)+"],'"+("','".join(checkWC))+"')")
  # finish up with single checks
  while len(checkOffsets)>0:
    checkOffset = checkOffsets.pop(0)
    checkCharacter = checkCharacters.pop(0)
    # This check is a hack for when class names are exactly the same length as the data field in varData (8 chars)
    if not "varData" in varName or checkOffset < 8: 
      checks.append(varName+"["+str(checkOffset)+"]=='"+checkCharacter+"'")
  return " && ".join(checks)
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
    print "No constructor found for "+className
    exit(1)

def getTestFor(className, static):
  if static:
    return getConstructorTestFor(className, "parent");
  else:
    if className=="String": return "jsvIsString(parent)"
    if className=="Pin": return "jsvIsPin(parent)"
    if className=="Integer": return "jsvIsInt(parent)"
    if className=="Double": return "jsvIsFloat(parent)"
    if className=="Number": return "jsvIsInt(parent) || jsvIsFloat(parent)"
    if className=="Object": return "parent" # we assume all are objects
    if className=="Array": return "jsvIsArray(parent)"
    if className=="ArrayBufferView": return "jsvIsArrayBuffer(parent) && parent->varData.arraybuffer.type!=ARRAYBUFFERVIEW_ARRAYBUFFER"
    if className=="Function": return "jsvIsFunction(parent)"
    return getConstructorTestFor(className, "constructorPtr");

def getUnLockGetter(varType, name, funcName):
  if varType=="float": return "jsvGetFloatAndUnLock("+name+")"
  if varType=="int": return "jsvGetIntegerAndUnLock("+name+")"
  if varType=="int32": return "(int)jsvGetIntegerAndUnLock("+name+")"
  if varType=="bool": return "jsvGetBoolAndUnLock("+name+")"
  if varType=="pin": return "jshGetPinFromVarAndUnLock("+name+")"
  sys.stderr.write("ERROR: getUnLockGetter: Unknown type '"+varType+"' for "+funcName+":"+name+"\n")
  exit(1)

def getGetter(varType, name, funcName):
  if varType=="float": return "jsvGetFloat("+name+")"
  if varType=="int": return "jsvGetInteger("+name+")"
  if varType=="int32": return "(int)jsvGetInteger("+name+")"
  if varType=="bool": return "jsvGetBool("+name+")"
  if varType=="pin": return "jshGetPinFromVar("+name+")"
  if varType=="JsVar": return name
  sys.stderr.write("ERROR: getGetter: Unknown type '"+varType+"' for "+funcName+":"+name+"\n")
  exit(1)

def getCreator(varType, value, funcName):
  if varType=="float": return "jsvNewFromFloat("+value+")"
  if varType=="int": return "jsvNewFromInteger("+value+")"
  if varType=="int32": return "jsvNewFromInteger((JsVarInt)"+value+")"
  if varType=="bool": return "jsvNewFromBool("+value+")"
  if varType=="JsVar": return value
  sys.stderr.write("ERROR: getCreator: Unknown type '"+varType+"'"+"' for "+funcName+"\n")
  exit(1)

def toArgumentType(argName):
  if argName=="": return "JSWAT_VOID";
  if argName=="JsVar": return "JSWAT_JSVAR";
  if argName=="JsVarArray": return "JSWAT_ARGUMENT_ARRAY";
  if argName=="bool": return "JSWAT_BOOL";
  if argName=="pin": return "JSWAT_PIN";
  if argName=="int32": return "JSWAT_INT32";
  if argName=="int": return "JSWAT_JSVARINT";
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
  if jsondata["type"]=="variable" or common.is_property(jsondata): s.append("JSWAT_EXECUTE_IMMEDIATELY")

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
  if hasThis(jsondata): s.append("JsVar*");
  for param in params:
    s.append(toCType(param[1]));
  return toCType(result[0])+" "+name+"("+",".join(s)+")";

def codeOutFunctionObject(indent, obj):
  codeOut(indent+"// Object "+obj["name"]+"  ("+obj["filename"]+")")
  if "#if" in obj: codeOut(indent+"#if "+obj["#if"]);
  codeOut(indent+"return jspNewObject(\""+obj["name"]+"\", \""+obj["instanceof"]+"\");");
  if "#if" in obj: codeOut(indent+"#endif //"+obj["#if"]);

def codeOutFunction(indent, func):
  if func["type"]=="object":
    codeOutFunctionObject(indent, func)
    return
  name = ""
  if "class" in func:
    name = name + func["class"]+".";
  name = name + func["name"]
#  print name
  codeOut(indent+"// "+name+"  ("+func["filename"]+")")
  codeOut(indent+"// "+getCDeclaration(func, name));
  
  if ("generate" in func):
    gen_name = func["generate"]
    if re.match('^[\w_]+$', gen_name) is None:
      sys.stderr.write("ERROR: generate for '"+func["name"]+"' is not simple function name: '"+gen_name+"'\n")
      exit(1)
  else:
    sys.stderr.write("ERROR: codeOutFunction: Function '"+func["name"]+"' does not have generate, generate_full or wrap elements'\n")
    exit(1)

  if func["type"]=="variable" or common.is_property(func):     
    if hasThis(func):
      gen_name = gen_name+"(parent)"
    else:
      gen_name = gen_name+"()";

    codeOut(indent+"return "+getCreator(func["return"][0], gen_name, func["name"])+";")
  else:
    codeOut(indent+"return jsvNewNativeFunction((void (*)(void))"+gen_name+", "+getArgumentSpecifier(func)+");")



def codeOutTree(indent, tree, offset):
  first = True
  for char in tree:
    if char!="":
      charOffset = offset
      charTree = tree[char]
      line = indent
      if first: first = False
      else: line = line + "} else "
      checkOffsets = [charOffset]
      checkCharacters = [char]
      while len(charTree)==1:
        charOffset = charOffset + 1
        char = charTree.keys()[0]
        charTree = charTree[char]        
        checkOffsets.append(charOffset)
        if char=='': checkCharacters.append('\\0')
        else: checkCharacters.append(char)
      line = line + "if (" + createStringCompare("name", checkOffsets, checkCharacters) + ") {"
      codeOut(line)   
      if char=='':
        codeOutFunction(indent+"  ", charTree)
      else:
        codeOutTree(indent+"  ", charTree, charOffset+1)
      # Now we do the handling part!
  if "" in tree:
    func = tree[""]
    line = indent;
    if first: first = False
    if not first: line = line + "} else "
    line = line + "if (name["+str(offset)+"]==0) {";
    codeOut(line)
    codeOutFunction(indent+"  ", func)
  if not first:
    codeOut(indent+'}')

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
    if "generate" in sym:
      listSymbols.append("{"+", ".join([str(strLen), "(void (*)(void))"+sym["generate"], getArgumentSpecifier(sym)])+"}")
      listChars = listChars + symName + "\\0";
      strLen = strLen + len(symName) + 1
    else: 
      print (codeName + "." + symName+" not included in Symbol Table because no 'generate'")
  builtin["symbolTableChars"] = "\""+listChars+"\"";
  builtin["symbolTableCount"] = str(len(listSymbols));
  codeOut("static const JswSymPtr jswSymbols_"+codeName+"[] = {\n  "+",\n  ".join(listSymbols)+"\n};");

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
classes = []
constructors = []
for jsondata in jsondatas:
  if "class" in jsondata :
    if not jsondata["class"] in classes:
      classes.append(jsondata["class"])
  if jsondata["type"]=="constructor":
    if not jsondata["name"] in constructors:
      constructors.append(jsondata["name"])

# Add constructors if we need them
for className in classes:
  if not className in constructors:
    jsondatas.append({
        "type":"constructor", "class": className,  "name": className,
        "generate_full" : "jsvNewWithFlags(JSV_OBJECT)",
        "filename" : "jswrapper.c",
        "return" : [ "JsVar", "" ]
    });

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

codeOut("#if( 'q\\0\\0\\0' & 'q' )")
codeOut('  #error( "architecture is big-endian. need to test and make sure this still works" )')
codeOut("#endif")
codeOut('// beware big endian!');
codeOut('#define CH2(a,b) ( ((b)<<8) | (a) )');
codeOut('#define CH4(a,b,c,d) ( ((d)<<24) | ((c)<<16) | ((b)<<8) | (a) )');
codeOut('#define CMP2(var, a,b) ((*(unsigned short*)&(var))==CH2(a,b))');
codeOut('#define CMP3(var, a,b,c) (((*(unsigned int*)&(var))&0x00FFFFFF)==CH4(a,b,c,0))');
codeOut('#define CMP4(var, a,b,c,d) ((*(unsigned int*)&(var))==CH4(a,b,c,d))');
codeOut('');

codeOut('// -----------------------------------------------------------------------------------------');
codeOut('// ----------------------------------------------------------------- AUTO-GENERATED WRAPPERS');
codeOut('// -----------------------------------------------------------------------------------------');
codeOut('');

for jsondata in jsondatas:
  if ("generate_full" in jsondata) or (jsondata["type"]=="object"):
    gen_name = "gen_jswrap"
    if "class" in jsondata: gen_name = gen_name + "_" + jsondata["class"];
    gen_name = gen_name + "_" + jsondata["name"];

    jsondata["generate"] = gen_name
    s = [ ]

    if jsondata["type"]=="object":
      jsondata["generate_full"] = "jspNewObject(\""+jsondata["name"]+"\", \""+jsondata["instanceof"]+"\")";
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

codeOut('// -----------------------------------------------------------------------------------------');
codeOut('// -----------------------------------------------------------------------------------------');
codeOut('// -----------------------------------------------------------------------------------------');
codeOut('');

codeOut("""
typedef struct {
  unsigned short strOffset;
  void (*functionPtr)(void);
  unsigned int functionSpec; // JsnArgumentType
} PACKED_FLAGS JswSymPtr;

typedef struct {
  const JswSymPtr *symbols;
  int symbolCount;
  const char *symbolChars;
} PACKED_FLAGS JswSymList;

JsVar *jswBinarySearch(const JswSymList *symbolsPtr, JsVar *parent, const char *name) {
  int searchMin = 0;
  int searchMax = symbolsPtr->symbolCount-1;
  while (searchMin <= searchMax) {
    int idx = (searchMin+searchMax) >> 1;
    const JswSymPtr *sym = &symbolsPtr->symbols[idx];
    int cmp = strcmp(name, &symbolsPtr->symbolChars[sym->strOffset]);
    if (cmp==0) {
      if (sym->functionSpec & JSWAT_EXECUTE_IMMEDIATELY)
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
codeOut('// -----------------------------------------------------------------------------------------');
codeOut('// -----------------------------------------------------------------------------------------');
codeOut('');
codeOut('');

print "Classifying Functions"
builtins = {}
for jsondata in jsondatas:
  if "name" in jsondata:
    jsondata["static"] = not (jsondata["type"]=="property" or jsondata["type"]=="method")

    testCode = "!parent"
    builtinName = "global"
    if not jsondata["type"]=="constructor":
      if "class" in jsondata: 
        testCode = getTestFor(jsondata["class"], jsondata["static"])
        builtinName = jsondata["class"]
        if not jsondata["static"]: builtinName = builtinName+"_proto";


    if not testCode in builtins: 
      print "Adding "+testCode+" to builtins"
      builtins[testCode] = { "name" : builtinName, "functions" : [] }

    builtins[testCode]["functions"].append(jsondata);


print "Outputting Symbol Tables"
idx = 0
for b in builtins:
  builtin = builtins[b]
  codeOutSymbolTable(builtin);
  builtins[b]["indexName"] = "jswSymbolIndex_"+builtin["name"];
  codeOut("static const unsigned char "+builtin["indexName"]+" = "+str(idx)+";");
  idx = idx + 1
codeOut('');
codeOut('');

codeOut('const JswSymList jswSymbolTables[] = {');
for b in builtins:
  builtin = builtins[b]
  codeOut("  {"+", ".join(["jswSymbols_"+builtin["name"], builtin["symbolTableCount"], builtin["symbolTableChars"]])+"},");
codeOut('};');

codeOut('');
codeOut('');


codeOut('JsVar *jswFindBuiltInFunction(JsVar *parent, const char *name) {')
codeOut('  if (parent) {')
codeOut('    // ------------------------------------------ METHODS ON OBJECT')
if "parent" in builtins:
  codeOutBuiltins("    JsVar *v = ", builtins["parent"])
  codeOut('    if (v) return v;');
codeOut('    // ------------------------------------------ INSTANCE + STATIC METHODS')
for className in builtins:
  if className!="parent" and  className!="!parent" and not "constructorPtr" in className:
    codeOut('    if ('+className+') {')
    codeOutBuiltins("      v = ", builtins[className])
    codeOut('      if (v) return v;');
    codeOut("    }")
codeOut('    // ------------------------------------------ INSTANCE METHODS WE MUST CHECK CONSTRUCTOR FOR')
codeOut('    JsVar *constructor = jsvIsObject(parent)?jsvSkipNameAndUnLock(jsvFindChildFromString(parent, JSPARSE_CONSTRUCTOR_VAR, false)):0;')
codeOut('    if (constructor && jsvIsNativeFunction(constructor)) {')
codeOut('      void *constructorPtr = constructor->varData.native.ptr;')
codeOut('      jsvUnLock(constructor);')
first = True
for className in builtins:
  if "constructorPtr" in className:
    if first:
      codeOut('    if ('+className+') {')
      first = False
    else:
      codeOut('    } else if ('+className+') {')
    codeOutBuiltins("          return ", builtins[className])
if not first:
  codeOut("    }")
codeOut('    } else {')
codeOut('      jsvUnLock(constructor);')
codeOut('    }')
codeOut('  } else { /* if (!parent) */')
codeOut('    // ------------------------------------------ FUNCTIONS')
codeOut('    // Handle pin names - eg LED1 or D5 (this is hardcoded in build_jsfunctions.py)')
codeOut('    Pin pin = jshGetPinFromString(name);')
codeOut('    if (pin != PIN_UNDEFINED) {')
codeOut('      return jsvNewFromPin(pin);')
codeOut('    }')
if "!parent" in builtins:
  codeOutBuiltins("    return ", builtins["!parent"])
codeOut('  }');

codeOut('  return 0;')
codeOut('}')

codeOut('')
codeOut('')


builtinLibraryChecks = []
for jsondata in jsondatas:
  if jsondata["type"]=="library":
    check = 'strcmp(name, "'+jsondata["class"]+'")==0';
    builtinLibraryChecks.append(check)


builtinChecks = []
for jsondata in jsondatas:
  if "class" in jsondata:
    check = 'strcmp(name, "'+jsondata["class"]+'")==0';
    if not check in builtinLibraryChecks:
      if not check in builtinChecks:
        builtinChecks.append(check)


codeOut('bool jswIsBuiltInObject(const char *name) {') 
codeOut('  return\n'+" ||\n    ".join(builtinChecks)+';')
codeOut('}')

codeOut('')
codeOut('')


codeOut('bool jswIsBuiltInLibrary(const char *name) {') 
if len(builtinLibraryChecks)==0:
  codeOut('  return false;')
else:
  codeOut('  return\n'+" ||\n    ".join(builtinLibraryChecks)+';')
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



