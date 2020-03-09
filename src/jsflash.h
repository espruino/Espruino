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
#ifndef JSFLASH_H_
#define JSFLASH_H_

#include "jsvar.h"

/// Simple filename used for Flash Storage. We use firstChars so we can do a quick first pass check for equality
typedef union {
  uint32_t firstChars; ///< Set these all to 0 to indicate a replaced/deleted file
  char c[28]; // whatever is left after 'size'
} JsfFileName;

/// Max length of filename in chars
#define JSF_MAX_FILENAME_LENGTH (sizeof(JsfFileName))

/// Structure for File Storage. It's important this is 8 byte aligned for platforms that only support 64 bit writes
typedef struct {
  uint32_t size; ///< Total size (and flags in the top 8 bits)
  JsfFileName name; ///< 0-padded filename
} JsfFileHeader;

typedef enum {
  JSFF_NONE,
  JSFF_STORAGEFILE = 64,  // This file is a 'storage file' created by Storage.open
  JSFF_COMPRESSED = 128   // This file contains compressed data
} JsfFileFlags; // these are stored in the top 8 bits of JsfFileHeader.size


// ------------------------------------------------------------------------ Flash Storage Functionality
/// utility function for creating JsfFileName
JsfFileName jsfNameFromString(const char *name);
/// utility function for creating JsfFileName
JsfFileName jsfNameFromVar(JsVar *name);
JsfFileName jsfNameFromVarAndUnLock(JsVar *name);
/// Return the size in bytes of a file based on the header
uint32_t jsfGetFileSize(JsfFileHeader *header);
/// Return the flags for this file based on the header
JsfFileFlags jsfGetFileFlags(JsfFileHeader *header);
/// Find a 'file' in the memory store. Return the address of data start (and header if returnedHeader!=0). Returns 0 if not found
uint32_t jsfFindFile(JsfFileName name, JsfFileHeader *returnedHeader);
/// Return the contents of a file as a memory mapped var
JsVar *jsfReadFile(JsfFileName name, int offset, int length);
/// Write a file. For simple stuff just leave offset and size as 0
bool jsfWriteFile(JsfFileName name, JsVar *data, JsfFileFlags flags, JsVarInt offset, JsVarInt _size);
/// Erase the given file, return true on success
bool jsfEraseFile(JsfFileName name);
/// Erase the entire contents of the memory store
bool jsfEraseAll();
/// Try and compact saved data so it'll fit in Flash again
bool jsfCompact();
/// Return all files in flash as a JsVar array of names. If regex is supplied, it is used to filter the filenames using String.match(regexp)
JsVar *jsfListFiles(JsVar *regex);
/// Output debug info for files stored in flash storage
void jsfDebugFiles();
// Get the amount of space free in this page (or all pages). addr=0 uses start page
uint32_t jsfGetFreeSpace(uint32_t addr, bool allPages);

// ------------------------------------------------------------------------ For loading/saving code to flash
/// Save contents of JsVars into Flash.
void jsfSaveToFlash();
/// Load the RAM image from flash (this is the actual interpreter state)
void jsfLoadStateFromFlash();

/// Save bootup code to flash - see jsfLoadBootCodeFromFlash
void jsfSaveBootCodeToFlash(JsVar *code, bool runAfterReset);
/// Load bootup code from flash - see jsfGetBootCodeFromFlash
bool jsfLoadBootCodeFromFlash(bool isReset);
/** Get bootup code from flash (this is textual JS code). return a pointer to it if it exists, or 0.
 * isReset should be set if we're loading after a reset (eg, does the user expect this to be run or not).
 * Set isReset=false to always return the code  */
JsVar *jsfGetBootCodeFromFlash(bool isReset);
/// Returns true if flash contains something useful
bool jsfFlashContainsCode();
/** Completely clear any saved code from flash. */
void jsfRemoveCodeFromFlash();

#endif //JSFLASH_H_
