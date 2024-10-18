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
#include "jswrap_graphics.h"
#include "lcd_arraybuffer.h"
#include "jsvar.h"
#include "jsvariterator.h"
#include "jsinteractive.h"

#ifndef SAVE_ON_FLASH
#ifndef ESPRUINOBOARD
// If an arraybuffer is flat, swap to faster arraybuffer ops
#define GRAPHICS_ARRAYBUFFER_OPTIMISATIONS
#endif
#endif

// returns the BIT index, so the bottom 3 bits specify the bit in the byte
unsigned int lcdGetPixelIndex_ArrayBuffer(JsGraphics *gfx, int x, int y, int pixelCount) {
  if (gfx->data.flags & JSGRAPHICSFLAGS_ARRAYBUFFER_ZIGZAG) {
    if (y&1) x = gfx->data.width - (x+pixelCount);
  }
  if (gfx->data.flags & JSGRAPHICSFLAGS_ARRAYBUFFER_INTERLEAVEX) {
    int h = gfx->data.height>>1;
    unsigned int idx = 0;
    if (y >= h) {
      y-=h;
      idx=gfx->data.bpp;
    }
    return idx + (unsigned int)((x + y*gfx->data.width)*(gfx->data.bpp<<1));
  }
  if (gfx->data.flags & JSGRAPHICSFLAGS_ARRAYBUFFER_VERTICAL_BYTE)
    return (unsigned int)(((x + (y>>3)*gfx->data.width)<<3) | (y&7));
  else
    return (unsigned int)((x + y*gfx->data.width)*gfx->data.bpp);
}

unsigned int lcdGetPixel_ArrayBuffer(JsGraphics *gfx, int x, int y) {
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
    if (gfx->data.flags & JSGRAPHICSFLAGS_ARRAYBUFFER_MSB) {
      for (int i=gfx->data.bpp-8;i>=0;i-=8) {
        col |= ((unsigned int)jsvArrayBufferIteratorGetIntegerValue(&it)) << i;
        jsvArrayBufferIteratorNext(&it);
      }
    } else {
      for (int i=0;i<gfx->data.bpp;i+=8) {
        col |= ((unsigned int)jsvArrayBufferIteratorGetIntegerValue(&it)) << i;
        jsvArrayBufferIteratorNext(&it);
      }
    }
  }
  jsvArrayBufferIteratorFree(&it);
  return col;
}

// set pixelCount pixels starting at x,y
void lcdSetPixels_ArrayBuffer(JsGraphics *gfx, int x, int y, int pixelCount, unsigned int col) {
  JsVar *buf = (JsVar*)gfx->backendData;
  unsigned int idx = lcdGetPixelIndex_ArrayBuffer(gfx,x,y,pixelCount);
  JsvArrayBufferIterator it;
  jsvArrayBufferIteratorNew(&it, buf, idx>>3 );

  unsigned int whiteMask = (1U<<gfx->data.bpp)-1;
  bool shortCut = (col==0 || (col&whiteMask)==whiteMask) && (!(gfx->data.flags&JSGRAPHICSFLAGS_ARRAYBUFFER_VERTICAL_BYTE)); // simple black or white fill
  int bppStride = gfx->data.bpp;
  if (gfx->data.flags&JSGRAPHICSFLAGS_ARRAYBUFFER_INTERLEAVEX) {
    bppStride <<= 1;
    shortCut = false;
  }

  while (pixelCount--) { // writing individual bits
    if (gfx->data.bpp&7/*not a multiple of one byte*/) {
      idx = idx & 7;
      if (shortCut && idx==0) {
        // Basically, if we're aligned and we're filling all 0 or all 1
        // then we can go really quickly and can just fill
        int wholeBytes = (gfx->data.bpp*(pixelCount+1)) >> 3;
        if (wholeBytes) {
          char c = (char)(col?0xFF:0);
          pixelCount = pixelCount+1 - (wholeBytes*8/gfx->data.bpp);
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
        idx += (unsigned)bppStride;
        if (idx>=8) jsvArrayBufferIteratorNext(&it);
      }
    } else { // we're writing whole bytes
      if (gfx->data.flags & JSGRAPHICSFLAGS_ARRAYBUFFER_MSB) {
        for (int i=gfx->data.bpp-8;i>=0;i-=8) {
          jsvArrayBufferIteratorSetByteValue(&it, (char)(col >> i));
          jsvArrayBufferIteratorNext(&it);
        }
      } else {
        for (int i=0;i<gfx->data.bpp;i+=8) {
          jsvArrayBufferIteratorSetByteValue(&it, (char)(col >> i));
          jsvArrayBufferIteratorNext(&it);
        }
      }
    }
  }
  jsvArrayBufferIteratorFree(&it);
}

void lcdSetPixel_ArrayBuffer(JsGraphics *gfx, int x, int y, unsigned int col) {
  lcdSetPixels_ArrayBuffer(gfx, x, y, 1, col);
}

void  lcdFillRect_ArrayBuffer(struct JsGraphics *gfx, int x1, int y1, int x2, int y2, unsigned int col) {
  int y;
  for (y=y1;y<=y2;y++)
    lcdSetPixels_ArrayBuffer(gfx, x1, y, 1+x2-x1, col);
}

#ifdef GRAPHICS_ARRAYBUFFER_OPTIMISATIONS
// Faster implementation for where we have a flat memory area
unsigned int lcdGetPixel_ArrayBuffer_flat(JsGraphics *gfx, int x, int y) {
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
    if (gfx->data.flags & JSGRAPHICSFLAGS_ARRAYBUFFER_MSB) {
      for (int i=gfx->data.bpp-8;i>=0;i-=8) {
        col |= ((unsigned int)*ptr) << i;
        ptr++;
      }
    } else {
      for (int i=0;i<gfx->data.bpp;i+=8) {
        col |= ((unsigned int)*ptr) << i;
        ptr++;
      }
    }
  }
  return col;
}

// set pixelCount pixels starting at x,y
// Faster implementation for where we have a flat memory area
void lcdSetPixels_ArrayBuffer_flat(JsGraphics *gfx, int x, int y, int pixelCount, unsigned int col) {
  unsigned char *ptr = (unsigned char*)gfx->backendData;
  unsigned int idx = lcdGetPixelIndex_ArrayBuffer(gfx,x,y,pixelCount);
  ptr += idx>>3;

  unsigned int whiteMask = (1U<<gfx->data.bpp)-1;
  bool shortCut = (col==0 || (col&whiteMask)==whiteMask) && (!(gfx->data.flags&JSGRAPHICSFLAGS_ARRAYBUFFER_VERTICAL_BYTE)); // simple black or white fill
  int bppStride = gfx->data.bpp;
  if (gfx->data.flags&JSGRAPHICSFLAGS_ARRAYBUFFER_INTERLEAVEX) {
    bppStride <<= 1;
    shortCut = false;
  }

  while (pixelCount--) { // writing individual bits
    if (gfx->data.bpp&7/*not a multiple of one byte*/) {
      idx = idx & 7;
      if (shortCut && idx==0) {
        // Basically, if we're aligned and we're filling all 0 or all 1
        // then we can go really quickly and can just fill
        int wholeBytes = (gfx->data.bpp*(pixelCount+1)) >> 3;
        if (wholeBytes) {
          unsigned char c = (unsigned char)(col?0xFF:0);
          pixelCount = pixelCount+1 - (wholeBytes*8/gfx->data.bpp);
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
      assert(ptr>=(unsigned char*)gfx->backendData && ptr<((unsigned char*)gfx->backendData + graphicsGetMemoryRequired(gfx)));
      *ptr = (unsigned char)((existing&~(mask<<bitIdx)) | ((col&mask)<<bitIdx));
      if (gfx->data.flags & JSGRAPHICSFLAGS_ARRAYBUFFER_VERTICAL_BYTE) {
        ptr++;
      } else {
        idx += (unsigned)bppStride;
        if (idx>=8) ptr++;
      }
    } else { // we're writing whole bytes
      if (gfx->data.flags & JSGRAPHICSFLAGS_ARRAYBUFFER_MSB) {
        for (int i=gfx->data.bpp-8;i>=0;i-=8)
          *(ptr++) = (unsigned char)(col >> i);
      } else {
        for (int i=0;i<gfx->data.bpp;i+=8)
          *(ptr++) = (unsigned char)(col >> i);
      }
    }
  }
}

// Faster implementation for where we have a flat memory area
void lcdSetPixel_ArrayBuffer_flat(JsGraphics *gfx, int x, int y, unsigned int col) {
  lcdSetPixels_ArrayBuffer_flat(gfx, x, y, 1, col);
}

// Faster implementation for where we have a flat memory area
void  lcdFillRect_ArrayBuffer_flat(struct JsGraphics *gfx, int x1, int y1, int x2, int y2, unsigned int col) {
  int y;
  for (y=y1;y<=y2;y++)
    lcdSetPixels_ArrayBuffer_flat(gfx, x1, y, 1+x2-x1, col);
}

void lcdScroll_ArrayBuffer_flat(JsGraphics *gfx, int xdir, int ydir, int x1, int y1, int x2, int y2) {
  // try and scroll quicker
  if (x1==0 && x2==(gfx->data.width-1) && xdir==0 && !(gfx->data.flags & JSGRAPHICSFLAGS_NONLINEAR)) { // can only do full width because we use memmove
    // TODO: for some cases we could cope with scrolling in X too (xdir!=0)
    int ylen = (y2+1-y1) - abs(ydir);
    int pixelCount = (x2+1-x1) * ylen;
    int ysrc = y1 + ((ydir<0) ? -ydir : 0);
    int ydst = y1 + ((ydir>0) ? ydir : 0);
    unsigned int idxsrc = lcdGetPixelIndex_ArrayBuffer(gfx,x1,ysrc,pixelCount);
    unsigned int idxdst = lcdGetPixelIndex_ArrayBuffer(gfx,x1,ydst,pixelCount);
    unsigned int bitCount = (unsigned)(pixelCount * gfx->data.bpp);

    if ((idxsrc&7)==0 && (idxdst&7)==0 && (bitCount&7)==0) { // if all aligned
      unsigned char *ptr = (unsigned char*)gfx->backendData;
      memmove(&ptr[idxdst>>3], &ptr[idxsrc>>3], bitCount>>3);
      return;
    }
  }
  // if we can't, fallback to slow scrolling
  return graphicsFallbackScroll(gfx, xdir, ydir, x1, y1, x2, y2);
}

#ifdef GRAPHICS_FAST_PATHS
// 1 bit
void lcdSetPixel_ArrayBuffer_flat1(JsGraphics *gfx, int x, int y, unsigned int col) {
  int p = x + y*gfx->data.width;
  uint8_t *byte = &((uint8_t*)gfx->backendData)[p>>3];
  if (col) *byte |= (uint8_t)(0x80 >> (p&7));
  else *byte &= (uint8_t)(0xFF7F >> (p&7));
}
unsigned int lcdGetPixel_ArrayBuffer_flat1(struct JsGraphics *gfx, int x, int y) {
  int p = x + y*gfx->data.width;
  uint8_t byte = ((uint8_t*)gfx->backendData)[p>>3];
  return (byte >> (7-(p&7))) & 1;
}
void lcdFillRect_ArrayBuffer_flat1(JsGraphics *gfx, int x1, int y1, int x2, int y2, unsigned int col) {
  if (x2-x1 < 8) return lcdFillRect_ArrayBuffer_flat(gfx,x1,y1,x2,y2,col); // not worth trying to work around this
  uint8_t *pixels = (uint8_t *)gfx->backendData;
  // build up 8 pixels in 1 byte for fast writes
  col &= 1;
  uint8_t colByte = (uint8_t)col | (uint8_t)(col<<1);
  colByte |= colByte<<2;
  colByte |= colByte<<4;
  // do each row separately
  for (int y=y1;y<=y2;y++) {
    int py = y*gfx->data.width;
    int p = x1 + py; // pixel index (not bit)
    int p2 = x2 + py; // pixel index (not bit)
    uint8_t *byte = &pixels[p>>3];
    // start off unaligned
    if (p&7) {
      int amt = (8-(p&7));
      int mask = ~(0xFF << amt);
      *byte = (*byte & ~mask) | (colByte & mask);
      byte++;
      p = (p&~7)+8;
    }
    // now we're aligned, just write bytes
    while (p+7<=p2) {
      *(byte++) = colByte;
      p+=8;
    }
    // finish off unaligned
    if (p<=p2) {
      int amt = 1+p2-p;
      int mask = ~(0xFF >> amt);
      *byte = (*byte & ~mask) | (colByte & mask);
    }
  }
}
// 2 bit
void lcdSetPixel_ArrayBuffer_flat2(JsGraphics *gfx, int x, int y, unsigned int col) {
  int p = (x + y*gfx->data.width); // pixel index (not bit)
  int b = (p&3) << 1; // bit
  uint8_t *byte = &((uint8_t*)gfx->backendData)[p>>2];
  *byte = (*byte & (0xFF3F>>b)) | ((col&3)<<(6-b));
}
unsigned int lcdGetPixel_ArrayBuffer_flat2(struct JsGraphics *gfx, int x, int y) {
  int p = x + y*gfx->data.width;  // pixel index (not bit)
  int b = (p&3) << 1; // bit
  uint8_t *byte = &((uint8_t*)gfx->backendData)[p>>2];
  return (*byte >> (6-b)) & 3;
}
void lcdFillRect_ArrayBuffer_flat2(JsGraphics *gfx, int x1, int y1, int x2, int y2, unsigned int col) {
  if (x2-x1 < 4) return lcdFillRect_ArrayBuffer_flat(gfx,x1,y1,x2,y2,col); // not worth trying to work around this
  uint8_t *pixels = (uint8_t *)gfx->backendData;
  // build up 4 pixels in 1 byte for fast writes
  col &= 3;
  uint8_t colByte = (uint8_t)col | (uint8_t)(col<<2);
  colByte |= colByte<<4;
  // do each row separately
  for (int y=y1;y<=y2;y++) {
    int py = y*gfx->data.width;
    int p = x1 + py; // pixel index (not bit)
    int p2 = x2 + py; // pixel index (not bit)
    uint8_t *byte = &pixels[p>>2];
    // start off unaligned
    if (p&3) {
      int amt = (4-(p&3));
      int mask = ~(0xFF << (amt<<1));
      *byte = (*byte & ~mask) | (colByte & mask);
      byte++;
      p = (p&~3)+4;
    }
    // now we're aligned, just write bytes
    while (p+3<=p2) {
      *(byte++) = colByte;
      p+=4;
    }
    // finish off unaligned
    if (p<=p2) {
      int amt = 1+p2-p;
      int mask = ~(0xFF >> (amt<<1));
      *byte = (*byte & ~mask) | (colByte & mask);
    }
  }
}
// 4 bit
void lcdSetPixel_ArrayBuffer_flat4(JsGraphics *gfx, int x, int y, unsigned int col) {
  int p = (x + y*gfx->data.width); // pixel index (not bit)
  int b = (p&1) << 2; // bit
  uint8_t *byte = &((uint8_t*)gfx->backendData)[p>>1];
  *byte = (*byte & (0xFF0F>>b)) | ((col&15)<<(4-b));
}
unsigned int lcdGetPixel_ArrayBuffer_flat4(struct JsGraphics *gfx, int x, int y) {
  int p = x + y*gfx->data.width;  // pixel index (not bit)
  int b = (p&1) << 2; // bit
  uint8_t *byte = &((uint8_t*)gfx->backendData)[p>>1];
  return (*byte >> (4-b)) & 15;
}
void lcdFillRect_ArrayBuffer_flat4(JsGraphics *gfx, int x1, int y1, int x2, int y2, unsigned int col) {
  if (x2-x1 < 2) return lcdFillRect_ArrayBuffer_flat(gfx,x1,y1,x2,y2,col); // not worth trying to work around this
  uint8_t *pixels = (uint8_t *)gfx->backendData;
  // build up 4 pixels in 1 byte for fast writes
  col &= 15;
  uint8_t colByte = (uint8_t)col | (uint8_t)(col<<4);
  // do each row separately
  for (int y=y1;y<=y2;y++) {
    int py = y*gfx->data.width;
    int p = x1 + py; // pixel index (not bit)
    int p2 = x2 + py; // pixel index (not bit)
    uint8_t *byte = &pixels[p>>1];
    // start off unaligned
    if (p&1) {
      *byte = (*byte & 0xF0) | col;
      byte++;
      p++;
    }
    // now we're aligned, just write bytes
    while (p+1<=p2) {
      *(byte++) = colByte;
      p+=2;
    }
    // finish off unaligned
    if (p<=p2) {
      *byte = (*byte & 0x0F) | (col<<4);
    }
  }
}
// 8 bit
void lcdSetPixel_ArrayBuffer_flat8(JsGraphics *gfx, int x, int y, unsigned int col) {
  ((uint8_t*)gfx->backendData)[x + y*gfx->data.width] = (uint8_t)col;
}
unsigned int lcdGetPixel_ArrayBuffer_flat8(struct JsGraphics *gfx, int x, int y) {
  return ((uint8_t*)gfx->backendData)[x + y*gfx->data.width];
}
void lcdFillRect_ArrayBuffer_flat8(JsGraphics *gfx, int x1, int y1, int x2, int y2, unsigned int col) {
  for (int y=y1;y<=y2;y++) {
    uint8_t *p = &((uint8_t*)gfx->backendData)[x1 + y*gfx->data.width];
    for (int x=x1;x<=x2;x++)
      *(p++) = (uint8_t)col;
  }
}

void lcdScroll_ArrayBuffer_flat8(JsGraphics *gfx, int xdir, int ydir, int x1, int y1, int x2, int y2) {
  int clipWidth = x2 - x1;
  int clipHeight = y2 - y1;
  int pixels = -(xdir + ydir*clipWidth);
  int startPixel = gfx->data.width * (y1 - ydir) + x1;
  int row;
  for (row=0; row<(clipHeight+ydir); row++) {
    if (pixels<0) {
      memcpy(&((uint8_t*)gfx->backendData)[startPixel-pixels],&((uint8_t*)gfx->backendData)[startPixel],(size_t)(clipWidth+xdir));
    } else {
      memcpy(&((uint8_t*)gfx->backendData)[startPixel],&((uint8_t*)gfx->backendData)[startPixel+pixels],(size_t)(clipWidth-xdir));
    }
    startPixel += gfx->data.width;
  }
}
#endif

#endif // GRAPHICS_ARRAYBUFFER_OPTIMISATIONS



void lcdInit_ArrayBuffer(JsGraphics *gfx, JsVar *optionalBuffer) {
  // create buffer
  if (optionalBuffer) {
    jsvAddNamedChild(gfx->graphicsVar, optionalBuffer, "buffer");
  } else {
    JsVar *buf = jswrap_arraybuffer_constructor((int)graphicsGetMemoryRequired(gfx));
    jsvAddNamedChildAndUnLock(gfx->graphicsVar, buf, "buffer");
  }
}

void lcdSetCallbacks_ArrayBuffer(JsGraphics *gfx) {
  JsVar *buf = jsvObjectGetChildIfExists(gfx->graphicsVar, "buffer");
#ifdef GRAPHICS_ARRAYBUFFER_OPTIMISATIONS
  size_t len = 0;
  char *dataPtr = jsvGetDataPointer(buf, &len);
#endif
  jsvUnLock(buf);
#ifdef GRAPHICS_ARRAYBUFFER_OPTIMISATIONS
  if (dataPtr && len>=graphicsGetMemoryRequired(gfx) && !(gfx->data.flags & JSGRAPHICSFLAGS_ARRAYBUFFER_ZIGZAG)) {
    gfx->backendData = dataPtr;
#ifdef GRAPHICS_FAST_PATHS
    if (gfx->data.bpp==1 &&
        (gfx->data.flags & JSGRAPHICSFLAGS_ARRAYBUFFER_MSB) &&
        !(gfx->data.flags & JSGRAPHICSFLAGS_NONLINEAR)
        ) { // super fast path for 1 bit
      gfx->setPixel = lcdSetPixel_ArrayBuffer_flat1;
      gfx->getPixel = lcdGetPixel_ArrayBuffer_flat1;
      gfx->fillRect = lcdFillRect_ArrayBuffer_flat1;
    } else if (gfx->data.bpp==2 &&
        (gfx->data.flags & JSGRAPHICSFLAGS_ARRAYBUFFER_MSB) &&
        !(gfx->data.flags & JSGRAPHICSFLAGS_NONLINEAR)
        ) { // super fast path for 1 bit
      gfx->setPixel = lcdSetPixel_ArrayBuffer_flat2;
      gfx->getPixel = lcdGetPixel_ArrayBuffer_flat2;
      gfx->fillRect = lcdFillRect_ArrayBuffer_flat2;
    } else if (gfx->data.bpp==4 &&
        (gfx->data.flags & JSGRAPHICSFLAGS_ARRAYBUFFER_MSB) &&
        !(gfx->data.flags & JSGRAPHICSFLAGS_NONLINEAR)
        ) { // super fast path for 1 bit
      gfx->setPixel = lcdSetPixel_ArrayBuffer_flat4;
      gfx->getPixel = lcdGetPixel_ArrayBuffer_flat4;
      gfx->fillRect = lcdFillRect_ArrayBuffer_flat4;
    } else if (gfx->data.bpp==8 &&
               !(gfx->data.flags & JSGRAPHICSFLAGS_NONLINEAR)
        ) { // super fast path for 8 bits
      gfx->setPixel = lcdSetPixel_ArrayBuffer_flat8;
      gfx->getPixel = lcdGetPixel_ArrayBuffer_flat8;
      gfx->fillRect = lcdFillRect_ArrayBuffer_flat8;
      gfx->scroll = lcdScroll_ArrayBuffer_flat8;
    } else
#endif
    {
      // nice fast mode
      gfx->setPixel = lcdSetPixel_ArrayBuffer_flat;
      gfx->getPixel = lcdGetPixel_ArrayBuffer_flat;
      gfx->fillRect = lcdFillRect_ArrayBuffer_flat;
      gfx->scroll = lcdScroll_ArrayBuffer_flat;
    }
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

