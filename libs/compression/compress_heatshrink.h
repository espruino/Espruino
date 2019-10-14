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
 *  Wrapper for heatshrink encode/decode
 * ----------------------------------------------------------------------------
 */

typedef struct {
  unsigned char *ptr;
  size_t len;
} HeatShrinkPtrInputCallbackInfo;

void heatshrink_ptr_output_cb(unsigned char ch, uint32_t *cbdata); // takes **data
int heatshrink_ptr_input_cb(uint32_t *cbdata); // takes *HeatShrinkPtrInputCallbackInfo
void heatshrink_var_output_cb(unsigned char ch, uint32_t *cbdata); // takes *JsvStringIterator
int heatshrink_var_input_cb(uint32_t *cbdata); // takes *JsvIterator

/** gets data from callback, writes to callback if nonzero. Returns total length. */
uint32_t heatshrink_encode_cb(int (*in_callback)(uint32_t *cbdata), uint32_t *in_cbdata, void (*out_callback)(unsigned char ch, uint32_t *cbdata), uint32_t *out_cbdata);

/** gets data from callback, writes it into callback if nonzero. Returns total length */
uint32_t heatshrink_decode_cb(int (*in_callback)(uint32_t *cbdata), uint32_t *in_cbdata, void (*out_callback)(unsigned char ch, uint32_t *cbdata), uint32_t *out_cbdata);

/** gets data from array, writes to callback if nonzero. Returns total length. */
uint32_t heatshrink_encode(unsigned char *in_data, size_t in_len, void (*out_callback)(unsigned char ch, uint32_t *cbdata), uint32_t *out_cbdata);

/** gets data from callback, writes it into array if nonzero. Returns total length */
uint32_t heatshrink_decode(int (*in_callback)(uint32_t *cbdata), uint32_t *in_cbdata, unsigned char *out_data);
