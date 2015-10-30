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

#ifndef ESP8266_STUB_SOCKETS_WINDOWS_H_
#define ESP8266_STUB_SOCKETS_WINDOWS_H_

#include "esp8266_stub.h"

void esp8266_stub_initSockets();
void esp8266_stub_startTelnetServer();
void esp8266_stub_sendCharacter(char c);
uint32_t esp8266_stub_getLocalIP();
int esp8266_stub_connect(struct stub_ESP8266Socket *pSocket);
int esp8266_stub_disconnect(struct stub_ESP8266Socket *pSocket);
int eps8266_stub_send(struct stub_ESP8266Socket *pSocket, uint8_t *pBuf, uint16_t size);
int esp8266_stub_recv(struct stub_ESP8266Socket *pSocket, uint8_t *pBuf, uint16_t size);
int esp8266_stub_listen(struct stub_ESP8266Socket *pSocket);
int esp8266_stub_checkAccept(struct stub_ESP8266Socket *pSocket);

#endif /* ESP8266_STUB_SOCKETS_WINDOWS_H_ */
