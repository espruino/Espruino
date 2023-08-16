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
Simple library for compression/decompression using
[heatshrink](https://github.com/atomicobject/heatshrink), an
[LZSS](https://en.wikipedia.org/wiki/Lempel%E2%80%93Ziv%E2%80%93Storer%E2%80%93Szymanski)
compression tool.

Espruino uses heatshrink internally to compress RAM down to fit in Flash memory
when `save()` is used. This just exposes that functionality.

Functions here take and return buffers of data. There is no support for
streaming, so both the compressed and decompressed data must be able to fit in
memory at the same time.

```
var c = require("heatshrink").compress("Hello World");
// =new Uint8Array([....]).buffer
var d = require("heatshrink").decompress(c);
// =new Uint8Array([72, 101, ...]).buffer
E.toString(d)
// ="Hello World"
```

If you'd like a way to perform compression/decompression on desktop, check out https://github.com/espruino/EspruinoWebTools#heatshrinkjs

*/


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
Compress the data supplied as input, and return heatshrink encoded data as an ArrayBuffer.

No type information is stored, and the `data` argument is treated as an array of bytes
(whether it is a `String`/`Uint8Array` or even `Uint16Array`), so the result of
decompressing any compressed data will always be an ArrayBuffer.

If you'd like a way to perform compression/decompression on desktop, check out https://github.com/espruino/EspruinoWebTools#heatshrinkjs
*/
JsVar *jswrap_heatshrink_compress(JsVar *data) {
  if (!jsvIsIterable(data)) {
    jsExceptionHere(JSET_TYPEERROR,"Expecting something iterable, got %t",data);
    return 0;
  }
  JsvIterator in_it;
  JsvStringIterator out_it;

  jsvIteratorNew(&in_it, data, JSIF_EVERY_ARRAY_ELEMENT);
  uint32_t compressedSize = heatshrink_encode_cb(heatshrink_var_input_cb, (uint32_t*)&in_it, NULL, NULL);
  jsvIteratorFree(&in_it);

  JsVar *outVar = jsvNewStringOfLength((unsigned int)compressedSize, NULL);
  if (!outVar) {
    jsError("Not enough memory for result");
    return 0;
  }

  jsvIteratorNew(&in_it, data, JSIF_EVERY_ARRAY_ELEMENT);
  jsvStringIteratorNew(&out_it,outVar,0);
  heatshrink_encode_cb(heatshrink_var_input_cb, (uint32_t*)&in_it, heatshrink_var_output_cb, (uint32_t*)&out_it);
  jsvStringIteratorFree(&out_it);
  jsvIteratorFree(&in_it);

  JsVar *ab = jsvNewArrayBufferFromString(outVar, 0);
  jsvUnLock(outVar);
  return ab;
}


/*JSON{
  "type" : "staticmethod",
  "class" : "heatshrink",
  "name" : "decompress",
  "generate" : "jswrap_heatshrink_decompress",
  "params" : [
    ["data","JsVar","The data to decompress"]
  ],
  "return" : ["JsVar","Returns the result as an `ArrayBuffer`"],
  "return_object" : "ArrayBuffer",
  "ifndef" : "SAVE_ON_FLASH"
}
Decompress the heatshrink-encoded data supplied as input, and return it as an `ArrayBuffer`.

To get the result as a String, wrap `require("heatshrink").decompress` in `E.toString`: `E.toString(require("heatshrink").decompress(...))`

If you'd like a way to perform compression/decompression on desktop, check out https://github.com/espruino/EspruinoWebTools#heatshrinkjs
*/
JsVar *jswrap_heatshrink_decompress(JsVar *data) {
  if (!jsvIsIterable(data)) {
    jsExceptionHere(JSET_TYPEERROR,"Expecting something iterable, got %t",data);
    return 0;
  }
  JsvIterator in_it;
  JsvStringIterator out_it;

  jsvIteratorNew(&in_it, data, JSIF_EVERY_ARRAY_ELEMENT);
  uint32_t decompressedSize = heatshrink_decode(heatshrink_var_input_cb, (uint32_t*)&in_it, NULL);
  jsvIteratorFree(&in_it);

  JsVar *outVar = jsvNewStringOfLength((unsigned int)decompressedSize, NULL);
  if (!outVar) {
    jsError("Not enough memory for result");
    return 0;
  }

  jsvIteratorNew(&in_it, data, JSIF_EVERY_ARRAY_ELEMENT);
  jsvStringIteratorNew(&out_it,outVar,0);
  heatshrink_decode_cb(heatshrink_var_input_cb, (uint32_t*)&in_it, heatshrink_var_output_cb, (uint32_t*)&out_it);
  jsvStringIteratorFree(&out_it);
  jsvIteratorFree(&in_it);

  JsVar *ab = jsvNewArrayBufferFromString(outVar, 0);
  jsvUnLock(outVar);
  return ab;
}
