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
 * Touchscreen gesture detection
 * ----------------------------------------------------------------------------
 */

/// initialise stroke detection
void unistroke_init();

/// Called when a touch event occurs, returns 'true' if an event should be created (by calling unistroke_getEventVar)
bool unistroke_touch(int x, int y, int dx, int dy, int pts);

/// Called when a touch event occurs, and returns the data that should be passed in the event
JsVar *unistroke_getEventVar();

/// Convert an array containing XY values to a unistroke var
JsVar *unistroke_convert(JsVar *xy);

/// Given an object containing values created with unistroke_convert, compare against an array containing XY values
JsVar *unistroke_recognise_xy(JsVar *strokes, JsVar *xy);
