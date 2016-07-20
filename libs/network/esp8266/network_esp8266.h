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
 * Contains ESP8266 board network specific function definitions.
 * ----------------------------------------------------------------------------
 */

#ifndef LIBS_NETWORK_ESP8266_NETWORK_ESP8266_H_
#define LIBS_NETWORK_ESP8266_NETWORK_ESP8266_H_

#include "network.h"

/**
 * The maximum number of concurrently open sockets we support.
 * We should probably pair this with the ESP8266 concept of the maximum number of sockets
 * that an ESP8266 instance can also support.
 */
#define MAX_SOCKETS (10)

void netInit_esp8266_board();
void netSetCallbacks_esp8266_board(JsNetwork *net);
void esp8266_dumpSocket(int socketId);
void esp8266_dumpAllSocketData();
int  net_ESP8266_BOARD_accept(JsNetwork *net, int serverSckt);
int  net_ESP8266_BOARD_recv(JsNetwork *net, int sckt, void *buf, size_t len);
int  net_ESP8266_BOARD_send(JsNetwork *net, int sckt, const void *buf, size_t len);
void net_ESP8266_BOARD_idle(JsNetwork *net);
bool net_ESP8266_BOARD_checkError(JsNetwork *net);
int  net_ESP8266_BOARD_createSocket(JsNetwork *net, uint32_t ipAddress, unsigned short port);
void net_ESP8266_BOARD_closeSocket(JsNetwork *net, int sckt);
void net_ESP8266_BOARD_gethostbyname(JsNetwork *net, char *hostName, uint32_t *outIp);
#endif /* LIBS_NETWORK_ESP8266_NETWORK_ESP8266_H_ */
