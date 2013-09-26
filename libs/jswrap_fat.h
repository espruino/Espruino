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

JsVar *wrap_fat_readdir(JsVar *path);
void wrap_fat_writeFile(JsVar *path, JsVar *data);
void wrap_fat_appendFile(JsVar *path, JsVar *data);
JsVar *wrap_fat_readFile(JsVar *path);
