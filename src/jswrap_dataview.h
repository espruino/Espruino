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
 * JavaScript DataView Implementation
 * ----------------------------------------------------------------------------
 */
#include "jsvar.h"

JsVar *jswrap_dataview_constructor(JsVar *buffer, int byteOffset, int byteLength);
JsVar *jswrap_dataview_get(JsVar *dataview, JsVarDataArrayBufferViewType type, int byteOffset, bool littleEndian);
void jswrap_dataview_set(JsVar *dataview, JsVarDataArrayBufferViewType type, int byteOffset, JsVar *value, bool littleEndian);
