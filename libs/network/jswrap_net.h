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
 * Contains JavaScript Net (socket) functions
 * ----------------------------------------------------------------------------
 */
#include "jsvar.h"
#include "socketserver.h"

#define DGRAM_ON_BIND_NAME          JS_EVENT_PREFIX"bind"

bool jswrap_net_idle();
void jswrap_net_init();
void jswrap_net_kill();

JsVar *jswrap_url_parse(JsVar *url, bool parseQuery);

JsVar *jswrap_net_createServer(JsVar *callback);
JsVar *jswrap_net_connect(JsVar *options, JsVar *callback, SocketType socketType);

JsVar *jswrap_net_server_listen(JsVar *parent, int port, SocketType socketType);
void jswrap_net_server_close(JsVar *parent);

bool jswrap_net_socket_write(JsVar *parent, JsVar *data);
void jswrap_net_socket_end(JsVar *parent, JsVar *data);

JsVar *jswrap_dgram_createSocket(JsVar *type, JsVar *callback);
JsVar *jswrap_dgramSocket_bind(JsVar *parent, unsigned short port, JsVar *callback);
void jswrap_dgram_close(JsVar *parent);
void jswrap_dgram_addMembership(JsVar *parent, JsVar *group, JsVar *ip);
void jswrap_dgram_socket_send(JsVar *parent, JsVar *buffer, JsVar *offset, JsVar *length, JsVar *args);
