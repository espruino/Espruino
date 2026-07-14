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

#include "jsi2s.h"

#include <jsinteractive.h>
#include <jspininfo.h>
#include <nrf_drv_i2s.h>
#include <nrfx_i2s.h>

bool jsi2s_initialized = false;
bool jsi2s_active = false;
uint32_t jsi2s_buffer_size_bytes = 0;
jsi2s_data_cb_t jsi2s_data_callback = NULL;
jsi2s_buffer_release_cb_t jsi2s_buffer_released_callback = NULL;

void jsi2s_on_buffer_released(JsVar *buf) {
  if (jsi2s_buffer_released_callback != NULL) {
    jsi2s_buffer_released_callback(buf);
  }
}

void jsi2s_set_callbacks(jsi2s_data_cb_t data_callback,
                         jsi2s_buffer_release_cb_t buffer_released_callback) {
  jsi2s_data_callback = data_callback;
  jsi2s_buffer_released_callback = buffer_released_callback;
}

void jsi2s_unset_callbacks() {
  jsi2s_data_callback = NULL;
  jsi2s_buffer_released_callback = NULL;
}

static void jsi2s_data_handler(nrfx_i2s_buffers_t const *p_released,
                               uint32_t status) {
  // ToDo: handle buffer underrun (buffers requested, but weren't set quickly enough)
  if (p_released != NULL) {
    if (p_released->p_tx_buffer != NULL) {
      if (jsi2s_buffer_released_callback != NULL) {
        jsi2s_buffer_released_callback(p_released->p_tx_buffer);
      }
    }
  }
  if (status & NRFX_I2S_STATUS_NEXT_BUFFERS_NEEDED) {
    if (jsi2s_data_callback != NULL) {
      jsi2s_data_callback();
    }
  }
}

bool jsi2s_init(Pin bclk, Pin lrck, Pin dout, int sample_width_bytes) {
  jsi2s_uninit();

  uint32_t bclkPin = (uint32_t) pinInfo[bclk].pin;
  uint32_t lrckPin = (uint32_t) pinInfo[lrck].pin;
  uint32_t doutPin = (uint32_t) pinInfo[dout].pin;

  nrfx_i2s_config_t config = NRF_DRV_I2S_DEFAULT_CONFIG;
  config.sck_pin = bclkPin;
  config.lrck_pin = lrckPin;
  config.mck_pin = NRFX_I2S_PIN_NOT_USED;
  config.sdout_pin = doutPin;
  config.sdin_pin = NRFX_I2S_PIN_NOT_USED;
  config.irq_priority = 7;
  config.mode = NRF_I2S_MODE_MASTER;
  config.format = NRF_I2S_FORMAT_I2S;
  config.alignment = NRF_I2S_ALIGN_LEFT;
  config.sample_width = NRF_I2S_SWIDTH_8BIT;
  switch (sample_width_bytes) {
    case 1:
      config.sample_width = NRF_I2S_SWIDTH_8BIT;
      break;
    case 2:
      config.sample_width = NRF_I2S_SWIDTH_16BIT;
      break;
    case 3:
      config.sample_width = NRF_I2S_SWIDTH_24BIT;
      break;
    default:
      // unsupported sample width
      return false;
  }
  config.channels = NRF_I2S_CHANNELS_LEFT;
  config.mck_setup = NRF_I2S_MCK_DISABLED;
  // configure sample rate
  // see https://docs.nordicsemi.com/bundle/ps_nrf52840/page/i2s.html#ariaid-title6 (section "Master clock (MCK)")
  config.mck_setup = NRF_I2S_MCK_32MDIV31;
  config.ratio = NRF_I2S_RATIO_32X;

  uint32_t err_code = nrfx_i2s_init(&config, jsi2s_data_handler);
  if (err_code != NRF_SUCCESS) {
    nrfx_i2s_uninit();
    jsi2s_initialized = false;
    return false;
  }
  jsi2s_initialized = true;
  return true;
}

void jsi2s_uninit() {
  if (!jsi2s_initialized) {
    return;
  }
  if (jsi2s_active) {
    jsi2s_stop();
  }
  nrfx_i2s_uninit();
  jsi2s_initialized = false;
}

bool jsi2s_start(uint32_t *buf_ptr, uint32_t buffer_size_bytes, jsi2s_data_cb_t data_callback,
                 jsi2s_buffer_release_cb_t buffer_released_callback) {
  if (!jsi2s_initialized) {
    jsiConsolePrint("I2S: not initialized\n");
    return false;
  }
  if (jsi2s_active) {
    jsi2s_stop();
  }
  if (buf_ptr == NULL) {
    return false;
  }
  if (buffer_size_bytes == 0) {
    jsiConsolePrint("I2S start: buffer must not be empty\n");
    return false;
  }
  if ((buffer_size_bytes % 4) != 0) {
    jsiConsolePrint("I2S: buffer must be a multiple of 32 Byte words in size\n");
    return false;
  }
  if (data_callback == NULL) {
    jsiConsolePrint("I2S: data_callback not provided\n");
    return false;
  }
  if (buffer_released_callback == NULL) {
    jsiConsolePrint("I2S: buffer_released_callback not provided\n");
    return false;
  }
  jsi2s_set_callbacks(data_callback, buffer_released_callback);
  nrfx_i2s_buffers_t buffers;
  memset(&buffers, 0, sizeof(buffers));
  buffers.p_rx_buffer = NULL;
  buffers.p_tx_buffer = buf_ptr;
  uint8_t flags = 0;
  jsi2s_buffer_size_bytes = buffer_size_bytes;
  jsi2s_active = true;
  uint32_t err_code = nrfx_i2s_start(&buffers, buffer_size_bytes / 4, flags);
  if (err_code != NRF_SUCCESS) {
    jsi2s_buffer_size_bytes = 0;
    jsi2s_unset_callbacks();
    jsi2s_active = false;
    return false;
  }
  return true;
}

void jsi2s_stop() {
  if (!jsi2s_initialized) {
    jsiConsolePrint("I2S: not initialized\n");
    return;
  }
  nrfx_i2s_stop();
  jsi2s_unset_callbacks();
  jsi2s_buffer_size_bytes = 0;
  jsi2s_active = false;
}

bool jsi2s_set_next_buffer(uint32_t *buf_ptr, uint32_t buffer_size_bytes) {
  if (!jsi2s_initialized) {
    jsiConsolePrint("I2S: not initialized\n");
    return false;
  }
  if (!jsi2s_active) {
    jsiConsolePrint("I2S: not active\n");
    return false;
  }
  if (buffer_size_bytes != jsi2s_buffer_size_bytes) {
    jsiConsolePrintf("I2S: Buffers must have the same size! (expected: %d, got: %d)\n", jsi2s_buffer_size_bytes,
                     buffer_size_bytes);
    return false;
  }
  if (buf_ptr == NULL) {
    return false;
  }
  nrfx_i2s_buffers_t buffers;
  memset(&buffers, 0, sizeof(buffers));
  buffers.p_rx_buffer = NULL;
  buffers.p_tx_buffer = buf_ptr;
  uint32_t err_code = nrfx_i2s_next_buffers_set(&buffers);
  if (err_code != NRF_SUCCESS) {
    return false;
  }
  return true;
}

bool jsi2s_idle() {
  return false; // we'll be woken up by the I2S interrupt
}
