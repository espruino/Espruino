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
#include "jswrap_net.h"
#include "jswrap_stream.h"
#include "jswrap_string.h"
#include "jswrap_functions.h"

#define HTTP_NAME_SOCKETTYPE "type" // normal socket or HTTP
#define HTTP_NAME_PORT "port"
#define HTTP_NAME_SOCKET "sckt"
#define HTTP_NAME_HAD_HEADERS "hdrs"
#define HTTP_NAME_ENDED "endd"
#define HTTP_NAME_RECEIVE_DATA "dRcv"
#define HTTP_NAME_RECEIVE_COUNT "cRcv"
#define HTTP_NAME_SEND_DATA "dSnd"
#define HTTP_NAME_RESPONSE_VAR "res"
#define HTTP_NAME_OPTIONS_VAR "opt"
#define HTTP_NAME_SERVER_VAR "svr"
#define HTTP_NAME_CHUNKED "chunked"
#define HTTP_NAME_HEADERS "headers"
#define HTTP_NAME_CLOSENOW "clsNow"  // boolean: gotta close
#define HTTP_NAME_CONNECTED "conn"     // boolean: we are connected
#define HTTP_NAME_CLOSE "cls"        // close after sending
#define HTTP_NAME_ON_CONNECT JS_EVENT_PREFIX"connect"
#define HTTP_NAME_ON_CLOSE JS_EVENT_PREFIX"close"
#define HTTP_NAME_ON_END JS_EVENT_PREFIX"end"
#define HTTP_NAME_ON_DRAIN JS_EVENT_PREFIX"drain"
#define HTTP_NAME_ON_ERROR JS_EVENT_PREFIX"error"

#define DGRAM_NAME_ON_MESSAGE JS_EVENT_PREFIX"message"

#define HTTP_ARRAY_HTTP_CLIENT_CONNECTIONS "HttpCC"
#define HTTP_ARRAY_HTTP_SERVERS "HttpS"
#define HTTP_ARRAY_HTTP_SERVER_CONNECTIONS "HttpSC"

#ifdef ESP8266
// esp8266 debugging, need to remove this eventually
extern int os_printf_plus(const char *format, ...)  __attribute__((format(printf, 1, 2)));
#define printf os_printf_plus
#endif

#if NET_DBG > 0
#define DBG(format, ...) os_printf(format, ## __VA_ARGS__)
// #include "jsinteractive.h"
// #define DBG(format, ...) jsiConsolePrintf(format, ## __VA_ARGS__)
static char DBG_LIB[] = "socketserver"; // library name
#else
#define DBG(format, ...) do { } while(0)
#endif

// -----------------------------

static ALWAYS_INLINE bool compareTransferEncodingAndUnlock(JsVar *encoding, char *value) {
    // RFC 2616: All transfer-coding values are case-insensitive.
    return jsvIsStringIEqualAndUnLock(encoding, value);
}

static void httpAppendHeaders(JsVar *string, JsVar *headerObject) {
  // append headers
  JsvObjectIterator it;
  jsvObjectIteratorNew(&it, headerObject);
  while (jsvObjectIteratorHasValue(&it)) {
    JsVar *k = jsvAsStringAndUnLock(jsvObjectIteratorGetKey(&it));
    JsVar *v = jsvAsStringAndUnLock(jsvObjectIteratorGetValue(&it));
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
    char ch = jsvStringIteratorGetCharAndNext(&it);
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
    strIdx++;
  }
  jsvStringIteratorFree(&it);
  // skip if we have no header
  if (headerEnd<0) return false;
  // Now parse the header
  JsVar *vHeaders = jsvNewObject();
  if (!vHeaders) return true;
  jsvUnLock(jsvAddNamedChild(objectForData, vHeaders, HTTP_NAME_HEADERS));
  strIdx = 0;
  int firstSpace = -1;
  int secondSpace = -1;
  int firstEOL = -1;
  int lineNumber = 0;
  int lastLineStart = 0;
  int colonPos = 0;
  int valueStart = 0;
  //jsiConsolePrintStringVar(receiveData);
  jsvStringIteratorNew(&it, *receiveData, 0);
    while (jsvStringIteratorHasChar(&it)) {
      char ch = jsvStringIteratorGetCharAndNext(&it);
      if (ch==' ' || ch=='\r') {
        if (firstSpace<0) firstSpace = strIdx;
        else if (secondSpace<0) secondSpace = strIdx;
      }
      if (ch == ':' && colonPos<0) colonPos = strIdx;
      else if (colonPos>0 && !valueStart && !isWhitespace(ch)) valueStart = strIdx; // ignore whitespace after :
      if (ch == '\r') {
        if (firstEOL<0) firstEOL=strIdx;
        if (lineNumber>0 && colonPos>lastLineStart && valueStart>lastLineStart && lastLineStart<strIdx) {
          JsVar *hVal = jsvNewFromEmptyString();
          if (hVal)
            jsvAppendStringVar(hVal, *receiveData, (size_t)valueStart, (size_t)(strIdx-valueStart));
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
        valueStart = 0;
      }
      if (ch == '\r' || ch == '\n') {
        lastLineStart = strIdx+1;
      }
      strIdx++;
    }
    jsvStringIteratorFree(&it);
  // flag the req/response if Transfer-Encoding:chunked was set
  JsVarInt contentToReceive;
  if (compareTransferEncodingAndUnlock(jsvObjectGetChildI(vHeaders, "Transfer-Encoding"), "chunked")) {
    jsvObjectSetChildAndUnLock(objectForData, HTTP_NAME_CHUNKED, jsvNewFromBool(true));
    contentToReceive = 1;
  } else {
    contentToReceive = jsvGetIntegerAndUnLock(jsvObjectGetChildI(vHeaders,"Content-Length"));
  }
  jsvObjectSetChildAndUnLock(objectForData, HTTP_NAME_RECEIVE_COUNT, jsvNewFromInteger(contentToReceive));
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
    *(str++) = jsvStringIteratorGetCharAndNext(&it);
  }
  jsvStringIteratorFree(&it);
  return len-l;
}

// -----------------------------

static JsVar *socketGetArray(const char *name, bool create) {
  return jsvObjectGetChild(execInfo.hiddenRoot, name, create?JSV_ARRAY:0);
}

static NO_INLINE SocketType socketGetType(JsVar *var) {
  return jsvGetIntegerAndUnLock(jsvObjectGetChildIfExists(var, HTTP_NAME_SOCKETTYPE));
}

static NO_INLINE void socketSetType(JsVar *var, SocketType socketType) {
  jsvObjectSetChildAndUnLock(var, HTTP_NAME_SOCKETTYPE, jsvNewFromInteger((JsVarInt)socketType));
}

void _socketConnectionKill(JsNetwork *net, JsVar *connection) {
  if (!net || networkState != NETWORKSTATE_ONLINE) return;
  int sckt = (int)jsvGetIntegerAndUnLock(jsvObjectGetChildIfExists(connection,HTTP_NAME_SOCKET))-1; // so -1 if undefined
  if (sckt>=0) {
    SocketType socketType = socketGetType(connection);
    netCloseSocket(net, socketType, sckt);
    jsvObjectRemoveChild(connection,HTTP_NAME_SOCKET);
    jsvObjectSetChildAndUnLock(connection, HTTP_NAME_CONNECTED, jsvNewFromBool(false));
    jsvObjectSetChildAndUnLock(connection, HTTP_NAME_CLOSE, jsvNewFromBool(true));
  }
}

bool _socketConnectionOpen(JsVar *connection) {
  return !(jsvGetBoolAndUnLock(jsvObjectGetChild(connection, HTTP_NAME_CLOSENOW, false)) ||
           jsvGetBoolAndUnLock(jsvObjectGetChild(connection, HTTP_NAME_CLOSE, false)));
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
  SocketType socketType = socketGetType(connection);

  assert(!jsvIsEmptyString(*sendData));

  size_t sndBufLen;
  if ((socketType&ST_TYPE_MASK)==ST_UDP) {
      sndBufLen = (size_t)jsvGetStringLength(*sendData);
      if (sndBufLen+1024 > jsuGetFreeStack()) {
          jsExceptionHere(JSET_ERROR, "Not enough free stack to send this amount of data");
          return -1;
      }
  } else {
      sndBufLen = (size_t)net->chunkSize;
  }
  char *buf = alloca(sndBufLen); // allocate on stack

  size_t bufLen = httpStringGet(*sendData, buf, sndBufLen);
  int num = netSend(net, socketType, sckt, buf, bufLen);
  DBG("socketSendData %x:%d (%d -> %d)\n", *(uint32_t*)buf, *(unsigned short*)(buf+sizeof(uint32_t)), bufLen, num);
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
      bool wantClose = jsvGetBoolAndUnLock(jsvObjectGetChildIfExists(connection,HTTP_NAME_CLOSE));
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

void socketPushReceiveData(JsVar *reader, JsVar **receiveData, bool isHttp, bool force) {
  if (!*receiveData || jsvIsEmptyString(*receiveData)) {
    // no data available (after headers)
    return;
  }

  JsVar *nextChunk = 0;
  JsVar *partialChunk = 0;

  // Keep track of how much we received (so we can close once we have it)
  if (isHttp) {
    size_t len = (size_t)jsvGetStringLength(*receiveData);
    if (jsvGetBoolAndUnLock(jsvObjectGetChildIfExists(reader, HTTP_NAME_CHUNKED))) {
      // check for incomplete chunk, at least "0\r\n\r\n"
      if (len < 5) return; // incomplete, wait for more data

      JsVar *crlf = jsvNewFromString("\r\n");
      JsVar *zero = jsvNewFromInteger(0);
      size_t startIdx = (size_t)jswrap_string_indexOf(*receiveData, crlf, zero, false);
      jsvUnLock2(crlf, zero);

      JsVar *sixteen = jsvNewFromInteger(16);
      int chunkLen = jsvGetIntegerAndUnLock(jswrap_parseInt(*receiveData, sixteen));
      jsvUnLock(sixteen);
      DBG("D:%d\n", chunkLen);

      // for 'chunked' set the counter to 1 to read on or 0 if at last chunk
      jsvObjectSetChildAndUnLock(reader, HTTP_NAME_RECEIVE_COUNT, jsvNewFromInteger(chunkLen ? 1 : 0));
      if (!chunkLen) { // no 'data' callback
        // clear received data
        jsvUnLock(*receiveData);
        *receiveData = 0;
        return;
      }

      startIdx += 2; // skip the CRLF
      size_t nextIdx = startIdx + (size_t)chunkLen + 2; // CRLF at the end
      if (nextIdx < len) { // there is another chunk in the buffer
        DBG("D:nextIdx %d %d\n", nextIdx, len);
        nextChunk = jsvNewFromEmptyString();
        if (!nextChunk) return; // out of memory
        jsvAppendStringVar(nextChunk, *receiveData, nextIdx, len-nextIdx);
      } else if (nextIdx > len) { // chunk not complete
        DBG("D:partialIdx %d %d %d\n", len - startIdx, nextIdx - len - 2, len);
        if (nextIdx - len < 3) return; // just CRLF missing, wait
        // use the data available, write remaining length, wait
        partialChunk = jsvNewFromEmptyString();
        if (!partialChunk) return; // out of memory
        jsvAppendPrintf(partialChunk, "%x\r\n", nextIdx - len - 2);
      }

      JsVar *chunkData = jsvNewFromEmptyString();
      if (!chunkData) return; // out of memory
      jsvAppendStringVar(chunkData, *receiveData, startIdx, (size_t)chunkLen);
      jsvUnLock(*receiveData);
      *receiveData = chunkData;
    } else {
      jsvObjectSetChildAndUnLock(reader, HTTP_NAME_RECEIVE_COUNT,
        jsvNewFromInteger(
          jsvGetIntegerAndUnLock(jsvObjectGetChild(reader, HTTP_NAME_RECEIVE_COUNT, JSV_INTEGER)) - (JsVarInt)len)
        );
    }
  }

  // execute 'data' callback or save data
  if (!jswrap_stream_pushData(reader, *receiveData, force)) {
    jsvUnLock2(nextChunk, partialChunk);
    return;
  }

  // clear received data
  jsvUnLock(*receiveData);
  *receiveData = nextChunk ? nextChunk : partialChunk;

  if (nextChunk) { // process following chunks if any
    socketPushReceiveData(reader, receiveData, isHttp, force);
  }
}

void socketReceivedUDP(JsVar *connection, JsVar **receiveData) {
  // Get the header
  size_t len = jsvGetStringLength(*receiveData);
  if (len < sizeof(JsNetUDPPacketHeader)) return; // not enough data for header!
  char buf[sizeof(JsNetUDPPacketHeader)];
  jsvGetStringChars(*receiveData, 0, buf, sizeof(JsNetUDPPacketHeader));
  JsNetUDPPacketHeader *header = (JsNetUDPPacketHeader*)buf;
  if (sizeof(JsNetUDPPacketHeader)+header->length < len) return; // not enough data yet

  JsVar *rinfo = jsvNewObject();
  if (rinfo) {
    // split the received data string to get the data we need
    JsVar *data = jsvNewFromStringVar(*receiveData, sizeof(JsNetUDPPacketHeader), header->length);
    JsVar *newReceiveData = 0;
    if (len > sizeof(JsNetUDPPacketHeader)+header->length)
      newReceiveData = jsvNewFromStringVar(*receiveData, sizeof(JsNetUDPPacketHeader)+header->length, JSVAPPENDSTRINGVAR_MAXLENGTH);
    jsvUnLock(*receiveData);
    *receiveData = newReceiveData;
    // fire the received data event
    jsvObjectSetChildAndUnLock(rinfo, "address", jsvVarPrintf("%d.%d.%d.%d", header->host[0], header->host[1], header->host[2], header->host[3]));
    jsvObjectSetChildAndUnLock(rinfo, "port", jsvNewFromInteger(header->port));
    jsvObjectSetChildAndUnLock(rinfo, "size", jsvNewFromInteger(header->length));
    JsVar *args[2] = { data, rinfo };
    jsiQueueObjectCallbacks(connection, DGRAM_NAME_ON_MESSAGE, args, 2);
    jsvUnLock2(data,rinfo);
  }
}

void socketReceived(JsVar *connection, JsVar *socket, SocketType socketType, JsVar **receiveData, bool isServer) {
  if ((socketType&ST_TYPE_MASK)==ST_UDP) {
    socketReceivedUDP(connection, receiveData);
    return;
  }
  JsVar *reader = isServer ? connection : socket;
  bool isHttp = (socketType&ST_TYPE_MASK)==ST_HTTP;
  bool hadHeaders = jsvGetBoolAndUnLock(jsvObjectGetChildIfExists(reader,HTTP_NAME_HAD_HEADERS));
  if (!hadHeaders) {
    if (!isHttp) {
      hadHeaders = true;
    } else if (httpParseHeaders(receiveData, reader, isServer)) {
      hadHeaders = true;

      // on connect only when just parsed the HTTP headers
      if (isServer) {
        JsVar *server = jsvObjectGetChildIfExists(connection,HTTP_NAME_SERVER_VAR);
        JsVar *args[2] = { connection, socket };
        jsiQueueObjectCallbacks(server, HTTP_NAME_ON_CONNECT, args, isHttp ? 2 : 1);
        jsvUnLock(server);
      } else {
        jsiQueueObjectCallbacks(connection, HTTP_NAME_ON_CONNECT, &socket, 1);
      }
    }
    jsvObjectSetChildAndUnLock(reader, HTTP_NAME_HAD_HEADERS, jsvNewFromBool(hadHeaders));
  }
  if (!hadHeaders) {
    // no headers yet, no 'data' callback
    return;
  }
  socketPushReceiveData(reader, receiveData, isHttp, false);
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
  char *buf = alloca((size_t)net->chunkSize); // allocate on stack

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
    bool isHttp = (socketType&ST_TYPE_MASK) == ST_HTTP;
    JsVar *socket = isHttp ? jsvObjectGetChildIfExists(connection,HTTP_NAME_RESPONSE_VAR) : jsvLockAgain(connection);

    int sckt = (int)jsvGetIntegerAndUnLock(jsvObjectGetChildIfExists(connection,HTTP_NAME_SOCKET))-1; // so -1 if undefined
    bool closeConnectionNow = jsvGetBoolAndUnLock(jsvObjectGetChild(connection, HTTP_NAME_CLOSENOW, false));
    int error = 0;

    if (!closeConnectionNow) {
      int num = netRecv(net, socketType, sckt, buf, (size_t)net->chunkSize);
      if (num<0) {
        // we probably disconnected so just get rid of this
        closeConnectionNow = true;
        error = num;
      } else {
        if (num>0) {
          JsVar *receiveData = jsvObjectGetChildIfExists(connection,HTTP_NAME_RECEIVE_DATA);
          if (!receiveData) receiveData = jsvNewFromEmptyString();
          if (receiveData) {
            jsvAppendStringBuf(receiveData, buf, (size_t)num);
            socketReceived(connection, socket, socketType, &receiveData, true);
            jsvObjectSetChild(connection,HTTP_NAME_RECEIVE_DATA,receiveData);
            jsvUnLock(receiveData);
          }
        }
      }

      // send data if possible
      JsVar *sendData = jsvObjectGetChildIfExists(socket,HTTP_NAME_SEND_DATA);
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
        jsvObjectSetChild(socket, HTTP_NAME_SEND_DATA, sendData); // socketSendData updated sendData
      }
      // only close if we want to close, have no data to send, and aren't receiving data
      if ((!sendData || jsvIsEmptyString(sendData)) && num<=0) {
        bool reallyCloseNow = jsvGetBoolAndUnLock(jsvObjectGetChildIfExists(socket,HTTP_NAME_CLOSE));
        if (isHttp) {
          bool hadHeaders = jsvGetBoolAndUnLock(jsvObjectGetChildIfExists(connection,HTTP_NAME_HAD_HEADERS));
          JsVarInt contentToReceive = jsvGetIntegerAndUnLock(jsvObjectGetChildIfExists(connection, HTTP_NAME_RECEIVE_COUNT));
          if (contentToReceive > 0 || !hadHeaders) {
            reallyCloseNow = false;
          } else if (!jsvGetBoolAndUnLock(jsvObjectGetChildIfExists(connection,HTTP_NAME_ENDED))) {
            jsvObjectSetChildAndUnLock(connection, HTTP_NAME_ENDED, jsvNewFromBool(true));
            jsiQueueObjectCallbacks(connection, HTTP_NAME_ON_END, NULL, 0);
            DBG("ONEND %d (%d)\n", contentToReceive, reallyCloseNow);
          }
        }
        closeConnectionNow = reallyCloseNow;
      } else if (num > 0)
        closeConnectionNow = false; // guarantee that anything received is processed
      jsvUnLock(sendData);
    }
    if (closeConnectionNow) {
      DBG("CLOSE NOW\n");

      // send out any data that we were POSTed
      bool hadHeaders = jsvGetBoolAndUnLock(jsvObjectGetChildIfExists(connection,HTTP_NAME_HAD_HEADERS));
      if (hadHeaders) {
        // execute 'data' callback or save data
        JsVar *receiveData = jsvObjectGetChildIfExists(connection,HTTP_NAME_RECEIVE_DATA);
        socketPushReceiveData(connection, &receiveData, isHttp, true);
        jsvUnLock(receiveData);
      }

      // fire error events
      bool hadError = fireErrorEvent(error, connection, socket);

      // fire end listeners
      jsiQueueObjectCallbacks(socket, HTTP_NAME_ON_END, NULL, 0);

      // fire the close listeners
      JsVar *params[1] = { jsvNewFromBool(hadError) };
      if (connection!=socket)
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


bool socketClientConnectionsIdle(JsNetwork *net) {
  char *buf = alloca((size_t)net->chunkSize); // allocate on stack

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
    bool isHttp = (socketType&ST_TYPE_MASK) == ST_HTTP;
    JsVar *socket = isHttp ? jsvObjectGetChildIfExists(connection,HTTP_NAME_RESPONSE_VAR) : jsvLockAgain(connection);
    bool socketClosed = false;
    JsVar *receiveData = 0;

    bool hadHeaders = false;
    int error = 0; // error code received from netXxxx functions
    bool closeConnectionNow = jsvGetBoolAndUnLock(jsvObjectGetChild(connection, HTTP_NAME_CLOSENOW, false));
    bool alreadyConnected = jsvGetBoolAndUnLock(jsvObjectGetChild(connection, HTTP_NAME_CONNECTED, false));
    int sckt = (int)jsvGetIntegerAndUnLock(jsvObjectGetChildIfExists(connection,HTTP_NAME_SOCKET))-1; // so -1 if undefined
    if (sckt>=0) {
      if (isHttp)
        hadHeaders = jsvGetBoolAndUnLock(jsvObjectGetChildIfExists(socket,HTTP_NAME_HAD_HEADERS));
      else
        hadHeaders = true;
      receiveData = jsvObjectGetChildIfExists(connection,HTTP_NAME_RECEIVE_DATA);

      /* We do this up here because we want to wait until we have been once
       * around the idle loop (=callbacks have been executed) before we run this */
      if (hadHeaders)
        socketPushReceiveData(socket, &receiveData, isHttp, false);

      if (!closeConnectionNow) {
        JsVar *sendData = jsvObjectGetChildIfExists(connection,HTTP_NAME_SEND_DATA);
        // send data if possible
        if (sendData && !jsvIsEmptyString(sendData)) {
          // don't try to send if we're already in error state
          int num = 0;
          if (error == 0) {
              num = socketSendData(net, connection, sckt, &sendData);
          }
          if (num > 0 && !alreadyConnected && !isHttp) { // whoa, we sent something, must be connected!
            jsiQueueObjectCallbacks(connection, HTTP_NAME_ON_CONNECT, &connection, 1);
            jsvObjectSetChildAndUnLock(connection, HTTP_NAME_CONNECTED, jsvNewFromBool(true));
            alreadyConnected = true;
          }
          if (num < 0) {
            closeConnectionNow = true;
            error = num;
          }
          jsvObjectSetChild(connection, HTTP_NAME_SEND_DATA, sendData); // socketSendData updated sendData
        } else {
          // no data to send, do we want to close? do so.
          if (jsvGetBoolAndUnLock(jsvObjectGetChild(connection, HTTP_NAME_CLOSE, false)))
            closeConnectionNow = true;
          if (isHttp) {
            JsVarInt contentToReceive = jsvGetIntegerAndUnLock(jsvObjectGetChildIfExists(socket, HTTP_NAME_RECEIVE_COUNT));
            if (contentToReceive > 0 || !hadHeaders) {
              closeConnectionNow = false;
            } else if (!jsvGetBoolAndUnLock(jsvObjectGetChildIfExists(socket,HTTP_NAME_ENDED))) {
              jsvObjectSetChildAndUnLock(socket, HTTP_NAME_ENDED, jsvNewFromBool(true));
              jsiQueueObjectCallbacks(socket, HTTP_NAME_ON_END, NULL, 0);
              DBG("onEnd %d (%d) %d\n", contentToReceive, closeConnectionNow, hadHeaders);
            }
          }
        }
        // Now read data if possible (and we have space for it)
        int num = netRecv(net, socketType, sckt, buf, (size_t)net->chunkSize);
        if (!alreadyConnected && num == SOCKET_ERR_NO_CONN) {
          ; // ignore... it's just telling us we're not connected yet
        } else if (num < 0) {
          closeConnectionNow = true;
          // only error out when the response was not completely received
          if (num == SOCKET_ERR_CLOSED) {
            JsVarInt contentToReceive = jsvGetIntegerAndUnLock(jsvObjectGetChildIfExists(socket, HTTP_NAME_RECEIVE_COUNT));
            if (!isHttp || contentToReceive > 0 || !hadHeaders) {
              error = num;
              // disconnected without headers? error.
              if (!hadHeaders) error = SOCKET_ERR_NO_RESP;
            }
          } else {
            error = num;
          }
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
            if (!receiveData)
              receiveData = jsvNewFromEmptyString();
            if (receiveData) { // could be out of memory
              jsvAppendStringBuf(receiveData, buf, (size_t)num);
              socketReceived(connection, socket, socketType, &receiveData, false);
              jsvObjectSetChild(connection, HTTP_NAME_RECEIVE_DATA, receiveData);
            }
          }
        }
        jsvUnLock(sendData);
      }
    }

    if (closeConnectionNow) {
      DBG("close now\n");

      socketPushReceiveData(socket, &receiveData, isHttp, true);
      if (!receiveData || jsvIsEmptyString(receiveData)) {
        // If we had data to send but the socket closed, this is an error
        JsVar *sendData = jsvObjectGetChildIfExists(connection,HTTP_NAME_SEND_DATA);
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

        if (!jsvGetBoolAndUnLock(jsvObjectGetChildIfExists(socket,HTTP_NAME_ENDED))) {
          jsvObjectSetChildAndUnLock(socket, HTTP_NAME_ENDED, jsvNewFromBool(true));
          jsiQueueObjectCallbacks(socket, HTTP_NAME_ON_END, NULL, 0);
          DBG("onEnd:force\n");
        }

        // close callback must happen after error callback
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
      SocketType socketType = socketGetType(server);

      int theClient = -1;
      // Check for new connections
      if ((socketType&ST_TYPE_MASK)!=ST_UDP) {
          int sckt = (int)jsvGetIntegerAndUnLock(jsvObjectGetChildIfExists(server,HTTP_NAME_SOCKET))-1; // so -1 if undefined
          theClient = netAccept(net, sckt);
      }
      if (theClient >= 0) { // We have a new connection
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
            jsvObjectSetChildAndUnLock(res, HTTP_NAME_SOCKET, jsvNewFromInteger(theClient+1));
            // Auto-add connection close header (in HTTP/1.0 this seemed implicit, now it must be explicit)
            // This can always be overwritten with setHeader or writeHead
            JsVar *name = jsvNewFromString("Connection");
            JsVar *value = jsvNewFromString("close");
            serverResponseSetHeader(res, name, value);
            jsvUnLock2(name, value);
          }
          jsvUnLock2(req, res);
        } else {
          // Normal sockets
          JsVar *sock = jspNewObject(0, "Socket");
          if (sock) { // out of memory?
            socketSetType(sock, socketType);
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

void serverAddMembership(JsNetwork *net, JsVar *server, JsVar *group, JsVar *ip) {
  NOT_USED(net);
  // FIXME: perhaps extend the JsNetwork with addmembership/removemembership instead of using options
  JsVar *options = jsvObjectGetChildIfExists(server, HTTP_NAME_OPTIONS_VAR);
  if (options) {
      jsvObjectSetChild(options, "multicastGroup", group);
      jsvObjectSetChild(options, "multicastIp", ip);
      jsvUnLock(options);
  }
}

void serverListen(JsNetwork *net, JsVar *server, unsigned short port, SocketType socketType) {
  JsVar *arr = socketGetArray(HTTP_ARRAY_HTTP_SERVERS, true);
  if (!arr) return; // out of memory

  jsvObjectSetChildAndUnLock(server, HTTP_NAME_PORT, jsvNewFromInteger(port));
  JsVar *options = jsvObjectGetChild(server, HTTP_NAME_OPTIONS_VAR, false);

  int sckt = netCreateSocket(net, socketType, 0/*server*/, port, options);
  if (sckt<0) {
    jsExceptionHere(JSET_INTERNALERROR, "Unable to create socket\n");
    jsvObjectSetChildAndUnLock(server, HTTP_NAME_CLOSENOW, jsvNewFromBool(true));
  } else {
    jsvObjectSetChildAndUnLock(server, HTTP_NAME_SOCKET, jsvNewFromInteger(sckt+1));

    if ((socketType&ST_TYPE_MASK)==ST_UDP) {
      JsVar *serverConns = socketGetArray(HTTP_ARRAY_HTTP_SERVER_CONNECTIONS, true);
      if (serverConns) {
        jsvArrayPush(serverConns, server);
        jsvUnLock(serverConns);
      }
    } else {
      // add to list of servers
      jsvArrayPush(arr, server);
    }
  }

  DBG("serverListen port=%d (%d)\n", port, sckt);
  jsvUnLock2(options, arr);
}

void serverClose(JsNetwork *net, JsVar *server) {
  JsVar *arr = socketGetArray(HTTP_ARRAY_HTTP_SERVERS,false);
  if (arr) {
    // close socket
    _socketConnectionKill(net, server);
    // remove from array
    JsVar *idx = jsvGetIndexOf(arr, server, true);
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
  } else if ((socketType&ST_TYPE_MASK)==ST_UDP) {
    req = jspNewObject(0, "dgramSocket");
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

void clientRequestWrite(JsNetwork *net, JsVar *httpClientReqVar, JsVar *data, JsVar *host, unsigned short portNumber) {
  if (!_socketConnectionOpen(httpClientReqVar)) {
    jsExceptionHere(JSET_ERROR, "This socket is closed.");
    return;
  }
  SocketType socketType = socketGetType(httpClientReqVar);

  // Append data to sendData
  JsVar *sendData = jsvObjectGetChildIfExists(httpClientReqVar, HTTP_NAME_SEND_DATA);
  if (!sendData) {
    JsVar *options = 0;
    // Only append a header if we're doing HTTP AND we haven't already connected
    if ((socketType&ST_TYPE_MASK) == ST_HTTP)
      if (jsvGetIntegerAndUnLock(jsvObjectGetChildIfExists(httpClientReqVar, HTTP_NAME_SOCKET))==0)
        options = jsvObjectGetChildIfExists(httpClientReqVar, HTTP_NAME_OPTIONS_VAR);
    if (options) {
      // We're an HTTP client - make a header
      JsVar *method = jsvObjectGetChildIfExists(options, "method");
      JsVar *path = jsvObjectGetChildIfExists(options, "path");
      sendData = jsvVarPrintf("%v %v HTTP/1.1\r\nUser-Agent: Espruino "JS_VERSION"\r\nConnection: close\r\n", method, path);
      jsvUnLock2(method, path);
      JsVar *headers = jsvObjectGetChildIfExists(options, HTTP_NAME_HEADERS);
      bool hasHostHeader = false;
      if (jsvIsObject(headers)) {
        JsVar *hostHeader = jsvObjectGetChildI(headers, "Host");
        hasHostHeader = hostHeader!=0;
        jsvUnLock(hostHeader);
        httpAppendHeaders(sendData, headers);
        // if Transfer-Encoding:chunked was set, subsequent writes need to 'chunk' the data that is sent
        if (compareTransferEncodingAndUnlock(jsvObjectGetChildIfExists(headers, "Transfer-Encoding"), "chunked")) {
          jsvObjectSetChildAndUnLock(httpClientReqVar, HTTP_NAME_CHUNKED, jsvNewFromBool(true));
        }
      }
      jsvUnLock(headers);
      if (!hasHostHeader) {
        JsVar *host = jsvObjectGetChildIfExists(options, "host");
        int port = (int)jsvGetIntegerAndUnLock(jsvObjectGetChildIfExists(options, "port"));
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
    JsVar *s = jsvAsString(data);
    if (s) {
      if (jsvGetBoolAndUnLock(jsvObjectGetChildIfExists(httpClientReqVar, HTTP_NAME_CHUNKED))) {
        // If we asked to send 'chunked' data, we need to wrap it up,
        // prefixed with the length
        jsvAppendPrintf(sendData, "%x\r\n%v\r\n", jsvGetStringLength(s), s);
      } else {
        if ((socketType&ST_TYPE_MASK) == ST_UDP) {
          char hostName[128];
          jsvGetString(host, hostName, sizeof(hostName));
          JsNetUDPPacketHeader header;
          networkGetHostByName(net, hostName, (uint32_t*)&header.host);
          header.port = portNumber;
          header.length = (uint16_t)jsvGetStringLength(s);
          jsvAppendStringBuf(sendData, (const char*)&header, sizeof(header));
        }
        jsvAppendStringVarComplete(sendData,s);
      }
      jsvUnLock(s);
    }
  }
  jsvUnLock(sendData);
  if ((socketType&ST_TYPE_MASK) != ST_NORMAL) {
    // on HTTP/UDP we connect on-demand with the first write/send
    clientRequestConnect(net, httpClientReqVar);
  }
}

// Connect this connection/socket
void clientRequestConnect(JsNetwork *net, JsVar *httpClientReqVar) {
  DBG("clientRequestConnect\n");
  // Have we already connected? If so, don't go further
  if (jsvGetIntegerAndUnLock(jsvObjectGetChildIfExists(httpClientReqVar, HTTP_NAME_SOCKET))>0)
    return;

  SocketType socketType = socketGetType(httpClientReqVar);

  JsVar *options = jsvObjectGetChildIfExists(httpClientReqVar, HTTP_NAME_OPTIONS_VAR);
  unsigned short port = (unsigned short)jsvGetIntegerAndUnLock(jsvObjectGetChildIfExists(options, "port"));

  uint32_t host_addr = 0;
  JsVar *hostNameVar = jsvObjectGetChildIfExists(options, "host");
  if (jsvIsUndefined(hostNameVar)) {
    host_addr = 0x0100007F; // 127.0.0.1
  } else {
    char hostName[128];
    jsvGetString(hostNameVar, hostName, sizeof(hostName));
    networkGetHostByName(net, hostName, &host_addr);
  }
  jsvUnLock(hostNameVar);




  if(!host_addr) {
    jsExceptionHere(JSET_INTERNALERROR, "Unable to locate host\n");
    // As this is already in the list of connections, an error will be thrown on idle anyway
    jsvObjectSetChildAndUnLock(httpClientReqVar, HTTP_NAME_CLOSENOW, jsvNewFromBool(true));
    jsvUnLock(options);
    netCheckError(net);
    return;
  }

#ifdef USE_TLS
  if (socketType & ST_TLS) {
    if (port==0) port = 443;
  }
#endif
  if ((socketType&ST_TYPE_MASK) == ST_HTTP) {
    if (port==0) port = 80;
  }

  int sckt =  netCreateSocket(net, socketType, host_addr, port, options);
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
    if (jsvGetBoolAndUnLock(jsvObjectGetChildIfExists(httpClientReqVar, HTTP_NAME_CHUNKED))) {
      // If we were asked to send 'chunked' data, we need to finish up
      finalData = jsvNewFromString("");
    }
    // on HTTP, this actually means we connect
    // force sendData to be made
    clientRequestWrite(net, httpClientReqVar, finalData, NULL, 0);
    jsvUnLock(finalData);
  } else {
    // if we never sent any data, make sure we close 'now'
    JsVar *sendData = jsvObjectGetChildIfExists(httpClientReqVar, HTTP_NAME_SEND_DATA);
    if (!sendData || jsvIsEmptyString(sendData))
      jsvObjectSetChildAndUnLock(httpClientReqVar, HTTP_NAME_CLOSENOW, jsvNewFromBool(true));
    jsvUnLock(sendData);
  }

  // request close after all data sent
  jsvObjectSetChildAndUnLock(httpClientReqVar, HTTP_NAME_CLOSE, jsvNewFromBool(true));
}

void serverResponseSetHeader(JsVar *httpServerResponseVar, JsVar *name, JsVar *value) {
  name = jsvAsString(name);
  value = jsvAsString(value);
  JsVar *headers = jsvObjectGetChild(httpServerResponseVar, HTTP_NAME_HEADERS, JSV_OBJECT);
  if (jsvIsObject(headers))
    jsvObjectSetChildVar(headers, name, value);
  jsvUnLock3(headers,name,value);
}


void serverResponseWriteHead(JsVar *httpServerResponseVar, int statusCode, JsVar *explicitHeaders) {
  DBG("serverResponseWriteHead %d\n", statusCode);
  if (!jsvIsUndefined(explicitHeaders) && !jsvIsObject(explicitHeaders)) {
    jsError("Headers sent to writeHead should be an object");
    return;
  }

  JsVar *sendData = jsvObjectGetChildIfExists(httpServerResponseVar, HTTP_NAME_SEND_DATA);
  if (sendData) {
    // If sendData!=0 then we were already called
    jsError("Headers have already been sent");
    jsvUnLock(sendData);
    return;
  }

  JsVar *headers = jsvNewObject();
  JsVar *implicitHeaders = jsvObjectGetChildIfExists(httpServerResponseVar, HTTP_NAME_HEADERS);
  if (jsvIsObject(implicitHeaders)) jsvObjectAppendAll(headers, implicitHeaders);
  jsvUnLock(implicitHeaders);
  if (jsvIsObject(explicitHeaders)) jsvObjectAppendAll(headers, explicitHeaders);


  sendData = jsvVarPrintf("HTTP/1.1 %d OK\r\nServer: Espruino "JS_VERSION"\r\n", statusCode);
  if (headers) {
    httpAppendHeaders(sendData, headers);
    // if Transfer-Encoding:chunked was set, subsequent writes need to 'chunk' the data that is sent
    if (compareTransferEncodingAndUnlock(jsvObjectGetChildI(headers, "Transfer-Encoding"), "chunked")) {
      jsvObjectSetChildAndUnLock(httpServerResponseVar, HTTP_NAME_CHUNKED, jsvNewFromBool(true));
    }
  }
  jsvUnLock(headers);
  // finally add ending newline
  jsvAppendString(sendData, "\r\n");
  jsvObjectSetChildAndUnLock(httpServerResponseVar, HTTP_NAME_SEND_DATA, sendData);
}


void serverResponseWrite(JsVar *httpServerResponseVar, JsVar *data) {
  if (!_socketConnectionOpen(httpServerResponseVar)) {
    jsExceptionHere(JSET_ERROR, "This socket is closed.");
    return;
  }
  // Append data to sendData
  JsVar *sendData = jsvObjectGetChildIfExists(httpServerResponseVar, HTTP_NAME_SEND_DATA);
  if (!sendData) {
    // There was no sent data, which means we haven't written headers yet.
    // Do that now with default values
    serverResponseWriteHead(httpServerResponseVar, 200, 0);
    // sendData should now have been set
    sendData = jsvObjectGetChildIfExists(httpServerResponseVar, HTTP_NAME_SEND_DATA);
  }
  // check, just in case!
  if (sendData && !jsvIsUndefined(data)) {
    JsVar *s = jsvAsString(data);
    if (s) {
      if (jsvGetBoolAndUnLock(jsvObjectGetChildIfExists(httpServerResponseVar, HTTP_NAME_CHUNKED))) {
        // If we asked to send 'chunked' data, we need to wrap it up,
        // prefixed with the length
        jsvAppendPrintf(sendData, "%x\r\n%v\r\n", jsvGetStringLength(s), s);
      } else {
        jsvAppendStringVarComplete(sendData,s);
      }
    }
    jsvUnLock(s);
  }
  DBG("serverResponseWrite %v\n", sendData);
  jsvUnLock(sendData);
}

void serverResponseEnd(JsVar *httpServerResponseVar) {
  JsVar *finalData = 0;
  if (jsvGetBoolAndUnLock(jsvObjectGetChildIfExists(httpServerResponseVar, HTTP_NAME_CHUNKED))) {
    // If we were asked to send 'chunked' data, we need to finish up
    finalData = jsvNewFromString("");
  }
  serverResponseWrite(httpServerResponseVar, finalData); // force connection->sendData to be created even if data not called
  jsvUnLock(finalData);

  jsvObjectSetChildAndUnLock(httpServerResponseVar, HTTP_NAME_CLOSE, jsvNewFromBool(true));
}

