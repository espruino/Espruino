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

#if defined(USE_CC3000) // ------------------------------------------------
 #include "socket.h"
 #include "board_spi.h"
 #include "cc3000_common.h"
 #include "jswrap_cc3000.h"

 #define MSG_NOSIGNAL 0x4000 /* don't raise SIGPIPE */ // IGNORED ANYWAY!
#elif defined(USE_WIZNET) // ------------------------------------------------
 #include "Ethernet/socket.h"
 #define closesocket(SOCK) close(SOCK)
 #define MSG_NOSIGNAL 0x4000 /* don't raise SIGPIPE */ // IGNORED ANYWAY!
 #define send(sock,ptr,len,flags) send(sock,(uint8_t*)(ptr),len) // throw away last arg of send
 #define recv(sock,ptr,len,flags) recv(sock,(uint8_t*)(ptr),len) // throw away last arg of send
 // name resolution
 #include "DNS/dns.h"
 extern uint8_t Server_IP_Addr[4];
#else                     // ------------------------------------------------
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
  JsVar *arrayName = jsvFindChildFromString(execInfo.root, name, create);
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

static void _httpServerConnectionKill(JsVar *connection) {
  if (networkState != NETWORKSTATE_ONLINE) return;
  SOCKET sckt = (SOCKET)jsvGetIntegerAndUnLock(jsvObjectGetChild(connection,HTTP_NAME_SOCKET,0))-1; // so -1 if undefined
  if (sckt!=INVALID_SOCKET) closesocket(sckt);
}

static void _httpClientConnectionKill(JsVar *connection) {
  if (networkState != NETWORKSTATE_ONLINE) return;
  SOCKET sckt = (SOCKET)jsvGetIntegerAndUnLock(jsvObjectGetChild(connection,HTTP_NAME_SOCKET,0))-1; // so -1 if undefined
  if (sckt!=INVALID_SOCKET) closesocket(sckt);
}

static void _httpCloseAllConnections() {
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
           if (networkState == NETWORKSTATE_ONLINE) {
             SOCKET sckt = (SOCKET)jsvGetIntegerAndUnLock(jsvObjectGetChild(connection,HTTP_NAME_SOCKET,0))-1; // so -1 if undefined
             if (sckt!=INVALID_SOCKET) closesocket(sckt);
           }
           jsvUnLock(connection);
           jsvArrayIteratorNext(&it);
         }
         jsvArrayIteratorFree(&it);
         jsvRemoveAllChildren(arr);
         jsvUnLock(arr);
       }
     }
}

void httpKill() {
  _httpCloseAllConnections();
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
    JsVar *vMethod = jsvNewFromEmptyString();
    if (vMethod) {
      jsvAppendStringVar(vMethod, *receiveData, 0, (size_t)firstSpace);
      jsvUnLock(jsvAddNamedChild(objectForData, vMethod, "method"));
      jsvUnLock(vMethod);
    }
    JsVar *vUrl = jsvNewFromEmptyString();
    if (vUrl) {
      jsvAppendStringVar(vUrl, *receiveData, (size_t)(firstSpace+1), (size_t)(secondSpace-(firstSpace+1)));
      jsvUnLock(jsvAddNamedChild(objectForData, vUrl, "url"));
      jsvUnLock(vUrl);
    }
  }
  // strip out the header
  JsVar *afterHeaders = jsvNewFromEmptyString();
  if (!afterHeaders) return true;
  jsvAppendStringVar(afterHeaders, *receiveData, (size_t)headerEnd, JSVAPPENDSTRINGVAR_MAXLENGTH);
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
    if (a!=len && a>0) {
      newSendData = jsvNewFromStringVar(*sendData, (size_t)a, JSVAPPENDSTRINGVAR_MAXLENGTH);
    }
    jsvUnLock(*sendData);
    *sendData = newSendData;
  }
#if !defined(USE_WIZNET)
  if (a<=0) {
#else
  if (a<0) { // could be SOCK_BUSY(0) which is ok
#endif
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

#if !defined(USE_WIZNET)
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
      // receive data
      int num = (int)recv(sckt,buf,sizeof(buf),0);
#else // defined(USE_WIZNET)
    // receive data - if none available it'll just return SOCK_BUSY
    int num = (int)recv(sckt,buf,sizeof(buf),0);
    if (num<0 && num!=SOCK_BUSY) {
      // we probably disconnected so just get rid of this
      closeConnectionNow = true;
    } else {
#endif
      hadData = true;
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
#if !defined(USE_WIZNET)
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
#else // defined(USE_WIZNET)
      {
#endif
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

    if (!closeConnectionNow && sckt!=INVALID_SOCKET) {
      JsVar *sendData = jsvObjectGetChild(connection,HTTP_NAME_SEND_DATA,0);
      // send data if possible
      if (sendData) {
#if !defined(USE_WIZNET)
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
#else // defined(USE_WIZNET)
        {
#endif
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
#if !defined(USE_WIZNET)
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
#else // defined(USE_WIZNET)
        // receive data - if none available it'll just return SOCK_BUSY
        int num = (int)recv(sckt,buf,sizeof(buf),0);
        if (num<0 && num!=SOCK_BUSY) {
          // we probably disconnected so just get rid of this
          closeConnectionNow = true;
        } else {
#endif
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
          }
#if !defined(USE_WIZNET) // no select on WIZnet, so this is not an issue :)
          else if (num==0) {
            // select says data, but recv says 0 means connection is closed
            closeConnectionNow = true;
          }
#endif
        }
  #ifdef USE_CC3000
        else {
          // Nothing to send or receive, and closed
          if (!sendData && cc3000_socket_has_closed(sckt))
            closeConnectionNow = true;
        }
#endif
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

static void httpCheckAndRecover() {
#ifdef USE_CC3000
  if (jspIsInterrupted()) {
    jsiConsolePrint("Looks like CC3000 has died again. Power cycling...\n");
    jspSetInterrupted(false);
    // remove all existing connections
    networkState = NETWORKSTATE_OFFLINE; // ensure we don't try and send the CC3k anything
    _httpCloseAllConnections();
    // power cycle
    JsVar *wlan = jsvObjectGetChild(execInfo.root, CC3000_OBJ_NAME, 0);
    if (wlan) {
      jswrap_wlan_reconnect(wlan);
      jsvUnLock(wlan);
    } else jsErrorInternal("No CC3000 object!\n");
  }
#endif
}


void httpIdle() {
#ifdef USE_CC3000
  cc3000_spi_check();
#endif
  if (networkState != NETWORKSTATE_ONLINE) {
    // clear all clients and servers
    _httpCloseAllConnections();
    return;
  }
  JsVar *arr = httpGetArray(HTTP_ARRAY_HTTP_SERVERS,false);
  if (arr) {
    JsvArrayIterator it;
    jsvArrayIteratorNew(&it, arr);
    while (jsvArrayIteratorHasElement(&it)) {
      JsVar *server = jsvArrayIteratorGetElement(&it);
      SOCKET sckt = (SOCKET)jsvGetIntegerAndUnLock(jsvObjectGetChild(server,HTTP_NAME_SOCKET,0))-1; // so -1 if undefined

  #if !defined(USE_CC3000) && !defined(USE_WIZNET)
      // TODO: look for unreffed servers?
      fd_set s;
      FD_ZERO(&s);
      FD_SET(sckt,&s);
      // check for waiting clients
      struct timeval timeout;
      timeout.tv_sec = 0;
      timeout.tv_usec = 0;
      int n = select(sckt+1,&s,NULL,NULL,&timeout);
  #else
      /* CC3000/WIZnet works a different way - we set accept as nonblocking,
       * and then we just call it and see if it works or not...
       */
      int n=1;
  #endif
      while (n-->0) {
        // we have a client waiting to connect... try to connect and see what happens
  #if defined(USE_CC3000)
        // CC3000's implementation doesn't accept NULL like everyone else's :(
        sockaddr addr;
        socklen_t addrlen = sizeof(addr);
        int theClient = accept(sckt,&addr,&addrlen);
  #elif defined(USE_WIZNET)
        // WIZnet's implementation doesn't use accept, it uses listen
        int theClient = listen(sckt);
        if (theClient==SOCK_OK) theClient = sckt; // we deal with the client on the same socket
  #else
        int theClient = accept(sckt,0,0);
  #endif
        if (theClient > -1) {
          JsVar *req = jspNewObject(0, "httpSRq");
          JsVar *res = jspNewObject(0, "httpSRs");
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
  httpCheckAndRecover();
}

// -----------------------------

JsVar *httpServerNew(JsVar *callback) {
#if !defined(USE_WIZNET)
  SOCKET sckt = socket(AF_INET,           // Go over TCP/IP
                       SOCK_STREAM,       // This is a stream-oriented socket
                       IPPROTO_TCP);      // Use TCP rather than UDP
  if (sckt == INVALID_SOCKET) {
    httpError("socket creation failed");
    return 0;
  }
#endif

  JsVar *arr = httpGetArray(HTTP_ARRAY_HTTP_SERVERS, true);
  if (!arr) return 0; // out of memory

  JsVar *server = jspNewObject(0, "httpSrv");
  if (!server) {
    jsvUnLock(arr);
    return 0; // out of memory
  }

  jsvObjectSetChild(server, HTTP_NAME_ON_CONNECT, callback); // no unlock needed
#if !defined(USE_WIZNET)
  jsvUnLock(jsvObjectSetChild(server, HTTP_NAME_SOCKET, jsvNewFromInteger(sckt+1)));

#if !defined(USE_CC3000)
  int optval = 1;
  if (setsockopt(sckt,SOL_SOCKET,SO_REUSEADDR,(const char *)&optval,sizeof(optval)) < 0)
#else
  int optval = SOCK_ON;
  if (setsockopt(sckt,SOL_SOCKET,SOCKOPT_ACCEPT_NONBLOCK,(const char *)&optval,sizeof(optval)) < 0)
#endif
    jsWarn("setsockopt failed\n");
#endif

  // add to list of servers
  jsvArrayPush(arr, server);
  jsvUnLock(arr);

  return server;
}

void httpServerListen(JsVar *server, int port) {
#if !defined(USE_WIZNET)
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
#else
  SOCKET sckt = socket(0/*SOCKET NUMBER FIXME*/, Sn_MR_TCP, port, SF_IO_NONBLOCK);
  jsvUnLock(jsvObjectSetChild(server, HTTP_NAME_SOCKET, jsvNewFromInteger(sckt+1)));
#endif
}


JsVar *httpClientRequestNew(JsVar *options, JsVar *callback) {
  JsVar *arr = httpGetArray(HTTP_ARRAY_HTTP_CLIENT_CONNECTIONS,true);
  if (!arr) return 0;
  JsVar *req = jspNewObject(0, "httpCRq");
  JsVar *res = jspNewObject(0, "httpCRs");
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

unsigned long parseIPAddress(const char *ip) {
  int n = 0;
  unsigned long addr = 0;
  while (*ip) {
    if (*ip>='0' && *ip<='9') {
      n = n*10 + (*ip-'0');
    } else if (*ip>='.') {
      addr = (addr>>8) | (n<<24);
      n=0;
    } else {
      return 0; // not an ip address
    }
    ip++;
  }
  addr = (addr>>8) | (n<<24);
  return addr;
}

void httpClientRequestEnd(JsVar *httpClientReqVar) {
  httpClientRequestWrite(httpClientReqVar, 0); // force sendData to be made

  JsVar *options = jsvObjectGetChild(httpClientReqVar, HTTP_NAME_OPTIONS_VAR, false);
  unsigned short port = (unsigned short)jsvGetIntegerAndUnLock(jsvObjectGetChild(options, "port", false));

#if defined(USE_CC3000)
  sockaddr       sin;
  sin.sa_family = AF_INET;
  sin.sa_data[0] = (port & 0xFF00) >> 8;
  sin.sa_data[1] = (port & 0x00FF);
#elif defined(USE_WIZNET)
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
#if defined(USE_CC3000)
  gethostbyname(hostName, strlen(hostName), &host_addr);
#elif defined(USE_WIZNET)
  host_addr = parseIPAddress(hostName); // first try and simply parse the IP address
  if (!host_addr && dns_query(0, 7/*DNS SOCKET FIXME*/, (uint8_t*)hostName) == 1) {
    host_addr = *(unsigned long*)&Server_IP_Addr[0];
  }
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
    httpCheckAndRecover();
    return;
  }


#if !defined(USE_WIZNET)
  SOCKET sckt = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
#else
  SOCKET sckt = socket(1/*SOCKET NUMBER FIXME*/, Sn_MR_TCP, port, SF_IO_NONBLOCK);
#endif
  if (sckt == INVALID_SOCKET) {
     httpError("Unable to create socket\n");
     jsvUnLock(jsvObjectSetChild(httpClientReqVar, HTTP_NAME_CLOSENOW, jsvNewFromBool(true)));
     jsvUnLock(options);
     httpCheckAndRecover();
     return;
  }

  jsvUnLock(jsvObjectSetChild(httpClientReqVar, HTTP_NAME_SOCKET, jsvNewFromInteger(sckt+1)));

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
  #elif defined(USE_WIZNET)
  // ...
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

#if defined(USE_CC3000)
  sin.sa_data[5] = (host_addr) & 0xFF;  // First octet of destination IP
  sin.sa_data[4] = (host_addr>>8) & 0xFF;   // Second Octet of destination IP
  sin.sa_data[3] = (host_addr>>16) & 0xFF;  // Third Octet of destination IP
  sin.sa_data[2] = (host_addr>>24) & 0xFF;  // Fourth Octet of destination IP
#elif defined(USE_WIZNET)
#else
  sin.sin_addr.s_addr = host_addr;
#endif

  //uint32_t a = sin.sin_addr.s_addr;
  //_DEBUG_PRINT( cout<<"Port :"<<sin.sin_port<<", Address : "<< sin.sin_addr.s_addr<<endl);
#ifdef USE_WIZNET
  int res = connect(sckt,(uint8_t*)&host_addr, port);
#else
  int res = connect(sckt,(const struct sockaddr *)&sin, sizeof(sockaddr_in) );
#endif

  if (res == SOCKET_ERROR) {
  #ifdef WIN_OS
   int err = WSAGetLastError();
  #elif defined(USE_WIZNET)
   int err = res;
  #else
   int err = errno;
  #endif
  #if !defined(USE_WIZNET)
   if (err != EINPROGRESS &&
       err != EWOULDBLOCK) {
  #else
   {
  #endif
     httpError("Connect failed\n" );
     jsvUnLock(jsvObjectSetChild(httpClientReqVar, HTTP_NAME_CLOSENOW, jsvNewFromBool(true)));
   }
  }

  jsvUnLock(options);

  httpCheckAndRecover();
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
