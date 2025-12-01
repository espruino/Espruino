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
 * This file is designed to be parsed during the build process
 *
 * JavaScript methods for accessing the built-in timer
 * ----------------------------------------------------------------------------
 */
#include "jsvar.h"


JsVar *jswrap_timer_list();
JsVar *jswrap_timer_get(int id);
int jswrap_timer_add(JsVar *timer);
void jswrap_timer_remove(int id);
