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

/// Turn heart rate sensor on - callback called on each data point
void hrm_sensor_on(HrmCallback callback);
/// Turn heart rate sensor off
void hrm_sensor_off();
/// Get extra HRM data as a JS Object
JsVar *hrm_sensor_getJsVar();


/// Called when JS engine torn down (disable timer/watch/etc)
void hrm_sensor_kill();
/// Called when JS engine initialised
void hrm_sensor_init();
