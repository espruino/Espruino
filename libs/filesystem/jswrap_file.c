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
#include "jswrap_file.h"

#define JS_FS_DATA_NAME JS_HIDDEN_CHAR_STR"FSdata" // the data in each file
#define JS_FS_OPEN_FILES_NAME JS_HIDDEN_CHAR_STR"FSOpenFiles" // the list of open files

// from jswrap_fat
extern bool jsfsInit();
extern void jsfsReportError(const char *msg, FRESULT res);

/*JSON{ "type":"library",
        "class" : "File",
        "description" : ["This is the File object - it allows you to stream data to and from files (As opposed to the `require('fs').readFile(..)` style functions that read an entire file).",
                         "To create a File object, you must type ```var fd = E.openFile('filepath','mode')``` - see [E.openFile](#l_E_openFile) for more information." ]
}*/

static JsVar* fsGetArray(bool create) {
  return jsvObjectGetChild(execInfo.hiddenRoot, JS_FS_OPEN_FILES_NAME, create ? JSV_ARRAY : 0);
}

static bool fileGetFromVar(JsFile *file, JsVar *parent) {
  bool ret = false;
  JsVar *fHandle = jsvObjectGetChild(parent, JS_FS_DATA_NAME, 0);
  if (fHandle) {
    jsvGetString(fHandle, (char*)&file->data, sizeof(JsFileData)+1/*trailing zero*/);
    jsvUnLock(fHandle);
    file->fileVar = parent;
    if(file->data.state == FS_OPEN) {// return false if the file has been closed.
      ret = true;
    }
  }
  return ret;
}

static void fileSetVar(JsFile *file) {
  JsVar *fHandle = jsvFindChildFromString(file->fileVar, JS_FS_DATA_NAME, true);
  JsVar *data = jsvSkipName(fHandle);
  if (!data) {
    data = jsvNewStringOfLength(sizeof(JsFileData));
    jsvSetValueOfName(fHandle, data);
  }
  jsvUnLock(fHandle);
  assert(data);
  jsvSetString(data, (char*)&file->data, sizeof(JsFileData));
  jsvUnLock(data);
}

/*JSON{ "type":"kill", "generate" : "jswrap_file_kill" }*/
void jswrap_file_kill() {
  {
    JsVar *arr = fsGetArray(false);
    if (arr) {
      JsvArrayIterator it;
      jsvArrayIteratorNew(&it, arr);
      while (jsvArrayIteratorHasElement(&it)) {
        JsVar *file = jsvArrayIteratorGetElement(&it);
        jswrap_file_close(file);
        jsvUnLock(file);
        jsvArrayIteratorNext(&it);
      }
      jsvArrayIteratorFree(&it);
      jsvRemoveAllChildren(arr);
      jsvUnLock(arr);
    }
  }
}

static bool allocateJsFile(JsFile* file,FileMode mode, FileType type) {
  JsVar *parent = jspNewObject(0, "File");
  if (!parent) return false; // low memory
  file->fileVar = parent;
  file->data.mode = mode;
  file->data.type = type;
  file->data.state = FS_NONE;
  return true;
}

/*JSON{ "type":"staticmethod",
        "class" : "E",
        "name" : "openFile",
        "generate" : "jswrap_E_openFile",
        "description" : [ "Open a file" ],
        "params" : [ [ "path", "JsVar", "the path to the file to open." ],
                      [ "mode", "JsVar", "The mode to use when opening the file. Valid values for mode are 'r' for read, 'w' for write new, 'w+' for write existing, and 'a' for append. If not specified, the default is 'r'."] ],
        "return" : ["JsVar", "A File object"], "return_object":"File"
}*/
JsVar *jswrap_E_openFile(JsVar* path, JsVar* mode) {
  FRESULT res = FR_INVALID_NAME;
  JsFile file;
  file.fileVar = 0;
  FileMode fMode = FM_NONE;
  if (jsfsInit()) {
    JsVar *arr = fsGetArray(true);
    if (!arr) return 0; // out of memory

    char pathStr[JS_DIR_BUF_SIZE] = "";
    char modeStr[3] = "r";
    if (!jsvIsUndefined(path)) {
      jsvGetString(path, pathStr, JS_DIR_BUF_SIZE);
      if (!jsvIsUndefined(mode))
        jsvGetString(mode, modeStr, 3);

#ifndef LINUX
      BYTE ff_mode = 0;
      bool append = false;
#endif

      if(strcmp(modeStr,"r") == 0) {
        fMode = FM_READ;
#ifndef LINUX
        ff_mode = FA_READ | FA_OPEN_EXISTING;
#endif
      } else if(strcmp(modeStr,"a") == 0) {
        fMode = FM_WRITE;
#ifndef LINUX
        ff_mode = FA_WRITE | FA_OPEN_ALWAYS;
        append = true;
#endif
      } else if(strcmp(modeStr,"w") == 0) {
        fMode = FM_WRITE;
#ifndef LINUX
        ff_mode = FA_WRITE | FA_CREATE_ALWAYS;
#endif
      } else if(strcmp(modeStr,"w+") == 0) {
        fMode = FM_READ_WRITE;
#ifndef LINUX
        ff_mode = FA_WRITE | FA_OPEN_ALWAYS;
#endif
      }
      if(fMode != FM_NONE && allocateJsFile(&file, fMode, FT_FILE)) {
#ifndef LINUX
        if ((res=f_open(&file.data.handle, pathStr, ff_mode)) == FR_OK) {
          if (append) f_lseek(&file.data.handle, file.data.handle.fsize); // move to end of file
#else
        file.data.handle = fopen(pathStr, modeStr);
        if (file.data.handle) {
          res=FR_OK;
#endif
          file.data.state = FS_OPEN;
          fileSetVar(&file);
          // add to list of open files
          jsvArrayPush(arr, file.fileVar);
          jsvUnLock(arr);
        } else {
          // File open failed
          jsvUnLock(file.fileVar);
          file.fileVar = 0;
        }

        if(res != FR_OK)
          jsfsReportError("Could not open file", res);

      }
    } else {
      jsError("Path is undefined");
    }
  }


  return file.fileVar;
}

/*JSON{  "type" : "method", "class" : "File", "name" : "close",
         "generate_full" : "jswrap_file_close(parent)",
         "description" : [ "Close an open file."]
}*/
void jswrap_file_close(JsVar* parent) {
  if (jsfsInit()) {
    JsFile file;
    if (fileGetFromVar(&file, parent) && file.data.state == FS_OPEN) {
#ifndef LINUX
      f_close(&file.data.handle);
#else
      fclose(file.data.handle);
      file.data.handle = 0;
#endif
      file.data.state = FS_CLOSED;
      fileSetVar(&file);
      // TODO: could try and free the memory used by file.data ?

      JsVar *arr = fsGetArray(false);
      if (arr) {
        JsVar *idx = jsvGetArrayIndexOf(arr, file.fileVar, true);
        if (idx) {
          jsvRemoveChild(arr, idx);
          jsvUnLock(idx);
        }
        jsvUnLock(arr);
      }
    }
  }
}

/*JSON{  "type" : "method", "class" : "File", "name" : "write",
         "generate" : "jswrap_file_write",
         "description" : [ "write data to a file"],
         "params" : [ ["buffer", "JsVar", "A string containing the bytes to write"] ],
         "return" : [ "int32", "the number of bytes written" ]
}*/
size_t jswrap_file_write(JsVar* parent, JsVar* buffer) {
  FRESULT res = 0;
  size_t bytesWritten = 0;
  if (jsfsInit()) {
    JsFile file;
    if (fileGetFromVar(&file, parent)) {
      if(file.data.mode == FM_WRITE || file.data.mode == FM_READ_WRITE) {
        JsvIterator it;
        jsvIteratorNew(&it, buffer);
        char buf[32];

        while (jsvIteratorHasElement(&it)) {
          // pull in a buffer's worth of data
          size_t n = 0;
          while (jsvIteratorHasElement(&it) && n<sizeof(buf)) {
            buf[n++] = (char)jsvIteratorGetIntegerValue(&it);
            jsvIteratorNext(&it);
          }
          // write it out
          size_t written = 0;
#ifndef LINUX
          res = f_write(&file.data.handle, &buf, n, &written);
#else
          written = fwrite(&buf, 1, n, file.data.handle);
#endif
          bytesWritten += written;
          if(written == 0)
            res = FR_DISK_ERR;
          if (res) break;
        }
        jsvIteratorFree(&it);
        // finally, sync - just in case there's a reset or something
#ifndef LINUX
        f_sync(&file.data.handle);
#else
        fflush(file.data.handle);
#endif
      }

      fileSetVar(&file);
    }
  }

  if (res) {
    jsfsReportError("Unable to write file", res);
  }
  return bytesWritten;
}

/*JSON{  "type" : "method", "class" : "File", "name" : "read",
         "generate" : "jswrap_file_read",
         "description" : [ "Read data in a file in byte size chunks"],
         "params" : [ ["length", "int32", "is an integer specifying the number of bytes to read."] ],
         "return" : [ "JsVar", "A string containing the characters that were read" ]
}*/
JsVar *jswrap_file_read(JsVar* parent, int length) {
  JsVar *buffer = 0;
  JsvStringIterator it;
  FRESULT res = 0;
  size_t bytesRead = 0;
  if (jsfsInit()) {
    JsFile file;
    if (fileGetFromVar(&file, parent)) {
      if(file.data.mode == FM_READ || file.data.mode == FM_READ_WRITE) {
        char buf[32];
        size_t actual = 0;

        while (bytesRead < (size_t)length) {
          size_t requested = (size_t)length - bytesRead;
          if (requested > sizeof( buf ))
            requested = sizeof( buf );
          actual = 0;
    #ifndef LINUX
          res = f_read(&file.data.handle, buf, requested, &actual);
          if(res) break;
    #else
          actual = fread(buf, 1, requested, file.data.handle);
    #endif
          if (actual>0) {
            if (!buffer) {
              buffer = jsvNewFromEmptyString();
              if (!buffer) return 0; // out of memory
              jsvStringIteratorNew(&it, buffer, 0);
            }
            size_t i;
            for (i=0;i<actual;i++)
              jsvStringIteratorAppend(&it, buf[i]);
          }
          bytesRead += actual;
          if(actual != requested) break;
        }
        fileSetVar(&file);
      }
    }
  }
  if (res) jsfsReportError("Unable to read file", res);

  if (buffer)
    jsvStringIteratorFree(&it);

  // automatically close this file if we're at the end of it
  if (bytesRead!=(size_t)length)
    jswrap_file_close(parent);

  return buffer;
}

/*JSON{  "type" : "method", "class" : "File", "name" : "skip",
         "generate" : "jswrap_file_skip",
         "description" : [ "Skip the specified number of bytes forwards"],
         "params" : [ ["nBytes", "int32", "is an integer specifying the number of bytes to skip forwards."] ]
}*/
void jswrap_file_skip(JsVar* parent, int length) {
  if (length<=0) {
    jsWarn("length for skip must be greater than 0");
    return;
  }
  FRESULT res = 0;
  if (jsfsInit()) {
    JsFile file;
    if (fileGetFromVar(&file, parent)) {
      if(file.data.mode == FM_READ || file.data.mode == FM_WRITE || file.data.mode == FM_READ_WRITE) {
  #ifndef LINUX
        res = (FRESULT)f_lseek(&file.data.handle, (DWORD)f_tell(&file.data.handle) + (DWORD)length);
  #else
        fseek(file.data.handle, length, SEEK_CUR);
  #endif
        fileSetVar(&file);
      }
    }
  }
  if (res) jsfsReportError("Unable to skip", res);
}

/*JSON{  "type" : "method", "class" : "File", "name" : "pipe", "ifndef" : "SAVE_ON_FLASH",
         "generate" : "jswrap_pipe",
         "description" : [ "Pipe this file to a stream (an object with a 'write' method)"],
         "params" : [ ["destination", "JsVar", "The destination file/stream that will receive content from the source."],
                      ["options", "JsVar", [ "An optional object `{ chunkSize : int=32, end : bool=true, complete : function }`",
                                             "chunkSize : The amount of data to pipe from source to destination at a time",
                                             "complete : a function to call when the pipe activity is complete",
                                             "end : call the 'end' function on the destination when the source is finished"] ] ]
}*/
