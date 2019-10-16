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

/*JSON{
  "type" : "library",
  "class" : "Storage",
  "ifndef" : "SAVE_ON_FLASH"
}

This module allows you to read and write part of the nonvolatile flash
memory of your device using a filesystem-like API.

Also see the `Flash` library, which provides a low level, more dangerous way
to access all parts of your flash memory.
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
    ["name","JsVar","The filename - max 8 characters (case sensitive)"]
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
    ["name","JsVar","The filename - max 8 characters (case sensitive)"]
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
JsVar *jswrap_storage_read(JsVar *name) {
  return jsfReadFile(jsfNameFromVar(name));
}

/*JSON{
  "type" : "staticmethod",
  "ifndef" : "SAVE_ON_FLASH",
  "class" : "Storage",
  "name" : "readJSON",
  "generate" : "jswrap_storage_readJSON",
  "params" : [
    ["name","JsVar","The filename - max 8 characters (case sensitive)"]
  ],
  "return" : ["JsVar","An object containing parsed JSON from the file, or undefined"]
}
Read a file from the flash storage area that has
been written with `require("Storage").write(...)`,
and parse JSON in it into a JavaScript object.

This is identical to `JSON.parse(require("Storage").read(...))`
*/
JsVar *jswrap_storage_readJSON(JsVar *name) {
  JsVar *v = jsfReadFile(jsfNameFromVar(name));
  if (!v) return 0;
  JsVar *r = jswrap_json_parse(v);
  jsvUnLock(v);
  return r;
}

/*JSON{
  "type" : "staticmethod",
  "ifndef" : "SAVE_ON_FLASH",
  "class" : "Storage",
  "name" : "readArrayBuffer",
  "generate" : "jswrap_storage_readArrayBuffer",
  "params" : [
    ["name","JsVar","The filename - max 8 characters (case sensitive)"]
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
  JsVar *v = jsfReadFile(jsfNameFromVar(name));
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
    ["name","JsVar","The filename - max 8 characters (case sensitive)"],
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
  "name" : "list",
  "generate" : "jswrap_storage_list",
  "return" : ["JsVar","An array of filenames"]
}
List all files in the flash storage area. An array of Strings is returned.

**Note:** This will output system files (eg. saved code) as well as
files that you may have written.
 */
JsVar *jswrap_storage_list() {
  return jsfListFiles();
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
    ["name","JsVar","The filename - max **7** characters (case sensitive)"]
  ],
  "return" : ["JsVar","An object containing {read,write,erase,close}"],
  "return_object" : "StorageFile"
}
Open a file in the Storage area. This can be used for appending data to (normal read/write operations
ony write the entire file).
*/
// TODO: add seek
const int STORAGEFILE_CHUNKSIZE = 32;//FLASH_PAGE_SIZE - sizeof(JsfFileHeader);
JsVar *jswrap_storage_open(JsVar *name) {
  JsVar *f = jspNewObject(0, "StorageFile");
  if (!f) return 0;

  JsVar *n = jsvNewFromStringVar(name,0,8);
  JsfFileName fname = jsfNameFromVar(n);
  jsvObjectSetChildAndUnLock(f,"name",n);

  int chunk = 1;
  JsfFileHeader header;
  fname.c[sizeof(fname)-1]=chunk;
  unsigned char lastCh = 255;

  uint32_t addr = jsfFindFile(fname, &header);
  if (addr) jshFlashRead(&lastCh, addr+jsfGetFileSize(&header)-1, 1);
  while (addr && lastCh!=255) {
    chunk++;
    fname.c[sizeof(fname)-1]=chunk;
    addr = jsfFindFile(fname, &header);
    if (addr) jshFlashRead(&lastCh, addr+jsfGetFileSize(&header)-1, 1);
  }
  // Now 'chunk' points to the last (or a free) page
  int offset = 0;
  // TODO: Look through a pre-opened file to find the end
  DBG("Open %j Chunk %d Offset %d addr 0x%08x\n",name,chunk,offset,addr);
  jsvObjectSetChildAndUnLock(f,"chunk",jsvNewFromInteger(chunk));
  jsvObjectSetChildAndUnLock(f,"offset",jsvNewFromInteger(offset));
  jsvObjectSetChildAndUnLock(f,"addr",jsvNewFromInteger(addr));

  return f;
}

/*JSON{
  "type" : "class",
  "class" : "StorageFile",
  "ifndef" : "SAVE_ON_FLASH"
}

These objects are created from `require("Storage").open`
and allow Storage items to be read/written.
*/
/*JSON{
  "type" : "method",
  "ifndef" : "SAVE_ON_FLASH",
  "class" : "StorageFile",
  "name" : "write",
  "generate" : "jswrap_storagefile_write",
  "params" : [
    ["data","JsVar","The data to write"]
  ]
}
Append the given data to a file
*/
void jswrap_storagefile_write(JsVar *f, JsVar *_data) {
  JsVar *data = jsvAsString(_data);
  if (!data) return;
  size_t len = jsvGetStringLength(data);
  if (len==0) return;
  JsfFileName fname = jsfNameFromVarAndUnLock(jsvObjectGetChild(f,"name",0));
  int offset = jsvGetIntegerAndUnLock(jsvObjectGetChild(f,"offset",0));
  int chunk = jsvGetIntegerAndUnLock(jsvObjectGetChild(f,"chunk",0));
  uint32_t addr = (uint32_t)jsvGetIntegerAndUnLock(jsvObjectGetChild(f,"addr",0));
  DBG("Write Chunk %d Offset %d addr 0x%08x\n",chunk,offset,addr);
  int remaining = STORAGEFILE_CHUNKSIZE - offset;
  if (!addr) {
    DBG("Write Create Chunk\n");
    if (jsfWriteFile(fname, data, JSFF_NONE, 0, STORAGEFILE_CHUNKSIZE)) {
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
  if (len<remaining) {
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
    chunk++;
    fname.c[sizeof(fname)-1]=chunk;
    jsvObjectSetChildAndUnLock(f,"chunk",jsvNewFromInteger(chunk));
    // Write Next page
    part = jsvNewFromStringVar(data,remaining,JSVAPPENDSTRINGVAR_MAXLENGTH);
    if (jsfWriteFile(fname, part, JSFF_NONE, 0, STORAGEFILE_CHUNKSIZE)) {
      JsfFileHeader header;
      addr = jsfFindFile(fname, &header);
      offset = len;
      jsvObjectSetChildAndUnLock(f,"offset",jsvNewFromInteger(offset));
      jsvObjectSetChildAndUnLock(f,"addr",jsvNewFromInteger(addr));
    } else {
      jsvUnLock(data);
      return; // there would already have been an exception
    }
    offset = jsvGetStringLength(jsvGetStringLength(data));
    jsvUnLock(part);
    jsvObjectSetChildAndUnLock(f,"offset",jsvNewFromInteger(offset));
  }
  jsvUnLock(data);
}
