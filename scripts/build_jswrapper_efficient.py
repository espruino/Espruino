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
# THIS BUILD_JSWRAPPER SHOULD BE MORE EFFICIENT - AS IT USES A SINGLE TREE
# FOR COMPARISONS AND THEN CHECKS THE OWNING CLASS AFTERWARDS. IT SEEMS LIKE IT'S NOT THOUGH!
# ----------------------------------------------------------------------------------------

import subprocess;
import re;
import json;
import sys;
import os;
sys.path.append(".");
import common

# Scans files for comments of the form /*JSON......*/ and then builds a tree structure of ifs to
# efficiently detect the symbols without using RAM. See common.py for formatting

jsondatas = common.get_jsondata(False)
includes = common.get_includes_from_jsondata(jsondatas)

# ------------------------------------------------------------------------------------------------------

def addToTree(tree, name, jsondata):
  if len(name)==0:
    if "" in tree: tree[""].append(jsondata)
    else: tree[""] = [ jsondata ]
  else:	
    firstchar = name[:1]
    if not firstchar in tree: tree[firstchar] = {}
    addToTree(tree[firstchar], name[1:], jsondata)

# ------------------------------------------------------------------------------------------------------
# Creates something like 'name[0]=='s' && name[1]=='e' && name[2]=='t' && name[3]==0'
def createStringCompare(varName, checkOffsets, checkCharacters):
  checks = []
  # if we're doing multiple checks, batch up into int compare
  while len(checkOffsets)>3:
    checkOffset = checkOffsets.pop(0)
    checkOffsets.pop(0)
    checkOffsets.pop(0)
    checkOffsets.pop(0)
    checkWC = [checkCharacters.pop(0),checkCharacters.pop(0),checkCharacters.pop(0),checkCharacters.pop(0)]
    checks.append("(*(unsigned int*)&"+varName+"["+str(checkOffset)+"])==CH('"+("','".join(checkWC))+"')")
  # finish up with single checks
  while len(checkOffsets)>0:
    checkOffset = checkOffsets.pop(0)
    checkCharacter = checkCharacters.pop(0)
    checks.append(varName+"["+str(checkOffset)+"]=='"+checkCharacter+"'")
  return " && ".join(checks)
# ------------------------------------------------------------------------------------------------------

def getTestFor(className, static):
  if static:
    #return 'jsvIsStringEqual(parentName, "'+className+'")'
    n = 0;
    # IMPORTANT - we expect built-in objects to have their name stored
    # as a string in the varData element
    #checkOffsets = []
    #checkCharacters = []
    #for ch in className:
    #  checkOffsets.append(n)
    #  checkCharacters.append(ch)
    #  n = n + 1        
    #checkOffsets.append(n)
    #checkCharacters.append("\\0")
    #return createStringCompare("parent->varData.str", checkOffsets, checkCharacters)
    return "jswIsParentNamed(parent, \""+className+"\")"
  else:
    if className=="String": return "jsvIsString(parent)"
    if className=="Pin": return "jsvIsPin(parent)"
    if className=="Integer": return "jsvIsInt(parent)"
    if className=="Double": return "jsvIsFloat(parent)"
    if className=="Number": return "jsvIsInt(parent) || jsvIsFloat(parent)"
    if className=="Object": return "parent" # we assume all are objects
    if className=="Array": return "jsvIsArray(parent)"
    if className=="ArrayBuffer": return "jsvIsArrayBuffer(parent) || jsvIsArrayBufferView(parent)"
    if className=="Function": return "jsvIsFunction(parent)"
    return "jswHasConstructorNamed(parent, \""+className+"\")"
    #n = 0
    #checkOffsets = []
    #checkCharacters = []
    #for ch in className:
    #  checkOffsets.append(n)
    #  checkCharacters.append(ch)
    #  n = n + 1        
    #checkOffsets.append(n)
    #checkCharacters.append("\\0")
    # return createStringCompare("constructorName->varData.str", checkOffsets, checkCharacters)

print("Building decision tree")
tree = {}
for jsondata in jsondatas:
  if "name" in jsondata:
    jsondata["static"] = not (jsondata["type"]=="property" or jsondata["type"]=="method")

    classTest = "!parent"
    if not jsondata["type"]=="constructor":
      if "class" in jsondata: classTest = getTestFor(jsondata["class"], jsondata["static"])
    jsondata["classTest"] = classTest
    # now add to tree
    addToTree(tree, jsondata["name"], jsondata)
#tree = sorted(tree, key=common.get_name_or_space)
# ------------------------------------------------------------------------------------------------------
#print json.dumps(tree, sort_keys=True, indent=2)
# ------------------------------------------------------------------------------------------------------    
print("Outputting decision tree")
wrapperFile = open('src/jswrapper.c', 'w')

def codeOut(s): 
#  print str(s)
  wrapperFile.write(s+"\n");

def getUnLockGetter(varType, name, funcName):
  if varType=="float": return "jsvGetFloatAndUnLock("+name+")"
  if varType=="int": return "jsvGetIntegerAndUnLock("+name+")"
  if varType=="bool": return "jsvGetBoolAndUnLock("+name+")"
  if varType=="pin": return "jshGetPinFromVarAndUnLock("+name+")"
  print("ERROR: getUnLockGetter: Unknown type '"+varType+"' for "+funcName+":"+name)
  exit(1)

def getCreator(varType, value, funcName):
  if varType=="float": return "jsvNewFromFloat("+value+")"
  if varType=="int": return "jsvNewFromInteger("+value+")"
  if varType=="bool": return "jsvNewFromBool("+value+")"
  if varType=="JsVar": return value
  print("ERROR: getCreator: Unknown type '"+varType+"'"+"' for "+funcName)
  exit(1)

def codeOutFunctionObject(indent, obj):
  codeOut(indent+"// Object "+obj["name"]+"  ("+obj["filename"]+")")
  if "#if" in obj: codeOut(indent+"#if "+obj["#if"]);
  codeOut(indent+"jspParseVariableName();")
  codeOut(indent+"return jspNewObject(jsiGetParser(), \""+obj["name"]+"\", \""+obj["instanceof"]+"\");");
  if "#if" in obj: codeOut(indent+"#endif //"+obj["#if"]);

def codeOutFunction(indent, func):
  if func["type"]=="object":
    codeOutFunctionObject(indent, func)
    return
  name = ""
  if "class" in func:
    name = name + func["class"]+".";
  name = name + func["name"]
  print(name)
  codeOut(indent+"// "+name+"  ("+func["filename"]+")")
  hasThis = func["type"]=="property" or func["type"]=="method"
  if ("generate" in func) or ("generate_full" in func):
    argNames = ["a","b","c","d"];
    params = []
    if "params" in func: params = func["params"]
    if len(params)==0: 
      if func["type"]=="variable" or common.is_property(func):
        codeOut(indent+"jspParseVariableName();")
      else:
        codeOut(indent+"jspParseEmptyFunction();")
    elif len(params)==1 and params[0][1]!="JsVarName": 
      codeOut(indent+"JsVar *"+params[0][0]+" = jspParseSingleFunction();")
    elif len(params)<9:
      funcName = "jspParseFunction8"
      paramCount = 8
      if len(params)<5:  
        funcName = "jspParseFunction"
        paramCount = 4
      paramDefs = []
      paramPtrs = []
      skipNames = "0"
      n = 0
      letters = ["A","B","C","D","E","F","G","H"];
      for param in params:
        paramDefs.append("*"+param[0])
        paramPtrs.append("&"+param[0])
        if param[1]=="JsVarName": skipNames = skipNames + "|JSP_NOSKIP_"+letters[n]
        n = n + 1
      while len(paramPtrs)<paramCount: paramPtrs.append("0")
      codeOut(indent+"JsVar "+', '.join(paramDefs)+";");
      codeOut(indent+funcName+"("+skipNames+", "+', '.join(paramPtrs)+");");
    else:
      print("ERROR: codeOutFunction unknown number of args "+str(len(params)))
      exit(1)
    if "generate" in func:
      commandargs = [];
      if hasThis:      
        commandargs.append("parent")
      if "needs_parentName" in func:
        commandargs.append("parentName")
      for param in params:
        if param[1]=="JsVar" or param[1]=="JsVarName":
          commandargs.append(param[0]);
        else:
          commandargs.append(getUnLockGetter(param[1], param[0], func["name"]));
      command = func["generate"]+"("+ ', '.join(commandargs) +")";
    else:	
      command = func["generate_full"];
    
    if "return" in func: 
      codeOut(indent+"JsVar *_r = " + getCreator(func["return"][0], command, func["name"])+";");
    else:
      codeOut(indent+command+";");

    # note: generate_full doesn't use commandargs, so doesn't unlock
    for param in params:
      if "generate_full" in func or param[1]=="JsVar" or param[1]=="JsVarName":
        codeOut(indent+"jsvUnLock("+param[0]+");");

    if "return" in func: 
      codeOut(indent+"return _r;");
    else:
      codeOut(indent+"return 0;");
  elif "wrap" in func:
    codeOut(indent+"return "+func["wrap"]+"(parent, parentName);")
  else:
    print("ERROR: codeOutFunction: Function '"+func["name"]+"' does not have generate, generate_full or wrap elements'")
    exit(1)

def codeOutFunctionList(indent, funcList):
  for func in funcList:
    codeOut(indent+"if ("+func["classTest"]+") {")
    codeOutFunction(indent+"  ", func)
    codeOut(indent+"}")

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
      # if char is '' then we've reached the end...
      while len(charTree)==1 and char!='':
        charOffset = charOffset + 1
        char = charTree.keys()[0]
        charTree = charTree[char]        
        checkOffsets.append(charOffset)
        if char=='': checkCharacters.append('\\0')
        else: checkCharacters.append(char)
      line = line + "if (" + createStringCompare("name", checkOffsets, checkCharacters) + ") {"
      codeOut(line)   
      if char=='':
        # charTree is now a list of possible functions
        codeOutFunctionList(indent+"  ", charTree)
      else:
        codeOutTree(indent+"  ", charTree, charOffset+1)
  # Now we do the handling part!
  if "" in tree:
    func = tree[""]
    line = indent;
    if first: first = False
    else: line = line + "} else "
    line = line + "if (name["+str(offset)+"]==0) {";
    codeOut(line)
    codeOutFunctionList(indent+"  ", func)
  if not first:
    codeOut(indent+'}')




print("")
print("") 

codeOut('// Automatically generated wrapper file ')
codeOut('// Generated by scripts/build_jsfunctions.py');
codeOut('');
codeOut('#include "jswrapper.h"');
for include in includes:
  codeOut('#include "'+include+'"');
codeOut('');
codeOut('');
codeOut("#if( 'q\\0\\0\\0' & 'q' )")
codeOut('  #error( "architecture is big-endian. need to test and make sure this works" )')
codeOut("#endif")
codeOut('// beware big endian!');
codeOut('#define CH(a,b,c,d) ( ((d)<<24) | ((c)<<16) | ((b)<<8) | (a) )');
codeOut('');
codeOut('');
codeOut('bool jswHasConstructorNamed(JsVar *parent, const char *str) {');
codeOut('    JsVar *constructorName = jsvIsObject(parent)?jsvSkipOneNameAndUnLock(jsvFindChildFromString(parent, JSPARSE_CONSTRUCTOR_VAR, false)):0;')
codeOut('    return  constructorName && jsvIsName(constructorName) && jsvIsStringEqual(constructorName, str);')
codeOut('}');
codeOut('');
codeOut('bool jswIsParentNamed(JsVar *parent, const char *str) {');
codeOut('  // IMPORTANT - we expect built-in objects to have their name stored');
codeOut('  // as a string in the varData element');
codeOut('    return  parent && strcmp(&parent->varData.str[0], str)==0;')
codeOut('}');
codeOut('');
codeOut('');

codeOut('JsVar *jswHandleFunctionCall(JsVar *parent, JsVar *parentName, const char *name) {')
codeOut('  switch (name[0]) {')
for firstChar in tree:
  codeOut("    case '"+firstChar+"': {")
  print("char '"+firstChar+"'")
  codeOutTree("          ", tree[firstChar], 1)
  codeOut("                 break;");
  codeOut("               }")
codeOut('  } /*switch*/')
codeOut('  // Handle pin names - eg LED1 or D5 (this is hardcoded in build_jsfunctions.py)')
codeOut('  int pin = jshGetPinFromString(name);')
codeOut('  if (pin>=0) {')
codeOut('    jspParseVariableName();')
codeOut('    return jsvNewFromPin(pin);')
codeOut('  }')
codeOut('  return JSW_HANDLEFUNCTIONCALL_UNHANDLED;')
codeOut('}')


#codeOut('  if (parent) {')
#codeOut('    // ------------------------------------------ METHODS ON OBJECT')
#if "parent" in tree:
#  codeOutTree("    ", tree["parent"], 0)
#codeOut('    // ------------------------------------------ INSTANCE + STATIC METHODS')
#for className in tree:
#  if className!="parent" and  className!="!parent" and not "parentName" in className and not "constructorName" in className:
#    codeOut('    if ('+className+') {')
#    codeOutTree("      ", tree[className], 0)
#    codeOut("    }")
#codeOut('    // ------------------------------------------ INSTANCE METHODS WE MUST CHECK CONSTRUCTOR FOR')
#codeOut('    JsVar *constructorName = jsvIsObject(parent)?jsvSkipOneNameAndUnLock(jsvFindChildFromString(parent, JSPARSE_CONSTRUCTOR_VAR, false)):0;')
#codeOut('    if (constructorName && jsvIsName(constructorName)) {')
#first = True
#for className in tree:
#  if "constructorName" in className:
#    if first:
#      codeOut('    if ('+className+') {')
#      first = False
#    else:
#      codeOut('    } else if ('+className+') {')
#    codeOut('      jsvUnLock(constructorName);constructorName=0;')
#    codeOutTree("          ", tree[className], 0)
#if not first:
#  codeOut("    } else ")
#codeOut('      jsvUnLock(constructorName);');
#codeOut('    }')
#codeOut('  } else { /* if (!parent) */')
#codeOut('    // ------------------------------------------ FUNCTIONS')
#codeOut('    // Handle pin names - eg LED1 or D5 (this is hardcoded in build_jsfunctions.py)')
#codeOut('    int pin = jshGetPinFromString(name);')
#codeOut('    if (pin>=0) {')
#codeOut('      jspParseVariableName();')
#codeOut('      return jsvNewFromPin(pin);')
#codeOut('    }')
#if "!parent" in tree:
#  codeOutTree("    ", tree["!parent"], 0)
#codeOut('  }');
#codeOut('  return JSW_HANDLEFUNCTIONCALL_UNHANDLED;')
#codeOut('}')
#codeOut('')


# ---------------------------------------------------------------------------------------------
# ---------------------------------------------------------------------------------------------
# ---------------------------------------------------------------------------------------------
builtinChecks = []
notRealObjects = []
for jsondata in jsondatas:
  if "class" in jsondata:
    check = 'strcmp(name, "'+jsondata["class"]+'")==0';
    if "not_real_object" in jsondata:
      notRealObjects.append(check)
    if not check in builtinChecks:
      builtinChecks.append(check)


codeOut('bool jswIsBuiltInObject(const char *name) {') 
codeOut('  return\n'+" ||\n    ".join(builtinChecks)+';')
codeOut('}')


