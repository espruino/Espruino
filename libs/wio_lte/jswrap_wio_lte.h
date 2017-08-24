/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2016 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * Contains JavaScript interface for the Seeed WIO LTE board
 * ----------------------------------------------------------------------------
 */
#include "jspin.h"

void jswrap_wio_lte_led(int r, int g, int b);
void jswrap_wio_lte_setGrovePower(bool pwr);
void jswrap_wio_lte_setLEDPower(bool pwr);
void jswrap_wio_lte_init();
