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
 * Contains built-in functions for SD card access, based on node.js 'fs' module
 * ----------------------------------------------------------------------------
 */

#include "jswrap_fs.h"
#include "jswrap_file.h"
#include "jsutils.h"
#include "jsvar.h"
#include "jsparse.h"
#include "jsinteractive.h"

#ifndef LINUX
#include "ff.h" // filesystem stuff
#else
#include <stdio.h>
#include <dirent.h> // for readdir
#endif


/*JSON{ "type":"library",
        "class" : "fs",
        "description" : ["This library handles interfacing with a FAT32 filesystem on an SD card. The API is designed to be similar to node.js's - However Espruino does not currently support asynchronous file IO, so the functions behave like node.js's xxxxSync functions. Versions of the functions with 'Sync' after them are also provided for compatibility.",
                         "Currently this provides minimal file IO - it's great for logging and loading/saving settings, but not good for loading large amounts of data as you will soon fill your memory up.",
                         "It is currently only available on boards that contain an SD card slot, such as the Olimexino and the HY. It can not currently be added to boards that did not ship with a card slot.",
                         "To use this, you must type ```var fs = require('fs')``` to get access to the library" ]
}*/

#ifndef LINUX
#define JS_DIR_BUF_SIZE 64
#else
#define JS_DIR_BUF_SIZE 256
#define FR_OK (0)
#endif

#ifndef LINUX

#if _USE_LFN
  #define GET_FILENAME(Finfo) *Finfo.lfname ? Finfo.lfname : Finfo.fname
#else
  #define GET_FILENAME(Finfo) Finfo.fname
#endif

FATFS jsfsFAT;
#endif

void jsfsReportError(const char *msg, FRESULT res) {
  const char *errStr = "UNKNOWN";
  if (res==FR_OK             ) errStr = "OK";
#ifndef LINUX
  else if (res==FR_DISK_ERR       ) errStr = "DISK_ERR";
  else if (res==FR_INT_ERR        ) errStr = "INT_ERR";
  else if (res==FR_NOT_READY      ) errStr = "NOT_READY";
  else if (res==FR_NO_FILE        ) errStr = "NO_FILE";
  else if (res==FR_NO_PATH        ) errStr = "NO_PATH";
  else if (res==FR_INVALID_NAME   ) errStr = "INVALID_NAME";
  else if (res==FR_DENIED         ) errStr = "DENIED";
  else if (res==FR_EXIST          ) errStr = "EXIST";
  else if (res==FR_INVALID_OBJECT ) errStr = "INVALID_OBJECT";
  else if (res==FR_WRITE_PROTECTED) errStr = "WRITE_PROTECTED";
  else if (res==FR_INVALID_DRIVE  ) errStr = "INVALID_DRIVE";
  else if (res==FR_NOT_ENABLED    ) errStr = "NOT_ENABLED";
  else if (res==FR_NO_FILESYSTEM  ) errStr = "NO_FILESYSTEM";
  else if (res==FR_MKFS_ABORTED   ) errStr = "MKFS_ABORTED";
  else if (res==FR_TIMEOUT        ) errStr = "TIMEOUT";
#endif
  jsError("%s : %s", msg, errStr);
}

bool fat_initialised = false;

bool jsfsInit() {
#ifndef LINUX
  if (!fat_initialised) {
    FRESULT res;
    if ((res = f_mount(&jsfsFAT, "", 1/*immediate*/)) != FR_OK) {
      jsfsReportError("Unable to mount SD card", res);
      return false;
    }
    fat_initialised = true;
  }
#endif
  return true;
}



/* Unmount...
    if (res==FR_OK) {
      jsiConsolePrint("Unmounting...\n");
      res = f_mount(0, 0);
    }
 */

/*JSON{ "type":"kill", "generate" : "jswrap_fs_kill", "ifndef" : "SAVE_ON_FLASH" }*/
void jswrap_fs_kill() { // Uninitialise fat
#ifndef LINUX
  if (fat_initialised) {
    fat_initialised = false;
    f_mount(0, 0, 0);
  }
#endif
}


/*JSON{  "type" : "staticmethod", "class" : "fs", "name" : "readdir",
         "generate" : "jswrap_fs_readdir",
         "description" : [ "List all files in the supplied directory, returning them as an array of strings.", "NOTE: Espruino does not yet support Async file IO, so this function behaves like the 'Sync' version." ],
         "params" : [ [ "path", "JsVar", "The path of the directory to list. If it is not supplied, '' is assumed, which will list the root directory" ] ],
         "return" : [ "JsVar", "An array of filename strings (or undefined if the directory couldn't be listed)" ]
}*/
/*JSON{  "type" : "staticmethod", "class" : "fs", "name" : "readdirSync", "ifndef" : "SAVE_ON_FLASH",
         "generate" : "jswrap_fs_readdir",
         "description" : [ "List all files in the supplied directory, returning them as an array of strings." ],
         "params" : [ [ "path", "JsVar", "The path of the directory to list. If it is not supplied, '' is assumed, which will list the root directory" ] ],
         "return" : [ "JsVar", "An array of filename strings (or undefined if the directory couldn't be listed)" ]
}*/

JsVar *jswrap_fs_readdir(JsVar *path) {
  JsVar *arr = 0; // undefined unless we can open card

  char pathStr[JS_DIR_BUF_SIZE] = "";
  if (!jsvIsUndefined(path))
    jsvGetString(path, pathStr, JS_DIR_BUF_SIZE);
#ifdef LINUX
  if (!pathStr[0]) strcpy(pathStr, "."); // deal with empty readdir
#endif

  FRESULT res = 0;
  if (jsfsInit()) {
#ifndef LINUX
    DIR dirs;
    if ((res=f_opendir(&dirs, pathStr)) == FR_OK) {
      FILINFO Finfo;
#if _USE_LFN!=0
      char lfnBuf[_MAX_LFN+1];
      Finfo.lfname = lfnBuf;
      Finfo.lfsize = sizeof(lfnBuf);
#endif
#else
    DIR *dir = opendir(pathStr);
    if(dir) {
#endif
      arr = jsvNewWithFlags(JSV_ARRAY);
      if (arr) { // could be out of memory
#ifndef LINUX
        while (((res=f_readdir(&dirs, &Finfo)) == FR_OK) && Finfo.fname[0]) {
          char *fn = GET_FILENAME(Finfo);
#else
        struct dirent *pDir=NULL;
        while((pDir = readdir(dir)) != NULL) {
          char *fn = (*pDir).d_name;
#endif
          JsVar *fnVar = jsvNewFromString(fn);
          if (fnVar) {// out of memory?
            jsvArrayPush(arr, fnVar);
            jsvUnLock(fnVar);
          }
        }
      }
#ifdef LINUX
      closedir(dir);
#endif
    }
  }
  if (res) jsfsReportError("Unable to list files", res);
  return arr;
}

/*JSON{  "type" : "staticmethod", "class" : "fs", "name" : "writeFile",
         "generate_full" : " jswrap_fs_writeOrAppendFile(path, data, false)",
         "description" : [ "Write the data to the given file", "NOTE: Espruino does not yet support Async file IO, so this function behaves like the 'Sync' version." ],
         "params" : [ [ "path", "JsVar", "The path of the file to write" ],
                      [ "data", "JsVar", "The data to write to the file" ] ],
         "return" : [ "bool", "True on success, false on failure" ]
}*/
/*JSON{  "type" : "staticmethod", "class" : "fs", "name" : "writeFileSync", "ifndef" : "SAVE_ON_FLASH",
         "generate_full" : " jswrap_fs_writeOrAppendFile(path, data, false)",
         "description" : [ "Write the data to the given file" ],
         "params" : [ [ "path", "JsVar", "The path of the file to write" ],
                      [ "data", "JsVar", "The data to write to the file" ] ],
         "return" : [ "bool", "True on success, false on failure" ]
}*/
/*JSON{  "type" : "staticmethod", "class" : "fs", "name" : "appendFile",
         "generate_full" : " jswrap_fs_writeOrAppendFile(path, data, true)",
         "description" : [ "Append the data to the given file, created a new file if it doesn't exist", "NOTE: Espruino does not yet support Async file IO, so this function behaves like the 'Sync' version." ],
         "params" : [ [ "path", "JsVar", "The path of the file to write" ],
                      [ "data", "JsVar", "The data to write to the file" ] ],
         "return" : [ "bool", "True on success, false on failure" ]
}*/
/*JSON{  "type" : "staticmethod", "class" : "fs", "name" : "appendFileSync", "ifndef" : "SAVE_ON_FLASH",
         "generate_full" : "jswrap_fs_writeOrAppendFile(path, data, true)",
         "description" : [ "Append the data to the given file, created a new file if it doesn't exist" ],
         "params" : [ [ "path", "JsVar", "The path of the file to write" ],
                      [ "data", "JsVar", "The data to write to the file" ] ],
         "return" : [ "bool", "True on success, false on failure" ]
}*/
bool jswrap_fs_writeOrAppendFile(JsVar *path, JsVar *data, bool append) {
  JsVar *fMode = jsvNewFromString(append ? "a" : "w");
  JsVar *f = jswrap_E_openFile(path, fMode);
  jsvUnLock(fMode);
  if (!f) return 0;
  size_t amt = jswrap_file_write(f, data);
  jswrap_file_close(f);
  jsvUnLock(f);
  return amt>0;
}

/*JSON{  "type" : "staticmethod", "class" : "fs", "name" : "readFile",
         "generate" : "jswrap_fs_readFile",
         "description" : [ "Read all data from a file and return as a string", "NOTE: Espruino does not yet support Async file IO, so this function behaves like the 'Sync' version." ],
         "params" : [ [ "path", "JsVar", "The path of the file to read" ] ],
         "return" : [ "JsVar", "A string containing the contents of the file (or undefined if the file doesn't exist)" ]
}*/
/*JSON{  "type" : "staticmethod", "class" : "fs", "name" : "readFileSync", "ifndef" : "SAVE_ON_FLASH",
         "generate" : "jswrap_fs_readFile",
         "description" : [ "Read all data from a file and return as a string.","**Note:** The size of files you can load using this method is limited by the amount of available RAM. To read files a bit at a time, see the `File` class." ],
         "params" : [ [ "path", "JsVar", "The path of the file to read" ] ],
         "return" : [ "JsVar", "A string containing the contents of the file (or undefined if the file doesn't exist)" ]
}*/
JsVar *jswrap_fs_readFile(JsVar *path) {
  JsVar *fMode = jsvNewFromString("r");
  JsVar *f = jswrap_E_openFile(path, fMode);
  jsvUnLock(fMode);
  if (!f) return 0;
  JsVar *buffer = jswrap_file_read(f, 0x7FFFFFFF);
  jswrap_file_close(f);
  jsvUnLock(f);
  return buffer;
}

  /*JSON{  "type" : "staticmethod", "class" : "fs", "name" : "unlink", "ifndef" : "SAVE_ON_FLASH",
           "generate" : "jswrap_fs_unlink",
           "description" : [ "Delete the given file", "NOTE: Espruino does not yet support Async file IO, so this function behaves like the 'Sync' version." ],
           "params" : [ [ "path", "JsVar", "The path of the file to delete" ] ],
           "return" : [ "bool", "True on success, or false on failure" ]
  }*/
  /*JSON{  "type" : "staticmethod", "class" : "fs", "name" : "unlinkSync", "ifndef" : "SAVE_ON_FLASH",
           "generate" : "jswrap_fs_unlink",
           "description" : [ "Delete the given file" ],
           "params" : [ [ "path", "JsVar", "The path of the file to delete" ] ],
           "return" : [ "bool", "True on success, or false on failure" ]
  }*/
bool jswrap_fs_unlink(JsVar *path) {
  char pathStr[JS_DIR_BUF_SIZE] = "";
  if (!jsvIsUndefined(path))
    jsvGetString(path, pathStr, JS_DIR_BUF_SIZE);

#ifndef LINUX
  FRESULT res = 0;
  if (jsfsInit()) {
    res = f_unlink(pathStr);
  }
#else
  FRESULT res = remove(pathStr);
#endif

  if (res) {
    jsfsReportError("Unable to delete file", res);
    return false;
  }
  return true;
}

