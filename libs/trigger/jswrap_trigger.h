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
 * Contains JavaScript interface for trigger wheel functionality
 * ----------------------------------------------------------------------------
 */

JsVarFloat jswrap_trig_getPosAtTime(JsVarFloat time);
void jswrap_trig_setup(Pin pin, JsVar *options);
void jswrap_trig_setTrigger(JsVarInt num, JsVarFloat position, JsVar *pins, JsVarFloat pulseLength);
void jswrap_trig_killTrigger(JsVarInt num);
JsVar *jswrap_trig_getTrigger(JsVarInt num);
JsVarFloat jswrap_trig_getRPM();
JsVarInt jswrap_trig_getErrors();
JsVar* jswrap_trig_getErrorArray();
