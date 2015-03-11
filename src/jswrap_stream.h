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
 * JavaScript Stream Functions
 * ----------------------------------------------------------------------------
 */
#include "jsvar.h"


#define STREAM_BUFFER_NAME JS_HIDDEN_CHAR_STR"buf" // the buffer to store data in when no listener is defined
#define STREAM_CALLBACK_NAME "#ondata"
#define STREAM_MAX_BUFFER_SIZE 512

JsVarInt jswrap_stream_available(JsVar *parent);
JsVar *jswrap_stream_read(JsVar *parent, JsVarInt chars);

/** Push data into a stream. To be used by Espruino (not a user).
 * This either calls the on('data') handler if it exists, or it
 * puts the data in a buffer. This MAY CLAIM the string that is
 * passed in.
 *
 * This will return true on success, or false if the buffer is
 * full. Setting force=true will attempt to fill the buffer as
 * full as possible, and will raise an error flag if data is lost.
 */
bool jswrap_stream_pushData(JsVar *parent, JsVar *dataString, bool force);

