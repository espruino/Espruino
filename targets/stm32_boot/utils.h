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
 * Utilities
 * ----------------------------------------------------------------------------
 */

#include "jshardware.h"

int _getc();
unsigned char _getc_blocking();
void _putc(char charData);


bool isButtonPressed();
bool jumpToEspruinoBinary();
void initHardware();
