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
 * Implementation of JsNetwork for Linux
 * ----------------------------------------------------------------------------
 */
#include "network.h"
#include "network_linux.h"

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

#if NET_DBG > 0
 #include "jsinteractive.h"
 #define DBG(format, ...) jsiConsolePrintf(format, ## __VA_ARGS__)
#else
#define DBG(format, ...) do { } while(0)
#endif


/// Get an IP address from a name. Sets out_ip_addr to 0 on failure
void net_linux_gethostbyname(JsNetwork *net, char * hostName, uint32_t* out_ip_addr) {
  NOT_USED(net);
  struct hostent * host_addr_p = gethostbyname(hostName);
  if (host_addr_p)
    *out_ip_addr = *(uint32_t*)*host_addr_p->h_addr_list;
}

/// Called on idle. Do any checks required for this device
void net_linux_idle(JsNetwork *net) {
  NOT_USED(net);
}

/// Call just before returning to idle loop. This checks for errors and tries to recover. Returns true if no errors.
bool net_linux_checkError(JsNetwork *net) {
  NOT_USED(net);
  bool hadErrors = false;
  return hadErrors;
}

/// if host=0, creates a server otherwise creates a client (and automatically connects). Returns >=0 on success
int net_linux_createsocket(JsNetwork *net, SocketType socketType, uint32_t host, unsigned short port, JsVar *options) {
  NOT_USED(net);
  int ippProto = socketType & ST_UDP ? IPPROTO_UDP : IPPROTO_TCP;
  int scktType = socketType & ST_UDP ? SOCK_DGRAM : SOCK_STREAM;
  int sckt = -1;

  if (host!=0) { // ------------------------------------------------- host (=client)

    sckt = socket(AF_INET, scktType, ippProto);
    if (sckt<0) {
      jsError("Socket creation failed");
      return sckt; // error
    }

    // turn on non-blocking mode
    #ifdef WIN_OS
    u_long n = 1;
    ioctlsocket(sckt,FIONBIO,&n);
    #endif

    if (scktType == SOCK_DGRAM) { // only for UDP
      // set broadcast
      int optval = 1;
      if (setsockopt(sckt,SOL_SOCKET,SO_BROADCAST,(const char *)&optval,sizeof(optval))<0)
        jsWarn("setsockopt(SO_BROADCAST) failed\n");
      return sckt;
    }

    sockaddr_in       sin;
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = (in_addr_t)host;
    sin.sin_port = htons( port );

    int res = connect(sckt,(struct sockaddr *)&sin, sizeof(sockaddr_in) );

    if (res == SOCKET_ERROR) {
    #ifdef WIN_OS
     int err = WSAGetLastError();
    #else
     int err = errno;
    #endif
     if (err != EINPROGRESS &&
         err != EWOULDBLOCK) {
       jsError("Connect failed (err %d)", err);
       closesocket(sckt);
       return -1;
     }
    }

  } else { // ------------------------------------------------- no host (=server)

    sckt = socket(AF_INET, scktType, ippProto);
    if (sckt == INVALID_SOCKET) {
      jsError("Socket creation failed");
      return 0;
    }

    if (jsvGetBoolAndUnLock(jsvObjectGetChild(options, "reuseAddr", 0))) {
      int optval = 1;
      if (setsockopt(sckt,SOL_SOCKET,SO_REUSEADDR,(const char *)&optval,sizeof(optval)) < 0)
        jsWarn("setsockopt(SO_REUSADDR) failed\n");
#ifdef SO_REUSEPORT
      if (setsockopt(sckt,SOL_SOCKET,SO_REUSEPORT,(const char *)&optval,sizeof(optval)) < 0)
        jsWarn("setsockopt(SO_REUSPORT) failed\n");
#endif
    }

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

    // multicast support
    // FIXME: perhaps extend the JsNetwork with addmembership/removemembership instead of using options
    JsVar *mgrpVar = jsvObjectGetChild(options, "multicastGroup", 0);
    if (mgrpVar) {
        char ipStr[18];

        uint32_t grpip;
        jsvGetString(mgrpVar, ipStr, sizeof(ipStr));
        jsvUnLock(mgrpVar);
        net_linux_gethostbyname(net, ipStr, &grpip);

        JsVar *ipVar = jsvObjectGetChild(options, "multicastIp", 0);
        jsvGetString(ipVar, ipStr, sizeof(ipStr));
        jsvUnLock(ipVar);
        uint32_t ip;
        net_linux_gethostbyname(net, ipStr, &ip);

        struct ip_mreq mreq;
        mreq.imr_multiaddr = *(struct in_addr *)&grpip;
        mreq.imr_interface = *(struct in_addr *)&ip;
        setsockopt (sckt, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq));
    }

    if (scktType == SOCK_STREAM) { // only for TCP
      // Make the socket listen
      nret = listen(sckt, 10); // 10 connections (but this ignored on CC30000)
      if (nret == SOCKET_ERROR) {
        jsError("Socket listen failed");
        closesocket(sckt);
        return -1;
      }
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
void net_linux_closesocket(JsNetwork *net, int sckt) {
  NOT_USED(net);
  closesocket(sckt);
}

/// If the given server socket can accept a connection, return it (or return < 0)
int net_linux_accept(JsNetwork *net, int sckt) {
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
int net_linux_recv(JsNetwork *net, SocketType socketType, int sckt, void *buf, size_t len) {
  NOT_USED(net);
  struct sockaddr_in fromAddr;
  int fromAddrLen = sizeof(fromAddr);
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
    if (socketType & ST_UDP) {
      // TODO: Use JsNetUDPPacketHeader here to tidy this up
      size_t delta =  sizeof(uint32_t) + sizeof(unsigned short) + sizeof(uint16_t);
      uint32_t *host = (uint32_t*)buf;
      unsigned short *port = (unsigned short*)&host[1];
      uint16_t *size = (unsigned short*)&port[1];
      num = (int)recvfrom(sckt,buf+delta,len-delta,0,(struct sockaddr *)&fromAddr,(socklen_t*)&fromAddrLen);
      *host = fromAddr.sin_addr.s_addr;
      *port = ntohs(fromAddr.sin_port);
      *size = (uint16_t)num;

      DBG("Recv %d %x:%d", num, *host, *port);
      if (num==0) return -1; // select says data, but recv says 0 means connection is closed
      num += (int)delta;
    } else {
      num = (int)recvfrom(sckt,buf,len,0,(struct sockaddr *)&fromAddr,(socklen_t*)&fromAddrLen);
      if (num==0) return -1; // select says data, but recv says 0 means connection is closed
    }
  }

  return num;
}

/// Send data if possible. returns nBytes on success, 0 on no data, or -1 on failure
int net_linux_send(JsNetwork *net, SocketType socketType, int sckt, const void *buf, size_t len) {
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
    if (socketType & ST_UDP) {
      // TODO: Use JsNetUDPPacketHeader here to tidy this up
      sockaddr_in       sin;
      size_t delta =  sizeof(uint32_t) + sizeof(unsigned short) + sizeof(uint16_t);
      uint32_t *host = (uint32_t*)buf;
      unsigned short *port = (unsigned short*)&host[1];
      uint16_t *size = (uint16_t*)&port[1];
      sin.sin_family = AF_INET;
      sin.sin_addr.s_addr = *(in_addr_t*)host;
      sin.sin_port = htons(*port);

      DBG("Send %d %x:%d", len - delta, *host, *port);
      n = (int)sendto(sckt, buf + delta, *size, flags, (struct sockaddr *)&sin, sizeof(sockaddr_in)) + (int)delta;
    } else {
      n = (int)send(sckt, buf, len, flags);
    }
    return n;
  } else
    return 0; // just not ready
}

void netSetCallbacks_linux(JsNetwork *net) {
  net->idle = net_linux_idle;
  net->checkError = net_linux_checkError;
  net->createsocket = net_linux_createsocket;
  net->closesocket = net_linux_closesocket;
  net->accept = net_linux_accept;
  net->gethostbyname = net_linux_gethostbyname;
  net->recv = net_linux_recv;
  net->send = net_linux_send;
  net->chunkSize = 536;
}
