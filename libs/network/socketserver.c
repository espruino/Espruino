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
#include "socketserver.h"
#include "jsparse.h"
#include "jsinteractive.h"
#include "jshardware.h"
#include "jswrap_stream.h"

#define HTTP_NAME_SOCKETTYPE "type" // normal socket or HTTP
#define HTTP_NAME_PORT "port"
#define HTTP_NAME_SOCKET "sckt"
#define HTTP_NAME_HAD_HEADERS "hdrs"
#define HTTP_NAME_RECEIVE_DATA "dRcv"
#define HTTP_NAME_SEND_DATA "dSnd"
#define HTTP_NAME_RESPONSE_VAR "res"
#define HTTP_NAME_OPTIONS_VAR "opt"
#define HTTP_NAME_SERVER_VAR "svr"
#define HTTP_NAME_CODE "code"
#define HTTP_NAME_HEADERS "hdr"
#define HTTP_NAME_CLOSENOW "closeNow"
#define HTTP_NAME_CLOSE "close" // close after sending
#define HTTP_NAME_ON_CONNECT "#onconnect"
#define HTTP_NAME_ON_CLOSE "#onclose"
#define HTTP_NAME_ON_END "#onend"

#define HTTP_ARRAY_HTTP_CLIENT_CONNECTIONS "HttpCC"
#define HTTP_ARRAY_HTTP_SERVERS "HttpS"
#define HTTP_ARRAY_HTTP_SERVER_CONNECTIONS "HttpSC"

// -----------------------------

static void httpAppendHeaders(JsVar *string, JsVar *headerObject) {
  // append headers
  JsvObjectIterator it;
  jsvObjectIteratorNew(&it, headerObject);
  while (jsvObjectIteratorHasValue(&it)) {
    JsVar *k = jsvAsString(jsvObjectIteratorGetKey(&it), true);
    JsVar *v = jsvAsString(jsvObjectIteratorGetValue(&it), true);
    jsvAppendStringVarComplete(string, k);
    jsvAppendString(string, ": ");
    jsvAppendStringVarComplete(string, v);
    jsvAppendString(string, "\r\n");
    jsvUnLock2(k, v);
    jsvObjectIteratorNext(&it);
  }
  jsvObjectIteratorFree(&it);

  // free headers
}

// httpParseHeaders(&receiveData, reqVar, true) // server
// httpParseHeaders(&receiveData, resVar, false) // client
bool httpParseHeaders(JsVar **receiveData, JsVar *objectForData, bool isServer) {
  // find /r/n/r/n
  int newlineIdx = 0;
  int strIdx = 0;
  int headerEnd = -1;
  JsvStringIterator it;
  jsvStringIteratorNew(&it, *receiveData, 0);
  while (jsvStringIteratorHasChar(&it)) {
    char ch = jsvStringIteratorGetChar(&it);
    if (ch == '\r') {
      if (newlineIdx==0) newlineIdx=1;
      else if (newlineIdx==2) newlineIdx=3;
    } else if (ch == '\n') {
      if (newlineIdx==1) newlineIdx=2;
      else if (newlineIdx==3) {
        headerEnd = strIdx+1;
      }
    } else newlineIdx=0;
    jsvStringIteratorNext(&it);
    strIdx++;
  }
  jsvStringIteratorFree(&it);
  // skip if we have no header
  if (headerEnd<0) return false;
  // Now parse the header
  JsVar *vHeaders = jsvNewWithFlags(JSV_OBJECT);
  if (!vHeaders) return true;
  jsvUnLock(jsvAddNamedChild(objectForData, vHeaders, "headers"));
  strIdx = 0;
  int firstSpace = -1;
  int secondSpace = -1;
  int firstEOL = -1;
  int lineNumber = 0;
  int lastLineStart = 0;
  int colonPos = 0;
  //jsiConsolePrintStringVar(receiveData);
  jsvStringIteratorNew(&it, *receiveData, 0);
    while (jsvStringIteratorHasChar(&it)) {
      char ch = jsvStringIteratorGetChar(&it);
      if (ch==' ' || ch=='\r') {
        if (firstSpace<0) firstSpace = strIdx;
        else if (secondSpace<0) secondSpace = strIdx;
      }
      if (ch == ':' && colonPos<0) colonPos = strIdx;
      if (ch == '\r') {
        if (firstEOL<0) firstEOL=strIdx;
        if (lineNumber>0 && colonPos>lastLineStart && lastLineStart<strIdx) {
          JsVar *hVal = jsvNewFromEmptyString();
          if (hVal)
            jsvAppendStringVar(hVal, *receiveData, (size_t)colonPos+2, (size_t)(strIdx-(colonPos+2)));
          JsVar *hKey = jsvNewFromEmptyString();
          if (hKey) {
            jsvMakeIntoVariableName(hKey, hVal);
            jsvAppendStringVar(hKey, *receiveData, (size_t)lastLineStart, (size_t)(colonPos-lastLineStart));
            jsvAddName(vHeaders, hKey);
            jsvUnLock(hKey);
          }
          jsvUnLock(hVal);
        }
        lineNumber++;
        colonPos=-1;
      }
      if (ch == '\r' || ch == '\n') {
        lastLineStart = strIdx+1;
      }

      jsvStringIteratorNext(&it);
      strIdx++;
    }
    jsvStringIteratorFree(&it);
  jsvUnLock(vHeaders);
  // try and pull out methods/etc
  if (isServer) {
    jsvObjectSetChildAndUnLock(objectForData, "method", jsvNewFromStringVar(*receiveData, 0, (size_t)firstSpace));
    jsvObjectSetChildAndUnLock(objectForData, "url", jsvNewFromStringVar(*receiveData, (size_t)(firstSpace+1), (size_t)(secondSpace-(firstSpace+1))));
  } else {
    jsvObjectSetChildAndUnLock(objectForData, "httpVersion", jsvNewFromStringVar(*receiveData, 5, (size_t)firstSpace-5));
    jsvObjectSetChildAndUnLock(objectForData, "statusCode", jsvNewFromStringVar(*receiveData, (size_t)(firstSpace+1), (size_t)(secondSpace-(firstSpace+1))));
    jsvObjectSetChildAndUnLock(objectForData, "statusMessage", jsvNewFromStringVar(*receiveData, (size_t)(secondSpace+1), (size_t)(firstEOL-(secondSpace+1))));
  }
  // strip out the header
  JsVar *afterHeaders = jsvNewFromStringVar(*receiveData, (size_t)headerEnd, JSVAPPENDSTRINGVAR_MAXLENGTH);
  jsvUnLock(*receiveData);
  *receiveData = afterHeaders;
  return true;
}

size_t httpStringGet(JsVar *v, char *str, size_t len) {
  size_t l = len;
  JsvStringIterator it;
  jsvStringIteratorNew(&it, v, 0);
  while (jsvStringIteratorHasChar(&it)) {
    if (l--==0) {
      jsvStringIteratorFree(&it);
      return len;
    }
    *(str++) = jsvStringIteratorGetChar(&it);
    jsvStringIteratorNext(&it);
  }
  jsvStringIteratorFree(&it);
  return len-l;
}

// -----------------------------

static JsVar *socketGetArray(const char *name, bool create) {
  return jsvObjectGetChild(execInfo.hiddenRoot, name, create?JSV_ARRAY:0);
}

static NO_INLINE SocketType socketGetType(JsVar *var) {
  return jsvGetIntegerAndUnLock(jsvObjectGetChild(var, HTTP_NAME_SOCKETTYPE, 0));
}

static NO_INLINE void socketSetType(JsVar *var, SocketType socketType) {
  jsvObjectSetChildAndUnLock(var, HTTP_NAME_SOCKETTYPE, jsvNewFromInteger((JsVarInt)socketType));
}

void _socketConnectionKill(JsNetwork *net, JsVar *connection) {
  if (!net || networkState != NETWORKSTATE_ONLINE) return;
  int sckt = (int)jsvGetIntegerAndUnLock(jsvObjectGetChild(connection,HTTP_NAME_SOCKET,0))-1; // so -1 if undefined
  if (sckt>=0) {
    net->closesocket(net, sckt);
    jsvObjectSetChild(connection,HTTP_NAME_SOCKET,0);
  }
}

// -----------------------------

NO_INLINE static void _socketCloseAllConnectionsFor(JsNetwork *net, char *name) {
  JsVar *arr = socketGetArray(name, false);
  if (!arr) return;
  JsvObjectIterator it;
  jsvObjectIteratorNew(&it, arr);
  while (jsvObjectIteratorHasValue(&it)) {
    JsVar *connection = jsvObjectIteratorGetValue(&it);
    _socketConnectionKill(net, connection);
    jsvUnLock(connection);
    jsvObjectIteratorNext(&it);
  }
  jsvObjectIteratorFree(&it);
  jsvRemoveAllChildren(arr);
  jsvUnLock(arr);
}

NO_INLINE static void _socketCloseAllConnections(JsNetwork *net) {
  // shut down connections
  _socketCloseAllConnectionsFor(net, HTTP_ARRAY_HTTP_SERVER_CONNECTIONS);
  _socketCloseAllConnectionsFor(net, HTTP_ARRAY_HTTP_CLIENT_CONNECTIONS);
  _socketCloseAllConnectionsFor(net, HTTP_ARRAY_HTTP_SERVERS);
}

bool socketSendData(JsNetwork *net, JsVar *connection, int sckt, JsVar **sendData) {
  char buf[64];

  int a=1;
  if (!jsvIsEmptyString(*sendData)) {
    size_t bufLen = httpStringGet(*sendData, buf, sizeof(buf));
    a = net->send(net, sckt, buf, bufLen);
    // Now cut what we managed to send off the beginning of sendData
    if (a>0) {
      JsVar *newSendData = 0;
      if (a < (int)jsvGetStringLength(*sendData)) {
        // we didn't send all of it... cut out what we did send
        newSendData = jsvNewFromStringVar(*sendData, (size_t)a, JSVAPPENDSTRINGVAR_MAXLENGTH);
      } else {
        // we sent all of it! Issue a drain event
        jsiQueueObjectCallbacks(connection, "#ondrain", &connection, 1);
      }
      jsvUnLock(*sendData);
      *sendData = newSendData;
    }
  }
  if (a<0) { // could just be busy which is ok
    jsError("Socket error %d while sending", a);
    return false;
  }
  return true;
}

// -----------------------------

void socketInit() {
#ifdef WIN32
  // Init winsock 1.1
  WORD sockVersion;
  WSADATA wsaData;
  sockVersion = MAKEWORD(1, 1);
  WSAStartup(sockVersion, &wsaData);
#endif
}

void socketKill(JsNetwork *net) {
  _socketCloseAllConnections(net);
#ifdef WIN32
   // Shutdown Winsock
   WSACleanup();
#endif
}

// -----------------------------

bool socketServerConnectionsIdle(JsNetwork *net) {
  char buf[64];

  JsVar *arr = socketGetArray(HTTP_ARRAY_HTTP_SERVER_CONNECTIONS,false);
  if (!arr) return false;

  bool hadSockets = false;
  JsvObjectIterator it;
  jsvObjectIteratorNew(&it, arr);
  while (jsvObjectIteratorHasValue(&it)) {
    hadSockets = true;
    // Get connection, socket, and socket type
    // For normal sockets, socket==connection, but for HTTP we split it into a request and a response
    JsVar *connection = jsvObjectIteratorGetValue(&it);
    SocketType socketType = socketGetType(connection);
    JsVar *socket = (socketType==ST_HTTP) ? jsvObjectGetChild(connection,HTTP_NAME_RESPONSE_VAR,0) : jsvLockAgain(connection);

    int sckt = (int)jsvGetIntegerAndUnLock(jsvObjectGetChild(connection,HTTP_NAME_SOCKET,0))-1; // so -1 if undefined
    bool closeConnectionNow = jsvGetBoolAndUnLock(jsvObjectGetChild(connection, HTTP_NAME_CLOSENOW, false));

    if (!closeConnectionNow) {
      int num = net->recv(net, sckt, buf,sizeof(buf));
      if (num<0) {
        // we probably disconnected so just get rid of this
        closeConnectionNow = true;
      } else {
        // add it to our request string
        if (num>0) {
          JsVar *receiveData = jsvObjectGetChild(connection,HTTP_NAME_RECEIVE_DATA,0);
          JsVar *oldReceiveData = receiveData;
          if (!receiveData) receiveData = jsvNewFromEmptyString();
          if (receiveData) {
            jsvAppendStringBuf(receiveData, buf, (size_t)num);
            bool hadHeaders = jsvGetBoolAndUnLock(jsvObjectGetChild(connection,HTTP_NAME_HAD_HEADERS,0));
            if (!hadHeaders && httpParseHeaders(&receiveData, connection, true)) {
              hadHeaders = true;
              jsvObjectSetChildAndUnLock(connection, HTTP_NAME_HAD_HEADERS, jsvNewFromBool(hadHeaders));
              JsVar *server = jsvObjectGetChild(connection,HTTP_NAME_SERVER_VAR,0);
              JsVar *args[2] = { connection, socket };
              jsiQueueObjectCallbacks(server, HTTP_NAME_ON_CONNECT, args, (socketType==ST_HTTP) ? 2 : 1);
              jsvUnLock(server);
            }
            if (hadHeaders && !jsvIsEmptyString(receiveData)) {
              // execute 'data' callback or save data
              if (jswrap_stream_pushData(connection, receiveData, false)) {
                // clear received data
                jsvUnLock(receiveData);
                receiveData = 0;
              }
            }
            // if received data changed, update it
            if (receiveData != oldReceiveData)
              jsvObjectSetChild(connection,HTTP_NAME_RECEIVE_DATA,receiveData);
            jsvUnLock(receiveData);
          }
        }
      }

      // send data if possible
      JsVar *sendData = jsvObjectGetChild(socket,HTTP_NAME_SEND_DATA,0);
      if (sendData) {
          if (!socketSendData(net, socket, sckt, &sendData))
            closeConnectionNow = true;
        jsvObjectSetChild(socket, HTTP_NAME_SEND_DATA, sendData); // socketSendData prob updated sendData
      }
      // only close if we want to close, have no data to send, and aren't receiving data
      if (jsvGetBoolAndUnLock(jsvObjectGetChild(socket,HTTP_NAME_CLOSE,0)) && !sendData && num<=0)
        closeConnectionNow = true;
      jsvUnLock(sendData);
    }
    if (closeConnectionNow) {
      // send out any data that we were POSTed
      JsVar *receiveData = jsvObjectGetChild(connection,HTTP_NAME_RECEIVE_DATA,0);
      bool hadHeaders = jsvGetBoolAndUnLock(jsvObjectGetChild(connection,HTTP_NAME_HAD_HEADERS,0));
      if (hadHeaders && !jsvIsEmptyString(receiveData)) {
        // execute 'data' callback or save data
        jswrap_stream_pushData(connection, receiveData, true);
      }
      jsvUnLock(receiveData);
      // fire the close listeners
      jsiQueueObjectCallbacks(connection, HTTP_NAME_ON_CLOSE, &connection, 1);
      jsiQueueObjectCallbacks(socket, HTTP_NAME_ON_CLOSE, &socket, 1);

      _socketConnectionKill(net, connection);
      JsVar *connectionName = jsvObjectIteratorGetKey(&it);
      jsvObjectIteratorNext(&it);
      jsvRemoveChild(arr, connectionName);
      jsvUnLock(connectionName);
    } else
      jsvObjectIteratorNext(&it);
    jsvUnLock2(connection, socket);
  }
  jsvObjectIteratorFree(&it);
  jsvUnLock(arr);

  return hadSockets;
}


void socketClientPushReceiveData(JsVar *connection, JsVar *socket, JsVar **receiveData) {
  if (*receiveData) {
    if (jsvIsEmptyString(*receiveData) ||
        jswrap_stream_pushData(socket, *receiveData, false)) {
      // clear - because we have issued a callback
      jsvObjectSetChild(connection,HTTP_NAME_RECEIVE_DATA,0);
      jsvUnLock(*receiveData);
      *receiveData = 0;
    }
  }
}

bool socketClientConnectionsIdle(JsNetwork *net) {
  char buf[64];

  JsVar *arr = socketGetArray(HTTP_ARRAY_HTTP_CLIENT_CONNECTIONS,false);
  if (!arr) return false;

  bool hadSockets = false;
  JsvObjectIterator it;
  jsvObjectIteratorNew(&it, arr);
  while (jsvObjectIteratorHasValue(&it)) {
    hadSockets = true;
    // Get connection, socket, and socket type
    // For normal sockets, socket==connection, but for HTTP we split it into a request and a response
    JsVar *connection = jsvObjectIteratorGetValue(&it);
    SocketType socketType = socketGetType(connection);
    JsVar *socket = (socketType==ST_HTTP) ? jsvObjectGetChild(connection,HTTP_NAME_RESPONSE_VAR,0) : jsvLockAgain(connection);

    bool closeConnectionNow = jsvGetBoolAndUnLock(jsvObjectGetChild(connection, HTTP_NAME_CLOSENOW, false));
    int sckt = (int)jsvGetIntegerAndUnLock(jsvObjectGetChild(connection,HTTP_NAME_SOCKET,0))-1; // so -1 if undefined
    if (sckt<0) closeConnectionNow = true;
    bool hadHeaders = true;
    if (socketType==ST_HTTP)
      hadHeaders = jsvGetBoolAndUnLock(jsvObjectGetChild(connection,HTTP_NAME_HAD_HEADERS,0));
    JsVar *receiveData = jsvObjectGetChild(connection,HTTP_NAME_RECEIVE_DATA,0);

    /* We do this up here because we want to wait until we have been once
     * around the idle loop (=callbacks have been executed) before we run this */
    if (hadHeaders)
      socketClientPushReceiveData(connection, socket, &receiveData);

    if (!closeConnectionNow) {
      JsVar *sendData = jsvObjectGetChild(connection,HTTP_NAME_SEND_DATA,0);
      // send data if possible
      if (sendData) {
        bool b = socketSendData(net, connection, sckt, &sendData);
        if (!b)
          closeConnectionNow = true;
        jsvObjectSetChild(connection, HTTP_NAME_SEND_DATA, sendData); // _http_send prob updated sendData
      } else {
        if (jsvGetBoolAndUnLock(jsvObjectGetChild(connection, HTTP_NAME_CLOSE, false)))
          closeConnectionNow = true;
      }
      // Now read data if possible (and we have space for it)
      if (!receiveData || !hadHeaders) {
        int num = net->recv(net, sckt, buf, sizeof(buf));
        if (num<0) {
          // we probably disconnected so just get rid of this
          closeConnectionNow = true;
        } else {
          // add it to our request string
          if (num>0) {
            if (!receiveData) {
              receiveData = jsvNewFromEmptyString();
              jsvObjectSetChild(connection, HTTP_NAME_RECEIVE_DATA, receiveData);
            }
            if (receiveData) { // could be out of memory
              jsvAppendStringBuf(receiveData, buf, (size_t)num);
              if (socketType==ST_HTTP && !hadHeaders) {
                JsVar *resVar = jsvObjectGetChild(connection,HTTP_NAME_RESPONSE_VAR,0);
                if (httpParseHeaders(&receiveData, resVar, false)) {
                  hadHeaders = true;
                  jsvObjectSetChildAndUnLock(connection, HTTP_NAME_HAD_HEADERS, jsvNewFromBool(hadHeaders));
                  jsiQueueObjectCallbacks(connection, HTTP_NAME_ON_CONNECT, &resVar, 1);
                }
                jsvUnLock(resVar);
                jsvObjectSetChild(connection, HTTP_NAME_RECEIVE_DATA, receiveData);
              }
            }
          }
        }
      }
      jsvUnLock(sendData);
    }

    bool closed = false;
    if (closeConnectionNow) {
      socketClientPushReceiveData(connection, socket, &receiveData);
      if (!receiveData) {
        if (socketType != ST_HTTP)
          jsiQueueObjectCallbacks(socket, HTTP_NAME_ON_END, &socket, 1);
        jsiQueueObjectCallbacks(socket, HTTP_NAME_ON_CLOSE, &socket, 1);

        _socketConnectionKill(net, connection);
        JsVar *connectionName = jsvObjectIteratorGetKey(&it);
        jsvObjectIteratorNext(&it);
        jsvRemoveChild(arr, connectionName);
        jsvUnLock(connectionName);
        closed = true;
      }
    }

    if (!closed) {
      jsvObjectIteratorNext(&it);
    }

    jsvUnLock3(receiveData, connection, socket);
  }
  jsvUnLock(arr);

  return hadSockets;
}


bool socketIdle(JsNetwork *net) {
  if (networkState != NETWORKSTATE_ONLINE) {
    // clear all clients and servers
    _socketCloseAllConnections(net);
    return false;
  }
  bool hadSockets = false;
  JsVar *arr = socketGetArray(HTTP_ARRAY_HTTP_SERVERS,false);
  if (arr) {
    JsvObjectIterator it;
    jsvObjectIteratorNew(&it, arr);
    while (jsvObjectIteratorHasValue(&it)) {
      hadSockets = true;

      JsVar *server = jsvObjectIteratorGetValue(&it);
      int sckt = (int)jsvGetIntegerAndUnLock(jsvObjectGetChild(server,HTTP_NAME_SOCKET,0))-1; // so -1 if undefined

      int theClient = net->accept(net, sckt);
      if (theClient >= 0) {
        SocketType socketType = socketGetType(server);
        if (socketType == ST_HTTP) {
          JsVar *req = jspNewObject(0, "httpSRq");
          JsVar *res = jspNewObject(0, "httpSRs");
          if (res && req) { // out of memory?
            socketSetType(req, ST_HTTP);
            JsVar *arr = socketGetArray(HTTP_ARRAY_HTTP_SERVER_CONNECTIONS, true);
            if (arr) {
              jsvArrayPush(arr, req);
              jsvUnLock(arr);
            }
            jsvObjectSetChild(req, HTTP_NAME_RESPONSE_VAR, res);
            jsvObjectSetChild(req, HTTP_NAME_SERVER_VAR, server);
            jsvObjectSetChildAndUnLock(req, HTTP_NAME_SOCKET, jsvNewFromInteger(theClient+1));
            // on response
            jsvObjectSetChildAndUnLock(res, HTTP_NAME_CODE, jsvNewFromInteger(200));
            jsvObjectSetChildAndUnLock(res, HTTP_NAME_HEADERS, jsvNewWithFlags(JSV_OBJECT));
          }
          jsvUnLock2(req, res);
        } else {
          // Normal sockets
          JsVar *sock = jspNewObject(0, "Socket");
          if (sock) { // out of memory?
            socketSetType(sock, ST_NORMAL);
            JsVar *arr = socketGetArray(HTTP_ARRAY_HTTP_CLIENT_CONNECTIONS, true);
            if (arr) {
              jsvArrayPush(arr, sock);
              jsvUnLock(arr);
            }
            jsvObjectSetChildAndUnLock(sock, HTTP_NAME_SOCKET, jsvNewFromInteger(theClient+1));
            jsiQueueObjectCallbacks(server, HTTP_NAME_ON_CONNECT, &sock, 1);
            jsvUnLock(sock);
          }
        }
      }

      jsvUnLock(server);
      jsvObjectIteratorNext(&it);
    }
    jsvObjectIteratorFree(&it);
    jsvUnLock(arr);
  }

  if (socketServerConnectionsIdle(net)) hadSockets = true;
  if (socketClientConnectionsIdle(net)) hadSockets = true;
  net->checkError(net);
  return hadSockets;
}

// -----------------------------

JsVar *serverNew(SocketType socketType, JsVar *callback) {
  JsVar *server = jspNewObject(0, (socketType==ST_HTTP) ? "httpSrv" : "Server");
  if (!server) return 0; // out of memory
  socketSetType(server, socketType);
  jsvObjectSetChild(server, HTTP_NAME_ON_CONNECT, callback); // no unlock needed
  return server;
}

void serverListen(JsNetwork *net, JsVar *server, int port) {
  JsVar *arr = socketGetArray(HTTP_ARRAY_HTTP_SERVERS, true);
  if (!arr) return; // out of memory

  jsvObjectSetChildAndUnLock(server, HTTP_NAME_PORT, jsvNewFromInteger(port));

  int sckt = net->createsocket(net, 0/*server*/, (unsigned short)port);
  if (sckt<0) {
    jsError("Unable to create socket\n");
    jsvObjectSetChildAndUnLock(server, HTTP_NAME_CLOSENOW, jsvNewFromBool(true));
  } else {
    jsvObjectSetChildAndUnLock(server, HTTP_NAME_SOCKET, jsvNewFromInteger(sckt+1));
    // add to list of servers
    jsvArrayPush(arr, server);
  }
  jsvUnLock(arr);
}

void serverClose(JsNetwork *net, JsVar *server) {
  JsVar *arr = socketGetArray(HTTP_ARRAY_HTTP_SERVERS,false);
  if (arr) {
    // close socket
    _socketConnectionKill(net, server);
    // remove from array
    JsVar *idx = jsvGetArrayIndexOf(arr, server, true);
    if (idx) {
      jsvRemoveChild(arr, idx);
      jsvUnLock(idx);
    } else
      jsWarn("Server not found!");
    jsvUnLock(arr);
  }
}


JsVar *clientRequestNew(SocketType socketType, JsVar *options, JsVar *callback) {
  JsVar *arr = socketGetArray(HTTP_ARRAY_HTTP_CLIENT_CONNECTIONS,true);
  if (!arr) return 0;
  JsVar *req, *res = 0;
  if (socketType==ST_HTTP) {
    res = jspNewObject(0, "httpCRs");
    if (!res) { jsvUnLock(arr); return 0; } // out of memory?
    req = jspNewObject(0, "httpCRq");
  } else {
    req = jspNewObject(0, "Socket");
  }
  if (req) { // out of memory?
   socketSetType(req, socketType);
   jsvUnLock(jsvAddNamedChild(req, callback, HTTP_NAME_ON_CONNECT));

   jsvArrayPush(arr, req);
   if (res)
     jsvObjectSetChild(req, HTTP_NAME_RESPONSE_VAR, res);
   jsvObjectSetChild(req, HTTP_NAME_OPTIONS_VAR, options);
  }
  jsvUnLock2(res, arr);
  return req;
}

void clientRequestWrite(JsVar *httpClientReqVar, JsVar *data) {
  SocketType socketType = socketGetType(httpClientReqVar);
  // Append data to sendData
  JsVar *sendData = jsvObjectGetChild(httpClientReqVar, HTTP_NAME_SEND_DATA, 0);
  if (!sendData) {
    JsVar *options = 0;
    // Only append a header if we're doing HTTP
    if (socketType == ST_HTTP)
      options = jsvObjectGetChild(httpClientReqVar, HTTP_NAME_OPTIONS_VAR, 0);
    if (options) {
      JsVar *method = jsvObjectGetChild(options, "method", 0);
      JsVar *path = jsvObjectGetChild(options, "path", 0);
      sendData = jsvVarPrintf("%v %v HTTP/1.0\r\nUser-Agent: Espruino "JS_VERSION"\r\nConnection: close\r\n", method, path);
      jsvUnLock2(method, path);
      JsVar *headers = jsvObjectGetChild(options, "headers", 0);
      bool hasHostHeader = false;
      if (jsvIsObject(headers)) {
        JsVar *hostHeader = jsvObjectGetChild(headers, "Host", 0);
        hasHostHeader = hostHeader!=0;
        jsvUnLock(hostHeader);
        httpAppendHeaders(sendData, headers);
      }
      jsvUnLock(headers);
      if (!hasHostHeader) {
        JsVar *host = jsvObjectGetChild(options, "host", 0);
        int port = (int)jsvGetIntegerAndUnLock(jsvObjectGetChild(options, "port", 0));
        if (port>0 && port!=80)
          jsvAppendPrintf(sendData, "Host: %v:%d\r\n", host, port);
        else
          jsvAppendPrintf(sendData, "Host: %v\r\n", host);
        jsvUnLock(host);
      }
      // finally add ending newline
      jsvAppendString(sendData, "\r\n");
    } else {
      sendData = jsvNewFromString("");
    }
    jsvObjectSetChild(httpClientReqVar, HTTP_NAME_SEND_DATA, sendData);
    jsvUnLock(options);
  }
  if (data && sendData) {
    JsVar *s = jsvAsString(data, false);
    if (s) jsvAppendStringVarComplete(sendData,s);
    jsvUnLock(s);
  }
  jsvUnLock(sendData);
}

// Connect this connection/socket
void clientRequestConnect(JsNetwork *net, JsVar *httpClientReqVar) {
  SocketType socketType = socketGetType(httpClientReqVar);
  clientRequestWrite(httpClientReqVar, 0); // force sendData to be made

  JsVar *options = jsvObjectGetChild(httpClientReqVar, HTTP_NAME_OPTIONS_VAR, false);
  unsigned short port = (unsigned short)jsvGetIntegerAndUnLock(jsvObjectGetChild(options, "port", 0));
  if (port==0) port=80;

  char hostName[128];
  JsVar *hostNameVar = jsvObjectGetChild(options, "host", 0);
  if (jsvIsUndefined(hostNameVar))
    strncpy(hostName, "localhost", sizeof(hostName));
  else
    jsvGetString(hostNameVar, hostName, sizeof(hostName));
  jsvUnLock(hostNameVar);

  uint32_t host_addr = 0;
  networkGetHostByName(net, hostName, &host_addr);

  if(!host_addr) {
    jsError("Unable to locate host");
    jsvObjectSetChildAndUnLock(httpClientReqVar, HTTP_NAME_CLOSENOW, jsvNewFromBool(true));
    jsvUnLock(options);
    net->checkError(net);
    return;
  }

  int sckt =  net->createsocket(net, host_addr, port);
  if (sckt<0) {
    jsError("Unable to create socket\n");
    jsvObjectSetChildAndUnLock(httpClientReqVar, HTTP_NAME_CLOSENOW, jsvNewFromBool(true));
  } else {
    jsvObjectSetChildAndUnLock(httpClientReqVar, HTTP_NAME_SOCKET, jsvNewFromInteger(sckt+1));

    // For HTTP we get the connection callback when we've got a header back
    // Otherwise we just call back on success
    if (socketType != ST_HTTP) {
      jsiQueueObjectCallbacks(httpClientReqVar, HTTP_NAME_ON_CONNECT, &httpClientReqVar, 1);
    }
  }

  jsvUnLock(options);

  net->checkError(net);
}

// 'end' this connection
void clientRequestEnd(JsNetwork *net, JsVar *httpClientReqVar) {
  SocketType socketType = socketGetType(httpClientReqVar);
  if (socketType == ST_HTTP) {
    // on HTTP, this actually means we connect
    clientRequestConnect(net, httpClientReqVar);
  } else {
    // on normal sockets, we actually request close after all data sent
    jsvObjectSetChildAndUnLock(httpClientReqVar, HTTP_NAME_CLOSE, jsvNewFromBool(true));
    // if we never sent any data, make sure we close 'now'
    JsVar *sendData = jsvObjectGetChild(httpClientReqVar, HTTP_NAME_SEND_DATA, 0);
    if (!sendData || jsvIsEmptyString(sendData))
      jsvObjectSetChildAndUnLock(httpClientReqVar, HTTP_NAME_CLOSENOW, jsvNewFromBool(true));
    jsvUnLock(sendData);
  }
}


void serverResponseWriteHead(JsVar *httpServerResponseVar, int statusCode, JsVar *headers) {
  if (!jsvIsUndefined(headers) && !jsvIsObject(headers)) {
    jsError("Headers sent to writeHead should be an object");
    return;
  }

  jsvObjectSetChildAndUnLock(httpServerResponseVar, HTTP_NAME_CODE, jsvNewFromInteger(statusCode));
  JsVar *sendHeaders = jsvObjectGetChild(httpServerResponseVar, HTTP_NAME_HEADERS, 0);
  if (sendHeaders) {
    if (!jsvIsUndefined(headers)) {
      jsvObjectSetChild(httpServerResponseVar, HTTP_NAME_HEADERS, headers);
    }
    jsvUnLock(sendHeaders);
  } else {
    // headers are set to 0 when they are sent
    jsError("Headers have already been sent");
  }
}


void serverResponseWrite(JsVar *httpServerResponseVar, JsVar *data) {
  // Append data to sendData
  JsVar *sendData = jsvObjectGetChild(httpServerResponseVar, HTTP_NAME_SEND_DATA, 0);
  if (!sendData) {
    // no sendData, so no headers - add them!
    JsVar *sendHeaders = jsvObjectGetChild(httpServerResponseVar, HTTP_NAME_HEADERS, 0);
    if (sendHeaders) {
      sendData = jsvVarPrintf("HTTP/1.0 %d OK\r\nServer: Espruino "JS_VERSION"\r\n", jsvGetIntegerAndUnLock(jsvObjectGetChild(httpServerResponseVar, HTTP_NAME_CODE, 0)));
      httpAppendHeaders(sendData, sendHeaders);
      jsvObjectSetChild(httpServerResponseVar, HTTP_NAME_HEADERS, 0);
      jsvUnLock(sendHeaders);
      // finally add ending newline
      jsvAppendString(sendData, "\r\n");
    } else if (!jsvIsUndefined(data)) {
      // we have already sent headers, but want to send more
      sendData = jsvNewFromEmptyString();
    }
    jsvObjectSetChild(httpServerResponseVar, HTTP_NAME_SEND_DATA, sendData);
  }
  if (sendData && !jsvIsUndefined(data)) {
    JsVar *s = jsvAsString(data, false);
    if (s) jsvAppendStringVarComplete(sendData,s);
    jsvUnLock(s);
  }
  jsvUnLock(sendData);
}

void serverResponseEnd(JsVar *httpServerResponseVar) {
  serverResponseWrite(httpServerResponseVar, 0); // force connection->sendData to be created even if data not called
  jsvObjectSetChildAndUnLock(httpServerResponseVar, HTTP_NAME_CLOSE, jsvNewFromBool(true));
}

