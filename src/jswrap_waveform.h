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
 * JavaScript methods for Waveforms (eg. Audio)
 * ----------------------------------------------------------------------------
 */
#ifndef JSWRAP_WAVEFORM_H_
#define JSWRAP_WAVEFORM_H_

#include "jshardware.h"

void jswrap_waveform_kill();
void jswrap_waveform_eventHandler(IOEventFlags eventFlags, uint8_t *data, int length);
JsVar *jswrap_waveform_constructor(JsVar *samples, JsVar *options);
void jswrap_waveform_startOutput(JsVar *waveform, Pin pin, JsVarFloat freq, JsVar *options);
void jswrap_waveform_startInput(JsVar *waveform, Pin pin, JsVarFloat freq, JsVar *options);
void jswrap_waveform_stop(JsVar *waveform);

#endif // JSWRAP_WAVEFORM_H_
