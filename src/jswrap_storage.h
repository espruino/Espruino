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

#define WIFI_CONFIG_STORAGE_NAME ".wificfg"

void jswrap_storage_eraseAll();
JsVar *jswrap_storage_read(JsVar *name, int offset, int length);
JsVar *jswrap_storage_readJSON(JsVar *name, bool noExceptions);
JsVar *jswrap_storage_readArrayBuffer(JsVar *name);
bool jswrap_storage_write(JsVar *name, JsVar *data, JsVarInt offset, JsVarInt size);
bool jswrap_storage_writeJSON(JsVar *name, JsVar *data);
void jswrap_storage_erase(JsVar *name);
void jswrap_storage_compact();
JsVar *jswrap_storage_list(JsVar *regex, JsVar *filter);
JsVarInt jswrap_storage_hash(JsVar *regex);
void jswrap_storage_debug();
int jswrap_storage_getFree();
JsVar *jswrap_storage_getStats();
void jswrap_storage_optimise();

JsVar *jswrap_storage_open(JsVar *name, JsVar *mode);
JsVar *jswrap_storagefile_read(JsVar *f, int len);
JsVar *jswrap_storagefile_readLine(JsVar *f);
int jswrap_storagefile_getLength(JsVar *f);
void jswrap_storagefile_write(JsVar *parent, JsVar *_data);
void jswrap_storagefile_erase(JsVar *f);

