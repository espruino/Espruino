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
 * Contains built-in functions for SD card access, based on node.js 'fs' module
 * ----------------------------------------------------------------------------
 */
#include "jsvar.h"

JsVar *jswrap_fs_readdir(JsVar *path);
bool jswrap_fs_writeOrAppendFile(JsVar *path, JsVar *data, bool append);
JsVar *jswrap_fs_readFile(JsVar *path);
bool jswrap_fs_unlink(JsVar *path);
JsVar *jswrap_fs_stat(JsVar *path);
bool jswrap_fs_mkdir(JsVar *path);
