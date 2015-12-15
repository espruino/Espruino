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
 * Definitions of errors with sockets
 * ----------------------------------------------------------------------------
 */

#ifndef _SOCKERR_H
#define _SOCKERR_H

/** Socket errors */
typedef enum {
  SOCKET_ERR_CLOSED       = -1,
  SOCKET_ERR_MEM          = -2,
  SOCKET_ERR_TIMEOUT      = -3,
  SOCKET_ERR_NO_ROUTE     = -4,
  SOCKET_ERR_BUSY         = -5,
  SOCKET_ERR_NOT_FOUND    = -6,
  SOCKET_ERR_MAX_SOCK     = -7,
  SOCKET_ERR_UNSENT_DATA  = -8,
  SOCKET_ERR_RESET        = -9,
  SOCKET_ERR_UNKNOWN      = -10,
  SOCKET_ERR_NO_CONN      = -11,
  SOCKET_ERR_BAD_ARG      = -12,
  SOCKET_ERR_SSL_HAND     = -13,
  SOCKET_ERR_SSL_INVALID  = -14,
  SOCKET_ERR_NO_RESP      = -15,
  SOCKET_ERR_LAST         = -15, // not an error, just value of last error
} SocketError;

/// Return a pointer to an error string given the (negative) error code
// The string is statically allocated and must not be modified
char *socketErrorString(int error);

#endif
