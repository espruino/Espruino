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
 * JavaScript Flash IO functions
 * ----------------------------------------------------------------------------
 */
#include "jswrap_flash.h"
#include "jshardware.h"
#include "jsvariterator.h"
#include "jsinteractive.h"

#ifdef LINUX
 // file IO for load/save
 #include <stdlib.h>
 #include <string.h>
 #include <stdio.h>
#endif

/*JSON{
  "type" : "library",
  "class" : "Flash"
}

This module allows access to read and write the STM32's flash memory.

It should be used with extreme caution, as it is easy to overwrite parts of Flash
memory belonging to Espruino or even its bootloader. If you damage the bootloader
then you may need external hardware such as a USB-TTL converter to restore it. For
more information on restoring the bootloader see `Advanced Reflashing` in your
board's reference pages.

To see which areas of memory you can and can't overwrite, look at the values
reported by `process.memory()`.
*/

/*JSON{
  "type" : "staticmethod",
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
  JsVar *obj = jsvNewWithFlags(JSV_OBJECT);
  if (!obj) return 0;
  jsvUnLock(jsvObjectSetChild(obj, "addr", jsvNewFromInteger((JsVarInt)pageStart)));
  jsvUnLock(jsvObjectSetChild(obj, "length", jsvNewFromInteger((JsVarInt)pageLength)));
  return obj;
}

/*JSON{
  "type" : "staticmethod",
  "class" : "Flash",
  "name" : "erasePage",
  "generate" : "jswrap_flash_erasePage",
  "params" : [
    ["addr","int","An address in the page that is to be erased"]
  ]
}
Erase a page of flash memory
*/
void jswrap_flash_erasePage(int addr) {
  jshFlashErasePage((uint32_t)addr);
}

/*JSON{
  "type" : "staticmethod",
  "class" : "Flash",
  "name" : "write",
  "generate" : "jswrap_flash_write",
  "params" : [
    ["data","JsVar","The data to write. This must be a multiple of 4 bytes."],
    ["addr","int","The address to start writing from, this must be on a word boundary (a multiple of 4)"]
  ]
}
Write data into memory at the given address - IN MULTIPLES OF 4 BYTES.

In flash memory you may only turn bits that are 1 into bits that are 0. If
you're writing data into an area that you have already written (so `read`
doesn't return all `0xFF`) you'll need to call `erasePage` to clear the
entire page.
*/
void jswrap_flash_write(JsVar *data, int addr) {
  size_t l = (size_t)jsvIterateCallbackCount(data);
  if ((addr&3) || (l&3)) {
    jsExceptionHere(JSET_ERROR, "Data and address must be multiples of 4");
    return;
  }
  if (l+256 > jsuGetFreeStack()) {
    jsExceptionHere(JSET_ERROR, "Not enough free stack to send this amount of data");
    return;
  }

  unsigned char *bytes = (unsigned char *)alloca(l);
  jsvIterateCallbackToBytes(data, bytes, (unsigned int)l);
  jshFlashWrite(bytes, (unsigned int)addr, (unsigned int)l);
}

/*JSON{
  "type" : "staticmethod",
  "class" : "Flash",
  "name" : "write",
  "generate" : "jswrap_flash_write",
  "params" : [
    ["length","int","The amount of data to read (in bytes)"],
    ["addr","int","The address to start writing from"]
  ],
  "return" : ["JsVar","A Uint8Array of data"]
}
Read flash memory from the given address
*/
JsVar *jswrap_flash_read(int length, int addr) {
  if (length<=0) return 0;
  JsVar *arr = jsvNewTypedArray(ARRAYBUFFERVIEW_UINT8, length);
  if (!arr) return 0;
  JsvArrayBufferIterator it;
  jsvArrayBufferIteratorNew(&it, arr, 0);
  while (jsvArrayBufferIteratorHasElement(&it)) {
    char c;
    jshFlashRead(&c, (uint32_t)(addr++), 1);
    jsvArrayBufferIteratorSetByteValue(&it, c);
    jsvArrayBufferIteratorNext(&it);
  }
  jsvArrayBufferIteratorFree(&it);
  return arr;
}

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
//                                                  Global flash read/write
// ------------------------------------------------------------------------
// ------------------------------------------------------------------------

void jsfSaveToFlash() {
#ifdef LINUX
  FILE *f = fopen("espruino.state","wb");
  if (f) {
    unsigned int jsVarCount = jsvGetMemoryTotal();
    jsiConsolePrintf("\nSaving %d bytes...", jsVarCount*sizeof(JsVar));
    JsVarRef i;

    for (i=1;i<=jsVarCount;i++) {
      fwrite(_jsvGetAddressOf(i),1,sizeof(JsVar),f);
    }
    fclose(f);
    jsiConsolePrint("\nDone!\n>");
  } else {
    jsiConsolePrint("\nFile Open Failed... \n>");
  }
#else // !LINUX
  unsigned int dataSize = jsvGetMemoryTotal() * sizeof(JsVar);
  uint32_t *basePtr = (uint32_t *)_jsvGetAddressOf(1);
  uint32_t pageStart, pageLength;

  jsiConsolePrint("Erasing Flash...");
  int addr = FLASH_SAVED_CODE_START;
  if (jshFlashGetPage((uint32_t)addr, &pageStart, &pageLength)) {
    jshFlashErasePage(pageStart);
    while (pageStart+pageLength < FLASH_SAVED_CODE_START+dataSize) {
      jsiConsolePrint(".");
      addr = pageStart+pageLength; // next page
      if (!jshFlashGetPage((uint32_t)addr, &pageStart, &pageLength)) break;
      jshFlashErasePage(pageStart);
    }
  }

  jsiConsolePrint("\nWriting...");
  unsigned int i;
  for (i=0;i<dataSize;i+=1024) {
    int l = dataSize-i;
    if (l>1024) l=1024;
    jshFlashWrite(&basePtr[i>>2], FLASH_SAVED_CODE_START+i, l);
    jsiConsolePrint(".");
  }
  int magic = FLASH_MAGIC;
  jshFlashWrite(&magic, FLASH_MAGIC_LOCATION, 4);

  jsiConsolePrint("\nChecking...");
  int data;
  int errors = 0;
  for (i=0;i<dataSize;i+=4) {
    jshFlashRead(&data, FLASH_SAVED_CODE_START+i, 4);
    if (data != basePtr[i>>2])
      errors++;
  }

  if (!jsfFlashContainsCode()) {
    jsiConsolePrint("\nFlash Magic Byte is wrong");
    errors++;
  }

  if (errors)
    jsiConsolePrintf("\nThere were %d errors!\n>", errors);
  else
    jsiConsolePrint("\nDone!\n");
#endif
}

void jsfLoadFromFlash() {
#ifdef LINUX
  FILE *f = fopen("espruino.state","rb");
  if (f) {
    fseek(f, 0L, SEEK_END);
    unsigned int fileSize = (unsigned int)ftell(f);
    fseek(f, 0L, SEEK_SET);

    jsiConsolePrintf("\nLoading %d bytes...\n>", fileSize);

    unsigned int jsVarCount = fileSize / sizeof(JsVar);
    jsvSetMemoryTotal(jsVarCount);
    JsVarRef i;
    for (i=1;i<=jsVarCount;i++) {
      fread(_jsvGetAddressOf(i),1,sizeof(JsVar),f);
    }
    fclose(f);
  } else {
    jsiConsolePrint("\nFile Open Failed... \n>");
  }
#else // !LINUX
  if (!jsfFlashContainsCode()) {
    jsiConsolePrintf("No code in flash!\n");
    return;
  }

  unsigned int dataSize = jsvGetMemoryTotal() * sizeof(JsVar);
  uint32_t *basePtr = (uint32_t *)_jsvGetAddressOf(1);

  jsiConsolePrintf("Loading from flash...\n");
  jshFlashRead(basePtr, FLASH_SAVED_CODE_START, dataSize);
#endif
}

bool jsfFlashContainsCode() {
#ifdef LINUX
  FILE *f = fopen("espruino.state","rb");
  if (f) fclose(f);
  return f!=0;
#else // !LINUX
  int magic;
  jshFlashRead(&magic ,FLASH_MAGIC_LOCATION, sizeof(magic));
  return (*(int*)FLASH_MAGIC_LOCATION) == (int)FLASH_MAGIC;
#endif
}
