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
 * Contains JavaScript HTTP Functions
 * ----------------------------------------------------------------------------
 */
#include "jsvar.h"

JsVar *jswrap_http_createServer(JsVar *callback);

JsVar *jswrap_http_request(JsVar *options, JsVar *callback);
JsVar *jswrap_http_get(JsVar *options, JsVar *callback);

void jswrap_httpSRs_writeHead(JsVar *parent, int statusCode, JsVar *headers);
bool jswrap_httpSRs_write(JsVar *parent, JsVar *data);
void jswrap_httpSRs_end(JsVar *parent, JsVar *data);

bool jswrap_httpCRq_write(JsVar *parent, JsVar *data);
void jswrap_httpCRq_end(JsVar *parent, JsVar *data);




