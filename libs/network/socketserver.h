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
#ifndef SOCKETSERVER_H
#define SOCKETSERVER_H


#include "jsutils.h"
#include "jsvar.h"
#include "network.h"

// -----------------------------
void socketInit();
void socketKill(JsNetwork *net);
bool socketIdle(JsNetwork *net);

// -----------------------------
JsVar *serverNew(SocketType socketType, JsVar *callback);
void serverAddMembership(JsNetwork *net, JsVar *socket, JsVar *group, JsVar *ip);
void serverListen(JsNetwork *net, JsVar *httpServerVar, unsigned short port, SocketType socketType);
void serverClose(JsNetwork *net, JsVar *server);

JsVar *clientRequestNew(SocketType socketType, JsVar *options, JsVar *callback);
void clientRequestWrite(JsNetwork *net, JsVar *httpClientReqVar, JsVar *data, JsVar *host, unsigned short port);
void clientRequestConnect(JsNetwork *net, JsVar *httpClientReqVar);
void clientRequestEnd(JsNetwork *net, JsVar *httpClientReqVar);

void serverResponseSetHeader(JsVar *parent, JsVar *name, JsVar *value); // for HTTP
void serverResponseWriteHead(JsVar *httpServerResponseVar, int statusCode, JsVar *headers); // for HTTP
void serverResponseWrite(JsVar *httpServerResponseVar, JsVar *data);
void serverResponseEnd(JsVar *httpServerResponseVar);

#endif // SOCKETSERVER_H
