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
 * JavaScript Flash IO functions
 * ----------------------------------------------------------------------------
 */
#include "jsvar.h"

// Flash Library exports
JsVar *jswrap_flash_getPage(int addr);
JsVar *jswrap_flash_getFree();
void jswrap_flash_erasePage(JsVar *addr);
void jswrap_flash_write(JsVar *data, int addr);
JsVar *jswrap_flash_read(int length, int addr);



/// Save contents of JsVars into Flash.
void jsfSaveToFlash();
/// Load the RAM image from flash (this is the actual interpreter state)
void jsfLoadStateFromFlash();

void jsfSaveBootCodeToFlash(JsVar *code, bool runAfterReset);
/** Load bootup code from flash (this is textual JS code). return true if it exists and was executed.
 * isReset should be set if we're loading after a reset (eg, does the user expect this to be run or not).
 * Set isReset=false to always run the code
 */
bool jsfLoadBootCodeFromFlash(bool isReset);
/** Get bootup code from flash (this is textual JS code). return a pointer to it if it exists, or 0.
 * isReset should be set if we're loading after a reset (eg, does the user expect this to be run or not).
 * Set isReset=false to always return the code  */
JsVar *jsfGetBootCodeFromFlash(bool isReset);
/// Returns true if flash contains something useful
bool jsfFlashContainsCode();
/** Completely clear any saved code from flash. */
void jsfRemoveCodeFromFlash();




typedef uint64_t JsfFileName;
typedef struct {
  uint32_t size; ///< Total size
  JsfFileName name; ///< 0-padded filename
  uint32_t replacement; ///< pointer to a replacement (eventually). For now this is 0xFFFFFFFF if ok, 0 if erased
} JsfFileHeader;


JsfFileName jsfNameFromString(const char *name);
JsfFileName jsfNameFromVar(JsVar *name);
uint32_t jsfCreateFile(JsfFileName name, uint32_t size, uint32_t startAddr, JsfFileHeader *returnedHeader);
/// Find a 'file' in the memory store. Return the address of data start (and header if returnedHeader!=0). Returns 0 if not found
uint32_t jsfFindFile(JsfFileName name, JsfFileHeader *returnedHeader);
/// Return the contents of a file as a memory mapped var
JsVar *jsfReadFile(JsfFileName name);
/// Write a file. For simple stuff just leave offset and size as 0
bool jsfWriteFile(JsfFileName name, JsVar *data, JsVarInt offset, JsVarInt _size);
/// Erase the given file
void jsfEraseFile(JsfFileName name);
/// Try and compact saved data so it'll fit in Flash again
bool jsfCompact();


void jswrap_flash_eraseFiles();
JsVar *jswrap_flash_readFile(JsVar *name);
bool jswrap_flash_writeFile(JsVar *name, JsVar *data, JsVarInt offset, JsVarInt size);
void jswrap_flash_eraseFile(JsVar *name);
void jswrap_flash_compactFiles();
JsVar *jswrap_flash_listFiles();
void jswrap_flash_debugFiles();
