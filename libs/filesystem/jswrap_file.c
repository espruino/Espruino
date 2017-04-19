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
#include "jsparse.h"

#define JS_FS_DATA_NAME JS_HIDDEN_CHAR_STR"FSd" // the data in each file
#define JS_FS_OPEN_FILES_NAME JS_HIDDEN_CHAR_STR"FSo" // the list of open files

#if !defined(LINUX) && !defined(USE_FILESYSTEM_SDIO) && !defined(USE_FLASHFS)
#define SD_CARD_ANYWHERE
#endif


#ifndef LINUX
FATFS jsfsFAT;
bool fat_initialised = false;
#endif

#ifdef SD_CARD_ANYWHERE
void sdSPISetup(JsVar *spi, Pin csPin);
bool isSdSPISetup();
#endif

#ifdef USE_FLASHFS
#include "flash_diskio.h"
#endif

// 'path' must be of JS_DIR_BUF_SIZE
bool jsfsGetPathString(char *pathStr, JsVar *path) {
  if (jsvGetString(path, pathStr, JS_DIR_BUF_SIZE)==JS_DIR_BUF_SIZE) {
    jsExceptionHere(JSET_ERROR, "File path too long\n");
    return false;
  }
  return true;
}

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

bool jsfsInit() {
   
#ifndef LINUX
  if (!fat_initialised) {
#ifndef USE_FLASHFS  
#ifdef SD_CARD_ANYWHERE
    if (!isSdSPISetup()) {
#ifdef SD_SPI
      const char *deviceStr = jshGetDeviceString(SD_SPI);
      JsVar *spi = jsvSkipNameAndUnLock(jspGetNamedVariable(deviceStr));
      JshSPIInfo inf;
      jshSPIInitInfo(&inf);
      inf.pinMISO = SD_DO_PIN;
      inf.pinMOSI = SD_DI_PIN;
      inf.pinSCK = SD_CLK_PIN;
      jshSPISetup(SD_SPI, &inf);
      sdSPISetup(spi, SD_CS_PIN);
      jsvUnLock(spi);
#else
      jsError("SD card must be setup with E.connectSDCard first");
      return false;
#endif // SD_SPI
    }
#endif // SD_CARD_ANYWHER
#endif // USE_FLASHFS 
    FRESULT res;

    if ((res = f_mount(&jsfsFAT, "", 1)) != FR_OK) {
       jsfsReportError("Unable to mount media", res);
       return false;
    }
    fat_initialised = true;
  }
#endif // LINUX
  return true;
}

/*JSON{
  "type" : "staticmethod",
  "class" : "E",
  "name" : "connectSDCard",
  "generate" : "jswrap_E_connectSDCard",
  "ifndef" : "SAVE_ON_FLASH",
  "params" : [
    ["spi","JsVar","The SPI object to use for communication"],
    ["csPin","pin","The pin to use for Chip Select"]
  ]
}
Setup the filesystem so that subsequent calls to `E.openFile` and `require('fs').*` will use an SD card on the supplied SPI device and pin.

It can even work using software SPI - for instance:

```
var spi = new SPI();
spi.setup({mosi:C7,miso:C8,sck:C9});
E.connectSDCard(spi,C6);
console.log(require("fs").readdirSync());
```
*/
void jswrap_E_connectSDCard(JsVar *spi, Pin csPin) {
#ifdef SD_CARD_ANYWHERE
  if (!jsvIsObject(spi)) {
    jsExceptionHere(JSET_ERROR, "First argument is a %t, not an SPI object\n", spi);
    return;
  }
  if (!jshIsPinValid(csPin)) {
    jsExceptionHere(JSET_ERROR, "Second argument is not a valid pin");
    return;
  }
  jswrap_E_unmountSD();
  sdSPISetup(spi, csPin);
#else
  NOT_USED(spi);
  NOT_USED(csPin);
  jsExceptionHere(JSET_ERROR, "Unimplemented on Linux");
#endif
}

/* TODO: maybe this should be in the 'E' library. However we don't currently
 * have a way of doing that in build_jswrapper.py  */
/*JSON{
  "type" : "class",
  "class" : "File"
}
This is the File object - it allows you to stream data to and from files (As opposed to the `require('fs').readFile(..)` style functions that read an entire file).

To create a File object, you must type ```var fd = E.openFile('filepath','mode')``` - see [E.openFile](#l_E_openFile) for more information.

**Note:** If you want to remove an SD card after you have started using it, you *must* call `E.unmountSD()` or you may cause damage to the card.
*/

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

/*JSON{
  "type" : "kill",
  "generate" : "jswrap_file_kill"
}*/
void jswrap_file_kill() {
  JsVar *arr = fsGetArray(false);
  if (arr) {
    JsvObjectIterator it;
    jsvObjectIteratorNew(&it, arr);
    while (jsvObjectIteratorHasValue(&it)) {
      JsVar *file = jsvObjectIteratorGetValue(&it);
      jswrap_file_close(file);
      jsvUnLock(file);
      jsvObjectIteratorNext(&it);
    }
    jsvObjectIteratorFree(&it);
    jsvRemoveAllChildren(arr);
    jsvUnLock(arr);
  }
  // close fs library
#ifndef LINUX
  if (fat_initialised) {
    fat_initialised = false;
    f_mount(0, 0, 0);
  }
#endif
#ifdef SD_CARD_ANYWHERE
  sdSPISetup(0, PIN_UNDEFINED);
#endif
}

/*JSON{
  "type" : "staticmethod",
  "class" : "E",
  "name" : "unmountSD",
  "generate" : "jswrap_E_unmountSD"
}
Unmount the SD card, so it can be removed. If you remove the SD card without calling this you may cause corruption, and you will be unable to access another SD card until you reset Espruino or call `E.unmountSD()`.
*/
void jswrap_E_unmountSD() {
  jswrap_file_kill();
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

/*JSON{
  "type" : "staticmethod",
  "class" : "E",
  "name" : "openFile",
  "generate" : "jswrap_E_openFile",
  "params" : [
    ["path","JsVar","the path to the file to open."],
    ["mode","JsVar","The mode to use when opening the file. Valid values for mode are 'r' for read, 'w' for write new, 'w+' for write existing, and 'a' for append. If not specified, the default is 'r'."]
  ],
  "return" : ["JsVar","A File object"],
  "return_object" : "File"
}
Open a file
*/
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
      if (!jsfsGetPathString(pathStr, path)) {
        jsvUnLock(arr);
        return 0;
      }

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

    jsvUnLock(arr);
  }


  return file.fileVar;
}

/*JSON{
  "type" : "method",
  "class" : "File",
  "name" : "close",
  "generate_full" : "jswrap_file_close(parent)"
}
Close an open file.
*/
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

/*JSON{
  "type" : "method",
  "class" : "File",
  "name" : "write",
  "generate" : "jswrap_file_write",
  "params" : [
    ["buffer","JsVar","A string containing the bytes to write"]
  ],
  "return" : ["int32","the number of bytes written"]
}
write data to a file
*/
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

/*JSON{
  "type" : "method",
  "class" : "File",
  "name" : "read",
  "generate" : "jswrap_file_read",
  "params" : [
    ["length","int32","is an integer specifying the number of bytes to read."]
  ],
  "return" : ["JsVar","A string containing the characters that were read"]
}
Read data in a file in byte size chunks
*/
JsVar *jswrap_file_read(JsVar* parent, int length) {
  if (length<0) length=0;
  JsVar *buffer = 0;
  JsvStringIterator it;
  FRESULT res = 0;
  size_t bytesRead = 0;
  if (jsfsInit()) {
    JsFile file;
    if (fileGetFromVar(&file, parent)) {
      if(file.data.mode == FM_READ || file.data.mode == FM_READ_WRITE) {
        size_t actual = 0;
#ifndef LINUX
        // if we're able to load this into a flat string, do it!
        size_t len = f_size(&file.data.handle)-f_tell(&file.data.handle);
        if ( len == 0 ) { // file all read
          return 0; // if called from a pipe signal end callback
        }
        if (len > (size_t)length) len = (size_t)length;
        buffer = jsvNewFlatStringOfLength(len);
        if (buffer) {
          res = f_read(&file.data.handle, jsvGetFlatStringPointer(buffer), len, &actual);
          if (res) jsfsReportError("Unable to read file", res);
          fileSetVar(&file);
          return buffer;
        }
#endif
        char buf[32];

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

  return buffer;
}

/*JSON{
  "type" : "method",
  "class" : "File",
  "name" : "skip",
  "generate_full" : "jswrap_file_skip_or_seek(parent,nBytes,true)",
  "params" : [
    ["nBytes","int32","is a positive integer specifying the number of bytes to skip forwards."]
  ]
}
Skip the specified number of bytes forward in the file
*/
/*JSON{
  "type" : "method",
  "class" : "File",
  "name" : "seek",
  "generate_full" : "jswrap_file_skip_or_seek(parent,nBytes,false)",
  "params" : [
    ["nBytes","int32","is an integer specifying the number of bytes to skip forwards."]
  ]
}
Seek to a certain position in the file
*/
void jswrap_file_skip_or_seek(JsVar* parent, int nBytes, bool is_skip) {
  if (nBytes<0) {
    if ( is_skip ) 
	  jsWarn("Bytes to skip must be >=0");
    else 
	  jsWarn("Position to seek to must be >=0");
    return;
  }
  FRESULT res = 0;
  if (jsfsInit()) {
    JsFile file;
    if (fileGetFromVar(&file, parent)) {
      if(file.data.mode == FM_READ || file.data.mode == FM_WRITE || file.data.mode == FM_READ_WRITE) {
  #ifndef LINUX
        res = (FRESULT)f_lseek(&file.data.handle, (DWORD)(is_skip ? f_tell(&file.data.handle) : 0) + (DWORD)nBytes);
  #else
        fseek(file.data.handle, nBytes, is_skip ? SEEK_CUR : SEEK_SET);
  #endif
        fileSetVar(&file);
      }
    }
  }
  if (res) jsfsReportError(is_skip?"Unable to skip":"Unable to seek", res);
}

/*JSON{
  "type" : "method",
  "class" : "File",
  "name" : "pipe",
  "ifndef" : "SAVE_ON_FLASH",
  "generate" : "jswrap_pipe",
  "params" : [
    ["destination","JsVar","The destination file/stream that will receive content from the source."],
    ["options","JsVar",["An optional object `{ chunkSize : int=32, end : bool=true, complete : function }`","chunkSize : The amount of data to pipe from source to destination at a time","complete : a function to call when the pipe activity is complete","end : call the 'end' function on the destination when the source is finished"]]
  ]
}
Pipe this file to a stream (an object with a 'write' method)
*/

#ifdef USE_FLASHFS

/*JSON{
  "type" : "staticmethod",
  "class" : "E",
  "name" : "flashFatFS",
  "generate" : "jswrap_E_flashFatFS",
  "ifdef" : "USE_FLASHFS",
   "params" : [
    ["options","JsVar",["An optional object `{ addr : int=0x300000, sectors : int=256, readonly : bool=false, , format : bool=false }`","addr : start address in flash","sectors: number of sectors to use","readonly: set to true if you want to disable write","format:  Format the media"]]
  ],
  "return" : ["bool","True on success, or false on failure"]  
}
Change the paramters used for the flash filesystem.
The default address is the last 1Mb of 4Mb Flash, 0x300000, with total size of 1Mb.

Before first use the media needs to be formatted.
```
fs=require("fs");

if ( typeof(fs.readdirSync())==="undefined" ) {
  console.log("Formatting FS");
  E.flashFatFS({format:true});
}

fs.writeFileSync("bang.txt", "This is the way the world ends\nnot with a bang but a whimper.\n");

fs.readdirSync();
```

This will create a drive of 100 * 4096 bytes at 0x200000. Be careful with the selection of flash addresses as you can overwrite firmware!
*/

int jswrap_E_flashFatFS(JsVar* options) {
  uint32_t addr = FS_FLASH_BASE;
  uint16_t sectors = FS_SECTOR_COUNT;
  uint8_t readonly = 0;
  uint8_t format = 0;
  // addr : 0x300000, sectors :256, readonly:false
  if (jsvIsObject(options)) {
    JsVar *a = jsvObjectGetChild(options, "addr", false);
    if (a) {
      if (jsvIsNumeric(a) && jsvGetInteger(a)>0x100000)
        addr = jsvGetInteger(a);
    }
    JsVar *s = jsvObjectGetChild(options, "sectors", false);
    if (s) {
      if (jsvIsNumeric(s) && jsvGetInteger(s)>0)
        sectors = jsvGetInteger(s);
    }
    JsVar *r = jsvObjectGetChild(options, "readonly", false);
    if (r) {
      if (jsvIsBoolean(r))
        readonly = jsvGetBool(r);
		jsWarn("readonly not implemented");
    }
     JsVar *f = jsvObjectGetChild(options, "format", false);
    if (f) {
      if (jsvIsBoolean(f))
        format = jsvGetBool(f);
    }
     jsWarn( "E.flashFatFs a:%d, s: %d r: %d f: %d", addr, sectors, readonly, format );
  }
  else if (!jsvIsUndefined(options)) {
    jsExceptionHere(JSET_TYPEERROR, "'options' must be an object, or undefined");
  }
  
  uint8_t init=flashFatFsInit( addr, sectors, readonly, format );
  if (init) {
    if ( format ) {
      uint8_t res = f_mount(&jsfsFAT, "", 0);
      jsWarn("Formatting Flash");
      res = f_mkfs("", 1, 0);  // Super Floppy format, using all space (not partition table)
      if (res != FR_OK) {
        jsfsReportError("Flash Formatting error:",res);
        return false;
     }
   }    
  }
  jsfsInit();
  return true;
}
#endif
