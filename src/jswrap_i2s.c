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

#include "jswrap_i2s.h"
#include "jsinteractive.h"
#include "jsi2s.h"

bool jswrap_i2s_initialized = false;
bool jswrap_i2s_active = false;
JsVar *jswrap_i2s_data_callback = NULL;
JsVar *jswrap_i2s_buffer_released_callback = NULL;
volatile bool jswrap_i2s_needs_more_data_flag = false;

struct jswrap_i2s_buf_t {
  JsVar *buf;
  volatile uint32_t *buf_ptr;
  volatile bool release;
};

#define JSWRAP_I2S_BUFFER_COUNT 2
struct jswrap_i2s_buf_t jswrap_i2s_buffers[JSWRAP_I2S_BUFFER_COUNT];

bool jswrap_i2s_acquire_buf(JsVar *buf, uint32_t *buf_ptr) {
  for (size_t i = 0; i < JSWRAP_I2S_BUFFER_COUNT; i++) {
    struct jswrap_i2s_buf_t *buf_entry = &jswrap_i2s_buffers[i];
    if (buf_entry->buf == NULL) {
      jsvLockAgain(buf);
      buf_entry->release = false;
      buf_entry->buf = buf;
      buf_entry->buf_ptr = buf_ptr;
      return true;
    }
  }
  return false;
}

bool jswrap_i2s_mark_buf_for_release(const uint32_t *buf_ptr) {
  for (size_t i = 0; i < JSWRAP_I2S_BUFFER_COUNT; i++) {
    struct jswrap_i2s_buf_t *buf_entry = &jswrap_i2s_buffers[i];
    if (buf_entry->buf_ptr == buf_ptr) {
      buf_entry->release = true;
      return true;
    }
  }
  return false;
}

void jswrap_i2s_on_buffer_released(JsVar *buf) {
  if (jswrap_i2s_buffer_released_callback != NULL) {
    jspExecuteFunction(jswrap_i2s_buffer_released_callback, NULL, 1, &buf);
  }
}

void jswrap_i2s_release_marked_bufs() {
  for (size_t i = 0; i < JSWRAP_I2S_BUFFER_COUNT; i++) {
    struct jswrap_i2s_buf_t *buf_entry = &jswrap_i2s_buffers[i];
    if (buf_entry->release && (buf_entry->buf != NULL)) {
      JsVar *buf = buf_entry->buf;
      buf_entry->buf = NULL;
      buf_entry->buf_ptr = NULL;
      buf_entry->release = false;
      jswrap_i2s_on_buffer_released(buf);
      jsvUnLock(buf);
    }
  }
}

void jswrap_i2s_release_all_buffers() {
  for (size_t i = 0; i < JSWRAP_I2S_BUFFER_COUNT; i++) {
    struct jswrap_i2s_buf_t *buf_entry = &jswrap_i2s_buffers[i];
    if (buf_entry->buf != NULL) {
      JsVar *buf = buf_entry->buf;
      buf_entry->buf = NULL;
      jswrap_i2s_on_buffer_released(buf);
      jsvUnLock(buf);
    }
  }
  memset(jswrap_i2s_buffers, 0, sizeof(struct jswrap_i2s_buf_t) * JSWRAP_I2S_BUFFER_COUNT);
}

bool jswrap_i2s_acquire_callbacks(JsVar *data_callback, JsVar *buffer_released_callback) {
  if ((data_callback == NULL) || (buffer_released_callback == NULL)) {
    return false;
  }
  jsvLockAgain(data_callback);
  jswrap_i2s_data_callback = data_callback;
  jsvLockAgain(buffer_released_callback);
  jswrap_i2s_buffer_released_callback = buffer_released_callback;
  return true;
}

void jswrap_i2s_release_callbacks() {
  if (jswrap_i2s_data_callback != NULL) {
    jsvUnLock(jswrap_i2s_data_callback);
    jswrap_i2s_data_callback = NULL;
  }
  if (jswrap_i2s_buffer_released_callback != NULL) {
    jsvUnLock(jswrap_i2s_buffer_released_callback);
    jswrap_i2s_buffer_released_callback = NULL;
  }
}

void jswrap_i2s_on_buffer_released_callback(uint32_t *buf_ptr) {
  if (jswrap_i2s_mark_buf_for_release(buf_ptr)) {
    jshHadEvent();
  }
}

void jswrap_i2s_on_more_data_requested_callback() {
  jswrap_i2s_needs_more_data_flag = true;
  jshHadEvent();
}

/*JSON{
  "type" : "staticmethod",
  "class" : "I2S",
  "name" : "init",
  "generate" : "jswrap_i2s_init",
  "params" : [
    ["bclk","pin","Bit Clock Pin"],
    ["lrck","pin","Left/Right Clock Pin"],
    ["dout","pin","Data Out Pin"],
    ["sample_width_bytes","int","Sample width in Bytes, one of 1, 2 or 3 (so 8 Bits, 16 Bits or 24 Bits)"]
  ],
  "return" : ["bool","true on success, false on error"]
}
Initialize I2S.
*/
bool jswrap_i2s_init(Pin bclk, Pin lrck, Pin dout, int sample_width_bytes) {
  if (jswrap_i2s_initialized) {
    jswrap_i2s_uninit();
  }
  memset(jswrap_i2s_buffers, 0, sizeof(struct jswrap_i2s_buf_t) * JSWRAP_I2S_BUFFER_COUNT);
  bool init_ok = jsi2s_init(bclk, lrck, dout, sample_width_bytes);
  jswrap_i2s_initialized = init_ok;
  jswrap_i2s_active = false;
  return init_ok;
}

/*JSON{
  "type" : "staticmethod",
  "class" : "I2S",
  "name" : "uninit",
  "generate" : "jswrap_i2s_uninit"
}
Uninitialize I2S.
*/
void jswrap_i2s_uninit() {
  if (!jswrap_i2s_initialized) {
    return;
  }
  jsi2s_uninit();
  jswrap_i2s_release_callbacks();
  jswrap_i2s_release_all_buffers();
  jswrap_i2s_initialized = false;
  jswrap_i2s_active = false;
}

/*JSON{
  "type" : "staticmethod",
  "class" : "I2S",
  "name" : "start",
  "generate" : "jswrap_i2s_start",
  "params" : [
    ["buf","JsVar","Data buffer"],
    ["data_callback","JsVar","Callback for requesting more data"],
    ["buffer_released_callback","JsVar","Called when a buffer is not used anymore (with that buffer as an argument)"]
  ],
  "return" : ["bool","true on success, false on error"]
}
Start an I2S transfer.
*/
bool jswrap_i2s_start(JsVar *buf, JsVar *data_callback, JsVar *buffer_released_callback) {
  if (!jswrap_i2s_initialized) {
    jsiConsolePrint("I2S: not initialized\n");
    return false;
  }
  if (jswrap_i2s_active) {
    jswrap_i2s_stop();
  }
  if (!jsvIsArrayBuffer(buf)) {
    jsiConsolePrint("I2S start: buffer is not an arraybuffer\n");
    return false;
  }
  size_t buf_len = 0;
  uint32_t *buf_ptr = (uint32_t *) jsvGetDataPointer(buf, &buf_len);
  if (buf_ptr == NULL) {
    jsiConsolePrint("I2S start: failed to get buffer address\n");
    return false;
  }
  if (!jsvIsFunction(data_callback)) {
    jsiConsolePrint("I2S: data_callback must be a function\n");
    return false;
  }
  if (!jsvIsFunction(buffer_released_callback)) {
    jsiConsolePrint("I2S: buffer_released_callback must be a function\n");
    return false;
  }
  jswrap_i2s_release_callbacks();
  jswrap_i2s_release_all_buffers();
  if (!jswrap_i2s_acquire_callbacks(data_callback, buffer_released_callback)) {
    jsiConsolePrint("I2S: failed to acquire callbacks\n");
    return false;
  }
  if (!jswrap_i2s_acquire_buf(buf, buf_ptr)) {
    jsiConsolePrint("I2S: failed to acquire buffer\n");
    jswrap_i2s_release_callbacks();
    return false;
  }
  jswrap_i2s_active = true;
  bool started = jsi2s_start(buf_ptr, buf_len, jswrap_i2s_on_more_data_requested_callback,
                             jswrap_i2s_on_buffer_released_callback);
  if (!started) {
    jswrap_i2s_active = false;
    jswrap_i2s_release_callbacks();
    jswrap_i2s_release_all_buffers();
  }
  return started;
}

/*JSON{
  "type" : "staticmethod",
  "class" : "I2S",
  "name" : "stop",
  "generate" : "jswrap_i2s_stop"
}
Stop an I2S transfer.
*/
void jswrap_i2s_stop() {
  if (!jswrap_i2s_initialized) {
    jsiConsolePrint("I2S: not initialized\n");
    return;
  }
  jsi2s_stop();
  jswrap_i2s_release_callbacks();
  jswrap_i2s_release_all_buffers();
  jswrap_i2s_active = false;
}

/*JSON{
  "type" : "staticmethod",
  "class" : "I2S",
  "name" : "setNextBuffer",
  "generate" : "jswrap_i2s_set_next_buffer",
  "params" : [
    ["buf","JsVar","Data buffer"]
  ],
  "return" : ["bool","true on success, false on error"]
}
Supply the next buffer used for an active I2S transfer.
*/
bool jswrap_i2s_set_next_buffer(JsVar *buf) {
  if (!jswrap_i2s_initialized) {
    jsiConsolePrint("I2S: not initialized\n");
    return false;
  }
  if (!jswrap_i2s_active) {
    jsiConsolePrint("I2S: not active\n");
    return false;
  }
  if (!jsvIsArrayBuffer(buf)) {
    jsiConsolePrint("I2S set next buffer: buffer is not an arraybuffer\n");
    return false;
  }
  size_t buf_len = 0;
  uint32_t *buf_ptr = (uint32_t *) jsvGetDataPointer(buf, &buf_len);
  if (buf_ptr == NULL) {
    jsiConsolePrint("I2S set next buffer: failed to get buffer address\n");
    return false;
  }
  jswrap_i2s_release_marked_bufs();
  if (!jswrap_i2s_acquire_buf(buf, buf_ptr)) {
    jsiConsolePrint("I2S: failed to acquire buffer\n");
    return false;
  }
  bool ok = jsi2s_set_next_buffer(buf_ptr, buf_len);
  if (!ok) {
    jswrap_i2s_mark_buf_for_release(buf_ptr);
    jswrap_i2s_release_marked_bufs();
    return false;
  }
  return ok;
}

/*JSON{
  "type" : "idle",
  "generate" : "jswrap_i2s_idle"
}*/
bool jswrap_i2s_idle() {
  if (!jswrap_i2s_initialized) {
    return false;
  }
  bool was_busy = jsi2s_idle();
  jswrap_i2s_release_marked_bufs();
  if (jswrap_i2s_needs_more_data_flag) {
    jswrap_i2s_needs_more_data_flag = false;
    if (jswrap_i2s_data_callback != NULL) {
      jspExecuteFunction(jswrap_i2s_data_callback, NULL, 0, NULL);
    }
  }
  return was_busy;
}

/*JSON{
  "type" : "kill",
  "generate" : "jswrap_i2s_kill"
}*/
void jswrap_i2s_kill() {
  if (jswrap_i2s_active) {
    jswrap_i2s_stop();
  }
  if (jswrap_i2s_initialized) {
    jswrap_i2s_uninit();
  }
}
