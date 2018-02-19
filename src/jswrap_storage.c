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
#include "jswrap_json.h"

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
  return jswrap_json_parse(v);
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
  return jsvNewArrayBufferFromString(v, 0);
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

You may also create a file and then populate data later as long as you
don't try and overwrite data that already exists. For instance:

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
`compactFiles` is called.

`compactFiles` may fail if there isn't enough RAM free on the stack to
use as swap space, however in this case it will not lose data.

**Note:** `compactFiles` rearranges the contents of memory. If code is
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
