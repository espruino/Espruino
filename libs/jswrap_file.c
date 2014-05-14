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

#define JS_FS_DATA_NAME JS_HIDDEN_CHAR_STR"FSdata"
#define JS_FS_OPEN_FILES_NAME JS_HIDDEN_CHAR_STR"FSOpenFiles"
#define JS_FS_OPEN_PIPES_NAME JS_HIDDEN_CHAR_STR"FSOpenPipes"

// from jswrap_fat
extern bool jsfsInit();
extern void jsfsReportError(const char *msg, FRESULT res);

//object methods handles
size_t _readFile(JsFile* file, JsVar* buffer, int length, int position, FRESULT* res);
size_t _writeFile(JsFile* file, JsVar* buffer, int length, int position, FRESULT* res);
void _closeFile(JsFile* file);

/*JSON{ "type":"library",
        "class" : "File",
        "description" : ["This is the stream related file IO library.",
                         "To use this, you must type ```var fd = require('fs').open('filepath','flags','mode')``` to open a file stream." ]
}*/

JsVar* fsGetArray(const char *name, bool create) {
  JsVar *arrayName = jsvFindChildFromString(execInfo.root, name, create);
  JsVar *arr = jsvSkipName(arrayName);
  if (!arr && create) {
    arr = jsvNewWithFlags(JSV_ARRAY);
    jsvSetValueOfName(arrayName, arr);
  }
  jsvUnLock(arrayName);
  return arr;
}

static bool fileGetFromVar(JsFile *file, JsVar *parent) {
  bool ret = false;
  JsVar *fHandle = jsvObjectGetChild(parent, JS_FS_DATA_NAME, 0);
  assert(fHandle);
  if (fHandle) {
    jsvGetString(fHandle, (char*)&file->data, sizeof(JsFileData)+1/*trailing zero*/);
    jsvUnLock(fHandle);
    file->fileVar = parent;
    file->read = _readFile;
    file->write = _writeFile;
    file->close = _closeFile;
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
    JsVar *arr = fsGetArray(JS_FS_OPEN_FILES_NAME,false);
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
  bool ret = false;
  JsVar *parent = jspNewObject(0, "File");
  if (parent) {// low memory
    file->fileVar = parent;
    file->data.mode = mode;
    file->data.type = type;
    file->data.state = FS_NONE;
    file->read = _readFile;
    file->write = _writeFile;
    file->close = _closeFile;
    ret = true;
  }
  return ret;
}

/*JSON{  "type" : "staticmethod", "class" : "fs", "name" : "open",
         "generate_full" : "jswrap_file_open(path, mode)",
         "description" : [ "Open a file."],
         "params" : [ [ "path", "JsVar", "the path to the file to open." ],
                      [ "mode", "JsVar", "The mode to use when opening the file. Valid values for mode are 'r' for read and 'w' for write"] ],
         "return" : [ "JsVar", "The file handle or undefined if the file specified does not exist." ]
}*/
JsVar *jswrap_file_open(JsVar* path, JsVar* mode) {
  FRESULT res = FR_INVALID_NAME;
  JsFile file;
  FileMode fMode = FM_NONE;
  if (jsfsInit()) {
    JsVar *arr = fsGetArray(JS_FS_OPEN_FILES_NAME, true);
    if (!arr) return 0; // out of memory

    char pathStr[JS_DIR_BUF_SIZE] = "";
    char modeStr[3] = "";
    if (!jsvIsUndefined(path) && !jsvIsUndefined(mode)) {
      jsvGetString(path, pathStr, JS_DIR_BUF_SIZE);
      jsvGetString(mode, modeStr, 3);

      if(strcmp(modeStr,"r") == 0) {
        fMode = FM_READ;
      } else if(strcmp(modeStr,"w") == 0) {
        fMode = FM_WRITE;
      } else if(strcmp(modeStr,"w+") == 0 || strcmp(modeStr,"r+") == 0) {
        fMode = FM_READ_WRITE;
      }
      if(fMode != FM_NONE && allocateJsFile(&file, fMode, FT_FILE)) {
#ifndef LINUX
        if ((res=f_open(&file.data.handle, pathStr, fMode)) == FR_OK) {
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
        }
      }
    }
  }

  if(res != FR_OK) {
    jsfsReportError("Could not open file", res);
  }

  return file.fileVar;
}

/*JSON{  "type" : "method", "class" : "File", "name" : "close",
         "generate_full" : "jswrap_file_close(parent)",
         "description" : [ "Close an open file."]
}*/
//fs.close(fd)
void jswrap_file_close(JsVar* parent) {
  if (jsfsInit()) {
    JsFile file;
    if (fileGetFromVar(&file, parent) && file.data.state == FS_OPEN) {
      file.close(&file);
      JsVar *arr = fsGetArray(JS_FS_OPEN_FILES_NAME, false);
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

void _closeFile(JsFile* file) {
  if(file) {
#ifndef LINUX
    f_close(&file->data.handle);
#else
    fclose(file->data.handle);
    file->data.handle = 0;
#endif
    file->data.state = FS_CLOSED;
    fileSetVar(file);
  }
}

/*JSON{  "type" : "method", "class" : "File", "name" : "write",
         "generate" : "jswrap_file_write",
         "description" : [ "write data to a file in byte size chunks"],
         "params" : [ ["buffer", "JsVar", "an array to use for storing bytes read."],
                      ["length", "int32", "is an integer specifying the number of bytes to write."],
                      ["position", "int32", "is an integer specifying where to begin writing to in the file.", "If position is null, data will be written from the current file position."],
                      ["callback", "JsVar", "a function to call when the data has been written."]],
         "return" : [ "int32", "the number of bytes written" ]
}*/
//fs.write(fd, buffer, offset, length, position, callback)
size_t jswrap_file_write(JsVar* parent, JsVar* buffer, int length, int position, JsVar* callback) {
  FRESULT res = 0;
  size_t bytesWritten = 0;
  if (jsfsInit()) {
    JsFile file;
    if (fileGetFromVar(&file, parent)) {
      bytesWritten = file.write(&file, buffer, length, position, &res);
    }
    if(callback) {
      JsVar *bytesWrittenVar = jsvNewFromInteger((JsVarInt)bytesWritten);
      jsiQueueEvents(callback, &bytesWrittenVar, 1);
      jsvUnLock(bytesWrittenVar);
    }
  }

  if (res) {
    jsfsReportError("Unable to write file", res);
  }
  return bytesWritten;
}

size_t _writeFile(JsFile* file, JsVar* buffer, int length, int position, FRESULT* res) {
  size_t bytesWritten = 0;
  if(file->data.mode == FM_WRITE || file->data.mode == FM_READ_WRITE) {
    if(position >= 0) {
#ifndef LINUX
      f_lseek(&file->data.handle, position);
#else
      fseek(file->data.handle, position, SEEK_SET);
#endif
    }

    JsvStringIterator it;
    JsVar *dataString = jsvSkipName(buffer);
    jsvStringIteratorNew(&it, dataString, 0);
    size_t written = 0;
    char buf = '\0';
    int i =0;
    for(i=0; i<length; i++) {
      if(!jsvStringIteratorHasChar(&it) || *res!=FR_OK) {
        break;
      }
      buf = jsvStringIteratorGetChar(&it);
#ifndef LINUX
      *res = f_write(&file->data.handle, &buf, sizeof(buf), &written);
      f_sync(&file->data.handle);
#else
      written = fwrite(&buf, sizeof(buf), sizeof(buf), file->data.handle);
      fflush(file->data.handle);
      if(written == 0) {
        *res = FR_DISK_ERR;
      }
#endif
      bytesWritten += written;
      jsvStringIteratorNext(&it);
    }
    jsvStringIteratorFree(&it);
    jsvUnLock(dataString);
  }
  return bytesWritten;
}

/*JSON{  "type" : "method", "class" : "File", "name" : "read",
         "generate" : "jswrap_file_read",
         "description" : [ "Read data in a file in byte size chunks"],
         "params" : [ ["buffer", "JsVar", "an array to use for storing bytes read."],
                      ["length", "int32", "is an integer specifying the number of bytes to read."],
                      ["position", "int32", "is an integer specifying where to begin reading from in the file.", "If position is null, data will be read from the current file position."],
                      ["callback", "JsVar", "a function to call when the data has been read."]],
         "return" : [ "int32", "the number of bytes read" ]
}*/
//fs.read(fd, buffer, length, position, callback)
size_t jswrap_file_read(JsVar* parent, JsVar* buffer, int length, int position, JsVar* callback) {
  FRESULT res = 0;
  size_t bytesRead = 0;
  if (jsfsInit()) {
    JsFile file;
    if (fileGetFromVar(&file, parent)) {
      bytesRead = file.read(&file, buffer, length, position, &res);
    }
    if(callback) {
      JsVar *bytesReadVar = jsvNewFromInteger((JsVarInt)bytesRead);
      jsiQueueEvents(callback, &bytesReadVar, 1);
      jsvUnLock(bytesReadVar);
    }
  }

  if (res) {
    jsfsReportError("Unable to read file", res);
  }
  return bytesRead;
}

size_t _readFile(JsFile* file, JsVar* buffer, int length, int position, FRESULT* res) {
  size_t bytesRead = 0;
  if(file->data.mode == FM_READ || file->data.mode == FM_READ_WRITE) {
    if(position >= 0) {
#ifndef LINUX
      f_lseek(&file->data.handle, position);
#else
      fseek(file->data.handle, position, SEEK_SET);
#endif
    }
    size_t readBytes=0;
    char buf = '\0';
    int i = 0;
    for(i=0; i < length;i++) {
#ifndef LINUX
      *res = f_read(&file->data.handle, &buf, sizeof( buf ), &readBytes);
      if(*res != FR_OK) {
        break;
      }
#else
      readBytes = fread(&buf,sizeof( buf ), sizeof( buf ), file->data.handle);
      if(readBytes > 0) {
        *res = FR_OK;
      }
#endif
      jsvAppendStringBuf(buffer, &buf, 1);
      bytesRead += readBytes;
    }
  }
  return bytesRead;
}
