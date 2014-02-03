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
#include "httpserver.h"
#define INVALID_SOCKET ((SOCKET)(-1))
#define SOCKET_ERROR (-1)
#include "jsparse.h"
#include "jsinteractive.h"
#include "../network.h"

#include <string.h> // for memset

#ifdef USE_CC3000
 #include "socket.h"
 #include "cc3000_common.h"
 #include "jswrap_cc3000.h"

 #define MSG_NOSIGNAL 0x4000 /* don't raise SIGPIPE */ // IGNORED ANYWAY!
#else
 #include <sys/stat.h>
 #include <errno.h>

 #define closesocket(SOCK) close(SOCK)
#endif

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
#define HTTP_NAME_CLOSE "close"
#define HTTP_NAME_ON_CONNECT "#onconnect"
#define HTTP_NAME_ON_DATA "#ondata"
#define HTTP_NAME_ON_CLOSE "#onclose"

#define HTTP_ARRAY_HTTP_CLIENT_CONNECTIONS JS_HIDDEN_CHAR_STR"HttpCC"
#define HTTP_ARRAY_HTTP_SERVERS JS_HIDDEN_CHAR_STR"HttpS"
#define HTTP_ARRAY_HTTP_SERVER_CONNECTIONS JS_HIDDEN_CHAR_STR"HttpSC"

// -----------------------------
static void httpError(const char *msg) {
  //WSAGetLastError?
  jsError(msg);
}

static void httpAppendHeaders(JsVar *string, JsVar *headerObject) {
  // append headers
  JsvObjectIterator it;
  jsvObjectIteratorNew(&it, headerObject);
  while (jsvObjectIteratorHasElement(&it)) {
    JsVar *k = jsvAsString(jsvObjectIteratorGetKey(&it), true);
    JsVar *v = jsvAsString(jsvObjectIteratorGetValue(&it), true);
    jsvAppendStringVarComplete(string, k);
    jsvAppendString(string, ": ");
    jsvAppendStringVarComplete(string, v);
    jsvAppendString(string, "\r\n");
    jsvUnLock(k);
    jsvUnLock(v);
    jsvObjectIteratorNext(&it);
  }
  jsvObjectIteratorFree(&it);
  // free headers
}

static JsVar *httpGetArray(const char *name, bool create) {
  JsVar *arrayName = jsvFindChildFromString(jsiGetParser()->root, name, create);
  JsVar *arr = jsvSkipName(arrayName);
  if (!arr && create) {
    arr = jsvNewWithFlags(JSV_ARRAY);
    jsvSetValueOfName(arrayName, arr);
  }
  jsvUnLock(arrayName);
  return arr;
}
// -----------------------------

void httpInit() {
#ifdef WIN32
  // Init winsock 1.1
  WORD sockVersion;
  WSADATA wsaData;
  sockVersion = MAKEWORD(1, 1);
  WSAStartup(sockVersion, &wsaData);
#endif
}

void _httpServerConnectionKill(JsVar *connection) {
  SOCKET socket = (SOCKET)jsvGetIntegerAndUnLock(jsvObjectGetChild(connection,HTTP_NAME_SOCKET,0))-1; // so -1 if undefined
    if (socket!=INVALID_SOCKET) closesocket(socket);
}

void _httpClientConnectionKill(JsVar *connection) {
  SOCKET socket = (SOCKET)jsvGetIntegerAndUnLock(jsvObjectGetChild(connection,HTTP_NAME_SOCKET,0))-1; // so -1 if undefined
  if (socket!=INVALID_SOCKET) closesocket(socket);
}

void httpKill() {
  // shut down connections
  {
      JsVar *arr = httpGetArray(HTTP_ARRAY_HTTP_SERVER_CONNECTIONS,false);
      if (arr) {
        JsvArrayIterator it;
        jsvArrayIteratorNew(&it, arr);
        while (jsvArrayIteratorHasElement(&it)) {
          JsVar *connection = jsvArrayIteratorGetElement(&it);
          _httpServerConnectionKill(connection);
          jsvUnLock(connection);
          jsvArrayIteratorNext(&it);
        }
        jsvArrayIteratorFree(&it);
        jsvRemoveAllChildren(arr);
        jsvUnLock(arr);
      }
    }
  {
    JsVar *arr = httpGetArray(HTTP_ARRAY_HTTP_CLIENT_CONNECTIONS,false);
    if (arr) {
      JsvArrayIterator it;
      jsvArrayIteratorNew(&it, arr);
      while (jsvArrayIteratorHasElement(&it)) {
        JsVar *connection = jsvArrayIteratorGetElement(&it);
        _httpClientConnectionKill(connection);
        jsvUnLock(connection);
        jsvArrayIteratorNext(&it);
      }
      jsvArrayIteratorFree(&it);
      jsvRemoveAllChildren(arr);
      jsvUnLock(arr);
    }
  }
  // shut down our listeners, unlock objects, free data
  {
      JsVar *arr = httpGetArray(HTTP_ARRAY_HTTP_SERVERS,false);
      if (arr) {
        JsvArrayIterator it;
        jsvArrayIteratorNew(&it, arr);
        while (jsvArrayIteratorHasElement(&it)) {
          JsVar *connection = jsvArrayIteratorGetElement(&it);
          SOCKET socket = (SOCKET)jsvGetIntegerAndUnLock(jsvObjectGetChild(connection,HTTP_NAME_SOCKET,0))-1; // so -1 if undefined
          if (socket!=INVALID_SOCKET) closesocket(socket);
          jsvUnLock(connection);
          jsvArrayIteratorNext(&it);
        }
        jsvArrayIteratorFree(&it);
        jsvRemoveAllChildren(arr);
        jsvUnLock(arr);
      }
    }
#ifdef WIN32
   // Shutdown Winsock
   WSACleanup();
#endif
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
        if (lineNumber>0 && colonPos>lastLineStart && lastLineStart<strIdx) {
          JsVar *hVal = jsvNewFromEmptyString();
          if (hVal)
            jsvAppendStringVar(hVal, *receiveData, colonPos+2, strIdx-(colonPos+2));
          JsVar *hKey = jsvNewFromEmptyString();
          if (hKey) {
            jsvMakeIntoVariableName(hKey, hVal);
            jsvAppendStringVar(hKey, *receiveData, lastLineStart, colonPos-lastLineStart);
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
    JsVar *vMethod = jsvNewFromEmptyString();
    if (vMethod) {
      jsvAppendStringVar(vMethod, *receiveData, 0, firstSpace);
      jsvUnLock(jsvAddNamedChild(objectForData, vMethod, "method"));
      jsvUnLock(vMethod);
    }
    JsVar *vUrl = jsvNewFromEmptyString();
    if (vUrl) {
      jsvAppendStringVar(vUrl, *receiveData, firstSpace+1, secondSpace-(firstSpace+1));
      jsvUnLock(jsvAddNamedChild(objectForData, vUrl, "url"));
      jsvUnLock(vUrl);
    }
  }
  // strip out the header
  JsVar *afterHeaders = jsvNewFromEmptyString();
  if (!afterHeaders) return true;
  jsvAppendStringVar(afterHeaders, *receiveData, headerEnd, JSVAPPENDSTRINGVAR_MAXLENGTH);
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

bool _http_send(SOCKET sckt, JsVar **sendData) {
  char buf[64];

  int a=1;
  int len = (int)jsvGetStringLength(*sendData);
  if (len>0) {
    size_t bufLen = httpStringGet(*sendData, buf, sizeof(buf));
    a = (int)send(sckt,buf,bufLen, MSG_NOSIGNAL);
    JsVar *newSendData = 0;
    if (a!=len) {
      newSendData = jsvNewFromEmptyString();
      jsvAppendStringVar(newSendData, *sendData, a, JSVAPPENDSTRINGVAR_MAXLENGTH);
    }
    jsvUnLock(*sendData);
    *sendData = newSendData;
  }
  if (a<=0) {
    httpError("Socket error while sending");
    return false;
  } return true;
}

void httpServerConnectionsIdle() {
  char buf[64];

  JsVar *arr = httpGetArray(HTTP_ARRAY_HTTP_SERVER_CONNECTIONS,false);
  if (!arr) return;
  JsvArrayIterator it;
  jsvArrayIteratorNew(&it, arr);
  while (jsvArrayIteratorHasElement(&it)) {
    JsVar *connection = jsvArrayIteratorGetElement(&it);
    JsVar *connectReponse = jsvObjectGetChild(connection,HTTP_NAME_RESPONSE_VAR,0);
    SOCKET sckt = (SOCKET)jsvGetIntegerAndUnLock(jsvObjectGetChild(connection,HTTP_NAME_SOCKET,0))-1; // so -1 if undefined
    bool closeConnectionNow = jsvGetBoolAndUnLock(jsvObjectGetChild(connection, HTTP_NAME_CLOSENOW, false));
    bool hadData = false;
    // TODO: look for unreffed connections?
    fd_set s;
    FD_ZERO(&s);
    FD_SET(sckt,&s);
    // check for waiting clients
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    int n = select(sckt+1,&s,NULL,NULL,&timeout);
    if (n==SOCKET_ERROR) {
      // we probably disconnected so just get rid of this
      closeConnectionNow = true;
    } else if (n>0) {
      hadData = true;
      // receive data
      int num = (int)recv(sckt,buf,sizeof(buf),0);
      // add it to our request string
      if (num>0) {
        JsVar *receiveData = jsvObjectGetChild(connection,HTTP_NAME_RECEIVE_DATA,0);
        if (!receiveData) {
          receiveData = jsvNewFromEmptyString();
          jsvObjectSetChild(connection,HTTP_NAME_RECEIVE_DATA,receiveData);
        }
        if (receiveData) {
          jsvAppendStringBuf(receiveData, buf, num);
          bool hadHeaders = jsvGetBoolAndUnLock(jsvObjectGetChild(connection,HTTP_NAME_HAD_HEADERS,0));
          if (!hadHeaders && httpParseHeaders(&receiveData, connection, true)) {
            hadHeaders = true;
            jsvUnLock(jsvObjectSetChild(connection, HTTP_NAME_HAD_HEADERS, jsvNewFromBool(hadHeaders)));
            JsVar *resVar = jsvObjectGetChild(connection,HTTP_NAME_RESPONSE_VAR,0);
            JsVar *server = jsvObjectGetChild(connection,HTTP_NAME_SERVER_VAR,0);
            jsiQueueObjectCallbacks(server, HTTP_NAME_ON_CONNECT, connection, resVar);
            jsvUnLock(server);
            jsvUnLock(resVar);
          }
          jsvUnLock(receiveData);
        }
      }
    }

    // send data if possible
    JsVar *sendData = jsvObjectGetChild(connectReponse,HTTP_NAME_SEND_DATA,0);
    if (sendData) {
      fd_set writefds;
      FD_ZERO(&writefds);
      FD_SET(sckt, &writefds);
      struct timeval time;
      time.tv_sec = 0;
      time.tv_usec = 0;
      int n = select(sckt+1, 0, &writefds, 0, &time);
      if (n==SOCKET_ERROR ) {
         // we probably disconnected so just get rid of this
        closeConnectionNow = true;
      } else if (FD_ISSET(sckt, &writefds)) {
        if (!_http_send(sckt, &sendData))
          closeConnectionNow = true;
      }
      jsvObjectSetChild(connectReponse, HTTP_NAME_SEND_DATA, sendData); // _http_send prob updated sendData
    } else {
#ifdef USE_CC3000
      // nothing to send, nothing to receive, and closed...
      if (!hadData && cc3000_socket_has_closed(sckt))
        closeConnectionNow = true;
#endif
    }
    if (jsvGetBoolAndUnLock(jsvObjectGetChild(connectReponse,HTTP_NAME_CLOSE,0)) && !sendData)
      closeConnectionNow = true;
    jsvUnLock(sendData);
    if (closeConnectionNow) {
      JsVar *resVar = jsvObjectGetChild(connection,HTTP_NAME_RESPONSE_VAR,0);
      jsiQueueObjectCallbacks(resVar, HTTP_NAME_ON_CLOSE, 0, 0);
      jsvUnLock(resVar);

      _httpServerConnectionKill(connection);
      JsVar *connectionName = jsvArrayIteratorGetIndex(&it);
      jsvArrayIteratorNext(&it);
      jsvRemoveChild(arr, connectionName);
      jsvUnLock(connectionName);
    } else
      jsvArrayIteratorNext(&it);
    jsvUnLock(connection);
    jsvUnLock(connectReponse);
  }
  jsvArrayIteratorFree(&it);
  jsvUnLock(arr);
}



void httpClientConnectionsIdle() {
  char buf[64];

  JsVar *arr = httpGetArray(HTTP_ARRAY_HTTP_CLIENT_CONNECTIONS,false);
  if (!arr) return;

  JsvArrayIterator it;
  jsvArrayIteratorNew(&it, arr);
  while (jsvArrayIteratorHasElement(&it)) {
    JsVar *connection = jsvArrayIteratorGetElement(&it);
    bool closeConnectionNow = jsvGetBoolAndUnLock(jsvObjectGetChild(connection, HTTP_NAME_CLOSENOW, false));
    SOCKET sckt = (SOCKET)jsvGetIntegerAndUnLock(jsvObjectGetChild(connection,HTTP_NAME_SOCKET,0))-1; // so -1 if undefined
    bool hadHeaders = jsvGetBoolAndUnLock(jsvObjectGetChild(connection,HTTP_NAME_HAD_HEADERS,0));
    JsVar *receiveData = jsvObjectGetChild(connection,HTTP_NAME_RECEIVE_DATA,0);

    /* We do this up here because we want to wait until we have been once
     * around the idle loop (=callbacks have been executed) before we run this */
    if (hadHeaders && receiveData) {
      JsVar *resVar = jsvObjectGetChild(connection,HTTP_NAME_RESPONSE_VAR,0);
      jsiQueueObjectCallbacks(resVar, HTTP_NAME_ON_DATA, receiveData, 0);
      jsvUnLock(resVar);
      // clear - because we have issued a callback
      jsvObjectSetChild(connection,HTTP_NAME_RECEIVE_DATA,0);
    }

    if (sckt!=INVALID_SOCKET) {
      JsVar *sendData = jsvObjectGetChild(connection,HTTP_NAME_SEND_DATA,0);
      // send data if possible
      if (sendData) {
        // this will wait to see if we can write any more, but ALSO
        // will wait for connection
        fd_set writefds;
        FD_ZERO(&writefds);
        FD_SET(sckt, &writefds);
        struct timeval time;
        time.tv_sec = 0;
        time.tv_usec = 0;
        int n = select(sckt+1, 0, &writefds, 0, &time);
        if (n==SOCKET_ERROR ) {
           // we probably disconnected so just get rid of this
          closeConnectionNow = true;
        } else if (FD_ISSET(sckt, &writefds)) {
          if (!_http_send(sckt, &sendData))
            closeConnectionNow = true;
          jsvObjectSetChild(connection, HTTP_NAME_SEND_DATA, sendData); // _http_send prob updated sendData
        }
#ifdef USE_CC3000
      } else { // When in CC3000, write then read (FIXME)
#else
      } // When in Linux, just read and write at the same time
      {
#endif
        // Now receive data
        fd_set s;
        FD_ZERO(&s);
        FD_SET(sckt,&s);
        // check for waiting clients
        struct timeval timeout;
        timeout.tv_sec = 0;
  #ifdef USE_CC3000
        timeout.tv_usec = 5000; // 5 millisec
  #else
        timeout.tv_usec = 0;
  #endif
        int n = select(sckt+1,&s,NULL,NULL,&timeout);
        if (n==SOCKET_ERROR) {
          // we probably disconnected so just get rid of this
          closeConnectionNow = true;
        } else if (n>0) {
          // receive data
          int num = (int)recv(sckt,buf,sizeof(buf),0);
          // add it to our request string
          if (num>0) {
            if (!receiveData) {
              receiveData = jsvNewFromEmptyString();
              jsvObjectSetChild(connection, HTTP_NAME_RECEIVE_DATA, receiveData);
            }
            if (receiveData) { // could be out of memory
              jsvAppendStringBuf(receiveData, buf, num);
              if (!hadHeaders) {
                JsVar *resVar = jsvObjectGetChild(connection,HTTP_NAME_RESPONSE_VAR,0);
                if (httpParseHeaders(&receiveData, resVar, false)) {
                  hadHeaders = true;
                  jsvUnLock(jsvObjectSetChild(connection, HTTP_NAME_HAD_HEADERS, jsvNewFromBool(hadHeaders)));
                  jsiQueueObjectCallbacks(connection, HTTP_NAME_ON_CONNECT, resVar, 0);
                }
                jsvUnLock(resVar);
                jsvObjectSetChild(connection, HTTP_NAME_RECEIVE_DATA, receiveData);
              }
            }
          } else if (num==0) {
            // select says data, but recv says 0 means connection is closed
            closeConnectionNow = true;
          }
        } else {
  #ifdef USE_CC3000
          // Nothing to send or receive, and closed
          if (!sendData && cc3000_socket_has_closed(sckt))
            closeConnectionNow = true;
  #endif
        }
      }
      jsvUnLock(sendData);
    }
    jsvUnLock(receiveData);
    if (closeConnectionNow) {
      JsVar *resVar = jsvObjectGetChild(connection,HTTP_NAME_RESPONSE_VAR,0);
      jsiQueueObjectCallbacks(resVar, HTTP_NAME_ON_CLOSE, 0, 0);
      jsvUnLock(resVar);

      _httpClientConnectionKill(connection);
      JsVar *connectionName = jsvArrayIteratorGetIndex(&it);
      jsvArrayIteratorNext(&it);
      jsvRemoveChild(arr, connectionName);
      jsvUnLock(connectionName);
    } else
      jsvArrayIteratorNext(&it);
    jsvUnLock(connection);
  }
  jsvUnLock(arr);
}


void httpIdle() {
  if (networkState != NETWORKSTATE_ONLINE) return;
  JsVar *arr = httpGetArray(HTTP_ARRAY_HTTP_SERVERS,false);
  if (arr) {
    JsvArrayIterator it;
    jsvArrayIteratorNew(&it, arr);
    while (jsvArrayIteratorHasElement(&it)) {
      JsVar *server = jsvArrayIteratorGetElement(&it);
      SOCKET socket = (SOCKET)jsvGetIntegerAndUnLock(jsvObjectGetChild(server,HTTP_NAME_SOCKET,0))-1; // so -1 if undefined
  #ifndef USE_CC3000
      // TODO: look for unreffed servers?
      fd_set s;
      FD_ZERO(&s);
      FD_SET(socket,&s);
      // check for waiting clients
      struct timeval timeout;
      timeout.tv_sec = 0;
      timeout.tv_usec = 0;
      int n = select(socket+1,&s,NULL,NULL,&timeout);
  #else
      /* CC3000 works a different way - we set accept as nonblocking,
       * and then we just call it and see if it works or not...
       */
      int n=1;
  #endif
      while (n-->0) {
        // we have a client waiting to connect... try to connect and see what happens
  #ifdef USE_CC3000
        // CC3000's implementation doesn't accept NULL like everyone else's :(
        sockaddr addr;
        socklen_t addrlen = sizeof(addr);
        int theClient = accept(socket,&addr,&addrlen); 
  #else
        int theClient = accept(socket,0,0);
  #endif
        if (theClient > -1) {
          JsVar *req = jspNewObject(jsiGetParser(), 0, "httpSRq");
          JsVar *res = jspNewObject(jsiGetParser(), 0, "httpSRs");
          if (res && req) { // out of memory?
            JsVar *arr = httpGetArray(HTTP_ARRAY_HTTP_SERVER_CONNECTIONS, true);
            if (arr) {
              jsvArrayPush(arr, req);
              jsvUnLock(arr);
            }
            jsvObjectSetChild(req, HTTP_NAME_RESPONSE_VAR, res);
            jsvObjectSetChild(req, HTTP_NAME_SERVER_VAR, server);
            jsvUnLock(jsvObjectSetChild(req, HTTP_NAME_SOCKET, jsvNewFromInteger(theClient+1)));
            // on response
            jsvUnLock(jsvObjectSetChild(res, HTTP_NAME_CODE, jsvNewFromInteger(200)));
            jsvUnLock(jsvObjectSetChild(res, HTTP_NAME_HEADERS, jsvNewWithFlags(JSV_OBJECT)));
          }
          jsvUnLock(req);
          jsvUnLock(res);
          //add(new CNetworkConnect(theClient, this));
          // add to service queue
        }
      }
      jsvUnLock(server);
      jsvArrayIteratorNext(&it);
    }
    jsvArrayIteratorFree(&it);
    jsvUnLock(arr);
  }

  httpServerConnectionsIdle();
  httpClientConnectionsIdle();
}

// -----------------------------

JsVar *httpServerNew(JsVar *callback) {
  SOCKET sckt = socket(AF_INET,           // Go over TCP/IP
                       SOCK_STREAM,       // This is a stream-oriented socket
                       IPPROTO_TCP);      // Use TCP rather than UDP
  if (sckt == INVALID_SOCKET) {
    httpError("socket creation failed");
    return 0;
  }

  JsVar *arr = httpGetArray(HTTP_ARRAY_HTTP_SERVERS, true);
  if (!arr) return 0; // out of memory

  JsVar *server = jspNewObject(jsiGetParser(),0,"httpSrv");
  if (!server) {
    jsvUnLock(arr);
    return 0; // out of memory
  }

  jsvObjectSetChild(server, HTTP_NAME_ON_CONNECT, callback); // no unlock needed
  jsvUnLock(jsvObjectSetChild(server, HTTP_NAME_SOCKET, jsvNewFromInteger(sckt+1)));

#ifndef USE_CC3000
  int optval = 1;
  if (setsockopt(sckt,SOL_SOCKET,SO_REUSEADDR,(const char *)&optval,sizeof(optval)) < 0)
#else
  int optval = SOCK_ON;
  if (setsockopt(sckt,SOL_SOCKET,SOCKOPT_ACCEPT_NONBLOCK,(const char *)&optval,sizeof(optval)) < 0)
#endif
    jsWarn("setsockopt failed\n");

  // add to list of servers
  jsvArrayPush(arr, server);
  jsvUnLock(arr);

  return server;
}

void httpServerListen(JsVar *server, int port) {
  SOCKET sckt = (SOCKET)(jsvGetIntegerAndUnLock(jsvObjectGetChild(server, HTTP_NAME_SOCKET, 0))-1);

  int nret;
  sockaddr_in serverInfo;
  memset(&serverInfo, 0, sizeof(serverInfo));
  serverInfo.sin_family = AF_INET;
  //serverInfo.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // allow only LOCAL clients to connect
  serverInfo.sin_addr.s_addr = INADDR_ANY; // allow anyone to connect
  serverInfo.sin_port = htons((unsigned short)port); // port
  nret = bind(sckt, (struct sockaddr*)&serverInfo, sizeof(serverInfo));
  if (nret == SOCKET_ERROR) {
    httpError("socket bind failed");
    closesocket(sckt);
    return;
  }

  // Make the socket listen
  nret = listen(sckt, 10); // 10 connections (but this ignored on CC30000)
  if (nret == SOCKET_ERROR) {
    httpError("socket listen failed");
    closesocket(sckt);
    return;
  }
}


JsVar *httpClientRequestNew(JsVar *options, JsVar *callback) {
  JsVar *arr = httpGetArray(HTTP_ARRAY_HTTP_CLIENT_CONNECTIONS,true);
  if (!arr) return 0;
  JsVar *req = jspNewObject(jsiGetParser(), 0, "httpCRq");
  JsVar *res = jspNewObject(jsiGetParser(), 0, "httpCRs");
  if (res && req) { // out of memory?
   jsvUnLock(jsvAddNamedChild(req, callback, HTTP_NAME_ON_CONNECT));

   jsvArrayPush(arr, req);
   jsvObjectSetChild(req, HTTP_NAME_RESPONSE_VAR, res);
   jsvObjectSetChild(req, HTTP_NAME_OPTIONS_VAR, options);
  }
  jsvUnLock(res);
  jsvUnLock(arr);
  return req;
}

void httpClientRequestWrite(JsVar *httpClientReqVar, JsVar *data) {
  // Append data to sendData
  JsVar *sendData = jsvObjectGetChild(httpClientReqVar, HTTP_NAME_SEND_DATA, false);
  if (!sendData) {
    JsVar *options = jsvObjectGetChild(httpClientReqVar, HTTP_NAME_OPTIONS_VAR, false);
    if (options) {
      sendData = jsvNewFromString("");
      JsVar *method = jsvObjectGetChild(options, "method", false);
      JsVar *path = jsvObjectGetChild(options, "path", false);
      jsvAppendPrintf(sendData, "%v %v HTTP/1.0\r\nUser-Agent: Espruino "JS_VERSION"\r\nConnection: close\r\n", method, path);
      jsvUnLock(method);
      jsvUnLock(path);
      JsVar *headers = jsvObjectGetChild(options, "headers", false);
      bool hasHostHeader = false;
      if (jsvIsObject(headers)) {
        JsVar *hostHeader = jsvObjectGetChild(headers, "Host", false);
        hasHostHeader = hostHeader!=0;
        jsvUnLock(hostHeader);
        httpAppendHeaders(sendData, headers);
      }
      jsvUnLock(headers);
      if (!hasHostHeader) {
        JsVar *host = jsvObjectGetChild(options, "host", false);
        JsVarInt port = jsvGetIntegerAndUnLock(jsvObjectGetChild(options, "port", false));
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

void httpClientRequestEnd(JsVar *httpClientReqVar) {
  httpClientRequestWrite(httpClientReqVar, 0); // force sendData to be made

  JsVar *options = jsvObjectGetChild(httpClientReqVar, HTTP_NAME_OPTIONS_VAR, false);

  SOCKET sckt = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (sckt == INVALID_SOCKET) {
     httpError("Unable to create socket\n");
     jsvUnLock(jsvObjectSetChild(httpClientReqVar, HTTP_NAME_CLOSENOW, jsvNewFromBool(true)));
  }
  jsvUnLock(jsvObjectSetChild(httpClientReqVar, HTTP_NAME_SOCKET, jsvNewFromInteger(sckt+1)));


  unsigned short port = (unsigned short)jsvGetIntegerAndUnLock(jsvObjectGetChild(options, "port", false));

#ifdef USE_CC3000
  sockaddr       sin;
  sin.sa_family = AF_INET;
  sin.sa_data[0] = (port & 0xFF00) >> 8;
  sin.sa_data[1] = (port & 0x00FF);
#else
  sockaddr_in       sin;
  sin.sin_family = AF_INET;
  sin.sin_port = htons( port );
#endif

  char hostName[128];
  JsVar *hostNameVar = jsvObjectGetChild(options, "host", false);
  jsvGetString(hostNameVar, hostName, sizeof(hostName));
  jsvUnLock(hostNameVar);

  unsigned long host_addr = 0;
#ifdef USE_CC3000
  gethostbyname(hostName, strlen(hostName), &host_addr);
#else
  struct hostent * host_addr_p = gethostbyname(hostName);
  if (host_addr_p)
    host_addr = *((int*)*host_addr_p->h_addr_list);
#endif
  /* getaddrinfo is newer than this?
  *
  * struct addrinfo * result;
  * error = getaddrinfo("www.example.com", NULL, NULL, &result);
  * if (0 != error)
  *   fprintf(stderr, "error %s\n", gai_strerror(error));
  *
  */
  if(!host_addr) {
    httpError("Unable to locate host");
    jsvUnLock(jsvObjectSetChild(httpClientReqVar, HTTP_NAME_CLOSENOW, jsvNewFromBool(true)));
    jsvUnLock(options);
    return;
  }

  // turn on non-blocking mode
  #ifdef WIN_OS
  u_long n = 1;
  ioctlsocket(sckt,FIONBIO,&n);
  #elif defined(USE_CC3000)
  int param;
  param = SOCK_ON;
  setsockopt(sckt, SOL_SOCKET, SOCKOPT_RECV_NONBLOCK, &param, sizeof(param)); // enable nonblock
  param = 5; // ms
  setsockopt(sckt, SOL_SOCKET, SOCKOPT_RECV_TIMEOUT, &param, sizeof(param)); // set a timeout
  #else
  int flags = fcntl(sckt, F_GETFL);
  if (flags < 0) {
     httpError("Unable to retrieve socket descriptor status flags");
     jsvUnLock(jsvObjectSetChild(httpClientReqVar, HTTP_NAME_CLOSENOW, jsvNewFromBool(true)));
     jsvUnLock(options);
     return;
  }
  if (fcntl(sckt, F_SETFL, flags | O_NONBLOCK) < 0)
     httpError("Unable to set socket descriptor status flags\n");
  #endif

#ifdef USE_CC3000
  sin.sa_data[5] = (host_addr) & 0xFF;  // First octet of destination IP
  sin.sa_data[4] = (host_addr>>8) & 0xFF;   // Second Octet of destination IP
  sin.sa_data[3] = (host_addr>>16) & 0xFF;  // Third Octet of destination IP
  sin.sa_data[2] = (host_addr>>24) & 0xFF;  // Fourth Octet of destination IP
#else
  sin.sin_addr.s_addr = host_addr;
#endif

  //uint32_t a = sin.sin_addr.s_addr;
  //_DEBUG_PRINT( cout<<"Port :"<<sin.sin_port<<", Address : "<< sin.sin_addr.s_addr<<endl);
  int res = connect (sckt,(const struct sockaddr *)&sin, sizeof(sockaddr_in) );
  if (res == SOCKET_ERROR) {
  #ifdef WIN_OS
   int err = WSAGetLastError();
  #else
   int err = errno;
  #endif
   if (err != EINPROGRESS &&
       err != EWOULDBLOCK) {
     httpError("Connect failed\n" );
     jsvUnLock(jsvObjectSetChild(httpClientReqVar, HTTP_NAME_CLOSENOW, jsvNewFromBool(true)));
   }
  }

  jsvUnLock(options);
}


void httpServerResponseWriteHead(JsVar *httpServerResponseVar, int statusCode, JsVar *headers) {
  if (!jsvIsUndefined(headers) && !jsvIsObject(headers)) {
    httpError("Headers sent to writeHead should be an object");
    return;
  }

  jsvUnLock(jsvObjectSetChild(httpServerResponseVar, HTTP_NAME_CODE, jsvNewFromInteger(statusCode)));
  JsVar *sendHeaders = jsvObjectGetChild(httpServerResponseVar, HTTP_NAME_HEADERS, 0);
  if (sendHeaders) {
    if (!jsvIsUndefined(headers)) {
      jsvObjectSetChild(httpServerResponseVar, HTTP_NAME_HEADERS, headers);
    }
    jsvUnLock(sendHeaders);
  } else {
    // headers are set to 0 when they are sent
    httpError("Headers have already been sent");
  }
}


void httpServerResponseData(JsVar *httpServerResponseVar, JsVar *data) {
  // Append data to sendData
  JsVar *sendData = jsvObjectGetChild(httpServerResponseVar, HTTP_NAME_SEND_DATA, 0);
  if (!sendData) {
    // no sendData, so no headers - add them!
    JsVar *sendHeaders = jsvObjectGetChild(httpServerResponseVar, HTTP_NAME_HEADERS, 0);
    if (sendHeaders) {
      sendData = jsvNewFromEmptyString();
      jsvAppendPrintf(sendData, "HTTP/1.0 %d OK\r\nServer: Espruino "JS_VERSION"\r\n", jsvGetIntegerAndUnLock(jsvObjectGetChild(httpServerResponseVar, HTTP_NAME_CODE, 0)));
      httpAppendHeaders(sendData, sendHeaders);
      jsvObjectSetChild(httpServerResponseVar, HTTP_NAME_HEADERS, 0);
      jsvUnLock(sendHeaders);
      // finally add ending newline
      jsvAppendString(sendData, "\r\n");
    } else {
      // we have already sent headers
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

void httpServerResponseEnd(JsVar *httpServerResponseVar) {
  httpServerResponseData(httpServerResponseVar, 0); // force onnection->sendData to be created even if data not called
  jsvUnLock(jsvObjectSetChild(httpServerResponseVar, HTTP_NAME_CLOSE, jsvNewFromBool(true)));
}
