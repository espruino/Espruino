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
#include "jswrap_date.h"

#ifndef LINUX
#include "ff.h" // filesystem stuff
#else
#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h> // for readdir
#endif


/*JSON{
  "type" : "library",
  "class" : "fs"
}
This library handles interfacing with a FAT32 filesystem on an SD card. The API is designed to be similar to node.js's - However Espruino does not currently support asynchronous file IO, so the functions behave like node.js's xxxxSync functions. Versions of the functions with 'Sync' after them are also provided for compatibility.

Currently this provides minimal file IO - it's great for logging and loading/saving settings, but not good for loading large amounts of data as you will soon fill your memory up.

It is currently only available on boards that contain an SD card slot, such as the Olimexino and the HY. It can not currently be added to boards that did not ship with a card slot.

To use this, you must type ```var fs = require('fs')``` to get access to the library
*/

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

#endif

// from jswrap_file
bool jsfsGetPathString(char *pathStr, JsVar *path);
extern bool jsfsInit();
extern void jsfsReportError(const char *msg, FRESULT res);

/*JSON{
  "type" : "staticmethod",
  "class" : "fs",
  "name" : "readdir",
  "generate" : "jswrap_fs_readdir",
  "params" : [
    ["path","JsVar","The path of the directory to list. If it is not supplied, '' is assumed, which will list the root directory"]
  ],
  "return" : ["JsVar","An array of filename strings (or undefined if the directory couldn't be listed)"]
}
List all files in the supplied directory, returning them as an array of strings.

NOTE: Espruino does not yet support Async file IO, so this function behaves like the 'Sync' version.
*/
/*JSON{
  "type" : "staticmethod",
  "class" : "fs",
  "name" : "readdirSync",
  "ifndef" : "SAVE_ON_FLASH",
  "generate" : "jswrap_fs_readdir",
  "params" : [
    ["path","JsVar","The path of the directory to list. If it is not supplied, '' is assumed, which will list the root directory"]
  ],
  "return" : ["JsVar","An array of filename strings (or undefined if the directory couldn't be listed)"]
}
List all files in the supplied directory, returning them as an array of strings.
*/

JsVar *jswrap_fs_readdir(JsVar *path) {
  JsVar *arr = 0; // undefined unless we can open card

  char pathStr[JS_DIR_BUF_SIZE] = "";
  if (!jsvIsUndefined(path))
    if (!jsfsGetPathString(pathStr, path)) return 0;
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
      arr = jsvNewEmptyArray();
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
#ifndef LINUX
      f_closedir(&dirs);
#else
      closedir(dir);
#endif
    }
  }
  if (res) jsfsReportError("Unable to list files", res);
  return arr;
}

/*JSON{
  "type" : "staticmethod",
  "class" : "fs",
  "name" : "writeFile",
  "generate_full" : " jswrap_fs_writeOrAppendFile(path, data, false)",
  "params" : [
    ["path","JsVar","The path of the file to write"],
    ["data","JsVar","The data to write to the file"]
  ],
  "return" : ["bool","True on success, false on failure"]
}
Write the data to the given file

NOTE: Espruino does not yet support Async file IO, so this function behaves like the 'Sync' version.
*/
/*JSON{
  "type" : "staticmethod",
  "class" : "fs",
  "name" : "writeFileSync",
  "ifndef" : "SAVE_ON_FLASH",
  "generate_full" : " jswrap_fs_writeOrAppendFile(path, data, false)",
  "params" : [
    ["path","JsVar","The path of the file to write"],
    ["data","JsVar","The data to write to the file"]
  ],
  "return" : ["bool","True on success, false on failure"]
}
Write the data to the given file
*/
/*JSON{
  "type" : "staticmethod",
  "class" : "fs",
  "name" : "appendFile",
  "generate_full" : " jswrap_fs_writeOrAppendFile(path, data, true)",
  "params" : [
    ["path","JsVar","The path of the file to write"],
    ["data","JsVar","The data to write to the file"]
  ],
  "return" : ["bool","True on success, false on failure"]
}
Append the data to the given file, created a new file if it doesn't exist

NOTE: Espruino does not yet support Async file IO, so this function behaves like the 'Sync' version.
*/
/*JSON{
  "type" : "staticmethod",
  "class" : "fs",
  "name" : "appendFileSync",
  "ifndef" : "SAVE_ON_FLASH",
  "generate_full" : "jswrap_fs_writeOrAppendFile(path, data, true)",
  "params" : [
    ["path","JsVar","The path of the file to write"],
    ["data","JsVar","The data to write to the file"]
  ],
  "return" : ["bool","True on success, false on failure"]
}
Append the data to the given file, created a new file if it doesn't exist
*/
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

/*JSON{
  "type" : "staticmethod",
  "class" : "fs",
  "name" : "readFile",
  "generate" : "jswrap_fs_readFile",
  "params" : [
    ["path","JsVar","The path of the file to read"]
  ],
  "return" : ["JsVar","A string containing the contents of the file (or undefined if the file doesn't exist)"]
}
Read all data from a file and return as a string

NOTE: Espruino does not yet support Async file IO, so this function behaves like the 'Sync' version.
*/
/*JSON{
  "type" : "staticmethod",
  "class" : "fs",
  "name" : "readFileSync",
  "ifndef" : "SAVE_ON_FLASH",
  "generate" : "jswrap_fs_readFile",
  "params" : [
    ["path","JsVar","The path of the file to read"]
  ],
  "return" : ["JsVar","A string containing the contents of the file (or undefined if the file doesn't exist)"]
}
Read all data from a file and return as a string.

**Note:** The size of files you can load using this method is limited by the amount of available RAM. To read files a bit at a time, see the `File` class.
*/
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

  /*JSON{
  "type" : "staticmethod",
  "class" : "fs",
  "name" : "unlink",
  "ifndef" : "SAVE_ON_FLASH",
  "generate" : "jswrap_fs_unlink",
  "params" : [
    ["path","JsVar","The path of the file to delete"]
  ],
  "return" : ["bool","True on success, or false on failure"]
}
Delete the given file

NOTE: Espruino does not yet support Async file IO, so this function behaves like the 'Sync' version.
*/
/*JSON{
  "type" : "staticmethod",
  "class" : "fs",
  "name" : "unlinkSync",
  "ifndef" : "SAVE_ON_FLASH",
  "generate" : "jswrap_fs_unlink",
  "params" : [
    ["path","JsVar","The path of the file to delete"]
  ],
  "return" : ["bool","True on success, or false on failure"]
}
Delete the given file
*/
bool jswrap_fs_unlink(JsVar *path) {
  char pathStr[JS_DIR_BUF_SIZE] = "";
  if (!jsvIsUndefined(path))
    if (!jsfsGetPathString(pathStr, path)) return 0;

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

/*JSON{
  "type" : "staticmethod",
  "class" : "fs",
  "name" : "statSync",
  "ifndef" : "SAVE_ON_FLASH",
  "generate" : "jswrap_fs_stat",
  "params" : [
    ["path","JsVar","The path of the file to get information on"]
  ],
  "return" : ["JsVar","An object describing the file, or undefined on failure"]
}
Return information on the given file. This returns an object with the following
fields:

size: size in bytes
dir: a boolean specifying if the file is a directory or not
mtime: A Date structure specifying the time the file was last modified
*/
JsVar *jswrap_fs_stat(JsVar *path) {
  char pathStr[JS_DIR_BUF_SIZE] = "";
  if (!jsvIsUndefined(path))
    if (!jsfsGetPathString(pathStr, path)) return 0;

#ifndef LINUX
  FRESULT res = 0;
  if (jsfsInit()) {
    FILINFO info;
    memset(&info,0,sizeof(info));
    res = f_stat(pathStr, &info);
    if (res==0 /*ok*/) {
      JsVar *obj = jsvNewObject();
      if (!obj) return 0;
      jsvObjectSetChildAndUnLock(obj, "size", jsvNewFromInteger((JsVarInt)info.fsize));
      jsvObjectSetChildAndUnLock(obj, "dir", jsvNewFromBool(info.fattrib & AM_DIR));

      CalendarDate date;
      date.year = 1980+(int)((info.fdate>>9)&127);
      date.month = (int)((info.fdate>>5)&15);
      date.day = (int)((info.fdate)&31);
      TimeInDay td;
      td.daysSinceEpoch = fromCalenderDate(&date);
      td.hour = (int)((info.ftime>>11)&31);
      td.min = (int)((info.ftime>>5)&63);
      td.sec = (int)((info.ftime)&63);
      td.ms = 0;
      td.zone = 0;
      jsvObjectSetChildAndUnLock(obj, "mtime", jswrap_date_from_milliseconds(fromTimeInDay(&td)));
      return obj;
    }
  }
#else
  struct stat info;
  if (stat(pathStr, &info)==0 /*ok*/) {
    JsVar *obj = jsvNewObject();
    if (!obj) return 0;
    jsvObjectSetChildAndUnLock(obj, "size", jsvNewFromInteger((JsVarInt)info.st_size));
    jsvObjectSetChildAndUnLock(obj, "dir", jsvNewFromBool(S_ISDIR(info.st_mode)));
    jsvObjectSetChildAndUnLock(obj, "mtime", jswrap_date_from_milliseconds((JsVarFloat)info.st_mtime*1000.0));
    return obj;
  }
#endif

  return 0;
}

  /*JSON{
  "type" : "staticmethod",
  "class" : "fs",
  "name" : "mkdir",
  "ifndef" : "SAVE_ON_FLASH",
  "generate" : "jswrap_fs_mkdir",
  "params" : [
    ["path","JsVar","The name of the directory to create"]
  ],
  "return" : ["bool","True on success, or false on failure"]
}
Create the directory

NOTE: Espruino does not yet support Async file IO, so this function behaves like the 'Sync' version.
*/
/*JSON{
  "type" : "staticmethod",
  "class" : "fs",
  "name" : "mkdirSync",
  "ifndef" : "SAVE_ON_FLASH",
  "generate" : "jswrap_fs_mkdir",
  "params" : [
    ["path","JsVar","The name of the directory to create"]
  ],
  "return" : ["bool","True on success, or false on failure"]
}
Create the directory
*/
bool jswrap_fs_mkdir(JsVar *path) {
  char pathStr[JS_DIR_BUF_SIZE] = "";
  if (!jsvIsUndefined(path))
    if (!jsfsGetPathString(pathStr, path)) return 0;

#ifndef LINUX
  FRESULT res = 0;
  if (jsfsInit()) {
    res = f_mkdir(pathStr);
  }
#else
  FRESULT res = mkdir(pathStr, 0777);
#endif

  if (res) {
    jsfsReportError("Unable to create the directory", res);
    return false;
  }
  return true;
}
