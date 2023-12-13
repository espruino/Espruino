/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2023 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * Contains JavaScript interface for Jolt.js Qwiic object
 * ----------------------------------------------------------------------------
 */


JsVar *jswrap_jqwiic_i2c(JsVar *parent);
JsVar *jswrap_jqwiic_setPower(JsVar *parent, bool isOn);

