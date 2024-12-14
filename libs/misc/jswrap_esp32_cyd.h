/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2024 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 *  ESP32 CYD (cheap yellow display)
 * ----------------------------------------------------------------------------
 */

#include "jsvar.h"

JsVar *jswrap_cyd_cn1();
JsVar *jswrap_cyd_p1();
JsVar *jswrap_cyd_p3();
void jswrap_cyd_init();
bool jswrap_cyd_idle();