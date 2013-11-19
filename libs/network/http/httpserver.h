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

#ifdef USE_CC3000
 #include "spi.h"
 #include "socket.h"
 typedef int SOCKET;
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
  typedef int SOCKET;
  typedef struct sockaddr_in sockaddr_in;
 #endif
#endif

 typedef struct HttpServer {
   struct HttpServer *prev;
   struct HttpServer *next;
   JsVar *var;
   SOCKET listeningSocket;
 } HttpServer;

 typedef struct HttpServerConnection {
   struct HttpServerConnection *prev;
   struct HttpServerConnection *next;
   JsVar *var; // server var
   JsVar *resVar; // response
   JsVar *reqVar; // request
   SOCKET socket;
   int sendCode; // http response code
   JsVar *sendHeaders; // object representing headers to send
   JsVar *sendData; // data to send
   JsVar *receiveData; // data that has been received
   bool close; // close connection after all data sent
   bool closeNow; // close connection right now!
   bool hadHeaders; // have we already parsed the headers?
 } HttpServerConnection;

#define HTTP_CLIENT_MAX_HOST_NAME 64

 typedef struct HttpClientConnection {
   struct HttpClientConnection *prev;
   struct HttpClientConnection *next;
   JsVar *resVar; // response
   JsVar *reqVar; // request
   SOCKET socket;
   JsVar *sendData; // data to send
   JsVar *receiveData; // data that has been received
   bool closeNow; // close connection right now!
   bool hadHeaders; // have we already parsed the headers?

   JsVar *options;
 } HttpClientConnection;

// -----------------------------
void httpServerInit();
void httpServerKill();
void httpServerIdle();
// -----------------------------
HttpServer *httpFindServer(JsVar *httpServerVar);
HttpServerConnection *httpFindServerConnectionFromResponse(JsVar *httpServerResponseVar);
HttpClientConnection *httpFindHttpClientConnectionFromRequest(JsVar *httpClientRequestVar);
// -----------------------------
JsVar *httpServerNew(JsVar *callback);
void httpServerListen(JsVar *httpServerVar, int port);

JsVar *httpClientRequestNew(JsVar *options, JsVar *callback);
void httpClientRequestWrite(JsVar *httpClientReqVar, JsVar *data);
void httpClientRequestEnd(JsVar *httpClientReqVar);

void httpServerResponseWriteHead(JsVar *httpServerResponseVar, int statusCode, JsVar *headers);
void httpServerResponseData(JsVar *httpServerResponseVar, JsVar *data);
void httpServerResponseEnd(JsVar *httpServerResponseVar);
