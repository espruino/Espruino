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
 * JavaScript methods for working with the Quite Ok Audio Format (QOA)
 * ----------------------------------------------------------------------------
 */

#include "jswrap_qoa.h"
#include "qoa.h"
#include "jsinteractive.h"

/*JSON{
  "type" : "class",
  "class" : "QOA"
}
Class for working with the [Quite Ok Audio Format (QOA)](https://qoaformat.org).

QOA does reasonably fast lossy audio compression at 3.2 bits per sample.

You can use the [reference encoder](https://github.com/phoboslab/qoa) to encode some audio,
decode that audio on-device and then play decoded audio with the `Waveform` class.
*/

/*JSON{
  "type" : "staticmethod",
  "class" : "QOA",
  "name" : "initDecode",
  "ifndef" : "SAVE_ON_FLASH",
  "generate" : "jswrap_qoa_init_decode",
  "params" : [
    ["header","JsVar","an arraybuffer containing (at least) the header of the qoa file"]
  ],
  "return" : ["JsVar","an object {firstFramePos, qoaState} containing the first frame pos as an integer and the qoa decoder state as an ArrayBuffer, or null on error"]
}
Initialise a QOA decoding context.
*/
JsVar *jswrap_qoa_init_decode(JsVar *header) {
  if (!jsvIsArrayBuffer(header)) {
    jsiConsolePrint("error: header is not an arraybuffer\n");
    return NULL;
  }
  unsigned char headerBuf[QOA_MIN_FILESIZE]; {
    JsVar *headerStr = jsvGetArrayBufferBackingString(header, NULL);
    size_t readChars = jsvGetStringChars(headerStr, 0, headerBuf, QOA_MIN_FILESIZE);
    jsvUnLock(headerStr);
    if (readChars < QOA_MIN_FILESIZE) {
      jsiConsolePrint("error: not enough data read from header\n");
      return NULL;
    }
  }
  qoa_desc *qoa = NULL;
  JsVar *qoaStateBuf = jsvNewArrayBufferWithPtr(sizeof(qoa_desc), (char **) &qoa);
  uint32_t firstFramePos = qoa_decode_header(headerBuf, QOA_MIN_FILESIZE, qoa);
  if (!firstFramePos) {
    jsvUnLock(qoaStateBuf);
    jsiConsolePrint("error: failed to decode QOA header\n");
    return NULL;
  }
  if (qoa->channels > 1) {
    jsvUnLock(qoaStateBuf);
    jsiConsolePrint("error: too many channels; only single channel audio is supported at the moment\n");
    return NULL;
  }
  JsVar *result = jsvNewObject();
  jsvAddNamedChildAndUnLock(result, jsvNewFromInteger((JsVarInt) firstFramePos), "firstFramePos");
  jsvAddNamedChildAndUnLock(result, qoaStateBuf, "qoaState");
  return result;
}

/*JSON{
  "type" : "staticproperty",
  "class" : "QOA",
  "name" : "FRAME_LEN",
  "generate" : "jswrap_qoa_frame_len",
  "return" : ["JsVar","Number of samples in a frame of decoded audio."]
}*/
JsVar *jswrap_qoa_frame_len() {
  return jsvNewFromInteger(QOA_FRAME_LEN);
}

/*JSON{
  "type" : "staticmethod",
  "class" : "QOA",
  "name" : "decode",
  "ifndef" : "SAVE_ON_FLASH",
  "generate" : "jswrap_qoa_decode",
  "params" : [
    ["encoded","JsVar","an ArrayBuffer containing some frames of a qoa file"],
    ["decoded","JsVar","an ArrayBuffer to write decoded samples into"],
    ["options","JsVar","an object containing {offset, qoaState, bits, fill}; see below"]
  ],
  "return" : ["JsVar","an object with two integers: {frameLen, frameSize}, or null on error"]
}

Decode some QOA encoded audio samples.

Data is read from the "encoded" buffer and written to the "decoded" buffer.
Note that the "decoded" buffer needs to be at least `QOA.FRAME_LEN` long and match the number of bits per sample.

Decoded samples are unsigned integers and take up either 8 or 16 bits, depending on the `bits` argument.
The default is 16 bits per sample, which is what QOA usually decodes to.
But you may choose 8 bits per sample if you want to save some RAM.

The options object must contain:
- offset: where to start reading from the encoded data buffer
- qoaState: an ArrayBuffer containing the qoa decoder state
And can optionally contain:
- bits: number of bits per sample to write to the decoded data buffer, either 8 or 16 (default is 16)
- fill: bool; true to fill remaining decoded data buffer with silence
*/
JsVar *jswrap_qoa_decode(JsVar *encoded, JsVar *decoded, JsVar *options) {
  bool use16Bit = true;
  bool fillRestOfBuffer = false;
  int offsetVal = 0; {
    JsVar *offset = jsvObjectGetChildIfExists(options, "offset");
    if (jsvIsIntegerish(offset)) {
      offsetVal = jsvGetInteger(offset);
    } else {
      jsiConsolePrint("error: options must contain a numeric \"offset\"\n");
      jsvUnLock(offset);
      return NULL;
    }
    jsvUnLock(offset);
  } {
    JsVar *bits = jsvObjectGetChildIfExists(options, "bits");
    if (jsvIsIntegerish(bits)) {
      int bitsVal = jsvGetInteger(bits);
      if (bitsVal == 8) {
        use16Bit = false;
      } else if (bitsVal == 16) {
        use16Bit = true;
      } else {
        jsiConsolePrint("error: bits must be either 8 or 16\n");
        jsvUnLock(bits);
        return NULL;
      }
    }
    jsvUnLock(bits);
  } {
    JsVar *fill = jsvObjectGetChildIfExists(options, "fill");
    if (jsvGetBool(fill)) {
      fillRestOfBuffer = true;
    }
    jsvUnLock(fill);
  } {
    uint32_t decodedBufStrOffset = 0;
    JsVar *decodedBufStr = jsvGetArrayBufferBackingString(decoded, &decodedBufStrOffset);
    size_t decodedBufLength = jsvGetStringLength(decodedBufStr) - decodedBufStrOffset;
    size_t minNeededDecodedBufLength = QOA_FRAME_LEN * (use16Bit ? 2 : 1);
    jsvUnLock(decodedBufStr);
    if (decodedBufLength < minNeededDecodedBufLength) {
      jsiConsolePrintf("error: encoded data buffer not big enough; must be at least %d samples long\n", QOA_FRAME_LEN);
      return NULL;
    }
  }
  JsVar *qoaState = jsvObjectGetChildIfExists(options, "qoaState");
  if (qoaState == NULL) {
    jsiConsolePrint("error: options must contain \"qoaState\"\n");
    jsvUnLock(qoaState);
    return NULL;
  }
  uint32_t qoaStateStrOffset = 0;
  JsVar *qoaStateStr = jsvGetArrayBufferBackingString(qoaState, &qoaStateStrOffset);
  if (!jsvIsFlatString(qoaStateStr)) {
    jsiConsolePrintf("error: qoaState isn't backed by a contiguous area of memory\n");
    jsvUnLock2(qoaState, qoaStateStr);
    return NULL;
  } {
    size_t qoaStateLength = jsvGetStringLength(qoaStateStr) - qoaStateStrOffset;
    if (qoaStateLength != sizeof(qoa_desc)) {
      jsvUnLock2(qoaState, qoaStateStr);
      jsiConsolePrintf("error: qoaState has wrong size (wanted: %d given: %d)\n", sizeof(qoa_desc), qoaStateLength);
      return NULL;
    }
  }
  qoa_desc *qoa = (qoa_desc *) (jsvGetFlatStringPointer(qoaStateStr) + qoaStateStrOffset);
  JsVar *encodedStr = jsvGetArrayBufferBackingString(encoded, NULL);
  static const size_t frameSize = QOA_FRAME_SIZE(1, QOA_SLICES_PER_FRAME);
  unsigned char encodedData[frameSize];
  size_t readChars = jsvGetStringChars(encodedStr, offsetVal, encodedData, frameSize);
  jsvUnLock(encodedStr);
  unsigned int frameLen;
  JsvArrayBufferIterator it;
  jsvArrayBufferIteratorNew(&it, decoded, 0);
  unsigned int encodedFrameSize = qoa_decode_frame(encodedData, readChars, qoa, &(it.it), &frameLen, use16Bit);
  if (fillRestOfBuffer) {
    while (jsvStringIteratorHasChar(&(it.it))) {
      if (use16Bit) {
        jsvStringIteratorSetCharAndNext(&(it.it), 0);
        jsvStringIteratorSetCharAndNext(&(it.it), 1 << 7);
      } else {
        jsvStringIteratorSetCharAndNext(&(it.it), 1 << 7);
      }
    }
  }
  jsvArrayBufferIteratorFree(&it);
  jsvUnLock2(qoaState, qoaStateStr);
  JsVar *result = jsvNewObject();
  jsvAddNamedChildAndUnLock(result, jsvNewFromInteger((JsVarInt) frameLen), "frameLen");
  jsvAddNamedChildAndUnLock(result, jsvNewFromInteger((JsVarInt) encodedFrameSize), "frameSize");
  return result;
}
