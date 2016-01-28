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
 * Utility functions to handle socket error messages
 * ----------------------------------------------------------------------------
 */
#include "socketerrors.h"

static char *socketErrors[] = {
  "connection closed",   // not really an error
  "out of memory",
  "timeout",
  "no route",
  "busy",
  "not found",
  "max sockets",
  "unsent data",
  "connection reset",
  "unknown error",
  "no connection",
  "bad argument",
  "SSL handshake failed",
  "invalid SSL data",
  "no response",
};

char *socketErrorString(int error) {
  if (error >= 0) return 0;
  if (error < SOCKET_ERR_LAST) error = SOCKET_ERR_UNKNOWN;
  return socketErrors[-1-error]; // map -1=>0, -2=>1, -3=>2, ...
}
