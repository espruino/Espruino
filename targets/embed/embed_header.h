/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2022 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * Espruino 'embedded' single-file JS interpreter
 * ----------------------------------------------------------------------------
 */

#ifndef ESPRUINO_EMBEDDED_H_
#define ESPRUINO_EMBEDDED_H_

#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h> // for va_args
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

void jsiConsolePrintf(const char *fmt, ...);
