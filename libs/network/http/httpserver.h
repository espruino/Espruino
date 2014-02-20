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


#if defined(USE_CC3000)
 #include "spi.h"
 #include "socket.h"
 typedef int SOCKET;
#elif defined(USE_WIZNET)
 #include "Ethernet/socket.h"
 typedef struct sockaddr_in sockaddr_in;
#else
 #ifdef WIN32
  #include <winsock.h>
 #else
  #include <sys/socket.h>
  #include <arpa/inet.h>
  #include <netdb.h>
  #include <netinet/in.h>
  #include <unistd.h>
  #include <fcntl.h>
  #include <stdio.h>
  #include <resolv.h>
  typedef struct sockaddr_in sockaddr_in;
  typedef int SOCKET;
 #endif
#endif

// -----------------------------
void httpInit();
void httpKill();
void httpIdle();
// -----------------------------
JsVar *httpServerNew(JsVar *callback);
void httpServerListen(JsVar *httpServerVar, int port);

JsVar *httpClientRequestNew(JsVar *options, JsVar *callback);
void httpClientRequestWrite(JsVar *httpClientReqVar, JsVar *data);
void httpClientRequestEnd(JsVar *httpClientReqVar);

void httpServerResponseWriteHead(JsVar *httpServerResponseVar, int statusCode, JsVar *headers);
void httpServerResponseData(JsVar *httpServerResponseVar, JsVar *data);
void httpServerResponseEnd(JsVar *httpServerResponseVar);
