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
 * 4x6 LCD font (but with last column as spaces)
 * ----------------------------------------------------------------------------
 */

#include "bitmap_font_4x6.h"

#define ___ 0
#define __X 1
#define _X_ 2
#define _XX 3
#define X__ 4
#define X_X 5
#define XX_ 6
#define XXX 7
#define PACK_5_TO_16(A,B,C,D,E) ((A) | (B<<3) | (C<<6) | (D<<9) | (E<<12))
 // 48

#define LCD_FONT_4X6_CHARS 95
const unsigned short LCD_FONT_4X6[] IN_FLASH_MEMORY = { // from 33 up to 127
    PACK_5_TO_16( _X_ , X_X , X_X , _X_ , X_X ),
    PACK_5_TO_16( _X_ , X_X , XXX , XXX , __X ),
    PACK_5_TO_16( _X_ , ___ , X_X , XX_ , _X_ ),
    PACK_5_TO_16( ___ , ___ , XXX , _XX , X__ ),
    PACK_5_TO_16( _X_ , ___ , X_X , XXX , X_X ),
    PACK_5_TO_16( ___ , ___ , ___ , _X_ , ___ ),

    PACK_5_TO_16( XX_ , _X_ , _X_ , _X_ , _X_ ),
    PACK_5_TO_16( XX_ , _X_ , X__ , __X , XXX ),
    PACK_5_TO_16( _XX , ___ , X__ , __X , X_X ),
    PACK_5_TO_16( XX_ , ___ , X__ , __X , ___ ),
    PACK_5_TO_16( XXX , ___ , _X_ , _X_ , ___ ),
    PACK_5_TO_16( ___ , ___ , ___ , ___ , ___ ),

    PACK_5_TO_16( ___ , ___ , ___ , ___ , __X ),
    PACK_5_TO_16( ___ , ___ , ___ , ___ , __X ),
    PACK_5_TO_16( _X_ , ___ , ___ , ___ , _X_ ),
    PACK_5_TO_16( XXX , ___ , XXX , ___ , X__ ),
    PACK_5_TO_16( _X_ , _X_ , ___ , _X_ , X__ ),
    PACK_5_TO_16( ___ , X__ , ___ , ___ , ___ ),

    PACK_5_TO_16( _X_ , _X_ , XX_ , XX_ , _XX ),
    PACK_5_TO_16( X_X , XX_ , __X , __X , X_X ),
    PACK_5_TO_16( X_X , _X_ , _X_ , _X_ , X_X ),
    PACK_5_TO_16( X_X , _X_ , X__ , __X , XXX ),
    PACK_5_TO_16( _X_ , XXX , XXX , XX_ , __X ),
    PACK_5_TO_16( ___ , ___ , ___ , ___ , ___ ),

    PACK_5_TO_16( XXX , _XX , XXX , _X_ , _X_ ),
    PACK_5_TO_16( X__ , X__ , __X , X_X , X_X ),
    PACK_5_TO_16( XX_ , XX_ , __X , _X_ , _XX ),
    PACK_5_TO_16( __X , X_X , _X_ , X_X , __X ),
    PACK_5_TO_16( XX_ , _X_ , _X_ , _X_ , _X_ ),
    PACK_5_TO_16( ___ , ___ , ___ , ___ , ___ ),

    PACK_5_TO_16( ___ , ___ , ___ , ___ , ___ ),
    PACK_5_TO_16( ___ , ___ , ___ , ___ , ___ ),
    PACK_5_TO_16( _X_ , _X_ , _X_ , XXX , _X_ ),
    PACK_5_TO_16( ___ , ___ , X__ , ___ , __X ),
    PACK_5_TO_16( _X_ , _X_ , _X_ , XXX , _X_ ),
    PACK_5_TO_16( ___ , X__ , ___ , ___ , ___ ),

    PACK_5_TO_16( XX_ , _X_ , _X_ , XX_ , _X_ ),
    PACK_5_TO_16( __X , X_X , X_X , X_X , X_X ),
    PACK_5_TO_16( _X_ , XXX , X_X , XX_ , X__ ),
    PACK_5_TO_16( ___ , X__ , XXX , X_X , X_X ),
    PACK_5_TO_16( _X_ , _XX , X_X , XX_ , _X_ ),
    PACK_5_TO_16( ___ , ___ , ___ , ___ , ___ ),

    PACK_5_TO_16( XX_ , XXX , XXX , _XX , X_X ),
    PACK_5_TO_16( X_X , X__ , X__ , X__ , X_X ),
    PACK_5_TO_16( X_X , XX_ , XX_ , X_X , XXX ),
    PACK_5_TO_16( X_X , X__ , X__ , X_X , X_X ),
    PACK_5_TO_16( XX_ , XXX , X__ , _XX , X_X ),
    PACK_5_TO_16( ___ , ___ , ___ , ___ , ___ ),

    PACK_5_TO_16( XXX , __X , X_X , X__ , X_X ),
    PACK_5_TO_16( _X_ , __X , XX_ , X__ , XXX ),
    PACK_5_TO_16( _X_ , __X , XX_ , X__ , XXX ),
    PACK_5_TO_16( _X_ , X_X , X_X , X__ , XXX ),
    PACK_5_TO_16( XXX , _X_ , X_X , XXX , XXX ),
    PACK_5_TO_16( ___ , ___ , ___ , ___ , ___ ),

    PACK_5_TO_16( X_X , _X_ , XX_ , _X_ , XX_ ),
    PACK_5_TO_16( XXX , X_X , X_X , X_X , X_X ),
    PACK_5_TO_16( XXX , X_X , X_X , X_X , X_X ),
    PACK_5_TO_16( XXX , X_X , XX_ , XXX , XX_ ),
    PACK_5_TO_16( X_X , _X_ , X__ , _XX , X_X ),
    PACK_5_TO_16( ___ , ___ , ___ , ___ , ___ ),

    PACK_5_TO_16( _XX , XXX , X_X , X_X , X_X ),
    PACK_5_TO_16( X__ , _X_ , X_X , X_X , X_X ),
    PACK_5_TO_16( _X_ , _X_ , X_X , X_X , XXX ),
    PACK_5_TO_16( __X , _X_ , X_X , XXX , XXX ),
    PACK_5_TO_16( XX_ , _X_ , XXX , _X_ , X_X ),
    PACK_5_TO_16( ___ , ___ , ___ , ___ , ___ ),

    PACK_5_TO_16( X_X , X_X , XXX , XXX , X__ ),
    PACK_5_TO_16( X_X , X_X , __X , X__ , X__ ),
    PACK_5_TO_16( _X_ , XXX , _X_ , X__ , _X_ ),
    PACK_5_TO_16( X_X , _X_ , X__ , X__ , __X ),
    PACK_5_TO_16( X_X , _X_ , XXX , XXX , __X ),
    PACK_5_TO_16( ___ , ___ , ___ , ___ , ___ ),

    PACK_5_TO_16( XXX , _X_ , ___ , X__ , ___ ),
    PACK_5_TO_16( __X , X_X , ___ , _X_ , XX_ ),
    PACK_5_TO_16( __X , ___ , ___ , ___ , __X ),
    PACK_5_TO_16( __X , ___ , ___ , ___ , XXX ),
    PACK_5_TO_16( XXX , ___ , XXX , ___ , XXX ),
    PACK_5_TO_16( ___ , ___ , ___ , ___ , ___ ),

    PACK_5_TO_16( X__ , ___ , __X , ___ , __X ),
    PACK_5_TO_16( XX_ , _XX , _XX , _X_ , _X_ ),
    PACK_5_TO_16( X_X , X__ , X_X , XXX , XXX ),
    PACK_5_TO_16( X_X , X__ , X_X , X__ , _X_ ),
    PACK_5_TO_16( XX_ , _XX , _XX , _XX , _X_ ),
    PACK_5_TO_16( ___ , ___ , ___ , ___ , ___ ),

    PACK_5_TO_16( ___ , X__ , _X_ , _X_ , X__ ),
    PACK_5_TO_16( _XX , XX_ , ___ , ___ , X_X ),
    PACK_5_TO_16( X_X , X_X , XX_ , XX_ , XX_ ),
    PACK_5_TO_16( _XX , X_X , _X_ , _X_ , X_X ),
    PACK_5_TO_16( __X , X_X , XXX , _X_ , X_X ),
    PACK_5_TO_16( XX_ , ___ , ___ , X__ , ___ ),

    PACK_5_TO_16( XX_ , ___ , ___ , ___ , ___ ),
    PACK_5_TO_16( _X_ , XX_ , XX_ , _X_ , XX_ ),
    PACK_5_TO_16( _X_ , XXX , X_X , X_X , X_X ),
    PACK_5_TO_16( _X_ , XXX , X_X , X_X , X_X ),
    PACK_5_TO_16( XXX , XXX , X_X , _X_ , XX_ ),
    PACK_5_TO_16( ___ , ___ , ___ , ___ , X__ ),

    PACK_5_TO_16( ___ , ___ , ___ , _X_ , ___ ),
    PACK_5_TO_16( _XX , X_X , _XX , XXX , X_X ),
    PACK_5_TO_16( X_X , XX_ , XX_ , _X_ , X_X ),
    PACK_5_TO_16( X_X , X__ , __X , _X_ , X_X ),
    PACK_5_TO_16( _XX , X__ , XX_ , __X , _XX ),
    PACK_5_TO_16( __X , ___ , ___ , ___ , ___ ),

    PACK_5_TO_16( ___ , ___ , ___ , ___ , ___ ),
    PACK_5_TO_16( X_X , X_X , X_X , X_X , XXX ),
    PACK_5_TO_16( X_X , XXX , _X_ , X_X , _X_ ),
    PACK_5_TO_16( XXX , XXX , X_X , _XX , X__ ),
    PACK_5_TO_16( _X_ , XXX , X_X , __X , XXX ),
    PACK_5_TO_16( ___ , ___ , ___ , XX_ , ___ ),

    PACK_5_TO_16( _XX , _X_ , XX_ , _XX , XXX ),
    PACK_5_TO_16( _X_ , _X_ , _X_ , XX_ , XXX ),
    PACK_5_TO_16( XX_ , _X_ , _XX , ___ , XXX ),
    PACK_5_TO_16( _X_ , _X_ , _X_ , ___ , XXX ),
    PACK_5_TO_16( _XX , _X_ , XX_ , ___ , XXX ),
    PACK_5_TO_16( ___ , ___ , ___ , ___ , XXX )
};

void graphicsDrawChar4x6(JsGraphics *gfx, short x1, short y1, char ch) {
  int idx = ((unsigned char)ch) - 33;
  if (idx<0 || idx>=LCD_FONT_4X6_CHARS) return; // no char for this - just return
  int cidx = idx % 5;
  idx = (idx/5)*6;
  int y;
  for (y=0;y<6;y++) {
    unsigned int line = READ_FLASH_UINT16(&LCD_FONT_4X6[idx + y]) >> (cidx*3);
    if (line&4) graphicsSetPixel(gfx, (short)(x1+0), (short)(y+y1), gfx->data.fgColor);
    if (line&2) graphicsSetPixel(gfx, (short)(x1+1), (short)(y+y1), gfx->data.fgColor);
    if (line&1) graphicsSetPixel(gfx, (short)(x1+2), (short)(y+y1), gfx->data.fgColor);
  }
}












