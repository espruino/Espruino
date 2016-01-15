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
#include "jsinteractive.h"
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
unsigned short wiznetSocketPorts[8];
unsigned char wiznetSocketAsServerClient = 0;


#define WIZNET_SERVER_CLIENT 256 // sockets are only 0-255 so this is masked out

uint8_t net_wiznet_getFreeSocket() {
  unsigned int i;
  for (i=0;i<8;i++)
    if (getSn_SR(i) == SOCK_CLOSED) // it's free!
      return (uint8_t)i;

  jsError("No free sockets found\n");
  // out of range will probably just make it error out
  return 8;
}




/// Get an IP address from a name. Sets out_ip_addr to 0 on failure
void net_wiznet_gethostbyname(JsNetwork *net, char * hostName, uint32_t* out_ip_addr) {
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
int net_wiznet_createsocket(JsNetwork *net, uint32_t host, unsigned short port) {
  int sckt = -1;
  if (host!=0) { // ------------------------------------------------- host (=client)

    //mgg1010 - added random source port - seems to solve problem of repeated GET failing
    
    sckt = socket(net_wiznet_getFreeSocket(), Sn_MR_TCP, (uint16_t)((rand() & 32767) + 2000), 0); // we set nonblocking later
     
    if (sckt<0) {
      return sckt; // error
    }

    int res = connect((uint8_t)sckt,(uint8_t*)&host, port);
    // now we set nonblocking - so that connect waited for the connection
    uint8_t ctl = SOCK_IO_NONBLOCK;
    ctlsocket((uint8_t)sckt, CS_SET_IOMODE, &ctl);

    if (res == SOCKET_ERROR) {
     jsError("Connect failed (err %d)\n", res );
    }
  } else { // ------------------------------------------------- no host (=server)
    sckt = socket(net_wiznet_getFreeSocket(), Sn_MR_TCP, port, SF_IO_NONBLOCK);
    listen((uint8_t)sckt);
  }
  wiznetSocketPorts[sckt&7] = port;
  //jsiConsolePrintf("Created socket %d\n", sckt);
  return sckt;
}

/// destroys the given socket
void net_wiznet_closesocket(JsNetwork *net, int sckt) {
  // try and close gracefully
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
    sckt = socket((uint8_t)sckt, Sn_MR_TCP, wiznetSocketPorts[sckt&7], SF_IO_NONBLOCK);
    listen((uint8_t)sckt);
    // Be sure to mark it as not a client socket any more
    wiznetSocketAsServerClient = wiznetSocketAsServerClient & (unsigned char)~(1<<(sckt&7));
  }
}

/// If the given server socket can accept a connection, return it (or return < 0)
int net_wiznet_accept(JsNetwork *net, int sckt) {

  // On WIZnet the same server socket is reused for clients - keep track so we don't get confused
  // and try and allocate a new HTTP Server Client
  if (wiznetSocketAsServerClient & (1<<sckt)) {
    return -1;
  }

  /* CC3000/WIZnet works a different way - we set accept as nonblocking,
   * and then we just call it and see if it works or not...
   */

  // we have a client waiting to connect... try to connect and see what happens
  // WIZnet's implementation doesn't use accept, it uses listen
  int status = getSn_SR((uint8_t)sckt);
  if (status == SOCK_ESTABLISHED) {
    wiznetSocketAsServerClient = wiznetSocketAsServerClient | (unsigned char)(1<<sckt); // mark that it's now being used as a client socket
    return ((int)sckt) | WIZNET_SERVER_CLIENT; // we deal with the client on the same socket (we use the flag so we know that it really is different!)
  }

  // WIZnet can get confused (somehow!) when handling repeated requests from the HTTP server
  if (status == SOCK_CLOSED || status == SOCK_CLOSE_WAIT) {
    // make sure we force-close again and re-init as a listener
    net_wiznet_closesocket(net, (uint16_t)(sckt | WIZNET_SERVER_CLIENT));
  }
  return -1;
}

/// Receive data if possible. returns nBytes on success, 0 on no data, or -1 on failure
int net_wiznet_recv(JsNetwork *net, int sckt, void *buf, size_t len) {
  int num = 0;
  if (getSn_SR((uint8_t)sckt) == SOCK_LISTEN) {
    // socket is operating as a TCP server - something has gone wrong.
    // just return -1 to close this connection immediately
    return -1;
  } else {
    // receive data - if none available it'll just return SOCK_BUSY
    num = (int)recv((uint8_t)sckt,buf,(uint16_t)len,0);
    if (num==SOCK_BUSY) num=0;
  }
  if (jspIsInterrupted()) return -1;
  return num;
}

/// Send data if possible. returns nBytes on success, 0 on no data, or -1 on failure
int net_wiznet_send(JsNetwork *net, int sckt, const void *buf, size_t len) {
  int r = (int)send((uint8_t)sckt, buf, (uint16_t)len, MSG_NOSIGNAL);
  if (jspIsInterrupted()) return -1;
  return r;
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
  net->chunkSize = 536;
}

