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
 *  Simple RLE EncoderDecoder
 * ----------------------------------------------------------------------------
 */

#include "compress_rle.h"

/** gets data from array, writes to callback if nonzero. Returns total length. */
uint32_t rle_encode(unsigned char *data, size_t dataLen, void (*callback)(unsigned char ch, uint32_t *cbdata), uint32_t *cbdata) {
  uint32_t outputLen = 0;
  int lastCh = -1; // not a valid char
  while (dataLen) {
    unsigned char ch = *(data++);
    dataLen--;
    outputLen++;
    if (callback) callback(ch, cbdata);
    if (ch==lastCh) {
      int cnt = 0;
      while (dataLen && lastCh==*data && cnt<255) {
        data++;
        dataLen--;
        cnt++;
      }
      outputLen++;
      if (callback) callback((unsigned char)cnt, cbdata);
    }
    lastCh = ch;
  }
  return outputLen;
}

/** gets data from callback, writes it into array if nonzero. Returns total length */
uint32_t rle_decode(int (*callback)(uint32_t *cbdata), uint32_t *cbdata, unsigned char *data) {
  uint32_t outputLen = 0;
  int lastCh = -256; // not a valid char
  while (true) {
    int ch = callback(cbdata);
    if (ch<0) return outputLen;
    if (data) data[outputLen] = (unsigned char)ch;
    outputLen++;
    if (ch==lastCh) {
      int cnt = callback(cbdata);
      while (cnt-->0) {
        if (data) data[outputLen] = (unsigned char)ch;
        outputLen++;
      }
    }
    lastCh = ch;
  }
  return outputLen;
}
