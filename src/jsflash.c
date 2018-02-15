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
 * JavaScript Flash IO functions
 * ----------------------------------------------------------------------------
 */
#include "jsflash.h"
#include "jshardware.h"
#include "jsvariterator.h"
#include "jsinteractive.h"

#define SAVED_CODE_BOOTCODE_RESET "bootrst" // bootcode that runs even after reset
#define SAVED_CODE_BOOTCODE "bootcode" // bootcode that doesn't run after reset
#define SAVED_CODE_VARIMAGE "varimage"


#ifndef FLASH_64BITS_ALIGNMENT
#define FLASH_UNITARY_WRITE_SIZE 4
#else
#define FLASH_UNITARY_WRITE_SIZE 8
#endif

#ifdef DEBUG
#define DBG(...) jsiConsolePrintf("[Flash] "__VA_ARGS__)
#else
#define DBG(...)
#endif

#define JSF_ALIGNMENT FLASH_UNITARY_WRITE_SIZE
#define JSF_START_ADDRESS FLASH_SAVED_CODE_START
#define JSF_END_ADDRESS (FLASH_SAVED_CODE_START+FLASH_SAVED_CODE_LENGTH)


#ifdef USE_HEATSHRINK
  #include "compress_heatshrink.h"
  #define COMPRESS heatshrink_encode
  #define DECOMPRESS heatshrink_decode
#else
  #include "compress_rle.h"
  #define COMPRESS rle_encode
  #define DECOMPRESS rle_decode
#endif

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------ Flash Storage Functionality
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------

/// Aligns a block, pushing it along in memory until it reaches the required alignment
static uint32_t jsfAlignAddress(uint32_t addr) {
  return (addr + (JSF_ALIGNMENT-1)) & (uint32_t)~(JSF_ALIGNMENT-1);
}

JsfFileName jsfNameFromString(const char *name) {
  assert(strlen(name)<=8);
  char nameBuf[sizeof(JsfFileName)+1];
  memset(nameBuf,0,sizeof(nameBuf));
  strcpy(nameBuf,name);
  return *(JsfFileName*)nameBuf;
}

JsfFileName jsfNameFromVar(JsVar *name) {
  char nameBuf[sizeof(JsfFileName)+1];
  memset(nameBuf,0,sizeof(nameBuf));
  jsvGetString(name, nameBuf, sizeof(nameBuf));
  return *(JsfFileName*)nameBuf;
}

static bool jsfGetFileHeader(uint32_t addr, JsfFileHeader *header) {
  assert(header);
  if (!addr) return false;
  jshFlashRead(header, addr, sizeof(JsfFileHeader));
  return header->size != 0xFFFFFFFF;
}

static bool jsfIsErased(uint32_t addr, uint32_t len) {
  uint32_t x;
  /* Read whole blocks at the alignment size and check
   * everything (even slightly past the length) */
  unsigned char buf[JSF_ALIGNMENT];
  for (x=0;x<len;x+=JSF_ALIGNMENT) {
    jshFlashRead(&buf, addr+x, JSF_ALIGNMENT);
    int i;
    for (i=0;i<JSF_ALIGNMENT;i++)
      if (buf[i]!=0xFF) return false;
  }
  return true;
}

static bool jsfIsEqual(uint32_t addr, const unsigned char *data, uint32_t len) {
  uint32_t x, buflen;
  unsigned char buf[JSF_ALIGNMENT];
  for (x=0;x<len;x+=JSF_ALIGNMENT) {
    jshFlashRead(&buf, addr+x,JSF_ALIGNMENT);

    buflen = (x<=len-JSF_ALIGNMENT) ? JSF_ALIGNMENT : (len-x);
    if (memcmp(buf, &data[x], buflen)) return false;
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
static void jsfEraseFileInternal(uint32_t addr, JsfFileHeader *header) {
  DBG("EraseFile 0x%08x\n", addr);

  addr -= (uint32_t)sizeof(JsfFileHeader);
  addr += (uint32_t)((char*)&header->replacement - (char*)header);
  header->replacement = 0;
  jshFlashWrite(&header->replacement,addr,(uint32_t)sizeof(header->replacement));
}

void jsfEraseFile(JsfFileName name) {
  JsfFileHeader header;
  uint32_t addr = jsfFindFile(name, &header);
  if (!addr) return;
  jsfEraseFileInternal(addr, &header);
}

// if we find a page with a header in, return that - or 0
static uint32_t jsfGetAddressOfNextPage(uint32_t addr, JsfFileHeader *header) {
  while(1) {
    uint32_t pageAddr,pageLen;
    if (!jshFlashGetPage(addr, &pageAddr, &pageLen))
      return 0;
    addr = pageAddr+pageLen;
    if (addr+sizeof(JsfFileHeader)>JSF_END_ADDRESS) return 0; // no pages in range
    if (jsfGetFileHeader(addr, header)) return addr;
  }
  return 0;
}

// Given the address and a header, work out where the next one should be
static uint32_t jsfGetAddressOfNextHeader(uint32_t addr, JsfFileHeader *header) {
  // Work out roughly where the start is
  uint32_t newAddr = addr + header->size + (uint32_t)sizeof(JsfFileHeader);
  // pad out to flash write boundaries
  newAddr = jsfAlignAddress(newAddr);
  // sanity check for bad data
  if (newAddr<addr) return 0; // corrupt!
  if (newAddr+sizeof(JsfFileHeader)>JSF_END_ADDRESS) return 0; // not enough space
  return newAddr;
}

// Get the amount of space free in this page
uint32_t jsfGetFreeSpace(uint32_t addr, bool allPages) {
  uint32_t pageEndAddr = JSF_END_ADDRESS;
  if (!allPages) {
    uint32_t pageAddr,pageLen;
    if (!jshFlashGetPage(addr, &pageAddr, &pageLen))
      return 0;
    assert(addr==pageAddr);
    pageEndAddr = pageAddr+pageLen;
  }
  JsfFileHeader header;
  while (jsfGetFileHeader(addr, &header) && addr+(uint32_t)sizeof(JsfFileHeader)<pageEndAddr) {
    addr = jsfGetAddressOfNextHeader(addr, &header);
  }
  return pageEndAddr-addr;
}

// Get the amount of space needed to mirror this page elsewhere (including padding for alignment)
uint32_t jsfGetAllocatedSpace(uint32_t addr, bool allPages) {
  uint32_t allocated = 0;
  uint32_t pageEndAddr = JSF_END_ADDRESS;
  if (!allPages) {
    uint32_t pageAddr,pageLen;
    if (!jshFlashGetPage(addr, &pageAddr, &pageLen))
      return 0;
    assert(addr==pageAddr);
    pageEndAddr = pageAddr+pageLen;
  }
  JsfFileHeader header;
  while ((jsfGetFileHeader(addr, &header) || (addr=jsfGetAddressOfNextPage(addr, &header))) &&
         (addr+(uint32_t)sizeof(JsfFileHeader)<pageEndAddr)) {
    if (header.replacement == 0xFFFFFFFF) // if not replaced
      allocated += jsfAlignAddress(header.size) + (uint32_t)sizeof(JsfFileHeader);
    addr = jsfGetAddressOfNextHeader(addr, &header);
  }
  return allocated;
}

// Try and compact saved data so it'll fit in Flash again
bool jsfCompact() {
  DBG("Compacting\n");
  uint32_t allocated = jsfGetAllocatedSpace(JSF_START_ADDRESS, true);
  if (allocated+1024 < jsuGetFreeStack()) {
    DBG("Compacting - all data fits in RAM\n");
    // All our allocated data will fit in RAM - awesome!
    // Allocate RAM
    char *swapBuffer = alloca(allocated);
    char *swapBufferPtr = swapBuffer;
    // Copy all data to swap
    DBG("Compacting - copy data to swap\n");
    JsfFileHeader header;
    uint32_t addr = JSF_START_ADDRESS;
    while ((jsfGetFileHeader(addr, &header) || (addr=jsfGetAddressOfNextPage(addr, &header))) &&
           (addr+(uint32_t)sizeof(JsfFileHeader)<JSF_END_ADDRESS)) {
      if (header.replacement == 0xFFFFFFFF) { // if not replaced
        memcpy(swapBufferPtr, &header, sizeof(JsfFileHeader));
        swapBufferPtr += sizeof(JsfFileHeader);
        uint32_t alignedSize = jsfAlignAddress(header.size);
        jshFlashRead(swapBufferPtr, addr+(uint32_t)sizeof(JsfFileHeader), alignedSize);
        memset(&swapBufferPtr[header.size], 0xFF, alignedSize-header.size);
        swapBufferPtr += alignedSize;
      }
      addr = jsfGetAddressOfNextHeader(addr, &header);
    }
    // Erase everything
    jsfEraseAll();
    // Copy data back
    DBG("Compacting - copy data from swap\n");
    char *swapBufferEnd = swapBufferPtr;
    swapBufferPtr = swapBuffer;
    while (swapBufferPtr < swapBufferEnd) {
      memcpy(&header, swapBufferPtr, sizeof(JsfFileHeader));
      swapBufferPtr += sizeof(JsfFileHeader);
      JsfFileHeader newHeader;
      uint32_t newFile = jsfCreateFile(header.name, header.size, JSF_START_ADDRESS, &newHeader);
      if (newFile) jshFlashWrite(swapBufferPtr, newFile, jsfAlignAddress(header.size));
      swapBufferPtr += jsfAlignAddress(header.size);
    }
    DBG("Compaction Complete\n");
    return true;
  }
  DBG("Uh-oh. Data doesn't fit in RAM\n");

  /*if (!jshFlashGetPage(pageAddr, &pageAddr, &pageLen))
    return 0;
  uint32_t pageAllocated = jsfGetAllocatedSpace(pageAddr, false);
  uint32_t newPageAddr,newPageLen;
  newPageAddr = pageAddr+pageLen;
  if (!jshFlashGetPage(newPageAddr, &newPageAddr, &newPageLen))
    return 0;
  uint32_t newPageFree = jsfGetFreeSpace(newPageAddr);
  if (newPageFree > pageAllocated) {
    // more free space in the new page than is allocated in the old one
    // move any allocated data into the new page
    uint32_t addr = pageAddr;
    JsfFileHeader header;
    while (jsfGetFileHeader(addr, &header) && addr+sizeof(JsfFileHeader)<JSF_END_ADDRESS) {
      if (header.replacement == 0xFFFFFFFF) {
        uint32_t oldFile = addr+sizeof(JsfFileHeader);
        DBG("Moving file at 0x%08x\n", oldFile);
        JsfFileHeader newHeader;
        uint32_t newFile = jsfCreateFile(header.name, header.size, newPageAddr, &newHeader);
        if (!newFile) {
          DBG("Creating new file failed!\n");
          return 0;
        }
        uint32_t x;
        for (x=0;x<header.size;x+=JSF_ALIGNMENT) {
          char buf[JSF_ALIGNMENT];
          jshFlashRead(buf, oldFile+x,JSF_ALIGNMENT);
          jshFlashWrite(buf, newFile+x,JSF_ALIGNMENT);
        }
        jsfEraseFileInternal(oldFile, &header);
      }
      addr = jsfGetAddressOfNextHeader(addr, &header);
      if (!addr) return 0; // corrupt!
    }
    DBG("Erase original page\n");
    jshFlashErasePage(pageAddr);
  }*/
  return false;
}

/// Create a new 'file' in the memory store. Return the address of data start, or 0 on error
uint32_t jsfCreateFile(JsfFileName name, uint32_t size, uint32_t startAddr, JsfFileHeader *returnedHeader) {
  DBG("CreateFile\n");
  assert(startAddr);
  uint32_t addr = startAddr;
  uint32_t existingAddr = 0;
  JsfFileHeader header;
  while (jsfGetFileHeader(addr, &header) && addr+sizeof(JsfFileHeader)<JSF_END_ADDRESS) {
    // check for something with the same name
    if (header.replacement == 0xFFFFFFFF &&
        header.name == name)
      existingAddr = addr;
    addr = jsfGetAddressOfNextHeader(addr, &header);
  }
  // do we have an existing file? Erase it.
  if (existingAddr) {
    jsfGetFileHeader(existingAddr, &header);
    jsfEraseFileInternal(existingAddr+(uint32_t)sizeof(JsfFileHeader), &header);
  }

  bool compacted = false;
  while (addr+size+(uint32_t)sizeof(JsfFileHeader)>=JSF_END_ADDRESS) {
    if (!compacted &&
        startAddr == JSF_START_ADDRESS) {
      if (!jsfCompact()) {
        DBG("CreateFile - Compact failed");
        return 0;
      }
      // now we're compacted, try and find the end again
      addr = JSF_START_ADDRESS;
      while (jsfGetFileHeader(addr, &header) && addr+sizeof(JsfFileHeader)<JSF_END_ADDRESS) {
        addr = jsfGetAddressOfNextHeader(addr, &header);
      }
    } else {
      DBG("CreateFile - Not enough space");
      return 0;
    }
  }
  if (!addr) {
    jsExceptionHere(JSET_ERROR,"Corrupted flash storage blocks. Try require('Storage').eraseAll()");
    return 0;
  }
  // write out the header
  DBG("CreateFile new 0x%08x\n", addr+(uint32_t)sizeof(JsfFileHeader));
  header.size = size;
  header.name = name;
  header.replacement = 0xFFFFFFFF;
  DBG("CreateFile write header\n");
  jshFlashWrite(&header,addr,(uint32_t)sizeof(JsfFileHeader));
  DBG("CreateFile written header\n");
  if (returnedHeader) *returnedHeader = header;
  return addr+(uint32_t)sizeof(JsfFileHeader);
}

/// Find a 'file' in the memory store. Return the address of data start (and header if returnedHeader!=0). Returns 0 if not found
uint32_t jsfFindFile(JsfFileName name, JsfFileHeader *returnedHeader) {
  uint32_t addr = JSF_START_ADDRESS;
  JsfFileHeader header;
  while ((jsfGetFileHeader(addr, &header) || (addr=jsfGetAddressOfNextPage(addr, &header))) &&
          addr+sizeof(JsfFileHeader)<JSF_END_ADDRESS) {
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

/// Output debug info for files stored in flash storage
void jsfDebugFiles() {
  uint32_t addr = JSF_START_ADDRESS;
  uint32_t pageAddr = 0, pageLen = 0, pageEndAddr = 0;

  jsiConsolePrintf("DEBUG FILES %d live\n", jsfGetAllocatedSpace(addr,true));

  JsfFileHeader header;
  while ((jsfGetFileHeader(addr, &header) || (addr=jsfGetAddressOfNextPage(addr, &header))) &&
          addr+sizeof(JsfFileHeader)<JSF_END_ADDRESS) {
    if (addr>=pageEndAddr) {
      if (!jshFlashGetPage(addr, &pageAddr, &pageLen)) {
        jsiConsolePrintf("Page not found!\n");
        return;
      }
      pageEndAddr = pageAddr+pageLen;
      jsiConsolePrintf("PAGE 0x%08x (%d bytes) - %d live %d free\n",
          pageAddr,pageLen,jsfGetAllocatedSpace(pageAddr,false),jsfGetFreeSpace(pageAddr,false));
    }

    char nameBuf[sizeof(JsfFileName)+1];
    memset(nameBuf,0,sizeof(nameBuf));
    memcpy(nameBuf,&header.name,sizeof(JsfFileName));
    jsiConsolePrintf("0x%08x\t%s\t(%d bytes)\t%s\n", addr+(uint32_t)sizeof(JsfFileHeader), nameBuf, header.size, (header.replacement == 0xFFFFFFFF)?"":" DELETED");
    // TODO: print page boundaries
    addr = jsfGetAddressOfNextHeader(addr, &header);
    if (!addr) {
      jsiConsolePrintf("Corrupt data\n");
      return; // corrupt!
    }
  }
}

JsVar *jsfReadFile(JsfFileName name) {
  JsfFileHeader header;
  uint32_t addr = jsfFindFile(name, &header);
  if (!addr) return 0;
#ifdef LINUX
  // linux fakes flash with a file, so we can't just return a pointer to it!
  uint32_t alignedSize = jsfAlignAddress(header.size);
  char *d = (char*)malloc(alignedSize);
  jshFlashRead(d, addr, alignedSize);
  JsVar *v = jsvNewStringOfLength(header.size, d);
  free(d);
  return v;
#else
  size_t mappedAddr = jshFlashGetMemMapAddress((size_t)addr);
  return jsvNewNativeString((char*)mappedAddr, header.size);
#endif
}

bool jsfWriteFile(JsfFileName name, JsVar *data, JsVarInt offset, JsVarInt _size) {
  if (offset<0 || _size<0) return false;
  uint32_t size = (uint32_t)_size;
  // Data length
  JSV_GET_AS_CHAR_ARRAY(dPtr, dLen, data);
  if (!dPtr || !dLen) return false;
  if (size==0) size=(uint32_t)dLen;
  // Lookup file
  JsfFileHeader header;
  uint32_t addr = jsfFindFile(name, &header);
  if ((!addr && offset==0) || // No file
      // we have a file, but it's wrong - remove it
      (addr && offset==0 && (size!=header.size || !jsfIsErased(addr, size)))) {
    if (addr && offset==0 && size==header.size && jsfIsEqual(addr, (unsigned char*)dPtr, (uint32_t)dLen)) {
      DBG("Equal\n");
      return true;
    }
    addr = jsfCreateFile(name, (uint32_t)size, JSF_START_ADDRESS, &header);
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
  DBG("jsfWriteFile writing contents 1\n");
  // Cope with unaligned first write
  uint32_t alignOffset = addr & (JSF_ALIGNMENT-1);
  if (alignOffset) {
    char buf[JSF_ALIGNMENT];
    jshFlashRead(buf, addr-alignOffset, JSF_ALIGNMENT);
    uint32_t alignRemainder = JSF_ALIGNMENT-alignOffset;
    if (alignRemainder > dLen)
      alignRemainder = (uint32_t)dLen;
    memcpy(&buf[alignOffset], dPtr, alignRemainder);
    dPtr += alignRemainder;
    jshFlashWrite(buf, addr-alignOffset, JSF_ALIGNMENT);
    addr += alignRemainder;
    if (alignRemainder >= dLen)
      return true; // we're done!
    dLen -= alignRemainder;
  }
  // Do aligned write
  DBG("jsfWriteFile writing contents 2\n");
  alignOffset = dLen & (JSF_ALIGNMENT-1);
  dLen -= alignOffset;
  if (dLen)
    jshFlashWrite(dPtr, addr, (uint32_t)dLen);
  addr += (uint32_t)dLen;
  dPtr += dLen;
  // Do final unaligned write
  DBG("jsfWriteFile writing contents 3\n");
  if (alignOffset) {
    char buf[JSF_ALIGNMENT];
    jshFlashRead(buf, addr, JSF_ALIGNMENT);
    memcpy(buf, dPtr, alignOffset);
    jshFlashWrite(buf, addr, JSF_ALIGNMENT);
  }
  DBG("jsfWriteFile writing contents done\n");
  return true;
}

/// Return all files in flash as a JsVar array of names
JsVar *jsfListFiles() {
  JsVar *files = jsvNewEmptyArray();
  if (!files) return 0;

  char nameBuf[sizeof(JsfFileName)+1];
  uint32_t addr = JSF_START_ADDRESS;
  uint32_t pageEndAddr = JSF_END_ADDRESS;
  JsfFileHeader header;
  while ((jsfGetFileHeader(addr, &header) || (addr=jsfGetAddressOfNextPage(addr, &header))) &&
         (addr+(uint32_t)sizeof(JsfFileHeader)<pageEndAddr)) {
    if (header.replacement == 0xFFFFFFFF) { // if not replaced
      memcpy(nameBuf, &header.name, sizeof(JsfFileName));
      nameBuf[sizeof(JsfFileName)]=0;
      jsvArrayPushAndUnLock(files, jsvNewFromString(nameBuf));
    }
    addr = jsfGetAddressOfNextHeader(addr, &header);
  }
  return files;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------ For loading/saving code to flash
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------

// cbdata = uint32_t
void jsfSaveToFlash_countcb(unsigned char ch, uint32_t *cbdata) {
  NOT_USED(ch);
  cbdata[0]++;
}

typedef struct {
  uint32_t address;
  uint32_t endAddress;
  uint32_t word; // word of data to write
#ifdef FLASH_64BITS_ALIGNMENT
  uint32_t dataToWrite[2];
#endif
} jsfcbData;
// cbdata = struct jsfcbData
void jsfSaveToFlash_writecb(unsigned char ch, uint32_t *cbdata) {
  jsfcbData *data = (jsfcbData*)cbdata;
  // write only a word at a time
  data->word = (uint32_t)(ch<<24) | (data->word>>8);
#ifndef FLASH_64BITS_ALIGNMENT
  if ((data->address&3)==3)
    jshFlashWrite(&data->word, data->address&(uint32_t)~3, 4);
#else
  // We want the flash writes to be done every 64 bits.
  // Store the first 32 bits and write on next 32 bits word.
  if ((data->address&7)==7){
    data->dataToWrite[1] = data->word;
    jshFlashWrite(data->dataToWrite, data->address&(uint32_t)~7, 8);
  } else if ((data->address&3)==3){
    data->dataToWrite[0] = data->word;
  }
#endif
  // inc address ptr
  data->address++;
  // output status characters
  if ((data->address&1023)==0) jsiConsolePrint(".");
}

// cbdata = struct jsfcbData
int jsfLoadFromFlash_readcb(uint32_t *cbdata) {
  jsfcbData *data = (jsfcbData*)cbdata;

  if (data->address >= data->endAddress) return -1; // at end
  unsigned char d;
  jshFlashRead(&d, data->address++, 1);
  return d;
}

/// Save the RAM image to flash (this is the actual interpreter state)
void jsfSaveToFlash() {
  unsigned int varSize = jsvGetMemoryTotal() * (unsigned int)sizeof(JsVar);
  unsigned char* varPtr = (unsigned char *)_jsvGetAddressOf(1);

  jsiConsolePrint("Compacting Flash...\n");
  // Ensure we get rid of any saved code we had before
  jsfEraseFile(jsfNameFromString(SAVED_CODE_VARIMAGE));
  // Try and compact, just to ensure we get the maximum amount saved
  jsfCompact();
  jsiConsolePrint("Calculating Size...\n");
  // Work out how much data this'll take
  uint32_t compressedSize = 0;
  COMPRESS(varPtr, varSize, jsfSaveToFlash_countcb, &compressedSize);
  // How much data do we have?
  uint32_t savedCodeAddr = jsfCreateFile(jsfNameFromString(SAVED_CODE_VARIMAGE), compressedSize, JSF_START_ADDRESS, 0);
  if (!savedCodeAddr) {
    jsiConsolePrintf("ERROR: Too big to save to flash (%d vs %d bytes)\n", compressedSize, jsfGetFreeSpace(JSF_START_ADDRESS,true));
    jsvSoftInit();
    jspSoftInit();
    jsiConsolePrint("Deleting command history and trying again...\n");
    while (jsiFreeMoreMemory());
    jspSoftKill();
    jsvSoftKill();
    savedCodeAddr = jsfCreateFile(jsfNameFromString(SAVED_CODE_VARIMAGE), compressedSize, JSF_START_ADDRESS, 0);
  }
  if (!savedCodeAddr) {
    jsiConsolePrint("Not enough free space to save.\n");
    return;
  }
  // Ok, we have space!
  // Now start writing
  jsfcbData cbData;
  cbData.address = savedCodeAddr;
  cbData.endAddress = jsfAlignAddress(savedCodeAddr+compressedSize);
  jsiConsolePrint("Writing..");
  COMPRESS(varPtr, varSize, jsfSaveToFlash_writecb, (uint32_t*)&cbData);
  // make sure we write everything in the buffer out
  int i;
  for(i=0;i<FLASH_UNITARY_WRITE_SIZE;i++)
    jsfSaveToFlash_writecb(0,(uint32_t*)&cbData);

  jsiConsolePrintf("\nCompressed %d bytes to %d\n", varSize, compressedSize);
}


/// Load the RAM image from flash (this is the actual interpreter state)
void jsfLoadStateFromFlash() {
  JsfFileHeader header;
  uint32_t savedCode = jsfFindFile(jsfNameFromString(SAVED_CODE_VARIMAGE),&header);
  if (!savedCode) {
    return;
  }

  //  unsigned int dataSize = jsvGetMemoryTotal() * sizeof(JsVar);
  unsigned char* varPtr = (unsigned char *)_jsvGetAddressOf(1);

  jsfcbData cbData;
  cbData.address = savedCode;
  cbData.endAddress = savedCode+header.size;
  jsiConsolePrintf("Loading %d bytes from flash...\n", header.size);
  DECOMPRESS(jsfLoadFromFlash_readcb, (uint32_t*)&cbData, varPtr);
}

void jsfSaveBootCodeToFlash(JsVar *code, bool runAfterReset) {
  jsfEraseFile(jsfNameFromString(SAVED_CODE_BOOTCODE));
  jsfEraseFile(jsfNameFromString(SAVED_CODE_BOOTCODE_RESET));
  if (jsvIsUndefined(code) || jsvGetLength(code)==0)
    return;
  jsfWriteFile(jsfNameFromString(runAfterReset ? SAVED_CODE_BOOTCODE_RESET : SAVED_CODE_BOOTCODE), code, 0, 0);
}

JsVar *jsfGetBootCodeFromFlash(bool isReset) {
  JsVar *resetCode = jsfReadFile(jsfNameFromString(SAVED_CODE_BOOTCODE_RESET));
  if (isReset || resetCode) return resetCode;
  return jsfReadFile(jsfNameFromString(SAVED_CODE_BOOTCODE));
}

bool jsfLoadBootCodeFromFlash(bool isReset) {
  JsVar *code = jsfGetBootCodeFromFlash(isReset);
  if (!code) return false;
  jsvUnLock2(jspEvaluateVar(code,0,0), code);
  return true;
}

bool jsfFlashContainsCode() {
  return
      jsfFindFile(jsfNameFromString(SAVED_CODE_VARIMAGE),0) ||
      jsfFindFile(jsfNameFromString(SAVED_CODE_BOOTCODE),0) ||
      jsfFindFile(jsfNameFromString(SAVED_CODE_BOOTCODE_RESET),0);
}

/** Completely clear any saved code from flash. */
void jsfRemoveCodeFromFlash() {
  jsiConsolePrint("Erasing saved code.");
  jsfEraseFile(jsfNameFromString(SAVED_CODE_VARIMAGE));
  jsfEraseFile(jsfNameFromString(SAVED_CODE_BOOTCODE));
  jsfEraseFile(jsfNameFromString(SAVED_CODE_BOOTCODE_RESET));
  jsiConsolePrint("\nDone!\n");
}

