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
 * Unistroke handling functionality
 * ----------------------------------------------------------------------------
 */
#include "jsutils.h"
#include "jsvar.h"

/// Create a new Unistroke based on XY coordinates
JsVar *jswrap_unistroke_new(JsVar *xy);

/// Recognise based on an object of named strokes, and a list of XY coordinates
JsVar *jswrap_unistroke_recognise(JsVar *strokes, JsVar *xy);
