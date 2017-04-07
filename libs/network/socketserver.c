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
#include "socketerrors.h"
#include "jsparse.h"
#include "jsinteractive.h"
#include "jshardware.h"
#include "jswrap_stream.h"

#define HTTP_NAME_SOCKETTYPE "type" // normal socket or HTTP
#define HTTP_NAME_PORT "port"
#define HTTP_NAME_SOCKET "sckt"
#define HTTP_NAME_HAD_HEADERS "hdrs"
#define HTTP_NAME_RECEIVE_DATA "dRcv"
#define HTTP_NAME_RECEIVE_COUNT "cRcv"
#define HTTP_NAME_SEND_DATA "dSnd"
#define HTTP_NAME_RESPONSE_VAR "res"
#define HTTP_NAME_OPTIONS_VAR "opt"
#define HTTP_NAME_SERVER_VAR "svr"
#define HTTP_NAME_CHUNKED "chunked"
#define HTTP_NAME_CLOSENOW "clsNow"  // boolean: gotta close
#define HTTP_NAME_CONNECTED "conn"     // boolean: we are connected
#define HTTP_NAME_CLOSE "cls"        // close after sending
#define HTTP_NAME_ON_CONNECT JS_EVENT_PREFIX"connect"
#define HTTP_NAME_ON_CLOSE JS_EVENT_PREFIX"close"
#define HTTP_NAME_ON_END JS_EVENT_PREFIX"end"
#define HTTP_NAME_ON_DRAIN JS_EVENT_PREFIX"drain"
#define HTTP_NAME_ON_ERROR JS_EVENT_PREFIX"error"

#define HTTP_ARRAY_HTTP_CLIENT_CONNECTIONS "HttpCC"
#define HTTP_ARRAY_HTTP_SERVERS "HttpS"
#define HTTP_ARRAY_HTTP_SERVER_CONNECTIONS "HttpSC"

#ifdef ESP8266
// esp8266 debugging, need to remove this eventually
extern int os_printf_plus(const char *format, ...)  __attribute__((format(printf, 1, 2)));
#define printf os_printf_plus
#endif

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
        break;
      }
    } else newlineIdx=0;
    jsvStringIteratorNext(&it);
    strIdx++;
  }
  jsvStringIteratorFree(&it);
  // skip if we have no header
  if (headerEnd<0) return false;
  // Now parse the header
  JsVar *vHeaders = jsvNewObject();
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
    netCloseSocket(net, sckt);
    jsvObjectRemoveChild(connection,HTTP_NAME_SOCKET);
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

// returns 0 on success and a (negative) error number on failure
int socketSendData(JsNetwork *net, JsVar *connection, int sckt, JsVar **sendData) {
  char *buf = alloca(net->chunkSize); // allocate on stack

  assert(!jsvIsEmptyString(*sendData));

  size_t bufLen = httpStringGet(*sendData, buf, net->chunkSize);
  int num = netSend(net, sckt, buf, bufLen);
  if (num < 0) return num; // an error occurred
  // Now cut what we managed to send off the beginning of sendData
  if (num > 0) {
    JsVar *newSendData = 0;
    if (num < (int)jsvGetStringLength(*sendData)) {
      // we didn't send all of it... cut out what we did send
      newSendData = jsvNewFromStringVar(*sendData, (size_t)num, JSVAPPENDSTRINGVAR_MAXLENGTH);
    } else {
      // we sent all of it! Issue a drain event, unless we want to close, then we shouldn't
      // callback for more data
      bool wantClose = jsvGetBoolAndUnLock(jsvObjectGetChild(connection,HTTP_NAME_CLOSE,0));
      if (!wantClose) {
        jsiQueueObjectCallbacks(connection, HTTP_NAME_ON_DRAIN, &connection, 1);
      }
      newSendData = jsvNewFromEmptyString();
    }
    jsvUnLock(*sendData);
    *sendData = newSendData;
  }

  return 0;
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

// Fire error events on up to two objects if there is an error, returns true if there is an error
// The error events have a code field and a message field.
static bool fireErrorEvent(int error, JsVar *obj1, JsVar *obj2) {
  bool hadError = error < 0 && error != SOCKET_ERR_CLOSED;
  JsVar *params[1];
  if (hadError) {
    params[0] = jsvNewObject();
    jsvObjectSetChildAndUnLock(params[0], "code", jsvNewFromInteger(error));
    jsvObjectSetChildAndUnLock(params[0], "message",
        jsvNewFromString(socketErrorString(error)));
    if (obj1 != NULL)
      jsiQueueObjectCallbacks(obj1, HTTP_NAME_ON_ERROR, params, 1);
    if (obj2 != NULL)
      jsiQueueObjectCallbacks(obj2, HTTP_NAME_ON_ERROR, params, 1);
    jsvUnLock(params[0]);
  }
  return hadError;
}

// -----------------------------

bool socketServerConnectionsIdle(JsNetwork *net) {
  char *buf = alloca(net->chunkSize); // allocate on stack

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
    JsVar *socket = ((socketType&ST_TYPE_MASK)==ST_HTTP) ? jsvObjectGetChild(connection,HTTP_NAME_RESPONSE_VAR,0) : jsvLockAgain(connection);

    int sckt = (int)jsvGetIntegerAndUnLock(jsvObjectGetChild(connection,HTTP_NAME_SOCKET,0))-1; // so -1 if undefined
    bool closeConnectionNow = jsvGetBoolAndUnLock(jsvObjectGetChild(connection, HTTP_NAME_CLOSENOW, false));
    int error = 0;

    if (!closeConnectionNow) {
      int num = netRecv(net, sckt, buf, net->chunkSize);
      if (num<0) {
        // we probably disconnected so just get rid of this
        closeConnectionNow = true;
        error = num;
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
              jsiQueueObjectCallbacks(server, HTTP_NAME_ON_CONNECT, args, ((socketType&ST_TYPE_MASK)==ST_HTTP) ? 2 : 1);
              jsvUnLock(server);
            }
            if (hadHeaders && !jsvIsEmptyString(receiveData)) {
              // Keep track of how much we received (so we can close once we have it)
              if ((socketType&ST_TYPE_MASK)==ST_HTTP) {
                jsvObjectSetChildAndUnLock(connection, HTTP_NAME_RECEIVE_COUNT,
                    jsvNewFromInteger(
                      jsvGetIntegerAndUnLock(jsvObjectGetChild(connection, HTTP_NAME_RECEIVE_COUNT, JSV_INTEGER)) +
                      jsvGetStringLength(receiveData)
                    ));
              }
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
      if (sendData && !jsvIsEmptyString(sendData)) {
        int sent = socketSendData(net, socket, sckt, &sendData);
        // FIXME? checking for errors is a bit iffy. With the esp8266 network that returns
        // varied error codes we'd want to skip SOCKET_ERR_CLOSED and let the recv side deal
        // with normal closing so we don't miss the tail of what's received, but other drivers
        // return -1 (which is the same value) for all errors. So we rely on the check ~12 lines
        // down if(num>0)closeConnectionNow=false instead.
        if (sent < 0) {
          closeConnectionNow = true;
          error = sent;
        }
        jsvObjectSetChild(socket, HTTP_NAME_SEND_DATA, sendData); // socketSendData prob updated sendData
      }
      // only close if we want to close, have no data to send, and aren't receiving data
      bool wantClose = jsvGetBoolAndUnLock(jsvObjectGetChild(socket,HTTP_NAME_CLOSE,0));
      if (wantClose && (!sendData || jsvIsEmptyString(sendData)) && num<=0) {
        bool reallyCloseNow = true;
        if ((socketType&ST_TYPE_MASK)==ST_HTTP) {
          // Check if we had a Content-Length header - if so, we need to wait until we have received that amount
          JsVar *headers = jsvObjectGetChild(connection,"headers",0);
          if (headers) {
            JsVarInt contentLength = jsvGetIntegerAndUnLock(jsvObjectGetChild(headers,"Content-Length",0));
            JsVarInt contentReceived = jsvGetIntegerAndUnLock(jsvObjectGetChild(connection, HTTP_NAME_RECEIVE_COUNT, 0));
            if (contentLength > contentReceived) {
              reallyCloseNow = false;
            }
            jsvUnLock(headers);
          }
        }
        closeConnectionNow = reallyCloseNow;
      } else if (num > 0)
        closeConnectionNow = false; // guarantee that anything received is processed
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

      // fire error events
      bool hadError = fireErrorEvent(error, connection, socket);

      // fire the close listeners
      jsiQueueObjectCallbacks(connection, HTTP_NAME_ON_END, NULL, 0);
      jsiQueueObjectCallbacks(socket, HTTP_NAME_ON_END, NULL, 0);
      JsVar *params[1] = { jsvNewFromBool(hadError) };
      jsiQueueObjectCallbacks(connection, HTTP_NAME_ON_CLOSE, params, 1);
      jsiQueueObjectCallbacks(socket, HTTP_NAME_ON_CLOSE, params, 1);
      jsvUnLock(params[0]);

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
      jsvObjectRemoveChild(connection,HTTP_NAME_RECEIVE_DATA);
      jsvUnLock(*receiveData);
      *receiveData = 0;
    }
  }
}

bool socketClientConnectionsIdle(JsNetwork *net) {
  char *buf = alloca(net->chunkSize); // allocate on stack

  JsVar *arr = socketGetArray(HTTP_ARRAY_HTTP_CLIENT_CONNECTIONS,false);
  if (!arr) return false;

  bool hadSockets = false;
  JsvObjectIterator it;
  jsvObjectIteratorNew(&it, arr);
  while (jsvObjectIteratorHasValue(&it)) {
    hadSockets = true;
    // Get connection, socket, and socket type
    // For normal sockets, socket==connection, but for HTTP connection is httpCRq and socket is httpCRs
    JsVar *connection = jsvObjectIteratorGetValue(&it);
    SocketType socketType = socketGetType(connection);
    JsVar *socket = ((socketType&ST_TYPE_MASK)==ST_HTTP) ? jsvObjectGetChild(connection,HTTP_NAME_RESPONSE_VAR,0) : jsvLockAgain(connection);
    bool socketClosed = false;
    JsVar *receiveData = 0;

    bool hadHeaders = false;
    int error = 0; // error code received from netXxxx functions
    bool isHttp = (socketType&ST_TYPE_MASK) == ST_HTTP;
    bool closeConnectionNow = jsvGetBoolAndUnLock(jsvObjectGetChild(connection, HTTP_NAME_CLOSENOW, false));
    bool alreadyConnected = jsvGetBoolAndUnLock(jsvObjectGetChild(connection, HTTP_NAME_CONNECTED, false));
    int sckt = (int)jsvGetIntegerAndUnLock(jsvObjectGetChild(connection,HTTP_NAME_SOCKET,0))-1; // so -1 if undefined
    if (sckt>=0) {
      if (isHttp)
        hadHeaders = jsvGetBoolAndUnLock(jsvObjectGetChild(connection,HTTP_NAME_HAD_HEADERS,0));
      else
        hadHeaders = true;
      receiveData = jsvObjectGetChild(connection,HTTP_NAME_RECEIVE_DATA,0);

      /* We do this up here because we want to wait until we have been once
       * around the idle loop (=callbacks have been executed) before we run this */
      if (hadHeaders)
        socketClientPushReceiveData(connection, socket, &receiveData);

      JsVar *sendData = jsvObjectGetChild(connection,HTTP_NAME_SEND_DATA,0);
      if (!closeConnectionNow) {
        // send data if possible
        if (sendData && !jsvIsEmptyString(sendData)) {
          // don't try to send if we're already in error state
          int num = 0;
          if (error == 0) num = socketSendData(net, connection, sckt, &sendData);
          if (num > 0 && !alreadyConnected && !isHttp) { // whoa, we sent something, must be connected!
            jsiQueueObjectCallbacks(connection, HTTP_NAME_ON_CONNECT, &connection, 1);
            jsvObjectSetChildAndUnLock(connection, HTTP_NAME_CONNECTED, jsvNewFromBool(true));
            alreadyConnected = true;
          }
          if (num < 0) {
            closeConnectionNow = true;
            error = num;
          }
          jsvObjectSetChild(connection, HTTP_NAME_SEND_DATA, sendData); // _http_send prob updated sendData
        } else {
          // no data to send, do we want to close? do so.
          if (jsvGetBoolAndUnLock(jsvObjectGetChild(connection, HTTP_NAME_CLOSE, false)))
            closeConnectionNow = true;
        }
        // Now read data if possible (and we have space for it)
        if (!receiveData || !hadHeaders) {
          int num = netRecv(net, sckt, buf, net->chunkSize);
          //if (num != 0) printf("recv returned %d\r\n", num);
          if (!alreadyConnected && num == SOCKET_ERR_NO_CONN) {
            ; // ignore... it's just telling us we're not connected yet
          } else if (num < 0) {
            closeConnectionNow = true;
            error = num;
            // disconnected without headers? error.
            if (!hadHeaders && error == SOCKET_ERR_CLOSED) error = SOCKET_ERR_NO_RESP;
          } else {
            // did we just get connected?
            if (!alreadyConnected && !isHttp) {
              jsiQueueObjectCallbacks(connection, HTTP_NAME_ON_CONNECT, &connection, 1);
              jsvObjectSetChildAndUnLock(connection, HTTP_NAME_CONNECTED, jsvNewFromBool(true));
              alreadyConnected = true;
              // if we do not have any data to send, issue a drain event
              if (!sendData || (int)jsvGetStringLength(sendData) == 0)
                jsiQueueObjectCallbacks(connection, HTTP_NAME_ON_DRAIN, &connection, 1);
            }
            // got data add it to our receive buffer
            if (num > 0) {
              if (!receiveData) {
                receiveData = jsvNewFromEmptyString();
                jsvObjectSetChild(connection, HTTP_NAME_RECEIVE_DATA, receiveData);
              }
              if (receiveData) { // could be out of memory
                jsvAppendStringBuf(receiveData, buf, (size_t)num);
                if ((socketType&ST_TYPE_MASK)==ST_HTTP && !hadHeaders) {
                  // for HTTP see whether we now have full response headers
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
    }

    if (closeConnectionNow) {
      socketClientPushReceiveData(connection, socket, &receiveData);
      if (!receiveData) {
        if ((socketType&ST_TYPE_MASK) != ST_HTTP)
          jsiQueueObjectCallbacks(socket, HTTP_NAME_ON_END, &socket, 1);

        // If we had data to send but the socket closed, this is an error
        JsVar *sendData = jsvObjectGetChild(connection,HTTP_NAME_SEND_DATA,0);
        if (sendData && jsvGetStringLength(sendData) > 0 && error == SOCKET_ERR_CLOSED)
          error = SOCKET_ERR_UNSENT_DATA;
        jsvUnLock(sendData);

        _socketConnectionKill(net, connection);
        JsVar *connectionName = jsvObjectIteratorGetKey(&it);
        jsvObjectIteratorNext(&it);
        jsvRemoveChild(arr, connectionName);
        jsvUnLock(connectionName);
        socketClosed = true;

        // fire error event, if there is an error
        bool hadError = fireErrorEvent(error, connection, NULL);

        // close callback must happen after error callback
        jsiQueueObjectCallbacks(socket, HTTP_NAME_ON_END, NULL, 0);
        JsVar *params[1] = { jsvNewFromBool(hadError) };
        jsiQueueObjectCallbacks(socket, HTTP_NAME_ON_CLOSE, params, 1);
        jsvUnLock(params[0]);
      }
    }


    if (!socketClosed) {
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

      int theClient = netAccept(net, sckt);
      if (theClient >= 0) {
        SocketType socketType = socketGetType(server);
        if ((socketType&ST_TYPE_MASK) == ST_HTTP) {
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
  netCheckError(net);
  return hadSockets;
}

// -----------------------------

JsVar *serverNew(SocketType socketType, JsVar *callback) {
  JsVar *server = jspNewObject(0, ((socketType&ST_TYPE_MASK)==ST_HTTP) ? "httpSrv" : "Server");
  if (!server) return 0; // out of memory
  socketSetType(server, socketType);
  jsvObjectSetChild(server, HTTP_NAME_ON_CONNECT, callback); // no unlock needed
  return server;
}

void serverListen(JsNetwork *net, JsVar *server, int port) {
  JsVar *arr = socketGetArray(HTTP_ARRAY_HTTP_SERVERS, true);
  if (!arr) return; // out of memory

  jsvObjectSetChildAndUnLock(server, HTTP_NAME_PORT, jsvNewFromInteger(port));

  int sckt = netCreateSocket(net, 0/*server*/, (unsigned short)port, NCF_NORMAL, 0 /*options*/);
  if (sckt<0) {
    jsExceptionHere(JSET_INTERNALERROR, "Unable to create socket\n");
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
  if ((socketType&ST_TYPE_MASK)==ST_HTTP) {
    res = jspNewObject(0, "httpCRs");
    if (!res) { jsvUnLock(arr); return 0; } // out of memory?
    req = jspNewObject(0, "httpCRq");
  } else {
    req = jspNewObject(0, "Socket");
  }
  if (req) { // out of memory?
   socketSetType(req, socketType);
   if (callback != NULL)
     jsvUnLock(jsvAddNamedChild(req, callback, HTTP_NAME_ON_CONNECT));

   jsvArrayPush(arr, req);
   if (res)
     jsvObjectSetChild(req, HTTP_NAME_RESPONSE_VAR, res);
   jsvObjectSetChild(req, HTTP_NAME_OPTIONS_VAR, options);
  }
  jsvUnLock2(res, arr);
  return req;
}

void clientRequestWrite(JsNetwork *net, JsVar *httpClientReqVar, JsVar *data) {
  SocketType socketType = socketGetType(httpClientReqVar);
  // Append data to sendData
  JsVar *sendData = jsvObjectGetChild(httpClientReqVar, HTTP_NAME_SEND_DATA, 0);
  if (!sendData) {
    JsVar *options = 0;
    // Only append a header if we're doing HTTP AND we haven't already connected
    if ((socketType&ST_TYPE_MASK) == ST_HTTP)
      if (jsvGetIntegerAndUnLock(jsvObjectGetChild(httpClientReqVar, HTTP_NAME_SOCKET, 0))==0)
        options = jsvObjectGetChild(httpClientReqVar, HTTP_NAME_OPTIONS_VAR, 0);
    if (options) {
      // We're an HTTP client - make a header
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
        // if Transfer-Encoding:chunked was set, subsequent writes need to 'chunk' the data that is sent
        if (jsvIsStringEqualAndUnLock(jsvObjectGetChild(headers, "Transfer-Encoding", 0), "chunked")) {
          jsvObjectSetChildAndUnLock(httpClientReqVar, HTTP_NAME_CHUNKED, jsvNewFromBool(true));
        }
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
    } else { // !options
      // We're not HTTP (or were already connected), so don't send any header
      sendData = jsvNewFromString("");
    }
    jsvObjectSetChild(httpClientReqVar, HTTP_NAME_SEND_DATA, sendData);
    jsvUnLock(options);
  }
  // We have data and aren't out of memory...
  if (data && sendData) {
    // append the data to what we want to send
    JsVar *s = jsvAsString(data, false);
    if (s) {
      if ((socketType&ST_TYPE_MASK) == ST_HTTP &&
          jsvGetBoolAndUnLock(jsvObjectGetChild(httpClientReqVar, HTTP_NAME_CHUNKED, 0))) {
        // If we asked to send 'chunked' data, we need to wrap it up,
        // prefixed with the length
        jsvAppendPrintf(sendData, "%x\r\n%v\r\n", jsvGetStringLength(s), s);
      } else {
        jsvAppendStringVarComplete(sendData,s);
      }
      jsvUnLock(s);
    }
  }
  jsvUnLock(sendData);
  if ((socketType&ST_TYPE_MASK) == ST_HTTP) {
    // on HTTP we connect after the first write
    clientRequestConnect(net, httpClientReqVar);
  }
}

// Connect this connection/socket
void clientRequestConnect(JsNetwork *net, JsVar *httpClientReqVar) {
  // Have we already connected? If so, don't go further
  if (jsvGetIntegerAndUnLock(jsvObjectGetChild(httpClientReqVar, HTTP_NAME_SOCKET, 0))>0)
    return;

  SocketType socketType = socketGetType(httpClientReqVar);

  JsVar *options = jsvObjectGetChild(httpClientReqVar, HTTP_NAME_OPTIONS_VAR, false);
  unsigned short port = (unsigned short)jsvGetIntegerAndUnLock(jsvObjectGetChild(options, "port", 0));

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
    jsExceptionHere(JSET_INTERNALERROR, "Unable to locate host\n");
    // As this is already in the list of connections, an error will be thrown on idle anyway
    jsvObjectSetChildAndUnLock(httpClientReqVar, HTTP_NAME_CLOSENOW, jsvNewFromBool(true));
    jsvUnLock(options);
    netCheckError(net);
    return;
  }

  NetCreateFlags flags = NCF_NORMAL;
#ifdef USE_TLS
  if (socketType & ST_TLS) {
    flags |= NCF_TLS;
    if (port==0) port = 443;
  }
#endif

  if (port==0) port = 80;

  int sckt =  netCreateSocket(net, host_addr, port, flags, options);
  if (sckt<0) {
    jsExceptionHere(JSET_INTERNALERROR, "Unable to create socket\n");
    // As this is already in the list of connections, an error will be thrown on idle anyway
    jsvObjectSetChildAndUnLock(httpClientReqVar, HTTP_NAME_CLOSENOW, jsvNewFromBool(true));
  } else {
    jsvObjectSetChildAndUnLock(httpClientReqVar, HTTP_NAME_SOCKET, jsvNewFromInteger(sckt+1));
  }

  jsvUnLock(options);

  netCheckError(net);
}

// 'end' this connection
void clientRequestEnd(JsNetwork *net, JsVar *httpClientReqVar) {
  SocketType socketType = socketGetType(httpClientReqVar);
  if ((socketType&ST_TYPE_MASK) == ST_HTTP) {
    JsVar *finalData = 0;
    if (jsvGetBoolAndUnLock(jsvObjectGetChild(httpClientReqVar, HTTP_NAME_CHUNKED, 0))) {
      // If we were asked to send 'chunked' data, we need to finish up
      finalData = jsvNewFromString("");
    }
    // on HTTP, this actually means we connect
    // force sendData to be made
    clientRequestWrite(net, httpClientReqVar, finalData);
    jsvUnLock(finalData);
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

  JsVar *sendData = jsvObjectGetChild(httpServerResponseVar, HTTP_NAME_SEND_DATA, 0);
  if (sendData) {
    // If sendData!=0 then we were already called
    jsError("Headers have already been sent");
    jsvUnLock(sendData);
    return;
  }

  sendData = jsvVarPrintf("HTTP/1.0 %d OK\r\nServer: Espruino "JS_VERSION"\r\n", statusCode);
  if (headers) httpAppendHeaders(sendData, headers);
  // finally add ending newline
  jsvAppendString(sendData, "\r\n");
  jsvObjectSetChildAndUnLock(httpServerResponseVar, HTTP_NAME_SEND_DATA, sendData);
}


void serverResponseWrite(JsVar *httpServerResponseVar, JsVar *data) {
  // Append data to sendData
  JsVar *sendData = jsvObjectGetChild(httpServerResponseVar, HTTP_NAME_SEND_DATA, 0);
  if (!sendData) {
    // There was no sent data, which means we haven't written headers yet.
    // Do that now with default values
    serverResponseWriteHead(httpServerResponseVar, 200, 0);
    // sendData should now have been set
    sendData = jsvObjectGetChild(httpServerResponseVar, HTTP_NAME_SEND_DATA, 0);
  }
  // check, just in case!
  if (sendData && !jsvIsUndefined(data)) {
    JsVar *s = jsvAsString(data, false);
    if (s) jsvAppendStringVarComplete(sendData,s);
    jsvUnLock(s);
  }
  jsvUnLock(sendData);
}

void serverResponseEnd(JsVar *httpServerResponseVar) {
  serverResponseWrite(httpServerResponseVar, 0); // force connection->sendData to be created even if data not called
  // TODO: This should only close the connection once the received data length == contentLength header
  jsvObjectSetChildAndUnLock(httpServerResponseVar, HTTP_NAME_CLOSE, jsvNewFromBool(true));
}

