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
 * Simple compression/decompression using the heatshrink library
 * ----------------------------------------------------------------------------
 */
#include "jsvar.h"

JsVar *jswrap_heatshrink_compress(JsVar *data);
JsVar *jswrap_heatshrink_decompress(JsVar *data);
