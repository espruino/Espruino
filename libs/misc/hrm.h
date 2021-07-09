/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2021 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * heart rate sensor common functions
 * ----------------------------------------------------------------------------
 */

#include "jsvar.h"

typedef void(*HrmCallback)(int ppgValue);

void hrm_sensor_on(HrmCallback callback);
void hrm_sensor_off();
JsVar *hrm_sensor_getJsVar();
