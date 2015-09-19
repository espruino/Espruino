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
#include "jsvariterator.h"

// returns the BIT index, so the bottom 3 bits specify the bit in the byte
unsigned int lcdGetPixelIndex_ArrayBuffer(JsGraphics *gfx, int x, int y, int pixelCount) {
  if (gfx->data.flags & JSGRAPHICSFLAGS_ARRAYBUFFER_ZIGZAG) {
    if (y&1) x = gfx->data.width - (x+pixelCount);
  }
  if (gfx->data.flags & JSGRAPHICSFLAGS_ARRAYBUFFER_VERTICAL_BYTE)
    return (unsigned int)(((x + (y>>3)*gfx->data.width)<<3) | (y&7));
  else
    return (unsigned int)((x + y*gfx->data.width)*gfx->data.bpp);
}

unsigned int lcdGetPixel_ArrayBuffer(JsGraphics *gfx, short x, short y) {
  unsigned int col = 0;
  JsVar *buf = jsvObjectGetChild(gfx->graphicsVar, "buffer", 0);
  if (buf && jsvIsArrayBuffer(buf)) {
    unsigned int idx = lcdGetPixelIndex_ArrayBuffer(gfx,x,y,1);
    JsvArrayBufferIterator it;
    jsvArrayBufferIteratorNew(&it, buf, idx>>3 );
    if (gfx->data.bpp&7/*not a multiple of one byte*/) {
      idx = idx & 7;
      unsigned int mask = (unsigned int)(1<<gfx->data.bpp)-1;
      unsigned int existing = (unsigned int)jsvArrayBufferIteratorGetIntegerValue(&it);
      unsigned int bitIdx = (gfx->data.flags & JSGRAPHICSFLAGS_ARRAYBUFFER_MSB) ? 8-(idx+gfx->data.bpp) : idx;
      col = ((existing>>bitIdx)&mask);
    } else {
      int i;
      for (i=0;i<gfx->data.bpp;i+=8) {
        col |= ((unsigned int)jsvArrayBufferIteratorGetIntegerValue(&it)) << i;
        jsvArrayBufferIteratorNext(&it);
      }
    }
    jsvArrayBufferIteratorFree(&it);
  }
  jsvUnLock(buf);
  return col;
}

// set pixelCount pixels starting at x,y
void lcdSetPixels_ArrayBuffer(JsGraphics *gfx, short x, short y, short pixelCount, unsigned int col) {
  JsVar *buf = jsvObjectGetChild(gfx->graphicsVar, "buffer", 0);
  if (buf && jsvIsArrayBuffer(buf)) {
    unsigned int idx = lcdGetPixelIndex_ArrayBuffer(gfx,x,y,pixelCount);
    JsvArrayBufferIterator it;
    jsvArrayBufferIteratorNew(&it, buf, idx>>3 );

    unsigned int whiteMask = (1U<<gfx->data.bpp)-1;
    bool shortCut = (col==0 || (col&whiteMask)==whiteMask) && (!(gfx->data.flags&JSGRAPHICSFLAGS_ARRAYBUFFER_VERTICAL_BYTE)); // simple black or white fill

    while (pixelCount--) { // writing individual bits
      if (gfx->data.bpp&7/*not a multiple of one byte*/) {
        idx = idx & 7;
        if (shortCut && idx==0) {
          // Basically, if we're aligned and we're filling all 0 or all 1
          // then we can go really quickly and can just fill
          int wholeBytes = (gfx->data.bpp*(pixelCount+1)) >> 3;
          if (wholeBytes) {
            char c = (char)(col?0xFF:0);
            pixelCount = (short)(pixelCount+1 - (wholeBytes*8/gfx->data.bpp));
            while (wholeBytes--) {
              jsvArrayBufferIteratorSetByteValue(&it,  c);
              jsvArrayBufferIteratorNext(&it);
            }
            continue;
          }
        }
        unsigned int mask = (unsigned int)(1<<gfx->data.bpp)-1;
        unsigned int existing = (unsigned int)jsvArrayBufferIteratorGetIntegerValue(&it);
        unsigned int bitIdx = (gfx->data.flags & JSGRAPHICSFLAGS_ARRAYBUFFER_MSB) ? 8-(idx+gfx->data.bpp) : idx;
        jsvArrayBufferIteratorSetByteValue(&it, (char)((existing&~(mask<<bitIdx)) | ((col&mask)<<bitIdx)));
        if (gfx->data.flags & JSGRAPHICSFLAGS_ARRAYBUFFER_VERTICAL_BYTE) {
          jsvArrayBufferIteratorNext(&it);
        } else {
          idx += gfx->data.bpp;
          if (idx>=8) jsvArrayBufferIteratorNext(&it);
        }
      } else { // we're writing whole bytes
        int i;
        for (i=0;i<gfx->data.bpp;i+=8) {
          jsvArrayBufferIteratorSetByteValue(&it, (char)(col >> i));
          jsvArrayBufferIteratorNext(&it);
        }
      }
    }
    jsvArrayBufferIteratorFree(&it);
  }
  jsvUnLock(buf);
}


void lcdSetPixel_ArrayBuffer(JsGraphics *gfx, short x, short y, unsigned int col) {
  lcdSetPixels_ArrayBuffer(gfx,x,y,1,col);
}

void  lcdFillRect_ArrayBuffer(struct JsGraphics *gfx, short x1, short y1, short x2, short y2) {
  short y;
  for (y=y1;y<=y2;y++)
    lcdSetPixels_ArrayBuffer(gfx, x1, y, (short)(1+x2-x1), gfx->data.fgColor);
}

void lcdInit_ArrayBuffer(JsGraphics *gfx) {
  // create buffer
  JsVar *buf = jswrap_arraybuffer_constructor((gfx->data.width * gfx->data.height * gfx->data.bpp + 7) >> 3);
  jsvUnLock2(jsvAddNamedChild(gfx->graphicsVar, buf, "buffer"), buf);
}

void lcdSetCallbacks_ArrayBuffer(JsGraphics *gfx) {
  gfx->setPixel = lcdSetPixel_ArrayBuffer;
  gfx->getPixel = lcdGetPixel_ArrayBuffer;
  gfx->fillRect = lcdFillRect_ArrayBuffer;
}
