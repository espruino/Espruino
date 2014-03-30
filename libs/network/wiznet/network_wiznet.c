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
 * Implementation of JsNetwork for WIZnet devices
 * ----------------------------------------------------------------------------
 */
#include "network.h"
#include "network_wiznet.h"



#define INVALID_SOCKET ((SOCKET)(-1))
#define SOCKET_ERROR (-1)

#include "wiznet/Ethernet/socket.h"

typedef struct sockaddr_in sockaddr_in;
#define closesocket(SOCK) close(SOCK)
#define MSG_NOSIGNAL 0x4000 /* don't raise SIGPIPE */ // IGNORED ANYWAY!
#define send(sock,ptr,len,flags) send(sock,(uint8_t*)(ptr),len) // throw away last arg of send
#define recv(sock,ptr,len,flags) recv(sock,(uint8_t*)(ptr),len) // throw away last arg of send

// name resolution
#include "wiznet/DNS/dns.h"
extern uint8_t Server_IP_Addr[4];


#define WIZNET_SERVER_CLIENT 256 // sockets are only 0-255 so this is masked out

uint8_t net_wiznet_getFreeSocket() {
  uint8_t i;
  for (i=0;i<8;i++)
    if (getSn_SR(i) == SOCK_CLOSED) // it's free!
      return i;

  jsError("No free sockets found\n");
  // out of range will probably just make it error out
  return 8;
}




/// Get an IP address from a name. Sets out_ip_addr to 0 on failure
void net_wiznet_gethostbyname(JsNetwork *net, char * hostName, unsigned long* out_ip_addr) {
  NOT_USED(net);
  if (dns_query(0, net_wiznet_getFreeSocket(), (uint8_t*)hostName) == 1) {
    *out_ip_addr = *(unsigned long*)&Server_IP_Addr[0];
  }
}

/// Called on idle. Do any checks required for this device
void net_wiznet_idle(JsNetwork *net) {
  NOT_USED(net);
}

/// Call just before returning to idle loop. This checks for errors and tries to recover. Returns true if no errors.
bool net_wiznet_checkError(JsNetwork *net) {
  NOT_USED(net);
  bool hadErrors = false;
  return hadErrors;
}

/// if host=0, creates a server otherwise creates a client (and automatically connects). Returns >=0 on success
int net_wiznet_createsocket(JsNetwork *net, unsigned long host, unsigned short port) {
  NOT_USED(net);
  int sckt = -1;
  if (host!=0) { // ------------------------------------------------- host (=client)
    sckt = socket(net_wiznet_getFreeSocket(), Sn_MR_TCP, port, 0); // we set nonblocking later
    if (sckt<0) return sckt; // error

    int res = connect((uint8_t)sckt,(uint8_t*)&host, port);
    // now we set nonblocking - so that connect waited for the connection
    uint8_t ctl = SOCK_IO_NONBLOCK;
    ctlsocket((uint8_t)sckt, CS_SET_IOMODE, &ctl);

    if (res == SOCKET_ERROR) {
     jsError("Connect failed (err %d)\n", res );
    }
 } else { // ------------------------------------------------- no host (=server)
    sckt = socket(net_wiznet_getFreeSocket(), Sn_MR_TCP, port, SF_IO_NONBLOCK);
 }
  return sckt;
}

/// destroys the given socket
void net_wiznet_closesocket(JsNetwork *net, int sckt) {
  NOT_USED(net);
  // close gracefully
  disconnect((uint8_t)sckt);
  JsSysTime timeout = jshGetSystemTime()+jshGetTimeFromMilliseconds(1000);
  uint8_t status;
  while ((status=getSn_SR((uint8_t)sckt)) != SOCK_CLOSED &&
         jshGetSystemTime()<timeout) ;
  // if that didn't work, force it
  if (status != SOCK_CLOSED)
    closesocket((uint8_t)sckt);
  // Wiznet is a bit strange in that it uses the same socket for server and client
  if (sckt & WIZNET_SERVER_CLIENT) {
    // so it's just closed, but put it into 'listen' mode again
    int port = 80; // FIXME
    sckt = socket((uint8_t)sckt, Sn_MR_TCP, port, SF_IO_NONBLOCK);
  }
}

/// If the given server socket can accept a connection, return it (or return < 0)
int net_wiznet_accept(JsNetwork *net, int sckt) {
  NOT_USED(net);
  /* CC3000/WIZnet works a different way - we set accept as nonblocking,
   * and then we just call it and see if it works or not...
   */
  // we have a client waiting to connect... try to connect and see what happens
  // WIZnet's implementation doesn't use accept, it uses listen
  int theClient = listen((uint8_t)sckt);
  if (theClient==SOCK_OK)
    theClient = sckt | WIZNET_SERVER_CLIENT; // we deal with the client on the same socket (we use the flag so we know that it really is different!)
  return theClient;
}

/// Receive data if possible. returns nBytes on success, 0 on no data, or -1 on failure
int net_wiznet_recv(JsNetwork *net, int sckt, void *buf, size_t len) {
  NOT_USED(net);
  int num = 0;
  if (getSn_SR(sckt)!=SOCK_LISTEN) {
    // receive data - if none available it'll just return SOCK_BUSY
    num = (int)recv(sckt,buf,len,0);
    if (num==SOCK_BUSY) num=0;
  }
  return num;
}

/// Send data if possible. returns nBytes on success, 0 on no data, or -1 on failure
int net_wiznet_send(JsNetwork *net, int sckt, const void *buf, size_t len) {
  NOT_USED(net);
  return (int)send(sckt, buf, len, MSG_NOSIGNAL);
}

void netSetCallbacks_wiznet(JsNetwork *net) {
  net->idle = net_wiznet_idle;
  net->checkError = net_wiznet_checkError;
  net->createsocket = net_wiznet_createsocket;
  net->closesocket = net_wiznet_closesocket;
  net->accept = net_wiznet_accept;
  net->gethostbyname = net_wiznet_gethostbyname;
  net->recv = net_wiznet_recv;
  net->send = net_wiznet_send;
}

