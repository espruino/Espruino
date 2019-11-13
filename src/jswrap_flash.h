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

// Flash Library exports
JsVar *jswrap_flash_getPage(int addr);
JsVar *jswrap_flash_getFree();
void jswrap_flash_erasePage(JsVar *addr);
void jswrap_flash_write(JsVar *data, int addr);
JsVar *jswrap_flash_read(int length, int addr);

