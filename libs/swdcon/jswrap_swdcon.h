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
 * Contains JavaScript SWD console
 * ----------------------------------------------------------------------------
 */
#ifndef LIBS_SWDCON_H_
#define LIBS_SWDCON_H_

#include "jsvar.h"

void jswrap_swdcon_init(void);
void jswrap_swdcon_kill(void);
bool jswrap_swdcon_idle(void);

#endif
