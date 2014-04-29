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
#include "jswrap_fs.h"

/*JSON{ "type":"library",
        "class" : "fs",
        "description" : ["This library handles interfacing with a FAT32 filesystem on an SD card. The API is designed to be similar to node.js's - However Espruino does not currently support asynchronous file IO, so the functions behave like node.js's xxxxSync functions. Versions of the functions with 'Sync' after them are also provided for compatibility.",
                         "Currently this provides minimal file IO - it's great for logging and loading/saving settings, but not good for loading large amounts of data as you will soon fill your memory up.",
                         "It is currently only available on boards that contain an SD card slot, such as the Olimexino and the HY. It can not currently be added to boards that did not ship with a card slot.",
                         "To use this, you must type ```var fs = require('fs')``` to get access to the library" ]
}*/

/*JSON{ "type":"library",
        "class" : "File",
        "description" : ["This is the stream related file IO library.",
                         "To use this, you must type ```var fd = require('fs').open('filepath','flags','mode')``` to open a file stream." ]
}*/

/*JSON{ "type":"library",
        "class" : "Pipe",
        "description" : ["This is the Pipe container for async related file IO library." ]
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

/*JSON{ "type":"idle", "generate" : "jswrap_fs_idle" }*/
bool jswrap_fs_idle() {
  bool ret = false;
  JsVar *arr = fsGetArray(JS_HIDDEN_CHAR_STR"FSOpenPipes",false);
  if (arr) {
    JsvArrayIterator it;
    jsvArrayIteratorNew(&it, arr);
    while (jsvArrayIteratorHasElement(&it)) {
      JsVar *pipe = jsvArrayIteratorGetElement(&it);
      JsVar *position = jsvObjectGetChild(pipe,"Position",0);
      JsVar *chunkSize = jsvObjectGetChild(pipe,"ChunkSize",0);
      JsVar *source = jsvObjectGetChild(pipe,"Source",0);
      JsVar *destination = jsvObjectGetChild(pipe,"Destination",0);
      if(!_Pipe(source, destination, chunkSize, position)) { // when no more chunks are possible, execute the callback.
        jsiQueueObjectCallbacks(pipe, "#oncomplete", &pipe, 1);
        JsVar *idx = jsvArrayIteratorGetIndex(&it);
        jsvRemoveChild(arr,idx);
        jsvUnLock(idx);
      } else {
        ret = true;
      }
      jsvUnLock(source);
      jsvUnLock(destination);
      jsvUnLock(pipe);
      jsvUnLock(chunkSize);
      jsvUnLock(position);
      jsvArrayIteratorNext(&it);
    }
    jsvArrayIteratorFree(&it);
    jsvUnLock(arr);
  }
  return ret;
}

bool _Pipe(JsVar* source, JsVar* destination, JsVar* chunkSize, JsVar* position) {
  bool InProgress = true;
  if(jsfsInit() && source && destination && chunkSize && position) {
    JsFile sourceFile;
    JsFile destinationFile;
    JsVar * Buffer;
    FRESULT res;
    if(fileGetFromVar(&sourceFile, source) && fileGetFromVar(&destinationFile, destination)) {
      size_t ReadLength = jsvGetInteger(chunkSize);
      size_t Position = jsvGetInteger(position);
      Buffer = jsvNewStringOfLength(0);
      if(Buffer) {// do we have enough memory?
        size_t BytesRead = sourceFile.read(&sourceFile, Buffer, ReadLength, Position, &res);
        if(res == FR_OK && BytesRead > 0) {
          destinationFile.write(&destinationFile, Buffer, BytesRead, Position, &res);
          jsvSetInteger(position, (Position+BytesRead));
        } else {
          InProgress = false;
        }
        jsvUnLock(Buffer);
      }
    } else {
      InProgress = false;
    }
  }
  return InProgress;
}

/*JSON{ "type":"kill", "generate" : "jswrap_fs_kill" }*/
void jswrap_fs_kill() {
  {
    JsVar *arr = fsGetArray(JS_HIDDEN_CHAR_STR"FSOpenFiles",false);
    if (arr) {
      JsvArrayIterator it;
      jsvArrayIteratorNew(&it, arr);
      while (jsvArrayIteratorHasElement(&it)) {
        JsVar *file = jsvArrayIteratorGetElement(&it);
        wrap_fat_close(file);
        jsvUnLock(file);
        jsvArrayIteratorNext(&it);
      }
      jsvArrayIteratorFree(&it);
      jsvRemoveAllChildren(arr);
      jsvUnLock(arr);
    }
  }
  // all open files will have been closed..
  // now remove all pipes...
  {
    JsVar *arr = fsGetArray(JS_HIDDEN_CHAR_STR"FSOpenPipes", false);
    if (arr) {
      jsvRemoveAllChildren(arr);
      jsvUnLock(arr);
    }
  }
}

bool fileGetFromVar(JsFile *file, JsVar *parent) {
  bool ret = false;
  JsVar *fHandle = jsvObjectGetChild(parent, JS_HIDDEN_CHAR_STR"FShandle", 0);
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

void fileSetVar(JsFile *file) {
  JsVar *fHandle = jsvFindChildFromString(file->fileVar, JS_HIDDEN_CHAR_STR"FShandle", true);
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

/*JSON{  "type" : "staticmethod", "class" : "fs", "name" : "open",
         "generate_full" : "wrap_fat_open(path, mode)",
         "description" : [ "Open a file."],
         "params" : [ [ "path", "JsVar", "the path to the file to open." ],
                      [ "mode", "JsVar", "The mode to use when opening the file. Valid values for mode are 'r' for read and 'w' for write"] ],
         "return" : [ "JsVar", "The file handle or undefined if the file specified does not exist." ]
}*/
//fs.open(path, mode, callback)
JsVar* wrap_fat_open(JsVar* path, JsVar* mode) {
  FRESULT res = FR_INVALID_NAME;
  JsFile file;
  FileMode fMode = FM_NONE;
  if (jsfsInit()) {
    JsVar *arr = fsGetArray(JS_HIDDEN_CHAR_STR"FSOpenFiles", true);
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
      if(fMode != FM_NONE && AllocateJsFile(&file, fMode, FT_FILE)) {
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

bool AllocateJsFile(JsFile* file,FileMode mode, FileType type) {
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

/*JSON{  "type" : "method", "class" : "File", "name" : "pipe",
         "generate" : "wrap_fat_pipe",
         "params" : [ ["destfd", "JsVar", "The destination file/stream that will receive this files contents."],
                      ["chunkSize", "JsVar", "The amount of data to pipe from source to destination at a time."],
                      ["callback", "JsVar", "a function to call when the pipe activity is complete."] ],
         "return" : [ "JsVar", "The source file handle, for chaining multiple pipe actions." ]
}*/
JsVar * wrap_fat_pipe(JsVar* parent, JsVar* destfd, JsVar* ChunkSize, JsVar* callback) {
  JsVar *arr = fsGetArray(JS_HIDDEN_CHAR_STR"FSOpenPipes", true);
  if (arr) {// out of memory?
    if (parent && destfd && ChunkSize) {
      JsVar *pipe = jspNewObject(0, "Pipe");
      if(pipe) {// out of memory?
        if(callback) {
          jsvUnLock(jsvAddNamedChild(pipe, callback, "#oncomplete"));
        }
        jsvArrayPush(arr, pipe);
        JsVar* Position = jsvNewFromInteger(0);
        if(Position) {
          jsvUnLock(jsvAddNamedChild(pipe, Position, "Position"));
          jsvUnLock(jsvAddNamedChild(pipe, ChunkSize, "ChunkSize"));
          jsvUnLock(jsvAddNamedChild(pipe, parent, "Source"));
          jsvUnLock(jsvAddNamedChild(pipe, destfd, "Destination"));
          jsvUnLock(Position);
        }
        jsvUnLock(pipe);
      }
    }
    jsvUnLock(arr);
  }
  jsvLockAgain(parent);
  return parent;
}

/*JSON{  "type" : "method", "class" : "File", "name" : "close",
         "generate_full" : "wrap_fat_close(parent)",
         "description" : [ "Close an open file."]
}*/
//fs.close(fd)
void wrap_fat_close(JsVar* parent) {
  if (jsfsInit()) {
    JsFile file;
    if (fileGetFromVar(&file, parent) && file.data.state == FS_OPEN) {
      file.close(&file);
      JsVar *arr = fsGetArray(JS_HIDDEN_CHAR_STR"FSOpenFiles", false);
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
         "generate" : "wrap_fat_write",
         "description" : [ "write data to a file in byte size chunks"],
         "params" : [ ["buffer", "JsVar", "an array to use for storing bytes read."],
                      ["length", "int32", "is an integer specifying the number of bytes to write."],
                      ["position", "int32", "is an integer specifying where to begin writing to in the file.", "If position is null, data will be written from the current file position."],
                      ["callback", "JsVar", "a function to call when the data has been written."]],
         "return" : [ "int32", "the number of bytes written" ]
}*/
//fs.write(fd, buffer, offset, length, position, callback)
size_t wrap_fat_write(JsVar* parent, JsVar* buffer, int length, int position, JsVar* callback) {
  FRESULT res = 0;
  size_t bytesWritten = 0;
  if (jsfsInit()) {
    JsFile file;
    if (fileGetFromVar(&file, parent)) {
      bytesWritten = file.write(&file, buffer, length, position, &res);
    }
    if(callback) {
      JsVar *bytesWrittenVar = jsvNewFromInteger(bytesWritten);
      jsiQueueEvents(jsvGetRef(callback), &bytesWrittenVar, 1);
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
         "generate" : "wrap_fat_read",
         "description" : [ "Read data in a file in byte size chunks"],
         "params" : [ ["buffer", "JsVar", "an array to use for storing bytes read."],
                      ["length", "int32", "is an integer specifying the number of bytes to read."],
                      ["position", "int32", "is an integer specifying where to begin reading from in the file.", "If position is null, data will be read from the current file position."],
                      ["callback", "JsVar", "a function to call when the data has been read."]],
         "return" : [ "int32", "the number of bytes read" ]
}*/
//fs.read(fd, buffer, length, position, callback)
size_t wrap_fat_read(JsVar* parent, JsVar* buffer, int length, int position, JsVar* callback) {
  FRESULT res = 0;
  size_t bytesRead = 0;
  if (jsfsInit()) {
    JsFile file;
    if (fileGetFromVar(&file, parent)) {
      bytesRead = file.read(&file, buffer, length, position, &res);
    }
    if(callback) {
      JsVar *bytesReadVar = jsvNewFromInteger(bytesRead);
      jsiQueueEvents(jsvGetRef(callback), &bytesReadVar, 1);
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

/*JSON{  "type" : "staticmethod", "class" : "fs", "name" : "createReadStream",
         "generate" : "createReadStream",
         "description" : [ "Create a stream object to read the specified file" ],
         "params" : [ [ "path", "JsVar", "The path of the file to read" ]],
         "return" : [ "JsVar", "The ReadStream Object" ]
}*/
//var r = fs.createReadStream('file.txt');
JsVar* createReadStream(JsVar* path) {
  JsVar *mode = jsvNewFromString("r");
  if(!mode) return 0; // low memory
  return wrap_fat_open(path, mode);
}

/*JSON{  "type" : "staticmethod", "class" : "fs", "name" : "createWriteStream",
         "generate" : "createWriteStream",
         "description" : [ "Create a stream object to write to the specified file" ],
         "params" : [ [ "path", "JsVar", "The path of the file to write" ]],
         "return" : [ "JsVar", "The WriteStream Object" ]
}*/
//var w = fs.createWriteStream('file.txt');
JsVar* createWriteStream(JsVar* path) {
  JsVar *mode = jsvNewFromString("w");
  if (!mode) return 0; // low memory
  return wrap_fat_open(path, mode);
}
