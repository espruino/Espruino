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
 * Recursive descent JIT
 * ----------------------------------------------------------------------------
 */
#ifndef JSJITC_H_
#define JSJITC_H_

#include "jsjit.h"

void jsjcLiteral32(int reg, uint32_t data);
void jsjcLiteral64(int reg, uint64_t data);
void jsjcCall(void *c);

#endif /* JSJIT_H_ */
