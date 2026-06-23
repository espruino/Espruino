/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2025 Simon Sievert <simon.sievert@mailbox.org>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * JavaScript I2S Functions
 * ----------------------------------------------------------------------------
 */

#ifndef JSWRAP_I2S_H
#define JSWRAP_I2S_H

#include "jsvar.h"
#include "jspin.h"

bool jswrap_i2s_init(Pin bclk, Pin lrck, Pin dout, int sample_width_bytes);

void jswrap_i2s_uninit();

bool jswrap_i2s_start(JsVar *buf, JsVar *data_callback, JsVar *buffer_released_callback);

void jswrap_i2s_stop();

bool jswrap_i2s_set_next_buffer(JsVar *buf);

bool jswrap_i2s_idle();

void jswrap_i2s_kill();

#endif //JSWRAP_I2S_H
