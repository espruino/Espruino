#!/usr/bin/python
import subprocess;
import re;
import json;
import sys;

jsondatas = []
print "Scanning for jswrap.c files"
jswraps = subprocess.check_output(["find", ".", "-name", "jswrap.c"]).strip().split("\n")
for jswrap in jswraps:
  print "Scanning "+jswrap
  code = open(jswrap, "r").read()

  for comment in re.findall(r"/\*JSON[^\*]*\*/", code, re.VERBOSE | re.MULTILINE | re.DOTALL):
    jsonstring = comment[6:-2]
    print "Parsing "+jsonstring
    try:
      jsondatas.append(json.loads(jsonstring))
    except ValueError as e:
      print "JSON PARSE FAILED -",  e
      exit(1)
    except:
      print "JSON PARSE FAILED",  sys.exc_info()[0]
      exit(1)
print "Scanning finished."


def treewalk(tree, name, jsondata):
  if len(name)==0:
    tree[""] = jsondata
  else:	
    firstchar = name[:1]
    if not firstchar in tree: tree[firstchar] = {}
    treewalk(tree[firstchar], name[1:], jsondata)

# ------------------------------------------------------------------------------------------------------

print "Building decision tree"
tree = {}
for jsondata in jsondatas:
  if "class" in jsondata: className = jsondata["class"]
  else: className = ""
  if not className in tree: 
    print "Adding "+className+" to tree"
    tree[className] = {}
  treewalk(tree[className], jsondata["name"], jsondata)
  classTree = tree[className]

# ------------------------------------------------------------------------------------------------------
print json.dumps(tree, sort_keys=True, indent=2)
# ------------------------------------------------------------------------------------------------------    
print "Outputting decision tree"

def codeOut(s): print str(s)

def getTestFor(className):
  if className=="": return "!parent"
  else: return 'parentName && jsvIsStringEqual(parentName, "'+className+'")'
#  else: return "is"+className+"(parent)"

def getUnLockGetter(varType, name):
  if varType=="float": return "jsvGetFloatAndUnLock("+name+")"
  print "ERROR: getGetter: Unknown type '"+varType+"'"
  exit(1)

def getCreator(varType, value):
  if varType=="float": return "jsvNewFromFloat("+value+")"
  print "ERROR: getCreator: Unknown type '"+varType+"'"
  exit(1)

def codeOutFunction(indent, func):
  if "generate" in func:
    argNames = ["a","b","c","d"];
    params = []
    if "params" in func: params = func["params"]
    if len(params)==0: 
      codeOut(indent+"jspParseEmptyFunction();")
    elif len(params)==1: 
      codeOut(indent+"JsVar *"+params.keys()[0]+" = jspParseSingleFunction();")
    elif len(params)==2: 
      paramDefs = []
      paramPtrs = []
      for paramName in params:
        paramDefs.append("*"+paramName)
        paramPtrs.append("&"+paramName)
      while len(paramPtrs)<4: paramPtrs.append("0")
      codeOut(indent+"JsVar "+', '.join(paramDefs)+";");
      codeOut(indent+"jspParseFunction(0, "+', '.join(paramPtrs)+");");
    else:
      print "ERROR: codeOutFunction unknown nukber of args "+str(len(params))
      exit(1)
    commandargs = [];
    for paramName in params:
      param = params[paramName]
      commandargs.append(getUnLockGetter(param[0], paramName));
    command = func["generate"]+"("+ ', '.join(commandargs) +")";
    line = "return " + getCreator(func["return"][0], command)+";";
    codeOut(indent+line);
  elif "wrap" in func:
    codeOut(indent+"return "+func["wrap"]+"(parent, parentName);")
  else:
    print "ERROR: codeOutFunction: Function '"+func["name"]+"' does not have generate or wrap elements'"
    exit(1)

def codeOutTree(indent, tree, offset):
  first = True
  for char in tree:
    if char!="":
      charOffset = offset
      charTree = tree[char]
      line = indent
      if first: first = False
      else: line = line + "} else "
      line = line + "if (name["+str(charOffset)+"]=='"+char+"'"
      while len(charTree)==1:
        charOffset = charOffset + 1
        char = charTree.keys()[0]
        charTree = charTree[char]        
        line = line + " && name["+str(charOffset)+"]=="
        if char=='': line = line + "0"
        else: line = line + "'"+char+"'"
      line = line + ") {"
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




print ""
print "" 

codeOut('JsVar *handleFunctionCall(JsExecInfo *execInfo, JsVar *parent, JsVar *parentName, const char *name) {')

first = True;
for className in tree:
  line = "  "
  if first: first = False
  else: line = line + '} else '
  line = line + 'if ('+getTestFor(className)+') {'
  codeOut(line)
  codeOutTree("    ", tree[className], 0)

if not first:
  codeOut('  }')
  codeOut('return 0; /*unhandled*/')
codeOut('}')
 
