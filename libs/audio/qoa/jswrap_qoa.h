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
 * JavaScript methods for working with the Quite Ok Audio Format (QOA)
 * ----------------------------------------------------------------------------
 */

#ifndef JSWRAP_QOA_H_
#define JSWRAP_QOA_H_

#include "jsvar.h"

JsVar *jswrap_qoa_frame_len();
JsVar *jswrap_qoa_init_decode(JsVar *header);
JsVar *jswrap_qoa_decode(JsVar *encoded, JsVar *decoded, JsVar *options);

#endif //JSWRAP_QOA_H_
