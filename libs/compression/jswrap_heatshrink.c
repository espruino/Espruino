/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2018 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * This file is designed to be parsed during the build process
 *
 * Simple compression/decompression using the heatshrink library
 * ----------------------------------------------------------------------------
 */
#include "jsvar.h"
#include "jsvariterator.h"
#include "compress_heatshrink.h"
#include "jswrap_heatshrink.h"
#include "jsparse.h"


/*JSON{
  "type" : "library",
  "class" : "heatshrink",
  "ifndef" : "SAVE_ON_FLASH"
}
Simple library for compression/decompression using [heatshrink](https://github.com/atomicobject/heatshrink), an [LZSS](https://en.wikipedia.org/wiki/Lempel%E2%80%93Ziv%E2%80%93Storer%E2%80%93Szymanski) compression tool.

Espruino uses heatshrink internally to compress RAM down to fit in Flash memory when `save()` is used. This just exposes that functionality.

Functions here take and return buffers of data. There is no support for streaming, so both the compressed and decompressed data must be able to fit in memory at the same time.
*/
typedef struct {
  char *ptr;
  size_t len;
} DecompressInfo;

static void _jswrap_heatshrink_compress_output(unsigned char ch, uint32_t *cbdata) {
  unsigned char **outPtr = (unsigned char**)cbdata;
  *((*outPtr)++) = ch;
}
static int _jswrap_heatshrink_decompress_input(uint32_t *cbdata) {
  DecompressInfo *decompressInfo = (DecompressInfo *)cbdata;
  if (!decompressInfo->len) return -1;
  decompressInfo->len--;
  return (unsigned char)*(decompressInfo->ptr++);
}

/*JSON{
  "type" : "staticmethod",
  "class" : "heatshrink",
  "name" : "compress",
  "generate" : "jswrap_heatshrink_compress",
  "params" : [
    ["data","JsVar","The data to compress"]
  ],
  "return" : ["JsVar","Returns the result as an ArrayBuffer"],
  "return_object" : "ArrayBuffer",
  "ifndef" : "SAVE_ON_FLASH"
}
*/
JsVar *jswrap_heatshrink_compress(JsVar *data) {
  JSV_GET_AS_CHAR_ARRAY(dataPtr, dataLen, data);
  if (!dataPtr) return 0;
  uint32_t compressedSize = heatshrink_encode((unsigned char*)dataPtr, dataLen, NULL, NULL);

  char *outPtr = 0;
  JsVar *outArr = jsvNewArrayBufferWithPtr((unsigned int)compressedSize, &outPtr);
  if (!outPtr) {
    jsError("Not enough memory for result");
    return 0;
  }
  heatshrink_encode((unsigned char*)dataPtr, dataLen, _jswrap_heatshrink_compress_output, (uint32_t*)&outPtr);
  return outArr;
}

/*JSON{
  "type" : "staticmethod",
  "class" : "heatshrink",
  "name" : "decompress",
  "generate" : "jswrap_heatshrink_decompress",
  "params" : [
    ["data","JsVar","The data to decompress"]
  ],
  "return" : ["JsVar","Returns the result as an ArrayBuffer"],
  "return_object" : "ArrayBuffer",
  "ifndef" : "SAVE_ON_FLASH"
}
*/
JsVar *jswrap_heatshrink_decompress(JsVar *data) {
  JSV_GET_AS_CHAR_ARRAY(dataPtr, dataLen, data);
  if (!dataPtr) return 0;
  DecompressInfo decompressInfo;
  decompressInfo.ptr = dataPtr;
  decompressInfo.len = dataLen;
  uint32_t decompressedSize = heatshrink_decode(_jswrap_heatshrink_decompress_input, (uint32_t*)&decompressInfo, NULL);

  char *outPtr = 0;
  JsVar *outArr = jsvNewArrayBufferWithPtr((unsigned int)decompressedSize, &outPtr);
  if (!outPtr) {
    jsError("Not enough memory for result");
    return 0;
  }
  decompressInfo.ptr = dataPtr;
  decompressInfo.len = dataLen;
  heatshrink_decode(_jswrap_heatshrink_decompress_input, (uint32_t*)&decompressInfo, (unsigned char*)outPtr);
  return outArr;
}
