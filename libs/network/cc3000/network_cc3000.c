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
 * Implementation of JsNetwork for CC3000
 * ----------------------------------------------------------------------------
 */
#include "network.h"
#include "network_cc3000.h"
#include "jsparse.h"
#include "jsinteractive.h"
#include "socketserver.h"

#include <string.h> // for memset

#define INVALID_SOCKET ((SOCKET)(-1))
#define SOCKET_ERROR (-1)
typedef int SOCKET;

 #include "cc3000/spi.h"
 #include "cc3000/socket.h"
 #include "cc3000/board_spi.h"
 #include "cc3000/cc3000_common.h"
 #include "cc3000/jswrap_cc3000.h"

 #define MSG_NOSIGNAL 0x4000 /* don't raise SIGPIPE */ // IGNORED ANYWAY!

/// Get an IP address from a name. Sets out_ip_addr to 0 on failure
void net_cc3000_gethostbyname(JsNetwork *net, char * hostName, uint32_t* out_ip_addr) {
  gethostbyname(hostName, strlen(hostName), out_ip_addr);
  *out_ip_addr = networkFlipIPAddress(*out_ip_addr);
}

/// Called on idle. Do any checks required for this device
void net_cc3000_idle(JsNetwork *net) {
  cc3000_spi_check();

  if (networkState == NETWORKSTATE_INVOLUNTARY_DISCONNECT) {
    JsVar *wlanObj = jsvObjectGetChild(execInfo.hiddenRoot, CC3000_OBJ_NAME, 0);
    if (wlanObj) {
      jswrap_wlan_reconnect(wlanObj);
      jsvUnLock(wlanObj);
      cc3000_spi_check();
    }
  }
}

/// Call just before returning to idle loop. This checks for errors and tries to recover. Returns true if no errors.
bool net_cc3000_checkError(JsNetwork *net) {
  bool hadErrors = false;
  while (jspIsInterrupted()) {
    hadErrors = true;
    jsiConsolePrint("CC3000 WiFi is not responding. Power cycling...\n");
    jspSetInterrupted(false);
    // remove all existing connections
    networkState = NETWORKSTATE_OFFLINE; // ensure we don't try and send the CC3k anything
    socketKill(net);
    socketInit();
    // power cycle
    JsVar *wlan = jsvObjectGetChild(execInfo.hiddenRoot, CC3000_OBJ_NAME, 0);
    if (wlan) {
      jswrap_wlan_reconnect(wlan);
      jsvUnLock(wlan);
    } else jsExceptionHere(JSET_INTERNALERROR, "No CC3000 object!\n");
    // jswrap_wlan_reconnect could fail, which would mean we have to do this all over again
  }
  return hadErrors;
}

/// if host=0, creates a server otherwise creates a client (and automatically connects). Returns >=0 on success
int net_cc3000_createsocket(JsNetwork *net, uint32_t host, unsigned short port) {
  int sckt = -1;
  if (host!=0) { // ------------------------------------------------- host (=client)

    host = networkFlipIPAddress(host);

    sockaddr       sin;
    sin.sa_family = AF_INET;
    sin.sa_data[0] = (unsigned char)((port & 0xFF00) >> 8);
    sin.sa_data[1] = (unsigned char)(port & 0x00FF);

    sckt = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sckt<0) return sckt; // error

    int param;
    param = SOCK_ON;
    setsockopt(sckt, SOL_SOCKET, SOCKOPT_RECV_NONBLOCK, &param, sizeof(param)); // enable nonblock
    param = 5; // ms
    setsockopt(sckt, SOL_SOCKET, SOCKOPT_RECV_TIMEOUT, &param, sizeof(param)); // set a timeout

    sin.sa_data[5] = (unsigned char)((host) & 0xFF);  // First octet of destination IP
    sin.sa_data[4] = (unsigned char)((host>>8) & 0xFF);   // Second Octet of destination IP
    sin.sa_data[3] = (unsigned char)((host>>16) & 0xFF);  // Third Octet of destination IP
    sin.sa_data[2] = (unsigned char)((host>>24) & 0xFF);  // Fourth Octet of destination IP

    int res = connect(sckt,(struct sockaddr *)&sin, sizeof(sockaddr_in) );

    if (res == SOCKET_ERROR) {
     int err = errno;
     if (err != EINPROGRESS &&
         err != EWOULDBLOCK) {
       jsError("Connect failed (err %d)\n", err );
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
    int optval = SOCK_ON;
    if (setsockopt(sckt,SOL_SOCKET,SOCKOPT_ACCEPT_NONBLOCK,(const char *)&optval,sizeof(optval)) < 0)
      jsWarn("setsockopt failed\n");

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
      jsError("Socket listen failed");
      closesocket(sckt);
      return -1;
    }
  }
  return sckt;
}

/// destroys the given socket
void net_cc3000_closesocket(JsNetwork *net, int sckt) {
  closesocket(sckt);
}

/// If the given server socket can accept a connection, return it (or return < 0)
int net_cc3000_accept(JsNetwork *net, int sckt) {
  /* CC3000/WIZnet works a different way - we set accept as nonblocking,
   * and then we just call it and see if it works or not...
   */
  // CC3000's implementation doesn't accept NULL like everyone else's :(
  sockaddr addr;
  socklen_t addrlen = sizeof(addr);
  return accept(sckt,&addr,&addrlen);
}

/// Receive data if possible. returns nBytes on success, 0 on no data, or -1 on failure
int net_cc3000_recv(JsNetwork *net, int sckt, void *buf, size_t len) {
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

  if ((num==0 && cc3000_socket_has_closed(sckt)) ||
      num>len) // Yes, this really does happen
    return -1;

  return num;
}

/// Send data if possible. returns nBytes on success, 0 on no data, or -1 on failure
int net_cc3000_send(JsNetwork *net, int sckt, const void *buf, size_t len) {
  if (cc3000_socket_has_closed(sckt))
    return -1;

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
    n = send(sckt, buf, len, MSG_NOSIGNAL);
    if (n>len) return -1;
    return n;
  } else
    return 0; // just not ready
}

void netSetCallbacks_cc3000(JsNetwork *net) {
  net->idle = net_cc3000_idle;
  net->checkError = net_cc3000_checkError;
  net->createsocket = net_cc3000_createsocket;
  net->closesocket = net_cc3000_closesocket;
  net->accept = net_cc3000_accept;
  net->gethostbyname = net_cc3000_gethostbyname;
  net->recv = net_cc3000_recv;
  net->send = net_cc3000_send;
  /* we're limited by CC3k buffer sizes - see CC3000_RX_BUFFER_SIZE/CC3000_TX_BUFFER_SIZE
   * We could however allocate RAM on the stack (since we now don't use IRQs)
   * and could then alloc more, increasing this. */
  net->chunkSize = 64;
}
