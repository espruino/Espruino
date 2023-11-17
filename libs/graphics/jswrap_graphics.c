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
 * This file is designed to be parsed during the build process
 *
 * Contains JavaScript Graphics Draw Functions
 * ----------------------------------------------------------------------------
 */
#include "jswrap_graphics.h"
#include "jswrap_math.h" // for jswrap_math_cos/sin
#include "jsutils.h"
#include "jsinteractive.h"

#include "lcd_arraybuffer.h"
#include "lcd_js.h"
#ifdef USE_LCD_SDL
#include "lcd_sdl.h"
#endif
#ifdef USE_LCD_FSMC
#include "lcd_fsmc.h"
#endif
#ifdef USE_LCD_ST7789_8BIT
#include "lcd_st7789_8bit.h"
#endif

#include "jswrap_functions.h" // for asURL
#include "jswrap_object.h" // for getFonts
#ifdef DICKENS
#include "jsflash.h" // for saveScreenshot
#include "lcd_spilcd.h"
#define ESPR_LINE_FONTS
#endif

#include "bitmap_font_4x6.h"
#include "bitmap_font_6x8.h"
#include "vector_font.h"
#ifdef ESPR_PBF_FONTS
#include "pbf_font.h"
#endif
#ifdef ESPR_LINE_FONTS
#include "line_font.h"
#endif


#ifdef GRAPHICS_PALETTED_IMAGES
#if defined(ESPR_GRAPHICS_12BIT)
#define PALETTE_BPP 12
// 8 color RGB - [0,0,0,0,0,0,0,0].map((x,i)=>((i&4)?0xF00:0)|((i&2)?0xF0:0)|((i&1)?0xF:0))
const uint16_t PALETTE_3BIT[16] = { 0, 15, 240, 255, 3840, 3855, 4080, 4095 };
// 16 color 12 bit MAC OS palette
const uint16_t PALETTE_4BIT[16] = { 0x0,0x444,0x888,0xbbb,0x963,0x630,0x60,0xa0,0x9f,0xc,0x309,0xf09,0xd00,0xf60,0xff0,0xfff };
// 256 color 12 bit Web-safe palette
const uint16_t PALETTE_8BIT[256] = { 0x0,0x3,0x6,0x9,0xc,0xf,0x30,0x33,0x36,0x39,0x3c,0x3f,0x60,0x63,0x66,0x69,0x6c,0x6f,0x90,0x93,0x96,0x99,0x9c,0x9f,0xc0,0xc3,0xc6,0xc9,0xcc,0xcf,0xf0,0xf3,0xf6,0xf9,0xfc,0xff,0x300,0x303,0x306,0x309,0x30c,0x30f,0x330,0x333,0x336,0x339,0x33c,0x33f,0x360,0x363,0x366,0x369,0x36c,0x36f,0x390,0x393,0x396,0x399,0x39c,0x39f,0x3c0,0x3c3,0x3c6,0x3c9,0x3cc,0x3cf,0x3f0,0x3f3,0x3f6,0x3f9,0x3fc,0x3ff,0x600,0x603,0x606,0x609,0x60c,0x60f,0x630,0x633,0x636,0x639,0x63c,0x63f,0x660,0x663,0x666,0x669,0x66c,0x66f,0x690,0x693,0x696,0x699,0x69c,0x69f,0x6c0,0x6c3,0x6c6,0x6c9,0x6cc,0x6cf,0x6f0,0x6f3,0x6f6,0x6f9,0x6fc,0x6ff,0x900,0x903,0x906,0x909,0x90c,0x90f,0x930,0x933,0x936,0x939,0x93c,0x93f,0x960,0x963,0x966,0x969,0x96c,0x96f,0x990,0x993,0x996,0x999,0x99c,0x99f,0x9c0,0x9c3,0x9c6,0x9c9,0x9cc,0x9cf,0x9f0,0x9f3,0x9f6,0x9f9,0x9fc,0x9ff,0xc00,0xc03,0xc06,0xc09,0xc0c,0xc0f,0xc30,0xc33,0xc36,0xc39,0xc3c,0xc3f,0xc60,0xc63,0xc66,0xc69,0xc6c,0xc6f,0xc90,0xc93,0xc96,0xc99,0xc9c,0xc9f,0xcc0,0xcc3,0xcc6,0xcc9,0xccc,0xccf,0xcf0,0xcf3,0xcf6,0xcf9,0xcfc,0xcff,0xf00,0xf03,0xf06,0xf09,0xf0c,0xf0f,0xf30,0xf33,0xf36,0xf39,0xf3c,0xf3f,0xf60,0xf63,0xf66,0xf69,0xf6c,0xf6f,0xf90,0xf93,0xf96,0xf99,0xf9c,0xf9f,0xfc0,0xfc3,0xfc6,0xfc9,0xfcc,0xfcf,0xff0,0xff3,0xff6,0xff9,0xffc,0xfff,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xfff };
#else // default to 16 bit
#define PALETTE_BPP 16
// 8 color RGB - [0,0,0,0,0,0,0,0].map((x,i)=>((i&4)?0xF800:0)|((i&2)?0x7E0:0)|((i&1)?0x1F:0))
const uint16_t PALETTE_3BIT[16] = { 0, 31, 2016, 2047, 63488, 63519, 65504, 65535 };
// 16 color MAC OS palette
#if LCD_BPP==3 // for 3bpp LCDs we use a more saturated palette that maps better to the less vibrant LCD
const uint16_t PALETTE_4BIT[16] = { 0x0,0x4228,0x8c51,0xbdd7,0xcc40,0x8a20,0x320,0x7e0,0x7ff,0x1f,0x8811,0xf81f,0xf800,0xfc40,0xffe0,0xffff };
#else // otherwise we use the standard Mac pallete
const uint16_t PALETTE_4BIT[16] = { 0x0,0x4228,0x8c51,0xbdd7,0x9b26,0x6180,0x320,0x540,0x4df,0x19,0x3013,0xf813,0xd800,0xfb20,0xffe0,0xffff };
#endif
// 256 color 16 bit Web-safe palette
const uint16_t PALETTE_8BIT[256] = {
    0x0,0x6,0xc,0x13,0x19,0x1f,0x180,0x186,0x18c,0x193,0x199,0x19f,0x320,0x326,0x32c,0x333,0x339,0x33f,0x4c0,
    0x4c6,0x4cc,0x4d3,0x4d9,0x4df,0x660,0x666,0x66c,0x673,0x679,0x67f,0x7e0,0x7e6,0x7ec,0x7f3,0x7f9,0x7ff,
    0x3000,0x3006,0x300c,0x3013,0x3019,0x301f,0x3180,0x3186,0x318c,0x3193,0x3199,0x319f,0x3320,0x3326,0x332c,
    0x3333,0x3339,0x333f,0x34c0,0x34c6,0x34cc,0x34d3,0x34d9,0x34df,0x3660,0x3666,0x366c,0x3673,0x3679,0x367f,
    0x37e0,0x37e6,0x37ec,0x37f3,0x37f9,0x37ff,0x6000,0x6006,0x600c,0x6013,0x6019,0x601f,0x6180,0x6186,0x618c,
    0x6193,0x6199,0x619f,0x6320,0x6326,0x632c,0x6333,0x6339,0x633f,0x64c0,0x64c6,0x64cc,0x64d3,0x64d9,0x64df,
    0x6660,0x6666,0x666c,0x6673,0x6679,0x667f,0x67e0,0x67e6,0x67ec,0x67f3,0x67f9,0x67ff,0x9800,0x9806,0x980c,
    0x9813,0x9819,0x981f,0x9980,0x9986,0x998c,0x9993,0x9999,0x999f,0x9b20,0x9b26,0x9b2c,0x9b33,0x9b39,0x9b3f,
    0x9cc0,0x9cc6,0x9ccc,0x9cd3,0x9cd9,0x9cdf,0x9e60,0x9e66,0x9e6c,0x9e73,0x9e79,0x9e7f,0x9fe0,0x9fe6,0x9fec,
    0x9ff3,0x9ff9,0x9fff,0xc800,0xc806,0xc80c,0xc813,0xc819,0xc81f,0xc980,0xc986,0xc98c,0xc993,0xc999,0xc99f,
    0xcb20,0xcb26,0xcb2c,0xcb33,0xcb39,0xcb3f,0xccc0,0xccc6,0xcccc,0xccd3,0xccd9,0xccdf,0xce60,0xce66,0xce6c,
    0xce73,0xce79,0xce7f,0xcfe0,0xcfe6,0xcfec,0xcff3,0xcff9,0xcfff,0xf800,0xf806,0xf80c,0xf813,0xf819,0xf81f,
    0xf980,0xf986,0xf98c,0xf993,0xf999,0xf99f,0xfb20,0xfb26,0xfb2c,0xfb33,0xfb39,0xfb3f,0xfcc0,0xfcc6,0xfccc,
    0xfcd3,0xfcd9,0xfcdf,0xfe60,0xfe66,0xfe6c,0xfe73,0xfe79,0xfe7f,0xffe0,0xffe6,0xffec,0xfff3,0xfff9,0xffff,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xffff
 };
// convert 16 bit to 8 bit with
// (c=>[(c&0xF800)>>8,(c&0x7E0)>>3,(c&0x1F)<<3])(0x0661)
// 16 bit to hex with
// (c=>"#"+[(c&0xF800)>>8,(c&0x7E0)>>3,(c&0x1F)<<3].map(c=>((c+8)>>4).toString(16)).join(""))(0x0661)
#endif
// map Mac to web-safe palette. Must be uint16_t because drawImage uses uint16_t* for palette
const uint16_t PALETTE_4BIT_TO_8BIT[16] = { 0, 43, 129, 172, 121, 78, 12, 18, 23, 4, 39, 183, 144, 192, 210, 215 };
// g2=Graphics.createArrayBuffer(8,8,8);print((new Uint8Array(8)).map((n,i)=>g2.toColor((i&4)>>2,(i&2)>>1,i&1)).join(","));
// map Mac to web-safe palette. Must be uint16_t because drawImage uses uint16_t* for palette
const uint16_t PALETTE_3BIT_TO_8BIT[8] = { 0,5,30,35,180,185,210,215 };
// g2=Graphics.createArrayBuffer(8,8,4);print((new Uint8Array(8)).map((n,i)=>g2.toColor((i&4)>>2,(i&2)>>1,i&1)).join(","));
// map Mac to web-safe palette. Must be uint16_t because drawImage uses uint16_t* for palette
const uint16_t PALETTE_3BIT_TO_4BIT[8] = { 0,9,7,8,12,11,14,15 };
#endif


// ==========================================================================================

void _jswrap_graphics_freeImageInfo(GfxDrawImageInfo *info) {
  jsvUnLock(info->buffer);
}

/** Parse an image into GfxDrawImageInfo. See drawImage for image format docs. Returns true on success.
 * if 'image' is a string or ArrayBuffer, imageOffset is the offset within that (usually 0)
 */
bool _jswrap_graphics_parseImage(JsGraphics *gfx, JsVar *image, unsigned int imageOffset, GfxDrawImageInfo *info) {
  memset(info, 0, sizeof(GfxDrawImageInfo));
  if (jsvIsObject(image)) {
#ifndef SAVE_ON_FLASH
    if (jsvIsInstanceOf(image,"Graphics")) { // handle a Graphics instance
      JsGraphics ig;
      if (!graphicsGetFromVar(&ig, image)) return false;
      if (ig.data.type!=JSGRAPHICSTYPE_ARRAYBUFFER) return false; // if not arraybuffer Graphics, bail out
      info->width = ig.data.width;
      info->height = ig.data.height;
      info->bpp = ig.data.bpp;
    } else
#endif
    { // Normal image object
      info->width = (int)jsvGetIntegerAndUnLock(jsvObjectGetChildIfExists(image, "width"));
      info->height = (int)jsvGetIntegerAndUnLock(jsvObjectGetChildIfExists(image, "height"));
      info->bpp = (int)jsvGetIntegerAndUnLock(jsvObjectGetChildIfExists(image, "bpp"));
      if (info->bpp<=0) info->bpp=1;
    }
    // Get the buffer for image data - this is the same in Graphics and Image objects
    JsVar *buf = jsvObjectGetChildIfExists(image, "buffer");
    info->buffer = jsvGetArrayBufferBackingString(buf, &info->bitmapOffset);
    jsvUnLock(buf);
    info->bitmapOffset += imageOffset;

    JsVar *v;
    v = jsvObjectGetChildIfExists(image, "transparent");
    info->isTransparent = v!=0;
    info->transparentCol = (unsigned int)jsvGetIntegerAndUnLock(v);
#ifndef SAVE_ON_FLASH_EXTREME
    v = jsvObjectGetChildIfExists(image, "palette");
    if (v) {
      if (jsvIsArrayBuffer(v) && v->varData.arraybuffer.type==ARRAYBUFFERVIEW_UINT16) {
        size_t l = 0;
        info->palettePtr = (uint16_t *)jsvGetDataPointer(v, &l);
        jsvUnLock(v);
        if (l==2 || l==4 || l==8 || l==16 || l==256) {
          info->paletteMask = (uint32_t)(l-1);
        } else {
          info->palettePtr = 0;
        }
      } else
        jsvUnLock(v);
      if (!info->palettePtr) {
        jsExceptionHere(JSET_ERROR, "Palette specified, but must be a flat Uint16Array of 2,4,8,16,256 elements");
        return false;
      }
    }
#endif

  } else if (jsvIsString(image) || jsvIsArrayBuffer(image)) {
    if (jsvIsArrayBuffer(image)) {
      info->buffer = jsvGetArrayBufferBackingString(image, &info->bitmapOffset);
    } else {
      info->buffer = jsvLockAgain(image);
    }
    JsvStringIterator it;
    jsvStringIteratorNewUTF8(&it, info->buffer, imageOffset);
    info->width = jsvStringIteratorGetUTF8CharAndNext(&it);
    info->height = jsvStringIteratorGetUTF8CharAndNext(&it);
    info->bpp = jsvStringIteratorGetUTF8CharAndNext(&it);
    info->bitmapOffset += imageOffset;
    if (info->bpp & 128) {
      info->bpp = info->bpp&127;
      info->isTransparent = true;
      info->transparentCol = jsvStringIteratorGetUTF8CharAndNext(&it);
      info->headerLength = 4;
    } else {
      info->headerLength = 3;
    }
    info->bitmapOffset += info->headerLength;
    if (info->bpp & 64) { // included palette data
      info->bpp = info->bpp&63;
#ifndef SAVE_ON_FLASH_EXTREME
      if (info->bpp > 8) {
        jsExceptionHere(JSET_ERROR, "Can't have palette on >8 bit images");
        _jswrap_graphics_freeImageInfo(info);
        jsvStringIteratorFree(&it);
        return false;
      }
      unsigned int paletteEntries = 1<<info->bpp;
      info->paletteMask = (uint32_t)paletteEntries-1;
      if (paletteEntries*2 <= (int)sizeof(info->_simplePalette)) {
        // if it'll fit, put the palette data in _simplePalette
        uint32_t n = info->bitmapOffset;
        for (unsigned int i=0;i<paletteEntries;i++) {
          info->_simplePalette[i] =
            ((unsigned char)jsvStringIteratorGetUTF8CharAndNext(&it)) |
            ((unsigned char)jsvStringIteratorGetUTF8CharAndNext(&it))<<8;
          n+=2;
        }
        info->palettePtr = info->_simplePalette;
      } else if (info->bpp<=8) { // otherwise if data is memory-mapped, use direct
        unsigned int imgStart = info->bitmapOffset + paletteEntries*2;
        size_t dataLen = 0;
        char *dataPtr = jsvGetDataPointer(info->buffer, &dataLen);
        if (info->bpp<=8 && dataPtr && imgStart<dataLen) {
          info->paletteMask = (uint32_t)(paletteEntries-1);
          info->palettePtr = (uint16_t*)&dataPtr[info->bitmapOffset+info->headerLength];
        }
      }
      // could allocate a flat string and copy data in here
      if (!info->palettePtr) {
        jsExceptionHere(JSET_ERROR, "Unable to get pointer to palette. Image in flash?");
        _jswrap_graphics_freeImageInfo(info);
        jsvStringIteratorFree(&it);
        return false;
      }
      // modify image start
      info->headerLength += (unsigned short)(paletteEntries*2);
      info->bitmapOffset += (unsigned short)(paletteEntries*2);
#else
      jsExceptionHere(JSET_ERROR, "Image Palette not supported on this build");
#endif
    }
    jsvStringIteratorFree(&it);
  } else {
    jsExceptionHere(JSET_ERROR, "First argument must be Object or String");
    return 0;
  }
  if (!info->isTransparent)
    info->transparentCol = 0xFFFFFFFF;

  if (info->palettePtr==0) {
    if (info->bpp==1) {
      info->_simplePalette[0] = (uint16_t)gfx->data.bgColor;
      info->_simplePalette[1] = (uint16_t)gfx->data.fgColor;
      info->palettePtr = info->_simplePalette;
      info->paletteMask = 1;
  #ifdef GRAPHICS_PALETTED_IMAGES
    } else if (info->bpp==2) { // Blend from bg to fg
      info->_simplePalette[0] = (uint16_t)gfx->data.bgColor;
      info->_simplePalette[1] = (uint16_t)graphicsBlendGfxColor(gfx, 85);
      info->_simplePalette[2] = (uint16_t)graphicsBlendGfxColor(gfx, 171);
      info->_simplePalette[3] = (uint16_t)gfx->data.fgColor;
      info->palettePtr = info->_simplePalette;
      info->paletteMask = 3;
    } else if (gfx->data.bpp==PALETTE_BPP && info->bpp==3) { // palette is 16 bits, so don't use it for other things
      info->palettePtr = PALETTE_3BIT;
      info->paletteMask = 7;
    } else if (gfx->data.bpp==PALETTE_BPP && info->bpp==4) { // palette is 16 bits, so don't use it for other things
      info->palettePtr = PALETTE_4BIT;
      info->paletteMask = 15;
    } else if (gfx->data.bpp==PALETTE_BPP && info->bpp==8) { // palette is 16 bits, so don't use it for other things
      info->palettePtr = PALETTE_8BIT;
      info->paletteMask = 255;
    } else if (gfx->data.bpp==8 && info->bpp==4) {
      info->palettePtr = PALETTE_4BIT_TO_8BIT;
      info->paletteMask = 15;
    } else if (gfx->data.bpp==8 && info->bpp==3) {
      info->palettePtr = PALETTE_3BIT_TO_8BIT;
      info->paletteMask = 7;
    } else if (gfx->data.bpp==4 && info->bpp==3) {
      info->palettePtr = PALETTE_3BIT_TO_4BIT;
      info->paletteMask = 7;
  #endif
    }
  }

  if ((!jsvIsString(info->buffer)) ||
      info->width<=0 ||
      info->height<=0 ||
      info->bpp>32) {
    jsExceptionHere(JSET_ERROR, "Expecting valid Image");
    _jswrap_graphics_freeImageInfo(info);
    return false;
  }
  info->bitMask = (unsigned int)((1L<<info->bpp)-1L);
  info->pixelsPerByteMask = (unsigned int)((info->bpp<8)?(8/info->bpp)-1:0);
  info->stride = (info->width*info->bpp + 7)>>3;
  info->bitmapLength = (unsigned short)((info->width*info->height*info->bpp + 7)>>3);
  return true;
}

bool _jswrap_drawImageLayerGetPixel(GfxDrawImageLayer *l, unsigned int *result) {
  int qx = l->qx+127;
  int qy = l->qy+127;
  if (qx>=0 && qy>=0 && qx<l->mx && qy<l->my) {
    unsigned int colData = 0;
    int imagex = qx>>8;
    int imagey = qy>>8;
   // TODO: getter callback for speed?
#ifndef SAVE_ON_FLASH
   if (l->img.bpp==8) { // fast path for 8 bits
     jsvStringIteratorGoto(&l->it, l->img.buffer, (size_t)((int)l->img.bitmapOffset+imagex+(imagey*l->img.stride)));
     colData = (unsigned char)jsvStringIteratorGetChar(&l->it);
   } else
#endif
   {
     int pixelOffset = (imagex+(imagey*l->img.width));
     int bitOffset = pixelOffset*l->img.bpp;
     jsvStringIteratorGoto(&l->it, l->img.buffer, (size_t)((int)l->img.bitmapOffset+(bitOffset>>3)));
     bitOffset &= 7; // so now it's bits within a byte
     // get first byte
     colData = (unsigned char)jsvStringIteratorGetChar(&l->it);
     // it may not fit within a byte, so if so grab more data
     int b;
     for (b=8-(bitOffset+l->img.bpp);b<0;b+=8) {
       jsvStringIteratorNext(&l->it);
       colData = (colData<<8) | (unsigned char)jsvStringIteratorGetChar(&l->it);
     }
     // finally shift down to the right size
     colData = (colData>>b) & l->img.bitMask;
   }
   if (l->img.transparentCol!=colData) {
     if (l->img.palettePtr) colData = l->img.palettePtr[colData&l->img.paletteMask];
     *result = colData;
     return true;
   }
  }
  return false;
}
NO_INLINE void _jswrap_drawImageLayerInit(GfxDrawImageLayer *l) {
  // image max
  l->mx = l->img.width<<8;
  l->my = l->img.height<<8;
  // step values for blitting rotated image
  double vcos = jswrap_math_cos(l->rotate);
  double vsin = jswrap_math_sin(l->rotate);
  l->sx = (int)((vcos/l->scale)*256 + 0.5);
  l->sy = (int)((vsin/l->scale)*256 + 0.5);
  // work out actual image width and height
  int iw = (int)(0.5 + l->scale*(l->img.width*fabs(vcos) + l->img.height*fabs(vsin)));
  int ih = (int)(0.5 + l->scale*(l->img.width*fabs(vsin) + l->img.height*fabs(vcos)));
  // if rotating, offset our start position from center
  if (l->center) {
    l->x1 -= iw/2;
    l->y1 -= ih/2;
  }
  l->x2 = l->x1 + iw;
  l->y2 = l->y1 + ih;
  // work out start position in the image
  int centerx = l->img.width*128;
  int centery = l->img.height*128;
  l->px = centerx - (1 + (l->sx*iw) + (l->sy*ih)) / 2;
  l->py = centery - (1 + (l->sx*ih) - (l->sy*iw)) / 2;
  // handle repetition
  if (l->repeat) {
    // for the range we're in, it's quicker/easier than modulo
    while (l->px < 0) l->px += l->mx;
    while (l->px >= l->mx) l->px -= l->mx;
    while (l->py < 0) l->py += l->my;
    while (l->py >= l->my) l->py -= l->my;
  }
}
NO_INLINE void _jswrap_drawImageLayerSetStart(GfxDrawImageLayer *l, int x, int y) {
  int dx = x - l->x1;
  int dy = y - l->y1;
  l->px += l->sx*dx + l->sy*dy;
  l->py += l->sx*dy - l->sy*dx;
}
NO_INLINE void _jswrap_drawImageLayerStartX(GfxDrawImageLayer *l) {
  l->qx = l->px;
  l->qy = l->py;
}
ALWAYS_INLINE void _jswrap_drawImageLayerNextX(GfxDrawImageLayer *l) {
  l->qx += l->sx;
  l->qy -= l->sy;
}
// Handle repeats
ALWAYS_INLINE void _jswrap_drawImageLayerNextXRepeat(GfxDrawImageLayer *l) {
  if (l->repeat) {
    // for the range we're in, it's quicker/easier than modulo
    if (l->qx < 0) l->qx += l->mx;
    if (l->qx >= l->mx) l->qx -= l->mx;
    if (l->qy < 0) l->qy += l->my;
    if (l->qy >= l->my) l->qy -= l->my;
  }
}
NO_INLINE void _jswrap_drawImageLayerNextY(GfxDrawImageLayer *l) {
  l->px += l->sy;
  l->py += l->sx;
  if (l->repeat) {
    // for the range we're in, it's quicker/easier than modulo
    if (l->px < 0) l->px += l->mx;
    if (l->px >= l->mx) l->px -= l->mx;
    if (l->py < 0) l->py += l->my;
    if (l->py >= l->my) l->py -= l->my;
  }
}

/* Draw an image 1:1 at xPos,yPos. If parseFullImage=true we ensure
we leave the StringIterator pointing right at the end of the image. If not
we can optimise if the image is clipped/offscreen. */
NO_INLINE void _jswrap_drawImageSimple(JsGraphics *gfx, int xPos, int yPos, GfxDrawImageInfo *img, JsvStringIterator *it, bool parseFullImage) {
  int bits=0;
  uint32_t colData=0;
  int x1 = xPos, y1 = yPos, x2 = xPos+img->width-1, y2 = yPos+img->height-1;
  if (!jsvStringIteratorHasChar(it)) return; // no data
#ifndef SAVE_ON_FLASH
  graphicsSetModifiedAndClip(gfx,&x1,&y1,&x2,&y2); // ensure we clip Y
  /* force a skip forward as many bytes as we need. Ideally we would use
  jsvStringIteratorGotoUTF8 but we don't have the UTF8 index or
  source string here. This is still better than trying to render every pixel! */
  if (y2<y1 || x2<x1) { // offscreen - skip everything and exit
    if (parseFullImage) {
      bits = -img->bpp*img->width*img->height;
      while (bits < 0) {
        jsvStringIteratorNextUTF8(it);
        bits += 8;
      }
    }
    return;
  } else // onscreen. y1!=yPos if clipped - ensure we skip enough bytes
    bits = -(y1-yPos)*img->bpp*img->width;
#endif
  JsGraphicsSetPixelFn setPixel = graphicsGetSetPixelUnclippedFn(gfx, xPos, y1, xPos+img->width-1, y2);
  for (int y=y1;y<=y2;y++) {
    for (int x=xPos;x<xPos+img->width;x++) {
      // Get the data we need...
      while (bits < img->bpp) {
        colData = (colData<<8) | ((unsigned char)jsvStringIteratorGetUTF8CharAndNext(it));
        bits += 8;
      }
      // extract just the bits we want
      unsigned int col = (colData>>(bits-img->bpp))&img->bitMask;
      bits -= img->bpp;
      // Try and write pixel!
      if (img->transparentCol!=col) {
        if (img->palettePtr) col = img->palettePtr[col&img->paletteMask];
        setPixel(gfx, x, y, col);
      }
    }
  }
#ifndef SAVE_ON_FLASH
  if (parseFullImage) {
    /* If we didn't render the last bit of the image, and the caller needs
    the StringIterator to point to the end of the image, skip forward */
    bits += img->bpp-(yPos+img->height-(1+y2))*img->bpp*img->width;
    while (bits < 0) {
      jsvStringIteratorNextUTF8(it);
      bits += 8;
    }
  }
#endif
}

// ==========================================================================================


/*JSON{
  "type" : "class",
  "class" : "Graphics",
  "typescript" : "Graphics<IsBuffer extends boolean = boolean>"
}
This class provides Graphics operations that can be applied to a surface.

Use Graphics.createXXX to create a graphics object that renders in the way you
want. See [the Graphics page](https://www.espruino.com/Graphics) for more
information.

**Note:** On boards that contain an LCD, there is a built-in `g` object of
type `Graphics`. For instance to draw a line you'd type:
```g.drawLine(0,0,100,100)```
*/

/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "params" : [ ["all","bool","[optional] (only on some devices) If `true` then copy all pixels, not just those that have changed."] ],
  "name" : "flip"
}
On instances of graphics that drive a display with an offscreen buffer, calling
this function will copy the contents of the offscreen buffer to the screen.

Call this when you have drawn something to Graphics and you want it shown on the
screen.

If a display does not have an offscreen buffer, it may not have a `g.flip()`
method.

On Bangle.js 1, there are different graphics modes chosen with
`Bangle.setLCDMode()`. The default mode is unbuffered and in this mode
`g.flip()` does not affect the screen contents.

On some devices, this command will attempt to only update the areas of the
screen that have changed in order to increase speed. If you have accessed the
`Graphics.buffer` directly then you may need to use `Graphics.flip(true)` to
force a full update of the screen.
*/
/*JSON{
  "type" : "property",
  "class" : "Graphics",
  "name" : "buffer",
  "return" : ["JsVar","An ArrayBuffer (or not defined on Graphics instances not created with `Graphics.createArrayBuffer`)"],
  "typescript" : "buffer: IsBuffer extends true ? ArrayBuffer : undefined"
}
On Graphics instances with an offscreen buffer, this is an `ArrayBuffer` that
provides access to the underlying pixel data.

```
g=Graphics.createArrayBuffer(8,8,8)
g.drawLine(0,0,7,7)
print(new Uint8Array(g.buffer))
new Uint8Array([
255, 0, 0, 0, 0, 0, 0, 0,
0, 255, 0, 0, 0, 0, 0, 0,
0, 0, 255, 0, 0, 0, 0, 0,
0, 0, 0, 255, 0, 0, 0, 0,
0, 0, 0, 0, 255, 0, 0, 0,
0, 0, 0, 0, 0, 255, 0, 0,
0, 0, 0, 0, 0, 0, 255, 0,
0, 0, 0, 0, 0, 0, 0, 255])
```
*/

/*JSON{
  "type" : "idle",
  "generate" : "jswrap_graphics_idle"
}*/
bool jswrap_graphics_idle() {
  graphicsIdle();
  return false;
}

/*JSON{
  "type" : "init",
  "generate" : "jswrap_graphics_init",
  "sortorder" : -100
}*/
void jswrap_graphics_init() {
  // sortorder is first because we don't want subsequent
  // _init to a) not have GFX and b) not get their theme
  // settings overwritten
#ifdef USE_LCD_FSMC
  JsVar *parent = jspNewObject("g", "Graphics");
  if (parent) {
    JsVar *parentObj = jsvSkipName(parent);
    jsvObjectSetChild(execInfo.hiddenRoot, JS_GRAPHICS_VAR, parentObj);
    JsGraphics gfx;
    gfx.data.type = JSGRAPHICSTYPE_FSMC;
    graphicsStructInit(&gfx,320,240,16);
    gfx.graphicsVar = parentObj;
    lcdInit_FSMC(&gfx);
    lcdSetCallbacks_FSMC(&gfx);
    graphicsSplash(&gfx);
    graphicsSetVarInitial(&gfx);
    jsvUnLock2(parentObj, parent);
  }
#endif
#ifdef GRAPHICS_THEME
  /// Global color scheme colours
  graphicsTheme.fg = (JsGraphicsThemeColor)-1;
  graphicsTheme.bg = (JsGraphicsThemeColor)0;
  graphicsTheme.fg2 = (JsGraphicsThemeColor)-1;
  graphicsTheme.bg2 = (JsGraphicsThemeColor)0;
  graphicsTheme.fgH = (JsGraphicsThemeColor)-1;
  graphicsTheme.bgH = (JsGraphicsThemeColor)0;
  graphicsTheme.dark = true;
#endif
}

/*JSON{
  "type" : "staticmethod",
  "class" : "Graphics",
  "name" : "getInstance",
  "generate" : "jswrap_graphics_getInstance",
  "return" : ["JsVar","An instance of `Graphics` or undefined"],
  "typescript" : "getInstance(): Graphics | undefined"
}
On devices like Pixl.js or HYSTM boards that contain a built-in display this
will return an instance of the graphics class that can be used to access that
display.

Internally, this is stored as a member called `gfx` inside the 'hiddenRoot'.
*/
JsVar *jswrap_graphics_getInstance() {
  return jsvObjectGetChildIfExists(execInfo.hiddenRoot, JS_GRAPHICS_VAR);
}

static bool isValidBPP(int bpp) {
  return bpp==1 || bpp==2 || bpp==4 || bpp==8 || bpp==16 || bpp==24 || bpp==32; // currently one colour can't ever be spread across multiple bytes
}

/*JSON{
  "type" : "staticmethod",
  "class" : "Graphics",
  "name" : "createArrayBuffer",
  "generate" : "jswrap_graphics_createArrayBuffer",
  "params" : [
    ["width","int32","Pixels wide"],
    ["height","int32","Pixels high"],
    ["bpp","int32","Number of bits per pixel"],
    ["options","JsVar",[
      "An object of other options. `{ zigzag : true/false(default), vertical_byte : true/false(default), msb : true/false(default), color_order: 'rgb'(default),'bgr',etc }`",
      "`zigzag` = whether to alternate the direction of scanlines for rows",
      "`vertical_byte` = whether to align bits in a byte vertically or not",
      "`msb` = when bits<8, store pixels most significant bit first, when bits>8, store most significant byte first",
      "`interleavex` = Pixels 0,2,4,etc are from the top half of the image, 1,3,5,etc from the bottom half. Used for P3 LED panels.",
      "`color_order` = re-orders the colour values that are supplied via setColor"
    ]]
  ],
  "return" : ["JsVar","The new Graphics object"],
  "return_object" : "Graphics",
  "typescript" : "createArrayBuffer(width: number, height: number, bpp: number, options?: { zigzag?: boolean, vertical_byte?: boolean, msb?: boolean, color_order?: \"rgb\" | \"rbg\" | \"brg\" | \"bgr\" | \"grb\" | \"gbr\" }): Graphics<true>;"
}
Create a Graphics object that renders to an Array Buffer. This will have a field
called 'buffer' that can get used to get at the buffer itself
*/
JsVar *jswrap_graphics_createArrayBuffer(int width, int height, int bpp, JsVar *options) {
  if (width<=0 || height<=0 || width>32767 || height>32767) {
    jsExceptionHere(JSET_ERROR, "Invalid Size");
    return 0;
  }
  if (!isValidBPP(bpp)) {
    jsExceptionHere(JSET_ERROR, "Invalid BPP");
    return 0;
  }

  JsVar *parent = jspNewObject(0, "Graphics");
  if (!parent) return 0; // low memory

  JsGraphics gfx;
  gfx.data.type = JSGRAPHICSTYPE_ARRAYBUFFER;
  graphicsStructInit(&gfx,width,height,bpp);
  gfx.data.flags = JSGRAPHICSFLAGS_NONE;
  gfx.graphicsVar = parent;

  if (jsvIsObject(options)) {
    if (jsvGetBoolAndUnLock(jsvObjectGetChildIfExists(options, "zigzag")))
      gfx.data.flags = (JsGraphicsFlags)(gfx.data.flags | JSGRAPHICSFLAGS_ARRAYBUFFER_ZIGZAG);
    if (jsvGetBoolAndUnLock(jsvObjectGetChildIfExists(options, "msb")))
      gfx.data.flags = (JsGraphicsFlags)(gfx.data.flags | JSGRAPHICSFLAGS_ARRAYBUFFER_MSB);
    if (jsvGetBoolAndUnLock(jsvObjectGetChildIfExists(options, "interleavex")))
      gfx.data.flags = (JsGraphicsFlags)(gfx.data.flags | JSGRAPHICSFLAGS_ARRAYBUFFER_INTERLEAVEX);
    if (jsvGetBoolAndUnLock(jsvObjectGetChildIfExists(options, "vertical_byte"))) {
      if (gfx.data.bpp==1)
        gfx.data.flags = (JsGraphicsFlags)(gfx.data.flags | JSGRAPHICSFLAGS_ARRAYBUFFER_VERTICAL_BYTE);
      else {
        jsExceptionHere(JSET_ERROR, "vertical_byte only works for 1bpp ArrayBuffers");
        return 0;
      }
      if (gfx.data.height&7) {
        jsExceptionHere(JSET_ERROR, "height must be a multiple of 8 when using vertical_byte");
        return 0;
      }
    }
    JsVar *colorv = jsvObjectGetChildIfExists(options, "color_order");
    if (colorv) {
      if (jsvIsStringEqual(colorv, "rgb")) ; // The default
      else if (!jsvIsStringEqual(colorv, "brg"))
        gfx.data.flags = (JsGraphicsFlags)(gfx.data.flags | JSGRAPHICSFLAGS_COLOR_BRG);
      else if (!jsvIsStringEqual(colorv, "bgr"))
        gfx.data.flags = (JsGraphicsFlags)(gfx.data.flags | JSGRAPHICSFLAGS_COLOR_BGR);
      else if (!jsvIsStringEqual(colorv, "gbr"))
        gfx.data.flags = (JsGraphicsFlags)(gfx.data.flags | JSGRAPHICSFLAGS_COLOR_GBR);
      else if (!jsvIsStringEqual(colorv, "grb"))
        gfx.data.flags = (JsGraphicsFlags)(gfx.data.flags | JSGRAPHICSFLAGS_COLOR_GRB);
      else if (!jsvIsStringEqual(colorv, "rbg"))
        gfx.data.flags = (JsGraphicsFlags)(gfx.data.flags | JSGRAPHICSFLAGS_COLOR_RBG);
      else
        jsWarn("color_order must be 3 characters");
      jsvUnLock(colorv);
    }
  }

  lcdInit_ArrayBuffer(&gfx);
  graphicsSetVarInitial(&gfx);
  return parent;
}

/*JSON{
  "type" : "staticmethod",
  "class" : "Graphics",
  "name" : "createCallback",
  "ifndef" : "SAVE_ON_FLASH",
  "generate" : "jswrap_graphics_createCallback",
  "params" : [
    ["width","int32","Pixels wide"],
    ["height","int32","Pixels high"],
    ["bpp","int32","Number of bits per pixel"],
    ["callback","JsVar","A function of the form ```function(x,y,col)``` that is called whenever a pixel needs to be drawn, or an object with: ```{setPixel:function(x,y,col),fillRect:function(x1,y1,x2,y2,col)}```. All arguments are already bounds checked."]
  ],
  "return" : ["JsVar","The new Graphics object"],
  "return_object" : "Graphics",
  "typescript" : "createCallback(width: number, height: number, bpp: number, callback: ((x: number, y: number, col: number) => void) | { setPixel: (x: number, y: number, col: number) => void; fillRect: (x1: number, y1: number, x2: number, y2: number, col: number) => void }): Graphics<false>;"
}
Create a Graphics object that renders by calling a JavaScript callback function
to draw pixels
*/
JsVar *jswrap_graphics_createCallback(int width, int height, int bpp, JsVar *callback) {
  if (width<=0 || height<=0 || width>32767 || height>32767) {
    jsExceptionHere(JSET_ERROR, "Invalid Size");
    return 0;
  }
  if (!isValidBPP(bpp)) {
    jsExceptionHere(JSET_ERROR, "Invalid BPP");
    return 0;
  }
  JsVar *callbackSetPixel = 0;
  JsVar *callbackFillRect = 0;
  if (jsvIsObject(callback)) {
    jsvUnLock(callbackSetPixel);
    callbackSetPixel = jsvObjectGetChildIfExists(callback, "setPixel");
    callbackFillRect = jsvObjectGetChildIfExists(callback, "fillRect");
  } else
    callbackSetPixel = jsvLockAgain(callback);
  if (!jsvIsFunction(callbackSetPixel)) {
    jsExceptionHere(JSET_ERROR, "Expecting Callback Function or an Object but got %t", callbackSetPixel);
    jsvUnLock2(callbackSetPixel, callbackFillRect);
    return 0;
  }
  if (!jsvIsUndefined(callbackFillRect) && !jsvIsFunction(callbackFillRect)) {
    jsExceptionHere(JSET_ERROR, "Expecting Callback Function or an Object but got %t", callbackFillRect);
    jsvUnLock2(callbackSetPixel, callbackFillRect);
    return 0;
  }

  JsVar *parent = jspNewObject(0, "Graphics");
  if (!parent) return 0; // low memory

  JsGraphics gfx;
  gfx.data.type = JSGRAPHICSTYPE_JS;
  graphicsStructInit(&gfx,width,height,bpp);
  gfx.graphicsVar = parent;
  lcdInit_JS(&gfx, callbackSetPixel, callbackFillRect);
  graphicsSetVarInitial(&gfx);
  jsvUnLock2(callbackSetPixel, callbackFillRect);
  return parent;
}

#ifdef USE_LCD_SDL
/*JSON{
  "type" : "staticmethod",
  "class" : "Graphics",
  "name" : "createSDL",
  "ifdef" : "USE_LCD_SDL",
  "generate" : "jswrap_graphics_createSDL",
  "params" : [
    ["width","int32","Pixels wide"],
    ["height","int32","Pixels high"],
    ["bpp","int32","Bits per pixel (8,16,24 or 32 supported)"]
  ],
  "return" : ["JsVar","The new Graphics object"],
  "return_object" : "Graphics"
}
Create a Graphics object that renders to SDL window (Linux-based devices only)
*/
JsVar *jswrap_graphics_createSDL(int width, int height, int bpp) {
  if (width<=0 || height<=0 || width>32767 || height>32767) {
    jsExceptionHere(JSET_ERROR, "Invalid Size");
    return 0;
  }

  JsVar *parent = jspNewObject(0, "Graphics");
  if (!parent) return 0; // low memory
  JsGraphics gfx;
  gfx.data.type = JSGRAPHICSTYPE_SDL;
  graphicsStructInit(&gfx,width,height,bpp);
  gfx.graphicsVar = parent;
  lcdInit_SDL(&gfx);
  graphicsSetVarInitial(&gfx);
  return parent;
}
#endif


/*TYPESCRIPT
type ImageObject = {
  width: number;
  height: number;
  bpp?: number;
  buffer: ArrayBuffer | string;
  transparent?: number;
  palette?: Uint16Array;
};

type Image = string | ImageObject | ArrayBuffer | Graphics<true>;
*/
/*JSON{
  "type" : "staticmethod",
  "class" : "Graphics",
  "name" : "createImage",
  "#if" : "!defined(SAVE_ON_FLASH) && !defined(ESPRUINOBOARD)",
  "generate" : "jswrap_graphics_createImage",
  "params" : [
    ["str","JsVar","A String containing a newline-separated image - space/. is 0, anything else is 1"]
  ],
  "return" : ["JsVar","An Image object that can be used with `Graphics.drawImage`"],
  "typescript" : "createImage(str: string): ImageObject;"
}
Create a simple Black and White image for use with `Graphics.drawImage`.

Use as follows:

```
var img = Graphics.createImage(`
XXXXXXXXX
X       X
X   X   X
X   X   X
X       X
XXXXXXXXX
`);
g.drawImage(img, x,y);
var img = Graphics.createImage(`
.....
.XXX.
.X.X.
.XXX.
.....
`);
g.drawImage(img, x,y);
```

If the characters at the beginning and end of the string are newlines, they will
be ignored. Spaces are treated as `0`, and any other character is a `1`
*/
JsVar *jswrap_graphics_createImage(JsVar *data) {
  if (!jsvIsString(data)) {
    jsExceptionHere(JSET_TYPEERROR, "Expecting String");
    return 0;
  }
  int x=0,y=0;
  int width=0, height=0;
  size_t startCharacter = 0;
  JsvStringIterator it;
  // First iterate and work out width and height
  jsvStringIteratorNew(&it,data,0);
  char ch;
  while (jsvStringIteratorHasChar(&it)) {
    ch = jsvStringIteratorGetCharAndNext(&it);
    if (ch=='\n') {
      if (x==0 && y==0 && !startCharacter) {
        startCharacter = 1; // ignore first character
      } else {
        x=0;
        y++;
        if (y+1>height) height=y+1;
      }
    } else {
      x++;
      if (x>width) width=x;
    }
  }
  if (height && ch=='\n') height--; // if the last char was a newline, ignore it
  jsvStringIteratorFree(&it);
  // Sorted - now create the object, set it up and create the buffer
  JsVar *img = jsvNewObject();
  if (!img) return 0;
  jsvObjectSetChildAndUnLock(img,"width",jsvNewFromInteger(width));
  jsvObjectSetChildAndUnLock(img,"height",jsvNewFromInteger(height));
  // bpp is 1, no need to set it
  int len = (width*height+7)>>3;
  JsVar *buffer = jsvNewStringOfLength((unsigned)len, NULL);
  if (!buffer) { // not enough memory
    jsvUnLock(img);
    return 0;
  }
  // Now set the characters!
  x=0;
  y=0;
  jsvStringIteratorNew(&it,data,startCharacter);
  while (jsvStringIteratorHasChar(&it)) {
    char ch = jsvStringIteratorGetCharAndNext(&it);
    if (ch=='\n') {
      x=0;
      y++;
    } else {
      if (ch!=' ' && ch!='.') {
        /* a pixel to set. This'll be slowish for non-flat strings,
         * but this is here for relatively small bitmaps
         * anyway so it's not a big deal */
        size_t idx = (size_t)(y*width + x);
        jsvSetCharInString(buffer, idx>>3, (char)(128>>(idx&7)), true/*OR*/);
      }
      x++;
    }
  }
  jsvStringIteratorFree(&it);
  jsvObjectSetChildAndUnLock(img, "buffer", buffer);
  return img;
}

/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "getWidth",
  "generate_full" : "jswrap_graphics_getWidthOrHeight(parent, false)",
  "return" : ["int","The width of this Graphics instance"]
}
The width of this Graphics instance
*/
/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "getHeight",
  "generate_full" : "jswrap_graphics_getWidthOrHeight(parent, true)",
  "return" : ["int","The height of this Graphics instance"]
}
The height of this Graphics instance
*/
int jswrap_graphics_getWidthOrHeight(JsVar *parent, bool height) {
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return 0;
  return height ? graphicsGetHeight(&gfx) : graphicsGetWidth(&gfx);
}
/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "getBPP",
  "generate" : "jswrap_graphics_getBPP",
  "return" : ["int","The bits per pixel of this Graphics instance"]
}
The number of bits per pixel of this Graphics instance

**Note:** Bangle.js 2 behaves a little differently here. The display is 3 bit,
so `getBPP` returns 3 and `asBMP`/`asImage`/etc return 3 bit images. However in
order to allow dithering, the colors returned by `Graphics.getColor` and
`Graphics.theme` are actually 16 bits.
*/
int jswrap_graphics_getBPP(JsVar *parent) {
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return 0;
#ifdef LCD_CONTROLLER_LPM013M126
  if (gfx.data.type==JSGRAPHICSTYPE_MEMLCD)
    return 3;
#endif
  return gfx.data.bpp;
}

/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "reset",
  "generate" : "jswrap_graphics_reset",
  "return" : ["JsVar","The instance of Graphics this was called on, to allow call chaining"],
  "return_object" : "Graphics"
}
Reset the state of Graphics to the defaults (e.g. Color, Font, etc) that would
have been used when Graphics was initialised.
*/
JsVar *jswrap_graphics_reset(JsVar *parent) {
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return 0;
#ifndef SAVE_ON_FLASH
  // If we had a custom font, remove it.
  jsvUnLock(jswrap_graphics_setFontSizeX(parent, 1+JSGRAPHICS_FONTSIZE_4X6, false));
#endif
  // properly reset state
  graphicsStructResetState(&gfx);
  graphicsSetVar(&gfx); // gfx data changed because modified area
  // reset font, which will unreference any custom fonts stored inside the instance
  return jswrap_graphics_setFontSizeX(parent, 1+JSGRAPHICS_FONTSIZE_4X6, false);
}

/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "clear",
  "generate" : "jswrap_graphics_clear",
  "params" : [
    ["reset","bool","[optional] If `true`, resets the state of Graphics to the default (eg. Color, Font, etc) as if calling `Graphics.reset`"]
  ],
  "return" : ["JsVar","The instance of Graphics this was called on, to allow call chaining"],
  "return_object" : "Graphics"
}
Clear the LCD with the Background Color
*/
JsVar *jswrap_graphics_clear(JsVar *parent, bool resetState) {
  if (resetState) jsvUnLock(jswrap_graphics_reset(parent));
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return 0;
  graphicsClear(&gfx);
  graphicsSetVar(&gfx); // gfx data changed because modified area
  return jsvLockAgain(parent);
}

void _jswrap_graphics_getRect(JsVar *opt, int *x1, int *y1, int *x2, int *y2, int *r) {
  *r = 0;
  if (jsvIsObject(opt)) {
    int w = -1,h = -1;
    jsvConfigObject configs[] = {
        {"x", JSV_INTEGER, x1},
        {"y", JSV_INTEGER, y1},
        {"x1", JSV_INTEGER, x1},
        {"y1", JSV_INTEGER, y1},
        {"x2", JSV_INTEGER, x2},
        {"y2", JSV_INTEGER, y2},
        {"w", JSV_INTEGER, &w},
        {"h", JSV_INTEGER, &h},
        {"r", JSV_INTEGER, r}
    };
    jsvReadConfigObject(opt, configs, sizeof(configs) / sizeof(jsvConfigObject));
    if (w>=0) *x2 = *x1 + w;
    if (h>=0) *y2 = *y1 + h;
  } else
    *x1 = jsvGetInteger(opt);
}

JsVar *_jswrap_graphics_fillRect_col(JsVar *parent, JsVar *opt, int y1, int x2, int y2, bool isFgCol) {
  int x1, r;
  _jswrap_graphics_getRect(opt, &x1, &y1, &x2, &y2, &r);
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return 0;
  uint32_t col = isFgCol ? gfx.data.fgColor : gfx.data.bgColor;
#ifndef SAVE_ON_FLASH
  if (r>0) {
    // rounded rects
    graphicsToDeviceCoordinates(&gfx, &x1, &y1);
    graphicsToDeviceCoordinates(&gfx, &x2, &y2);
    int x,y;
    if (x1 > x2) { x = x1; x1 = x2; x2 = x; }
    if (y1 > y2) { y = y1; y1 = y2; y2 = y; }
    // clip if radius too big
    x = (x2-x1)/2;
    y = (y2-y1)/2;
    if (x<r) r=x;
    if (y<r) r=y;
    // rect in middle
    int cx1 = x1+r, cx2 = x2-r;
    int cy1 = y1+r, cy2 = y2-r;
    graphicsFillRectDevice(&gfx, x1, cy1, x2, cy2, col);
    // draw rounded top and bottom
    int dx = 0;
    int dy = r;
    int r2 = r*r;
    int err = r2-(2*r-1)*r2;
    int e2;
    bool changed = false;
    do {
      changed = false;
      e2 = 2*err;
      if (e2 <  (2*dx+1)*r2) { dx++; err += (2*dx+1)*r2; changed=true; }
      if (e2 > -(2*dy-1)*r2) {
        // draw only just before we change Y, to avoid a bunch of overdraw
        graphicsFillRectDevice(&gfx, cx1-dx,cy2+dy, cx2+dx,cy2+dy, col);
        graphicsFillRectDevice(&gfx, cx1-dx,cy1-dy, cx2+dx,cy1-dy, col);
        dy--; err -= (2*dy-1)*r2; changed=true;
      }
    } while (changed && dy >= 0);
  } else
#endif
  graphicsFillRect(&gfx, x1,y1,x2,y2,col);
  graphicsSetVar(&gfx); // gfx data changed because modified area
  return jsvLockAgain(parent);
}

/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "fillRect",
  "generate" : "jswrap_graphics_fillRect",
  "params" : [
    ["x1","JsVar","The left X coordinate OR an object containing `{x,y,x2,y2}` or `{x,y,w,h}`"],
    ["y1","int32","The top Y coordinate"],
    ["x2","int32","The right X coordinate"],
    ["y2","int32","The bottom Y coordinate"]
  ],
  "return" : ["JsVar","The instance of Graphics this was called on, to allow call chaining"],
  "return_object" : "Graphics",
  "typescript" : [
    "fillRect(x1: number, y1: number, x2: number, y2: number): Graphics;",
    "fillRect(rect: { x: number, y: number, x2: number, y2: number } | { x: number, y: number, w: number, h: number }): Graphics;"
  ]
}
Fill a rectangular area in the Foreground Color

On devices with enough memory, you can specify `{x,y,x2,y2,r}` as the first
argument, which allows you to draw a rounded rectangle.
*/
JsVar *jswrap_graphics_fillRect(JsVar *parent, JsVar *opt, int y1, int x2, int y2) {
  return _jswrap_graphics_fillRect_col(parent,opt,y1,x2,y2,true);
}



/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "clearRect",
  "ifndef" : "SAVE_ON_FLASH",
  "generate" : "jswrap_graphics_clearRect",
  "params" : [
    ["x1","JsVar","The left X coordinate OR an object containing `{x,y,x2,y2}` or `{x,y,w,h}`"],
    ["y1","int32","The top Y coordinate"],
    ["x2","int32","The right X coordinate"],
    ["y2","int32","The bottom Y coordinate"]
  ],
  "return" : ["JsVar","The instance of Graphics this was called on, to allow call chaining"],
  "return_object" : "Graphics",
  "typescript" : [
    "clearRect(x1: number, y1: number, x2: number, y2: number): Graphics;",
    "clearRect(rect: { x: number, y: number, x2: number, y2: number } | { x: number, y: number, w: number, h: number }): Graphics;"
  ]
}
Fill a rectangular area in the Background Color

On devices with enough memory, you can specify `{x,y,x2,y2,r}` as the first
argument, which allows you to draw a rounded rectangle.
*/
JsVar *jswrap_graphics_clearRect(JsVar *parent, JsVar *opt, int y1, int x2, int y2) {
  return _jswrap_graphics_fillRect_col(parent,opt,y1,x2,y2,false);
}

/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "drawRect",
  "generate" : "jswrap_graphics_drawRect",
  "params" : [
    ["x1","JsVar","The left X coordinate OR an object containing `{x,y,x2,y2}` or `{x,y,w,h}`"],
    ["y1","int32","The top Y coordinate"],
    ["x2","int32","The right X coordinate"],
    ["y2","int32","The bottom Y coordinate"]
  ],
  "return" : ["JsVar","The instance of Graphics this was called on, to allow call chaining"],
  "return_object" : "Graphics",
  "typescript" : [
    "drawRect(x1: number, y1: number, x2: number, y2: number): Graphics;",
    "drawRect(rect: { x: number, y: number, x2: number, y2: number } | { x: number, y: number, w: number, h: number }): Graphics;"
  ]
}
Draw an unfilled rectangle 1px wide in the Foreground Color
*/
JsVar *jswrap_graphics_drawRect(JsVar *parent, JsVar *opt, int y1, int x2, int y2) {
  int x1, r;
  _jswrap_graphics_getRect(opt, &x1, &y1, &x2, &y2, &r);
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return 0;
  graphicsDrawRect(&gfx, x1,y1,x2,y2);
  graphicsSetVar(&gfx); // gfx data changed because modified area
  return jsvLockAgain(parent);
}

/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "fillCircle",
  "ifndef" : "SAVE_ON_FLASH",
  "generate" : "jswrap_graphics_fillCircle",
  "params" : [
    ["x","int32","The X axis"],
    ["y","int32","The Y axis"],
    ["rad","int32","The circle radius"]
  ],
  "return" : ["JsVar","The instance of Graphics this was called on, to allow call chaining"],
  "return_object" : "Graphics"
}
Draw a filled circle in the Foreground Color
*/
 JsVar *jswrap_graphics_fillCircle(JsVar *parent, int x, int y, int rad) {
   return jswrap_graphics_fillEllipse(parent, x-rad, y-rad, x+rad, y+rad);
 }

/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "fillAnnulus",
  "#if" : "defined(DICKENS)",
  "generate" : "jswrap_graphics_fillAnnulus",
  "params" : [
    ["x","int32","The X axis"],
    ["y","int32","The Y axis"],
    ["r1","int32","The annulus inner radius"],
    ["r2","int32","The annulus outer radius"]
  ],
  "return" : ["JsVar","The instance of Graphics this was called on, to allow call chaining"],
  "return_object" : "Graphics"
}
Draw a filled annulus in the Foreground Color
*/
// JsVar *jswrap_graphics_fillAnnulus(JsVar *parent, int x, int y, int r1, int r2, unsigned short quadrants) {  // Too many arguments!
 JsVar *jswrap_graphics_fillAnnulus(JsVar *parent, int x, int y, int r1, int r2) {
   JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return 0;
   unsigned short quadrants = 0x0F; // Just do all quadrants for now
   graphicsFillAnnulus(&gfx, x,y,r1,r2,quadrants);
   graphicsSetVar(&gfx); // gfx data changed because modified area
   return jsvLockAgain(parent);
 }

/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "drawCircle",
  "ifndef" : "SAVE_ON_FLASH",
  "generate" : "jswrap_graphics_drawCircle",
  "params" : [
    ["x","int32","The X axis"],
    ["y","int32","The Y axis"],
    ["rad","int32","The circle radius"]
  ],
  "return" : ["JsVar","The instance of Graphics this was called on, to allow call chaining"],
  "return_object" : "Graphics"
}
Draw an unfilled circle 1px wide in the Foreground Color
*/
JsVar *jswrap_graphics_drawCircle(JsVar *parent, int x, int y, int rad) {
  return jswrap_graphics_drawEllipse(parent, x-rad, y-rad, x+rad, y+rad);
}

/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "drawCircleAA",
  "ifdef" : "GRAPHICS_ANTIALIAS",
  "generate" : "jswrap_graphics_drawCircleAA",
  "params" : [
    ["x","int32","Centre x-coordinate"],
    ["y","int32","Centre y-coordinate"],
    ["r","int32","Radius"]
  ],
  "return" : ["JsVar","The instance of Graphics this was called on, to allow call chaining"],
  "return_object" : "Graphics"
}
Draw a circle, centred at (x,y) with radius r in the current foreground color
*/
#ifdef GRAPHICS_ANTIALIAS
JsVar *jswrap_graphics_drawCircleAA(JsVar *parent, int x, int y, int r) {
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return 0;
  graphicsDrawCircleAA(&gfx, x,y,r);
  graphicsSetVar(&gfx); // gfx data changed because modified area
  return jsvLockAgain(parent);
}
#endif


/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "fillEllipse",
  "ifndef" : "SAVE_ON_FLASH",
  "generate" : "jswrap_graphics_fillEllipse",
  "params" : [
    ["x1","int32","The left X coordinate"],
    ["y1","int32","The top Y coordinate"],
    ["x2","int32","The right X coordinate"],
    ["y2","int32","The bottom Y coordinate"]
  ],
  "return" : ["JsVar","The instance of Graphics this was called on, to allow call chaining"],
  "return_object" : "Graphics"
}
Draw a filled ellipse in the Foreground Color
*/
JsVar *jswrap_graphics_fillEllipse(JsVar *parent, int x, int y, int x2, int y2) {
   JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return 0;
   graphicsFillEllipse(&gfx, x,y,x2,y2);
   graphicsSetVar(&gfx); // gfx data changed because modified area
   return jsvLockAgain(parent);
 }

/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "drawEllipse",
  "ifndef" : "SAVE_ON_FLASH",
  "generate" : "jswrap_graphics_drawEllipse",
  "params" : [
    ["x1","int32","The left X coordinate"],
    ["y1","int32","The top Y coordinate"],
    ["x2","int32","The right X coordinate"],
    ["y2","int32","The bottom Y coordinate"]
  ],
  "return" : ["JsVar","The instance of Graphics this was called on, to allow call chaining"],
  "return_object" : "Graphics"
}
Draw an ellipse in the Foreground Color
*/
JsVar *jswrap_graphics_drawEllipse(JsVar *parent, int x, int y, int x2, int y2) {
   JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return 0;
   graphicsDrawEllipse(&gfx, x,y,x2,y2);
   graphicsSetVar(&gfx); // gfx data changed because modified area
   return jsvLockAgain(parent);
 }

 /*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "getPixel",
  "generate" : "jswrap_graphics_getPixel",
  "params" : [
    ["x","int32","The left"],
    ["y","int32","The top"]
  ],
  "return" : ["int32","The color"]
}
Get a pixel's color
*/
int jswrap_graphics_getPixel(JsVar *parent, int x, int y) {
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return 0;
  return (int)graphicsGetPixel(&gfx, x, y);
}

/*TYPESCRIPT
type ColorResolvable = number | `#${string}`;
*/
/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "setPixel",
  "generate" : "jswrap_graphics_setPixel",
  "params" : [
    ["x","int32","The left"],
    ["y","int32","The top"],
    ["col","JsVar","The color (if `undefined`, the foreground color is useD)"]
  ],
  "return" : ["JsVar","The instance of Graphics this was called on, to allow call chaining"],
  "return_object" : "Graphics",
  "typescript" : "setPixel(x: number, y: number, col?: ColorResolvable): Graphics;"
}
Set a pixel's color
*/
JsVar *jswrap_graphics_setPixel(JsVar *parent, int x, int y, JsVar *color) {
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return 0;
  unsigned int col = gfx.data.fgColor;
  if (!jsvIsUndefined(color))
    col = jswrap_graphics_toColor(parent,color,0,0);
  graphicsSetPixel(&gfx, x, y, col);
  gfx.data.cursorX = (short)x;
  gfx.data.cursorY = (short)y;
  graphicsSetVar(&gfx); // gfx data changed because modified area
  return jsvLockAgain(parent);
}

/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "toColor",
  "ifndef" : "SAVE_ON_FLASH",
  "generate" : "jswrap_graphics_toColor",
  "params" : [
    ["r","JsVar","Red (between 0 and 1) **OR** an integer representing the color in the current bit depth and color order **OR** a hexidecimal color string of the form `'#rrggbb' or `'#rgb'`"],
    ["g","JsVar","Green (between 0 and 1)"],
    ["b","JsVar","Blue (between 0 and 1)"]
  ],
  "return" : ["int","The color index represented by the arguments"],
  "typescript" : [
    "toColor(r: number, g: number, b: number): number;",
    "toColor(col: ColorResolvable): number;"
  ]
}
Work out the color value to be used in the current bit depth based on the arguments.

This is used internally by setColor and setBgColor

```
// 1 bit
g.toColor(1,1,1) => 1
// 16 bit
g.toColor(1,0,0) => 0xF800
```
*/

unsigned int jswrap_graphics_toColor(JsVar *parent, JsVar *r, JsVar *g, JsVar *b) {
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return 0;
  unsigned int color = 0;
#ifdef SAVE_ON_FLASH
  if (false) {
#else
  JsVarFloat rf, gf, bf;
  if (jsvIsString(r)) {
    char buf[9];
    memset(buf,0,sizeof(buf));
    jsvGetString(r,buf,sizeof(buf));
    if (buf[4]==0) {
      rf = chtod(buf[1])/15.0;
      gf = chtod(buf[2])/15.0;
      bf = chtod(buf[3])/15.0;
    } else {
      rf = hexToByte(buf[1],buf[2])/255.0;
      gf = hexToByte(buf[3],buf[4])/255.0;
      bf = hexToByte(buf[5],buf[6])/255.0;
    }
    if (rf<0 || gf<0 || bf<0 || buf[7]!=0) {
      jsExceptionHere(JSET_ERROR, "If Color is a String, it must be of the form '#rrggbb' or '#rgb'");
      return 0;
    }
  } else {
    rf = jsvGetFloat(r);
    gf = jsvGetFloat(g);
    bf = jsvGetFloat(b);
  }
  if (isfinite(rf) && isfinite(gf) && isfinite(bf)) {
    int ri = (int)(rf*256);
    int gi = (int)(gf*256);
    int bi = (int)(bf*256);
    if (ri>255) ri=255;
    if (gi>255) gi=255;
    if (bi>255) bi=255;
    if (ri<0) ri=0;
    if (gi<0) gi=0;
    if (bi<0) bi=0;
    // Check if we need to twiddle colors
    int colorMask = gfx.data.flags & JSGRAPHICSFLAGS_COLOR_MASK;
    if (colorMask) {
      int tmpr, tmpg, tmpb;
      tmpr = ri;
      tmpg = gi;
      tmpb = bi;
      switch (colorMask) {
        case JSGRAPHICSFLAGS_COLOR_BRG:
          ri = tmpb;
          gi = tmpr;
          bi = tmpg;
          break;
        case JSGRAPHICSFLAGS_COLOR_BGR:
          ri = tmpb;
          bi = tmpr;
          break;
        case JSGRAPHICSFLAGS_COLOR_GBR:
          ri = tmpg;
          gi = tmpb;
          bi = tmpr;
          break;
        case JSGRAPHICSFLAGS_COLOR_GRB:
          ri = tmpg;
          gi = tmpr;
          break;
        case JSGRAPHICSFLAGS_COLOR_RBG:
          gi = tmpb;
          bi = tmpg;
          break;
        default: break;
      }
    }
    if (gfx.data.bpp==16) {
      color = (unsigned int)((bi>>3) | (gi>>2)<<5 | (ri>>3)<<11);
#ifdef ESPR_GRAPHICS_12BIT
    } else if (gfx.data.bpp==12) {
      color = (unsigned int)((bi>>4) | (gi>>4)<<4 | (ri>>4)<<8);
#endif
    } else if (gfx.data.bpp==32) {
      color = 0xFF000000 | (unsigned int)(bi | (gi<<8) | (ri<<16));
    } else if (gfx.data.bpp==24) {
      color = (unsigned int)(bi | (gi<<8) | (ri<<16));
#if defined(GRAPHICS_PALETTED_IMAGES)
    } else if (gfx.data.bpp==4) {
      // LCD is paletted - look up in our palette to find the best match
      int d = 0x7FFFFFFF;
      color = 0;
      for (unsigned int i=0;i<16;i++) {
        int p = PALETTE_4BIT[i];
        int pr = (p>>8)&0xF8;
        int pg = (p>>3)&0xFC;
        int pb = (p<<3)&0xF8;
        pr |= pr>>5;
        pg |= pb>>6;
        pb |= pb>>5;
        int dr = pr-ri;
        int dg = pg-gi;
        int db = pb-bi;
        int dd = dr*dr+dg*dg+db*db;
        if (dd<d) {
          d=dd;
          color=i;
        }
      }
    } else if (gfx.data.bpp==8) {
      // LCD is paletted - look up in our palette to find the best match
      // TODO: For web palette we should be able to cheat without searching...
      int d = 0x7FFFFFFF;
      color = 0;
      for (int i=0;i<255;i++) {
        int p = PALETTE_8BIT[i];
        int pr = (p>>8)&0xF8;
        int pg = (p>>3)&0xFC;
        int pb = (p<<3)&0xF8;
        pr |= pr>>5;
        pg |= pb>>6;
        pb |= pb>>5;
        int dr = pr-ri;
        int dg = pg-gi;
        int db = pb-bi;
        int dd = dr*dr+dg*dg+db*db;
        if (dd<d) {
          d=dd;
          color=(unsigned int)i;
        }
      }
#endif
    } else
      color = (unsigned int)(((ri+gi+bi)>=384) ? 0xFFFFFFFF : 0);
#endif
  } else {
    // just rgb
    color = (unsigned int)jsvGetInteger(r);
  }
  return color;
}

/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "blendColor",
  "ifdef" : "GRAPHICS_ANTIALIAS",
  "generate" : "jswrap_graphics_blendColor",
  "params" : [
    ["col_a","JsVar","Color to blend from (either a single integer color value, or a string)"],
    ["col_b","JsVar","Color to blend to (either a single integer color value, or a string)"],
    ["amt","JsVar","The amount to blend. 0=col_a, 1=col_b, 0.5=halfway between (and so on)"]
  ],
  "return" : ["int","The color index represented by the blended colors"],
  "typescript" : "blendColor(col_a: ColorResolvable, col_b: ColorResolvable, amt: number): number;"
}
Blend between two colors, and return the result.

```
// dark yellow - halfway between red and green
var col = g.blendColor("#f00","#0f0", 0.5);
// Get a color 25% brighter than the theme's background colour
var col = g.blendColor(g.theme.fg,g.theme.bg, 0.75);
// then...
g.setColor(col).fillRect(10,10,100,100);
```
*/

unsigned int jswrap_graphics_blendColor(JsVar *parent, JsVar *ca, JsVar *cb, JsVar* amt) {
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return 0;
  unsigned int a = jswrap_graphics_toColor(parent, ca, NULL, NULL);
  unsigned int b = jswrap_graphics_toColor(parent, cb, NULL, NULL);
  return graphicsBlendColor(&gfx, b, a, (int)(jsvGetFloat(amt)*256));
}

/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "setColor",
  "generate_full" : "jswrap_graphics_setColorX(parent, r,g,b, true)",
  "params" : [
    ["r","JsVar","Red (between 0 and 1) **OR** an integer representing the color in the current bit depth and color order **OR** a hexidecimal color string of the form `'#012345'`"],
    ["g","JsVar","[optional] Green (between 0 and 1)"],
    ["b","JsVar","[optional] Blue (between 0 and 1)"]
  ],
  "return" : ["JsVar","The instance of Graphics this was called on, to allow call chaining"],
  "return_object" : "Graphics",
  "typescript" : [
    "setColor(r: number, g: number, b: number): Graphics;",
    "setColor(col: ColorResolvable): Graphics;"
  ]
}
Set the color to use for subsequent drawing operations.

If just `r` is specified as an integer, the numeric value will be written directly into a pixel. eg. On a 24 bit `Graphics` instance you set bright blue with either `g.setColor(0,0,1)` or `g.setColor(0x0000FF)`.

A good shortcut to ensure you get white on all platforms is to use `g.setColor(-1)`

The mapping is as follows:

* 32 bit: `r,g,b` => `0xFFrrggbb`
* 24 bit: `r,g,b` => `0xrrggbb`
* 16 bit: `r,g,b` => `0brrrrrggggggbbbbb` (RGB565)
* Other bpp: `r,g,b` => white if `r+g+b > 50%`, otherwise black (use `r` on its own as an integer)

If you specified `color_order` when creating the `Graphics` instance, `r`,`g` and `b` will be swapped as you specified.

**Note:** On devices with low flash memory, `r` **must** be an integer representing the color in the current bit depth. It cannot
be a floating point value, and `g` and `b` are ignored.
*/
/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "setBgColor",
  "generate_full" : "jswrap_graphics_setColorX(parent, r,g,b, false)",
  "params" : [
    ["r","JsVar","Red (between 0 and 1) **OR** an integer representing the color in the current bit depth and color order **OR** a hexidecimal color string of the form `'#012345'`"],
    ["g","JsVar","Green (between 0 and 1)"],
    ["b","JsVar","Blue (between 0 and 1)"]
  ],
  "return" : ["JsVar","The instance of Graphics this was called on, to allow call chaining"],
  "return_object" : "Graphics",
  "typescript" : [
    "setBgColor(r: number, g: number, b: number): Graphics;",
    "setBgColor(col: ColorResolvable): Graphics;"
  ]
}
Set the background color to use for subsequent drawing operations.

See `Graphics.setColor` for more information on the mapping of `r`, `g`, and `b` to pixel values.

**Note:** On devices with low flash memory, `r` **must** be an integer representing the color in the current bit depth. It cannot
be a floating point value, and `g` and `b` are ignored.
*/

JsVar *jswrap_graphics_setColorX(JsVar *parent, JsVar *r, JsVar *g, JsVar *b, bool isForeground) {
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return 0;
  unsigned int color = jswrap_graphics_toColor(parent,r,g,b);
  if (isForeground)
    gfx.data.fgColor = color;
  else
    gfx.data.bgColor = color;
  graphicsSetVar(&gfx);
  return jsvLockAgain(parent);
}

/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "getColor",
  "generate_full" : "jswrap_graphics_getColorX(parent, true)",
  "return" : ["int","The integer value of the colour"]
}
Get the color to use for subsequent drawing operations
*/
/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "getBgColor",
  "generate_full" : "jswrap_graphics_getColorX(parent, false)",
  "return" : ["int","The integer value of the colour"]
}
Get the background color to use for subsequent drawing operations
*/
JsVarInt jswrap_graphics_getColorX(JsVar *parent, bool isForeground) {
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return 0;
  return (JsVarInt)((isForeground ? gfx.data.fgColor : gfx.data.bgColor) & ((1UL<<gfx.data.bpp)-1));
}


/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "setClipRect",
  "ifndef" : "SAVE_ON_FLASH",
  "generate" : "jswrap_graphics_setClipRect",
  "params" : [
    ["x1","int","Top left X coordinate"],
    ["y1","int","Top left Y coordinate"],
    ["x2","int","Bottom right X coordinate"],
    ["y2","int","Bottom right Y coordinate"]
  ],
  "return" : ["JsVar","The instance of Graphics this was called on, to allow call chaining"],
  "return_object" : "Graphics"
}
This sets the 'clip rect' that subsequent drawing operations are clipped to sit
between.

These values are inclusive - e.g. `g.setClipRect(1,0,5,0)` will ensure that only
pixel rows 1,2,3,4,5 are touched on column 0.

**Note:** For maximum flexibility on Bangle.js 1, the values here are not range
checked. For normal use, X and Y should be between 0 and
`getWidth()-1`/`getHeight()-1`.

**Note:** The x/y values here are rotated, so that if `Graphics.setRotation` is
used they correspond to the coordinates given to the draw functions, *not to the
physical device pixels*.
*/
JsVar *jswrap_graphics_setClipRect(JsVar *parent, int x1, int y1, int x2, int y2) {
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return 0;
  graphicsToDeviceCoordinates(&gfx, &x1, &y1);
  graphicsToDeviceCoordinates(&gfx, &x2, &y2);
#ifndef SAVE_ON_FLASH
#ifdef USE_LCD_ST7789_8BIT
  if (gfx.data.type!=JSGRAPHICSTYPE_ST7789_8BIT) {
#endif
    if (x1<0) x1=0;
    if (y1<0) y1=0;
    if (x2<0) x2=0;
    if (y2<0) y2=0;
    if (x1>=gfx.data.width) x1=gfx.data.width-1;
    if (y1>=gfx.data.height) y1=gfx.data.height-1;
    if (x2>=gfx.data.width) x2=gfx.data.width-1;
    if (y2>=gfx.data.height) y2=gfx.data.height-1;
    if (x1>x2) { int t=x1;x1=x2;x2=t; };
    if (y1>y2) { int t=y1;y1=y2;y2=t; };
#ifdef USE_LCD_ST7789_8BIT
  }
#endif
  gfx.data.clipRect.x1 = (unsigned short)x1;
  gfx.data.clipRect.y1 = (unsigned short)y1;
  gfx.data.clipRect.x2 = (unsigned short)x2;
  gfx.data.clipRect.y2 = (unsigned short)y2;
  graphicsSetVar(&gfx);
#endif
  return jsvLockAgain(parent);
}


/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "setFontBitmap",
  "generate_full" : "jswrap_graphics_setFontSizeX(parent, 1+JSGRAPHICS_FONTSIZE_4X6, false)",
  "return" : ["JsVar","The instance of Graphics this was called on, to allow call chaining"],
  "return_object" : "Graphics"
}
Make subsequent calls to `drawString` use the built-in 4x6 pixel bitmapped Font

It is recommended that you use `Graphics.setFont("4x6")` for more flexibility.
*/
/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "setFontVector",
  "ifndef" : "SAVE_ON_FLASH",
  "generate_full" : "jswrap_graphics_setFontSizeX(parent, size, true)",
  "params" : [
    ["size","int32","The height of the font, as an integer"]
  ],
  "return" : ["JsVar","The instance of Graphics this was called on, to allow call chaining"],
  "return_object" : "Graphics"
}
Make subsequent calls to `drawString` use a Vector Font of the given height.

It is recommended that you use `Graphics.setFont("Vector", size)` for more
flexibility.
*/
JsVar *jswrap_graphics_setFontSizeX(JsVar *parent, int size, bool isVectorFont) {
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return 0;
#ifdef NO_VECTOR_FONT
  if (isVectorFont)
    jsExceptionHere(JSET_ERROR, "No vector font in this build");
#else
  if (isVectorFont) {
    if (size<1) size=1;
    if (size>JSGRAPHICS_FONTSIZE_SCALE_MASK) size=JSGRAPHICS_FONTSIZE_SCALE_MASK;
  }
#ifndef SAVE_ON_FLASH
  if ((gfx.data.fontSize&JSGRAPHICS_FONTSIZE_CUSTOM_BIT) &&
      !(size&JSGRAPHICS_FONTSIZE_CUSTOM_BIT)) {
    jsvObjectRemoveChild(parent, JSGRAPHICS_CUSTOMFONT_BMP);
    jsvObjectRemoveChild(parent, JSGRAPHICS_CUSTOMFONT_WIDTH);
    jsvObjectRemoveChild(parent, JSGRAPHICS_CUSTOMFONT_HEIGHT);
    jsvObjectRemoveChild(parent, JSGRAPHICS_CUSTOMFONT_FIRSTCHAR);
  }
#endif
  gfx.data.fontSize = (unsigned short)size;
  graphicsSetVar(&gfx);
#endif
  return jsvLockAgain(parent);
}
/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "setFontCustom",
  "ifndef" : "SAVE_ON_FLASH",
  "generate" : "jswrap_graphics_setFontCustom",
  "params" : [
    ["bitmap","JsVar","A column-first, MSB-first, 1bpp bitmap containing the font bitmap"],
    ["firstChar","int32","The first character in the font - usually 32 (space)"],
    ["width","JsVar","The width of each character in the font. Either an integer, or a string where each character represents the width"],
    ["height","int32","The height as an integer (max 255). Bits 8-15 represent the scale factor (eg. `2<<8` is twice the size). Bits 16-23 represent the BPP (0,1=1 bpp, 2=2 bpp, 4=4 bpp)"]
  ],
  "return" : ["JsVar","The instance of Graphics this was called on, to allow call chaining"],
  "return_object" : "Graphics",
  "typescript" : "setFontCustom(bitmap: ArrayBuffer, firstChar: number, width: number | string, height: number): Graphics;"
}
Make subsequent calls to `drawString` use a Custom Font of the given height. See
the [Fonts page](http://www.espruino.com/Fonts) for more information about
custom fonts and how to create them.

For examples of use, see the [font
modules](https://www.espruino.com/Fonts#font-modules).

**Note:** while you can specify the character code of the first character with
`firstChar`, the newline character 13 will always be treated as a newline and
not rendered.
*/
#ifndef SAVE_ON_FLASH
JsVar *jswrap_graphics_setFontCustom(JsVar *parent, JsVar *bitmap, int firstChar, JsVar *width, int height) {
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return 0;

  if (!jsvIsString(bitmap)) {
    jsExceptionHere(JSET_ERROR, "Font bitmap must be a String");
    return 0;
  }
  if (firstChar<0 || firstChar>255) {
    jsExceptionHere(JSET_ERROR, "First character out of range");
    return 0;
  }
  if (!jsvIsString(width) && !jsvIsInt(width)) {
    jsExceptionHere(JSET_ERROR, "Font width must be a String or an integer");
    return 0;
  }
  int scale = (height>>8)&255;
  if (scale<1) scale=1;
  int bpp = height>>16;
  if (bpp<1) bpp=1;
  JsGraphicsFontSize fontType;
  if (bpp==1) fontType = JSGRAPHICS_FONTSIZE_CUSTOM_1BPP;
  else if (bpp==2) fontType = JSGRAPHICS_FONTSIZE_CUSTOM_2BPP;
  else if (bpp==4) fontType = JSGRAPHICS_FONTSIZE_CUSTOM_4BPP;
  else {
    jsExceptionHere(JSET_ERROR, "Invalid BPP - 1,2,4 supported");
    return 0;
  }
  height = height&255;
  jsvObjectSetChild(parent, JSGRAPHICS_CUSTOMFONT_BMP, bitmap);
  jsvObjectSetChild(parent, JSGRAPHICS_CUSTOMFONT_WIDTH, width);
  jsvObjectSetChildAndUnLock(parent, JSGRAPHICS_CUSTOMFONT_HEIGHT, jsvNewFromInteger(height));
  jsvObjectSetChildAndUnLock(parent, JSGRAPHICS_CUSTOMFONT_FIRSTCHAR, jsvNewFromInteger(firstChar));
  gfx.data.fontSize = (unsigned short)((unsigned)scale + fontType);
  graphicsSetVar(&gfx);
  return jsvLockAgain(parent);
}
#endif

/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "setFontPBF",
  "ifndef" : "SAVE_ON_FLASH",
  "generate" : "jswrap_graphics_setFontPBF",
  "params" : [
    ["file","JsVar","The font as a PBF file"]
  ],
  "return" : ["JsVar","The instance of Graphics this was called on, to allow call chaining"],
  "return_object" : "Graphics"
}
*/
JsVar *jswrap_graphics_setFontPBF(JsVar *parent, JsVar *file) {
#ifdef ESPR_PBF_FONTS
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return 0;

  if (!jsvIsString(file)) {
    jsExceptionHere(JSET_ERROR, "Font must be a String");
    return 0;
  }
  int scale = 1;
  jsvObjectSetChild(parent, JSGRAPHICS_CUSTOMFONT_BMP, file);
  gfx.data.fontSize = (unsigned short)(scale | JSGRAPHICS_FONTSIZE_CUSTOM_PBF);
  graphicsSetVar(&gfx);
  return jsvLockAgain(parent);
#else
  jsExceptionHere(JSET_ERROR, "PBF Fonts not enabled on this build");
  return 0;
#endif
}


/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "setFontAlign",
  "ifndef" : "SAVE_ON_FLASH",
  "generate" : "jswrap_graphics_setFontAlign",
  "params" : [
    ["x","int32","X alignment. -1=left (default), 0=center, 1=right"],
    ["y","int32","Y alignment. -1=top (default), 0=center, 1=bottom"],
    ["rotation","int32","Rotation of the text. 0=normal, 1=90 degrees clockwise, 2=180, 3=270"]
  ],
  "return" : ["JsVar","The instance of Graphics this was called on, to allow call chaining"],
  "return_object" : "Graphics",
  "typescript" : "setFontAlign(x: -1 | 0 | 1, y?: -1 | 0 | 1, rotation?: 0 | 1 | 2 | 3): Graphics;"
}
Set the alignment for subsequent calls to `drawString`
*/
JsVar *jswrap_graphics_setFontAlign(JsVar *parent, int x, int y, int r) {
#ifndef SAVE_ON_FLASH
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return 0;
  if (x<-1) x=-1;
  if (x>1) x=1;
  if (y<-1) y=-1;
  if (y>1) y=1;
  if (r<0) r=0;
  if (r>3) r=3;
  gfx.data.fontAlignX = x;
  gfx.data.fontAlignY = y;
  gfx.data.fontRotate = r;
  graphicsSetVar(&gfx);
  return jsvLockAgain(parent);
#else
  return 0;
#endif
}

/*TYPESCRIPT
type FontName =
  | "4x4"
  | "4x4Numeric"
  | "4x5"
  | "4x5Numeric"
  | "4x8Numeric"
  | "5x7Numeric7Seg"
  | "5x9Numeric7Seg"
  | "6x8"
  | "6x12"
  | "7x11Numeric7Seg"
  | "8x12"
  | "8x16"
  | "Dennis8"
  | "Cherry6x10"
  | "Copasectic40x58Numeric"
  | "Dylex7x13"
  | "HaxorNarrow7x17"
  | "Sinclair"
  | "Teletext10x18Mode7"
  | "Teletext5x9Ascii"
  | "Teletext5x9Mode7"
  | "Vector";

type FontNameWithScaleFactor =
  | FontName
  | `${FontName}:${number}`
  | `${FontName}:${number}x${number}`;
*/
/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "setFont",
  "ifndef" : "SAVE_ON_FLASH",
  "generate" : "jswrap_graphics_setFont",
  "params" : [
    ["name","JsVar","The name of the font to use (if undefined, the standard 4x6 font will be used)"],
    ["size","int","The size of the font (or undefined)"]
  ],
  "return" : ["JsVar","The instance of Graphics this was called on, to allow call chaining"],
  "return_object" : "Graphics",
  "typescript" : [
    "setFont(name: FontNameWithScaleFactor): Graphics;",
    "setFont(name: FontName, size: number): Graphics;"
  ]
}
Set the font by name. Various forms are available:

* `g.setFont("4x6")` - standard 4x6 bitmap font
* `g.setFont("Vector:12")` - vector font 12px high
* `g.setFont("4x6:2")` - 4x6 bitmap font, doubled in size
* `g.setFont("6x8:2x3")` - 6x8 bitmap font, doubled in width, tripled in height

You can also use these forms, but they are not recommended:

* `g.setFont("Vector12")` - vector font 12px high
* `g.setFont("4x6",2)` - 4x6 bitmap font, doubled in size

`g.getFont()` will return the current font as a String.

For a list of available font names, you can use `g.getFonts()`.

*/
JsVar *jswrap_graphics_setFont(JsVar *parent, JsVar *fontId, int size) {
#ifndef SAVE_ON_FLASH
  if (!jsvIsString(fontId)) return 0;
  bool isVector = false;
  int fontSizeCharIdx = -1;
#ifndef NO_VECTOR_FONT
  if (jsvIsStringEqualOrStartsWith(fontId, "Vector", true)) {
    if (jsvGetStringLength(fontId)>6)
      fontSizeCharIdx = 6;
    isVector = true;
  }
#endif
  // Is font is 'FontName:2' pull the font size out of it
  int colonIdx = jsvGetStringIndexOf(fontId,':');
  if (colonIdx>=0)
    fontSizeCharIdx = colonIdx+1;
  JsVar *name;
  if (fontSizeCharIdx>=0) { // "FontName:FontSize"
    JsVar *sizeVar = jsvNewFromStringVar(fontId, fontSizeCharIdx, JSVAPPENDSTRINGVAR_MAXLENGTH);
    int xIndex = jsvGetStringIndexOf(sizeVar,'x');
    if (xIndex>=0) { // "FontName:1x3"
      int sizex = jsvGetIntegerAndUnLock(jsvNewFromStringVar(sizeVar, 0, xIndex));
      int sizey = jsvGetIntegerAndUnLock(jsvNewFromStringVar(sizeVar, xIndex+1, JSVAPPENDSTRINGVAR_MAXLENGTH));
      if (sizex<0) sizex=0;
      if (sizey<0) sizey=0;
      if (sizex>63) sizex=63;
      if (sizey>63) sizey=63;
      size = sizex | (sizey<<JSGRAPHICS_FONTSIZE_SCALE_Y_SHIFT) | JSGRAPHICS_FONTSIZE_SCALE_X_Y;
    } else { // // "FontName:12"
      size = jsvGetInteger(sizeVar);
    }
    jsvUnLock(sizeVar);
    name = jsvNewFromStringVar(fontId, 0, (fontSizeCharIdx>0)?fontSizeCharIdx-1:0);
  } else {
    name = jsvLockAgain(fontId);
  }
  if (size<1) size=1;
  if (size>JSGRAPHICS_FONTSIZE_SCALE_MASK) size=JSGRAPHICS_FONTSIZE_SCALE_MASK;
  unsigned short sz = 0xFFFF; // the actual data mask
  if (isVector) {
    sz = size;
  } else if (jsvIsUndefined(name) || jsvGetStringLength(name)==0 || jsvIsStringEqual(name, "4x6"))
    sz = (unsigned short)(size + JSGRAPHICS_FONTSIZE_4X6);
#ifdef USE_FONT_6X8
  if (jsvIsStringEqual(name, "6x8"))
    sz = (unsigned short)(size + JSGRAPHICS_FONTSIZE_6X8);
#endif
#ifndef SAVE_ON_FLASH
  // if function named 'setFontXYZ' exists, run it
  if (sz==0xFFFF) {
    JsVar *setterName = jsvVarPrintf("setFont%v",name);
    JsVar *fontSetter = jspGetVarNamedField(parent,setterName,false);
    if (fontSetter) {
      jsvUnLock(jspExecuteFunction(fontSetter,parent,0,NULL));
      JsGraphics gfx; graphicsGetFromVar(&gfx, parent); // graphicsGetFromVar *may* fail, but all it means is size won't get set (and jswrap_graphics_setFontSizeX would fail too)
      sz = (unsigned short)(size | (gfx.data.fontSize & JSGRAPHICS_FONTSIZE_FONT_MASK));
    }
    jsvUnLock2(fontSetter,setterName);
  }
#endif
  if (sz==0xFFFF) {
    jsExceptionHere(JSET_ERROR, "Unknown font %j", name);
  }
  jsvUnLock(name);
  return jswrap_graphics_setFontSizeX(parent, sz, isVector);
#else
  return 0;
#endif
}

/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "getFont",
  "ifndef" : "SAVE_ON_FLASH",
  "generate" : "jswrap_graphics_getFont",
  "return" : ["JsVar","Get the name of the current font"],
  "return_object" : "String",
  "typescript" : "getFont(): FontNameWithScaleFactor | \"Custom\""
}
Get the font by name - can be saved and used with `Graphics.setFont`.

Normally this might return something like `"4x6"`, but if a scale factor is
specified, a colon and then the size is reported, like "4x6:2"

**Note:** For custom fonts, `Custom` is currently reported instead of the font
name.
*/
JsVar *jswrap_graphics_getFont(JsVar *parent) {
#ifndef SAVE_ON_FLASH
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return 0;
  JsGraphicsFontSize f = gfx.data.fontSize & JSGRAPHICS_FONTSIZE_FONT_MASK;
  const char *name = 0;
#ifndef NO_VECTOR_FONT
  if (f == JSGRAPHICS_FONTSIZE_VECTOR) {
    name = "Vector";
  }
#endif
  if (f == JSGRAPHICS_FONTSIZE_4X6)
    name = "4x6";
#ifdef USE_FONT_6X8
  if (f == JSGRAPHICS_FONTSIZE_6X8)
    name = "6x8";
#endif
#ifndef SAVE_ON_FLASH
  if (f & JSGRAPHICS_FONTSIZE_CUSTOM_BIT) {
    /*JsVar *n = jsvObjectGetChildIfExists(parent, JSGRAPHICS_CUSTOMFONT_NAME);
    if (n) return n;*/
    name = "Custom";
  }
  if (name) {
    int scale = gfx.data.fontSize & JSGRAPHICS_FONTSIZE_SCALE_MASK;
    if (scale & JSGRAPHICS_FONTSIZE_SCALE_X_Y)
      return jsvVarPrintf("%s:%dx%d",name, scale&JSGRAPHICS_FONTSIZE_SCALE_X_MASK, (scale & JSGRAPHICS_FONTSIZE_SCALE_Y_MASK)>>JSGRAPHICS_FONTSIZE_SCALE_Y_SHIFT);
    else if (scale>1) return jsvVarPrintf("%s:%d",name, scale);
    else return jsvNewFromString(name);
  }
#endif
  return jsvNewFromInteger(gfx.data.fontSize);
#else
  return 0;
#endif
}
/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "getFonts",
  "ifndef" : "SAVE_ON_FLASH",
  "generate" : "jswrap_graphics_getFonts",
  "return" : ["JsVar","And array of font names"],
  "return_object" : "Array",
  "typescript" : "getFonts(): FontName[];"
}
Return an array of all fonts currently in the Graphics library.

**Note:** Vector fonts are specified as `Vector#` where `#` is the font height.
As there are effectively infinite fonts, just `Vector` is included in the list.
*/

void jswrap_graphics_getFonts_callback(void *cbdata, JsVar *key) {
  JsVar *result = (JsVar*)cbdata;
  if (jsvGetStringLength(key)>7 &&
      jsvIsStringEqualOrStartsWith(key, "setFont", true)) {
    if (jsvIsStringEqual(key,"setFontBitmap") ||
        jsvIsStringEqual(key,"setFontAlign") ||
        jsvIsStringEqual(key,"setFontCustom")) return; // throw away non-font functions
    JsVar *n = jsvNewFromStringVar(key, 7, JSVAPPENDSTRINGVAR_MAXLENGTH);
    jsvArrayPush(result, n);
    jsvUnLock(n);
  }
}

JsVar *jswrap_graphics_getFonts(JsVar *parent) {
#ifndef SAVE_ON_FLASH
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return 0;
  JsVar *arr = jsvNewEmptyArray();
  if (!arr) return 0;
  jsvArrayPushAndUnLock(arr, jsvNewFromString("4x6"));
#ifdef USE_FONT_6X8
  jsvArrayPushAndUnLock(arr, jsvNewFromString("6x8"));
#endif
  // vector font is added by below..
  // scan for any functions 'setFont*' and add those names
  jswrap_object_keys_or_property_names_cb(parent, JSWOKPF_INCLUDE_NON_ENUMERABLE|JSWOKPF_INCLUDE_PROTOTYPE, jswrap_graphics_getFonts_callback, arr);
  return arr;
#else
  return 0;
#endif
}

typedef struct {
  JsGraphicsFontSize font;
  unsigned short scale;
  unsigned short scalex, scaley;
  unsigned char customFirstChar;
  JsVar *widths; // Array of widths (for old-style fonts) or just an int for fixed width
  JsVar *bitmap; // Bitmap/font data
#ifdef ESPR_PBF_FONTS
  PbfFontLoaderInfo pbfInfo;
#endif
} JsGraphicsFontInfo;

static void _jswrap_graphics_getFontInfo(JsGraphics *gfx, JsGraphicsFontInfo *info) {
  info->font = gfx->data.fontSize & JSGRAPHICS_FONTSIZE_FONT_MASK;
  info->scale = gfx->data.fontSize & JSGRAPHICS_FONTSIZE_SCALE_MASK;
  info->scalex = info->scale;
  info->scaley = info->scale;
  if (info->scale & JSGRAPHICS_FONTSIZE_SCALE_X_Y) {
    info->scalex = info->scale & JSGRAPHICS_FONTSIZE_SCALE_X_MASK;
    info->scaley = (info->scale & JSGRAPHICS_FONTSIZE_SCALE_Y_MASK) >> JSGRAPHICS_FONTSIZE_SCALE_Y_SHIFT;
  }
#ifndef SAVE_ON_FLASH
  if (info->font & JSGRAPHICS_FONTSIZE_CUSTOM_BIT) {
    info->customFirstChar = (unsigned char)jsvGetIntegerAndUnLock(jsvObjectGetChildIfExists(gfx->graphicsVar, JSGRAPHICS_CUSTOMFONT_FIRSTCHAR));
    info->widths = jsvObjectGetChildIfExists(gfx->graphicsVar, JSGRAPHICS_CUSTOMFONT_WIDTH);
    info->bitmap = jsvObjectGetChildIfExists(gfx->graphicsVar, JSGRAPHICS_CUSTOMFONT_BMP);
#ifdef ESPR_PBF_FONTS
    if ((info->font & JSGRAPHICS_FONTSIZE_FONT_MASK) == JSGRAPHICS_FONTSIZE_CUSTOM_PBF)
      jspbfFontNew(&info->pbfInfo, info->bitmap);
#endif
  } else
#endif
    info->customFirstChar = 0;

}

static void _jswrap_graphics_freeFontInfo(JsGraphicsFontInfo *info) {
#ifndef SAVE_ON_FLASH
  if (info->font & JSGRAPHICS_FONTSIZE_CUSTOM_BIT) {
    jsvUnLock(info->widths);
    jsvUnLock(info->bitmap);
#ifdef ESPR_PBF_FONTS
    if ((info->font & JSGRAPHICS_FONTSIZE_FONT_MASK) == JSGRAPHICS_FONTSIZE_CUSTOM_PBF)
      jspbfFontFree(&info->pbfInfo);
#endif
  }
#endif
}

static int _jswrap_graphics_getCharWidth(JsGraphics *gfx, JsGraphicsFontInfo *info, int ch) {
  if ((info->font == JSGRAPHICS_FONTSIZE_VECTOR) && (ch<256)) {
#ifndef NO_VECTOR_FONT
    return (int)graphicsVectorCharWidth(info->scalex, (char)ch);
#endif
  } else if ((info->font == JSGRAPHICS_FONTSIZE_4X6) && (ch<256)) {
    return 4*info->scalex;
#ifdef USE_FONT_6X8
  } else if ((info->font == JSGRAPHICS_FONTSIZE_6X8) && (ch<256)) {
    return 6*info->scalex;
#endif
#ifndef SAVE_ON_FLASH
  } else if (info->font & JSGRAPHICS_FONTSIZE_CUSTOM_BIT) {
#ifdef ESPR_PBF_FONTS
    if ((info->font & JSGRAPHICS_FONTSIZE_FONT_MASK)==JSGRAPHICS_FONTSIZE_CUSTOM_PBF) {
      PbfFontLoaderGlyph result;
      if (jspbfFontFindGlyph(&info->pbfInfo, ch, &result))
        return result.advance;
      else
        return 0;
    }
#endif
    int w = 0;
    if (ch<256) {
      if (jsvIsString(info->widths)) {
        if (ch>=info->customFirstChar)
          w = info->scalex*(unsigned char)jsvGetCharInString(info->widths, (size_t)(ch-info->customFirstChar));
      } else
        w = info->scalex*(int)jsvGetInteger(info->widths);
    }
    return w;
#endif
  }
  return 0;
}

/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "getFontHeight",
  "ifndef" : "SAVE_ON_FLASH",
  "generate" : "jswrap_graphics_getFontHeight",
  "return" : ["int","The height in pixels of the current font"]
}
Return the height in pixels of the current font
*/
static int _jswrap_graphics_getFontHeightInternal(JsGraphics *gfx, JsGraphicsFontInfo *info) {
  if (info->font == JSGRAPHICS_FONTSIZE_VECTOR) {
    return info->scaley;
  } else if (info->font == JSGRAPHICS_FONTSIZE_4X6) {
    return 6*info->scaley;
#ifdef USE_FONT_6X8
  } else if (info->font == JSGRAPHICS_FONTSIZE_6X8) {
    return 8*info->scaley;
#endif
#ifndef SAVE_ON_FLASH
  } else if (info->font & JSGRAPHICS_FONTSIZE_CUSTOM_BIT) {
#ifdef ESPR_PBF_FONTS
    if ((info->font & JSGRAPHICS_FONTSIZE_FONT_MASK)==JSGRAPHICS_FONTSIZE_CUSTOM_PBF)
      return info->pbfInfo.lineHeight;
#endif
    return info->scaley*(int)jsvGetIntegerAndUnLock(jsvObjectGetChildIfExists(gfx->graphicsVar, JSGRAPHICS_CUSTOMFONT_HEIGHT));
#endif
  }
  return 0;
}
int jswrap_graphics_getFontHeight(JsVar *parent) {
#ifndef SAVE_ON_FLASH
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return 0;
  JsGraphicsFontInfo info;
  _jswrap_graphics_getFontInfo(&gfx, &info);
  int h = _jswrap_graphics_getFontHeightInternal(&gfx, &info);
  _jswrap_graphics_freeFontInfo(&info);
  return h;
#else
  return 0;
#endif
}

typedef struct {
  int stringWidth;     // width in pixels
  int stringHeight;    // height in pixels
  bool unrenderableChars; // are any chars in this not renderable in the current font?
#ifndef SAVE_ON_FLASH
  int imageCount;      // how many inline images are in this string?
  int maxImageHeight;   // maximum height of image in this string
#endif
} StringMetricsResult;

/** Work out the width and height of a bit of text. If 'lineStartIndex' is -1 the whole string is used
 * otherwise *just* the line of text starting at that char index is used */
void _jswrap_graphics_stringMetrics(JsGraphics *gfx, JsVar *var, int lineStartIndex, StringMetricsResult *result) {
  JsGraphicsFontInfo info;
  _jswrap_graphics_getFontInfo(gfx, &info);
  memset(result, 0, sizeof(result));

  int fontHeight = _jswrap_graphics_getFontHeightInternal(gfx, &info);
  JsVar *str = jsvAsString(var);
  JsvStringIterator it;
  jsvStringIteratorNewUTF8(&it, str, (lineStartIndex<0)?0:lineStartIndex);
  int width = 0;
  int height = fontHeight;
  int maxWidth = 0;

  while (jsvStringIteratorHasChar(&it)) {
    int ch = jsvStringIteratorGetUTF8CharAndNext(&it);
    if (ch=='\n') {
      if (width>maxWidth) maxWidth=width;
      width = 0;
      height += fontHeight;
      if (lineStartIndex>=0) break; // only do one line
    }
#ifndef SAVE_ON_FLASH
    if (ch==0) { // If images are described in-line in the string, render them
      result->imageCount++;
      GfxDrawImageInfo img;
      size_t idx = jsvConvertToUTF8Index(str, jsvStringIteratorGetIndex(&it));
      if (_jswrap_graphics_parseImage(gfx, str, idx, &img)) {
        jsvStringIteratorGotoUTF8(&it, str, idx+img.headerLength+img.bitmapLength);
        _jswrap_graphics_freeImageInfo(&img);
        // string iterator now points to the next char after image
        width += img.width;
        if (img.height > result->maxImageHeight)
          result->maxImageHeight = img.height;
      }
      continue;
    }
#endif
    int w = _jswrap_graphics_getCharWidth(gfx, &info, ch);
    width += w;
    if (w==0) result->unrenderableChars = true; // assume width=0 means char not found
  }
  jsvStringIteratorFree(&it);
  jsvUnLock(str);
  result->stringWidth = width>maxWidth ? width : maxWidth;
  result->stringHeight = height;
  _jswrap_graphics_freeFontInfo(&info);
}
JsVarInt _jswrap_graphics_stringWidth(JsGraphics *gfx, JsVar *var, int lineStartIndex) {
  StringMetricsResult metrics;
  _jswrap_graphics_stringMetrics(gfx, var, lineStartIndex, &metrics);
  return metrics.stringWidth;
}

/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "stringWidth",
  "generate" : "jswrap_graphics_stringWidth",
  "params" : [
    ["str","JsVar","The string"]
  ],
  "return" : ["int","The length of the string in pixels"],
  "typescript" : "stringWidth(str: string): number;"
}
Return the size in pixels of a string of text in the current font
*/
JsVarInt jswrap_graphics_stringWidth(JsVar *parent, JsVar *var) {
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return 0;
  return _jswrap_graphics_stringWidth(&gfx, var, -1);
}

/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "stringMetrics",
  "generate" : "jswrap_graphics_stringMetrics",
  "params" : [
    ["str","JsVar","The string"]
  ],
  "return" : ["JsVar","An object containing `{width,height,etc}` for the string - see below"],
  "typescript" : "stringMetrics(str: string): { width: number, height: number, unrenderableChars: boolean, imageCount : number, maxImageHeight : number };"
}
Return the width and height in pixels of a string of text in the current font. The object returned contains:

```JS
{
  width,              // Width of the string in pixels
  height,             // Height of the string in pixels
  unrenderableChars,  // If true, the string contains characters that the current font isn't able to render.
  imageCount,         // How many inline images are in this string?
  maxImageHeight,     // If there are images, what is the maximum height of all images?
}
```

*/
JsVar* jswrap_graphics_stringMetrics(JsVar *parent, JsVar *var) {
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return 0;
  StringMetricsResult metrics;
  JsVar *o = jsvNewObject();
  if (o) {
    _jswrap_graphics_stringMetrics(&gfx, var, -1, &metrics);
    jsvObjectSetChildAndUnLock(o, "width", jsvNewFromInteger(metrics.stringWidth));
    jsvObjectSetChildAndUnLock(o, "height", jsvNewFromInteger(metrics.stringHeight));
    jsvObjectSetChildAndUnLock(o, "unrenderableChars", jsvNewFromBool(metrics.unrenderableChars));
#ifndef SAVE_ON_FLASH
    jsvObjectSetChildAndUnLock(o, "imageCount", jsvNewFromInteger(metrics.imageCount));
    jsvObjectSetChildAndUnLock(o, "maxImageHeight", jsvNewFromInteger(metrics.maxImageHeight));
#endif
  }
  return o;
}

/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "wrapString",
  "generate" : "jswrap_graphics_wrapString",
  "params" : [
    ["str","JsVar","The string"],
    ["maxWidth","int","The width in pixels"]
  ],
  "return" : ["JsVar","An array of lines that are all less than `maxWidth`"],
  "typescript" : "wrapString(str: string, maxWidth: number): string[];"
}
Wrap a string to the given pixel width using the current font, and return the
lines as an array.

To render within the screen's width you can do:

```
g.drawString(g.wrapString(text, g.getWidth()).join("\n")),
```
*/
JsVar *jswrap_graphics_wrapString(JsVar *parent, JsVar *str, int maxWidth) {
  if (!str) return jsvNewEmptyArray();
  if (maxWidth<=0) return 0;
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return 0;
  JsGraphicsFontInfo info;
  _jswrap_graphics_getFontInfo(&gfx, &info);
  str = jsvAsString(str);
  JsVar *lines = jsvNewEmptyArray();
  JsVar *currentLine = jsvNewFromEmptyString();
#ifdef ESPR_UNICODE_SUPPORT
  if (jsvIsUTF8String(str))
    currentLine = jsvNewUTF8StringAndUnLock(currentLine);
#endif

  int spaceWidth = _jswrap_graphics_getCharWidth(&gfx, &info, ' ');
  int wordWidth = 0;
  int lineWidth = 0;
  bool canSplitAfter = false;
  char wordBreakCharacter = 0; // character that came before wordStartIdx (if space we may have left it off)
  int wordStartIdx = 0;
  bool endOfText = false;
  bool wasNewLine = false;

  /* What we do is try and step over one whole word/image at a time, then when we've got that
   (with wordStartIdx at the start) we see whether we have space to append to currentLine,
   or whether we have to start a new line.

   This is all a bit of a mess but it appears to work as intended now. */
  JsvStringIterator it;
  jsvStringIteratorNewUTF8(&it, str, 0);
  while (jsvStringIteratorHasChar(&it) || endOfText) {
    int ch = jsvStringIteratorGetUTF8CharAndNext(&it);
    bool canBreakOnCh = endOfText || ch=='\n' || ch==' ';
    if (canBreakOnCh || canSplitAfter) { // is breakable - newline,space,dash, image before
      int currentPos = (int)jsvStringIteratorGetIndex(&it);
      if ((lineWidth + spaceWidth + wordWidth <= maxWidth) &&
          !wasNewLine) {
        // all on one line, just append the last word
        if (wordBreakCharacter && (lineWidth || wordBreakCharacter!=' ')) {
          // add the space/etc before (but not a space at the start of a newline)
          jsvAppendCharacter(currentLine, wordBreakCharacter);
          lineWidth += spaceWidth;
          wordBreakCharacter = 0;
        }
        jsvAppendStringVar(currentLine, str, wordStartIdx, currentPos-(wordStartIdx+1));
        lineWidth += wordWidth;
      } else { // doesn't fit on one line - put word on new line
        lineWidth = wordWidth;
        if (jsvGetStringLength(currentLine) || wasNewLine)
          jsvArrayPush(lines, currentLine);
        jsvUnLock(currentLine);
        currentLine = 0;
        // if the word is too big to fit in the line, split it until it fits
        while (wordWidth > maxWidth) {
          int width = 0;
          currentLine = jsvNewFromEmptyString();
          while (wordStartIdx < currentPos) {
            char wordCh = jsvGetCharInString(str, wordStartIdx);
            int w = _jswrap_graphics_getCharWidth(&gfx, &info, wordCh);
            if (width+w < maxWidth) {
              wordStartIdx++;
              wordWidth -= w;
              lineWidth -= w;
              width += w;
              jsvAppendCharacter(currentLine, wordCh);
            } else
              break;
          }
          jsvArrayPush(lines, currentLine);
          jsvUnLock(currentLine);
        }
        // Add the remaining bit of word
        currentLine = jsvNewFromStringVar(str, wordStartIdx, currentPos-(wordStartIdx+1));
        // jsvNewFromStringVar will create a unicode string is str was a unicode string
        if (wasNewLine) wordBreakCharacter = ' ';
      }
      if (canSplitAfter && !canBreakOnCh) currentPos--; // include the current ch in the next word
      if (canBreakOnCh && ch>0)
        wordBreakCharacter = (char)ch;
      // we're now starting a new word
      wordWidth = 0;
      wordStartIdx = currentPos;
      wasNewLine = ch=='\n';
      canSplitAfter = false;
      if (endOfText) break;
      if (ch!=0) continue; // allow us to handle images next
    }
    canSplitAfter = false;
#ifndef SAVE_ON_FLASH
    if (ch==0) { // If images are described in-line in the string, render them
      GfxDrawImageInfo img;
      size_t idx = jsvConvertToUTF8Index(str, jsvStringIteratorGetIndex(&it));
      if (_jswrap_graphics_parseImage(&gfx, str, idx, &img)) {
        jsvStringIteratorGotoUTF8(&it, str, idx+img.headerLength+img.bitmapLength);
        _jswrap_graphics_freeImageInfo(&img);
        // string iterator now points to the next char after image
        wordWidth += img.width;
        canSplitAfter = true;
        if (!jsvStringIteratorHasChar(&it)) endOfText=true;
      }
      continue;
    }
#endif
    wordWidth += _jswrap_graphics_getCharWidth(&gfx, &info, ch);
    if (ch==',' || ch=='.' || ch=='-' || ch=='-' || ch==':')
      canSplitAfter = true;
    if (!jsvStringIteratorHasChar(&it)) endOfText=true;
  }
  jsvStringIteratorFree(&it);
  // deal with final line
  if (jsvGetStringLength(currentLine)) {
    jsvArrayPush(lines, currentLine);
  }
  jsvUnLock2(str,currentLine);
   _jswrap_graphics_freeFontInfo(&info);
  return lines;
}

/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "drawString",
  "generate" : "jswrap_graphics_drawString",
  "params" : [
    ["str","JsVar","The string"],
    ["x","int32","The X position of the leftmost pixel"],
    ["y","int32","The Y position of the topmost pixel"],
    ["solid","bool","For bitmap fonts, should empty pixels be filled with the background color?"]
  ],
  "return" : ["JsVar","The instance of Graphics this was called on, to allow call chaining"],
  "return_object" : "Graphics",
  "typescript" : "drawString(str: string, x: number, y: number, solid?: boolean): Graphics;"
}
Draw a string of text in the current font.

```
g.drawString("Hello World", 10, 10);
```

Images may also be embedded inside strings (e.g. to render Emoji or characters
not in the current font). To do this, just add `0` then the image string ([about
Images](http://www.espruino.com/Graphics#images-bitmaps)) For example:

```
g.drawString("Hi \0\7\5\1\x82 D\x17\xC0");
// draws:
// # #  #      #     #
// # #            #
// ### ##         #
// # #  #      #     #
// # # ###      #####
```
*/
JsVar *jswrap_graphics_drawString(JsVar *parent, JsVar *var, int x, int y, bool solidBackground) {
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return 0;

  JsGraphicsFontInfo info;
  _jswrap_graphics_getFontInfo(&gfx, &info);
  int fontHeight = _jswrap_graphics_getFontHeightInternal(&gfx, &info);

#ifndef SAVE_ON_FLASH
  int customBPP = 1;

  if (info.font & JSGRAPHICS_FONTSIZE_CUSTOM_BIT) {
    if ((info.font&JSGRAPHICS_FONTSIZE_FONT_MASK)==JSGRAPHICS_FONTSIZE_CUSTOM_2BPP) customBPP = 2;
    if ((info.font&JSGRAPHICS_FONTSIZE_FONT_MASK)==JSGRAPHICS_FONTSIZE_CUSTOM_4BPP) customBPP = 4;
  }
#endif
#ifndef SAVE_ON_FLASH
  // Handle text rotation
  JsGraphicsFlags oldFlags = gfx.data.flags;
  if (gfx.data.fontRotate==1) {
    gfx.data.flags ^= JSGRAPHICSFLAGS_SWAP_XY | JSGRAPHICSFLAGS_INVERT_X;
    int t = gfx.data.width - (x+1);
    x = y;
    y = t;
  } else if (gfx.data.fontRotate==2) {
    gfx.data.flags ^= JSGRAPHICSFLAGS_INVERT_X | JSGRAPHICSFLAGS_INVERT_Y;
    x = gfx.data.width - (x+1);
    y = gfx.data.height - (y+1);
  } else if (gfx.data.fontRotate==3) {
    gfx.data.flags ^= JSGRAPHICSFLAGS_SWAP_XY | JSGRAPHICSFLAGS_INVERT_Y;
    int t = gfx.data.height - (y+1);
    y = x;
    x = t;
  }
  JsVar *str = jsvAsString(var);
  // Handle font alignment
  int startx = x;
  if (gfx.data.fontAlignX<2) // 0=center, 1=right, 2=undefined, 3=left
    x = startx - (_jswrap_graphics_stringWidth(&gfx, str, 0) * (gfx.data.fontAlignX+1)/2);
  if (gfx.data.fontAlignY<2) { // 0=center, 1=bottom, 2=undefined, 3=top
    StringMetricsResult metrics;
    // Get width/height of entire string
    _jswrap_graphics_stringMetrics(&gfx, str, -1, &metrics);
    y -= metrics.stringHeight * (gfx.data.fontAlignY+1)/2;
  }
  // figure out clip rectangles
  int minX = gfx.data.clipRect.x1, minY = gfx.data.clipRect.y1;
  int maxX = gfx.data.clipRect.x2, maxY = gfx.data.clipRect.y2;
  deviceToGraphicsCoordinates(&gfx, &minX, &minY);
  deviceToGraphicsCoordinates(&gfx, &maxX, &maxY);
  if (maxX < minX) {
    int t = minX; minX = maxX; maxX = t;
  }
  if (maxY < minY) {
    int t = minY; minY = maxY; maxY = t;
  }
#else
  int minX = 0;
  int minY = 0;
  int maxX = graphicsGetWidth(&gfx) - 1;
  int maxY = graphicsGetHeight(&gfx) - 1;
  int startx = x;
  JsVar *str = jsvAsString(var);
#endif
  JsvStringIterator it;
  jsvStringIteratorNewUTF8(&it, str, 0);
  while (jsvStringIteratorHasChar(&it)) {
    int ch = jsvStringIteratorGetUTF8CharAndNext(&it);
    if (ch=='\n') {
      x = startx;
#ifndef SAVE_ON_FLASH
      // alignment for non-left aligned multi-line strings
      if (gfx.data.fontAlignX<2) // 0=center, 1=right, 2=undefined, 3=left
        x = startx - (_jswrap_graphics_stringWidth(&gfx, str, jsvStringIteratorGetIndex(&it)) * (gfx.data.fontAlignX+1)/2);
#endif
      y += fontHeight;
      continue;
    }
#ifndef SAVE_ON_FLASH
    if (ch==0) { // If images are described in-line in the string, render them
      GfxDrawImageInfo img;
      size_t idx = jsvConvertToUTF8Index(str, jsvStringIteratorGetIndex(&it));
      if (_jswrap_graphics_parseImage(&gfx, str, idx, &img)) {
        jsvStringIteratorGotoUTF8(&it, str, idx+img.headerLength);
        _jswrap_drawImageSimple(&gfx, x, y+(fontHeight-img.height)/2, &img, &it, true/*string iterator now points to the next char after image*/);
        _jswrap_graphics_freeImageInfo(&img);
        x += img.width;
      }
      continue;
    }
#endif
    if ((info.font == JSGRAPHICS_FONTSIZE_VECTOR) && (ch<256)) {
#ifndef NO_VECTOR_FONT
      int w = (int)graphicsVectorCharWidth(info.scalex, (char)ch);
      // TODO: potentially we could do this in x16 accuracy so vector chars rendered together better
      if (x>minX-w && x<maxX  && y>minY-fontHeight && y<=maxY) {
        if (solidBackground)
          graphicsFillRect(&gfx,x,y,x+w-1,y+fontHeight-1, gfx.data.bgColor);
        graphicsGetVectorChar((graphicsPolyCallback)graphicsFillPoly, &gfx, x, y, info.scalex, info.scaley, (char)ch);
      }
      x+=w;
#endif
    } else if ((info.font == JSGRAPHICS_FONTSIZE_4X6) && (ch<256)) {
      if (x>minX-4*info.scalex && x<maxX && y>minY-fontHeight && y<=maxY)
        graphicsDrawChar4x6(&gfx, x, y, (char)ch, info.scalex, info.scaley, solidBackground);
      x+=4*info.scalex;
#ifdef USE_FONT_6X8
    } else if (info.font == JSGRAPHICS_FONTSIZE_6X8) {
      if (x>minX-6*info.scalex && x<maxX && y>minY-fontHeight && y<=maxY)
        graphicsDrawChar6x8(&gfx, x, y, (char)ch, info.scalex, info.scaley, solidBackground);
      x+=6*info.scalex;
#endif
#ifndef SAVE_ON_FLASH
#ifdef ESPR_PBF_FONTS
    } else if ((info.font & JSGRAPHICS_FONTSIZE_FONT_MASK)==JSGRAPHICS_FONTSIZE_CUSTOM_PBF) {
      PbfFontLoaderGlyph glyph;
      if (jspbfFontFindGlyph(&info.pbfInfo, ch, &glyph)) {
        jspbfFontRenderGlyph(&info.pbfInfo, &glyph, &gfx,
                x+glyph.x*info.scalex, y+glyph.y*info.scaley,
                solidBackground, info.scalex, info.scaley);
        x+=glyph.advance*info.scalex;
      }
#endif
    } else if ((info.font & JSGRAPHICS_FONTSIZE_CUSTOM_BIT) && (ch<256)) {
      int customBPPRange = (1<<customBPP)-1;
      // get char width and offset in string
      int width = 0, bmpOffset = 0;
      if (jsvIsString(info.widths)) {
        if (ch>=info.customFirstChar) {
          JsvStringIterator wit;
          jsvStringIteratorNew(&wit, info.widths, 0);
          while (jsvStringIteratorHasChar(&wit) && (int)jsvStringIteratorGetIndex(&wit)<(ch-info.customFirstChar)) {
            bmpOffset += (unsigned char)jsvStringIteratorGetCharAndNext(&wit);
          }
          width = (unsigned char)jsvStringIteratorGetChar(&wit);
          jsvStringIteratorFree(&wit);
        }
      } else {
        width = (int)jsvGetInteger(info.widths);
        bmpOffset = width*(ch-info.customFirstChar);
      }
      if (ch>=info.customFirstChar && (x>minX-width*info.scalex) && (x<maxX) && (y>minY-fontHeight) && y<=maxY) {
        int ch = fontHeight/info.scaley;
        bmpOffset *= ch * customBPP;
        // now render character
        JsvStringIterator cit;
        jsvStringIteratorNew(&cit, info.bitmap, (size_t)(bmpOffset>>3));
        bmpOffset &= 7;
        int cx,cy;
        int citdata = jsvStringIteratorGetChar(&cit);
        citdata <<= customBPP*bmpOffset;
        for (cx=0;cx<width;cx++) {
          for (cy=0;cy<ch;cy++) {
            int col = ((citdata&255)>>(8-customBPP));
            if (solidBackground || col)
              graphicsFillRect(&gfx,
                  (x + cx*info.scalex),
                  (y + cy*info.scaley),
                  (x + cx*info.scalex + info.scalex-1),
                  (y + cy*info.scaley + info.scaley-1),
                  graphicsBlendGfxColor(&gfx, (256*col)/customBPPRange));
            bmpOffset += customBPP;
            citdata <<= customBPP;
            if (bmpOffset>=8) {
              bmpOffset=0;
              jsvStringIteratorNext(&cit);
              citdata = jsvStringIteratorGetChar(&cit);
            }
          }
        }
        jsvStringIteratorFree(&cit);
      }
      x += width*info.scalex;
#endif
    }
    if (jspIsInterrupted()) break;
  }
  jsvStringIteratorFree(&it);
  jsvUnLock(str);
#ifndef SAVE_ON_FLASH
  gfx.data.flags = oldFlags; // restore flags because of text rotation
  graphicsSetVar(&gfx); // gfx data changed because modified area
#endif
_jswrap_graphics_freeFontInfo(&info);
  return jsvLockAgain(parent);
}

/// Convenience function for using drawString from C code
void jswrap_graphics_drawCString(JsGraphics *gfx, int x, int y, char *str) {
  JsVar *s = jsvNewFromString(str);
  jsvUnLock2(jswrap_graphics_drawString(gfx->graphicsVar, s, x, y, false),s);
}


/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "getVectorFontPolys",
  "#if" : "!defined(SAVE_ON_FLASH) || !defined(NO_VECTOR_FONT)",
  "generate" : "jswrap_graphics_getVectorFontPolys",
  "params" : [
    ["str","JsVar","The string"],
    ["options","JsVar","[optional] `{x,y,w,h}` (see below)"]
  ],
  "return" : ["JsVar","An array of Uint8Arrays for vector font polygons"],
  "return_object" : "Array"
}
Return the current string as a series of polygons (using the current vector font). `options` is as follows:

* `x` - X offset of font (default 0)
* `y` - Y offset of font (default 0)
* `w` - Width of font (default 256) - the actual width will likely be less than this as most characters are non-square
* `h` - Height of font (default 256) - the actual height will likely be less than this as most characters don't fully fill the font box

```
g.getVectorFontPolys("Hi", {x:-80,y:-128});
```
*/
void _jswrap_graphics_getVectorFontPolys_cb(void *data, int points, short *vertices) {
  JsVar *arr = (JsVar *)data;
  if (!arr) return;
  // scale back down
  for (int i=0;i<points*2;i+=2) {
    vertices[i  ] = (short)(vertices[i  ]+8)>>4;
    vertices[i+1] = (short)(vertices[i+1]+8)>>4;
  }
  // create 16 bit array with the data
  JsVar *v = jsvNewTypedArray(ARRAYBUFFERVIEW_INT16, points*2);
  if (v) {
    JsVar *d = jsvGetArrayBufferBackingString(v, NULL);
    if (d) {
      jsvSetString(d, (char*)vertices, points*4);
      jsvUnLock(d);
    }
    jsvArrayPushAndUnLock(arr, v);
  }

}
JsVar *jswrap_graphics_getVectorFontPolys(JsGraphics *gfx, JsVar *str, JsVar *options) {
#ifndef NO_VECTOR_FONT
  int x = 0, y = 0, scalex = 256, scaley = 256;
  jsvConfigObject configs[] = {
      {"x", JSV_INTEGER, &x},
      {"y", JSV_INTEGER, &y},
      {"w", JSV_INTEGER, &scalex},
      {"h", JSV_INTEGER, &scaley},
  };
  jsvReadConfigObject(options, configs, sizeof(configs) / sizeof(jsvConfigObject));
  str = jsvAsString(str);
  JsVar *arr = jsvNewEmptyArray();
  JsvStringIterator it;
  jsvStringIteratorNew(&it, str, 0);
  while (jsvStringIteratorHasChar(&it)) {
    char ch = jsvStringIteratorGetCharAndNext(&it);
    x += graphicsGetVectorChar(_jswrap_graphics_getVectorFontPolys_cb, arr, x, y, scalex, scaley, ch);
  }
  jsvStringIteratorFree(&it);
  jsvUnLock(str);
  return arr;
#else
  return 0;
#endif
}


/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "drawLineString",
  "#if" : "defined(DICKENS)",
  "generate" : "jswrap_graphics_drawLineString",
  "params" : [
    ["str","JsVar","The string"],
    ["x","int32","The X position of the start of the text string"],
    ["y","int32","The Y position of the middle of the text string"],
    ["options","JsVar","Options for drawing this font (see below)"]
  ],
  "return" : ["JsVar","The instance of Graphics this was called on, to allow call chaining"],
  "return_object" : "Graphics"
}
Draw a string of text as a fixed-width line font

`options` contains:

* `size`: font size in pixels (char width is half font size) - default 16
* `rotate`: Initial rotation in radians - default 0
* `twist`: Subsequent rotation per character in radians - default 0
*/
#ifdef ESPR_LINE_FONTS
JsVar *jswrap_graphics_drawLineString(JsVar *parent, JsVar *var, int x, int y, JsVar *options) {
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return 0;

  int fontSize = 16;
  double rotate = 0, twist = 0;
  jsvConfigObject configs[] = {
      {"size", JSV_INTEGER, &fontSize},
      {"rotate", JSV_FLOAT, &rotate},
      {"twist", JSV_FLOAT, &twist}
  };
  if (!jsvReadConfigObject(options, configs, sizeof(configs) / sizeof(jsvConfigObject))) {
    jsExceptionHere(JSET_ERROR, "Invalid options");
    return 0;
  }
  fontSize *= 16;

  x = x*16 - 8;
  y = y*16 - 8;
  int startx = x;

  JsVar *str = jsvAsString(var);
  JsvStringIterator it;
  jsvStringIteratorNew(&it, str, 0);
  while (jsvStringIteratorHasChar(&it)) {
    char ch = jsvStringIteratorGetCharAndNext(&it);
    if (ch=='\n') {
      x = startx;
      y += fontSize;
      continue;
    }
    int xdx = (int)(0.5 + fontSize*cos(rotate));
    int xdy = (int)(0.5 + fontSize*sin(rotate));
    graphicsDrawLineChar(&gfx, x, y, xdx, xdy, ch);
    x += xdx * 1 / 2;
    y += xdy * 1 / 2;
    rotate += twist;
    if (jspIsInterrupted()) break;
  }
  jsvStringIteratorFree(&it);
  jsvUnLock(str);
  graphicsSetVar(&gfx); // gfx data changed because modified area
  return jsvLockAgain(parent);
}
#endif

/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "drawLine",
  "generate" : "jswrap_graphics_drawLine",
  "params" : [
    ["x1","int32","The left"],
    ["y1","int32","The top"],
    ["x2","int32","The right"],
    ["y2","int32","The bottom"]
  ],
  "return" : ["JsVar","The instance of Graphics this was called on, to allow call chaining"],
  "return_object" : "Graphics"
}
Draw a line between x1,y1 and x2,y2 in the current foreground color
*/
JsVar *jswrap_graphics_drawLine(JsVar *parent, int x1, int y1, int x2, int y2) {
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return 0;
  graphicsDrawLine(&gfx, x1,y1,x2,y2);
  graphicsSetVar(&gfx); // gfx data changed because modified area
  return jsvLockAgain(parent);
}

/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "drawLineAA",
  "ifdef" : "GRAPHICS_ANTIALIAS",
  "generate" : "jswrap_graphics_drawLineAA",
  "params" : [
    ["x1","float","The left"],
    ["y1","float","The top"],
    ["x2","float","The right"],
    ["y2","float","The bottom"]
  ],
  "return" : ["JsVar","The instance of Graphics this was called on, to allow call chaining"],
  "return_object" : "Graphics"
}
Draw a line between x1,y1 and x2,y2 in the current foreground color
*/
#ifdef GRAPHICS_ANTIALIAS
JsVar *jswrap_graphics_drawLineAA(JsVar *parent, double x1, double y1, double x2, double y2) {
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return 0;
  graphicsDrawLineAA(&gfx,
      (int)(x1*16+0.5),
      (int)(y1*16+0.5),
      (int)(x2*16+0.5),
      (int)(y2*16+0.5));
  graphicsSetVar(&gfx); // gfx data changed because modified area
  return jsvLockAgain(parent);
}
#endif


/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "lineTo",
  "generate" : "jswrap_graphics_lineTo",
  "params" : [
    ["x","int32","X value"],
    ["y","int32","Y value"]
  ],
  "return" : ["JsVar","The instance of Graphics this was called on, to allow call chaining"],
  "return_object" : "Graphics"
}
Draw a line from the last position of `lineTo` or `moveTo` to this position
*/
JsVar *jswrap_graphics_lineTo(JsVar *parent, int x, int y) {
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return 0;
  graphicsDrawLine(&gfx, gfx.data.cursorX, gfx.data.cursorY, x, y);
  gfx.data.cursorX = (short)x;
  gfx.data.cursorY = (short)y;
  graphicsSetVar(&gfx);
  return jsvLockAgain(parent);
}

/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "moveTo",
  "generate" : "jswrap_graphics_moveTo",
  "params" : [
    ["x","int32","X value"],
    ["y","int32","Y value"]
  ],
  "return" : ["JsVar","The instance of Graphics this was called on, to allow call chaining"],
  "return_object" : "Graphics"
}
Move the cursor to a position - see lineTo
*/
JsVar *jswrap_graphics_moveTo(JsVar *parent, int x, int y) {
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return 0;
  gfx.data.cursorX = (short)x;
  gfx.data.cursorY = (short)y;
  graphicsSetVar(&gfx);
  return jsvLockAgain(parent);
}

/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "drawPoly",
  "ifndef" : "SAVE_ON_FLASH",
  "generate_full" : "jswrap_graphics_drawPoly_X(parent, poly, closed, false)",
  "params" : [
    ["poly","JsVar","An array of vertices, of the form ```[x1,y1,x2,y2,x3,y3,etc]```"],
    ["closed","bool","Draw another line between the last element of the array and the first"]
  ],
  "return" : ["JsVar","The instance of Graphics this was called on, to allow call chaining"],
  "return_object" : "Graphics",
  "typescript" : "drawPoly(poly: number[], closed?: boolean): Graphics;"
}
Draw a polyline (lines between each of the points in `poly`) in the current
foreground color

**Note:** there is a limit of 64 points (128 XY elements) for polygons
*/
/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "drawPolyAA",
  "ifdef" : "GRAPHICS_ANTIALIAS",
  "generate_full" : "jswrap_graphics_drawPoly_X(parent, poly, closed, true)",
  "params" : [
    ["poly","JsVar","An array of vertices, of the form ```[x1,y1,x2,y2,x3,y3,etc]```"],
    ["closed","bool","Draw another line between the last element of the array and the first"]
  ],
  "return" : ["JsVar","The instance of Graphics this was called on, to allow call chaining"],
  "return_object" : "Graphics",
  "typescript" : "drawPolyAA(poly: number[], closed?: boolean): Graphics;"
}
Draw an **antialiased** polyline (lines between each of the points in `poly`) in
the current foreground color

**Note:** there is a limit of 64 points (128 XY elements) for polygons
*/
JsVar *jswrap_graphics_drawPoly_X(JsVar *parent, JsVar *poly, bool closed, bool antiAlias) {
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return 0;
  if (!jsvIsIterable(poly)) return 0;
  int scale;
  void (*drawFn)(JsGraphics *gfx, int x1, int y1, int x2, int y2);
#ifdef GRAPHICS_ANTIALIAS
  if (antiAlias) {
    scale = 16;
    drawFn = graphicsDrawLineAA;
  } else
#endif
  {
    scale = 1;
    drawFn = graphicsDrawLine;
  }

  int lx,ly;
  int startx, starty;
  int idx = 0;
  JsvIterator it;
  jsvIteratorNew(&it, poly, JSIF_EVERY_ARRAY_ELEMENT);
  while (jsvIteratorHasElement(&it)) {
    int x,y;
    x = (int)((jsvIteratorGetFloatValue(&it)*scale)+0.5);
    jsvIteratorNext(&it);
    if (!jsvIteratorHasElement(&it)) break;
    y = (int)((jsvIteratorGetFloatValue(&it)*scale)+0.5);
    jsvIteratorNext(&it);
    if (idx==0) { // save xy positions of first point
      startx = x;
      starty = y;
    } else {
      // only start drawing between the first 2 points
      drawFn(&gfx, lx, ly, x, y);
    }
    lx = x;
    ly = y;
    idx++;
  }
  jsvIteratorFree(&it);
  gfx.data.cursorX = (short)(lx/scale);
  gfx.data.cursorY = (short)(ly/scale);
  // if closed, draw between first and last points
  if (closed)
    drawFn(&gfx, lx, ly, startx, starty);

  graphicsSetVar(&gfx); // gfx data changed because modified area
  return jsvLockAgain(parent);
}

/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "fillPoly",
  "ifndef" : "SAVE_ON_FLASH",
  "generate_full" : "jswrap_graphics_fillPoly_X(parent, poly, false);",
  "params" : [
    ["poly","JsVar","An array of vertices, of the form ```[x1,y1,x2,y2,x3,y3,etc]```"]
  ],
  "return" : ["JsVar","The instance of Graphics this was called on, to allow call chaining"],
  "return_object" : "Graphics",
  "typescript" : "fillPoly(poly: number[]): Graphics;"
}
Draw a filled polygon in the current foreground color.

```
g.fillPoly([
  16, 0,
  31, 31,
  26, 31,
  16, 12,
  6, 28,
  0, 27 ]);
```

This fills from the top left hand side of the polygon (low X, low Y) *down to
but not including* the bottom right. When placed together polygons will align
perfectly without overdraw - but this will not fill the same pixels as
`drawPoly` (drawing a line around the edge of the polygon).

**Note:** there is a limit of 64 points (128 XY elements) for polygons
*/
/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "fillPolyAA",
  "ifdef" : "GRAPHICS_ANTIALIAS",
  "generate_full" : "jswrap_graphics_fillPoly_X(parent, poly, true);",
  "params" : [
    ["poly","JsVar","An array of vertices, of the form ```[x1,y1,x2,y2,x3,y3,etc]```"]
  ],
  "return" : ["JsVar","The instance of Graphics this was called on, to allow call chaining"],
  "return_object" : "Graphics",
  "typescript" : "fillPolyAA(poly: number[]): Graphics;"
}
Draw a filled polygon in the current foreground color.

```
g.fillPolyAA([
  16, 0,
  31, 31,
  26, 31,
  16, 12,
  6, 28,
  0, 27 ]);
```

This fills from the top left hand side of the polygon (low X, low Y) *down to
but not including* the bottom right. When placed together polygons will align
perfectly without overdraw - but this will not fill the same pixels as
`drawPoly` (drawing a line around the edge of the polygon).

**Note:** there is a limit of 64 points (128 XY elements) for polygons
*/
JsVar *jswrap_graphics_fillPoly_X(JsVar *parent, JsVar *poly, bool antiAlias) {
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return 0;
  if (!jsvIsIterable(poly)) return 0;
  const int maxVerts = 128;
  short verts[maxVerts];
  int idx = 0;
  JsvIterator it;
  jsvIteratorNew(&it, poly, JSIF_EVERY_ARRAY_ELEMENT);
  while (jsvIteratorHasElement(&it) && idx<maxVerts) {
    int v = (int)(0.5 + jsvIteratorGetFloatValue(&it)*16);
    if (v<-32768) v=-32768;
    if (v>32767) v=32767;
    verts[idx++] = (short)v;
    jsvIteratorNext(&it);
  }
  if (jsvIteratorHasElement(&it))
    jsExceptionHere(JSET_ERROR, "Maximum number of points (%d) exceeded for fillPoly", maxVerts/2);
  jsvIteratorFree(&it);
#ifdef GRAPHICS_ANTIALIAS
  // For antialiased fillPoly the easiest solution is just to draw AA lines
  // around the edge first, then fill solidly
  if (antiAlias) {
    int lx = verts[idx-2];
    int ly = verts[idx-1];
    for (int i=0;i<idx;i+=2) {
      int vx = verts[i];
      int vy = verts[i+1];
      graphicsDrawLineAA(&gfx, vx,vy, lx,ly);
      lx = vx;
      ly = vy;
    }
  }
#endif
  graphicsFillPoly(&gfx, idx/2, verts);

  graphicsSetVar(&gfx); // gfx data changed because modified area
  return jsvLockAgain(parent);
}

/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "setRotation",
  "generate" : "jswrap_graphics_setRotation",
  "params" : [
    ["rotation","int32","The clockwise rotation. 0 for no rotation, 1 for 90 degrees, 2 for 180, 3 for 270"],
    ["reflect","bool","Whether to reflect the image"]
  ],
  "return" : ["JsVar","The instance of Graphics this was called on, to allow call chaining"],
  "return_object" : "Graphics",
  "typescript" : "setRotation(rotation: 0 | 1 | 2 | 3, reflect?: boolean): Graphics;"
}
Set the current rotation of the graphics device.
*/
JsVar *jswrap_graphics_setRotation(JsVar *parent, int rotation, bool reflect) {
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return 0;

  // clear flags
  gfx.data.flags &= (JsGraphicsFlags)~(JSGRAPHICSFLAGS_SWAP_XY | JSGRAPHICSFLAGS_INVERT_X | JSGRAPHICSFLAGS_INVERT_Y);
  // set flags
  switch (rotation) {
    case 0:
      break;
    case 1:
      gfx.data.flags |= JSGRAPHICSFLAGS_SWAP_XY | JSGRAPHICSFLAGS_INVERT_X;
      break;
    case 2:
      gfx.data.flags |= JSGRAPHICSFLAGS_INVERT_X | JSGRAPHICSFLAGS_INVERT_Y;
      break;
    case 3:
      gfx.data.flags |= JSGRAPHICSFLAGS_SWAP_XY | JSGRAPHICSFLAGS_INVERT_Y;
      break;
  }

  if (reflect) {
    if (gfx.data.flags & JSGRAPHICSFLAGS_SWAP_XY)
      gfx.data.flags ^= JSGRAPHICSFLAGS_INVERT_Y;
    else
      gfx.data.flags ^= JSGRAPHICSFLAGS_INVERT_X;
  }

  graphicsSetVar(&gfx);
  return jsvLockAgain(parent);
}

/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "imageMetrics",
  "generate" : "jswrap_graphics_imageMetrics",
  "params" : [
    ["str","JsVar","The string"]
  ],
  "return" : ["JsVar","An object containing `{width,height,bpp,transparent}` for the image"],
  "typescript" : "imageMetrics(img: Image): { width: number, height: number, bpp: number, transparent: number, frames?: ArrayBuffer[] } | undefined;"
}
Return the width and height in pixels of an image (either Graphics, Image
Object, Image String or ArrayBuffer). Returns `undefined` if image couldn't be
decoded.

`frames` is also included is the image contains more information than you'd
expect for a single bitmap. In this case the bitmap might be an animation with
multiple frames
*/
JsVar *jswrap_graphics_imageMetrics(JsVar *parent, JsVar *var) {
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return 0;
  GfxDrawImageInfo img;
  if (!_jswrap_graphics_parseImage(&gfx, var, 0, &img))
    return 0;
  int bufferLen = jsvGetLength(img.buffer) - img.bitmapOffset;
  _jswrap_graphics_freeImageInfo(&img);
  JsVar *o = jsvNewObject();
  if (o) {
    jsvObjectSetChildAndUnLock(o, "width", jsvNewFromInteger(img.width));
    jsvObjectSetChildAndUnLock(o, "height", jsvNewFromInteger(img.height));
    jsvObjectSetChildAndUnLock(o, "bpp", jsvNewFromInteger(img.bpp));
    jsvObjectSetChildAndUnLock(o, "transparent", jsvNewFromBool(img.isTransparent));
    int frames = bufferLen / img.bitmapLength;
    if (frames>1) jsvObjectSetChildAndUnLock(o, "frames", jsvNewFromInteger(frames));
  }
  return o;
}

/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "drawImage",
  "generate" : "jswrap_graphics_drawImage",
  "params" : [
    ["image","JsVar","An image to draw, either a String or an Object (see below)"],
    ["x","int32","The X offset to draw the image"],
    ["y","int32","The Y offset to draw the image"],
    ["options","JsVar","options for scaling,rotation,etc (see below)"]
  ],
  "return" : ["JsVar","The instance of Graphics this was called on, to allow call chaining"],
  "return_object" : "Graphics",
  "typescript" : "drawImage(image: Image, x: number, y: number, options?: { rotate?: number, scale?: number, frame?: number }): Graphics;"
}
Image can be:

* An object with the following fields `{ width : int, height : int, bpp :
  optional int, buffer : ArrayBuffer/String, transparent: optional int,
  palette : optional Uint16Array(2/4/16) }`. bpp = bits per pixel (default is
  1), transparent (if defined) is the colour that will be treated as
  transparent, and palette is a color palette that each pixel will be looked up
  in first
* A String where the the first few bytes are:
  `width,height,bpp,[transparent,]image_bytes...`. If a transparent colour is
  specified the top bit of `bpp` should be set.
* An ArrayBuffer Graphics object (if `bpp<8`, `msb:true` must be set) - this is
  disabled on devices without much flash memory available. If a Graphics object
  is supplied, it can also contain transparent/palette fields as if it were
  an image.

Draw an image at the specified position.

* If the image is 1 bit, the graphics foreground/background colours will be
  used.
* If `img.palette` is a Uint16Array or 2/4/16 elements, color data will be
  looked from the supplied palette
* On Bangle.js, 2 bit images blend from background(0) to foreground(1) colours
* On Bangle.js, 4 bit images use the Apple Mac 16 color palette
* On Bangle.js, 8 bit images use the Web Safe 216 color palette
* Otherwise color data will be copied as-is. Bitmaps are rendered MSB-first

If `options` is supplied, `drawImage` will allow images to be rendered at any
scale or angle. If `options.rotate` is set it will center images at `x,y`.
`options` must be an object of the form:

```
{
  rotate : float, // the amount to rotate the image in radians (default 0)
  scale : float, // the amount to scale the image up (default 1)
  frame : int    // if specified and the image has frames of data
                 //  after the initial frame, draw one of those frames from the image
  filter : bool  // (2v19+) when set, if scale<0.75 perform 2x2 supersampling to smoothly downscale the image
}
```

For example:

```
// In the top left of the screen
g.drawImage(img,0,0);
// In the top left of the screen, twice as big
g.drawImage(img,0,0,{scale:2});
// In the center of the screen, twice as big, 45 degrees
g.drawImage(img, g.getWidth()/2, g.getHeight()/2,
            {scale:2, rotate:Math.PI/4});
```
*/
JsVar *jswrap_graphics_drawImage(JsVar *parent, JsVar *image, int xPos, int yPos, JsVar *options) {
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return 0;
  GfxDrawImageInfo img;
  if (!_jswrap_graphics_parseImage(&gfx, image, 0, &img))
    return 0;


  double scale = 1, rotate = 0;
  bool centerImage = false;
#ifndef SAVE_ON_FLASH
  bool filter = false;
#endif
  if (jsvIsObject(options)) {
    // support for multi-frame rendering
    int frame = jsvGetIntegerAndUnLock(jsvObjectGetChildIfExists(options,"frame"));
    if (frame>0)
      img.bitmapOffset += img.bitmapLength * frame;
    // rotate, scale
    scale = jsvGetFloatAndUnLock(jsvObjectGetChildIfExists(options,"scale"));
    if (!isfinite(scale) || scale<=0) scale=1;
    rotate = jsvGetFloatAndUnLock(jsvObjectGetChildIfExists(options,"rotate"));
    centerImage = isfinite(rotate);
    if (!centerImage) rotate = 0;
#ifndef SAVE_ON_FLASH
    filter = jsvGetBoolAndUnLock(jsvObjectGetChildIfExists(options,"filter"));
#endif
  }

  int x=0, y=0;
  unsigned int colData = 0;
  JsvStringIterator it;
  jsvStringIteratorNew(&it, img.buffer, (size_t)img.bitmapOffset);

#ifdef USE_LCD_ST7789_8BIT // can we blit directly to the display?
  bool isST7789 =
        gfx.data.type==JSGRAPHICSTYPE_ST7789_8BIT && // it's the display
        (gfx.data.flags & JSGRAPHICSFLAGS_MAPPEDXY)==0 && // no messing with coordinates
        gfx.data.bpp==16 && // normal BPP
        (img.bpp==8 || img.bpp==1) && // image bpp is handled by fast path
        !img.isTransparent; // not transparent
#endif

  if (scale==1 && rotate==0 && !centerImage) {
    // Standard 1:1 blitting
#ifdef USE_LCD_ST7789_8BIT // can we blit directly to the display?
    if (isST7789 &&
        xPos>=gfx.data.clipRect.x1 && yPos>=gfx.data.clipRect.y1 && // check it's all on-screen
        (xPos+img.width)<=gfx.data.clipRect.x2+1 && (yPos+img.height)<=gfx.data.clipRect.y2+1) {
      if (img.bpp==1) lcdST7789_blit1Bit(xPos, yPos, img.width, img.height, 1, &it, img.palettePtr);
      else if (img.bpp==8) lcdST7789_blit8Bit(xPos, yPos, img.width, img.height, 1, &it, img.palettePtr);
    } else {
#else
    {
#endif
      _jswrap_drawImageSimple(&gfx, xPos, yPos, &img, &it, false/*don't care about string iterator now*/);
    }
  } else {
#ifndef GRAPHICS_DRAWIMAGE_ROTATED
    jsExceptionHere(JSET_ERROR,"Image scale/rotate not implemented on this device");
#else
    // fancy rotation/scaling

#ifdef GRAPHICS_FAST_PATHS
    bool fastPath =
        (!centerImage) &&  // not rotating
        (scale-floor(scale))==0 && // integer scale
        (gfx.data.flags & JSGRAPHICSFLAGS_MAPPEDXY)==0; // no messing with coordinates
    if (fastPath) { // fast path for non-rotated, integer scale
      int s = (int)scale;
      // Scaled blitting
      /* Output as scanlines rather than small fillrects so
       * that on direct-coupled displays we can optimise away
       * coordinate setting
       */
#ifdef USE_LCD_ST7789_8BIT // can we blit directly to the display?
      if (isST7789 &&
          s>=1 &&
          xPos>=gfx.data.clipRect.x1 && yPos>=gfx.data.clipRect.y1 && // check it's all on-screen
          (xPos+img.width*s)<=gfx.data.clipRect.x2+1 && (yPos+img.height*s)<=gfx.data.clipRect.y2+1) {
        if (img.bpp==1) lcdST7789_blit1Bit(xPos, yPos, img.width, img.height, s, &it, img.palettePtr);
        else lcdST7789_blit8Bit(xPos, yPos, img.width, img.height, s, &it, img.palettePtr);
      } else
#endif
      {
        int bits=0;
        int yp = yPos;
        for (y=0;y<img.height;y++) {
          // Store current pos as we need to rewind
          size_t lastIt = jsvStringIteratorGetIndex(&it);
          int lastBits = bits;
          unsigned int lastColData = colData;
          // do a new iteration for each line we're scaling
          for (int iy=0;iy<s;iy++) {
            if (iy) { // rewind for all but the first line of scaling
              jsvStringIteratorGoto(&it, img.buffer, lastIt);
              bits = lastBits;
              colData = lastColData;
            }
            // iterate over x
            int xp = xPos;
            for (x=0;x<img.width;x++) {
              // Get the data we need...
              while (bits < img.bpp) {
                colData = (colData<<8) | ((unsigned char)jsvStringIteratorGetCharAndNext(&it));
                bits += 8;
              }
              // extract just the bits we want
              unsigned int col = (colData>>(bits-img.bpp))&img.bitMask;
              bits -= img.bpp;
              // Try and write pixel!
              if (img.transparentCol!=col && yp>=gfx.data.clipRect.y1 && yp<=gfx.data.clipRect.y2) {
                if (img.palettePtr) col = img.palettePtr[col&img.paletteMask];
                for (int ix=0;ix<s;ix++) {
                  if (xp>=gfx.data.clipRect.x1 && xp<=gfx.data.clipRect.x2)
                    gfx.setPixel(&gfx, xp, yp, col);
                  xp++;
                }
              } else xp += s;
            }
            yp++;
          }
        }
        // update modified area since we went direct
        int x1=xPos, y1=yPos, x2=xPos+s*img.width, y2=yPos+s*img.height;
        graphicsSetModifiedAndClip(&gfx,&x1,&y1,&x2,&y2);
      }
    } else { // handle rotation, and default to center the image
#else
    if (true) {
#endif // GRAPHICS_FAST_PATHS
      GfxDrawImageLayer l;
      l.x1 = xPos;
      l.y1 = yPos;
      l.img = img;
      l.it = it;
      l.rotate = rotate;
      l.scale = scale;
      l.center = centerImage;
      l.repeat = false;
      _jswrap_drawImageLayerInit(&l);
      int x1=l.x1, y1=l.y1, x2=l.x2-1, y2=l.y2-1;
      graphicsSetModifiedAndClip(&gfx, &x1, &y1, &x2, &y2);
      _jswrap_drawImageLayerSetStart(&l, x1, y1);
      JsGraphicsSetPixelFn setPixel = graphicsGetSetPixelFn(&gfx);

#ifndef SAVE_ON_FLASH
      if (filter && scale<0.75) { // 2x2 antialiasing
        int sx = (int)(l.sx * scale); // use scale rather than 0.5, so if scaling dithered it still works nicely
        int sy = (int)(l.sy * scale);
        int s2x = l.sx - sx; // sx+s2x = l.sx
        int s2y = l.sy - sy;
        GfxDrawImageLayer l2;
        memcpy(&l2, &l, sizeof(l));
        jsvStringIteratorNew(&l2.it, l2.img.buffer, 0);
        l2.px += sy;
        l2.py += sx;
         _jswrap_drawImageLayerNextY(&l2);
         // scan across image
        for (y = y1; y <= y2; y++) {
          _jswrap_drawImageLayerStartX(&l);
          _jswrap_drawImageLayerStartX(&l2);
          for (x = x1; x <= x2 ; x++) {
            uint32_t ca,cb,cc,cd;
            bool nonTransparent = true;
            nonTransparent &= _jswrap_drawImageLayerGetPixel(&l, &ca);
            l.qx += sx;
            l.qy -= sy;
            nonTransparent &= _jswrap_drawImageLayerGetPixel(&l, &cb);
            l.qx += s2x;
            l.qy -= s2y;
            nonTransparent &= _jswrap_drawImageLayerGetPixel(&l2, &cc);
            l2.qx += sx;
            l2.qy -= sy;
            nonTransparent &= _jswrap_drawImageLayerGetPixel(&l2, &cd);
            l2.qx += s2x;
            l2.qy -= s2y;
            if (true) {
              ca = graphicsBlendColor(&gfx, ca, cb, 128);
              cc = graphicsBlendColor(&gfx, cc, cd, 128);
              colData = graphicsBlendColor(&gfx, ca, cc, 128);
              setPixel(&gfx, x, y, colData);
            }
          }
          _jswrap_drawImageLayerNextY(&l);
          _jswrap_drawImageLayerNextY(&l2);
        }
        jsvStringIteratorFree(&l2.it);
      } else
#endif
      {
        // scan across image
        for (y = y1; y <= y2; y++) {
          _jswrap_drawImageLayerStartX(&l);
          for (x = x1; x <= x2 ; x++) {
            if (_jswrap_drawImageLayerGetPixel(&l, &colData)) {
              setPixel(&gfx, x, y, colData);
            }
            _jswrap_drawImageLayerNextX(&l);
          }
          _jswrap_drawImageLayerNextY(&l);
        }
      }
      it = l.it; // make sure it gets freed properly
    }
#endif // GRAPHICS_DRAWIMAGE_ROTATED
  }
  jsvStringIteratorFree(&it);
  _jswrap_graphics_freeImageInfo(&img);
  graphicsSetVar(&gfx); // gfx data changed because modified area
  return jsvLockAgain(parent);
}




/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "drawImages",
  "#if" : "defined(BANGLEJS) || defined(LINUX)",
  "generate" : "jswrap_graphics_drawImages",
  "params" : [
    ["layers","JsVar","An array of objects {x,y,image,scale,rotate,center} (up to 3)"],
    ["options","JsVar","options for rendering - see below"]
  ],
  "return" : ["JsVar","The instance of Graphics this was called on, to allow call chaining"],
  "return_object" : "Graphics",
  "typescript" : "drawImages(layers: { x: number, y: number, image: Image, scale?: number, rotate?: number, center?: boolean, repeat?: boolean, nobounds?: boolean }[], options?: { x: number, y: number, width: number, height: number }): Graphics;"
}
Draws multiple images *at once* - which avoids flicker on unbuffered systems
like Bangle.js. Maximum layer count right now is 4.

```
layers = [ {
  {x : int, // x start position
   y : int, // y start position
   image : string/object,
   scale : float, // scale factor, default 1
   rotate : float, // angle in radians
   center : bool // center on x,y? default is top left
   repeat : should this image be repeated (tiled?)
   nobounds : bool // if true, the bounds of the image are not used to work out the default area to draw
  }
]
options = { // the area to render. Defaults to rendering just enough to cover what's requested
 x,y,
 width,height
}
```
*/
JsVar *jswrap_graphics_drawImages(JsVar *parent, JsVar *layersVar, JsVar *options) {
  const int MAXIMAGES = 4;
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return 0;
  GfxDrawImageLayer layers[MAXIMAGES];
  int i,layerCount;
  if (!(jsvIsArray(layersVar) &&  (layerCount=jsvGetArrayLength(layersVar))<=MAXIMAGES)) {
    jsExceptionHere(JSET_TYPEERROR,"Expecting Array for first argument with <%d entries", MAXIMAGES);
    return 0;
  }
  // default bounds (=nothing)
  int x=10000,y=10000;
  int width=10000;
  int height=10000;
  // now run through all layers getting stuff ready and checking
  bool ok = true;
  for (i=0;i<layerCount;i++) {
    JsVar *layer = jsvGetArrayItem(layersVar, i);
    if (jsvIsObject(layer)) {
      JsVar *image = jsvObjectGetChildIfExists(layer,"image");
      if (_jswrap_graphics_parseImage(&gfx, image, 0, &layers[i].img)) {
        layers[i].x1 = jsvGetIntegerAndUnLock(jsvObjectGetChildIfExists(layer,"x"));
        layers[i].y1 = jsvGetIntegerAndUnLock(jsvObjectGetChildIfExists(layer,"y"));
        // rotate, scale
        layers[i].scale = jsvGetFloatAndUnLock(jsvObjectGetChildIfExists(layer,"scale"));
        if (!isfinite(layers[i].scale) || layers[i].scale<=0)
          layers[i].scale=1;
        layers[i].rotate = jsvGetFloatAndUnLock(jsvObjectGetChildIfExists(layer,"rotate"));
        if (!isfinite(layers[i].rotate)) layers[i].rotate=0;
        layers[i].center = jsvGetBoolAndUnLock(jsvObjectGetChildIfExists(layer,"center"));
        layers[i].repeat = jsvGetBoolAndUnLock(jsvObjectGetChildIfExists(layer,"repeat"));
        _jswrap_drawImageLayerInit(&layers[i]);
        // add the calculated bounds to our default bounds
        if (!jsvGetBoolAndUnLock(jsvObjectGetChildIfExists(layer,"nobounds"))) {
          if (layers[i].x1<x) x=layers[i].x1;
          if (layers[i].y1<y) y=layers[i].y1;
          if (layers[i].x2>x+width) width=layers[i].x2-x;
          if (layers[i].y2>y+height) height=layers[i].y2-y;
        }
      } else ok = false;
      jsvUnLock(image);
    } else ok = false;
    jsvUnLock(layer);
  }

  jsvConfigObject configs[] = {
      {"x", JSV_INTEGER, &x},
      {"y", JSV_INTEGER, &y},
      {"width", JSV_INTEGER, &width},
      {"height", JSV_INTEGER, &height}
  };
  if (!jsvReadConfigObject(options, configs, sizeof(configs) / sizeof(jsvConfigObject)))
    ok =  false;
  int x2 = x+width-1, y2 = y+height-1;
  graphicsSetModifiedAndClip(&gfx, &x, &y, &x2, &y2);
  JsGraphicsSetPixelFn setPixel = graphicsGetSetPixelFn(&gfx);

  // If all good, start rendering!
  if (ok) {
    for (i=0;i<layerCount;i++) {
      jsvStringIteratorNew(&layers[i].it, layers[i].img.buffer, (size_t)layers[i].img.bitmapOffset);
      _jswrap_drawImageLayerSetStart(&layers[i], x, y);
    }
    // scan across image
    for (int yi = y; yi <= y2; yi++) {
      for (i=0;i<layerCount;i++)
        _jswrap_drawImageLayerStartX(&layers[i]);
      for (int xi = x; xi <= x2 ; xi++) {
        // scan backwards until we hit a 'solid' pixel
        bool solid = false;
        unsigned int colData = 0;
        for (i=layerCount-1;i>=0;i--) {
          if (_jswrap_drawImageLayerGetPixel(&layers[i], &colData)) {
            solid = true;
            break;
          }
        }
        // if nontransparent, draw it!
        if (solid)
          setPixel(&gfx, xi, yi, colData);
        // next in layers!
        for (i=0;i<layerCount;i++) {
          _jswrap_drawImageLayerNextX(&layers[i]);
          _jswrap_drawImageLayerNextXRepeat(&layers[i]);
        }
      }
      for (i=0;i<layerCount;i++)
        _jswrap_drawImageLayerNextY(&layers[i]);
    }
    for (i=0;i<layerCount;i++)
      jsvStringIteratorFree(&layers[i].it);
  }
  // tidy up
  for (i=0;i<layerCount;i++) {
    jsvUnLock(layers[i].img.buffer);
  }
  return jsvLockAgain(parent);
}


/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "asImage",
  "ifndef" : "SAVE_ON_FLASH",
  "generate" : "jswrap_graphics_asImage",
  "params" : [
    ["type","JsVar","The type of image to return. Either `object`/undefined to return an image object, or `string` to return an image string"]
  ],
  "return" : ["JsVar","An Image that can be used with `Graphics.drawImage`"],
  "typescript" : [
    "asImage(type?: \"object\"): ImageObject;",
    "asImage(type: \"string\"): string;"
  ]
}
Return this Graphics object as an Image that can be used with
`Graphics.drawImage`. Check out [the Graphics reference
page](http://www.espruino.com/Graphics#images-bitmaps) for more information on
images.

Will return undefined if data can't be allocated for the image.

The image data itself will be referenced rather than copied if:

* An image `object` was requested (not `string`)
* The Graphics instance was created with `Graphics.createArrayBuffer`
* Is 8 bpp *OR* the `{msb:true}` option was given
* No other format options (zigzag/etc) were given

Otherwise data will be copied, which takes up more space and may be quite slow.

If the `Graphics` object contains `transparent` or `pelette` fields,
[as you might find in an image](http://www.espruino.com/Graphics#images-bitmaps),
those will be included in the generated image too.
*/
JsVar *jswrap_graphics_asImage(JsVar *parent, JsVar *imgType) {
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return 0;
  bool isObject;
  if (jsvIsUndefined(imgType) || jsvIsStringEqual(imgType,"object"))
    isObject = true;
  else if (jsvIsStringEqual(imgType,"string")) {
    isObject = false;
  } else {
    jsExceptionHere(JSET_ERROR, "Unknown image type %j", imgType);
    return 0;
  }
  int w = jswrap_graphics_getWidthOrHeight(parent,false);
  int h = jswrap_graphics_getWidthOrHeight(parent,true);
  int bpp = gfx.data.bpp;
  int colorCount = (1<<bpp);
#ifdef LCD_CONTROLLER_LPM013M126
  // memory LCD reports bit depth as 16 so it can do dithering
  // but when we get an image we only want it the real bit depth (3!)
  if (gfx.data.type==JSGRAPHICSTYPE_MEMLCD) bpp=3;
#endif
  int len = (w*h*bpp+7)>>3;
  int transparent = -1;
  JsVar *v = jsvObjectGetChildIfExists(parent, "transparent");
  if (v) transparent = jsvGetIntegerAndUnLock(v);
  JsVar *palette = jsvObjectGetChildIfExists(parent, "palette");
  if (!(jsvIsArray(palette) || jsvIsArrayBuffer(palette))) {
    jsvUnLock(palette);
    palette = 0;
  }

  JsVar *img = 0;
  if (isObject) {
    img = jsvNewObject();
    if (!img) return 0;
    jsvObjectSetChildAndUnLock(img,"width",jsvNewFromInteger(w));
    jsvObjectSetChildAndUnLock(img,"height",jsvNewFromInteger(h));
    if (bpp!=1) jsvObjectSetChildAndUnLock(img,"bpp",jsvNewFromInteger(bpp));
    /* IF we have an arraybuffer of the right form then
    we can return the original buffer directly */
    if (gfx.data.type == JSGRAPHICSTYPE_ARRAYBUFFER &&
        (bpp==8 || // 8 bit data is fine
        ((gfx.data.flags & JSGRAPHICSFLAGS_ARRAYBUFFER_MSB) && // must be MSB first
          !(gfx.data.flags & JSGRAPHICSFLAGS_NONLINEAR)))) { // must be in-order
      jsvObjectSetChildAndUnLock(img,"buffer",jsvObjectGetChildIfExists(gfx.graphicsVar, "buffer"));
      return img;
    }
    if (transparent>=0)
      jsvObjectSetChildAndUnLock(img,"transparent",jsvNewFromInteger(transparent));
    if (palette) jsvObjectSetChild(img,"palette",palette);
  } else {

    len += 3 + ((transparent>=0)?1:0) + (palette?(colorCount*2):0); // for the header!
  }
  JsVar *buffer = jsvNewStringOfLength((unsigned)len, NULL);
  if (!buffer) { // not enough memory
    jsvUnLock(img);
    return 0;
  }
  int x=0, y=0;
  unsigned int pixelBits = 0;
  unsigned int pixelBitCnt = 0;
  JsvStringIterator it;
  jsvStringIteratorNew(&it, buffer, 0);
  if (!isObject) { // if not an object, add the header
    // info on image format at http://www.espruino.com/Graphics#images-bitmaps
    jsvStringIteratorSetCharAndNext(&it, (char)(uint8_t)w);
    jsvStringIteratorSetCharAndNext(&it, (char)(uint8_t)h);
    jsvStringIteratorSetCharAndNext(&it, (char)(uint8_t)(bpp | ((transparent>=0)?128:0)) | (palette?64:0));
    if (transparent>=0)
      jsvStringIteratorSetCharAndNext(&it, (char)(uint8_t)transparent);
    if (palette) {
      JsvIterator pit;
      int colorIdx = 0;
      jsvIteratorNew(&pit, palette, JSIF_EVERY_ARRAY_ELEMENT);
      while (jsvIteratorHasElement(&pit)) {
        int colorValue = jsvIteratorGetIntegerValue(&pit);
        if (colorIdx<colorCount) {
          jsvStringIteratorSetCharAndNext(&it, (char)(uint8_t)(colorValue>>8));
          jsvStringIteratorSetCharAndNext(&it, (char)(uint8_t)colorValue);
        }
        jsvIteratorNext(&pit);
        colorIdx++;
      }
      jsvIteratorFree(&pit);
      while (colorIdx<colorCount) {
        jsvStringIteratorSetCharAndNext(&it, 0);
        jsvStringIteratorSetCharAndNext(&it, 0);
        colorIdx++;
      }
    }
  }
  while (jsvStringIteratorHasChar(&it)) {
    unsigned int pixel = graphicsGetPixel(&gfx, x, y);
#ifdef LCD_CONTROLLER_LPM013M126
    if (gfx.data.type==JSGRAPHICSTYPE_MEMLCD)
      pixel = GRAPHICS_COL_16_TO_3(pixel);
#endif
    pixelBits = (pixelBits<<bpp) | pixel;
    pixelBitCnt += (unsigned)bpp;
    x++;
    if (x>=w) {
      x=0;
      y++;
    }
    while (pixelBitCnt>=8) {
      jsvStringIteratorSetCharAndNext(&it, (char)(pixelBits>>(pixelBitCnt-8)));
      pixelBitCnt -= 8;
    }
  }
  jsvStringIteratorFree(&it);
  jsvUnLock(palette);
  if (isObject) {
    jsvObjectSetChildAndUnLock(img,"buffer",buffer);
    return img;
  } else
    return buffer;
}

/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "getModified",
  "ifndef" : "SAVE_ON_FLASH",
  "generate" : "jswrap_graphics_getModified",
  "params" : [
    ["reset","bool","Whether to reset the modified area or not"]
  ],
  "return" : ["JsVar","An object {x1,y1,x2,y2} containing the modified area, or undefined if not modified"],
  "typescript" : "getModified(reset?: boolean): { x1: number, y1: number, x2: number, y2: number };"
}
Return the area of the Graphics canvas that has been modified, and optionally
clear the modified area to 0.

For instance if `g.setPixel(10,20)` was called, this would return `{x1:10,
y1:20, x2:10, y2:20}`
*/
JsVar *jswrap_graphics_getModified(JsVar *parent, bool reset) {
#ifndef NO_MODIFIED_AREA
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return 0;
  JsVar *obj = 0;
  if (gfx.data.modMinX <= gfx.data.modMaxX) { // do we have a rect?
    obj = jsvNewObject();
    if (obj) {
      jsvObjectSetChildAndUnLock(obj, "x1", jsvNewFromInteger(gfx.data.modMinX));
      jsvObjectSetChildAndUnLock(obj, "y1", jsvNewFromInteger(gfx.data.modMinY));
      jsvObjectSetChildAndUnLock(obj, "x2", jsvNewFromInteger(gfx.data.modMaxX));
      jsvObjectSetChildAndUnLock(obj, "y2", jsvNewFromInteger(gfx.data.modMaxY));
    }
  }
  if (reset) {
    gfx.data.modMaxX = -32768;
    gfx.data.modMaxY = -32768;
    gfx.data.modMinX = 32767;
    gfx.data.modMinY = 32767;
    graphicsSetVar(&gfx);
  }
  return obj;
#else
  return 0;
#endif
}

/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "scroll",
  "#if" : "!defined(SAVE_ON_FLASH) && !defined(ESPRUINOBOARD)",
  "generate" : "jswrap_graphics_scroll",
  "params" : [
    ["x","int32","X direction. >0 = to right"],
    ["y","int32","Y direction. >0 = down"]
  ],
  "return" : ["JsVar","The instance of Graphics this was called on, to allow call chaining"],
  "return_object" : "Graphics"
}
Scroll the contents of this graphics in a certain direction. The remaining area
is filled with the background color.

Note: This uses repeated pixel reads and writes, so will not work on platforms
that don't support pixel reads.
*/
JsVar *jswrap_graphics_scroll(JsVar *parent, int xdir, int ydir) {
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return 0;
  graphicsScroll(&gfx, xdir, ydir);
  // update modified area
  graphicsSetVar(&gfx);
  return jsvLockAgain(parent);
}

/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "blit",
  "#if" : "!defined(SAVE_ON_FLASH) && !defined(ESPRUINOBOARD)",
  "generate" : "jswrap_graphics_blit",
  "params" : [
    ["options","JsVar","options - see below"]
  ],
  "return" : ["JsVar","The instance of Graphics this was called on, to allow call chaining"],
  "return_object" : "Graphics",
  "typescript" : "blit(options: { x1: number, y1: number, x2: number, y2: number, w: number, h: number, setModified?: boolean }): Graphics;"
}
Blit one area of the screen (x1,y1 w,h) to another (x2,y2 w,h)

```
g.blit({
  x1:0, y1:0,
  w:32, h:32,
  x2:100, y2:100,
  setModified : true // should we set the modified area?
});
```

Note: This uses repeated pixel reads and writes, so will not work on platforms
that don't support pixel reads.
*/
JsVar *jswrap_graphics_blit(JsVar *parent, JsVar *options) {
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return 0;
  int sw = gfx.data.width;
  int sh = gfx.data.height;
  int x1=0,y1=0,w=0,h=0,x2=0,y2=0;
  bool setModified = false;
  jsvConfigObject configs[] = {
      {"x1", JSV_INTEGER, &x1},
      {"y1", JSV_INTEGER, &y1},
      {"w", JSV_INTEGER, &w},
      {"h", JSV_INTEGER, &h},
      {"x2", JSV_INTEGER, &x2},
      {"y2", JSV_INTEGER, &y2},
      {"setModified", JSV_BOOLEAN, &setModified}};
  if (!jsvIsObject(options) ||
      !jsvReadConfigObject(options, configs, sizeof(configs) / sizeof(jsvConfigObject))) {
    jsExceptionHere(JSET_ERROR, "Invalid options");
    return 0;
  }
  int ex;
  // clip source positions
  if (x1<0) {
    x2 -= x1;
    w += x1;
    x1 = 0;
  }
  if (y1<0) {
    y2 -= y1;
    h += y1;
    y1 = 0;
  }
  ex = (x1+w) - sw;
  if (ex > 0) w -= ex;
  ex = (y1+h) - sh;
  if (ex > 0) h -= ex;
  // clip destination positions
  if (x2<0) {
    x1 -= x2;
    w += x2;
    x2 = 0;
  }
  if (y2<0) {
    y1 -= y2;
    h += y2;
    y2 = 0;
  }
  ex = (x2+w) - sw;
  if (ex > 0) w -= ex;
  ex = (y2+h) - sh;
  if (ex > 0) h -= ex;
  if (w>0 || h>0) {
    gfx.blit(&gfx, x1,y1,w,h,x2,y2);
    if (setModified) {
      graphicsSetModified(&gfx, x2,y2,x2+w,y2+h);
      graphicsSetVar(&gfx);
    }
  }
  return jsvLockAgain(parent);
}

/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "asBMP",
  "#if" : "!defined(SAVE_ON_FLASH) && !defined(ESPRUINOBOARD)",
  "generate" : "jswrap_graphics_asBMP",
  "return" : ["JsVar","A String representing the Graphics as a Windows BMP file (or 'undefined' if not possible)"],
  "typescript" : "asBMP(): string;"
}
Create a Windows BMP file from this Graphics instance, and return it as a
String.
*/
JsVar *jswrap_graphics_asBMP_X(JsVar *parent, bool printBase64) {
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return 0;
  int width = graphicsGetWidth(&gfx);
  int height = graphicsGetHeight(&gfx);
  int realBPP = gfx.data.bpp;
#ifdef LCD_CONTROLLER_LPM013M126
  // memory LCD reports bit depth as 16 so it can do dithering
  // but when we get a bitmap we only want it the real bit depth (3!)
  if (gfx.data.type==JSGRAPHICSTYPE_MEMLCD) realBPP=3;
#endif
  int bpp = realBPP;
  if (bpp>1 && bpp<4) bpp=4;
  else if (bpp>4 && bpp<8) bpp=8;
  bool hasPalette = bpp<=8;
  int rowstride = (((width*bpp)+31) >> 5) << 2; // padded to 32 bits
  // palette length (byte size is 3x this)
  int paletteEntries = hasPalette?(1<<bpp):0;
  int headerLen = 14 + 12 + paletteEntries*3;
  int fileSize = headerLen + height*rowstride;
  // if printing base64 we only need enough memory for header + one row
  int imgDataLen = printBase64 ? (headerLen + rowstride) : fileSize;
  JsVar *imgData = jsvNewFlatStringOfLength((unsigned)imgDataLen);
  if (!imgData) return 0; // not enough memory
  unsigned char *imgPtr = (unsigned char *)jsvGetFlatStringPointer(imgData);
  imgPtr[0]=66; //B
  imgPtr[1]=77; //M
  imgPtr[2]=(unsigned char)fileSize;
  imgPtr[3]=(unsigned char)(fileSize>>8);  // plus 2 more bytes for size
  imgPtr[10]=(unsigned char)headerLen;
  // maybe we want the InfoHeader, not BITMAPCOREHEADER (http://www.ece.ualberta.ca/~elliott/ee552/studentAppNotes/2003_w/misc/bmp_file_format/bmp_file_format.htm)
  // Chrome doesn't like 16 bit BMPs in this format
  // BITMAPCOREHEADER
  imgPtr[14]=12; // sizeof(BITMAPCOREHEADER)
  imgPtr[18]=(unsigned char)width;
  imgPtr[19]=(unsigned char)(width>>8);
  imgPtr[20]=(unsigned char)height;
  imgPtr[21]=(unsigned char)(height>>8);
  imgPtr[22]=1; // color planes, should be 1
  imgPtr[24]=(unsigned char)bpp; // bpp
  if (hasPalette) {
    // palette starts at 26
    if (bpp==1) {
      // first is white(?)
      imgPtr[26]=255;
      imgPtr[27]=255;
      imgPtr[28]=255;
    } else {
      if (realBPP==3) {
        for (int i=0;i<paletteEntries;i++) {
          imgPtr[26 + (i*3)] = (i&1) ? 255 : 0;
          imgPtr[27 + (i*3)] = (i&2) ? 255 : 0;
          imgPtr[28 + (i*3)] = (i&4) ? 255 : 0;
        }
#if defined(GRAPHICS_PALETTED_IMAGES)
      } else if (realBPP==4) {
        for (int i=0;i<16;i++) {
          int p = PALETTE_4BIT[i];
          imgPtr[26 + (i*3)] = (unsigned char)((p<<3)&0xF8);
          imgPtr[27 + (i*3)] = (unsigned char)((p>>3)&0xFC);
          imgPtr[28 + (i*3)] = (unsigned char)((p>>8)&0xF8);
        }
      } else if (realBPP==8) {
        for (int i=0;i<255;i++) {
          int p = PALETTE_8BIT[i];
          imgPtr[26 + (i*3)] = (unsigned char)((p<<3)&0xF8);
          imgPtr[27 + (i*3)] = (unsigned char)((p>>3)&0xFC);
          imgPtr[28 + (i*3)] = (unsigned char)((p>>8)&0xF8);
        }
#endif
      } else { // otherwise default to greyscale
        for (int i=0;i<(1<<realBPP);i++) {
          unsigned char c = (unsigned char)(255 * i / (1<<realBPP));
          imgPtr[26 + (i*3)] = c;
          imgPtr[27 + (i*3)] = c;
          imgPtr[28 + (i*3)] = c;
        }
      }
    }
  }

  int pixelMask = (1<<bpp)-1;
  int pixelsPerByte = 8 / bpp;
  int idx = headerLen; //< index we're writing to our data at
  for (int y=height-1;y>=0;y--) {
    int yi = height-(y+1);
    int bytesWritten = 0;
    if (bpp<8) { // >1 pixels per byte
      for (int x=0;x<width;) {
        unsigned int b = 0;
        for (int i=0;i<pixelsPerByte;i++) {
          unsigned int pixel = graphicsGetPixel(&gfx, x++, y);
#ifdef LCD_CONTROLLER_LPM013M126
        if (gfx.data.type==JSGRAPHICSTYPE_MEMLCD)
          pixel = GRAPHICS_COL_16_TO_3(pixel);
#endif
          b = (b<<bpp)|(pixel&pixelMask);
        }
        imgPtr[idx++] = (unsigned char)b;
        bytesWritten++;
      }

    } else { // <= 1 pixel per byte
      for (int x=0;x<width;x++) {
        unsigned int c = graphicsGetPixel(&gfx, x, y);
        if (bpp==16) // 16 bit BMP is RGB555, not RGB565
          c = (c&31) | ((c>>1)&~31);
        for (int j=0;j<bpp;j+=8) {
          imgPtr[idx++] = (unsigned char)(c);
          bytesWritten++;
          c >>= 8;
        }
      }
    }
    // ensure our index matches up
    if (bytesWritten<rowstride)
      idx += rowstride-bytesWritten;
    // if printing to console, we're going to print everything as long as we have a multiple of 3 (or we're at the end)
    if (printBase64 && idx>2) {
      bool isLastRow = y==0;
      int count = isLastRow ? idx : (idx-(idx%3));
      JsVar *view = jsvNewArrayBufferFromString(imgData, count); // create an arraybuffer - this means we can pass to btoa with zero allocations
      JsVar *b64 = jswrap_btoa(view);
      jsvUnLock(view);
      if (b64) jsiConsolePrintf("%v", b64);
      jsvUnLock(b64);
      // shift everything back so we can start again
      if (count < idx)
        memmove(imgPtr, &imgPtr[count], idx-count);
      idx -= count;
    }
  }
  if (printBase64) {
    jsiConsolePrintf("\n");
    jsvUnLock(imgData);
    return 0;
  }
  return imgData;
}
JsVar *jswrap_graphics_asBMP(JsVar *parent) {
  return jswrap_graphics_asBMP_X(parent, false/*printBase64*/);
}

/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "asURL",
  "#if" : "!defined(SAVE_ON_FLASH) && !defined(ESPRUINOBOARD)",
  "generate" : "jswrap_graphics_asURL",
  "return" : ["JsVar","A String representing the Graphics as a URL (or 'undefined' if not possible)"],
  "typescript" : "asURL(): string;"
}
Create a URL of the form `data:image/bmp;base64,...` that can be pasted into the
browser.

The Espruino Web IDE can detect this data on the console and render the image
inline automatically.
*/
JsVar *jswrap_graphics_asURL(JsVar *parent) {
  JsVar *imgData = jswrap_graphics_asBMP_X(parent, false/*printBase64*/);
  if (!imgData) return 0; // not enough memory
  JsVar *b64 = jswrap_btoa(imgData);
  jsvUnLock(imgData);
  if (!b64) return 0; // not enough memory
  JsVar *r = jsvVarPrintf("data:image/bmp;base64,%v",b64);
  jsvUnLock(b64);
  return r;
}

/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "dump",
  "#if" : "!defined(SAVE_ON_FLASH) && !defined(ESPRUINOBOARD)",
  "generate" : "jswrap_graphics_dump"
}
Output this image as a bitmap URL of the form `data:image/bmp;base64,...`. The
Espruino Web IDE will detect this on the console and will render the image
inline automatically.

This is identical to `console.log(g.asURL())` - it is just a convenient function
for easy debugging and producing screenshots of what is currently in the
Graphics instance.

**Note:** This may not work on some bit depths of Graphics instances. It will
also not work for the main Graphics instance of Bangle.js 1 as the graphics on
Bangle.js 1 are stored in write-only memory.
*/
void jswrap_graphics_dump(JsVar *parent) {
  jsiConsolePrintf("data:image/bmp;base64,");
  jsvUnLock(jswrap_graphics_asBMP_X(parent, true/*printBase64*/));
}

/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "saveScreenshot",
  "#if" : "defined(DICKENS)",
  "generate" : "jswrap_graphics_saveScreenshot",
  "params" : [
    ["filename","JsVar","If supplied, a file to save, otherwise 'screenshot.img'"]
  ]
}
*/
void jswrap_graphics_saveScreenshot(JsVar *parent, JsVar *fileNameVar) {
#ifdef DICKENS
  JsfFileName fileName = jsfNameFromString("screenshot.img");
  if (fileNameVar) fileName = jsfNameFromVar(fileNameVar);

  JsVar *v = jsvNewFromString("\xF0\xF0\x10");
  jsfWriteFile(fileName, v, JSFF_NONE, 0, sizeof(lcdBuffer)+3);
  jsvUnLock(v);
  const int chunkSize = 16384;
  for (int i=0;i<sizeof(lcdBuffer);i+=chunkSize) {
    int s = chunkSize;
    if (s+i > sizeof(lcdBuffer))
      s=sizeof(lcdBuffer)-i;
    JsVar *gfxBufferString = jsvNewNativeString(&lcdBuffer[i], s);
    jsfWriteFile(fileName, gfxBufferString, JSFF_NONE, 3+i, 0);
    jsvUnLock(gfxBufferString);
  }
#endif
}


/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "quadraticBezier",
  "#if" : "!defined(SAVE_ON_FLASH) && !defined(ESPRUINOBOARD)",
  "generate" : "jswrap_graphics_quadraticBezier",
  "params" : [
    ["arr","JsVar","An array of three vertices, six enties in form of ```[x0,y0,x1,y1,x2,y2]```"],
    ["options","JsVar","number of points to calulate"]
  ],
  "return" : ["JsVar", "Array with calculated points" ],
  "typescript" : "quadraticBezier(arr: [number, number, number, number, number, number], options?: number): number[];"
}
 Calculate the square area under a Bezier curve.

 x0,y0: start point x1,y1: control point y2,y2: end point

 Max 10 points without start point.
*/
JsVar *jswrap_graphics_quadraticBezier( JsVar *parent, JsVar *arr, JsVar *options ){
  NOT_USED(parent);
  JsVar *result = jsvNewEmptyArray();
  if (!result) return 0;

  if (jsvGetArrayLength(arr) != 6) return result;

  int sn = 5;
  typedef struct { int x,y; } XY;
  XY xy[3];
  int count = 0;

  JsvIterator it;
  jsvIteratorNew(&it, arr, JSIF_EVERY_ARRAY_ELEMENT);
  for (int i=0;i<6;i++) {
    ((int*)xy)[i] = jsvIteratorGetIntegerValue(&it);
    jsvIteratorNext(&it);
  }
  jsvIteratorFree(&it);


  if (jsvIsObject(options)) count = jsvGetIntegerAndUnLock(jsvObjectGetChildIfExists(options,"count"));

  const int FP_MUL = 4096;
  const int FP_SHIFT = 12;
  int dx = xy[0].x - xy[2].x;
  if (dx<0) dx=-dx;
  int dy = xy[0].y - xy[2].y;
  if (dy<0) dy=-dy;
  int dmin = (dx < dy) ? dx : dy;
  if (dmin==0) dmin=1;
  int s =  FP_MUL*sn / dmin;
  if ( s >= FP_MUL)  s = FP_MUL/3;
  if ( s < FP_MUL/10) s = FP_MUL/10;
  if (count > 0) s = FP_MUL / count;

  jsvArrayPush2Int(result, xy[0].x, xy[0].y);

  for ( int t = s; t <= FP_MUL; t += s ) {
    int t2 = (t*t) >> FP_SHIFT;
    int tp2 = ((FP_MUL - t) * (FP_MUL - t)) >> FP_SHIFT;
    int tpt = (2 * (FP_MUL - t) * t) >> FP_SHIFT;
    jsvArrayPush2Int(result,
        (xy[0].x * tp2 + xy[1].x * tpt + xy[2].x * t2 + (FP_MUL/2)) >> FP_SHIFT,
        (xy[0].y * tp2 + xy[1].y * tpt + xy[2].y * t2 + (FP_MUL/2)) >> FP_SHIFT);
  }

  jsvArrayPush2Int(result, xy[2].x, xy[2].y);

  return  result;
}

/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "transformVertices",
  "#if" : "!defined(SAVE_ON_FLASH) && !defined(ESPRUINOBOARD)",
  "generate" : "jswrap_graphics_transformVertices",
  "params" : [
    ["verts","JsVar","An array of vertices, of the form ```[x1,y1,x2,y2,x3,y3,etc]```"],
    ["transformation","JsVar","The transformation to apply, either an Object or an Array (see below)"]
  ],
  "return" : ["JsVar", "Array of transformed vertices" ],
  "typescript" : "transformVertices(arr: number[], transformation: { x?: number, y?: number, scale?: number, rotate?: number } | [number, number, number, number, number, number]): number[];"
}
Transformation can be:

* An object of the form
```
{
  x: float, // x offset (default 0)
  y: float, // y offset (default 0)
  scale: float, // scale factor (default 1)
  rotate: float, // angle in radians (default 0)
}
```
* A six-element array of the form `[a,b,c,d,e,f]`, which represents the 2D transformation matrix
```
a c e
b d f
0 0 1
```

 Apply a transformation to an array of vertices.
*/
JsVar *jswrap_graphics_transformVertices(JsVar *parent, JsVar *verts, JsVar *transformation) {
  NOT_USED(parent);
  JsVar *result = jsvNewEmptyArray();
  if (!result) return 0;
  if (!jsvIsIterable(verts)) return result;

  double m[6];

  if (jsvIsObject(transformation)) {
    double x=0, y=0, scale=1, rotate=0;
    jsvConfigObject configs[] = {
        {"x", JSV_FLOAT, &x},
        {"y", JSV_FLOAT, &y},
        {"scale", JSV_FLOAT, &scale},
        {"rotate", JSV_FLOAT, &rotate}
    };
    if (!jsvReadConfigObject(transformation, configs, sizeof(configs) / sizeof(jsvConfigObject)))
        return result;
    double cosr = 1, sinr = 0;
    if (rotate) {
      cosr = jswrap_math_cos(rotate);
      sinr = jswrap_math_sin(rotate);
    }
    m[0] = cosr*scale; m[2] = -sinr*scale; m[4] = x;
    m[1] = sinr*scale; m[3] = cosr*scale; m[5] = y;
  } else if (jsvIsIterable(transformation) && jsvGetLength(transformation) == 6) {
    JsvIterator it;
    jsvIteratorNew(&it, transformation, JSIF_EVERY_ARRAY_ELEMENT);
    for (int i = 0; i < 6; ++i) {
      m[i] = jsvIteratorGetFloatValue(&it);
      jsvIteratorNext(&it);
    }
    jsvIteratorFree(&it);
  } else {
    jsExceptionHere(JSET_TYPEERROR,"Expected either an object or an array with 6 entries for second argument");
    return 0;
  }

  JsvIterator it;
  jsvIteratorNew(&it, verts, JSIF_EVERY_ARRAY_ELEMENT);
  while (jsvIteratorHasElement(&it)) {
    double x = jsvIteratorGetFloatValue(&it);
    jsvIteratorNext(&it);
    if (!jsvIteratorHasElement(&it)) break;
    double y = jsvIteratorGetFloatValue(&it);
    jsvIteratorNext(&it);

    jsvArrayPushAndUnLock(result, jsvNewFromFloat(m[0]*x + m[2]*y + m[4]));
    jsvArrayPushAndUnLock(result, jsvNewFromFloat(m[1]*x + m[3]*y + m[5]));
  }
  jsvIteratorFree(&it);

  return result;
}

/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "floodFill",
  "#if" : "!defined(SAVE_ON_FLASH) && !defined(ESPRUINOBOARD)",
  "generate" : "jswrap_graphics_floodFill",
  "params" : [
    ["x","int","X coordinate to start from"],
    ["y","int","Y coordinate to start from"],
    ["col","JsVar","The color to fill with (if undefined, foreground is used)"]
  ],
  "return" : ["JsVar","The instance of Graphics this was called on, to allow call chaining"],
  "return_object" : "Graphics"
}
Flood fills the given Graphics instance out from a particular point.

**Note:** This only works on Graphics instances that support readback with `getPixel`. It
is also not capable of filling over dithered patterns (eg non-solid colours on Bangle.js 2)
*/
static bool _jswrap_graphics_floodFill_inside(JsGraphics *gfx, int x, int y, unsigned int col) {
  if (x<0 || y<0 || x>=gfx->data.width || y>=gfx->data.height) return false;
  return graphicsGetPixel(gfx, x, y)==col;
}

JsVar *jswrap_graphics_floodFill(JsVar *parent, int x, int y, JsVar *col) {
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return 0;
  if (gfx.getPixel == graphicsFallbackGetPixel) {
    jsExceptionHere(JSET_ERROR, "Can't use floodFill on Graphics with no getPixel");
    return 0;
  }
  unsigned int fillCol = gfx.data.fgColor;
  if (col) fillCol = jswrap_graphics_toColor(parent, col, NULL, NULL);
  // https://en.wikipedia.org/wiki/Flood_fill#Span_filling
  unsigned int currentCol = graphicsGetPixel(&gfx, x, y);
  const int QUEUE_LEN = 64;
  short s[QUEUE_LEN];
  int si = 0; // index in queue
  #define S_ADD(x,y) if (si<QUEUE_LEN) {s[si++]=(short)x;s[si++]=(short)y;} else {jsiConsolePrintf("floodFill overflow\n");return jsvLockAgain(parent);}
  S_ADD(x, y);
  while (si) {
    // get new area to work from...
    short y = s[--si], x = s[--si], lx=x;
    // scan left
    while (_jswrap_graphics_floodFill_inside(&gfx, lx-1, y, currentCol))
      graphicsSetPixel(&gfx, --lx, y, fillCol);
    // scan right
    while (_jswrap_graphics_floodFill_inside(&gfx, x, y, currentCol))
      graphicsSetPixel(&gfx, x++, y, fillCol);
    // scan down and add
    bool span_added = false;
    for (short ix=lx;ix<x;ix++) {
      if (!_jswrap_graphics_floodFill_inside(&gfx, ix, y+1, currentCol))
        span_added = false;
      else if (!span_added) {
        S_ADD(ix, y+1);
        span_added = true;
      }
    }
    // scan up and add
    span_added = false;
    for (short ix=lx;ix<x;ix++) {
      if (!_jswrap_graphics_floodFill_inside(&gfx, ix, y-1, currentCol))
        span_added = false;
      else if (!span_added) {
        S_ADD(ix, y-1);
        span_added = true;
      }
    }
  }
  #undef S_ADD
  return jsvLockAgain(parent);
}

/*TYPESCRIPT
type Theme = {
  fg: number;
  bg: number;
  fg2: number;
  bg2: number;
  fgH: number;
  bgH: number;
  dark: boolean;
};
*/
/*JSON{
  "type" : "property",
  "class" : "Graphics",
  "name" : "theme",
  "#if" : "!defined(SAVE_ON_FLASH) && !defined(ESPRUINOBOARD)",
  "generate" : "jswrap_graphics_theme",
  "return" : ["JsVar","An object containing the current 'theme' (see below)"],
  "typescript" : "theme: Theme;"
}
Returns an object of the form:

```
{
  fg : 0xFFFF,  // foreground colour
  bg : 0,       // background colour
  fg2 : 0xFFFF,  // accented foreground colour
  bg2 : 0x0007,  // accented background colour
  fgH : 0xFFFF,  // highlighted foreground colour
  bgH : 0x02F7,  // highlighted background colour
  dark : true,  // Is background dark (e.g. foreground should be a light colour)
}
```

These values can then be passed to `g.setColor`/`g.setBgColor` for example
`g.setColor(g.theme.fg2)`. When the Graphics instance is reset, the background
color is automatically set to `g.theme.bg` and foreground is set to
`g.theme.fg`.

On Bangle.js these values can be changed by writing updated values to `theme` in
`settings.js` and reloading the app - or they can be changed temporarily by
calling `Graphics.setTheme`
*/
JsVar *jswrap_graphics_theme(JsVar *parent) {
  NOT_USED(parent);
#ifdef GRAPHICS_THEME
  JsVar *o = jsvNewObject();
  jsvObjectSetChildAndUnLock(o,"fg",jsvNewFromInteger((JsVarInt)(uint32_t)graphicsTheme.fg));
  jsvObjectSetChildAndUnLock(o,"bg",jsvNewFromInteger((JsVarInt)(uint32_t)graphicsTheme.bg));
  jsvObjectSetChildAndUnLock(o,"fg2",jsvNewFromInteger((JsVarInt)(uint32_t)graphicsTheme.fg2));
  jsvObjectSetChildAndUnLock(o,"bg2",jsvNewFromInteger((JsVarInt)(uint32_t)graphicsTheme.bg2));
  jsvObjectSetChildAndUnLock(o,"fgH",jsvNewFromInteger((JsVarInt)(uint32_t)graphicsTheme.fgH));
  jsvObjectSetChildAndUnLock(o,"bgH",jsvNewFromInteger((JsVarInt)(uint32_t)graphicsTheme.bgH));
  jsvObjectSetChildAndUnLock(o,"dark",jsvNewFromBool((JsVarInt)(uint32_t)graphicsTheme.dark));
  return o;
#else
  return 0;
#endif
}

/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "setTheme",
  "#if" : "!defined(SAVE_ON_FLASH) && !defined(ESPRUINOBOARD)",
  "generate" : "jswrap_graphics_setTheme",
  "params" : [
    ["theme","JsVar","An object of the form returned by `Graphics.theme`"]
  ],
  "return" : ["JsVar","The instance of Graphics this was called on, to allow call chaining"],
  "return_object" : "Graphics",
  "typescript" : "setTheme(theme: { [key in keyof Theme]?: Theme[key] extends number ? ColorResolvable : Theme[key] }): Graphics;"
}
Set the global colour scheme. On Bangle.js, this is reloaded from
`settings.json` for each new app loaded.

See `Graphics.theme` for the fields that can be provided. For instance you can
change the background to red using:

```
g.setTheme({bg:"#f00"});
```

*/
JsVar *jswrap_graphics_setTheme(JsVar *parent, JsVar *theme) {
#ifdef GRAPHICS_THEME
  if (jsvIsObject(theme)) {
    JsVar *v;
    v = jsvObjectGetChildIfExists(theme, "fg");
    if (v) {
      graphicsTheme.fg = jswrap_graphics_toColor(parent, v,0,0);
      jsvUnLock(v);
    }
    v = jsvObjectGetChildIfExists(theme, "bg");
    if (v) {
      graphicsTheme.bg = jswrap_graphics_toColor(parent, v,0,0);
      jsvUnLock(v);
    }
    v = jsvObjectGetChildIfExists(theme, "fg2");
    if (v) {
      graphicsTheme.fg2 = jswrap_graphics_toColor(parent, v,0,0);
      jsvUnLock(v);
    }
    v = jsvObjectGetChildIfExists(theme, "bg2");
    if (v) {
      graphicsTheme.bg2 = jswrap_graphics_toColor(parent, v,0,0);
      jsvUnLock(v);
    }
    v = jsvObjectGetChildIfExists(theme, "fgH");
    if (v) {
      graphicsTheme.fgH = jswrap_graphics_toColor(parent, v,0,0);
      jsvUnLock(v);
    }
    v = jsvObjectGetChildIfExists(theme, "bgH");
    if (v) {
      graphicsTheme.bgH = jswrap_graphics_toColor(parent, v,0,0);
      jsvUnLock(v);
    }
    v = jsvObjectGetChildIfExists(theme, "dark");
    if (v) graphicsTheme.dark = jsvGetBoolAndUnLock(v);
  }
#endif
  return jsvLockAgain(parent);
}

