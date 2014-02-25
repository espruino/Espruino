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
#include "jshardware.h"


JsVar *jswrap_waveform_constructor(Pin pin, int samples);
void jswrap_waveform_startOutput(JsVar *waveform, JsVarFloat freq, bool repeat);
bool jswrap_waveform_idle();
