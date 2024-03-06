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
 * JavaScript methods for Stepper Motor control using builtin timer
 * ----------------------------------------------------------------------------
 */
#ifndef JSWRAP_STEPPER_H_
#define JSWRAP_STEPPER_H_

#include "jshardware.h"

bool jswrap_stepper_idle();
void jswrap_stepper_kill();
JsVar *jswrap_stepper_constructor(JsVar *options);
JsVar *jswrap_stepper_moveTo(JsVar *stepper, int position, JsVar *options);
void jswrap_stepper_stop(JsVar *stepper, JsVar *options);
int jswrap_stepper_getPosition(JsVar *stepper);

#endif // JSWRAP_WAVEFORM_H_
