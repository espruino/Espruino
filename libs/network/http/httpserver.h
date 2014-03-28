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
 * Contains HTTP client and server
 * ----------------------------------------------------------------------------
 */
#include "jsutils.h"
#include "jsvar.h"
#include "../network.h"


// -----------------------------
void httpInit();
void httpKill(JsNetwork *net);
bool httpIdle(JsNetwork *net);
// -----------------------------
JsVar *httpServerNew(JsVar *callback);
void httpServerListen(JsNetwork *net, JsVar *httpServerVar, int port);
void httpServerClose(JsNetwork *net, JsVar *server);

JsVar *httpClientRequestNew(JsVar *options, JsVar *callback);
void httpClientRequestWrite(JsVar *httpClientReqVar, JsVar *data);
void httpClientRequestEnd(JsNetwork *net, JsVar *httpClientReqVar);

void httpServerResponseWriteHead(JsVar *httpServerResponseVar, int statusCode, JsVar *headers);
void httpServerResponseData(JsVar *httpServerResponseVar, JsVar *data);
void httpServerResponseEnd(JsVar *httpServerResponseVar);
