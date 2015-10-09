/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2015 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * This file is designed to be parsed during the build process
 *
 * Contains ESP8266 board specific functions.
 * ----------------------------------------------------------------------------
 */
#include <winsock2.h>
#include <stdio.h>
#include <assert.h>

#undef TRUE
#undef FALSE

#include "esp8266_stub_sockets.h"

static int telnetServerSocket;
static int telnetClient;

void esp8266_stub_initSockets() {
  WSADATA wsaData;
  int rc = WSAStartup(0x202, &wsaData);
  if (rc != 0) {
    printf("Error with WSAStartup(): %d\n", WSAGetLastError());
  }
}

/**
 * \brief Set the socket to be non-blocking.
 */
static void setSocketNonblocking(int s) {
  u_long iMode = 1; // Any value other than 0 means non-blocking is enabled.
  ioctlsocket(s, FIONBIO, &iMode);
}

/**
 * \brief Set the socket to be blocking.
 */
/*
static void setSocketBlocking(int s) {
  u_long iMode = 0; // Disable non-blocking (i.e. make blocking)
  ioctlsocket(s, FIONBIO, &iMode);
}
*/

/**
 * \brief Start the telnet server.
 */
void esp8266_stub_startTelnetServer() {
  telnetServerSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (telnetServerSocket < 0) {
    printf("socket: %d", WSAGetLastError());
    return;
  }
  struct sockaddr_in servAddr;
  servAddr.sin_family = AF_INET;
  servAddr.sin_addr.s_addr = INADDR_ANY;
  servAddr.sin_port = htons(23);
  int rc = bind(telnetServerSocket, (struct sockaddr *)&servAddr, sizeof(servAddr));
  if (rc == SOCKET_ERROR) {
    printf("esp8266_stub_startTelnetServer: bind: %d", WSAGetLastError());
    return;
  }
  listen(telnetServerSocket, 5);

  printf("Starting to listen for a client ...\n");
  telnetClient = accept(telnetServerSocket, NULL, NULL);
  setSocketNonblocking(telnetClient);
  printf("We got a telnet client!\n");
}

void esp8266_stub_stopTelnetServer() {

}

void esp8266_stub_sendCharacter(char c) {
  int rc = send(telnetClient, &c, 1, 0);
  if (rc == SOCKET_ERROR) {
    printf("Send error: %d\n", WSAGetLastError());
  }
}

/**
 * \brief Get a character from the telnet client.
 * \return <0 An error occurred.  0 no character.
 */
int esp8266_stub_getCharacter() {
  char c;
  int rc = recv(telnetClient, &c, 1, 0);
  if (rc == SOCKET_ERROR) {
    if (WSAGetLastError() == WSAEWOULDBLOCK) {
      return 0;
    }
    printf("Recv error: %d\n", WSAGetLastError());
    return -1;
  }
  return c;
}


/**
 * \brief Retrieve the local IP address.
 * \return The local IP address.
 */
uint32_t esp8266_stub_getLocalIP() {
  char ac[80];
  if (gethostname(ac, sizeof(ac)) == SOCKET_ERROR) {
    return 0;
  }
  struct hostent *phe = gethostbyname(ac);
  if (phe == 0) {
    return 0;
  }
  return *(uint32_t *)(phe->h_addr_list[0]);
}

/**
 * \brief Connect to a partner.
 * We form a connection to the partner specified in the socket structure.
 * Specifically, the parameters remoteAddress and remotePort are used to define
 * the target of the connection.
 * \return 0 on error and 1 on success.
 */
int esp8266_stub_connect(struct stub_ESP8266Socket *pSocket) {
  assert(pSocket != NULL);

  // Perform a TCP/IP connect request.

  // Create a new socket
  pSocket->socketId = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (pSocket->socketId == INVALID_SOCKET) {
    printf("esp8266_stub_connect: socket: %d\n", WSAGetLastError());
    return 0;
  }
  printf("esp8266_stub_connect: Created a new socket with id: %d\n", pSocket->socketId);

  // Connect to the partner over the new socket.

  // Prep the address structure.
  struct sockaddr_in clientService;
  clientService.sin_family = AF_INET;
  clientService.sin_port = htons(pSocket->pEspconn->proto.tcp->remote_port);
  clientService.sin_addr.s_addr = *(uint32_t *)pSocket->pEspconn->proto.tcp->remote_ip;

  // Debug
  printf("esp8266_stub_connect .. connecting to %s[%d]\n",
      inet_ntoa((struct in_addr)clientService.sin_addr),
      pSocket->pEspconn->proto.tcp->remote_port);

  // Perform the actual network level connection.  We need to make sure that the socket
  // is flagged as blocking otherwise we will be a WSAEWOULDBLOCK error indication.
  int rc = connect(pSocket->socketId, (struct sockaddr *)&clientService, sizeof(clientService));
  if (rc == SOCKET_ERROR) {
    printf("esp8266_stub_connect: Error: %d\n", WSAGetLastError());
    return 0;
  }

  setSocketNonblocking(pSocket->socketId);

  // Retrieve the local port number and local address.
  struct sockaddr_in localName;
  int size = sizeof(localName);
  getsockname(pSocket->socketId, (struct sockaddr *)&localName, &size );
  pSocket->pEspconn->proto.tcp->local_port = ntohs(localName.sin_port);
  *(uint32_t *)pSocket->pEspconn->proto.tcp->local_ip = localName.sin_addr.s_addr;
  return 1;
}

/**
 * Return 0 on success.
 */
int esp8266_stub_listen(struct stub_ESP8266Socket *pSocket) {
  pSocket->socketId = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  struct sockaddr_in servAddr;
  servAddr.sin_family = AF_INET;
  servAddr.sin_addr.s_addr = INADDR_ANY;
  servAddr.sin_port = htons(pSocket->pEspconn->proto.tcp->local_port);
  int rc = bind(telnetServerSocket, (struct sockaddr *)&servAddr, sizeof(servAddr));
  if (rc<0) {
    printf("esp8266_stub_listen: bind: %d", WSAGetLastError());
    return 0;
  }

  setSocketNonblocking(pSocket->socketId);

  listen(pSocket->socketId, 5);
  return 1;
}


/**
 * \brief Disconnect (close) the socket represented by the pSocket.
 * \return Returns 1 on success.
 */
int esp8266_stub_disconnect(struct stub_ESP8266Socket *pSocket) {
  closesocket(pSocket->socketId);
  return 1;
}


/**
 * \brief Send data to a TCP partner.
 * Send data to a TCP partner where the partner is identified by the socket contained in
 * the pSocket structure.
 * \return If <0 then an error has occurred otherwise we return how much data was actually sent.
 */
int eps8266_stub_send(
    struct stub_ESP8266Socket *pSocket,
    uint8_t *pBuf,
    uint16_t size
  ) {
  assert(pSocket != NULL);
  assert(pSocket->socketId != -1);
  int rc = send(pSocket->socketId, (char *)pBuf, size, 0);
  if (rc == SOCKET_ERROR) {
    printf("eps8266_stub_send: Error: %d on socket: %d\n", WSAGetLastError(), pSocket->socketId);
    return -1;
  }
  return rc;
}


/**
 * \brief Receive data from the partner.
 * Receive data in a non-blocking fashion from the partner.
 * \return <0 then an error has occurred otherwise how much data actually read.
 */
int esp8266_stub_recv(
    struct stub_ESP8266Socket *pSocket, //!< The identity of the socket to receive from.
    uint8_t *pBuf,                      //!< A buffer into which to read received data.
    uint16_t size                       //!< The size of the data buffer into which we can receive.
  ) {
  int rc = recv(pSocket->socketId, (char *)pBuf, size, 0);
  if (rc == SOCKET_ERROR) {
    if (WSAGetLastError() == WSAEWOULDBLOCK) {
      return 0;
    }
    printf("esp8266_stub_recv: recv: %d\n", WSAGetLastError());
    return -1;
  }
  return rc;
}

/**
 * \brief Check to see if the given socket has a new client connection.
 * \return -1 if there is no new client.
 */
int esp8266_stub_checkAccept(struct stub_ESP8266Socket *pSocket) {
  int newClientSocket = -1;
  return newClientSocket;
}
