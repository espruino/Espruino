/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2013 Thorsten von Eicken
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * Contains JavaScript HTTP Functions
 * ----------------------------------------------------------------------------
 */
#ifndef LIBS_NETWORK_TELNET_H_
#define LIBS_NETWORK_TELNET_H_

#include "jsvar.h"

void jswrap_telnet_setOptions(JsVar *options);

void jswrap_telnet_init(void);
void jswrap_telnet_kill(void);

// Listen, accept, send, and recv on telnet console connections. Returns true if something
// was done.
bool jswrap_telnet_idle(void);

#endif
