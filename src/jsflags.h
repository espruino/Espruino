/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2017 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * JS Interpreter specific flags
 * ----------------------------------------------------------------------------
 */
#ifndef JSFLAGS_H_
#define JSFLAGS_H_
#include "jsvar.h"


typedef enum {
  JSF_NONE,
  JSF_DEEP_SLEEP          = 1<<0, ///< Allow deep sleep modes (also set by setDeepSleep)
  JSF_UNSAFE_FLASH        = 1<<1, ///< Some platforms stop writes/erases to interpreter memory to stop you bricking the device accidentally - this removes that protection
  JSF_UNSYNC_FILES        = 1<<2, ///< When accessing files, *don't* flush all data to the SD card after each command. Faster, but risky if power is lost
#ifndef ESPR_NO_PRETOKENISE
  JSF_PRETOKENISE         = 1<<3, ///< When adding functions, pre-minify them and tokenise reserved words (adding "ram" at the start does this too)
#endif
#ifdef ESPR_JIT
  JSF_JIT_DEBUG           = 1<<4, ///< When JIT enabled, output debugging info
#endif
  JSF_ON_ERROR_SAVE      = 1<<5, ///< If set, save error and stack trace to an 'ERROR' file in internal Storage
  JSF_ON_ERROR_FLASH_LED = 1<<6, ///< If set, when we get an error, flash the Red LED
} PACKED_FLAGS JsFlags;


#define JSFLAG_NAMES "deepSleep\0unsafeFlash\0unsyncFiles\0pretokenise\0jitDebug\0onErrorSave\0onErrorFlash\0"
// NOTE: \0 also added by compiler - two \0's are required!

extern volatile JsFlags jsFlags;

/// Get the state of a flag
bool jsfGetFlag(JsFlags flag);
/// Set the state of a flag
void jsfSetFlag(JsFlags flag, bool isOn);
/// Get a list of all flags and their status
JsVar *jsfGetFlags();
/// Set any of the specified flags
void jsfSetFlags(JsVar *flags);

#endif //JSFLAGS_H_
