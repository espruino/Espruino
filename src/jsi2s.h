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
 * I2S support
 * ----------------------------------------------------------------------------
 */

#ifndef JSI2S_H
#define JSI2S_H

#include "jspin.h"

typedef void (*jsi2s_data_cb_t)(void);

typedef void (*jsi2s_buffer_release_cb_t)(uint32_t *buf_ptr);

bool jsi2s_init(Pin bclk, Pin lrck, Pin dout, int sample_width_bytes);

void jsi2s_uninit();

bool jsi2s_start(uint32_t *buf_ptr, uint32_t buffer_size_bytes, jsi2s_data_cb_t data_callback,
                 jsi2s_buffer_release_cb_t buffer_released_callback);

void jsi2s_stop();

bool jsi2s_set_next_buffer(uint32_t *buf_ptr, uint32_t buffer_size_bytes);

bool jsi2s_idle();

#endif // JSI2S_H
