/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2018 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * This file is designed to be parsed during the build process
 *
 * JavaScript Filesystem-style Flash IO functions
 * ----------------------------------------------------------------------------
 */
#include "jswrap_storage.h"
#include "jswrap_flash.h"
#include "jshardware.h"
#include "jsflash.h"
#include "jsvar.h"
#include "jsvariterator.h"
#include "jsparse.h"
#include "jsinteractive.h"
#include "jswrap_json.h"

#ifdef DEBUG
#define DBG(...) jsiConsolePrintf("[Storage] "__VA_ARGS__)
#else
#define DBG(...)
#endif

const int STORAGEFILE_CHUNKSIZE = FLASH_PAGE_SIZE - sizeof(JsfFileHeader); // use 32 for testing

/*JSON{
  "type" : "library",
  "class" : "Storage",
  "ifndef" : "SAVE_ON_FLASH"
}

This module allows you to read and write part of the nonvolatile flash
memory of your device using a filesystem-like API.

Also see the `Flash` library, which provides a low level, more dangerous way
to access all parts of your flash memory.

**Note:** In firmware 2v05 and later, the maximum length for filenames
is 28 characters. However in 2v04 and earlier the max length is 8.
*/

/*JSON{
  "type" : "staticmethod",
  "ifndef" : "SAVE_ON_FLASH",
  "class" : "Storage",
  "name" : "eraseAll",
  "generate" : "jswrap_storage_eraseAll"
}
Erase the flash storage area. This will remove all files
created with `require("Storage").write(...)` as well
as any code saved with `save()` or `E.setBootCode()`.
 */
void jswrap_storage_eraseAll() {
  jsfEraseAll();
}

/*JSON{
  "type" : "staticmethod",
  "ifndef" : "SAVE_ON_FLASH",
  "class" : "Storage",
  "name" : "erase",
  "generate" : "jswrap_storage_erase",
  "params" : [
    ["name","JsVar","The filename - max 28 characters (case sensitive)"]
  ]
}
Erase a single file from the flash storage area.
 */
void jswrap_storage_erase(JsVar *name) {
  jsfEraseFile(jsfNameFromVar(name));
}

/*JSON{
  "type" : "staticmethod",
  "ifndef" : "SAVE_ON_FLASH",
  "class" : "Storage",
  "name" : "read",
  "generate" : "jswrap_storage_read",
  "params" : [
    ["name","JsVar","The filename - max 28 characters (case sensitive)"],
    ["offset","int","(optional) The offset in bytes to start from"],
    ["length","int","(optional) The length to read in bytes (if <=0, the entire file is read)"]
  ],
  "return" : ["JsVar","A string of data"]
}
Read a file from the flash storage area that has
been written with `require("Storage").write(...)`.

This function returns a String that points to the actual
memory area in read-only memory, so it won't use up RAM.

If you evaluate this string with `eval`, any functions
contained in the String will keep their code stored
in flash memory.
*/
JsVar *jswrap_storage_read(JsVar *name, int offset, int length) {
  return jsfReadFile(jsfNameFromVar(name), offset, length);
}

/*JSON{
  "type" : "staticmethod",
  "ifndef" : "SAVE_ON_FLASH",
  "class" : "Storage",
  "name" : "readJSON",
  "generate" : "jswrap_storage_readJSON",
  "params" : [
    ["name","JsVar","The filename - max 28 characters (case sensitive)"],
    ["noExceptions","bool","If true and the JSON is not valid, just return `undefined` - otherwise an `Exception` is thrown"]
  ],
  "return" : ["JsVar","An object containing parsed JSON from the file, or undefined"]
}
Read a file from the flash storage area that has
been written with `require("Storage").write(...)`,
and parse JSON in it into a JavaScript object.

This is identical to `JSON.parse(require("Storage").read(...))`.
It will throw an exception if the data in the file is not
valid JSON.
*/
JsVar *jswrap_storage_readJSON(JsVar *name, bool noExceptions) {
  JsVar *v = jsfReadFile(jsfNameFromVar(name),0,0);
  if (!v) return 0;
  JsVar *r = jswrap_json_parse(v);
  jsvUnLock(v);
  if (noExceptions) jsvUnLock(jspGetException());
  return r;
}

/*JSON{
  "type" : "staticmethod",
  "ifndef" : "SAVE_ON_FLASH",
  "class" : "Storage",
  "name" : "readArrayBuffer",
  "generate" : "jswrap_storage_readArrayBuffer",
  "params" : [
    ["name","JsVar","The filename - max 28 characters (case sensitive)"]
  ],
  "return" : ["JsVar","An ArrayBuffer containing data from the file, or undefined"]
}
Read a file from the flash storage area that has
been written with `require("Storage").write(...)`,
and return the raw binary data as an ArrayBuffer.

This can be used:

* In a `DataView` with `new DataView(require("Storage").readArrayBuffer("x"))`
* In a `Uint8Array/Float32Array/etc` with `new Uint8Array(require("Storage").readArrayBuffer("x"))`
*/
JsVar *jswrap_storage_readArrayBuffer(JsVar *name) {
  JsVar *v = jsfReadFile(jsfNameFromVar(name),0,0);
  if (!v) return 0;
  JsVar *r = jsvNewArrayBufferFromString(v, 0);
  jsvUnLock(v);
  return r;
}

/*JSON{
  "type" : "staticmethod",
  "ifndef" : "SAVE_ON_FLASH",
  "class" : "Storage",
  "name" : "write",
  "generate" : "jswrap_storage_write",
  "params" : [
    ["name","JsVar","The filename - max 28 characters (case sensitive)"],
    ["data","JsVar","The data to write"],
    ["offset","int","The offset within the file to write"],
    ["size","int","The size of the file (if a file is to be created that is bigger than the data)"]
  ],
  "return" : ["bool","True on success, false on failure"]
}
Write/create a file in the flash storage area. This is
nonvolatile and will not disappear when the device resets
or power is lost.

Simply write `require("Storage").write("MyFile", "Some data")` to write
a new file, and `require("Storage").read("MyFile")` to read it.

If you supply:

* A String, it will be written as-is
* An array, will be written as a byte array (but read back as a String)
* An object, it will automatically be converted to
a JSON string before being written.

**Note:** If an array is supplied it will not be converted to JSON.
To be explicit about the conversion you can use `Storage.writeJSON`

You may also create a file and then populate data later **as long as you
don't try and overwrite data that already exists**. For instance:

```
var f = require("Storage");
f.write("a","Hello",0,14);
f.write("a"," ",5);
f.write("a","World!!!",6);
print(f.read("a"));
```

This can be useful if you've got more data to write than you
have RAM available.
*/
bool jswrap_storage_write(JsVar *name, JsVar *data, JsVarInt offset, JsVarInt _size) {
  JsVar *d;
  if (jsvIsObject(data)) {
    d = jswrap_json_stringify(data,0,0);
    offset = 0;
    _size = 0;
  } else
    d = jsvLockAgainSafe(data);
  bool success = jsfWriteFile(jsfNameFromVar(name), d, JSFF_NONE, offset, _size);
  jsvUnLock(d);
  return success;
}


/*JSON{
  "type" : "staticmethod",
  "ifndef" : "SAVE_ON_FLASH",
  "class" : "Storage",
  "name" : "writeJSON",
  "generate" : "jswrap_storage_writeJSON",
  "params" : [
    ["name","JsVar","The filename - max 28 characters (case sensitive)"],
    ["data","JsVar","The JSON data to write"]
  ],
  "return" : ["bool","True on success, false on failure"]
}
Write/create a file in the flash storage area. This is
nonvolatile and will not disappear when the device resets
or power is lost.

Simply write `require("Storage").writeJSON("MyFile", [1,2,3])` to write
a new file, and `require("Storage").readJSON("MyFile")` to read it.

This is equivalent to: `require("Storage").write(name, JSON.stringify(data))`
*/
bool jswrap_storage_writeJSON(JsVar *name, JsVar *data) {
  JsVar *d = jswrap_json_stringify(data,0,0);
  return jsfWriteFile(jsfNameFromVar(name), d, JSFF_NONE, 0, 0);
}

/*JSON{
  "type" : "staticmethod",
  "ifndef" : "SAVE_ON_FLASH",
  "class" : "Storage",
  "name" : "list",
  "generate" : "jswrap_storage_list",
  "params" : [
    ["regex","JsVar","(optional) If supplied, filenames are checked against this regular expression (with `String.match(regexp)`) to see if they match before being returned"]
  ],
  "return" : ["JsVar","An array of filenames"]
}
List all files in the flash storage area. An array of Strings is returned.

**Note:** This will output system files (eg. saved code) as well as
files that you may have written.
 */
JsVar *jswrap_storage_list(JsVar *regex) {
  return jsfListFiles(regex);
}

/*JSON{
  "type" : "staticmethod",
  "ifndef" : "SAVE_ON_FLASH",
  "class" : "Storage",
  "name" : "compact",
  "generate" : "jswrap_storage_compact"
}
The Flash Storage system is journaling. To make the most of the limited
write cycles of Flash memory, Espruino marks deleted/replaced files as
garbage and moves on to a fresh part of flash memory. Espruino only
fully erases those files when it is running low on flash, or when
`compact` is called.

`compact` may fail if there isn't enough RAM free on the stack to
use as swap space, however in this case it will not lose data.

**Note:** `compact` rearranges the contents of memory. If code is
referencing that memory (eg. functions that have their code stored in flash)
then they may become garbled when compaction happens. To avoid this,
call `eraseFiles` before uploading data that you intend to reference to
ensure that uploaded files are right at the start of flash and cannot be
compacted further.
 */
void jswrap_storage_compact() {
  jsfCompact();
}

/*JSON{
  "type" : "staticmethod",
  "ifdef" : "DEBUG",
  "class" : "Storage",
  "name" : "debug",
  "generate" : "jswrap_storage_debug"
}
This writes information about all blocks in flash
memory to the console - and is only useful for debugging
flash storage.
 */
void jswrap_storage_debug() {
  jsfDebugFiles();
}

/*JSON{
  "type" : "staticmethod",
  "ifndef" : "SAVE_ON_FLASH",
  "class" : "Storage",
  "name" : "getFree",
  "generate" : "jswrap_storage_getFree",
  "return" : ["int","The amount of free bytes"]
}
Return the amount of free bytes available in
Storage. Due to fragmentation there may be more
bytes available, but this represents the maximum
size of file that can be written.
 */
int jswrap_storage_getFree() {
  return (int)jsfGetFreeSpace(0,true);
}

/*JSON{
  "type" : "staticmethod",
  "ifndef" : "SAVE_ON_FLASH",
  "class" : "Storage",
  "name" : "open",
  "generate" : "jswrap_storage_open",
  "params" : [
    ["name","JsVar","The filename - max **7** characters (case sensitive)"],
    ["mode","JsVar","The open mode - must be either `'r'` for read,`'w'` for write , or `'a'` for append"]
  ],
  "return" : ["JsVar","An object containing {read,write,erase}"],
  "return_object" : "StorageFile"
}
Open a file in the Storage area. This can be used for appending data
(normal read/write operations only write the entire file).

Please see `StorageFile` for more information (and examples).

**Note:** These files write through immediately - they do not need closing.

*/
JsVar *jswrap_storage_open(JsVar *name, JsVar *modeVar) {
  char mode = 0;
  if (jsvIsStringEqual(modeVar,"r")) mode='r';
  else if (jsvIsStringEqual(modeVar,"w")) mode='w';
  else if (jsvIsStringEqual(modeVar,"a")) mode='a';
  else {
    jsExceptionHere(JSET_ERROR, "Invalid mode %j", modeVar);
    return 0;
  }

  JsVar *f = jspNewObject(0, "StorageFile");
  if (!f) return 0;

  int chunk = 1;

  JsVar *n = jsvNewFromStringVar(name,0,sizeof(JsfFileName));
  JsfFileName fname = jsfNameFromVar(n);
  int fnamei = sizeof(fname)-1;
  while (fnamei && fname.c[fnamei-1]==0) fnamei--;
  fname.c[fnamei]=chunk;
  jsvObjectSetChildAndUnLock(f,"name",n);

  int offset = 0; // offset in file
  JsfFileHeader header;
  uint32_t addr = jsfFindFile(fname, &header);
  if (mode=='w') { // write,
    if (addr) { // we had a file - erase it
      jswrap_storagefile_erase(f);
      addr = 0;
    }
  }
  if (mode=='a') { // append
    // Find the last free page
    unsigned char lastCh = 255;
    if (addr) jshFlashRead(&lastCh, addr+jsfGetFileSize(&header)-1, 1);
    while (addr && lastCh!=255 && chunk<255) {
      chunk++;
      fname.c[fnamei]=chunk;
      addr = jsfFindFile(fname, &header);
      if (addr) jshFlashRead(&lastCh, addr+jsfGetFileSize(&header)-1, 1);
    }
    if (addr) {
      // if we have a page, try and find the end of it
      char buf[64];
      bool foundEnd = false;
      while (!foundEnd) {
        int l = STORAGEFILE_CHUNKSIZE-offset;
        if (l<=0) {
          foundEnd = true;
          break;
        }
        if (l>(int)sizeof(buf)) l=(int)sizeof(buf);
        jshFlashRead(buf, addr+offset, l);
        for (int i=0;i<l;i++) {
          if (buf[i]==(char)255) {
            l = i;
            foundEnd = true;
            break;
          }
        }
        offset += l;
      }
    }
    // Now 'chunk' and offset points to the last (or a free) page
  }
  if (mode=='r') {
    // read - do nothing, we're good.
  }

  DBG("Open %j Chunk %d Offset %d addr 0x%08x\n",name,chunk,offset,addr);
  jsvObjectSetChildAndUnLock(f,"chunk",jsvNewFromInteger(chunk));
  jsvObjectSetChildAndUnLock(f,"offset",jsvNewFromInteger(offset));
  jsvObjectSetChildAndUnLock(f,"addr",jsvNewFromInteger(addr));
  jsvObjectSetChildAndUnLock(f,"mode",jsvNewFromInteger(mode));

  return f;
}

/*JSON{
  "type" : "class",
  "class" : "StorageFile",
  "ifndef" : "SAVE_ON_FLASH"
}

These objects are created from `require("Storage").open`
and allow Storage items to be read/written.

The `Storage` library writes into Flash memory (which
can only be erased in chunks), and unlike a normal filesystem
it allocates files in one long contiguous area to allow them
to be accessed easily from Espruino.

This presents a challenge for `StorageFile` which allows you
to append to a file, so instead `StorageFile` stores files
in chunks. It uses 7 character filenames and uses the last
character to denote the chunk number (eg `"foobar\1"`, `"foobar\2"`, etc).

This means that while `StorageFile` files exist in the same
area as those from `Storage`, they should be
read using `StorageFile.open` (and not `Storage.read`).

```
f = s.open("foobar","w");
f.write("Hell");
f.write("o World\n");
f.write("Hello\n");
f.write("World 2\n");
// there's no need to call 'close'
// then
f = s.open("foobar","r");
f.read(13) // "Hello World\nH"
f.read(13) // "ello\nWorld 2\n"
f.read(13) // "Hello World 3"
f.read(13) // "\n"
f.read(13) // undefined
// or
f = s.open("foobar","r");
f.readLine() // "Hello World\n"
f.readLine() // "Hello\n"
f.readLine() // "World 2\n"
f.readLine() // "Hello World 3\n"
f.readLine() // undefined
// now get rid of file
f.erase();
```

**Note:** `StorageFile` uses the fact that all bits of erased flash memory
are 1 to detect the end of a file. As such you should not write character
code 255 (`"\xFF"`) to these files.
*/

JsVar *jswrap_storagefile_read_internal(JsVar *f, int len) {
  bool isReadLine = len<0;
  char mode = (char)jsvGetIntegerAndUnLock(jsvObjectGetChild(f,"mode",0));
  if (mode!='r') {
    jsExceptionHere(JSET_ERROR, "Can't read in this mode");
    return 0;
  }

  uint32_t addr = (uint32_t)jsvGetIntegerAndUnLock(jsvObjectGetChild(f,"addr",0));
  if (!addr) return 0; // end of file
  int offset = jsvGetIntegerAndUnLock(jsvObjectGetChild(f,"offset",0));
  int chunk = jsvGetIntegerAndUnLock(jsvObjectGetChild(f,"chunk",0));
  JsfFileName fname = jsfNameFromVarAndUnLock(jsvObjectGetChild(f,"name",0));
  int fnamei = sizeof(fname)-1;
  while (fnamei && fname.c[fnamei-1]==0) fnamei--;
  fname.c[fnamei]=chunk;

  JsVar *result = 0;
  char buf[32];
  if (isReadLine) len = sizeof(buf);
  while (len) {
    int remaining = STORAGEFILE_CHUNKSIZE-offset;
    if (remaining<=0) { // next page
      offset = 0;
      if (chunk==255) {
        addr=0;
      } else {
        chunk++;
        fname.c[fnamei]=chunk;
        JsfFileHeader header;
        addr = jsfFindFile(fname, &header);
      }
      jsvObjectSetChildAndUnLock(f,"addr",jsvNewFromInteger(addr));
      jsvObjectSetChildAndUnLock(f,"offset",jsvNewFromInteger(offset));
      jsvObjectSetChildAndUnLock(f,"chunk",jsvNewFromInteger(chunk));
      remaining = STORAGEFILE_CHUNKSIZE;
      if (!addr) {
        // end of file!
        return result;
      }
    }
    int l = len;
    if (l>(int)sizeof(buf)) l=(int)sizeof(buf);
    if (l>remaining) l=remaining;
    jshFlashRead(buf, addr+offset, l);
    for (int i=0;i<l;i++) {
      if (buf[i]==(char)255) {
        // end of file!
        l = i;
        len = l;
        break;
      }
      if (isReadLine && buf[i]=='\n') {
        l = i+1;
        len = l;
        isReadLine = false; // done
        break;
      }
    }

    if (!l) break;
    if (!result)
      result = jsvNewFromEmptyString();
    if (result)
      jsvAppendStringBuf(result,buf,l);

    len -= l;
    offset += l;
    // if we're still reading lines, set the length to buffer size
    if (isReadLine)
      len = sizeof(buf);
  }
  jsvObjectSetChildAndUnLock(f,"offset",jsvNewFromInteger(offset));
  return result;
}
/*JSON{
  "type" : "method",
  "ifndef" : "SAVE_ON_FLASH",
  "class" : "StorageFile",
  "name" : "read",
  "generate" : "jswrap_storagefile_read",
  "params" : [
    ["len","int","How many bytes to read"]
  ],
  "return" : ["JsVar","A String, or undefined "],
  "return_object" : "String"
}
Read 'len' bytes of data from the file, and return a String containing those bytes.

If the end of the file is reached, the String may be smaller than the amount of bytes
requested, or if the file is already at the end, `undefined` is returned.
*/
JsVar *jswrap_storagefile_read(JsVar *f, int len) {
  if (len<0) len=0;
  return jswrap_storagefile_read_internal(f,len);
}
/*JSON{
  "type" : "method",
  "ifndef" : "SAVE_ON_FLASH",
  "class" : "StorageFile",
  "name" : "readLine",
  "generate" : "jswrap_storagefile_readLine",
  "return" : ["JsVar","A line of data"],
  "return_object" : "String"
}
Read a line of data from the file (up to and including `"\n"`)
*/
JsVar *jswrap_storagefile_readLine(JsVar *f) {
  return jswrap_storagefile_read_internal(f,-1);
}
/*JSON{
  "type" : "method",
  "ifndef" : "SAVE_ON_FLASH",
  "class" : "StorageFile",
  "name" : "getLength",
  "generate" : "jswrap_storagefile_getLength",
  "return" : ["int","The current length in bytes of the file"]
}
Return the length of the current file.

This requires Espruino to read the file from scratch,
which is not a fast operation.
*/
int jswrap_storagefile_getLength(JsVar *f) {
  // Get name and position of name digit
  JsVar *n = jsvObjectGetChild(f,"name",0);
  JsfFileName fname = jsfNameFromVar(n);
  jsvUnLock(n);
  int fnamei = sizeof(fname)-1;
  while (fnamei && fname.c[fnamei-1]==0) fnamei--;
  int chunk = 1;
  fname.c[fnamei]=chunk;

  int length = 0; // actual length
  int offset = 0; // offset in file
  JsfFileHeader header;
  uint32_t addr = jsfFindFile(fname, &header);
  // Find the last free page
  unsigned char lastCh = 255;
  if (addr) jshFlashRead(&lastCh, addr+jsfGetFileSize(&header)-1, 1);
  while (addr && lastCh!=255 && chunk<255) {
    length += jsfGetFileSize(&header);
    chunk++;
    fname.c[fnamei]=chunk;
    addr = jsfFindFile(fname, &header);
    if (addr) jshFlashRead(&lastCh, addr+jsfGetFileSize(&header)-1, 1);
  }
  if (addr) {
    // if we have a page, try and find the end of it
    char buf[64];
    bool foundEnd = false;
    while (!foundEnd) {
      int l = STORAGEFILE_CHUNKSIZE-offset;
      if (l<=0) {
        foundEnd = true;
        break;
      }
      if (l>sizeof(buf)) l=sizeof(buf);
      jshFlashRead(buf, addr+offset, l);
      for (int i=0;i<l;i++) {
        if (buf[i]==(char)255) {
          l = i;
          foundEnd = true;
          break;
        }
      }
      offset += l;
    }
  }
  length += offset;
  return length;
}



/*JSON{
  "type" : "method",
  "ifndef" : "SAVE_ON_FLASH",
  "class" : "StorageFile",
  "name" : "write",
  "generate" : "jswrap_storagefile_write",
  "params" : [
    ["data","JsVar","The data to write. This should not include `'\\xFF'` (character code 255)"]
  ]
}
Append the given data to a file. You should not attempt to append  `"\xFF"` (character code 255).
*/
void jswrap_storagefile_write(JsVar *f, JsVar *_data) {
  char mode = (char)jsvGetIntegerAndUnLock(jsvObjectGetChild(f,"mode",0));
  if (mode!='w' && mode!='a') {
    jsExceptionHere(JSET_ERROR, "Can't write in this mode");
    return;
  }

  JsVar *data = jsvAsString(_data);
  if (!data) return;
  size_t len = jsvGetStringLength(data);
  if (len==0) return;
  int offset = jsvGetIntegerAndUnLock(jsvObjectGetChild(f,"offset",0));
  int chunk = jsvGetIntegerAndUnLock(jsvObjectGetChild(f,"chunk",0));
  JsfFileName fname = jsfNameFromVarAndUnLock(jsvObjectGetChild(f,"name",0));
  int fnamei = sizeof(fname)-1;
  while (fnamei && fname.c[fnamei-1]==0) fnamei--;
  //DBG("Filename[%d]=%d\n",fnamei,chunk);
  fname.c[fnamei]=chunk;
  uint32_t addr = (uint32_t)jsvGetIntegerAndUnLock(jsvObjectGetChild(f,"addr",0));
  DBG("Write Chunk %d Offset %d addr 0x%08x\n",chunk,offset,addr);
  int remaining = STORAGEFILE_CHUNKSIZE - offset;
  if (!addr) {
    DBG("Write Create Chunk\n");
    if (jsfWriteFile(fname, data, JSFF_STORAGEFILE, 0, STORAGEFILE_CHUNKSIZE)) {
      JsfFileHeader header;
      addr = jsfFindFile(fname, &header);
      offset = len;
      jsvObjectSetChildAndUnLock(f,"offset",jsvNewFromInteger(offset));
      jsvObjectSetChildAndUnLock(f,"addr",jsvNewFromInteger(addr));
    } else {
      // there would already have been an exception
    }
    jsvUnLock(data);
    return;
  }
  if ((int)len<remaining) {
    DBG("Write Append Chunk\n");
    // Great, it all fits in
    jswrap_flash_write(data, addr+offset);
    offset += len;
    jsvObjectSetChildAndUnLock(f,"offset",jsvNewFromInteger(offset));
  } else {
    DBG("Write Append Chunk and create new\n");
    // Fill up this page, do part of old page
    // End of this page
    JsVar *part = jsvNewFromStringVar(data,0,remaining);
    jswrap_flash_write(part, addr+offset);
    jsvUnLock(part);
    // Next page
    if (chunk==255) {
      jsExceptionHere(JSET_ERROR, "File too big!");
      jsvUnLock(data);
      return;
    } else {
      chunk++;
      fname.c[fnamei]=chunk;
      jsvObjectSetChildAndUnLock(f,"chunk",jsvNewFromInteger(chunk));
    }
    // Write Next page
    part = jsvNewFromStringVar(data,remaining,JSVAPPENDSTRINGVAR_MAXLENGTH);
    if (jsfWriteFile(fname, part, JSFF_STORAGEFILE, 0, STORAGEFILE_CHUNKSIZE)) {
      JsfFileHeader header;
      addr = jsfFindFile(fname, &header);
      offset = len;
      jsvObjectSetChildAndUnLock(f,"offset",jsvNewFromInteger(offset));
      jsvObjectSetChildAndUnLock(f,"addr",jsvNewFromInteger(addr));
    } else {
      jsvUnLock(data);
      return; // there would already have been an exception
    }
    offset = jsvGetStringLength(part);
    jsvUnLock(part);
    jsvObjectSetChildAndUnLock(f,"offset",jsvNewFromInteger(offset));
  }
  jsvUnLock(data);
}

/*JSON{
  "type" : "method",
  "ifndef" : "SAVE_ON_FLASH",
  "class" : "StorageFile",
  "name" : "erase",
  "generate" : "jswrap_storagefile_erase"
}
Erase this file
*/
void jswrap_storagefile_erase(JsVar *f) {
  JsfFileName fname = jsfNameFromVarAndUnLock(jsvObjectGetChild(f,"name",0));
  int fnamei = sizeof(fname)-1;
  while (fnamei && fname.c[fnamei-1]==0) fnamei--;
  // erase all numbered files
  int chunk = 1;
  bool ok = true;
  while (ok) {
    fname.c[fnamei]=chunk;
    ok = jsfEraseFile(fname);
    chunk++;
  }
  // reset everything
  jsvObjectSetChildAndUnLock(f,"chunk",jsvNewFromInteger(1));
  jsvObjectSetChildAndUnLock(f,"offset",jsvNewFromInteger(0));
  jsvObjectSetChildAndUnLock(f,"addr",jsvNewFromInteger(0));
  jsvObjectSetChildAndUnLock(f,"mode",jsvNewFromInteger(0));
}
