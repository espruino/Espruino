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
 * 3x5 LCD font
 * ----------------------------------------------------------------------------
 */

#include "bitmap_font_3x5.h"

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

#define LCD_FONT_3X5_CHARS 95
const unsigned short LCD_FONT_3X5[] IN_FLASH_MEMORY = { // from 33 up to 127
    PACK_5_TO_16( _X_ , X_X , _X_ , _X_ , X_X ), // !"#$%
    PACK_5_TO_16( _X_ , ___ , XXX , XX_ , __X ),
    PACK_5_TO_16( _X_ , ___ , _X_ , XXX , _X_ ),
    PACK_5_TO_16( ___ , ___ , XXX , _XX , X__ ),
    PACK_5_TO_16( _X_ , ___ , _X_ , _X_ , X_X ),

    PACK_5_TO_16( XXX , _X_ , __X , X__ , X_X ), // &'()*
    PACK_5_TO_16( X_X , ___ , _X_ , _X_ , _X_ ),
    PACK_5_TO_16( X_X , ___ , _X_ , _X_ , XXX ),
    PACK_5_TO_16( XXX , ___ , _X_ , _X_ , _X_ ),
    PACK_5_TO_16( _XX , ___ , __X , X__ , X_X ),

    PACK_5_TO_16( ___ , ___ , ___ , ___ , __X ), // +,-./
    PACK_5_TO_16( _X_ , ___ , ___ , ___ , __X ),
    PACK_5_TO_16( XXX , ___ , XXX , ___ , _X_ ),
    PACK_5_TO_16( _X_ , _X_ , ___ , ___ , X__ ),
    PACK_5_TO_16( ___ , X__ , ___ , _X_ , X__ ),

    PACK_5_TO_16( XXX , _X_ , XXX , XX_ , X_X ), // 01234
    PACK_5_TO_16( X_X , XX_ , __X , __X , X_X ),
    PACK_5_TO_16( X_X , _X_ , _X_ , XX_ , XXX ),
    PACK_5_TO_16( X_X , _X_ , X__ , __X , __X ),
    PACK_5_TO_16( XXX , XXX , XXX , XX_ , __X ),

    PACK_5_TO_16( XXX , XXX , XXX , XXX , XXX ), // 56789
    PACK_5_TO_16( X__ , X__ , __X , X_X , X_X ),
    PACK_5_TO_16( XXX , XXX , _X_ , XXX , XXX ),
    PACK_5_TO_16( __X , X_X , _X_ , X_X , __X ),
    PACK_5_TO_16( XXX , XXX , _X_ , XXX , XXX ),

    PACK_5_TO_16( ___ , ___ , __X , ___ , X__ ), // :;<=>
    PACK_5_TO_16( _X_ , _X_ , _X_ , XXX , _X_ ),
    PACK_5_TO_16( ___ , ___ , X__ , ___ , __X ),
    PACK_5_TO_16( _X_ , _X_ , _X_ , XXX , _X_ ),
    PACK_5_TO_16( ___ , X__ , __X , ___ , X__ ),

    PACK_5_TO_16( _X_ , _X_ , _X_ , XX_ , _XX ), // ?@ABC
    PACK_5_TO_16( X_X , X_X , X_X , X_X , X__ ),
    PACK_5_TO_16( __X , XXX , XXX , XX_ , X__ ),
    PACK_5_TO_16( ___ , X_X , X_X , X_X , X__ ),
    PACK_5_TO_16( _X_ , _XX , X_X , XX_ , _XX ),

    PACK_5_TO_16( XX_ , XXX , XXX , _XX , X_X ), // DEFGH
    PACK_5_TO_16( X_X , X__ , X__ , X__ , X_X ),
    PACK_5_TO_16( X_X , XX_ , XXX , X_X , XXX ),
    PACK_5_TO_16( X_X , X__ , X__ , X_X , X_X ),
    PACK_5_TO_16( XX_ , XXX , X__ , _XX , X_X ),

    PACK_5_TO_16( XXX , XXX , X_X , X__ , X_X ), // IJKLM
    PACK_5_TO_16( _X_ , __X , X_X , X__ , XXX ),
    PACK_5_TO_16( _X_ , __X , XX_ , X__ , XXX ),
    PACK_5_TO_16( _X_ , __X , X_X , X__ , X_X ),
    PACK_5_TO_16( XXX , XX_ , X_X , XXX , X_X ),

    PACK_5_TO_16( XX_ , _XX , XX_ , _X_ , XX_ ), // NOPQR
    PACK_5_TO_16( X_X , X_X , X_X , X_X , X_X ),
    PACK_5_TO_16( X_X , X_X , XX_ , X_X , XX_ ),
    PACK_5_TO_16( X_X , X_X , X__ , X_X , X_X ),
    PACK_5_TO_16( X_X , _X_ , X__ , _XX , X_X ),

    PACK_5_TO_16( _XX , XXX , X_X , X_X , X_X ), // STUVW
    PACK_5_TO_16( X__ , _X_ , X_X , X_X , X_X ),
    PACK_5_TO_16( _X_ , _X_ , X_X , X_X , XXX ),
    PACK_5_TO_16( __X , _X_ , X_X , _X_ , XXX ),
    PACK_5_TO_16( XX_ , _X_ , _X_ , _X_ , X_X ),

    PACK_5_TO_16( X_X , X_X , XXX , _XX , X__ ), // XYZ[\
    PACK_5_TO_16( X_X , X_X , __X , _X_ , X__ ),
    PACK_5_TO_16( _X_ , _X_ , _X_ , _X_ , _X_ ),
    PACK_5_TO_16( X_X , _X_ , X__ , _X_ , __X ),
    PACK_5_TO_16( X_X , _X_ , XXX , _XX , __X ),

    PACK_5_TO_16( XX_ , _X_ , ___ , X__ , _X_ ), // ]^_`a
    PACK_5_TO_16( _X_ , X_X , ___ , _X_ , X_X ),
    PACK_5_TO_16( _X_ , ___ , ___ , ___ , XXX ),
    PACK_5_TO_16( _X_ , ___ , ___ , ___ , X_X ),
    PACK_5_TO_16( XX_ , ___ , XXX , ___ , X_X ),

    PACK_5_TO_16( XX_ , _XX , XX_ , XXX , XXX ), // bcdef
    PACK_5_TO_16( X_X , X__ , X_X , X__ , X__ ),
    PACK_5_TO_16( XX_ , X__ , X_X , XX_ , XXX ),
    PACK_5_TO_16( X_X , X__ , X_X , X__ , X__ ),
    PACK_5_TO_16( XX_ , _XX , XX_ , XXX , X__ ),

    PACK_5_TO_16( _XX , X_X , XXX , XXX , X_X ), // ghijk
    PACK_5_TO_16( X__ , X_X , _X_ , __X , X_X ),
    PACK_5_TO_16( X_X , XXX , _X_ , __X , XX_ ),
    PACK_5_TO_16( X_X , X_X , _X_ , __X , X_X ),
    PACK_5_TO_16( _XX , X_X , XXX , XX_ , X_X ),

    PACK_5_TO_16( X__ , X_X , XX_ , _XX , XX_ ), // lmnop
    PACK_5_TO_16( X__ , XXX , X_X , X_X , X_X ),
    PACK_5_TO_16( X__ , XXX , X_X , X_X , XX_ ),
    PACK_5_TO_16( X__ , X_X , X_X , X_X , X__ ),
    PACK_5_TO_16( XXX , X_X , X_X , _X_ , X__ ),

    PACK_5_TO_16( _X_ , XX_ , _XX , XXX , X_X ), // qrstu
    PACK_5_TO_16( X_X , X_X , X__ , _X_ , X_X ),
    PACK_5_TO_16( X_X , XX_ , _X_ , _X_ , X_X ),
    PACK_5_TO_16( X_X , X_X , __X , _X_ , X_X ),
    PACK_5_TO_16( _XX , X_X , XX_ , _X_ , _X_ ),

    PACK_5_TO_16( ___ , ___ , ___ , X_X , ___ ), // vwxyz
    PACK_5_TO_16( ___ , X_X , X_X , X_X , ___ ),
    PACK_5_TO_16( X_X , X_X , _X_ , XXX , XXX ),
    PACK_5_TO_16( X_X , XXX , _X_ , __X , _X_ ),
    PACK_5_TO_16( _X_ , X_X , X_X , XXX , XXX ),

    PACK_5_TO_16( __X , _X_ , X__ , X__ , XXX ), // {|}~ del
    PACK_5_TO_16( _X_ , _X_ , _X_ , XXX , XXX ),
    PACK_5_TO_16( XX_ , _X_ , _XX , __X , XXX ),
    PACK_5_TO_16( _X_ , _X_ , _X_ , ___ , XXX ),
    PACK_5_TO_16( __X , _X_ , X__ , ___ , XXX ),
};

void graphicsDrawChar4x6(JsGraphics *gfx, short x1, short y1, char ch) {
  int idx = ((unsigned char)ch) - 33;
  if (idx<0 || idx>=LCD_FONT_3X5_CHARS) return; // no char for this - just return
  int cidx = idx % 5;
  idx -= cidx;
  int y;
  for (y=0;y<6;y++) {
    unsigned short line = LCD_FONT_3X5[idx + y] >> (cidx*3);
    if (line&1) graphicsSetPixel(gfx, x1+0, y+y1, gfx->data.fgColor);
    if (line&2) graphicsSetPixel(gfx, x1+1, y+y1, gfx->data.fgColor);
    if (line&4) graphicsSetPixel(gfx, x1+2, y+y1, gfx->data.fgColor);
  }
}
