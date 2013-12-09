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
 * Contains built-in functions for SD card access
 * ----------------------------------------------------------------------------
 */

#include "jswrap_fat.h"
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
typedef int FRESULT;
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
  char buf[JS_ERROR_BUF_SIZE];
  strncpy(buf, msg, JS_ERROR_BUF_SIZE);
  if (res==FR_OK             ) strncat(buf," : OK", JS_ERROR_BUF_SIZE);
#ifndef LINUX
  if (res==FR_DISK_ERR       ) strncat(buf," : DISK_ERR", JS_ERROR_BUF_SIZE);
  if (res==FR_INT_ERR        ) strncat(buf," : INT_ERR", JS_ERROR_BUF_SIZE);
  if (res==FR_NOT_READY      ) strncat(buf," : NOT_READY", JS_ERROR_BUF_SIZE);
  if (res==FR_NO_FILE        ) strncat(buf," : NO_FILE", JS_ERROR_BUF_SIZE);
  if (res==FR_NO_PATH        ) strncat(buf," : NO_PATH", JS_ERROR_BUF_SIZE);
  if (res==FR_INVALID_NAME   ) strncat(buf," : INVALID_NAME", JS_ERROR_BUF_SIZE);
  if (res==FR_DENIED         ) strncat(buf," : DENIED", JS_ERROR_BUF_SIZE);
  if (res==FR_EXIST          ) strncat(buf," : EXIST", JS_ERROR_BUF_SIZE);
  if (res==FR_INVALID_OBJECT ) strncat(buf," : INVALID_OBJECT", JS_ERROR_BUF_SIZE);
  if (res==FR_WRITE_PROTECTED) strncat(buf," : WRITE_PROTECTED", JS_ERROR_BUF_SIZE);
  if (res==FR_INVALID_DRIVE  ) strncat(buf," : INVALID_DRIVE", JS_ERROR_BUF_SIZE);
  if (res==FR_NOT_ENABLED    ) strncat(buf," : NOT_ENABLED", JS_ERROR_BUF_SIZE);
  if (res==FR_NO_FILESYSTEM  ) strncat(buf," : NO_FILESYSTEM", JS_ERROR_BUF_SIZE);
  if (res==FR_MKFS_ABORTED   ) strncat(buf," : MKFS_ABORTED", JS_ERROR_BUF_SIZE);
  if (res==FR_TIMEOUT        ) strncat(buf," : TIMEOUT", JS_ERROR_BUF_SIZE);
#endif
  jsError(buf);
}

bool jsfsInit() {
#ifndef LINUX
  static bool inited = false;

  if (!inited) {
    FRESULT res;
    if ((res = f_mount(0, &jsfsFAT)) != FR_OK) {
      jsfsReportError("Unable to mount SD card", res);
      return false;
    }
    inited = true;
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


/*JSON{  "type" : "staticmethod", "class" : "fs", "name" : "readdir",
         "generate" : "wrap_fat_readdir",
         "description" : [ "List all files in the supplied directory, returning them as an array of strings.", "NOTE: Espruino does not yet support Async file IO, so this function behaves like the 'Sync' version." ],
         "params" : [ [ "path", "JsVar", "The path of the directory to list. If it is not supplied, '' is assumed, which will list the root directory" ] ],
         "return" : [ "JsVar", "An array of filename strings" ]
}*/
/*JSON{  "type" : "staticmethod", "class" : "fs", "name" : "readdirSync", "ifndef" : "SAVE_ON_FLASH",
         "generate" : "wrap_fat_readdir",
         "description" : [ "List all files in the supplied directory, returning them as an array of strings." ],
         "params" : [ [ "path", "JsVar", "The path of the directory to list. If it is not supplied, '' is assumed, which will list the root directory" ] ],
         "return" : [ "JsVar", "An array of filename strings" ]
}*/

JsVar *wrap_fat_readdir(JsVar *path) {
  JsVar *arr = jsvNewWithFlags(JSV_ARRAY);
  if (!arr) return 0; // out of memory

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
      while (((res=f_readdir(&dirs, &Finfo)) == FR_OK) && Finfo.fname[0]) {
        char *fn = GET_FILENAME(Finfo);
#else
    DIR *dir = opendir(pathStr);
    if(dir) {
      struct dirent *pDir=NULL;
      while((pDir = readdir(dir)) != NULL) {
        char *fn = (*pDir).d_name;
#endif
        JsVar *fnVar = jsvNewFromString(fn);
        if (fnVar) // out of memory?
          jsvArrayPush(arr, fnVar);
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
         "generate" : "wrap_fat_writeFile",
         "description" : [ "Write the data to the given file", "NOTE: Espruino does not yet support Async file IO, so this function behaves like the 'Sync' version." ],
         "params" : [ [ "path", "JsVar", "The path of the file to write" ],
                      [ "data", "JsVar", "The data to write to the file" ] ]
}*/
/*JSON{  "type" : "staticmethod", "class" : "fs", "name" : "writeFileSync", "ifndef" : "SAVE_ON_FLASH",
         "generate" : "wrap_fat_writeFile",
         "description" : [ "Write the data to the given file" ],
         "params" : [ [ "path", "JsVar", "The path of the file to write" ],
                      [ "data", "JsVar", "The data to write to the file" ] ]
}*/
/*JSON{  "type" : "staticmethod", "class" : "fs", "name" : "appendFile",
         "generate" : "wrap_fat_appendFile",
         "description" : [ "Append the data to the given file, created a new file if it doesn't exist", "NOTE: Espruino does not yet support Async file IO, so this function behaves like the 'Sync' version." ],
         "params" : [ [ "path", "JsVar", "The path of the file to write" ],
                      [ "data", "JsVar", "The data to write to the file" ] ]
}*/
/*JSON{  "type" : "staticmethod", "class" : "fs", "name" : "appendFileSync", "ifndef" : "SAVE_ON_FLASH",
         "generate" : "wrap_fat_appendFile",
         "description" : [ "Append the data to the given file, created a new file if it doesn't exist" ],
         "params" : [ [ "path", "JsVar", "The path of the file to write" ],
                      [ "data", "JsVar", "The data to write to the file" ] ]
}*/
void wrap_fat_writeOrAppendFile(JsVar *path, JsVar *data, bool append) {
  char pathStr[JS_DIR_BUF_SIZE] = "";
  if (!jsvIsUndefined(path))
    jsvGetString(path, pathStr, JS_DIR_BUF_SIZE);

  FRESULT res = 0;
  if (jsfsInit()) {
#ifndef LINUX
    FIL file;

    if ((res=f_open(&file, pathStr, FA_WRITE|(append ? FA_OPEN_ALWAYS : FA_CREATE_ALWAYS))) == FR_OK) {

      if (append) {
        // move to end of file to append data
        f_lseek(&file, file.fsize);
//        if (res != FR_OK) jsfsReportError("Unable to move to end of file", res);
      }
#else
      FILE *file = fopen(pathStr, append?"a":"w");
      if (file) {
#endif

      JsvStringIterator it;
      JsVar *dataString = jsvAsString(data, false);
      jsvStringIteratorNew(&it, dataString, 0);
      size_t toWrite = 0;
      size_t written = 0;

      while (jsvStringIteratorHasChar(&it) && res==FR_OK && written==toWrite) {

        // re-use pathStr buffer
        toWrite = 0;
        while (jsvStringIteratorHasChar(&it) && toWrite < JS_DIR_BUF_SIZE) {
          pathStr[toWrite++] = jsvStringIteratorGetChar(&it);
          jsvStringIteratorNext(&it);
        }
#ifndef LINUX
        res = f_write(&file, pathStr, toWrite, &written);
#else
        written = fwrite(pathStr, 1, toWrite, file);
#endif
      }
      jsvStringIteratorFree(&it);
      jsvUnLock(dataString);
#ifndef LINUX
      f_close(&file);
#else
      fclose(file);
#endif
    }
  }
  if (res) jsfsReportError("Unable to write file", res);
}
void wrap_fat_writeFile(JsVar *path, JsVar *data) {
  wrap_fat_writeOrAppendFile(path, data, false);
}
void wrap_fat_appendFile(JsVar *path, JsVar *data) {
  wrap_fat_writeOrAppendFile(path, data, true);
}

/*JSON{  "type" : "staticmethod", "class" : "fs", "name" : "readFile",
         "generate" : "wrap_fat_readFile",
         "description" : [ "Read all data from a file and return as a string", "NOTE: Espruino does not yet support Async file IO, so this function behaves like the 'Sync' version." ],
         "params" : [ [ "path", "JsVar", "The path of the file to read" ] ],
         "return" : [ "JsVar", "A string containing the contents of the file" ]
}*/
/*JSON{  "type" : "staticmethod", "class" : "fs", "name" : "readFileSync", "ifndef" : "SAVE_ON_FLASH",
         "generate" : "wrap_fat_readFile",
         "description" : [ "Read all data from a file and return as a string" ],
         "params" : [ [ "path", "JsVar", "The path of the file to read" ] ],
         "return" : [ "JsVar", "A string containing the contents of the file" ]
}*/
JsVar *wrap_fat_readFile(JsVar *path) {
  char pathStr[JS_DIR_BUF_SIZE] = "";
  if (!jsvIsUndefined(path))
    jsvGetString(path, pathStr, JS_DIR_BUF_SIZE);

  JsVar *result = jsvNewFromEmptyString();
  if (!result) return 0; // out of memory

  FRESULT res = 0;
  if (jsfsInit()) {
#ifndef LINUX
    FIL file;
    if ((res=f_open(&file, pathStr, FA_READ)) == FR_OK) {
#else
    FILE *file = fopen(pathStr, "r");
    if (file) {
#endif
      // re-use pathStr buffer
      size_t bytesRead = JS_DIR_BUF_SIZE;
      while (res==FR_OK && bytesRead==JS_DIR_BUF_SIZE) {
#ifndef LINUX
        res = f_read (&file, pathStr, JS_DIR_BUF_SIZE, &bytesRead);
#else
        bytesRead = fread(pathStr,1,JS_DIR_BUF_SIZE,file);
#endif
        jsvAppendStringBuf(result, pathStr, (int)bytesRead);
      }
#ifndef LINUX
      f_close(&file);
#else
      fclose(file);
#endif
    }
  }

  if (res) jsfsReportError("Unable to read file", res);
  return result;
}
