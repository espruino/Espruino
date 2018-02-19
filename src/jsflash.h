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
#include "jsvar.h"

/// Simple filename used for Flash Storage. We use uint here so we don't have to memcpy/memcmp all the time
typedef uint64_t JsfFileName;

#ifdef FLASH_64BITS_ALIGNMENT
typedef uint64_t JsfWord;
#define JSF_ALIGNMENT 8
#define JSF_WORD_UNSET 0xFFFFFFFFFFFFFFFFULL
#else
typedef uint32_t JsfWord;
#define JSF_ALIGNMENT 4
#define JSF_WORD_UNSET 0xFFFFFFFF
#endif

/// Structure for File Storage. It's important this is 8 byte aligned for platforms that only support 64 bit writes
typedef struct {
  JsfWord size; ///< Total size
  JsfWord replacement; ///< pointer to a replacement (eventually). For now this is 0xFFFFFFFF if ok, 0 if erased
  JsfFileName name; ///< 0-padded filename
} JsfFileHeader;

typedef enum {
  JSFF_NONE,
  JSFF_COMPRESSED = 128   // This file contains compressed data
} JsfFileFlags;


// ------------------------------------------------------------------------ Flash Storage Functionality
/// utility function for creating JsfFileName
JsfFileName jsfNameFromString(const char *name);
/// utility function for creating JsfFileName
JsfFileName jsfNameFromVar(JsVar *name);
/// Return the size in bytes of a file based on the header
uint32_t jsfGetFileSize(JsfFileHeader *header);
/// Return the flags for this file based on the header
JsfFileFlags jsfGetFileFlags(JsfFileHeader *header);
/// Find a 'file' in the memory store. Return the address of data start (and header if returnedHeader!=0). Returns 0 if not found
uint32_t jsfFindFile(JsfFileName name, JsfFileHeader *returnedHeader);
/// Return the contents of a file as a memory mapped var
JsVar *jsfReadFile(JsfFileName name);
/// Write a file. For simple stuff just leave offset and size as 0
bool jsfWriteFile(JsfFileName name, JsVar *data, JsfFileFlags flags, JsVarInt offset, JsVarInt _size);
/// Erase the given file
void jsfEraseFile(JsfFileName name);
/// Erase the entire contents of the memory store
bool jsfEraseAll();
/// Try and compact saved data so it'll fit in Flash again
bool jsfCompact();
/// Return all files in flash as a JsVar array of names
JsVar *jsfListFiles();
/// Output debug info for files stored in flash storage
void jsfDebugFiles();

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
