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
  "type"     : "staticmethod",
    "ifndef" : "SAVE_ON_FLASH",
  "class"    : "Flash",
  "name"     : "getFree",
  "generate" : "jswrap_flash_getFree",
  "return"   : ["JsVar", "Array of objects with `addr` and `length` properties"]
}
This method returns an array of objects of the form `{addr : #, length : #}`, representing
contiguous areas of flash memory in the chip that are not used for anything.

The memory areas returned are on page boundaries. This means that you can
safely erase the page containing any address here, and you won't risk
deleting part of the Espruino firmware.
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


// cbdata = uint32_t[end_address, address, data]
void jsfSaveToFlash_writecb(unsigned char ch, uint32_t *cbdata) {

#ifdef FLASH_64BITS_ALIGNEMENT
	static uint32_t dataToWrite[2];
#endif

  // Only write if we can fit in flash
  if (cbdata[1]<cbdata[0]) {
    // write only a word at a time
    cbdata[2]=(uint32_t)(ch<<24) | (cbdata[2]>>8);
#ifndef FLASH_64BITS_ALIGNEMENT
    if ((cbdata[1]&3)==3)
    jshFlashWrite(&cbdata[2], cbdata[1]&(uint32_t)~3, 4);
#else
	// We want the flash writes to be done every 64 bits.
	// Store the first 32 bits and write on next 32 bits word.
    if ((cbdata[1]&7)==7){
      dataToWrite[1] = cbdata[2];
      jshFlashWrite(dataToWrite, cbdata[1]&(uint32_t)~7, 8);
    } else if ((cbdata[1]&3)==3){
      dataToWrite[0] = cbdata[2];
    }
#endif
  }
  // inc address ptr
  cbdata[1]++;
  if ((cbdata[1]&1023)==0) jsiConsolePrint(".");
}

// cbdata = uint32_t[address, errorcount]
void jsfSaveToFlash_checkcb(unsigned char ch, uint32_t *cbdata) {
  unsigned char data;

  jshFlashRead(&data,cbdata[0]++, 1);
  if (data!=ch) {
      //jsiConsolePrintf("\n checkcb error : ch=%x, cbdata: %x, %x, data = %x", ch, cbdata[0], cbdata[1], data);
      cbdata[1]++; // error count
  }
}
// cbdata = uint32_t[end_address, address]
int jsfLoadFromFlash_readcb(uint32_t *cbdata) {
  if (cbdata[1]>=cbdata[0]) return -1; // at end
  unsigned char data;
  jshFlashRead(&data, cbdata[1]++, 1);
  return data;
}

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
//                                                  Global flash read/write
// ------------------------------------------------------------------------
// ------------------------------------------------------------------------

/*
 *   Code is saved starting from FLASH_SAVED_CODE_START.
 *   A magic number written to FLASH_MAGIC_LOCATION (the end of saved code)
 *     determines whether flash data has been successfully written or not
 *   The first word at FLASH_SAVED_CODE_START is the amount of Boot code
 *      that is saved
 *   The second word at FLASH_SAVED_CODE_START+4 is the end address of
 *      decompressed JS code
 *   Boot code starts at FLASH_SAVED_CODE_START+8
 *   Saved state starts at FLASH_SAVED_CODE_START+8+boot_code_length
 *
 */

#define BOOT_CODE_LENGTH_MASK 0x00FFFFFF
#define BOOT_CODE_RUN_ALWAYS  0x80000000

#ifndef FLASH_64BITS_ALIGNEMENT
#define FLASH_UNITARY_WRITE_SIZE 4
#else
#define FLASH_UNITARY_WRITE_SIZE 8
#endif

#define FLASH_BOOT_CODE_INFO_LOCATION FLASH_SAVED_CODE_START
#define FLASH_STATE_END_LOCATION (FLASH_SAVED_CODE_START+FLASH_UNITARY_WRITE_SIZE)
#define FLASH_DATA_LOCATION (FLASH_SAVED_CODE_START+2*FLASH_UNITARY_WRITE_SIZE)

void jsfSaveToFlash(JsvSaveFlashFlags flags, JsVar *bootCode) {
  unsigned int dataSize = jsvGetMemoryTotal() * (unsigned int)sizeof(JsVar);
  uint32_t *basePtr = (uint32_t *)_jsvGetAddressOf(1);
  uint32_t pageStart, pageLength;
  bool tryAgain = true;
  bool success = false;
  uint32_t writtenBytes;
  uint32_t endOfData;
  uint32_t cbData[3];

  /* If we didn't specify boot code this time, but boot code was set previously,
   * load it into RAM so we can keep it. */
  uint32_t originalBootCodeInfo = 0;
  char *originalBootCode = 0;
  uint32_t bootCodeLen = 0;
  if (!(jsvIsString(bootCode) && jsvGetStringLength(bootCode)) && jsfFlashContainsCode()) {
    jshFlashRead(&originalBootCodeInfo, FLASH_BOOT_CODE_INFO_LOCATION, 4);
    bootCodeLen = originalBootCodeInfo & BOOT_CODE_LENGTH_MASK;
    if (bootCodeLen == BOOT_CODE_LENGTH_MASK) bootCodeLen = 0;
    if (bootCodeLen) {
      if (originalBootCodeInfo & BOOT_CODE_RUN_ALWAYS)
        flags |= SFF_BOOT_CODE_ALWAYS;
      else
        flags &= ~SFF_BOOT_CODE_ALWAYS;
      if (bootCodeLen+64 < jsuGetFreeStack())
        originalBootCode = (char *)alloca(bootCodeLen);
      if (originalBootCode) {
        jshFlashRead(originalBootCode, FLASH_DATA_LOCATION, bootCodeLen);
      } else {
        // There may not be room on the stack, in which case we'll warn
        jsWarn("Unable to keep Boot Code - not enough room on the stack\n");
        bootCodeLen = 0;
      }
    }
  }

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
    // Now start writing
    cbData[0] = FLASH_MAGIC_LOCATION; // end of available flash
    cbData[1] = FLASH_DATA_LOCATION;
    cbData[2] = 0; // word data (can only save a word ata a time)
    jsiConsolePrint("\nWriting...");
    // boot code....
    if (jsvIsString(bootCode)) {
      bootCodeLen = (uint32_t)jsvGetStringLength(bootCode);
      if (bootCodeLen) {
        // Only write code if we actually have any
        originalBootCodeInfo = bootCodeLen;
        if (flags & SFF_BOOT_CODE_ALWAYS)
          originalBootCodeInfo |= BOOT_CODE_RUN_ALWAYS;
        JsvStringIterator it;
        jsvStringIteratorNew(&it, bootCode, 0);
        while (jsvStringIteratorHasChar(&it)) {
          jsfSaveToFlash_writecb(jsvStringIteratorGetChar(&it), cbData);
          jsvStringIteratorNext(&it);
        }
        // terminate with a 0!
        jsfSaveToFlash_writecb(0, cbData);
        bootCodeLen++;
      }

    } else if (originalBootCode) {
      // previously saved boot code that we want to keep
      assert(originalBootCode && bootCodeLen);
      size_t i;
      for (i=0;i<bootCodeLen;i++)
        jsfSaveToFlash_writecb(originalBootCode[i], cbData);
    }
    // write size of boot code to flash
    jshFlashWrite(&originalBootCodeInfo, FLASH_BOOT_CODE_INFO_LOCATION, FLASH_UNITARY_WRITE_SIZE);
    // state....
    if (flags & SFF_SAVE_STATE) {
      COMPRESS((unsigned char*)basePtr, dataSize, jsfSaveToFlash_writecb, cbData);
    }
    endOfData = cbData[1];
    // make sure we write everything in buffer
    int i;
    for(i=0;i<FLASH_UNITARY_WRITE_SIZE;i++)
      jsfSaveToFlash_writecb(0,cbData);

    writtenBytes = endOfData - FLASH_SAVED_CODE_START;

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
    jshFlashWrite(&endOfData, FLASH_STATE_END_LOCATION, FLASH_UNITARY_WRITE_SIZE); // write position of end of data, at start of address space

    uint32_t magic = FLASH_MAGIC;
    jshFlashWrite(&magic, FLASH_MAGIC_LOCATION, FLASH_UNITARY_WRITE_SIZE);

    jsiConsolePrint("\nChecking...");
    cbData[0] = FLASH_DATA_LOCATION;
    if (bootCodeLen) {
      cbData[0] += bootCodeLen;
    }
    cbData[1] = 0; // increment if fails
    // TODO: check boot code written ok
    if (flags & SFF_SAVE_STATE)
      COMPRESS((unsigned char*)basePtr, dataSize, jsfSaveToFlash_checkcb, cbData);
    uint32_t errors = cbData[1];

    if (!jsfFlashContainsCode()) {
      jsiConsolePrint("\nFlash Magic Byte is wrong");

      errors++;
    }

    if (errors)
      jsiConsolePrintf("\nThere were %d errors!\n", errors);
    else
      jsiConsolePrint("\nDone!\n");
  }
}


/// Load the RAM image from flash (this is the actual interpreter state)
void jsfLoadStateFromFlash() {
  if (!jsfFlashContainsCode()) {
    jsiConsolePrintf("No code in flash!\n");
    return;
  }

  //  unsigned int dataSize = jsvGetMemoryTotal() * sizeof(JsVar);
  uint32_t *basePtr = (uint32_t *)_jsvGetAddressOf(1);

  uint32_t cbData[2];
  uint32_t bootCodeLen;
  jshFlashRead(&bootCodeLen, FLASH_BOOT_CODE_INFO_LOCATION, 4); // length of boot code
  bootCodeLen &= BOOT_CODE_LENGTH_MASK;
  jshFlashRead(&cbData[0], FLASH_STATE_END_LOCATION, 4); // end address
  cbData[1] = FLASH_DATA_LOCATION; // start address
  if (bootCodeLen) cbData[1] += bootCodeLen;
  uint32_t len = cbData[0]-FLASH_SAVED_CODE_START;
  if (len>1000000) {
    jsiConsolePrintf("Invalid saved code in flash!\n");
    return;
  }
  jsiConsolePrintf("Loading %d bytes from flash...\n", len);
  DECOMPRESS(jsfLoadFromFlash_readcb, cbData, (unsigned char*)basePtr);
}

/** Get bootup code from flash (this is textual JS code). return a pointer to it if it exists, or 0.
 * isReset should be set if we're loading after a reset (eg, does the user expect this to be run or not).
 * Set isReset=false to always return the code  */
const char *jsfGetBootCodeFromFlash(bool isReset) {
  char *code = 0;
  if (!jsfFlashContainsCode()) return 0;

  uint32_t bootCodeInfo;
  jshFlashRead(&bootCodeInfo, FLASH_BOOT_CODE_INFO_LOCATION, 4); // length of boot code
  uint32_t bootCodeLen = bootCodeInfo & BOOT_CODE_LENGTH_MASK;
  if (bootCodeLen==BOOT_CODE_LENGTH_MASK || !bootCodeLen) return 0;
  // Don't execute code if we've reset and code shouldn't always be run
  if (isReset && !(bootCodeInfo & BOOT_CODE_RUN_ALWAYS)) return 0;

#ifdef ESP32
  // romdata_jscode is memory mapped address from the js_code partition in rom - targets/esp32/main.c
  extern char* romdata_jscode;
  if (romdata_jscode==0) {
    jsError("Couldn't find js_code partition - update with partition_espruino.bin\n");
    return 0;
  }
  code = &romdata_jscode[FLASH_DATA_LOCATION-FLASH_SAVED_CODE_START];
#else
  code = (char *)(FLASH_DATA_LOCATION);
#endif  
#ifdef ESP8266
  // the flash address is just the offset into the flash chip, but to evaluate the code
  // below we need to jump to the memory-mapped window onto flash, so adjust here
  code += 0x40200000;
#endif
  return code;
}

/** Load bootup code from flash (this is textual JS code). return true if it exists and was executed.
 * isReset should be set if we're loading after a reset (eg, does the user expect this to be run or not).
 * Set isReset=false to always run the code
 */
bool jsfLoadBootCodeFromFlash(bool isReset) {
  const char *code = jsfGetBootCodeFromFlash(isReset);
  if (code)
    jsvUnLock(jspEvaluate(code, true /* We are expecting this ptr to hang around */));
  return true;
}

bool jsfFlashContainsCode() {
  int magic;
  jshFlashRead(&magic, FLASH_MAGIC_LOCATION, sizeof(magic));
  return magic == (int)FLASH_MAGIC;
}

/** Completely clear any saved code from flash. */
void jsfRemoveCodeFromFlash() {
  jsiConsolePrint("Erasing saved code.");
  uint32_t pageStart, pageLength;
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
  jsiConsolePrint("\nDone!\n");
}



// ----------------------------------------------------------------------------------------------

#ifdef DEBUG
#define DBG(...) jsiConsolePrintf("[Flash] "__VA_ARGS__)
#else
#define DBG(...)
#endif

typedef uint64_t JsfFileName;
typedef struct {
  uint32_t size; ///< Total size
  JsfFileName name; ///< 0-padded filename
  uint32_t replacement; ///< pointer to a replacement (eventually). For now this is 0xFFFFFFFF if ok, 0 if erased
} JsfFileHeader; // keep this

#define JSF_ALIGNMENT 4
#define JSF_START_ADDRESS FLASH_SAVED_CODE_START
#define JSF_END_ADDRESS (FLASH_SAVED_CODE_START+FLASH_SAVED_CODE_LENGTH)

bool jsfGetFileHeader(uint32_t addr, JsfFileHeader *header) {
  assert(header);
  jshFlashRead(header, addr, sizeof(JsfFileHeader));
  return header->size != 0xFFFFFFFF;
}

bool jsfIsErased(uint32_t addr, uint32_t len) {
  for (uint32_t x=0;x<len;x+=4) {
    uint32_t buf;
    jshFlashRead(&buf, addr+x,4);
    if (buf!=0xFFFFFFFF) return false;
  }
  return true;
}

bool jsfIsEqual(uint32_t addr, const unsigned char *data, uint32_t len) {
  for (uint32_t x=0;x<len;) {
    uint32_t bufa;
    jshFlashRead(&bufa, addr+x,4);
    uint32_t bufb = 0xFFFFFFFF;
    for (int i=0;i<4;i++) {
      if (x<len) bufb = (bufb>>8) | (data[x]<<24);
      x++;
    }
    if (bufa!=bufb) return false;
  }
  return true;
}

/// Erase the entire contents of the memory store
bool jsfEraseAll() {
  DBG("EraseAll\n");
  uint32_t addr, len;
  if (!jshFlashGetPage(JSF_START_ADDRESS, &addr, &len))
    return false;
  while (addr<JSF_END_ADDRESS) {
    if (!jsfIsErased(addr,len))
      jshFlashErasePage(addr);
    if (!jshFlashGetPage(addr+len, &addr, &len))
      return true;
  }
  return true;
}

/// When a file is found in memory, erase it (by setting replacement to 0). addr=ptr to data, NOT header
void jsfEraseFile(uint32_t addr, JsfFileHeader *header) {
  DBG("EraseFile 0x%08x\n", addr);

  addr -= sizeof(JsfFileHeader);
  addr += (uint32_t)((char*)&header->replacement - (char*)header);
  header->replacement = 0;
  jshFlashWrite(&header->replacement,addr,(uint32_t)sizeof(header->replacement));
}

// Given the address and a header, work out where the next one should be
uint32_t jsfGetAddressOfNextHeader(uint32_t addr, JsfFileHeader *header) {
  // Work out roughly where the start is
  uint32_t newAddr = addr + header->size + (uint32_t)sizeof(JsfFileHeader);
  // pad out to flash write boundaries
  addr = (addr + (JSF_ALIGNMENT-1)) & (uint32_t)~(JSF_ALIGNMENT-1);
  // sanity check for bad data
  if (newAddr<addr) return 0; // corrupt!
  if (newAddr+sizeof(JsfFileHeader)>JSF_END_ADDRESS) return 0; // not enough space
  return newAddr;
}

// Get the amount of space free in this page
uint32_t jsfGetFreeSpaceInPage(uint32_t addr) {
  uint32_t pageAddr,pageLen;
  if (!jshFlashGetPage(addr, &pageAddr, &pageLen))
    return 0;
  assert(addr==pageAddr);
  uint32_t pageEndAddr = pageAddr+pageLen;
  JsfFileHeader header;
  while (jsfGetFileHeader(addr, &header) && addr+(uint32_t)sizeof(JsfFileHeader)<pageEndAddr) {
    addr = jsfGetAddressOfNextHeader(addr, &header);
    if (!addr) return 0; // corrupt!
  }
  return pageEndAddr-addr;
}

// Get the amount of space needed to mirror this page elsewhere
uint32_t jsfGetAllocatedSpaceInPage(uint32_t addr) {
  uint32_t allocated = 0;
  uint32_t pageAddr,pageLen;
  if (!jshFlashGetPage(addr, &pageAddr, &pageLen))
    return 0;
  assert(addr==pageAddr);
  uint32_t pageEndAddr = pageAddr+pageLen;
  JsfFileHeader header;
  while (jsfGetFileHeader(addr, &header) && addr+(uint32_t)sizeof(JsfFileHeader)<pageEndAddr) {
    if (header.replacement == 0xFFFFFFFF) // if not replaced
      allocated += header.size + (uint32_t)sizeof(JsfFileHeader);
    addr = jsfGetAddressOfNextHeader(addr, &header);
    if (!addr) return 0; // corrupt!
  }
  return allocated;
}

uint32_t jsfCompact() {
  DBG("Compacting");
  return 0;
}

/// Create a new 'file' in the memory store. Return the address of data start, or 0 on error
uint32_t jsfCreateFile(JsfFileName name, uint32_t size, uint32_t startAddr, JsfFileHeader *returnedHeader) {
  DBG("CreateFile\n");
  uint32_t addr = startAddr;
  uint32_t existingAddr = 0;
  JsfFileHeader header;
  while (jsfGetFileHeader(addr, &header) && addr+sizeof(JsfFileHeader)<JSF_END_ADDRESS) {
    // check for something with the same name
    if (header.replacement == 0xFFFFFFFF &&
        header.name == name)
      existingAddr = addr;
    addr = jsfGetAddressOfNextHeader(addr, &header);
    if (!addr) return 0; // corrupt!
  }
  // do we have an existing file? Erase it.
  if (existingAddr) {
    jsfGetFileHeader(existingAddr, &header);
    jsfEraseFile(existingAddr+(uint32_t)sizeof(JsfFileHeader), &header);
  }
  bool compacted = false;
  while (addr+size+(uint32_t)sizeof(JsfFileHeader)>=JSF_END_ADDRESS) {
    if (startAddr == JSF_START_ADDRESS && !compacted) {
      addr = jsfCompact();
      if (!addr) {
        DBG("Compact failed");
        return 0;
      }
      compacted = true;
    } else
      return 0; // no space... defrag?
  }
  // write out the header
  DBG("CreateFile new 0x%08x\n", addr);
  header.size = size;
  header.name = name;
  header.replacement = 0xFFFFFFFF;
  jshFlashWrite(&header,addr,(uint32_t)sizeof(JsfFileHeader));
  if (returnedHeader) *returnedHeader = header;
  return addr+(uint32_t)sizeof(JsfFileHeader);
}

/// Find a 'file' in the memory store. Return the address of data start (and header if returnedHeader!=0). Returns 0 if not found
uint32_t jsfFindFile(JsfFileName name, JsfFileHeader *returnedHeader) {
  uint32_t addr = JSF_START_ADDRESS;
  JsfFileHeader header;
  while (jsfGetFileHeader(addr, &header) && addr+sizeof(JsfFileHeader)<JSF_END_ADDRESS) {
    // check to see if this header has been allocated or not
    if (header.size == 0xFFFFFFFF)
      break; // TODO: page boundaries
    // check for something with the same name that hasn't been replaced
    if (header.replacement == 0xFFFFFFFF &&
        header.name == name) {
      uint32_t endOfFile = addr + (uint32_t)sizeof(JsfFileHeader) + header.size;
      if (endOfFile<addr || endOfFile>JSF_END_ADDRESS)
        return 0; // corrupt - file too long
      if (returnedHeader)
        *returnedHeader = header;
      return addr+(uint32_t)sizeof(JsfFileHeader);
    }
    addr = jsfGetAddressOfNextHeader(addr, &header);
    if (!addr) return 0; // corrupt!
  }
  return 0;
}

/// Output debug info
void jsfDebugFiles() {
  uint32_t addr = JSF_START_ADDRESS;
  uint32_t pageAddr = 0, pageLen = 0, pageEndAddr = 0;

  JsfFileHeader header;
  while (jsfGetFileHeader(addr, &header) && addr+sizeof(JsfFileHeader)<JSF_END_ADDRESS) {
    if (addr>=pageEndAddr) {
      if (!jshFlashGetPage(addr, &pageAddr, &pageLen)) {
        jsiConsolePrintf("Page not found!\n");
        return;
      }
      pageEndAddr = pageAddr+pageLen;
      jsiConsolePrintf("PAGE 0x%08x (%d bytes) - %d live %d free\n", pageAddr,pageLen,jsfGetAllocatedSpaceInPage(pageAddr),jsfGetFreeSpaceInPage(pageAddr));
    }

    // check to see if this file has been allocated or not
    if (header.size == 0xFFFFFFFF)
      break;
    char nameBuf[sizeof(JsfFileName)+1];
    memset(nameBuf,0,sizeof(nameBuf));
    memcpy(nameBuf,&header.name,sizeof(JsfFileName));
    jsiConsolePrintf("0x%08x\t%s\t(%d bytes)\t%s\n", addr, nameBuf, header.size, (header.replacement == 0xFFFFFFFF)?"":" DELETED");
    // TODO: print page boundaries
    addr = jsfGetAddressOfNextHeader(addr, &header);
    if (!addr) {
      jsiConsolePrintf("Corrupt data\n");
      return; // corrupt!
    }
  }
}


/*JSON{
  "type" : "staticmethod",
  "ifndef" : "SAVE_ON_FLASH",
  "class" : "Flash",
  "name" : "eraseFiles",
  "generate" : "jswrap_flash_eraseFiles"
}
 */
void jswrap_flash_eraseFiles() {
  jsfEraseAll();
}

/*JSON{
  "type" : "staticmethod",
  "ifndef" : "SAVE_ON_FLASH",
  "class" : "Flash",
  "name" : "getFile",
  "generate" : "jswrap_flash_getFile",
  "params" : [
    ["name","JsVar","The filename - max 8 characters"]
  ],
  "return" : ["JsVar","A string of data"]
}
 */
JsVar *jswrap_flash_getFile(JsVar *name) {
  char nameBuf[sizeof(JsfFileName)+1];
  memset(nameBuf,0,sizeof(nameBuf));
  jsvGetString(name, nameBuf, sizeof(nameBuf));
  JsfFileHeader header;
  uint32_t addr = jsfFindFile(*(JsfFileName*)nameBuf, &header);
  if (!addr) return 0;
#ifdef LINUX
  // linux fakes flash with a file, so we can't just return a pointer to it!
  char *d = (char*)malloc(header.size);
  jshFlashRead(d, addr, header.size);
  JsVar *v = jsvNewStringOfLength(header.size, d);
  free(d);
  return v;
#else
  return jsvNewNativeString((char*)addr, header.size);
#endif
}

/*JSON{
  "type" : "staticmethod",
  "ifndef" : "SAVE_ON_FLASH",
  "class" : "Flash",
  "name" : "writeFile",
  "generate" : "jswrap_flash_writeFile",
  "params" : [
    ["name","JsVar","The filename - max 8 characters"],
    ["data","JsVar","The data to write"],
    ["offset","int","The offset within the file to write"],
    ["size","int","The size of the file (if a file is to be created that is bigger than the data)"]
  ],
  "return" : ["bool","True on success, false on failure"]
}
Write/create a file.
*/
bool jswrap_flash_writeFile(JsVar *name, JsVar *data, JsVarInt offset, JsVarInt size) {
  // Filename
  char nameBuf[sizeof(JsfFileName)+1];
  memset(nameBuf,0,sizeof(nameBuf));
  jsvGetString(name, nameBuf, sizeof(nameBuf));
  if (offset<0 || size<0) return false;
  // Data length
  JSV_GET_AS_CHAR_ARRAY(dPtr, dLen, data);
  if (!dPtr || !dLen) return false;
  if (size==0) size=(JsVarInt)dLen;
  // Lookup file
  JsfFileHeader header;
  uint32_t addr = jsfFindFile(*(JsfFileName*)nameBuf, &header);
  if ((!addr && offset==0) || // No file
      // we have a file, but it's wrong - remove it
      (addr && offset==0 && (size!=header.size || !jsfIsErased(addr, size)))) {
    if (addr && offset==0 && size==header.size && jsfIsEqual(addr, dPtr, dLen)) {
      DBG("Equal\n");
      return true;
    }
    addr = jsfCreateFile(*(JsfFileName*)nameBuf, (uint32_t)size, JSF_START_ADDRESS, &header);
  }
  if (!addr) {
    jsExceptionHere(JSET_ERROR, "Unable to find or create file");
    return false;
  }
  if ((uint32_t)offset+(uint32_t)dLen > header.size) {
    jsExceptionHere(JSET_ERROR, "Too much data for file size");
    return false;
  }
  addr += (uint32_t)offset;
  if (!jsfIsErased(addr, (uint32_t)dLen)) {
    jsExceptionHere(JSET_ERROR, "File already written with different data");
    return false;
  }
  jshFlashWrite(dPtr, addr, (uint32_t)dLen);
  return true;
}


/*JSON{
  "type" : "staticmethod",
  "ifdef" : "DEBUG",
  "class" : "Flash",
  "name" : "debugFiles",
  "generate" : "jswrap_flash_debugFiles"
}
 */
void jswrap_flash_debugFiles() {
  jsfDebugFiles();
}
