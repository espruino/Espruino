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
 * 6x8 LCD font - based on http://uzebox.org/wiki/index.php?title=File:Sebasic_charset_192w.png
 * ----------------------------------------------------------------------------
 */

#ifdef USE_FONT_6X8

#include "bitmap_font_6x8.h"

#define ______ 0
#define _____X 1
#define ____X_ 2
#define ____XX 3
#define ___X__ 4
#define ___X_X 5
#define ___XX_ 6
#define ___XXX 7
#define __X___ 8
#define __X__X 9
#define __X_X_ 10
#define __X_XX 11
#define __XX__ 12
#define __XX_X 13
#define __XXX_ 14
#define __XXXX 15
#define _X____ 16
#define _X___X 17
#define _X__X_ 18
#define _X__XX 19
#define _X_X__ 20
#define _X_X_X 21
#define _X_XX_ 22
#define _X_XXX 23
#define _XX___ 24
#define _XX__X 25
#define _XX_X_ 26
#define _XX_XX 27
#define _XXX__ 28
#define _XXX_X 29
#define _XXXX_ 30
#define _XXXXX 31
#define X_____ 32
#define X____X 33
#define X___X_ 34
#define X___XX 35
#define X__X__ 36
#define X__X_X 37
#define X__XX_ 38
#define X__XXX 39
#define X_X___ 40
#define X_X__X 41
#define X_X_X_ 42
#define X_X_XX 43
#define X_XX__ 44
#define X_XX_X 45
#define X_XXX_ 46
#define X_XXXX 47
#define XX____ 48
#define XX___X 49
#define XX__X_ 50
#define XX__XX 51
#define XX_X__ 52
#define XX_X_X 53
#define XX_XX_ 54
#define XX_XXX 55
#define XXX___ 56
#define XXX__X 57
#define XXX_X_ 58
#define XXX_XX 59
#define XXXX__ 60
#define XXXX_X 61
#define XXXXX_ 62
#define XXXXXX 63
#define PACK_5_TO_32(A,B,C,D,E) ((A) | (B<<6) | (C<<12) | (D<<18) | (E<<24))
 // 48

#define LCD_FONT_6X8_CHARS 95
const uint32_t LCD_FONT_6X8[] IN_FLASH_MEMORY = { // from 33 up to 128
    PACK_5_TO_32( __X___ , _X_X__ , __X_X_ , _XXX__ , _XXXX_ ),
    PACK_5_TO_32( __X___ , _X_X__ , XXXXX_ , X_X_X_ , X_X_X_ ),
    PACK_5_TO_32( __X___ , ______ , _X_X__ , X_X___ , X_XX__ ),
    PACK_5_TO_32( __X___ , ______ , XXXXX_ , _XXX__ , _XXXX_ ),
    PACK_5_TO_32( __X___ , ______ , X_X___ , __X_X_ , __XX_X ),
    PACK_5_TO_32( ______ , ______ , ______ , X_X_X_ , _X_X_X ),
    PACK_5_TO_32( __X___ , ______ , ______ , _XXX__ , _X__X_ ),
    PACK_5_TO_32( ______ , ______ , ______ , __X___ , ______ ),

    PACK_5_TO_32( __X___ , __X___ , ___X__ , _X____ , __X___ ),
    PACK_5_TO_32( _X_X__ , __X___ , __X___ , __X___ , X_X_X_ ),
    PACK_5_TO_32( __X___ , ______ , _X____ , ___X__ , _XXX__ ),
    PACK_5_TO_32( _X_X_X , ______ , _X____ , ___X__ , _X_X__ ),
    PACK_5_TO_32( X___X_ , ______ , _X____ , ___X__ , X___X_ ),
    PACK_5_TO_32( X___X_ , ______ , _X____ , ___X__ , ______ ),
    PACK_5_TO_32( _XXX_X , ______ , __X___ , __X___ , ______ ),
    PACK_5_TO_32( ______ , ______ , ___X__ , _X____ , ______ ),

    PACK_5_TO_32( ______ , ______ , ______ , ______ , ____X_ ),
    PACK_5_TO_32( __X___ , ______ , ______ , ______ , ____X_ ),
    PACK_5_TO_32( __X___ , ______ , ______ , ______ , ___X__ ),
    PACK_5_TO_32( XXXXX_ , ______ , _XXXX_ , ______ , ___X__ ),
    PACK_5_TO_32( __X___ , ______ , ______ , ______ , __X___ ),
    PACK_5_TO_32( __X___ , __X___ , ______ , ______ , __X___ ),
    PACK_5_TO_32( ______ , __X___ , ______ , __X___ , _X____ ),
    PACK_5_TO_32( ______ , _X____ , ______ , ______ , _X____ ),

    PACK_5_TO_32( _XXX__ , __X___ , _XXX__ , XXXXX_ , ___X__ ),
    PACK_5_TO_32( X___X_ , _XX___ , X___X_ , ___X__ , __XX__ ),
    PACK_5_TO_32( X__XX_ , __X___ , ____X_ , __X___ , _X_X__ ),
    PACK_5_TO_32( X_X_X_ , __X___ , ___X__ , _XXX__ , X__X__ ),
    PACK_5_TO_32( XX__X_ , __X___ , __X___ , ____X_ , XXXXX_ ),
    PACK_5_TO_32( X___X_ , __X___ , _X____ , X___X_ , ___X__ ),
    PACK_5_TO_32( _XXX__ , __X___ , XXXXX_ , _XXX__ , ___X__ ),
    PACK_5_TO_32( ______ , ______ , ______ , ______ , ______ ),

    PACK_5_TO_32( XXXXX_ , __XX__ , XXXXX_ , _XXX__ , _XXX__ ),
    PACK_5_TO_32( X_____ , _X____ , ____X_ , X___X_ , X___X_ ),
    PACK_5_TO_32( XXXX__ , X_____ , ___X__ , X___X_ , X___X_ ),
    PACK_5_TO_32( ____X_ , XXXX__ , ___X__ , _XXX__ , _XXXX_ ),
    PACK_5_TO_32( ____X_ , X___X_ , __X___ , X___X_ , ____X_ ),
    PACK_5_TO_32( X___X_ , X___X_ , __X___ , X___X_ , ___X__ ),
    PACK_5_TO_32( _XXX__ , _XXX__ , __X___ , _XXX__ , _XX___ ),
    PACK_5_TO_32( ______ , ______ , ______ , ______ , ______ ),

    PACK_5_TO_32( ______ , ______ , ______ , ______ , ______ ),
    PACK_5_TO_32( ______ , ______ , ___X__ , ______ , _X____ ),
    PACK_5_TO_32( __X___ , __X___ , __X___ , XXXXX_ , __X___ ),
    PACK_5_TO_32( ______ , ______ , _X____ , ______ , ___X__ ),
    PACK_5_TO_32( ______ , ______ , __X___ , XXXXX_ , __X___ ),
    PACK_5_TO_32( ______ , __X___ , ___X__ , ______ , _X____ ),
    PACK_5_TO_32( __X___ , __X___ , ______ , ______ , ______ ),
    PACK_5_TO_32( ______ , _X____ , ______ , ______ , ______ ),

    PACK_5_TO_32( __XX__ , __XX__ , __X___ , XXXX__ , _XXX__ ),
    PACK_5_TO_32( _X__X_ , _X__X_ , __X___ , X___X_ , X___X_ ),
    PACK_5_TO_32( ____X_ , X_XX_X , _X_X__ , X___X_ , X_____ ),
    PACK_5_TO_32( ___X__ , XX_X_X , _X_X__ , XXXX__ , X_____ ),
    PACK_5_TO_32( __X___ , XX_X_X , XXXXX_ , X___X_ , X_____ ),
    PACK_5_TO_32( ______ , X_XXX_ , X___X_ , X___X_ , X___X_ ),
    PACK_5_TO_32( __X___ , _X____ , X___X_ , XXXX__ , _XXX__ ),
    PACK_5_TO_32( ______ , __XX__ , ______ , ______ , ______ ),

    PACK_5_TO_32( XXX___ , _XXXX_ , _XXXX_ , _XXX__ , X___X_ ),
    PACK_5_TO_32( X__X__ , _X____ , _X____ , X___X_ , X___X_ ),
    PACK_5_TO_32( X___X_ , _X____ , _X____ , X_____ , X___X_ ),
    PACK_5_TO_32( X___X_ , _XXX__ , _XXX__ , X__XX_ , XXXXX_ ),
    PACK_5_TO_32( X___X_ , _X____ , _X____ , X___X_ , X___X_ ),
    PACK_5_TO_32( X__X__ , _X____ , _X____ , X___X_ , X___X_ ),
    PACK_5_TO_32( XXX___ , _XXXX_ , _X____ , _XXX__ , X___X_ ),
    PACK_5_TO_32( ______ , ______ , ______ , ______ , ______ ),

    PACK_5_TO_32( __X___ , ____X_ , X___X_ , _X____ , X___X_ ),
    PACK_5_TO_32( __X___ , ____X_ , X__X__ , _X____ , XX_XX_ ),
    PACK_5_TO_32( __X___ , ____X_ , X_X___ , _X____ , X_X_X_ ),
    PACK_5_TO_32( __X___ , ____X_ , XX____ , _X____ , X___X_ ),
    PACK_5_TO_32( __X___ , X___X_ , X_X___ , _X____ , X___X_ ),
    PACK_5_TO_32( __X___ , X___X_ , X__X__ , _X____ , X___X_ ),
    PACK_5_TO_32( __X___ , _XXX__ , X___X_ , _XXXX_ , X___X_ ),
    PACK_5_TO_32( ______ , ______ , ______ , ______ , ______ ),

    PACK_5_TO_32( XX__X_ , _XXX__ , XXXX__ , _XXX__ , XXXX__ ),
    PACK_5_TO_32( XX__X_ , X___X_ , X___X_ , X___X_ , X___X_ ),
    PACK_5_TO_32( X_X_X_ , X___X_ , X___X_ , X___X_ , X___X_ ),
    PACK_5_TO_32( X_X_X_ , X___X_ , XXXX__ , X___X_ , XXXX__ ),
    PACK_5_TO_32( X__XX_ , X___X_ , X_____ , X___X_ , X_X___ ),
    PACK_5_TO_32( X__XX_ , X___X_ , X_____ , X_X_X_ , X__X__ ),
    PACK_5_TO_32( X___X_ , _XXX__ , X_____ , _XXX__ , X___X_ ),
    PACK_5_TO_32( ______ , ______ , ______ , ___X__ , ______ ),

    PACK_5_TO_32( _XXX__ , XXXXX_ , X___X_ , X___X_ , X___X_ ),
    PACK_5_TO_32( X___X_ , __X___ , X___X_ , X___X_ , X___X_ ),
    PACK_5_TO_32( X_____ , __X___ , X___X_ , X___X_ , X_X_X_ ),
    PACK_5_TO_32( _XXX__ , __X___ , X___X_ , _X_X__ , X_X_X_ ),
    PACK_5_TO_32( ____X_ , __X___ , X___X_ , _X_X__ , _X_X__ ),
    PACK_5_TO_32( X___X_ , __X___ , X___X_ , __X___ , _X_X__ ),
    PACK_5_TO_32( _XXX__ , __X___ , _XXX__ , __X___ , _X_X__ ),
    PACK_5_TO_32( ______ , ______ , ______ , ______ , ______ ),

    PACK_5_TO_32( X___X_ , X___X_ , _XXXX_ , __XX__ , _X____ ),
    PACK_5_TO_32( X___X_ , X___X_ , ____X_ , __X___ , _X____ ),
    PACK_5_TO_32( _X_X__ , _X_X__ , ___X__ , __X___ , __X___ ),
    PACK_5_TO_32( __X___ , __X___ , __X___ , __X___ , __X___ ),
    PACK_5_TO_32( _X_X__ , __X___ , _X____ , __X___ , ___X__ ),
    PACK_5_TO_32( X___X_ , __X___ , _X____ , __X___ , ___X__ ),
    PACK_5_TO_32( X___X_ , __X___ , _XXXX_ , __X___ , ____X_ ),
    PACK_5_TO_32( ______ , ______ , ______ , __XX__ , ____X_ ),

    PACK_5_TO_32( _XX___ , __X___ , ______ , __XX__ , ______ ),
    PACK_5_TO_32( __X___ , _X_X__ , ______ , _X____ , ______ ),
    PACK_5_TO_32( __X___ , ______ , ______ , _X____ , __XX__ ),
    PACK_5_TO_32( __X___ , ______ , ______ , XXX___ , ____X_ ),
    PACK_5_TO_32( __X___ , ______ , ______ , _X____ , __XXX_ ),
    PACK_5_TO_32( __X___ , ______ , ______ , _X__X_ , _X__X_ ),
    PACK_5_TO_32( __X___ , ______ , ______ , XXXX__ , __XXX_ ),
    PACK_5_TO_32( _XX___ , ______ , XXXXXX , ______ , ______ ),

    PACK_5_TO_32( _X____ , ______ , ____X_ , ______ , ___XX_ ),
    PACK_5_TO_32( _X____ , ______ , ____X_ , ______ , __X___ ),
    PACK_5_TO_32( _XXX__ , __XX__ , __XXX_ , __XX__ , _XXX__ ),
    PACK_5_TO_32( _X__X_ , _X__X_ , _X__X_ , _X__X_ , __X___ ),
    PACK_5_TO_32( _X__X_ , _X____ , _X__X_ , _XXXX_ , __X___ ),
    PACK_5_TO_32( _X__X_ , _X__X_ , _X__X_ , _X____ , __X___ ),
    PACK_5_TO_32( _XXX__ , __XX__ , __XXX_ , __XX__ , __X___ ),
    PACK_5_TO_32( ______ , ______ , ______ , ______ , ______ ),

    PACK_5_TO_32( ______ , _X____ , __X___ , ___X__ , _X____ ),
    PACK_5_TO_32( ______ , _X____ , ______ , ______ , _X____ ),
    PACK_5_TO_32( __XXX_ , _XXX__ , __X___ , ___X__ , _X__X_ ),
    PACK_5_TO_32( _X__X_ , _X__X_ , __X___ , ___X__ , _X_X__ ),
    PACK_5_TO_32( _X__X_ , _X__X_ , __X___ , ___X__ , _XX___ ),
    PACK_5_TO_32( __XXX_ , _X__X_ , __X___ , ___X__ , _X_X__ ),
    PACK_5_TO_32( ____X_ , _X__X_ , __X___ , ___X__ , _X__X_ ),
    PACK_5_TO_32( __XX__ , ______ , ______ , _XX___ , ______ ),

    PACK_5_TO_32( __X___ , ______ , ______ , ______ , ______ ),
    PACK_5_TO_32( __X___ , ______ , ______ , ______ , ______ ),
    PACK_5_TO_32( __X___ , XX_X__ , _XXX__ , __XX__ , _XXX__ ),
    PACK_5_TO_32( __X___ , X_X_X_ , _X__X_ , _X__X_ , _X__X_ ),
    PACK_5_TO_32( __X___ , X_X_X_ , _X__X_ , _X__X_ , _X__X_ ),
    PACK_5_TO_32( __X___ , X_X_X_ , _X__X_ , _X__X_ , _XXX__ ),
    PACK_5_TO_32( ___X__ , X_X_X_ , _X__X_ , __XX__ , _X____ ),
    PACK_5_TO_32( ______ , ______ , ______ , ______ , _X____ ),

    PACK_5_TO_32( ______ , ______ , ______ , __X___ , ______ ),
    PACK_5_TO_32( ______ , ______ , ______ , __X___ , ______ ),
    PACK_5_TO_32( __XXX_ , _X_XX_ , __XXX_ , _XXX__ , _X__X_ ),
    PACK_5_TO_32( _X__X_ , _XX___ , _X____ , __X___ , _X__X_ ),
    PACK_5_TO_32( _X__X_ , _X____ , __XX__ , __X___ , _X__X_ ),
    PACK_5_TO_32( __XXX_ , _X____ , ____X_ , __X___ , _X__X_ ),
    PACK_5_TO_32( ____X_ , _X____ , _XXX__ , ___X__ , __XX__ ),
    PACK_5_TO_32( ____X_ , ______ , ______ , ______ , ______ ),

    PACK_5_TO_32( ______ , ______ , ______ , ______ , ______ ),
    PACK_5_TO_32( ______ , ______ , ______ , ______ , ______ ),
    PACK_5_TO_32( X___X_ , X___X_ , X___X_ , X___X_ , _XXXX_ ),
    PACK_5_TO_32( _X_X__ , X___X_ , _X_X__ , _X_X__ , ___X__ ),
    PACK_5_TO_32( _X_X__ , X_X_X_ , __X___ , _X_X__ , __X___ ),
    PACK_5_TO_32( __X___ , _X_X__ , _X_X__ , __X___ , _X____ ),
    PACK_5_TO_32( __X___ , _X_X__ , X___X_ , __X___ , _XXXX_ ),
    PACK_5_TO_32( ______ , ______ , ______ , XX____ , ______ ),

    PACK_5_TO_32( ___X__ , __X___ , _X____ , _XX_X_ , __XX__ ),
    PACK_5_TO_32( __X___ , __X___ , __X___ , X_XX__ , _X__X_ ),
    PACK_5_TO_32( __X___ , __X___ , __X___ , ______ , X_XX_X ),
    PACK_5_TO_32( _X____ , ______ , ___X__ , ______ , XX___X ),
    PACK_5_TO_32( __X___ , ______ , __X___ , ______ , XX___X ),
    PACK_5_TO_32( __X___ , __X___ , __X___ , ______ , X_XX_X ),
    PACK_5_TO_32( __X___ , __X___ , __X___ , ______ , _X__X_ ),
    PACK_5_TO_32( ___X__ , __X___ , _X____ , ______ , __XX__ )

};


void graphicsDrawChar6x8(JsGraphics *gfx, short x1, short y1, char ch) {
  int idx = ((unsigned char)ch) - 33;
  if (idx<0 || idx>=LCD_FONT_6X8_CHARS) return; // no char for this - just return
  int cidx = idx % 5;
  idx = (idx/5)*8;
  int y;
  for (y=0;y<8;y++) {
    unsigned int line = LCD_FONT_6X8[idx + y] >> (cidx*6);
    if (line&32) graphicsSetPixel(gfx, (short)(x1+0), (short)(y+y1), gfx->data.fgColor);
    if (line&16) graphicsSetPixel(gfx, (short)(x1+1), (short)(y+y1), gfx->data.fgColor);
    if (line&8) graphicsSetPixel(gfx, (short)(x1+2), (short)(y+y1), gfx->data.fgColor);
    if (line&4) graphicsSetPixel(gfx, (short)(x1+3), (short)(y+y1), gfx->data.fgColor);
    if (line&2) graphicsSetPixel(gfx, (short)(x1+4), (short)(y+y1), gfx->data.fgColor);
    if (line&1) graphicsSetPixel(gfx, (short)(x1+5), (short)(y+y1), gfx->data.fgColor);
  }
}

#endif // USE_FONT_6X8










