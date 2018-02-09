/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2013 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * This file is designed to be parsed during the build process
 *
 * JavaScript Functions for handling Modules
 * ----------------------------------------------------------------------------
 */
#include "jswrap_functions.h"
#include "jslex.h"
#include "jsvar.h"
#include "jsparse.h"
#include "jsinteractive.h"
#include "jswrapper.h"
#ifdef USE_FILESYSTEM
#include "jswrap_fs.h"
#endif

/*JSON{
  "type" : "class",
  "class" : "Modules"
}
Built-in class that caches the modules used by the `require` command
 */

static JsVar *jswrap_modules_getModuleList() {
  return jsvObjectGetChild(execInfo.hiddenRoot, JSPARSE_MODULE_CACHE_NAME, JSV_OBJECT);
}

/*JSON{
  "type" : "function",
  "name" : "require",
  "generate" : "jswrap_require",
  "params" : [
    ["moduleName","JsVar","A String containing the name of the given module"]
  ],
  "return" : ["JsVar","The result of evaluating the string"]
}
Load the given module, and return the exported functions
 */
JsVar *jswrap_require(JsVar *moduleName) {
  if (!jsvIsString(moduleName)) {
    jsExceptionHere(JSET_TYPEERROR, "Expecting a module name as a string, but got %t", moduleName);
    return 0;
  }
  char moduleNameBuf[128];
  if (jsvGetString(moduleName, moduleNameBuf, sizeof(moduleNameBuf))>=sizeof(moduleNameBuf)) {
    jsExceptionHere(JSET_TYPEERROR, "Module name too long (max 128 chars)");
    return 0;
  }
#ifdef ESPRUINOWIFI
  // Big hack to work around module renaming
  if (!strcmp(moduleNameBuf,"EspruinoWiFi"))
    strcpy(moduleNameBuf,"Wifi");
#endif


  // Search to see if we have already loaded this module
  JsVar *moduleList = jswrap_modules_getModuleList();
  if (!moduleList) return 0; // out of memory
  JsVar *moduleExport = jsvSkipNameAndUnLock(jsvFindChildFromString(moduleList, moduleNameBuf, false));
  jsvUnLock(moduleList);
  if (moduleExport) {
    // Found the module!
    return moduleExport;
  }

  // Now check if it is built-in (as an actual native function)
  void *builtInLib = jswGetBuiltInLibrary(moduleNameBuf);
  if (builtInLib) {
    // create a 'fake' module that Espruino can use to map its built-in functions against
    moduleExport = jsvNewNativeFunction(builtInLib, 0);
  } 
  // Ok - it's not built-in as native. We want to get the actual text and execute it
  // Look and see if it's built-in as JS
  if (!moduleExport) {
    const char *builtInJS = jswGetBuiltInJSLibrary(moduleNameBuf);
    if (builtInJS) {
      JsVar *fileContents = jsvNewNativeString((char*)builtInJS, strlen(builtInJS));       
      if (fileContents) {
        moduleExport = jspEvaluateModule(fileContents);
        jsvUnLock(fileContents);
      }
    }
  }
  // If we have filesystem support, look on the filesystem
#ifdef USE_FILESYSTEM
  if (!moduleExport) {
    JsVar *fileContents = 0;        
    JsVar *modulePath = jsvNewFromString("node_modules/");
    if (modulePath) { // out of memory
      jsvAppendString(modulePath, moduleNameBuf);
      jsvAppendString(modulePath,".js");
      fileContents = jswrap_fs_readFile(modulePath);
      jsvUnLock(modulePath);
      JsVar *exception = jspGetException();
      if (exception) {  // throw away exception & file if we had one
        execInfo.execute = execInfo.execute & (JsExecFlags)~EXEC_EXCEPTION;
        jsvUnLock2(fileContents, exception);
        fileContents = 0;
      }
      if (fileContents && jsvGetStringLength(fileContents)>0)
        moduleExport = jspEvaluateModule(fileContents);
      jsvUnLock(fileContents);
    }
  }
#endif    
   
  // Now save module
  if (moduleExport) { // could have been out of memory
    JsVar *moduleList = jswrap_modules_getModuleList();
    if (moduleList)
      jsvObjectSetChild(moduleList, moduleNameBuf, moduleExport);
    jsvUnLock(moduleList);
  } else {
    // nope. no module
    jsExceptionHere(JSET_ERROR, "Module %s not found", moduleNameBuf);
  }

  return moduleExport;
}

/*JSON{
  "type" : "staticmethod",
  "class" : "Modules",
  "name" : "getCached",
  "generate" : "jswrap_modules_getCached",
  "return" : ["JsVar","An array of module names"]
}
Return an array of module names that have been cached
 */
JsVar *jswrap_modules_getCached() {
  JsVar *arr = jsvNewEmptyArray();
  if (!arr) return 0; // out of memory

  JsVar *moduleList = jswrap_modules_getModuleList();
  if (!moduleList) return arr; // out of memory

  JsvObjectIterator it;
  jsvObjectIteratorNew(&it, moduleList);
  while (jsvObjectIteratorHasValue(&it)) {
    JsVar *idx = jsvObjectIteratorGetKey(&it);
    JsVar *idxCopy  = jsvCopyNameOnly(idx, false, false);
    jsvArrayPushAndUnLock(arr, idxCopy);
    jsvUnLock(idx);
    jsvObjectIteratorNext(&it);
  }
  jsvObjectIteratorFree(&it);
  jsvUnLock(moduleList);
  return arr;
}

/*JSON{
  "type" : "staticmethod",
  "class" : "Modules",
  "name" : "removeCached",
  "generate" : "jswrap_modules_removeCached",
  "params" : [
    ["id","JsVar","The module name to remove"]
  ]
}
Remove the given module from the list of cached modules
 */
void jswrap_modules_removeCached(JsVar *id) {
  if (!jsvIsString(id)) {
    jsExceptionHere(JSET_ERROR, "The argument to removeCached must be a string");
    return;
  }
  JsVar *moduleList = jswrap_modules_getModuleList();
  if (!moduleList) return; // out of memory

  JsVar *moduleExportName = jsvFindChildFromVar(moduleList, id, false);
  if (!moduleExportName) {
    jsExceptionHere(JSET_ERROR, "Module %q not found", id);
  } else {
    jsvRemoveChild(moduleList, moduleExportName);
    jsvUnLock(moduleExportName);
  }

  jsvUnLock(moduleList);
}

/*JSON{
  "type" : "staticmethod",
  "class" : "Modules",
  "name" : "removeAllCached",
  "generate" : "jswrap_modules_removeAllCached"
}
Remove all cached modules
 */
void jswrap_modules_removeAllCached() {
  JsVar *moduleList = jswrap_modules_getModuleList();
  if (!moduleList) return; // out of memory
  jsvRemoveAllChildren(moduleList);
  jsvUnLock(moduleList);
}

/*JSON{
  "type" : "staticmethod",
  "class" : "Modules",
  "name" : "addCached",
  "generate" : "jswrap_modules_addCached",
  "params" : [
    ["id","JsVar","The module name to add"],
    ["sourcecode","JsVar","The module's sourcecode"]
  ]
}
Add the given module to the cache
 */
void jswrap_modules_addCached(JsVar *id, JsVar *sourceCode) {
  if (!jsvIsString(id) ||
      !(jsvIsString(sourceCode) || jsvIsFunction(sourceCode))) {
    jsExceptionHere(JSET_ERROR, "args must be addCached(string, string|function)");
    return;
  }

  JsVar *moduleList = jswrap_modules_getModuleList();
  if (!moduleList) return; // out of memory

  JsVar *moduleExport = jspEvaluateModule(sourceCode);
  if (!moduleExport) {
    jsExceptionHere(JSET_ERROR, "Unable to load module %q", id);
  } else {
    jsvObjectSetChildVar(moduleList, id, moduleExport);
    jsvUnLock(moduleExport);
  }
  jsvUnLock(moduleList);

}
