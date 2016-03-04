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

#ifdef USE_HEATSHRINK
  #include "compress_heatshrink.h"
  #define COMPRESS heatshrink_encode
  #define DECOMPRESS heatshrink_decode
#else
  #include "compress_rle.h"
  #define COMPRESS rle_encode
  #define DECOMPRESS rle_decode
#endif

#ifdef LINUX
// file IO for load/save
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#endif

/*JSON{
  "type" : "library",
  "class" : "Flash",
  "ifndef" : "SAVE_ON_FLASH"
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
  if (jsvIsUndefined(data)) {
    jsExceptionHere(JSET_ERROR, "Data is not defined");
    return;
  }

  JSV_GET_AS_CHAR_ARRAY(flashData, flashDataLen, data);
  if ((addr&3) || (flashDataLen&3)) {
    jsExceptionHere(JSET_ERROR, "Data and address must be multiples of 4");
    return;
  }

  if (flashData && flashDataLen)
    jshFlashWrite(flashData, (unsigned int)addr, (unsigned int)flashDataLen);
}

/*JSON{
  "type" : "staticmethod",
  "ifndef" : "SAVE_ON_FLASH",
  "class" : "Flash",
  "name" : "read",
  "generate" : "jswrap_flash_read",
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


#ifndef LINUX
// cbdata = uint32_t[end_address, address, data]
void jsfSaveToFlash_writecb(unsigned char ch, uint32_t *cbdata) {
  // Only write if we can fit in flash
  if (cbdata[1]<cbdata[0]) {
    // write only a word at a time
    cbdata[2]=(uint32_t)(ch<<24) | (cbdata[2]>>8);
    if ((cbdata[1]&3)==3)
      jshFlashWrite(&cbdata[2], cbdata[1]&(uint32_t)~3, 4);
  }
  // inc address ptr
  cbdata[1]++;
  if ((cbdata[1]&1023)==0) jsiConsolePrint(".");
}
// cbdata = uint32_t[address, errorcount]
void jsfSaveToFlash_checkcb(unsigned char ch, uint32_t *cbdata) {
  unsigned char data;
  jshFlashRead(&data,cbdata[0]++, 1);
  if (data!=ch) cbdata[1]++; // error count
}
// cbdata = uint32_t[end_address, address]
int jsfLoadFromFlash_readcb(uint32_t *cbdata) {
  if (cbdata[1]>=cbdata[0]) return -1; // at end
  unsigned char data;
  jshFlashRead(&data, cbdata[1]++, 1);
  return data;
}
#else
int jsfLoadFromFlash_readcb(uint32_t *cbdata) {
  unsigned char ch;
  if (fread(&ch,1,1,(FILE*)cbdata)==1) return ch;
  return -1;
}

void jsfSaveToFlash_writecb(unsigned char ch, uint32_t *cbdata) {
  fwrite(&ch,1,1,(FILE*)cbdata);
}
#endif


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
    fwrite(&jsVarCount, sizeof(unsigned int), 1, f);
    /*JsVarRef i;
    for (i=1;i<=jsVarCount;i++) {
      fwrite(_jsvGetAddressOf(i),1,sizeof(JsVar),f);
    }*/
    COMPRESS((unsigned char*)_jsvGetAddressOf(1), jsVarCount*sizeof(JsVar), jsfSaveToFlash_writecb, (uint32_t*)f);
    fclose(f);
    jsiConsolePrint("\nDone!\n");

#ifdef DEBUG
    jsiConsolePrint("Checking...\n");
    FILE *f = fopen("espruino.state","rb");
    fread(&jsVarCount, sizeof(unsigned int), 1, f);
    if (jsVarCount != jsvGetMemoryTotal())
      jsiConsolePrint("Error: memory sizes different\n");
    unsigned char *decomp = (unsigned char*)malloc(jsVarCount*sizeof(JsVar));
    DECOMPRESS(jsfLoadFromFlash_readcb, (uint32_t *)f, decomp);
    fclose(f);
    unsigned char *comp = (unsigned char *)_jsvGetAddressOf(1);
    size_t j;
    for (j=0;j<jsVarCount*sizeof(JsVar);j++)
      if (decomp[j]!=comp[j])
        jsiConsolePrintf("Error at %d: original %d, decompressed %d\n", j, comp[j], decomp[j]);
    free(decomp);
    jsiConsolePrint("Done!\n>");
#endif
  } else {
    jsiConsolePrint("\nFile Open Failed... \n>");
  }
#else // !LINUX
  unsigned int dataSize = jsvGetMemoryTotal() * sizeof(JsVar);
  uint32_t *basePtr = (uint32_t *)_jsvGetAddressOf(1);
  uint32_t pageStart, pageLength;
  bool tryAgain = true;
  bool success = false;
  uint32_t writtenBytes;
  uint32_t endOfData;
  uint32_t cbData[3];
  uint32_t rleStart;

  while (tryAgain) {
    tryAgain = false;
    jsiConsolePrint("Erasing Flash...");
    uint32_t addr = FLASH_SAVED_CODE_START;
    if (jshFlashGetPage((uint32_t)addr, &pageStart, &pageLength)) {
      jshFlashErasePage(pageStart);
      while (pageStart+pageLength < FLASH_MAGIC_LOCATION) { // until end address
        jsiConsolePrint(".");
        addr = pageStart+pageLength; // next page
        if (!jshFlashGetPage((uint32_t)addr, &pageStart, &pageLength)) break;
        jshFlashErasePage(pageStart);

      }
    }
    rleStart = FLASH_SAVED_CODE_START+4;
    cbData[0] = FLASH_MAGIC_LOCATION; // end of available flash
    cbData[1] = rleStart;
    cbData[2] = 0; // word data (can only save a word ata a time)
    jsiConsolePrint("\nWriting...");
    COMPRESS((unsigned char*)basePtr, dataSize, jsfSaveToFlash_writecb, cbData);
    endOfData = cbData[1];
    writtenBytes = endOfData - FLASH_SAVED_CODE_START;
    // make sure we write everything in buffer
    jsfSaveToFlash_writecb(0,cbData);
    jsfSaveToFlash_writecb(0,cbData);
    jsfSaveToFlash_writecb(0,cbData);

    if (cbData[1]>=cbData[0]) {
      jsiConsolePrintf("\nERROR: Too big to save to flash (%d vs %d bytes)\n", writtenBytes, FLASH_MAGIC_LOCATION-FLASH_SAVED_CODE_START);
      jsvSoftInit();
      jspSoftInit();
      if (jsiFreeMoreMemory()) {
        jsiConsolePrint("Deleting command history and trying again...\n");
        while (jsiFreeMoreMemory());
        tryAgain = true;
      }
      jspSoftKill();
      jsvSoftKill();
    } else {
      success = true;
    }
  }

  if (success) {
    jsiConsolePrintf("\nCompressed %d bytes to %d", dataSize, writtenBytes);
    jshFlashWrite(&endOfData, FLASH_SAVED_CODE_START, 4); // write position of end of data, at start of address space

    uint32_t magic = FLASH_MAGIC;
    jshFlashWrite(&magic, FLASH_MAGIC_LOCATION, 4);

    jsiConsolePrint("\nChecking...");
    cbData[0] = rleStart;
    cbData[1] = 0; // increment if fails
    COMPRESS((unsigned char*)basePtr, dataSize, jsfSaveToFlash_checkcb, cbData);
    uint32_t errors = cbData[1];

    if (!jsfFlashContainsCode()) {
      jsiConsolePrint("\nFlash Magic Byte is wrong");
      errors++;
    }

    if (errors)
      jsiConsolePrintf("\nThere were %d errors!\n>", errors);
    else
      jsiConsolePrint("\nDone!\n");
  }
#endif
}

void jsfLoadFromFlash() {
#ifdef LINUX
  FILE *f = fopen("espruino.state","rb");
  if (f) {
    unsigned int jsVarCount;
    fread(&jsVarCount, sizeof(unsigned int), 1, f);

    jsiConsolePrintf("\nDecompressing to %d bytes...", jsVarCount*sizeof(JsVar));
    jsvSetMemoryTotal(jsVarCount);
    /*JsVarRef i;
    for (i=1;i<=jsVarCount;i++) {
      fread(_jsvGetAddressOf(i),1,sizeof(JsVar),f);
    }*/
    DECOMPRESS(jsfLoadFromFlash_readcb, (uint32_t*)f, (unsigned char*)_jsvGetAddressOf(1));
    fclose(f);
  } else {
    jsiConsolePrint("\nFile Open Failed... \n");
  }
#else // !LINUX
  if (!jsfFlashContainsCode()) {
    jsiConsolePrintf("No code in flash!\n");
    return;
  }

  //  unsigned int dataSize = jsvGetMemoryTotal() * sizeof(JsVar);
  uint32_t *basePtr = (uint32_t *)_jsvGetAddressOf(1);

  uint32_t cbData[2];
  jshFlashRead(&cbData[0], FLASH_SAVED_CODE_START, 4); // end address
  cbData[1] = FLASH_SAVED_CODE_START+4; // start address
  jsiConsolePrintf("Loading %d bytes from flash...\n", cbData[0]-FLASH_SAVED_CODE_START);
  DECOMPRESS(jsfLoadFromFlash_readcb, cbData, (unsigned char*)basePtr);
#endif
}

bool jsfFlashContainsCode() {
#ifdef LINUX
  FILE *f = fopen("espruino.state","rb");
  if (f) fclose(f);
  return f!=0;
#else // !LINUX
  int magic;
  jshFlashRead(&magic, FLASH_MAGIC_LOCATION, sizeof(magic));
  return magic == (int)FLASH_MAGIC;
#endif
}
