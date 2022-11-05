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
#include "jswrap_string.h" //jswrap_string_match
#include "jswrap_espruino.h" //jswrap_espruino_CRC

#define SAVED_CODE_BOOTCODE_RESET ".bootrst" // bootcode that runs even after reset
#define SAVED_CODE_BOOTCODE ".bootcde" // bootcode that doesn't run after reset
#ifndef ESPR_NO_VARIMAGE
#define SAVED_CODE_VARIMAGE ".varimg" // Image of all JsVars written to flash
#endif

#define JSF_START_ADDRESS FLASH_SAVED_CODE_START
#define JSF_END_ADDRESS (FLASH_SAVED_CODE_START+FLASH_SAVED_CODE_LENGTH)


#ifdef FLASH_SAVED_CODE2_START // if there's a second bank of flash to use..
#define JSF_BANK2_START_ADDRESS FLASH_SAVED_CODE2_START
#define JSF_BANK2_END_ADDRESS (FLASH_SAVED_CODE2_START+FLASH_SAVED_CODE2_LENGTH)
#define JSF_DEFAULT_START_ADDRESS JSF_BANK2_START_ADDRESS
#define JSF_DEFAULT_END_ADDRESS JSF_BANK2_END_ADDRESS
#else
#define JSF_DEFAULT_START_ADDRESS JSF_START_ADDRESS
#define JSF_DEFAULT_END_ADDRESS JSF_END_ADDRESS
#endif

#ifdef USE_HEATSHRINK
  #include "compress_heatshrink.h"
  #define COMPRESS heatshrink_encode
  #define DECOMPRESS heatshrink_decode
#else
  #include "compress_rle.h"
  #define COMPRESS rle_encode
  #define DECOMPRESS rle_decode
#endif

#define JSF_CACHE_NOT_FOUND 0xFFFFFFFF
#define JSF_MAX_FILES 10000 // 10k files max - we use this for sanity checking our data
#define JSF_FILENAME_TABLE_NAME "[FILENAME_TABLE]"

#ifdef ESPR_STORAGE_FILENAME_TABLE
uint32_t jsfFilenameTableBank1Addr = 0; // address of DATA in the table, NOT THE HEADER (or 0 if no table)
uint32_t jsfFilenameTableBank1Size = 0; // size of table in bytes
#endif

#if ESPR_USE_STORAGE_CACHE
/* Filename lookups can take over 1ms per file even on a reasonably empty SPI Flash memory,
so we can have a cache of the most used file *addresses* in RAM. The data is still in
flash but not having to do the search really helps us.

To use this, add '-DESPR_USE_STORAGE_CACHE=32' or some other number to the BOARD.py file

TODO: we could potentially have some scoring system such that the most used files
like settings.js are never ever taken out of the cache. Right now reading
ESPR_USE_STORAGE_CACHE different files will flush out the cache.
*/
typedef struct {
  uint32_t addr; ///< Address as returned by jsfFindFile
  JsfFileHeader header; ///< The file header
} JsfCacheEntry;

JsfCacheEntry jsfCache[ESPR_USE_STORAGE_CACHE];
uint8_t jsfCacheEntries = 0;

static void jsfCacheClear() {
  jsfCacheEntries = 0;
}
static void jsfCacheClearFile(JsfFileName name) {
  for (int i=0;i<jsfCacheEntries;i++) {
    if (!jsfIsNameEqual(jsfCache[i].header.name, name))
      continue;
    // if found, shift subsequent files forward over this one
    for (;i<jsfCacheEntries-1;i++)
      jsfCache[i] = jsfCache[i+1];
    // reduce amount of entries
    jsfCacheEntries--;
    return;
  }
}

// Find an item in the cache - returns JSF_CACHE_NOT_FOUND on failure as it's handy to know about files that don't exist too
static uint32_t jsfCacheFind(JsfFileName name, JsfFileHeader *returnedHeader) {
  for (int i=0;i<jsfCacheEntries;i++)
    if (jsfIsNameEqual(jsfCache[i].header.name, name)) {
      JsfCacheEntry curr = jsfCache[i];
      if (i) { // if not at front, put to front
        // shift others forward
        for (int j=i-1;j>=0;j--)
          jsfCache[j+1] = jsfCache[j];
        // put at front
        jsfCache[0].header = curr.header;
        jsfCache[0].addr = curr.addr;
      }
      if (returnedHeader)
        *returnedHeader = curr.header;
      return curr.addr;
    }

  return JSF_CACHE_NOT_FOUND;
}
static void jsfCachePut(JsfFileHeader *header, uint32_t addr) {
  // TODO: ListFiles could lazily fill the list with all
  // files it finds at the end...
  if (jsfCacheEntries<ESPR_USE_STORAGE_CACHE)
    jsfCacheEntries++;
  for (int i=jsfCacheEntries-2;i>=0;i--)
    jsfCache[i+1] = jsfCache[i];
  jsfCache[0].header = *header;
  jsfCache[0].addr = addr;
}
#else // no cache, just stub with code that does nothing
static void jsfCacheClear() {}
static void jsfCacheClearFile(JsfFileName name) {}
static uint32_t jsfCacheFind(JsfFileName name, JsfFileHeader *header) { return JSF_CACHE_NOT_FOUND; }
static void jsfCachePut(JsfFileHeader *header, uint32_t addr) { }
#endif

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------ Flash Storage Functionality
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------

static uint32_t jsfCreateFile(JsfFileName name, uint32_t size, JsfFileFlags flags, JsfFileHeader *returnedHeader);
#ifdef ESPR_STORAGE_FILENAME_TABLE
static uint32_t jsfBankCreateFileTable(uint32_t startAddr);
#endif

/// Aligns a block, pushing it along in memory until it reaches the required alignment
static uint32_t jsfAlignAddress(uint32_t addr) {
  return (addr + (JSF_ALIGNMENT-1)) & (uint32_t)~(JSF_ALIGNMENT-1);
}

JsfFileName jsfNameFromString(const char *name) {
  assert(strlen(name)<=sizeof(JsfFileName));
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

JsfFileName jsfNameFromVarAndUnLock(JsVar *name) {
  JsfFileName n = jsfNameFromVar(name);
  jsvUnLock(name);
  return n;
}

JsVar *jsfVarFromName(JsfFileName name) {
  char nameBuf[sizeof(JsfFileName)+1];
  nameBuf[sizeof(JsfFileName)] = 0;
  memcpy(nameBuf, &name, sizeof(JsfFileName));
  return jsvNewFromString(nameBuf);
}

/// Are two filenames equal?
bool jsfIsNameEqual(JsfFileName a, JsfFileName b) {
  return memcmp(a.c, b.c, sizeof(a.c))==0;
}

/// Return the size in bytes of a file based on the header
uint32_t jsfGetFileSize(JsfFileHeader *header) {
  return (uint32_t)(header->size & 0x00FFFFFF);
}

/// Return the flags for this file based on the header
JsfFileFlags jsfGetFileFlags(JsfFileHeader *header) {
  return (JsfFileFlags)((uint32_t)header->size >> 24);
}

/** returns true if this file isn't deleted, or some internal
type of file like FILENAME_TABLE that shouldn't be listed
or kept when compacting */
static bool jsfIsRealFile(JsfFileHeader *header) {
  return (header->name.firstChars != 0) // if not replaced
#ifdef ESPR_STORAGE_FILENAME_TABLE
         && !(jsfGetFileFlags(header) & JSFF_FILENAME_TABLE)
#endif
         ;
}


/// Return the flags for this file based on the header
static uint32_t jsfGetBankEndAddress(uint32_t addr) {
#ifdef JSF_BANK2_START_ADDRESS
  if (addr>=JSF_BANK2_START_ADDRESS && addr<=JSF_BANK2_END_ADDRESS)
    return JSF_BANK2_END_ADDRESS;
#endif
  return JSF_END_ADDRESS;
}

/** Load a file header from flash, return true if it is valid.
 * If readFullName==false, only the first 4 bytes of the name are loaded */
static bool jsfGetFileHeader(uint32_t addr, JsfFileHeader *header, bool readFullName) {
  assert(header);
  if (!addr) return false;
  jshFlashRead(header, addr, readFullName ? sizeof(JsfFileHeader) : 8/* size + name.firstChars */);
  uint32_t endAddress = addr + (uint32_t)sizeof(JsfFileHeader) + jsfGetFileSize(header);
  return (header->size != JSF_WORD_UNSET) && (header->size != 0) &&
         (endAddress <= jsfGetBankEndAddress(addr));
}

/// Is an area of flash completely erased?
static bool jsfIsErased(uint32_t addr, uint32_t len) {
  /* Read whole blocks at the alignment size and check
   * everything (even slightly past the length) */
  unsigned char buf[128];
  assert((sizeof(buf)&(JSF_ALIGNMENT-1))==0);
  int watchdogCtr = 0;
  while (len) {
    uint32_t l = len;
    if (l>sizeof(buf)) l=sizeof(buf);
    jshFlashRead(&buf, addr, l);
    for (uint32_t i=0;i<l;i++)
      if (buf[i]!=0xFF) return false;
    addr += l;
    len -= l;
    if (watchdogCtr++ > 500) {
      // stop watchdog reboots when checking large areas
      // we don't kick all the time so that in *normal* work
      // we don't end up calling jshKickWatchDog, so it's harder
      // to get in a state where things lock up.
      jshKickWatchDog();
      watchdogCtr = 0;
    }
  }
  return true;
}

/// Is an area of flash equal to something that's in RAM?
static bool jsfIsEqual(uint32_t addr, const unsigned char *data, uint32_t len) {
  unsigned char buf[128];
  assert((sizeof(buf)&(JSF_ALIGNMENT-1))==0);
  uint32_t x=0;
  while (len) {
    uint32_t l = len;
    if (l>sizeof(buf)) l=sizeof(buf);
    jshFlashRead(&buf, addr+x, l);
    if (memcmp(buf, &data[x], l)) return false;
    x += l;
    len -= l;
  }
  return true;
}

/// Erase the entire contents of the memory store
bool jsfEraseAll() {
  jsDebug(DBG_INFO,"EraseAll\n");
  jsfCacheClear();
#ifdef ESPR_STORAGE_FILENAME_TABLE
  jsfFilenameTableBank1Addr = 0;
  jsfFilenameTableBank1Size = 0;
#endif
#ifdef JSF_BANK2_START_ADDRESS
  if (!jshFlashErasePages(JSF_BANK2_START_ADDRESS, JSF_BANK2_END_ADDRESS-JSF_BANK2_START_ADDRESS)) return false;
#endif
  return jshFlashErasePages(JSF_START_ADDRESS, JSF_END_ADDRESS-JSF_START_ADDRESS);
}

/// When a file is found in memory, erase it (by setting first bytes of name to 0). addr=ptr to data, NOT header
static void jsfEraseFileInternal(uint32_t addr, JsfFileHeader *header, bool createFilenameTable) {
  jsDebug(DBG_INFO,"EraseFile 0x%08x\n", addr);

  addr -= (uint32_t)sizeof(JsfFileHeader);
  addr += (uint32_t)((char*)&header->name.firstChars - (char*)header);
  header->name.firstChars = 0;
  jshFlashWrite(&header->name.firstChars,addr,(uint32_t)sizeof(header->name.firstChars));

#ifdef ESPR_STORAGE_FILENAME_TABLE
  if (createFilenameTable && addr>=JSF_START_ADDRESS && addr<JSF_END_ADDRESS) { // if was erasing in Bank 1
    // do a scan from the last FILENAME_TABLE to see how many files there are
    uint32_t scanAddr = 0;
    if (jsfFilenameTableBank1Addr)
      scanAddr = jsfFilenameTableBank1Addr + jsfFilenameTableBank1Size;
    JsfStorageStats stats = jsfGetStorageStats(scanAddr, true);
    /* if more than 200 files were added/deleted since the last
    FILENAME_TABLE, try and make a new one. 100 files seems to add
    around 5ms to each Storage.list call, or 2ms to a file read. */
    if ((stats.trashCount+stats.fileCount)>200)
      jsfBankCreateFileTable(JSF_START_ADDRESS);
  }
#endif
}

bool jsfEraseFile(JsfFileName name) {
  JsfFileHeader header;
  uint32_t addr = jsfFindFile(name, &header);
  if (!addr) return false;
  jsfCacheClearFile(name);
  jsfEraseFileInternal(addr, &header, true);
  return true;
}

// Get the address of the page after the current one, or 0. THE NEXT PAGE MAY HAVE A PREVIOUS PAGE'S DATA SPANNING OVER IT
static uint32_t jsfGetAddressOfNextPage(uint32_t addr) {
  uint32_t pageAddr,pageLen;
  if (!jshFlashGetPage(addr, &pageAddr, &pageLen))
    return 0;
  uint32_t endAddr = jsfGetBankEndAddress(addr);
  addr = pageAddr+pageLen;
  if (addr>=endAddr) {
    return 0; // no pages in range
  }
  return addr;
}

/* Get the space left for a file (including header) between the address and the next page.
 * If the next page is empty, return the space in that as well (and so on) */
static uint32_t jsfGetSpaceLeftInPage(uint32_t addr) {
  uint32_t pageAddr,pageLen;
  if (!jshFlashGetPage(addr, &pageAddr, &pageLen))
    return 0;
  uint32_t endAddr = jsfGetBankEndAddress(addr);
  uint32_t nextPageStart = pageAddr+pageLen;
  // if the next page is empty, assume it's empty until the end of flash
  JsfFileHeader header;
  if (nextPageStart<endAddr &&
      !jsfGetFileHeader(nextPageStart, &header, false)) {
    nextPageStart = endAddr;
  }
  return nextPageStart - addr;
}

typedef enum {
  GNFH_GET_EMPTY = 0,    ///< stop on an empty header even if there are pages after
  GNFH_GET_ALL   = 1,      ///< get all headers
  GNFH_READ_ONLY_FILENAME_START = 2 ///< Get size and the first 4 chars of the filename
} jsfGetNextFileHeaderType;

/** Given the address and a header, work out where the next one should be and load it.
 Both addr and header are updated. Returns true if the header is valid, false if not.
 If skipPages==true, if a header isn't valid but there's another page, jump to that.
 */
static bool jsfGetNextFileHeader(uint32_t *addr, JsfFileHeader *header, jsfGetNextFileHeaderType type) {
  assert(addr && header);
  uint32_t oldAddr = *addr;
  *addr = 0;
  // Work out roughly where the start is
  uint32_t newAddr = oldAddr + jsfGetFileSize(header) + (uint32_t)sizeof(JsfFileHeader);
  // pad out to flash write boundaries
  newAddr = jsfAlignAddress(newAddr);
  // sanity check for bad data
  if (newAddr<oldAddr) return 0; // corrupt!
  if (newAddr+sizeof(JsfFileHeader) > jsfGetBankEndAddress(oldAddr)) return 0; // not enough space
  *addr = newAddr;
  bool valid = jsfGetFileHeader(newAddr, header, !(type&GNFH_READ_ONLY_FILENAME_START));
  if ((type&GNFH_GET_ALL) && !valid) {
    // there wasn't another header in this page - check the next page
    newAddr = jsfGetAddressOfNextPage(newAddr);
    *addr = newAddr;
    if (!newAddr) return false; // no valid address
    valid = jsfGetFileHeader(newAddr, header, !(type&GNFH_READ_ONLY_FILENAME_START));
    // we can't have a blank page and then a header, so stop our search
  }
  return valid;
}

// Get the address of the page that starts with a header (or is clear) after the current one, or 0
static uint32_t jsfGetAddressOfNextStartPage(uint32_t addr) {
  uint32_t next = jsfGetAddressOfNextPage(addr);
  if (next==0) return 0; // no next page
  JsfFileHeader header;
  if (jsfGetFileHeader(addr, &header, false)) do {
    if (addr>next) {
      next = jsfGetAddressOfNextPage(addr);
      if (next==0) return 0;
    }
    if (addr==next) return addr; // we stumbled on a header that was right on the boundary
  } while (jsfGetNextFileHeader(&addr, &header, GNFH_GET_EMPTY|GNFH_READ_ONLY_FILENAME_START));
  return next;
}

/// Get info about the current filesystem
JsfStorageStats jsfGetStorageStats(uint32_t addr, bool allPages) {
  if (!addr) addr=JSF_DEFAULT_START_ADDRESS;
  uint32_t startAddr = addr;
  JsfStorageStats stats;
  memset(&stats, 0, sizeof(JsfStorageStats));
  JsfFileHeader header;
  memset(&header,0,sizeof(JsfFileHeader));
  uint32_t lastAddr = addr;
  if (jsfGetFileHeader(addr, &header, false)) do {
    uint32_t fileSize = jsfAlignAddress(jsfGetFileSize(&header)) + (uint32_t)sizeof(JsfFileHeader);
    lastAddr = addr + fileSize;
    if (header.name.firstChars != 0) { // if not replaced
      stats.fileBytes += fileSize;
      stats.fileCount++;
    } else { // replaced
      stats.trashBytes += fileSize;
      stats.trashCount++;
    }
  } while (jsfGetNextFileHeader(&addr, &header, (allPages ? GNFH_GET_ALL : GNFH_GET_EMPTY)|GNFH_READ_ONLY_FILENAME_START));
  uint32_t pageEndAddr = allPages ? jsfGetBankEndAddress(startAddr) : jsfGetAddressOfNextPage(startAddr);
  stats.total = pageEndAddr - startAddr;
  stats.free = pageEndAddr - lastAddr;
  return stats;
}

#ifndef SAVE_ON_FLASH

// Copy one memory buffer to another *circular buffer*
static void memcpy_circular(char *dst, uint32_t *dstIndex, uint32_t dstSize, char *src, size_t len) {
  while (len--) {
    dst[*dstIndex] = *(src++);
    *dstIndex = (*dstIndex+1) % dstSize;
  }
}

static void jsfCompactWriteBuffer(uint32_t *writeAddress, uint32_t readAddress, char *swapBuffer, uint32_t swapBufferSize, uint32_t *swapBufferUsed, uint32_t *swapBufferTail) {
  uint32_t endAddr = jsfGetBankEndAddress(*writeAddress);
  uint32_t nextFlashPage = jsfGetAddressOfNextPage(*writeAddress);
  if (nextFlashPage==0) nextFlashPage=endAddr;
  // write any data between swapBufferTail and the end of the buffer
  while (*swapBufferUsed) {
    uint32_t s = *swapBufferUsed;
    // don't read past end - it's circular
    if (s+*swapBufferTail > swapBufferSize)
      s = swapBufferSize - *swapBufferTail;
    // don't write into a new page
    if (s+*writeAddress > nextFlashPage)
      s = nextFlashPage-*writeAddress;
    if (readAddress < nextFlashPage) {
      jsDebug(DBG_INFO,"compact> skip write (0x%08x) as we're still reading from the page (0x%08x)\n", &writeAddress, readAddress);
      return;
    }
    jsDebug(DBG_INFO,"compact> write %d from buf[%d] => 0x%08x\n", s, *swapBufferTail, *writeAddress);
    // if on a new page, erase it
    uint32_t pAddr, pLen;
    if (jshFlashGetPage(*writeAddress, &pAddr, &pLen) &&  (pAddr == *writeAddress)) {
      jsDebug(DBG_INFO,"compact> erase page 0x%08x\n", *writeAddress);
      jshFlashErasePage(*writeAddress);
    }
    assert(jsfIsErased(*writeAddress, s)); 
    //if (!jsfIsErased(*writeAddress, s)) jsiConsolePrintf("ERROR: AREA NOT ERASED 0x%08x => 0x%08x\n", *writeAddress, *writeAddress + s);
    jsDebug(DBG_INFO,"compact> write 0x%08x => 0x%08x\n", *writeAddress, *writeAddress + s);
    jshFlashWrite(&swapBuffer[*swapBufferTail], *writeAddress, s);
    *writeAddress += s;
    nextFlashPage = jsfGetAddressOfNextPage(*writeAddress);
    if (nextFlashPage==0) nextFlashPage=endAddr;
    *swapBufferTail = (*swapBufferTail+s) % swapBufferSize;
    *swapBufferUsed -= s;
    // ensure we don't reboot here if it takes a long time
    jshKickWatchDog();
  }
}

/* Try and compact saved data so it'll fit in Flash again.
 */
static bool jsfCompactInternal(uint32_t startAddress, char *swapBuffer, uint32_t swapBufferSize) {
  uint32_t writeAddress = startAddress;
  jsDebug(DBG_INFO,"Compacting from 0x%08x (%d byte buffer)\n", startAddress, swapBufferSize);
  uint32_t swapBufferHead = 0;
  uint32_t swapBufferTail = 0;
  uint32_t swapBufferUsed = 0;
  JsfFileHeader header;
  memset(&header,0,sizeof(JsfFileHeader));
  uint32_t addr = startAddress;
  if (jsfGetFileHeader(addr, &header, true)) do {
    if (jsfIsRealFile(&header)) { // if not replaced or system file
      jsDebug(DBG_INFO,"compact> copying file at 0x%08x\n", addr);
      // Rewrite file position for any JsVars that used this file *if* the file changed position
      uint32_t newAddress = writeAddress+swapBufferUsed;
      if (addr != newAddress)
        jsvUpdateMemoryAddress(addr, sizeof(JsfFileHeader) + jsfGetFileSize(&header), newAddress);
      // Copy the file into the circular buffer, one bit at a time.
      // Write the header
      memcpy_circular(swapBuffer, &swapBufferHead, swapBufferSize, (char*)&header, sizeof(JsfFileHeader));
      swapBufferUsed += (uint32_t)sizeof(JsfFileHeader);
      // Write the contents
      uint32_t alignedSize = jsfAlignAddress(jsfGetFileSize(&header));
      uint32_t alignedPtr = addr+(uint32_t)sizeof(JsfFileHeader);
      jsfCompactWriteBuffer(&writeAddress, alignedPtr, swapBuffer, swapBufferSize, &swapBufferUsed, &swapBufferTail);
      while (alignedSize) {
        // How much space do we have available in our swapBuffer
        uint32_t s = swapBufferSize-swapBufferUsed;
        if (s > swapBufferSize-swapBufferHead)
          s = swapBufferSize-swapBufferHead;
        if ((swapBufferTail>swapBufferHead) && (s > (swapBufferTail-swapBufferHead)))
          s = swapBufferTail-swapBufferHead;
        if (s==0) {
          jsDebug(DBG_INFO,"compact> error - no space left!\n");
          return false;
        }
        if (s>alignedSize) s=alignedSize;
        jsDebug(DBG_INFO,"compact> read %d from 0x%08x => buf[%d]\n", s, alignedPtr, swapBufferHead);
        jshFlashRead(&swapBuffer[swapBufferHead], alignedPtr, s);
        alignedSize -= s;
        alignedPtr += s;
        swapBufferUsed += s;
        swapBufferHead = (swapBufferHead+s) % swapBufferSize;
        // Is the buffer big enough to write?
        jsfCompactWriteBuffer(&writeAddress, alignedPtr, swapBuffer, swapBufferSize, &swapBufferUsed, &swapBufferTail);
      }
    }
    // kick watchdog to ensure we don't reboot
    jshKickWatchDog();
  } while (jsfGetNextFileHeader(&addr, &header, GNFH_GET_ALL));
  jsDebug(DBG_INFO,"compact> finished reading...\n");
  // try and write the remaining
  jsfCompactWriteBuffer(&writeAddress, jsfGetBankEndAddress(writeAddress), swapBuffer, swapBufferSize, &swapBufferUsed, &swapBufferTail);
  // Finished - erase remaining
  jsDebug(DBG_INFO,"compact> almost there - erase remaining pages\n");
  if (writeAddress!=startAddress)
    writeAddress = jsfGetAddressOfNextPage(writeAddress-1);
  if (writeAddress) {
    // addr can be zero if last file was right at the end of storage. If so, set to end of storage area
    if (!addr) addr=jsfGetBankEndAddress(writeAddress);
    jsDebug(DBG_INFO,"compact> erase 0x%08x => 0x%08x\n", writeAddress, addr);
    // addr is the address of the last area in flash
    jshFlashErasePages(writeAddress, addr-writeAddress);
  }
  jsDebug(DBG_INFO,"Compaction Complete\n");
  return true;
}
#endif

bool jsfBankCompact(uint32_t startAddress) {
#ifndef SAVE_ON_FLASH
  jsDebug(DBG_INFO,"Compacting\n");
  uint32_t pageAddr,pageSize;
  if (!jshFlashGetPage(startAddress, &pageAddr, &pageSize))
    return 0;
  uint32_t maxRequired = pageSize + (uint32_t)sizeof(JsfFileHeader);
  // TODO: We could skip forward pages if we think they are already fully compacted?

  JsfStorageStats stats = jsfGetStorageStats(startAddress, true);
  if (!stats.trashBytes) {
    jsDebug(DBG_INFO,"Already fully compacted\n");
    return true;
  }
  uint32_t swapBufferSize = stats.fileBytes;
  if (swapBufferSize > maxRequired) swapBufferSize=maxRequired;
  // See if we have enough memory...
  if (swapBufferSize+256 < jsuGetFreeStack()) {
    jsDebug(DBG_INFO,"Enough stack for %d byte buffer\n", swapBufferSize);
    char *swapBuffer = alloca(swapBufferSize);
    return jsfCompactInternal(startAddress, swapBuffer, swapBufferSize);
  } else {
    jsDebug(DBG_INFO,"Not enough stack for (%d bytes)\n", swapBufferSize);
    JsVar *buf = jsvNewFlatStringOfLength(swapBufferSize);
    if (buf) {
      jsDebug(DBG_INFO,"Allocated data in JsVars\n");
      char *swapBuffer = jsvGetFlatStringPointer(buf);
      bool r = jsfCompactInternal(startAddress, swapBuffer, swapBufferSize);
      jsvUnLock(buf);
      return r;
    }
  }
  jsDebug(DBG_INFO,"Not enough memory to compact anything\n");

#else
  /* If low on flash assume we only have a tiny bit of flash. Chances
   * are there'll only be one file so just erasing flash will do it. */
  bool allocated = jsvGetBoolAndUnLock(jsfListFiles(NULL,0,0));
  if (!allocated) {
    jsfEraseAll();
    return true;
  }
#endif
  return false;
}

// Try and compact saved data so it'll fit in Flash again
bool jsfCompact() {
  jsfCacheClear();
#ifdef ESPR_STORAGE_FILENAME_TABLE
  jsfFilenameTableBank1Addr = 0;
  jsfFilenameTableBank1Size = 0;
#endif
  bool compacted = jsfBankCompact(JSF_START_ADDRESS);
#ifdef JSF_BANK2_START_ADDRESS
  compacted |= jsfBankCompact(JSF_BANK2_START_ADDRESS);
#endif
  return compacted;
}
char jsfStripDriveFromName(JsfFileName *name){
#ifndef SAVE_ON_FLASH
  if (name->c[1]==':') { // if a 'drive' is specified like "C:foobar.js"
    char drive = name->c[0];
    memmove(name->c, name->c+2, sizeof(JsfFileName)-2); // shift back and clear the rest
    name->c[sizeof(JsfFileName)-2]=0;name->c[sizeof(JsfFileName)-1]=0;
    return drive;
  }
#ifdef JSF_BANK2_START_ADDRESS
  int l = 0;
  while (name->c[l] && l<sizeof(JsfFileName)) l++;
  if (strcmp(name,".boot0")==0 ||
      (name->c[l-3]=='.' && name->c[l-2]=='j' && name->c[l-1]=='s')) {
    return 'C';
  }
#endif
#endif
  return 0;
}
void jsfGetDriveBankAddress(char drive, uint32_t *bankStartAddr, uint32_t *bankEndAddr){
#ifdef JSF_BANK2_START_ADDRESS
  if (drive){
    if ((drive&(~0x20)) == 'C'){ // make drive case insensitive
      *bankStartAddr=JSF_START_ADDRESS;
      *bankEndAddr=JSF_END_ADDRESS;
    } else {
      *bankStartAddr=JSF_BANK2_START_ADDRESS;
      *bankEndAddr=JSF_BANK2_END_ADDRESS;
    }
    return;
  }
#endif
  *bankStartAddr=JSF_DEFAULT_START_ADDRESS;
  *bankEndAddr=JSF_DEFAULT_END_ADDRESS;
}
/// Create a new 'file' in the memory store - DOES NOT remove existing files with same name. Return the address of data start, or 0 on error
static uint32_t jsfCreateFile(JsfFileName name, uint32_t size, JsfFileFlags flags, JsfFileHeader *returnedHeader) {
  jsDebug(DBG_INFO,"CreateFile (%d bytes)\n", size);
  char drive = jsfStripDriveFromName(&name);
  jsfCacheClearFile(name);
  uint32_t bankStartAddress,bankEndAddress;
  jsfGetDriveBankAddress(drive,&bankStartAddress,&bankEndAddress);
  /* TODO: do we want to start our scan from jsfFilenameTableBank1Addr to
   * make writing files faster? */

  uint32_t requiredSize = jsfAlignAddress(size)+(uint32_t)sizeof(JsfFileHeader);
  bool compacted = false;
  uint32_t addr = 0;
  JsfFileHeader header;
  uint32_t freeAddr = 0;
  while (!freeAddr) {
    addr = bankStartAddress;
    freeAddr = 0;
    // Find a hole that's big enough for our file
    do {
      if (jsfGetFileHeader(addr, &header, false)) do {
      } while (jsfGetNextFileHeader(&addr, &header, GNFH_GET_EMPTY));
      // If not enough space, skip to next page
      if (jsfGetSpaceLeftInPage(addr)<requiredSize) {
        addr = jsfGetAddressOfNextPage(addr);
      } else { // if enough space, we can write a file!
        freeAddr = addr;
      }
    } while (addr && !freeAddr);
    // If we don't have space, compact
    if (!freeAddr) {
      // check this for sanity - in future we might compact forward into other pages, and don't compact if so
      if (!compacted) {
        compacted = true;
        if (!jsfCompact()) {
          jsDebug(DBG_INFO,"CreateFile - Compact failed\n");
          return 0;
        }
        addr = bankStartAddress; // addr->startAddr = restart
      } else {
        // FIXME: if we have 2 banks and there is no room in this one, what about the other bank?
        jsDebug(DBG_INFO,"CreateFile - Not enough space\n");
        return 0;
      }
    }
  };
  /* We used to push files forward to the nearest page boundary but now there's very little point
  doing this. While we still have to cope with it when reading storage, we now don't try and align
  new files - see https://github.com/espruino/Espruino/issues/2232 */
  addr = freeAddr;
  // write out the header
  jsDebug(DBG_INFO,"CreateFile new 0x%08x\n", addr+(uint32_t)sizeof(JsfFileHeader));
  header.size = size | (flags<<24);
  header.name = name;
  jsDebug(DBG_INFO,"CreateFile write header\n");
  jshFlashWrite(&header,addr,(uint32_t)sizeof(JsfFileHeader));
  jsDebug(DBG_INFO,"CreateFile written header\n");
  if (returnedHeader) *returnedHeader = header;
  addr += (uint32_t)sizeof(JsfFileHeader); // address of actual file data
  jsfCachePut(&header, addr);
  return addr;
}

static uint32_t jsfBankFindFile(uint32_t bankAddress, uint32_t bankEndAddress, JsfFileName name, JsfFileHeader *returnedHeader) {
  uint32_t addr = bankAddress;
  JsfFileHeader header;
#ifdef ESPR_STORAGE_FILENAME_TABLE
  if (jsfFilenameTableBank1Addr && addr==JSF_START_ADDRESS) {
    uint32_t baseAddr = addr;
    uint32_t tableAddr = jsfFilenameTableBank1Addr;
    uint32_t tableEnd = tableAddr + jsfFilenameTableBank1Size;
    // Now scan the table and call back for each item
    while (tableAddr < tableEnd) {
      // read the address and name...
      jshFlashRead(&header, tableAddr, sizeof(JsfFileHeader));
      tableAddr += (uint32_t)sizeof(JsfFileHeader);
      if (jsfIsNameEqual(header.name, name)) { // name matches
        uint32_t fileAddr = baseAddr + header.size;
        if (jsfGetFileHeader(fileAddr, &header, true) && // read the real header
            (header.name.firstChars != 0)) { // check the file was not replaced
          if (returnedHeader)
            *returnedHeader = header;
          return fileAddr+(uint32_t)sizeof(JsfFileHeader);
        }
        /* Or... the file was in our table but it's been replaced. In this case
        stop scanning our table and instead just  do a normal scan for files
        added after the table... */
      }
    }
    // We didn't find the file in our table...
    // Now point 'addr' to the start of this table and fill in the header.
    // the normal code will see this, skip over it like a normal file
    // and carry on regardless.
    addr = jsfFilenameTableBank1Addr - sizeof(JsfFileHeader); // address of jsfFilenameTable's header
    header.name.firstChars = 0;
    header.size = jsfFilenameTableBank1Size;
  } else
#endif
  if (!jsfGetFileHeader(addr, &header, false)) return 0;
  // Now search through files in storage
  do {
    // check for something with the same first 4 chars of name that hasn't been replaced.
    if (header.name.firstChars == name.firstChars) {
      // Now load the whole header (with name) and check properly
      if (jsfGetFileHeader(addr, &header, true) && // get file (checks file length is ok)
          jsfIsNameEqual(header.name, name)) {
        if (returnedHeader)
          *returnedHeader = header;
        return addr+(uint32_t)sizeof(JsfFileHeader);
      }
    }
  } while (jsfGetNextFileHeader(&addr, &header, GNFH_GET_ALL|GNFH_READ_ONLY_FILENAME_START)); // still only get first 4 chars of name
  return 0;
}

/// Find a 'file' in the memory store. Return the address of data start (and header if returnedHeader!=0). Returns 0 if not found
uint32_t jsfFindFile(JsfFileName name, JsfFileHeader *returnedHeader) {
  char drive = jsfStripDriveFromName(&name);
  uint32_t a = jsfCacheFind(name, returnedHeader);
  if (a!=JSF_CACHE_NOT_FOUND) return a;
  JsfFileHeader header;

#ifdef JSF_BANK2_START_ADDRESS
  if (drive) {
    // if more banks defined search only in one determined from drive letter
    uint32_t startAddress,endAddress;
    jsfGetDriveBankAddress(drive,&startAddress,&endAddress);
    a = jsfBankFindFile(startAddress, endAddress, name, &header);
  } else {
    // if no drive letter specified, search in both
    a = jsfBankFindFile(JSF_START_ADDRESS, JSF_END_ADDRESS, name, &header);
    if (!a) a = jsfBankFindFile(JSF_BANK2_START_ADDRESS, JSF_BANK2_END_ADDRESS, name, &header);
  }
#else
  a = jsfBankFindFile(JSF_START_ADDRESS, JSF_END_ADDRESS, name, &header);
#endif
  if (!a) header.name = name;
  jsfCachePut(&header, a); // we put the file in even if it's not found, as that's handy too
  if (returnedHeader) *returnedHeader = header;
  return a;
}

static uint32_t jsfBankFindFileFromAddr(uint32_t bankAddress, uint32_t bankEndAddress, uint32_t containsAddr, JsfFileHeader *returnedHeader) {
  uint32_t addr = bankAddress;
  JsfFileHeader header;
  memset(&header,0,sizeof(JsfFileHeader));
  if (jsfGetFileHeader(addr, &header, false)) do {
    uint32_t endOfFile = addr + (uint32_t)sizeof(JsfFileHeader) + jsfGetFileSize(&header);
    if ((header.name.firstChars != 0) && // not been replaced
        (addr<=containsAddr && containsAddr<=endOfFile)) {
      // Now load the whole header (with name) and check properly
      jsfGetFileHeader(addr, &header, true);
      if (returnedHeader)
        *returnedHeader = header;
      return addr+(uint32_t)sizeof(JsfFileHeader);
    }
  } while (jsfGetNextFileHeader(&addr, &header, GNFH_GET_ALL|GNFH_READ_ONLY_FILENAME_START)); // still only get first 4 chars of name
  return 0;
}

uint32_t jsfFindFileFromAddr(uint32_t containsAddr, JsfFileHeader *returnedHeader) {
  if (containsAddr>=JSF_START_ADDRESS && containsAddr<=JSF_END_ADDRESS) {
    uint32_t a = jsfBankFindFileFromAddr(JSF_START_ADDRESS, JSF_END_ADDRESS, containsAddr, returnedHeader);
    if (a) return a;
  }
#ifdef JSF_BANK2_START_ADDRESS
  if (containsAddr>=JSF_BANK2_START_ADDRESS && containsAddr<=JSF_BANK2_END_ADDRESS) {
    uint32_t a = jsfBankFindFileFromAddr(JSF_BANK2_START_ADDRESS, JSF_BANK2_END_ADDRESS, containsAddr, returnedHeader);
    if (a) return a;
  }
#endif
  return 0;
}

static void jsfBankDebugFiles(uint32_t addr) {
  uint32_t pageAddr = 0, pageLen = 0, pageEndAddr = 0;

  JsfStorageStats stats = jsfGetStorageStats(addr,true);
  jsiConsolePrintf("DEBUG FILES (0x%08x)\n  %db (%d files) live\n  %db (%d files) trash\n", addr, stats.fileBytes, stats.fileCount, stats.trashBytes, stats.trashCount);

  JsfFileHeader header;
  memset(&header,0,sizeof(JsfFileHeader));
  if (jsfGetFileHeader(addr, &header, true)) do {
    if (addr>=pageEndAddr) {
      if (!jshFlashGetPage(addr, &pageAddr, &pageLen)) {
        jsiConsolePrintf("Page not found!\n");
        return;
      }
      pageEndAddr = pageAddr+pageLen;
      uint32_t nextStartPage = jsfGetAddressOfNextStartPage(addr);
      JsfStorageStats stats = jsfGetStorageStats(pageAddr,false);
      if (nextStartPage==pageEndAddr) {
        jsiConsolePrintf("PAGE 0x%08x (%d bytes) - %d live %d free\n",
            pageAddr,pageLen,stats.fileBytes,pageLen - stats.fileBytes);
      } else {
        pageLen = nextStartPage-pageAddr;
        jsiConsolePrintf("PAGES 0x%08x -> 0x%08x (%d bytes) - %d live %d free\n",
            pageAddr,nextStartPage,pageLen,
            stats.fileBytes,pageLen-stats.fileBytes);
      }
    }

    char nameBuf[sizeof(JsfFileName)+1];
    memset(nameBuf,0,sizeof(nameBuf));
    memcpy(nameBuf,&header.name,sizeof(JsfFileName));
    jsiConsolePrintf("0x%08x\t%s\t(%d bytes)\n", addr+(uint32_t)sizeof(JsfFileHeader), nameBuf[0]?nameBuf:"DELETED", jsfGetFileSize(&header));
    // TODO: print page boundaries
  } while (jsfGetNextFileHeader(&addr, &header, GNFH_GET_ALL));
}

/// Output debug info for files stored in flash storage
void jsfDebugFiles() {
  jsfBankDebugFiles(JSF_START_ADDRESS);
#ifdef JSF_BANK2_START_ADDRESS
  jsfBankDebugFiles(JSF_BANK2_START_ADDRESS);
#endif
}

static bool jsfIsBankStorageValid(uint32_t startAddr, JsfStorageTestType testFlags) {
  JsfStorageTestType testType = testFlags&JSFSTT_TYPE_MASK;
  uint32_t addr = startAddr;
  uint32_t endAddr = jsfGetBankEndAddress(addr);
  uint32_t oldAddr = addr;
  JsfFileHeader header;
  unsigned char *headerPtr = (unsigned char *)&header;

  bool valid = jsfGetFileHeader(addr, &header, true);
  if (valid) {
    while (jsfGetNextFileHeader(&addr, &header, GNFH_GET_ALL)) {
#ifdef ESPR_STORAGE_FILENAME_TABLE
      if ((testFlags & JSFSTT_FIND_FILENAME_TABLE) &&
          (startAddr==JSF_START_ADDRESS) &&
          (jsfGetFileFlags(&header) & JSFF_FILENAME_TABLE)) {
        jsfGetFileHeader(addr, &header, true); // get all data from header
        if (jsfIsNameEqual(header.name, jsfNameFromString(JSF_FILENAME_TABLE_NAME)) &&
            (jsfGetFileSize(&header) < JSF_MAX_FILES)) {
          // Only set the table if we're sure it's ok (sensible size, correct filename)
          jsfFilenameTableBank1Addr = addr + (uint32_t)sizeof(JsfFileHeader);
          jsfFilenameTableBank1Size  = jsfGetFileSize(&header);
        }
      }
#endif
      oldAddr = addr;
      jshKickWatchDog(); // stop watchdog reboots
    }
    if (!addr) { // may have returned 0 just because storage is full
      // Work out roughly where the start is
      uint32_t newAddr = jsfAlignAddress(oldAddr + jsfGetFileSize(&header) + (uint32_t)sizeof(JsfFileHeader));
      if (newAddr<oldAddr) return false; // definitely corrupt!
      if (newAddr <= endAddr &&
          newAddr+sizeof(JsfFileHeader)>endAddr) return true; // not enough space - this is fine
    }
  }
  bool allFF = true;
  for (size_t i=0;i<sizeof(JsfFileHeader);i++)
    if (headerPtr[i]!=0xFF) allFF=false;

  if (allFF && ((addr && testType==JSFSTT_ALL) || // FULL: always search the remaining area
                (addr==startAddr && testType==JSFSTT_NORMAL))) { // NORMAL: if no files, only search everything if storage is empty
    return jsfIsErased(addr, endAddr-addr);
  }
  return allFF;
}

/** Return false if the current storage is not valid
 * or is corrupt somehow. Basically that means if
 * jsfGet[Next]FileHeader returns false but the header isn't all FF
 *
 * If fullTest is true, all of storage is scanned.
 * For instance the first page may be blank but other pages
 * may contain info (which is invalid)...
 */
bool jsfIsStorageValid(JsfStorageTestType testFlags) {
  if (!jsfIsBankStorageValid(JSF_START_ADDRESS, testFlags))
    return false;
#ifdef JSF_BANK2_START_ADDRESS
  if (!jsfIsBankStorageValid(JSF_BANK2_START_ADDRESS, testFlags))
    return false;
#endif
  return true;
}

/** Return true if there is nothing at all in Storage (first header on first page is all 0xFF) */
bool jsfIsBankStorageEmpty(uint32_t addr) {
  JsfFileHeader header;
  unsigned char *headerPtr = (unsigned char *)&header;
  jsfGetFileHeader(addr, &header, true);
  bool allFF = true;
  for (size_t i=0;i<sizeof(JsfFileHeader);i++)
    if (headerPtr[i]!=0xFF) allFF=false;
  return allFF;
}

/** Return true if there is nothing at all in Storage (first header on first page is all 0xFF) */
bool jsfIsStorageEmpty() {
  if (!jsfIsBankStorageEmpty(JSF_START_ADDRESS))
    return false;
#ifdef JSF_BANK2_START_ADDRESS
  if (!jsfIsBankStorageEmpty(JSF_BANK2_START_ADDRESS))
    return false;
#endif
  return true;
}

JsVar *jsfReadFile(JsfFileName name, int offset, int length) {
  JsfFileHeader header;
  uint32_t addr = jsfFindFile(name, &header);
  if (!addr) return 0;
  // clip requested read lengths
  if (offset<0) offset=0;
  int fileLen = (int)jsfGetFileSize(&header);
  if (length<=0) length=fileLen;
  if (offset>fileLen) offset=fileLen;
  if (offset+length>fileLen) length=fileLen-offset;
  if (length<=0) return jsvNewFromEmptyString();
  // now increment address by offset
  addr += (uint32_t)offset;
  return jsvAddressToVar(addr, (uint32_t)length);
}

JsVar* jsvAddressToVar(size_t addr, uint32_t length) {
  if (length<=0) return jsvNewFromEmptyString();
  size_t mappedAddr = jshFlashGetMemMapAddress((size_t)addr);
#ifdef SPIFLASH_BASE // if using SPI flash it can't be memory-mapped
  if (!mappedAddr) {
    return jsvNewFlashString((char*)(size_t)addr, (size_t)length);
  }
#endif
#ifdef LINUX
  // linux fakes flash with a file, so we can't just return a pointer to it!
  uint32_t alignedSize = jsfAlignAddress((uint32_t)length);
  char *d = (char*)malloc(alignedSize);
  jshFlashRead(d, (size_t)addr, alignedSize);
  JsVar *v = jsvNewStringOfLength((uint32_t)length, d);
  free(d);
  return v;
#else
  return jsvNewNativeString((char*)mappedAddr, length);
#endif
}

bool jsfWriteFile(JsfFileName name, JsVar *data, JsfFileFlags flags, JsVarInt offset, JsVarInt _size) {
  if (offset<0 || _size<0) return false;
  uint32_t size = (uint32_t)_size;
  // Data length
  JSV_GET_AS_CHAR_ARRAY(dPtr, dLen, data);
  if (!dPtr) {
    jsExceptionHere(JSET_ERROR, "Can't get pointer to data to write");
    return false;
  }
  if (size==0) size=(uint32_t)dLen;
  if (!size) {
    jsExceptionHere(JSET_ERROR, "Can't create zero length file");
    return false;
  }
  // Lookup file
  JsfFileHeader header;
  uint32_t addr = jsfFindFile(name, &header);
#ifdef JSF_BANK2_START_ADDRESS
  if (!addr && name.c[1]==':'){
    // if not found where it should be, try also another bank to not end with two files
    JsfFileName shortname = name;
    jsfStripDriveFromName(&shortname);
    JsfFileHeader header2;
    uint32_t addr2 = jsfFindFile(shortname, &header2);
    if (addr2) jsfEraseFileInternal(addr2, &header2, true);  // erase if in wrong bank
  }
#endif  
  if ((!addr && offset==0) || // No file
      // we have a file, but it's wrong - remove it
      (addr && offset==0 && (
          flags!=jsfGetFileFlags(&header) ||
          size!=jsfGetFileSize(&header) ||
          !jsfIsErased(addr, size)))) {
    if (addr && offset==0 &&
        size==jsfGetFileSize(&header) &&
        flags==jsfGetFileFlags(&header) &&
        dLen==size && // setting all in one go
        jsfIsEqual(addr, (unsigned char*)dPtr, (uint32_t)dLen)) {
      jsDebug(DBG_INFO,"jsfWriteFile files Equal\n");
      return true;
    }
    if (addr) { // file exists, remove it!
      jsDebug(DBG_INFO,"jsfWriteFile remove existing file\n");
      jsfEraseFileInternal(addr, &header, true);
    }
    jsDebug(DBG_INFO,"jsfWriteFile create file\n");
    addr = jsfCreateFile(name, (uint32_t)size, flags, &header);
  }
  if (!addr) {
    jsExceptionHere(JSET_ERROR, "Unable to find or create file");
    return false;
  }
  if ((uint32_t)offset+(uint32_t)dLen > jsfGetFileSize(&header)) {
    jsExceptionHere(JSET_ERROR, "Too much data for file size");
    return false;
  }
  addr += (uint32_t)offset;
  if (!jsfIsErased(addr, (uint32_t)dLen)) {
    jsExceptionHere(JSET_ERROR, "File already written with different data");
    return false;
  }
  jsDebug(DBG_INFO,"jsfWriteFile write contents\n");
  jshFlashWriteAligned(dPtr, addr, (uint32_t)dLen);
  jsDebug(DBG_INFO,"jsfWriteFile written contents\n");
  return true;
}

static void jsfBankListFilesHandleFile(JsVar *files, uint32_t addr, JsfFileHeader *header, JsVar *regex, JsfFileFlags containing, JsfFileFlags notContaining, uint32_t *hash) {
  JsfFileFlags flags = jsfGetFileFlags(header);
  if (notContaining&flags) return;
  if (containing && !(containing&flags)) return;
  if (flags&JSFF_STORAGEFILE) {
    // find last char
    int i = 0;
    while (i+1<sizeof(header->name) && header->name.c[i+1]) i++;
    // if last ch isn't \1 (eg first StorageFile) ignore this
    if (header->name.c[i]!=1) return;
    // if we're specifically asking for StorageFile, remove last char
    if (containing&JSFF_STORAGEFILE)
      header->name.c[i]=0;
  }
  JsVar *v = jsfVarFromName(header->name);
  bool match = true;
  if (regex) {
    JsVar *m = jswrap_string_match(v,regex);
    match = !(jsvIsUndefined(m) || jsvIsNull(m));
    jsvUnLock(m);
  }
#ifndef SAVE_ON_FLASH
  if (hash && match) {
    *hash = (*hash<<1) | (*hash>>31); // roll hash
    *hash = *hash ^ addr ^ jsvGetIntegerAndUnLock(jswrap_espruino_CRC32(v)); // apply filename
  }
#endif
  if (match && files) jsvArrayPushAndUnLock(files, v);
  else jsvUnLock(v);
}

/** Return all files in flash as a JsVar array of names. If regex is supplied, it is used to filter the filenames using String.match(regexp)
 * If containing!=0, file flags must contain one of the 'containing' argument's bits.
 * Flags can't contain any bits in the 'notContaining' argument
 */
static void jsfBankListFiles(JsVar *files, uint32_t addr, JsVar *regex, JsfFileFlags containing, JsfFileFlags notContaining, uint32_t *hash) {
  JsfFileHeader header;
  memset(&header,0,sizeof(JsfFileHeader));
#ifdef ESPR_STORAGE_FILENAME_TABLE
  if (jsfFilenameTableBank1Addr && addr==JSF_START_ADDRESS) {
    //jsiConsolePrintf("jsfFilenameTable 0x%08x\n", jsfFilenameTableBank1Addr);
    uint32_t baseAddr = addr;
    uint32_t tableAddr = jsfFilenameTableBank1Addr;
    uint32_t tableEnd = tableAddr + jsfFilenameTableBank1Size;
    // Now scan the table and call back for each item
    while (tableAddr < tableEnd) {
      // read just the address
      jshFlashRead(&header, tableAddr, 4);
      tableAddr += (uint32_t)sizeof(JsfFileHeader);
      // Now read the header at the address we have in our table (file may have been deleted)
      uint32_t fileAddr = baseAddr + header.size;
      if (jsfGetFileHeader(fileAddr, &header, true) && jsfIsRealFile(&header)) {
        jsfBankListFilesHandleFile(files, fileAddr, &header, regex, containing, notContaining, hash);
      }
    }
    // We didn't find the file in our table...
    // Now point 'addr' to the start of this table and fill in the header.
    // the normal code will see this, skip over it like a normal file
    // and carry on regardless.
    addr = jsfFilenameTableBank1Addr - sizeof(JsfFileHeader); // address of jsfFilenameTable's header
    header.name.firstChars = 0;
    header.size = jsfFilenameTableBank1Size;
  } else
#endif
  if (!jsfGetFileHeader(addr, &header, true)) return;
  do {
    if (jsfIsRealFile(&header)) { // if not replaced or a system file
      jsfBankListFilesHandleFile(files, addr, &header, regex, containing, notContaining, hash);
    }
  } while (jsfGetNextFileHeader(&addr, &header, GNFH_GET_ALL));
}

/** Return all files in flash as a JsVar array of names. If regex is supplied, it is used to filter the filenames using String.match(regexp)
 * If containing!=0, file flags must contain one of the 'containing' argument's bits.
 * Flags can't contain any bits in the 'notContaining' argument
 */
JsVar *jsfListFiles(JsVar *regex, JsfFileFlags containing, JsfFileFlags notContaining) {
  JsVar *files = jsvNewEmptyArray();
  if (!files) return 0;
  jsfBankListFiles(files, JSF_START_ADDRESS, regex, containing, notContaining, NULL);
#ifdef JSF_BANK2_START_ADDRESS
  jsfBankListFiles(files, JSF_BANK2_START_ADDRESS, regex, containing, notContaining, NULL);
#endif
  return files;
}


/** Hash all files matching regex
 * If containing!=0, file flags must contain one of the 'containing' argument's bits.
 * Flags can't contain any bits in the 'notContaining' argument
 */
uint32_t jsfHashFiles(JsVar *regex, JsfFileFlags containing, JsfFileFlags notContaining) {
  uint32_t hash = 0xABCDDCBA;
  jsfBankListFiles(NULL, JSF_START_ADDRESS, regex, containing, notContaining, &hash);
#ifdef JSF_BANK2_START_ADDRESS
  jsfBankListFiles(NULL, JSF_BANK2_START_ADDRESS, regex, containing, notContaining, &hash);
#endif
  return hash;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------ For loading/saving code to flash
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------

// Get a hash of the current Git commit, so new builds won't load saved code
static uint32_t getBuildHash() {
#ifdef GIT_COMMIT
  const unsigned char *s = (unsigned char*)STRINGIFY(GIT_COMMIT);
  uint32_t hash = 0;
  while (*s)
    hash = (hash<<1) ^ *(s++);
  return hash;
#else
  return 0;
#endif
}

typedef struct {
  uint32_t address;          // current address in memory
  uint32_t endAddress;       // address at which to end
  uint32_t byteCount;
  unsigned char buffer[128]; // buffer for read/written data
  uint32_t bufferCnt;        // where are we in the buffer?
} jsfcbData;
// cbdata = struct jsfcbData
void jsfSaveToFlash_writecb(unsigned char ch, uint32_t *cbdata) {
  jsfcbData *data = (jsfcbData*)cbdata;
  data->buffer[data->bufferCnt++] = ch;
  if (data->bufferCnt>=(uint32_t)sizeof(data->buffer)) {
    jshFlashWrite(data->buffer, data->address, data->bufferCnt);
    data->address += data->bufferCnt;
    data->bufferCnt = 0;
    if ((data->address&1023)==0) jsiConsolePrint(".");
  }
}
void jsfSaveToFlash_finish(jsfcbData *data) {
  // pad to alignment
  while (data->bufferCnt & (JSF_ALIGNMENT-1))
    data->buffer[data->bufferCnt++] = 0xFF;
  // write
  jshFlashWrite(data->buffer, data->address, data->bufferCnt);
}

// cbdata = struct jsfcbData
int jsfLoadFromFlash_readcb(uint32_t *cbdata) {
  jsfcbData *data = (jsfcbData*)cbdata;

  if (data->address >= data->endAddress) return -1; // at end
  if (data->byteCount==0 || data->bufferCnt>=data->byteCount) {
    data->byteCount = data->endAddress - data->address;
    if (data->byteCount > sizeof(data->buffer))
      data->byteCount = sizeof(data->buffer);
    jshFlashRead(data->buffer, data->address, data->byteCount);
    data->bufferCnt = 0;
  }
  data->address++;
  return data->buffer[data->bufferCnt++];
}

/// Save the RAM image to flash (this is the actual interpreter state)
void jsfSaveToFlash() {
#ifdef ESPR_NO_VARIMAGE
  jsiConsolePrint("Not implemented in this build\n");
#else
  unsigned int varSize = jsvGetMemoryTotal() * (unsigned int)sizeof(JsVar);
  unsigned char* varPtr = (unsigned char *)_jsvGetAddressOf(1);

  jsiConsolePrint("Compacting Flash...\n");
  JsfFileName name = jsfNameFromString(SAVED_CODE_VARIMAGE);
  // Ensure we get rid of any saved code we had before
  jsfEraseFile(name);
  // Try and compact, just to ensure we get the maximum amount saved
  jsfCompact();
  jsiConsolePrint("Calculating Size...\n");
  // Work out how much data this'll take, plus 4 bytes for build hash
  uint32_t compressedSize = 4 + COMPRESS(varPtr, varSize, NULL, NULL);
  // How much data do we have?
  uint32_t savedCodeAddr = jsfCreateFile(name, compressedSize, JSFF_COMPRESSED, NULL);
  if (!savedCodeAddr) {
    jsiConsolePrintf("ERROR: Too big to save to flash (%d vs %d bytes)\n", compressedSize, jsfGetStorageStats(0,true).free);
    jsvSoftInit();
    jspSoftInit();
    jsiConsolePrint("Deleting command history and trying again...\n");
    while (jsiFreeMoreMemory());
    jspSoftKill();
    jsvSoftKill();
    compressedSize = 4 + COMPRESS(varPtr, varSize, NULL, NULL);
    savedCodeAddr = jsfCreateFile(name, compressedSize, JSFF_COMPRESSED, NULL);
  }
  if (!savedCodeAddr) {
    if (jsfGetStorageStats(JSF_DEFAULT_START_ADDRESS, true).fileBytes)
      jsiConsolePrint("Not enough free space to save. Try require('Storage').eraseAll()\n");
    else
      jsiConsolePrint("Code is too big to save to Flash.\n");
    return;
  }
  // Ok, we have space!
  // Now start writing
  jsfcbData cbData;
  memset(&cbData, 0, sizeof(cbData));
  cbData.address = savedCodeAddr;
  cbData.endAddress = jsfAlignAddress(savedCodeAddr+compressedSize);
  jsiConsolePrint("Writing..");
  // write the hash
  uint32_t hash = getBuildHash();
  int i;
  for (i=0;i<4;i++)
    jsfSaveToFlash_writecb(((unsigned char*)&hash)[i], (uint32_t*)&cbData);
  // write compressed data
  COMPRESS(varPtr, varSize, jsfSaveToFlash_writecb, (uint32_t*)&cbData);
  jsfSaveToFlash_finish(&cbData);
  jsiConsolePrintf("\nCompressed %d bytes to %d\n", varSize, compressedSize);
#endif
}


/// Load the RAM image from flash (this is the actual interpreter state)
void jsfLoadStateFromFlash() {
#ifndef ESPR_NO_VARIMAGE
  JsfFileHeader header;
  uint32_t savedCode = jsfFindFile(jsfNameFromString(SAVED_CODE_VARIMAGE),&header);
  if (!savedCode) {
    return;
  }

  //  unsigned int dataSize = jsvGetMemoryTotal() * sizeof(JsVar);
  unsigned char* varPtr = (unsigned char *)_jsvGetAddressOf(1);

  jsfcbData cbData;
  memset(&cbData, 0, sizeof(cbData));
  cbData.address = savedCode;
  cbData.endAddress = savedCode+jsfGetFileSize(&header);

  uint32_t hash;
  int i;
  for (i=0;i<4;i++)
    ((char*)&hash)[i] = (char)jsfLoadFromFlash_readcb((uint32_t*)&cbData);
  if (hash != getBuildHash()) {
    jsiConsolePrintf("Not loading saved code from different Espruino firmware.\n");
    return;
  }
  jsiConsolePrintf("Loading %d bytes from flash...\n", jsfGetFileSize(&header));
  DECOMPRESS(jsfLoadFromFlash_readcb, (uint32_t*)&cbData, varPtr);
#endif
}

void jsfSaveBootCodeToFlash(JsVar *code, bool runAfterReset) {
  jsfEraseFile(jsfNameFromString(SAVED_CODE_BOOTCODE));
  jsfEraseFile(jsfNameFromString(SAVED_CODE_BOOTCODE_RESET));
  if (jsvIsUndefined(code) || jsvGetLength(code)==0)
    return;
  jsfWriteFile(jsfNameFromString(runAfterReset ? SAVED_CODE_BOOTCODE_RESET : SAVED_CODE_BOOTCODE), code, JSFF_NONE, 0, 0);
}

JsVar *jsfGetBootCodeFromFlash(bool isReset) {
  JsVar *resetCode = jsfReadFile(jsfNameFromString(SAVED_CODE_BOOTCODE_RESET),0,0);
  if (isReset || resetCode) return resetCode;
  return jsfReadFile(jsfNameFromString(SAVED_CODE_BOOTCODE),0,0);
}

bool jsfLoadBootCodeFromFlash(bool isReset) {
  // Load code in .boot0/1/2/3 UNLESS BTN1 is HELD DOWN FOR BANGLE.JS ON FIRST BOOT
  // On an average Bangle.js 2 this takes 0.25 ms (so not worth optimising)
#if (defined(BANGLEJS) && !defined(DICKENS))
  if (!(jshPinGetValue(BTN1_PININDEX)==BTN1_ONSTATE &&
       (jsiStatus & JSIS_FIRST_BOOT)))
#endif
  {
    char filename[7] = ".bootX";
    for (int i=0;i<4;i++) {
      filename[5] = (char)('0'+i);
      JsVar *code = jsfReadFile(jsfNameFromString(filename),0,0);
      if (code)
        jsvUnLock2(jspEvaluateVar(code,0,0), code);
    }
  }
  // Load normal boot code
  JsVar *code = jsfGetBootCodeFromFlash(isReset);
  if (!code) return false;
  jsvUnLock2(jspEvaluateVar(code,0,0), code);
  return true;
}

bool jsfFlashContainsCode() {
  return
#ifndef ESPR_NO_VARIMAGE
      jsfFindFile(jsfNameFromString(SAVED_CODE_VARIMAGE),0) ||
#endif
      jsfFindFile(jsfNameFromString(SAVED_CODE_BOOTCODE),0) ||
      jsfFindFile(jsfNameFromString(SAVED_CODE_BOOTCODE_RESET),0);
}

/** Completely clear any saved code from flash. */
void jsfRemoveCodeFromFlash() {
  jsiConsolePrint("Erasing saved code.");
#ifndef ESPR_NO_VARIMAGE
  jsfEraseFile(jsfNameFromString(SAVED_CODE_VARIMAGE));
#endif
  jsfEraseFile(jsfNameFromString(SAVED_CODE_BOOTCODE));
  jsfEraseFile(jsfNameFromString(SAVED_CODE_BOOTCODE_RESET));
  jsiConsolePrint("\nDone!\n");
}

// Erase storage to 'factory' values.
void jsfResetStorage() {
  jsiConsolePrintf("Erasing Storage Area...\n");
  jsfEraseAll();
  jsiConsolePrintf("Erase complete.\n");
#if ESPR_STORAGE_INITIAL_CONTENTS
  // if we store initial contents, write them here after erasing storage
  jsiConsolePrintf("Writing initial storage contents...\n");
  extern const unsigned char jsfStorageInitialContents[];
  extern const int jsfStorageInitialContentLength;
  jshFlashWrite(jsfStorageInitialContents, FLASH_SAVED_CODE_START, jsfStorageInitialContentLength);
  jsiConsolePrintf("Write complete.\n");
#endif
}

#ifdef ESPR_STORAGE_FILENAME_TABLE
/// Create a lookup table for filenames. On success return file's address
static uint32_t jsfBankCreateFileTable(uint32_t startAddr) {
  JsfFileHeader header;
  memset(&header,0,sizeof(JsfFileHeader));
  uint32_t fileCount = 0;
  // first count files
  uint32_t addr = startAddr; // address of file header
  if (jsfGetFileHeader(addr, &header, false)) do {
    if (jsfIsRealFile(&header)) fileCount++;
  } while (jsfGetNextFileHeader(&addr, &header, GNFH_GET_ALL));
  jsDebug(DBG_INFO,"jsfBankCreateFileTable - %d files\n", fileCount);
  // now write table
  JsfFileName name = jsfNameFromString(JSF_FILENAME_TABLE_NAME);
  uint32_t tableAddr = jsfFindFile(name, &header); // address of file data (not header)
  if (tableAddr) jsfEraseFileInternal(tableAddr, &header, false);
  uint32_t tableSize = fileCount * (uint32_t)sizeof(JsfFileHeader);
  if (tableSize==0) return 0; // empty table
  tableAddr = jsfCreateFile(name, tableSize, JSFF_FILENAME_TABLE, &header);
  if (!tableAddr) return 0; // couldn't create file
  // Now rescan files and write into file
  addr = startAddr;
  uint32_t writeAddr = tableAddr;
  if (jsfGetFileHeader(addr, &header, true)) do {
    if (jsfIsRealFile(&header)) {
      JsfFileHeader filenameTableHeader = header;
      filenameTableHeader.size = addr - startAddr; // write file address into file
      jshFlashWriteAligned(&filenameTableHeader, writeAddr, sizeof(JsfFileHeader));
      writeAddr += (uint32_t)sizeof(JsfFileHeader);
    }
  } while (jsfGetNextFileHeader(&addr, &header, GNFH_GET_ALL));
  if (startAddr == JSF_START_ADDRESS) {
    jsfFilenameTableBank1Addr = tableAddr;
    jsfFilenameTableBank1Size = tableSize;
  }
  return tableAddr;
}

/// Create a lookup table for files - this speeds up file access
void jsfCreateFileTable() {
  jsfBankCreateFileTable(JSF_START_ADDRESS);
}
#endif

