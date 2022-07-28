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
#include "jsflash.h" // look in flash for modules
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
  "return" : ["JsVar","The result of evaluating the string"],
  "typescript": [
    "declare function require<T extends keyof Libraries>(moduleName: T): Libraries[T];",
    "declare function require<T extends Exclude<string, keyof Libraries>>(moduleName: T): any;"
  ]
}
Load the given module, and return the exported functions and variables.

For example:

```
var s = require("Storage");
s.write("test", "hello world");
print(s.read("test"));
// prints "hello world"
```

Check out [the page on Modules](/Modules) for an explanation of what modules are
and how you can use them.
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

#ifndef SAVE_ON_FLASH
  // Has it been manually saved to Flash Storage? Use Storage support.
  if ((!moduleExport) && (strlen(moduleNameBuf) <= JSF_MAX_FILENAME_LENGTH)) {
    JsfFileName storageName = jsfNameFromString(moduleNameBuf);
    JsVar *storageFile = jsfReadFile(storageName,0,0);
    if (storageFile) {
      moduleExport = jspEvaluateModule(storageFile);
      jsvUnLock(storageFile);
    }
  }
#endif


  // Ok - it's not built-in as native or storage.
  // Look and see if it's compiled-in as a C-String of JS - if so get the actual text and execute it
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
   

  if (moduleExport) { // Found - now save module
    JsVar *moduleList = jswrap_modules_getModuleList();
    if (moduleList)
      jsvObjectSetChild(moduleList, moduleNameBuf, moduleExport);
    jsvUnLock(moduleList);
  } else { // module not found...
#ifdef ESPRUINOWIFI
    // Big hack to work around module renaming. IF EspruinoWiFi wasn't found, rename it
    if (!strcmp(moduleNameBuf,"EspruinoWiFi")) {
      JsVar *n = jsvNewFromString("Wifi");
      JsVar *r = jswrap_require(n);
      jsvUnLock(n);
      return r;
    }
#endif
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
