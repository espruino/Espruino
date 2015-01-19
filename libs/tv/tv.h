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
 * TV output capability on STM32 devices
 * ----------------------------------------------------------------------------
 */

JsVar *tv_setup_pal(Pin pinVideo, Pin pinSync, int width, int height);
JsVar *tv_setup_vga(Pin pinVideo, Pin pinSync, Pin pinSyncV, int width, int height);
