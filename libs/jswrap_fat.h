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
 * Contains built-in functions for SD card access
 * ----------------------------------------------------------------------------
 */
#include "jsvar.h"

void jswrap_fat_kill();
JsVar *jswrap_fat_readdir(JsVar *path);
bool jswrap_fat_writeOrAppendFile(JsVar *path, JsVar *data, bool append);
JsVar *jswrap_fat_readFile(JsVar *path);
bool jswrap_fat_unlink(JsVar *path);
