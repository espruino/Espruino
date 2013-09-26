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
 * Graphics Backend for drawing to ArrayBuffer
 * ----------------------------------------------------------------------------
 */
#include "jswrap_arraybuffer.h"
#include "lcd_arraybuffer.h"
#include "jsvar.h"

int lcdGetPixelIndex_ArrayBuffer(JsGraphics *gfx, short x, short y) {
  if (gfx->data.flags & JSGRAPHICSFLAGS_ARRAYBUFFER_ZIGZAG) {
    if (y&1) x = gfx->data.width-(1+x);
  }
  if (gfx->data.flags & JSGRAPHICSFLAGS_ARRAYBUFFER_VERTICAL_BYTE)
    return ((x + (y>>3)*gfx->data.width)<<3) | (y&7);
  else
    return (x + y*gfx->data.width)*gfx->data.bpp;
}

unsigned int lcdGetPixel_ArrayBuffer(JsGraphics *gfx, short x, short y) {
  unsigned int col = 0;
  JsVar *buf = jsvSkipNameAndUnLock(jsvFindChildFromString(gfx->graphicsVar, "buffer", false));
  if (buf && jsvIsArrayBuffer(buf)) {
    int idx = lcdGetPixelIndex_ArrayBuffer(gfx,x,y);
    JsvArrayBufferIterator it;
    jsvArrayBufferIteratorNew(&it, buf, idx>>3 );
    if (gfx->data.bpp < 8) {
      idx = idx & 7;
      int mask = (1<<gfx->data.bpp)-1;
      int existing = jsvArrayBufferIteratorGetIntegerValue(&it);
      col = (existing>>idx)&mask;
    } else {
      int i;
      for (i=0;i<gfx->data.bpp;i+=8) {
        col |= jsvArrayBufferIteratorGetIntegerValue(&it)<<i;
        jsvArrayBufferIteratorNext(&it);
      }
    }
    jsvArrayBufferIteratorFree(&it);
  }
  return col;
}


void lcdSetPixel_ArrayBuffer(JsGraphics *gfx, short x, short y, unsigned int col) {
  JsVar *buf = jsvSkipNameAndUnLock(jsvFindChildFromString(gfx->graphicsVar, "buffer", false));
  if (buf && jsvIsArrayBuffer(buf)) {
    int idx = lcdGetPixelIndex_ArrayBuffer(gfx,x,y);
    JsvArrayBufferIterator it;
    jsvArrayBufferIteratorNew(&it, buf, idx>>3 );
    if (gfx->data.bpp < 8) {
      idx = idx & 7;
      int mask = (1<<gfx->data.bpp)-1;
      int existing = jsvArrayBufferIteratorGetIntegerValue(&it);
      jsvArrayBufferIteratorSetIntegerValue(&it, (existing&~(mask<<idx)) | ((col&mask)<<idx));
    } else {
      int i;
      for (i=0;i<gfx->data.bpp;i+=8) {
        jsvArrayBufferIteratorSetIntegerValue(&it, col >> i);
        jsvArrayBufferIteratorNext(&it);
      }
    }
    jsvArrayBufferIteratorFree(&it);
    jsvUnLock(buf);
  }
}

void lcdInit_ArrayBuffer(JsGraphics *gfx) {
  // create buffer
  JsVar *buf = jswrap_arraybuffer_constructor(gfx->data.width * gfx->data.height * gfx->data.bpp / 8);
  jsvAddNamedChild(gfx->graphicsVar, buf, "buffer");
  jsvUnLock(buf);
}

void lcdSetCallbacks_ArrayBuffer(JsGraphics *gfx) {
  gfx->setPixel = lcdSetPixel_ArrayBuffer;
  gfx->getPixel = lcdGetPixel_ArrayBuffer;
  // TODO: Optimised fill?
}
