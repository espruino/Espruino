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

#ifndef ESP8266_STUB_H_
#define ESP8266_STUB_H_

#include <c_types.h>
#include <ip_addr.h>
#include <espconn.h>
struct stub_ESP8266Socket {
  struct espconn *pEspconn;
  int socketId;
};



#endif /* ESP8266_STUB_H_ */
