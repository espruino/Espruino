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
 *  Wrapper for LZ4 encode/decode
 * ----------------------------------------------------------------------------
 */

#include "jsutils.h"
#include "jsinteractive.h"
#include "jsdevices.h"
#include "jsvariterator.h"
#include "compress_heatshrink.h"
#include "heatshrink_encoder.h"
#include "heatshrink_decoder.h"

#define BUFFERSIZE 128

void heatshrink_ptr_output_cb(unsigned char ch, uint32_t *cbdata) {
  unsigned char **outPtr = (unsigned char**)cbdata;
  *((*outPtr)++) = ch;
}
int heatshrink_ptr_input_cb(uint32_t *cbdata) {
  HeatShrinkPtrInputCallbackInfo *info = (HeatShrinkPtrInputCallbackInfo *)cbdata;
  if (!info->len) return -1;
  info->len--;
  return *(info->ptr++);
}

void heatshrink_var_output_cb(unsigned char ch, uint32_t *cbdata) {
  JsvStringIterator *it = (JsvStringIterator *)cbdata;
  jsvStringIteratorSetCharAndNext(it, (char)ch);
}

int heatshrink_var_input_cb(uint32_t *cbdata) {
  JsvIterator *it = (JsvIterator *)cbdata;
  int d = -1;
  if (jsvIteratorHasElement(it))
    d = jsvIteratorGetIntegerValue(it) & 0xFF;
  jsvIteratorNext(it);
  return d;
}

/** gets data from callback, writes to callback if nonzero. Returns total length. */
uint32_t heatshrink_encode_cb(int (*in_callback)(uint32_t *cbdata), uint32_t *in_cbdata, void (*out_callback)(unsigned char ch, uint32_t *cbdata), uint32_t *out_cbdata) {
  heatshrink_encoder hse;
  uint8_t inBuf[BUFFERSIZE];
  uint8_t outBuf[BUFFERSIZE];
  heatshrink_encoder_reset(&hse);

  size_t i;
  size_t count = 0;
  size_t sunk = 0;
  size_t polled = 0;
  int lastByte = 0;
  size_t inBufCount = 0;
  size_t inBufOffset = 0;
  while (lastByte >= 0 || inBufCount>0) {
    // Read data from input
    if (inBufCount==0) {
      inBufOffset = 0;
      while (inBufCount<BUFFERSIZE && lastByte>=0) {
        lastByte = in_callback(in_cbdata);
        if (lastByte >= 0)
          inBuf[inBufCount++] = (uint8_t)lastByte;
      }
    }
    // encode
    bool ok = heatshrink_encoder_sink(&hse, &inBuf[inBufOffset], inBufCount, &count) >= 0;
    assert(ok);NOT_USED(ok);
    inBufCount -= count;
    inBufOffset += count;
    sunk += count;
    if ((inBufCount==0) && (lastByte < 0)) {
      heatshrink_encoder_finish(&hse);
    }

    HSE_poll_res pres;
    do {
      pres = heatshrink_encoder_poll(&hse, outBuf, sizeof(outBuf), &count);
      assert(pres >= 0);
      if (out_callback)
        for (i=0;i<count;i++)
          out_callback(outBuf[i], out_cbdata);
      polled += count;
    } while (pres == HSER_POLL_MORE);
    assert(pres == HSER_POLL_EMPTY);
    if ((inBufCount==0) && (lastByte < 0)) {
      heatshrink_encoder_finish(&hse);
    }
  }
  return (uint32_t)polled;
}

/** gets data from callback, writes it into callback if nonzero. Returns total length */
uint32_t heatshrink_decode_cb(int (*in_callback)(uint32_t *cbdata), uint32_t *in_cbdata, void (*out_callback)(unsigned char ch, uint32_t *cbdata), uint32_t *out_cbdata) {
  heatshrink_decoder hsd;
  uint8_t inBuf[BUFFERSIZE];
  uint8_t outBuf[BUFFERSIZE];
  heatshrink_decoder_reset(&hsd);

  size_t i;
  size_t count = 0;
  size_t sunk = 0;
  size_t polled = 0;
  int lastByte = 0;
  size_t inBufCount = 0;
  size_t inBufOffset = 0;
  while (lastByte >= 0 || inBufCount>0) {
    // Read data from input
    if (inBufCount==0) {
      inBufOffset = 0;
      while (inBufCount<BUFFERSIZE && lastByte>=0) {
        lastByte = in_callback(in_cbdata);
        if (lastByte >= 0)
          inBuf[inBufCount++] = (uint8_t)lastByte;
      }
    }
    // decode
    bool ok = heatshrink_decoder_sink(&hsd, &inBuf[inBufOffset], inBufCount, &count) >= 0;
    assert(ok);NOT_USED(ok);
    inBufCount -= count;
    inBufOffset += count;
    sunk += count;
    if ((inBufCount==0) && (lastByte < 0)) {
      heatshrink_decoder_finish(&hsd);
    }

    HSE_poll_res pres;
    do {
      pres = heatshrink_decoder_poll(&hsd, outBuf, sizeof(outBuf), &count);
      assert(pres >= 0);
      if (out_callback)
        for (i=0;i<count;i++)
          out_callback(outBuf[i], out_cbdata);
      polled += count;
    } while (pres == HSER_POLL_MORE);
    assert(pres == HSER_POLL_EMPTY);
    if (lastByte < 0) {
      heatshrink_decoder_finish(&hsd);
    }
  }
  return (uint32_t)polled;
}



/** gets data from array, writes to callback if nonzero. Returns total length. */
uint32_t heatshrink_encode(unsigned char *in_data, size_t in_len, void (*out_callback)(unsigned char ch, uint32_t *cbdata), uint32_t *out_cbdata) {
  HeatShrinkPtrInputCallbackInfo cbi;
  cbi.ptr = in_data;
  cbi.len = in_len;
  return heatshrink_encode_cb(heatshrink_ptr_input_cb, (uint32_t*)&cbi, out_callback, out_cbdata);
}

/** gets data from callback, writes it into array if nonzero. Returns total length */
uint32_t heatshrink_decode(int (*in_callback)(uint32_t *cbdata), uint32_t *in_cbdata, unsigned char *out_data) {
  unsigned char *dataptr = out_data;
  return heatshrink_decode_cb(in_callback, in_cbdata, out_data?heatshrink_ptr_output_cb:NULL, out_data?(uint32_t*)&dataptr:NULL);
}
