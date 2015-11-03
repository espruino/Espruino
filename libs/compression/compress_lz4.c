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
#include "compress_lz4.h"
#include "lz4.h"

#define CHUNKSIZE 238

/** gets data from array, writes to callback */
void lz4_encode(unsigned char *data, size_t dataLen, void (*callback)(unsigned char ch, uint32_t *cbdata), uint32_t *cbdata) {
  LZ4_stream_t lz4Stream;
  LZ4_resetStream(&lz4Stream);

  const size_t cmpBufBytes = LZ4_COMPRESSBOUND(CHUNKSIZE);
  assert(cmpBufBytes < 256);
  char cmpBuf[cmpBufBytes];

  size_t inpOffset = 0;

  while (inpOffset < dataLen) {
    size_t inpBytes = CHUNKSIZE;
    if (inpOffset+inpBytes > dataLen)
      inpBytes = dataLen-inpOffset;

    int cmpBytes = LZ4_compress_fast_continue(
        &lz4Stream, (char*)&data[inpOffset], cmpBuf, (int)inpBytes, (int)cmpBufBytes, 1);
    inpOffset += inpBytes;
    if (cmpBytes <= 0) break;
    assert(cmpBytes < 256);
    // write chunk length
    callback((unsigned char)cmpBytes, cbdata);
    // write chunk data
    int i;
    for (i=0;i<cmpBytes;i++) callback((unsigned char)cmpBuf[i], cbdata);
  }
  callback(0, cbdata);
}

/** gets data from callback, writes it into array */
void lz4_decode(int (*callback)(uint32_t *cbdata), uint32_t *cbdata, unsigned char *data) {
  LZ4_streamDecode_t lz4StreamDecode;
  memset(&lz4StreamDecode, 0, sizeof(lz4StreamDecode));

  char cmpBuf[LZ4_COMPRESSBOUND(CHUNKSIZE)];
  int decOffset = 0;

  while (true) {
    // chunk length
    int cmpBytes = callback(cbdata);
    // chunk length 0 = end, <0 is EOF
    if (cmpBytes<=0) break;
    // chunk data
    int i;
    for (i=0;i<cmpBytes;i++)
      cmpBuf[i] = (char)callback(cbdata);
    // decompress
    int decBytes = LZ4_decompress_safe_continue(
             &lz4StreamDecode, cmpBuf, (char*)&data[decOffset], cmpBytes, CHUNKSIZE);
    if (decBytes <= 0) break;
    decOffset += decBytes;
  }
}
