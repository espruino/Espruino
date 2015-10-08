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

#ifndef USER_TELNET_H_
#define USER_TELNET_H_

/**
 * Register a telnet serever listener on port 23 to handle incoming telnet client
 * connections.  The parameter called pLineCB is a pointer to a callback function
 * that should be called when we have received a link of input.
 */
void telnet_startListening(void (*lineCB)(char *arg));

/**
 * Send the string passed as a parameter to the telnet client.
 */
void telnet_send(char *text);

#endif /* USER_TELNET_H_ */
