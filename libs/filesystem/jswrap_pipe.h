/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2014 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * Handling of the 'pipe' function - to pipe one stream to another
 * ----------------------------------------------------------------------------
 */

#include "jsutils.h"
#include "jsvar.h"
#include "jsparse.h"
#include "jsinteractive.h"

bool jswrap_pipe_idle();
void jswrap_pipe_kill();

void jswrap_pipe(JsVar* source, JsVar* dest, JsVar* options);
