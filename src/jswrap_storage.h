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
 * JavaScript Filesystem-style Flash IO functions
 * ----------------------------------------------------------------------------
 */
#include "jsvar.h"

void jswrap_storage_eraseAll();
JsVar *jswrap_storage_read(JsVar *name);
JsVar *jswrap_storage_readJSON(JsVar *name);
JsVar *jswrap_storage_readArrayBuffer(JsVar *name);
bool jswrap_storage_write(JsVar *name, JsVar *data, JsVarInt offset, JsVarInt size);
void jswrap_storage_erase(JsVar *name);
void jswrap_storage_compact();
JsVar *jswrap_storage_list();
void jswrap_storage_debug();
