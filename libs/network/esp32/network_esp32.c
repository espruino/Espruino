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
 * Implementation of JsNetwork for ESP32 - cloned from linux
 * ----------------------------------------------------------------------------
 */
#include "network_esp32.h"

#include "network.h"
#include <string.h> // for memset

#define INVALID_SOCKET ((SOCKET)(-1))
#define SOCKET_ERROR (-1)

 #include <sys/stat.h>
 #include <errno.h>
#ifdef WIN32
 #include <winsock.h>
#else
 #ifndef ESP_PLATFORM
  #include <sys/select.h>
  #include <arpa/inet.h>
  #include <netinet/in.h>
  #include <resolv.h>
 #endif
 #include <sys/socket.h>
 #include <netdb.h>
 #include <unistd.h>
 #include <fcntl.h>
 #include <stdio.h>

 typedef struct sockaddr_in sockaddr_in;
 typedef int SOCKET;
#endif

 #define closesocket(SOCK) close(SOCK)


/// Get an IP address from a name. Sets out_ip_addr to 0 on failure
void net_esp32_gethostbyname(JsNetwork *net, char * hostName, uint32_t* out_ip_addr) {
  NOT_USED(net);
  struct hostent * host_addr_p = gethostbyname(hostName);
  if (host_addr_p)
    *out_ip_addr = *(uint32_t*)*host_addr_p->h_addr_list;
}

/// Called on idle. Do any checks required for this device
void net_esp32_idle(JsNetwork *net) {
  NOT_USED(net);
}

/// Call just before returning to idle loop. This checks for errors and tries to recover. Returns true if no errors.
bool net_esp32_checkError(JsNetwork *net) {
  NOT_USED(net);
  bool hadErrors = false;
  return hadErrors;
}

/// if host=0, creates a server otherwise creates a client (and automatically connects). Returns >=0 on success
int net_esp32_createsocket(JsNetwork *net, uint32_t host, unsigned short port) {
  NOT_USED(net);
  int sckt = -1;
  if (host!=0) { // ------------------------------------------------- host (=client)
    sockaddr_in       sin;
    sin.sin_family = AF_INET;
    sin.sin_port = htons( port );

    sckt = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sckt<0) return sckt; // error

    // turn on non-blocking mode
    #ifdef WIN_OS
    u_long n = 1;
    ioctlsocket(sckt,FIONBIO,&n);
    #endif

    sin.sin_addr.s_addr = (in_addr_t)host;

    int res = connect(sckt,(struct sockaddr *)&sin, sizeof(sockaddr_in) );

    if (res == SOCKET_ERROR) {
    #ifdef WIN_OS
     int err = WSAGetLastError();
    #else
     int err = errno;
    #endif
     if (err != EINPROGRESS &&
         err != EWOULDBLOCK) {
       jsError("Connect failed (err %d)\n", err );
       closesocket(sckt);
       return -1;
     }
    }

  } else { // ------------------------------------------------- no host (=server)

    sckt = socket(AF_INET,           // Go over TCP/IP
                         SOCK_STREAM,       // This is a stream-oriented socket
                         IPPROTO_TCP);      // Use TCP rather than UDP
    if (sckt == INVALID_SOCKET) {
      jsError("Socket creation failed");
      return 0;
    }
    int optval = 1;
    if (setsockopt(sckt,SOL_SOCKET,SO_REUSEADDR,(const char *)&optval,sizeof(optval)) < 0)
      jsWarn("setsockopt(SO_REUSADDR) failed\n");

    int nret;
    sockaddr_in serverInfo;
    memset(&serverInfo, 0, sizeof(serverInfo));
    serverInfo.sin_family = AF_INET;
    //serverInfo.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // allow only LOCAL clients to connect
    serverInfo.sin_addr.s_addr = INADDR_ANY; // allow anyone to connect
    serverInfo.sin_port = (unsigned short)htons((unsigned short)port); // port
    nret = bind(sckt, (struct sockaddr*)&serverInfo, sizeof(serverInfo));
    if (nret == SOCKET_ERROR) {
      jsError("Socket bind failed");
      closesocket(sckt);
      return -1;
    }

    // Make the socket listen
    nret = listen(sckt, 10); // 10 connections (but this ignored on CC30000)
    if (nret == SOCKET_ERROR) {
      jsError("Socket listen failed, host:%d, port:%d\n",(int)host,(int)port);
      closesocket(sckt);
      return -1;
    }
  }

#ifdef SO_NOSIGPIPE
  // disable SIGPIPE
  int optval = 1;
  if (setsockopt(sckt,SOL_SOCKET,SO_NOSIGPIPE,(const char *)&optval,sizeof(optval))<0)
    jsWarn("setsockopt(SO_NOSIGPIPE) failed\n");
#endif

  return sckt;
}

/// destroys the given socket
void net_esp32_closesocket(JsNetwork *net, int sckt) {
  NOT_USED(net);
  closesocket(sckt);
}

/// If the given server socket can accept a connection, return it (or return < 0)
int net_esp32_accept(JsNetwork *net, int sckt) {
  NOT_USED(net);
  // TODO: look for unreffed servers?
  fd_set s;
  FD_ZERO(&s);
  FD_SET(sckt,&s);
  // check for waiting clients
  struct timeval timeout;
  timeout.tv_sec = 0;
  timeout.tv_usec = 0;
  int n = select(sckt+1,&s,NULL,NULL,&timeout);
  if (n>0) {
    // we have a client waiting to connect... try to connect and see what happens
    int theClient = accept(sckt,0,0);
    return theClient;
  }
  return -1;
}

/// Receive data if possible. returns nBytes on success, 0 on no data, or -1 on failure
int net_esp32_recv(JsNetwork *net, int sckt, void *buf, size_t len) {
  NOT_USED(net);
  int num = 0;
  fd_set s;
  FD_ZERO(&s);
  FD_SET(sckt,&s);
  // check for waiting clients
  struct timeval timeout;
  timeout.tv_sec = 0;
  timeout.tv_usec = 0;
  int n = select(sckt+1,&s,NULL,NULL,&timeout);
  if (n==SOCKET_ERROR) {
    // we probably disconnected
    return -1;
  } else if (n>0) {
    // receive data
    num = (int)recv(sckt,buf,len,0);
    if (num==0) num=-1; // select says data, but recv says 0 means connection is closed
  }

  return num;
}

/// Send data if possible. returns nBytes on success, 0 on no data, or -1 on failure
int net_esp32_send(JsNetwork *net, int sckt, const void *buf, size_t len) {
  NOT_USED(net);
  fd_set writefds;
  FD_ZERO(&writefds);
  FD_SET(sckt, &writefds);
  struct timeval time;
  time.tv_sec = 0;
  time.tv_usec = 0;
  int n = select(sckt+1, 0, &writefds, 0, &time);
  if (n==SOCKET_ERROR ) {
     // we probably disconnected so just get rid of this
    return -1;
  } else if (FD_ISSET(sckt, &writefds)) {
    int flags = 0;
#if !defined(SO_NOSIGPIPE) && defined(MSG_NOSIGNAL)
    flags |= MSG_NOSIGNAL;
#endif
    n = (int)send(sckt, buf, len, flags);
    return n;
  } else
    return 0; // just not ready
}

void netSetCallbacks_esp32(JsNetwork *net) {
  net->idle = net_esp32_idle;
  net->checkError = net_esp32_checkError;
  net->createsocket = net_esp32_createsocket;
  net->closesocket = net_esp32_closesocket;
  net->accept = net_esp32_accept;
  net->gethostbyname = net_esp32_gethostbyname;
  net->recv = net_esp32_recv;
  net->send = net_esp32_send;
  net->chunkSize = 536;
}
