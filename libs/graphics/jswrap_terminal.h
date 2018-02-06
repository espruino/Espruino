/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2017 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * Contains VT100 terminal emulator
 * ----------------------------------------------------------------------------
 */

#include "jsutils.h"
#include "jsvar.h"

/// Handle data sent to the VT100 terminal
void terminalSendChar(char c);
/// Initialise the terminal
void jswrap_terminal_init();

