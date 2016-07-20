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

JsVar *jswrap_flash_getPage(int addr);
JsVar *jswrap_flash_getFree();
void jswrap_flash_erasePage(JsVar *addr);
void jswrap_flash_write(JsVar *data, int addr);
JsVar *jswrap_flash_read(int length, int addr);

typedef enum {
  SFF_SAVE_STATE = 1,      // Should we save state to flash?
  SFF_BOOT_CODE_ALWAYS = 2 // When saving boot code, ensure it should always be run - even after reset
} JsvSaveFlashFlags;

/// Save contents of JsVars into Flash. If bootCode is specified, save bootup code too.
void jsfSaveToFlash(JsvSaveFlashFlags flags, JsVar *bootCode);
/// Load the RAM image from flash (this is the actual interpreter state)
void jsfLoadStateFromFlash();
/** Load bootup code from flash (this is textual JS code). return true if it exists and was executed.
 * isReset should be set if we're loading after a reset (eg, does the user expect this to be run or not)
 */
bool jsfLoadBootCodeFromFlash(bool isReset);
/// Returns true if flash contains something useful
bool jsfFlashContainsCode();
