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
  JsVar *buf = (JsVar*)gfx->backendData;
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
  return col;
}

// set pixelCount pixels starting at x,y
void lcdSetPixels_ArrayBuffer(JsGraphics *gfx, short x, short y, short pixelCount, unsigned int col) {
  JsVar *buf = (JsVar*)gfx->backendData;
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

void lcdSetPixel_ArrayBuffer(JsGraphics *gfx, short x, short y, unsigned int col) {
  lcdSetPixels_ArrayBuffer(gfx, x, y, 1, col);
}

void  lcdFillRect_ArrayBuffer(struct JsGraphics *gfx, short x1, short y1, short x2, short y2) {
  short y;
  for (y=y1;y<=y2;y++)
    lcdSetPixels_ArrayBuffer(gfx, x1, y, (short)(1+x2-x1), gfx->data.fgColor);
}

#ifndef SAVE_ON_FLASH
// Faster implementation for where we have a flat memory area
unsigned int lcdGetPixel_ArrayBuffer_flat(JsGraphics *gfx, short x, short y) {
  unsigned int col = 0;
  unsigned char *ptr = (unsigned char*)gfx->backendData;
  unsigned int idx = lcdGetPixelIndex_ArrayBuffer(gfx,x,y,1);
  ptr += idx>>3;
  if (gfx->data.bpp&7/*not a multiple of one byte*/) {
    idx = idx & 7;
    unsigned int mask = (unsigned int)(1<<gfx->data.bpp)-1;
    unsigned int existing = (unsigned int)*ptr;
    unsigned int bitIdx = (gfx->data.flags & JSGRAPHICSFLAGS_ARRAYBUFFER_MSB) ? 8-(idx+gfx->data.bpp) : idx;
    col = ((existing>>bitIdx)&mask);
  } else {
    int i;
    for (i=0;i<gfx->data.bpp;i+=8) {
      col |= ((unsigned int)*ptr) << i;
      ptr++;
    }
  }
  return col;
}

// set pixelCount pixels starting at x,y
// Faster implementation for where we have a flat memory area
void lcdSetPixels_ArrayBuffer_flat(JsGraphics *gfx, short x, short y, short pixelCount, unsigned int col) {
  unsigned char *ptr = (unsigned char*)gfx->backendData;
  unsigned int idx = lcdGetPixelIndex_ArrayBuffer(gfx,x,y,pixelCount);
  ptr += idx>>3;

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
            *ptr = c;
            ptr++;
          }
          continue;
        }
      }
      unsigned int mask = (unsigned int)(1<<gfx->data.bpp)-1;
      unsigned int existing = (unsigned int)*ptr;
      unsigned int bitIdx = (gfx->data.flags & JSGRAPHICSFLAGS_ARRAYBUFFER_MSB) ? 8-(idx+gfx->data.bpp) : idx;
      assert(ptr>=gfx->backendData && ptr<((char*)gfx->backendData + graphicsGetMemoryRequired(gfx)));
      *ptr = (char)((existing&~(mask<<bitIdx)) | ((col&mask)<<bitIdx));
      if (gfx->data.flags & JSGRAPHICSFLAGS_ARRAYBUFFER_VERTICAL_BYTE) {
        ptr++;
      } else {
        idx += gfx->data.bpp;
        if (idx>=8) ptr++;
      }
    } else { // we're writing whole bytes
      int i;
      for (i=0;i<gfx->data.bpp;i+=8) {
        *ptr = (char)(col >> i);
        ptr++;
      }
    }
  }
}

// Faster implementation for where we have a flat memory area
void lcdSetPixel_ArrayBuffer_flat(JsGraphics *gfx, short x, short y, unsigned int col) {
  lcdSetPixels_ArrayBuffer_flat(gfx, x, y, 1, col);
}

// Faster implementation for where we have a flat memory area
void  lcdFillRect_ArrayBuffer_flat(struct JsGraphics *gfx, short x1, short y1, short x2, short y2) {
  short y;
  for (y=y1;y<=y2;y++)
    lcdSetPixels_ArrayBuffer_flat(gfx, x1, y, (short)(1+x2-x1), gfx->data.fgColor);
}
#endif // SAVE_ON_FLASH

void lcdInit_ArrayBuffer(JsGraphics *gfx) {
  // create buffer
  JsVar *buf = jswrap_arraybuffer_constructor(graphicsGetMemoryRequired(gfx));
  jsvUnLock2(jsvAddNamedChild(gfx->graphicsVar, buf, "buffer"), buf);
}

void lcdSetCallbacks_ArrayBuffer(JsGraphics *gfx) {
  JsVar *buf = jsvObjectGetChild(gfx->graphicsVar, "buffer", 0);
#ifndef SAVE_ON_FLASH
  size_t len = 0;
  char *dataPtr = jsvGetDataPointer(buf, &len);
#endif
  jsvUnLock(buf);
#ifndef SAVE_ON_FLASH
  if (dataPtr && len>=graphicsGetMemoryRequired(gfx)) {
    // nice fast mode
    gfx->backendData = dataPtr;
    gfx->setPixel = lcdSetPixel_ArrayBuffer_flat;
    gfx->getPixel = lcdGetPixel_ArrayBuffer_flat;
    gfx->fillRect = lcdFillRect_ArrayBuffer_flat;
#else
  if (false) {
#endif
  } else if (jsvIsArrayBuffer(buf)) {
    /* NOTE: This is nasty as 'buf' is not locked. HOWEVER we know that
     gfx->graphicsVar IS locked, so 'buf' isn't going anywhere */
    gfx->backendData = buf;
    gfx->setPixel = lcdSetPixel_ArrayBuffer;
    gfx->getPixel = lcdGetPixel_ArrayBuffer;
    gfx->fillRect = lcdFillRect_ArrayBuffer;
  }
}
