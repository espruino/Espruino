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
 * JavaScript Flash IO functions
 * ----------------------------------------------------------------------------
 */
#include "jswrap_flash.h"
#include "jshardware.h"
#include "jsflash.h"
#include "jsvar.h"
#include "jsvariterator.h"

/*JSON{
  "type" : "library",
  "class" : "Flash",
  "ifndef" : "SAVE_ON_FLASH"
}

This module allows you to read and write the nonvolatile flash memory of your
device.

Also see the `Storage` library, which provides a safer file-like interface to
nonvolatile storage.

It should be used with extreme caution, as it is easy to overwrite parts of
Flash memory belonging to Espruino or even its bootloader. If you damage the
bootloader then you may need external hardware such as a USB-TTL converter to
restore it. For more information on restoring the bootloader see `Advanced
Reflashing` in your board's reference pages.

To see which areas of memory you can and can't overwrite, look at the values
reported by `process.memory()`.

**Note:** On Nordic platforms there are checks in place to help you avoid
'bricking' your device be damaging the bootloader. You can disable these with
`E.setFlags({unsafeFlash:1})`
 */

/*JSON{
  "type" : "staticmethod",
  "ifndef" : "SAVE_ON_FLASH",
  "class" : "Flash",
  "name" : "getPage",
  "generate" : "jswrap_flash_getPage",
  "params" : [
    ["addr","int","An address in memory"]
  ],
  "return" : ["JsVar","An object of the form `{ addr : #, length : #}`, where `addr` is the start address of the page, and `length` is the length of it (in bytes). Returns undefined if no page at address"]
}
Returns the start and length of the flash page containing the given address.
 */
JsVar *jswrap_flash_getPage(int addr) {
  uint32_t pageStart, pageLength;
  if (!jshFlashGetPage((uint32_t)addr, &pageStart, &pageLength))
    return 0;
  JsVar *obj = jsvNewObject();
  if (!obj) return 0;
  jsvObjectSetChildAndUnLock(obj, "addr", jsvNewFromInteger((JsVarInt)pageStart));
  jsvObjectSetChildAndUnLock(obj, "length", jsvNewFromInteger((JsVarInt)pageLength));
  return obj;
}

/*JSON{
  "type"     : "staticmethod",
    "ifndef" : "SAVE_ON_FLASH",
  "class"    : "Flash",
  "name"     : "getFree",
  "generate" : "jswrap_flash_getFree",
  "return"   : ["JsVar", "Array of objects with `addr` and `length` properties"]
}
This method returns an array of objects of the form `{addr : #, length : #}`,
representing contiguous areas of flash memory in the chip that are not used for
anything.

The memory areas returned are on page boundaries. This means that you can safely
erase the page containing any address here, and you won't risk deleting part of
the Espruino firmware.
*/
JsVar *jswrap_flash_getFree() {
  JsVar *arr = jshFlashGetFree();
  if (!arr) arr=jsvNewEmptyArray();
  return arr;
}

/*JSON{
  "type" : "staticmethod",
  "ifndef" : "SAVE_ON_FLASH",
  "class" : "Flash",
  "name" : "erasePage",
  "generate" : "jswrap_flash_erasePage",
  "params" : [
    ["addr","JsVar","An address in the page that is to be erased"]
  ]
}
Erase a page of flash memory
 */
void jswrap_flash_erasePage(JsVar *addr) {
  if (!jsvIsInt(addr)) {
    jsExceptionHere(JSET_ERROR, "Address should be an integer, got %t", addr);
    return;
  }
  jshFlashErasePage((uint32_t)jsvGetInteger(addr));
}

/*JSON{
  "type" : "staticmethod",
  "ifndef" : "SAVE_ON_FLASH",
  "class" : "Flash",
  "name" : "write",
  "generate" : "jswrap_flash_write",
  "params" : [
    ["data","JsVar","The data to write"],
    ["addr","int","The address to start writing from"]
  ]
}
Write data into memory at the given address

In flash memory you may only turn bits that are 1 into bits that are 0. If
you're writing data into an area that you have already written (so `read`
doesn't return all `0xFF`) you'll need to call `erasePage` to clear the entire
page.
 */
void jswrap_flash_write(JsVar *data, int addr) {
  if (jsvIsUndefined(data)) {
    jsExceptionHere(JSET_ERROR, "Data is not defined");
    return;
  }

  JSV_GET_AS_CHAR_ARRAY(flashData, flashDataLen, data);

  if (flashData && flashDataLen)
    jshFlashWriteAligned(flashData, (unsigned int)addr, (unsigned int)flashDataLen);
}

/*JSON{
  "type" : "staticmethod",
  "ifndef" : "SAVE_ON_FLASH",
  "class" : "Flash",
  "name" : "read",
  "generate" : "jswrap_flash_read",
  "params" : [
    ["length","int","The amount of data to read (in bytes)"],
    ["addr","int","The address to start reading from"]
  ],
  "return" : ["JsVar","A Uint8Array of data"]
}
Read flash memory from the given address
 */
JsVar *jswrap_flash_read(int length, int addr) {
  if (length<=0) return 0;
  JsVar *arr = jsvNewTypedArray(ARRAYBUFFERVIEW_UINT8, length);
  if (!arr) return 0;
  uint32_t offset;
  JsVar *str = jsvGetArrayBufferBackingString(arr, &offset);
  if (str) {
    JsvStringIterator it;
    jsvStringIteratorNew(&it, str, offset);
    while (length>0 && jsvStringIteratorHasChar(&it)) {
      unsigned char *data;
      unsigned int l = 0;
      jsvStringIteratorGetPtrAndNext(&it, &data, &l);
      jshFlashRead(data, (uint32_t)addr, l);
      addr += l;
      length -= l;
    }
    jsvStringIteratorFree(&it);
    jsvUnLock(str);
  }
  return arr;
}
