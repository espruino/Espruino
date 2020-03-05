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

#define _____ 0
#define ____X 1
#define ___X_ 2
#define ___XX 3
#define __X__ 4
#define __X_X 5
#define __XX_ 6
#define __XXX 7
#define _X___ 8
#define _X__X 9
#define _X_X_ 10
#define _X_XX 11
#define _XX__ 12
#define _XX_X 13
#define _XXX_ 14
#define _XXXX 15
#define X____ 16
#define X___X 17
#define X__X_ 18
#define X__XX 19
#define X_X__ 20
#define X_X_X 21
#define X_XX_ 22
#define X_XXX 23
#define XX___ 24
#define XX__X 25
#define XX_X_ 26
#define XX_XX 27
#define XXX__ 28
#define XXX_X 29
#define XXXX_ 30
#define XXXXX 31
#define PACK_6_TO_32(A,B,C,D,E,F) ((A) | (B<<5) | (C<<10) | (D<<15) | (E<<20) | (F<<25))

// Generated from bitmap-fonts/bitmap/dina/Dina_r400-6.bdf at https://github.com/Tecate/bitmap-fonts
// -windows-Dina-medium-r-normal--8-60-96-96-c-60-microsoft-cp1252// BOUNDINGBOX 6 10 0 -2// _ASCENT 8// _DESCENT 2
// Copyright 2013 Joergen Ibsen, MIT license, rev 218
// With tweaks from Gordon Williams to get into 8 line height

#define LCD_FONT_6X8_CHARS 223 // also 4 extra>255 that we don't have right now
const uint32_t LCD_FONT_6X8[] IN_FLASH_MEMORY = { // from 33 up...
    PACK_6_TO_32( _____ , X_X__ , _____ , __X__ , XX__X , __X__ ),
    PACK_6_TO_32( X____ , X_X__ , _X_X_ , _XXX_ , XX_X_ , _X_X_ ),
    PACK_6_TO_32( X____ , X_X__ , XXXXX , X_X__ , ___X_ , __X__ ),
    PACK_6_TO_32( X____ , _____ , _X_X_ , _XXX_ , __X__ , _XX_X ),
    PACK_6_TO_32( X____ , _____ , _X_X_ , __X_X , _X___ , X__X_ ),
    PACK_6_TO_32( _____ , _____ , XXXXX , X_X_X , _X_XX , X__X_ ),
    PACK_6_TO_32( X____ , _____ , _X_X_ , _XXX_ , X__XX , _XX_X ),
    PACK_6_TO_32( _____ , _____ , _____ , __X__ , _____ , _____ ),

    PACK_6_TO_32( X____ , __X__ , X____ , _____ , _____ , _____ ),
    PACK_6_TO_32( X____ , _X___ , _X___ , _____ , _____ , _____ ),
    PACK_6_TO_32( X____ , X____ , __X__ , _X_X_ , __X__ , _____ ),
    PACK_6_TO_32( _____ , X____ , __X__ , __X__ , __X__ , _____ ),
    PACK_6_TO_32( _____ , X____ , __X__ , XXXXX , XXXXX , _____ ),
    PACK_6_TO_32( _____ , X____ , __X__ , __X__ , __X__ , _X___ ),
    PACK_6_TO_32( _____ , _X___ , _X___ , _X_X_ , __X__ , _X___ ),
    PACK_6_TO_32( _____ , __X__ , X____ , _____ , _____ , X____ ),

    PACK_6_TO_32( _____ , _____ , ____X , _____ , _____ , _____ ),
    PACK_6_TO_32( _____ , _____ , ___X_ , _XXX_ , __X__ , _XXX_ ),
    PACK_6_TO_32( _____ , _____ , ___X_ , X__XX , _XX__ , X___X ),
    PACK_6_TO_32( _____ , _____ , __X__ , X_X_X , X_X__ , ____X ),
    PACK_6_TO_32( XXXXX , _____ , __X__ , X_X_X , __X__ , _XXX_ ),
    PACK_6_TO_32( _____ , X____ , _X___ , XX__X , __X__ , X____ ),
    PACK_6_TO_32( _____ , X____ , _X___ , _XXX_ , XXXXX , XXXXX ),
    PACK_6_TO_32( _____ , _____ , X____ , _____ , _____ , _____ ),

    PACK_6_TO_32( _____ , _____ , _____ , _____ , _____ , _____ ),
    PACK_6_TO_32( _XXX_ , ___X_ , XXXXX , __XX_ , XXXXX , _XXX_ ),
    PACK_6_TO_32( X___X , __XX_ , X____ , _X___ , ____X , X___X ),
    PACK_6_TO_32( __XX_ , _X_X_ , XXXX_ , XXXX_ , ___X_ , _XXX_ ),
    PACK_6_TO_32( ____X , X__X_ , ____X , X___X , __X__ , X___X ),
    PACK_6_TO_32( X___X , XXXXX , X___X , X___X , _X___ , X___X ),
    PACK_6_TO_32( _XXX_ , ___X_ , _XXX_ , _XXX_ , _X___ , _XXX_ ),
    PACK_6_TO_32( _____ , _____ , _____ , _____ , _____ , _____ ),

    PACK_6_TO_32( _____ , _____ , _____ , _____ , _____ , _____ ),
    PACK_6_TO_32( _XXX_ , _____ , _____ , ___X_ , _____ , X____ ),
    PACK_6_TO_32( X___X , X____ , _X___ , __X__ , _____ , _X___ ),
    PACK_6_TO_32( X___X , X____ , _X___ , _X___ , XXXXX , __X__ ),
    PACK_6_TO_32( _XXXX , _____ , _____ , X____ , _____ , ___X_ ),
    PACK_6_TO_32( ____X , X____ , _X___ , _X___ , XXXXX , __X__ ),
    PACK_6_TO_32( _XXX_ , X____ , _X___ , __X__ , _____ , _X___ ),
    PACK_6_TO_32( _____ , _____ , X____ , ___X_ , _____ , X____ ),

    PACK_6_TO_32( _____ , _XXX_ , _____ , _____ , _____ , _____ ),
    PACK_6_TO_32( _XXX_ , X___X , __X__ , XXXX_ , _XXX_ , XXX__ ),
    PACK_6_TO_32( X___X , X__XX , __X__ , X___X , X___X , X__X_ ),
    PACK_6_TO_32( ___X_ , X_X_X , _X_X_ , XXXX_ , X____ , X___X ),
    PACK_6_TO_32( __X__ , X_X_X , _XXX_ , X___X , X____ , X___X ),
    PACK_6_TO_32( _____ , X__XX , X___X , X___X , X___X , X__X_ ),
    PACK_6_TO_32( __X__ , X____ , X___X , XXXX_ , _XXX_ , XXX__ ),
    PACK_6_TO_32( _____ , _XXXX , _____ , _____ , _____ , _____ ),

    PACK_6_TO_32( _____ , _____ , _____ , _____ , _____ , _____ ),
    PACK_6_TO_32( XXXXX , XXXXX , _XXX_ , X___X , _XXX_ , __XXX ),
    PACK_6_TO_32( X____ , X____ , X____ , X___X , __X__ , ____X ),
    PACK_6_TO_32( XXX__ , XXX__ , X____ , XXXXX , __X__ , ____X ),
    PACK_6_TO_32( X____ , X____ , X_XXX , X___X , __X__ , X___X ),
    PACK_6_TO_32( X____ , X____ , X___X , X___X , __X__ , X___X ),
    PACK_6_TO_32( XXXXX , X____ , _XXX_ , X___X , _XXX_ , _XXX_ ),
    PACK_6_TO_32( _____ , _____ , _____ , _____ , _____ , _____ ),
//  PACK_6_TO_32( _____ , _____ , _____ , _____ , _____ , _____ ),

    PACK_6_TO_32( _____ , _____ , _____ , _____ , _____ , _____ ),
    PACK_6_TO_32( X___X , X____ , X___X , X___X , _XXX_ , XXXX_ ),
    PACK_6_TO_32( X__X_ , X____ , XX_XX , XX__X , X___X , X___X ),
    PACK_6_TO_32( X_X__ , X____ , X_X_X , X_X_X , X___X , X___X ),
    PACK_6_TO_32( XXX__ , X____ , X___X , X__XX , X___X , XXXX_ ),
    PACK_6_TO_32( X__X_ , X____ , X___X , X___X , X___X , X____ ),
    PACK_6_TO_32( X___X , XXXXX , X___X , X___X , _XXX_ , X____ ),
    PACK_6_TO_32( _____ , _____ , _____ , _____ , _____ , _____ ),

    PACK_6_TO_32( _____ , _____ , _____ , _____ , _____ , _____ ),
    PACK_6_TO_32( _XXX_ , XXXX_ , _XXX_ , XXXXX , X___X , X___X ),
    PACK_6_TO_32( X___X , X___X , X____ , __X__ , X___X , X___X ),
    PACK_6_TO_32( X___X , XXXX_ , _XXX_ , __X__ , X___X , X___X ),
    PACK_6_TO_32( X___X , X___X , ____X , __X__ , X___X , _X_X_ ),
    PACK_6_TO_32( X__X_ , X___X , X___X , __X__ , X___X , _X_X_ ),
    PACK_6_TO_32( _XX_X , X___X , _XXX_ , __X__ , _XXX_ , __X__ ),
    PACK_6_TO_32( ____X , _____ , _____ , _____ , _____ , _____ ),

    PACK_6_TO_32( _____ , _____ , _____ , _____ , XXX__ , X____ ),
    PACK_6_TO_32( X___X , X___X , X___X , XXXXX , X____ , _X___ ),
    PACK_6_TO_32( X___X , _X_X_ , X___X , ___X_ , X____ , _X___ ),
    PACK_6_TO_32( X_X_X , __X__ , _X_X_ , __X__ , X____ , __X__ ),
    PACK_6_TO_32( X_X_X , _X_X_ , __X__ , _X___ , X____ , __X__ ),
    PACK_6_TO_32( _XXX_ , X___X , __X__ , X____ , X____ , ___X_ ),
    PACK_6_TO_32( _X_X_ , X___X , __X__ , XXXXX , X____ , ___X_ ),
    PACK_6_TO_32( _____ , _____ , _____ , _____ , XXX__ , ____X ),

    PACK_6_TO_32( XXX__ , __X__ , _____ , X____ , _____ , X____ ),
    PACK_6_TO_32( __X__ , __X__ , _____ , _X___ , _____ , X____ ),
    PACK_6_TO_32( __X__ , _X_X_ , _____ , _____ , _XXX_ , XXXX_ ),
    PACK_6_TO_32( __X__ , _X_X_ , _____ , _____ , ____X , X___X ),
    PACK_6_TO_32( __X__ , X___X , _____ , _____ , _XXXX , X___X ),
    PACK_6_TO_32( __X__ , _____ , _____ , _____ , X___X , X___X ),
    PACK_6_TO_32( __X__ , _____ , _____ , _____ , _XXXX , XXXX_ ),
    PACK_6_TO_32( XXX__ , _____ , XXXXX , _____ , _____ , _____ ),

    PACK_6_TO_32( _____ , ____X , _____ , __XXX , _____ , X____ ),
    PACK_6_TO_32( _____ , ____X , _____ , _X___ , _____, X____ ),
    PACK_6_TO_32( _XXX_ , _XXXX , _XXX_ , XXXX_ , _XXXX, XXXX_ ),
    PACK_6_TO_32( X____ , X___X , X___X , _X___ , X___X, X___X ),
    PACK_6_TO_32( X____ , X___X , XXXXX , _X___ , X___X, X___X ),
    PACK_6_TO_32( X____ , X___X , X____ , _X___ , _XXXX, X___X ),
    PACK_6_TO_32( _XXXX , _XXXX , _XXXX , _X___ , ____X, X___X ),
    PACK_6_TO_32( _____ , _____ , _____ , _____ , _XXX_, _____ ),


    PACK_6_TO_32( __X__ , ___X_ , X____ , _XX__ , _____ , _____ ),
    PACK_6_TO_32( _____ , _____ , X____ , __X__ , _____ , _____ ),
    PACK_6_TO_32( _XX__ , _XXX_ , X___X , __X__ , XX_X_ , XXXX_ ),
    PACK_6_TO_32( __X__ , ___X_ , X__X_ , __X__ , X_X_X , X___X ),
    PACK_6_TO_32( __X__ , ___X_ , XXX__ , __X__ , X_X_X , X___X ),
    PACK_6_TO_32( __X__ , ___X_ , X__X_ , __X__ , X_X_X , X___X ),
    PACK_6_TO_32( _XXX_ , ___X_ , X___X , __XX_ , X_X_X , X___X ),
    PACK_6_TO_32( _____ , XXX__ , _____ , _____ , _____ , _____ ),

    PACK_6_TO_32( _____ , _____ , _____ , _____ , _____ , _X___ ),
    PACK_6_TO_32( _____ , _____ , _____ , _____ , _____ , _X___ ),
    PACK_6_TO_32( _XXX_ , XXXX_ , _XXXX , X_XX_ , _XXX_ , XXXX_ ),
    PACK_6_TO_32( X___X , X___X , X___X , XX__X , X____ , _X___ ),
    PACK_6_TO_32( X___X , X___X , X___X , X____ , _XXX_ , _X___ ),
    PACK_6_TO_32( X___X , XXXX_ , _XXXX , X____ , ____X , _X___ ),
    PACK_6_TO_32( _XXX_ , X____ , ____X , X____ , XXXX_ , __XXX ),
    PACK_6_TO_32( _____ , X____ , ____X , _____ , _____ , _____ ),


    PACK_6_TO_32( _____ , _____ , _____ , _____ , _____ , _____ ),
    PACK_6_TO_32( _____ , _____ , _____ , _____ , _____ , _____ ),
    PACK_6_TO_32( X___X , X___X , X___X , X___X , X___X , XXXXX ),
    PACK_6_TO_32( X___X , X___X , X___X , _X_X_ , X___X , ___X_ ),
    PACK_6_TO_32( X___X , _X_X_ , X_X_X , __X__ , X___X , __X__ ),
    PACK_6_TO_32( X___X , _X_X_ , X_X_X , _X_X_ , _XXXX , _X___ ),
    PACK_6_TO_32( _XXXX , __X__ , _X_X_ , X___X , ____X , XXXXX ),
    PACK_6_TO_32( _____ , _____ , _____ , _____ , _XXX_ , _____ ),

    PACK_6_TO_32( ___XX , X____ , XX___ , _____ , _____ , __XX_ ),
    PACK_6_TO_32( __X__ , X____ , __X__ , _____ , _____ , _X__X ),
    PACK_6_TO_32( __X__ , X____ , __X__ , _____ , _____ , XXX__ ),
    PACK_6_TO_32( XX___ , X____ , ___XX , _X__X , _____ , _X___ ),
    PACK_6_TO_32( XX___ , X____ , ___XX , X_X_X , _____ , XXX__ ),
    PACK_6_TO_32( __X__ , X____ , __X__ , X__X_ , _____ , _X__X ),
    PACK_6_TO_32( __X__ , X____ , __X__ , _____ , _____ , __XX_ ),
    PACK_6_TO_32( ___XX , X____ , XX___ , _____ , _____ , _____ ),

    PACK_6_TO_32( _____ , _____ , ___XX , _____ , _____ , _____ ),
    PACK_6_TO_32( _____ , _____ , __X__ , _____ , _____ , __X__ ),
    PACK_6_TO_32( _____ , _____ , __X__ , _____ , _____ , __X__ ),
    PACK_6_TO_32( _____ , _____ , _XXX_ , _____ , _____ , XXXXX ),
    PACK_6_TO_32( _____ , _____ , __X__ , _____ , _____ , __X__ ),
    PACK_6_TO_32( _____ , XX___ , __X__ , XX_XX , _____ , __X__ ),
    PACK_6_TO_32( _____ , _X___ , __X__ , _X__X , X_X_X , __X__ ),
    PACK_6_TO_32( _____ , X____ , XX___ , X__X_ , _____ , __X__ ),

    PACK_6_TO_32( __X__ , _____ , XX__X , __X__ , _____ , _____ ),
    PACK_6_TO_32( __X__ , _X___ , XX_X_ , _XXX_ , _____ , _XXXX ),
    PACK_6_TO_32( XXXXX , X_X__ , __X__ , X____ , __X__ , X_X__ ),
    PACK_6_TO_32( __X__ , _____ , _X___ , _XXX_ , _X___ , X_XX_ ),
    PACK_6_TO_32( __X__ , _____ , X____ , ____X , X____ , X_X__ ),
    PACK_6_TO_32( XXXXX , _____ , XX_XX , X___X , _X___ , X_X__ ),
    PACK_6_TO_32( __X__ , _____ , XX_XX , _XXX_ , __X__ , _XXXX ),
    PACK_6_TO_32( __X__ , _____ , _____ , _____ , _____ , _____ ),

    PACK_6_TO_32( _____ , __X__ , _____ , _____ , _X___ , XX___ ),
    PACK_6_TO_32( _____ , XXXXX , _____ , _____ , X____ , _X___ ),
    PACK_6_TO_32( _____ , ___X_ , _____ , _____ , XX___ , X____ ),
    PACK_6_TO_32( _____ , __X__ , _____ , _____ , _____ , _____ ),
    PACK_6_TO_32( _____ , _X___ , _____ , _____ , _____ , _____ ),
    PACK_6_TO_32( _____ , X____ , _____ , _____ , _____ , _____ ),
    PACK_6_TO_32( _____ , XXXXX , _____ , _____ , _____ , _____ ),
    PACK_6_TO_32( _____ , _____ , _____ , _____ , _____ , _____ ),

    PACK_6_TO_32( _X__X , XX_XX , _____ , _____ , _____ , _____ ),
    PACK_6_TO_32( X__X_ , _X__X , _____ , _____ , _____ , _XX_X ),
    PACK_6_TO_32( XX_XX , X__X_ , __X__ , _____ , _____ , X_XX_ ),
    PACK_6_TO_32( _____ , _____ , _XXX_ , _____ , _____ , _____ ),
    PACK_6_TO_32( _____ , _____ , XXXXX , XXX__ , XXXXX , _____ ),
    PACK_6_TO_32( _____ , _____ , _XXX_ , _____ , _____ , _____ ),
    PACK_6_TO_32( _____ , _____ , __X__ , _____ , _____ , _____ ),
    PACK_6_TO_32( _____ , _____ , _____ , _____ , _____ , _____ ),

    PACK_6_TO_32( _____ , __X__ , _____ , _____ , _____ , __X__ ),
    PACK_6_TO_32( XXXXX , _____ , _____ , _____ , _____ , _____ ),
    PACK_6_TO_32( _X_XX , _XXX_ , X____ , _X_X_ , _____ , XXXXX ),
    PACK_6_TO_32( _X_XX , X____ , _X___ , X_X_X , _____ , ___X_ ),
    PACK_6_TO_32( _____ , _XXX_ , __X__ , X_XX_ , _____ , __X__ ),
    PACK_6_TO_32( _____ , ____X , _X___ , X_X__ , _____ , _X___ ),
    PACK_6_TO_32( _____ , XXXX_ , X____ , _X_XX , _____ , XXXXX ),
    PACK_6_TO_32( _____ , _____ , _____ , _____ , _____ , _____ ),

    PACK_6_TO_32( _____ , _____ , _____ , __X__ , _____ , _____ ),
    PACK_6_TO_32( X___X , _____ , _____ , __X__ , __XX_ , _____ ),
    PACK_6_TO_32( X___X , _____ , X____ , _XXX_ , _X___ , X___X ),
    PACK_6_TO_32( _X_X_ , _____ , _____ , X_X__ , XXX__ , _XXX_ ),
    PACK_6_TO_32( __X__ , _____ , X____ , X_X__ , _X___ , _X_X_ ),
    PACK_6_TO_32( __X__ , _____ , X____ , _XXXX , _X___ , _XXX_ ),
    PACK_6_TO_32( __X__ , _____ , X____ , __X__ , X_XXX , X___X ),
    PACK_6_TO_32( _____ , _____ , X____ , __X__ , _____ , _____ ),

    PACK_6_TO_32( X___X , X____ , _XXX_ , X_X__ , _XXX_ , _XX__ ),
    PACK_6_TO_32( X___X , X____ , X____ , _____ , X___X , ___X_ ),
    PACK_6_TO_32( _X_X_ , X____ , _XX__ , _____ , X_X_X , _XXX_ ),
    PACK_6_TO_32( XXXXX , _____ , X__X_ , _____ , XX__X , X__X_ ),
    PACK_6_TO_32( __X__ , _____ , _X__X , _____ , XX__X , _XXX_ ),
    PACK_6_TO_32( XXXXX , X____ , __XX_ , _____ , X_X_X , _____ ),
    PACK_6_TO_32( __X__ , X____ , ____X , _____ , X___X , _____ ),
    PACK_6_TO_32( _____ , X____ , XXXX_ , _____ , _XXX_ , _____ ),

    PACK_6_TO_32( _____ , _____ , _____ , _XXX_ , XXXXX , _XX__ ),
    PACK_6_TO_32( _____ , _____ , _____ , X___X , _____ , X__X_ ),
    PACK_6_TO_32( __X_X , _____ , _____ , XXX_X , _____ , X__X_ ),
    PACK_6_TO_32( _X_X_ , _____ , _____ , XX_XX , _____ , _XX__ ),
    PACK_6_TO_32( X_X__ , XXXX_ , _____ , XXX_X , _____ , _____ ),
    PACK_6_TO_32( _X_X_ , ___X_ , _____ , XX_XX , _____ , _____ ),
    PACK_6_TO_32( __X_X , ___X_ , _____ , X___X , _____ , _____ ),
    PACK_6_TO_32( _____ , _____ , _____ , _XXX_ , _____ , _____ ),

    PACK_6_TO_32( _____ , XX___ , XX___ , _X___ , _____ , _____ ),
    PACK_6_TO_32( __X__ , __X__ , __X__ , X____ , _X__X , _XXXX ),
    PACK_6_TO_32( __X__ , _X___ , _X___ , _____ , _X__X , XXX_X ),
    PACK_6_TO_32( XXXXX , X____ , __X__ , _____ , _X__X , _XX_X ),
    PACK_6_TO_32( __X__ , XXX__ , XX___ , _____ , _XX_X , __X_X ),
    PACK_6_TO_32( __X__ , _____ , _____ , _____ , _X_X_ , __X_X ),
    PACK_6_TO_32( XXXXX , _____ , _____ , _____ , _X___ , __X_X ),
    PACK_6_TO_32( _____ , _____ , _____ , _____ , X____ , __X_X ),

    PACK_6_TO_32( _____ , _____ , _X___ , _XX__ , _____ , X__X_ ),
    PACK_6_TO_32( _____ , _____ , XX___ , X__X_ , _____ , X__X_ ),
    PACK_6_TO_32( _____ , _____ , _X___ , X__X_ , X_X__ , X_X__ ),
    PACK_6_TO_32( XX___ , _____ , _X___ , X__X_ , _X_X_ , __X__ ),
    PACK_6_TO_32( XX___ , _____ , XXX__ , _XX__ , __X_X , _X__X ),
    PACK_6_TO_32( _____ , _X___ , _____ , _____ , _X_X_ , _X_XX ),
    PACK_6_TO_32( _____ , __X__ , _____ , _____ , X_X__ , X_XXX ),
    PACK_6_TO_32( _____ , XX___ , _____ , _____ , _____ , X___X ),

    PACK_6_TO_32( X__X_ , X____ , _____ , __X__ , __X__ , _X_X_ ),
    PACK_6_TO_32( X__X_ , _X_X_ , _____ , __X__ , __X__ , _____ ),
    PACK_6_TO_32( X_X__ , X__X_ , __X__ , __X__ , __X__ , __X__ ),
    PACK_6_TO_32( __X__ , _XX__ , _____ , _X_X_ , _X_X_ , _X_X_ ),
    PACK_6_TO_32( _X_XX , X_X__ , __X__ , _XXX_ , _XXX_ , _XXX_ ),
    PACK_6_TO_32( _X__X , _X__X , _X___ , X___X , X___X , X___X ),
    PACK_6_TO_32( X__X_ , _X_XX , X___X , X___X , X___X , X___X ),
    PACK_6_TO_32( X__XX , X_X_X , _XXX_ , _____ , _____ , _____ ),

    PACK_6_TO_32( X_XX_ , _____ , _X_X_ , _____ , _____ , __X__ ),
    PACK_6_TO_32( __X__ , __X__ , __X__ , __XXX , _XXX_ , XXXXX ),
    PACK_6_TO_32( __X__ , __X__ , __X__ , _XX__ , X___X , X____ ),
    PACK_6_TO_32( _X_X_ , _X_X_ , _X_X_ , X_XX_ , X____ , XXX__ ),
    PACK_6_TO_32( _XXX_ , _XXX_ , _XXX_ , XXX__ , X____ , X____ ),
    PACK_6_TO_32( X___X , X___X , X___X , X_X__ , X___X , X____ ),
    PACK_6_TO_32( X___X , X___X , X___X , X_XXX , _XXX_ , XXXXX ),
    PACK_6_TO_32( _____ , _____ , _____ , _____ , __X__ , _____ ),

    PACK_6_TO_32( __X__ , _X_X_ , _____ , _X___ , _X___ , X_X__ ),
    PACK_6_TO_32( XXXXX , XXXXX , XXXXX , XXX__ , XXX__ , XXX__ ),
    PACK_6_TO_32( X____ , X____ , X____ , _X___ , _X___ , _X___ ),
    PACK_6_TO_32( XXX__ , XXX__ , XXX__ , _X___ , _X___ , _X___ ),
    PACK_6_TO_32( X____ , X____ , X____ , _X___ , _X___ , _X___ ),
    PACK_6_TO_32( X____ , X____ , X____ , _X___ , _X___ , _X___ ),
    PACK_6_TO_32( XXXXX , XXXXX , XXXXX , XXX__ , XXX__ , XXX__ ),
    PACK_6_TO_32( _____ , _____ , _____ , _____ , _____ , _____ ),

    PACK_6_TO_32( _____ , _____ , X_XX_ , __X__ , __X__ , _X_X_ ),
    PACK_6_TO_32( XXX__ , _XX__ , X___X , _XXX_ , _XXX_ , _XXX_ ),
    PACK_6_TO_32( _X___ , _X_X_ , XX__X , X___X , X___X , X___X ),
    PACK_6_TO_32( _X___ , XXX_X , X_X_X , X___X , X___X , X___X ),
    PACK_6_TO_32( _X___ , _X__X , X__XX , X___X , X___X , X___X ),
    PACK_6_TO_32( _X___ , _X_X_ , X___X , X___X , X___X , X___X ),
    PACK_6_TO_32( XXX__ , _XX__ , X___X , _XXX_ , _XXX_ , _XXX_ ),
    PACK_6_TO_32( _____ , _____ , _____ , _____ , _____ , _____ ),

    PACK_6_TO_32( X_XX_ , _____ , _____ , ____X , __X__ , __X__ ),
    PACK_6_TO_32( _XXX_ , _XXX_ , _____ , _XXX_ , X___X , X___X ),
    PACK_6_TO_32( X___X , X___X , _____ , X__XX , X___X , X___X ),
    PACK_6_TO_32( X___X , X___X , X_X__ , X_X_X , X___X , X___X ),
    PACK_6_TO_32( X___X , X___X , _X___ , X_X_X , X___X , X___X ),
    PACK_6_TO_32( X___X , X___X , X_X__ , XX__X , X___X , X___X ),
    PACK_6_TO_32( _XXX_ , _XXX_ , _____ , _XXX_ , _XXX_ , _XXX_ ),
    PACK_6_TO_32( _____ , _____ , _____ , X____ , _____ , _____ ),

    PACK_6_TO_32( _X_X_ , _____ , __X__ , _____ , _____ , __X__ ),
    PACK_6_TO_32( X___X , X___X , X___X , X____ , _XX__ , _____ ),
    PACK_6_TO_32( X___X , X___X , X___X , XXXX_ , X__X_ , _XXX_ ),
    PACK_6_TO_32( X___X , X___X , _X_X_ , X___X , X_X__ , ____X ),
    PACK_6_TO_32( X___X , X___X , __X__ , X___X , X__XX , _XXXX ),
    PACK_6_TO_32( X___X , X___X , __X__ , XXXX_ , X___X , X___X ),
    PACK_6_TO_32( _XXX_ , _XXX_ , __X__ , X____ , X_XX_ , _XXXX ),
    PACK_6_TO_32( _____ , _____ , _____ , _____ , _____ , _____ ),

    PACK_6_TO_32( __X__ , _X_X_ , X_XX_ , _X_X_ , _X_X_ , _____ ),
    PACK_6_TO_32( _____ , _____ , _____ , _____ , __X__ , _____ ),
    PACK_6_TO_32( _XXX_ , _XXX_ , _XXX_ , _XXX_ , _XXX_ , XXXX_ ),
    PACK_6_TO_32( ____X , ____X , ____X , ____X , ____X , __X_X ),
    PACK_6_TO_32( _XXXX , _XXXX , _XXXX , _XXXX , _XXXX , XXXXX ),
    PACK_6_TO_32( X___X , X___X , X___X , X___X , X___X , X_X__ ),
    PACK_6_TO_32( _XXXX , _XXXX , _XXXX , _XXXX , _XXXX , _X_XX ),
    PACK_6_TO_32( _____ , _____ , _____ , _____ , _____ , _____ ),

    PACK_6_TO_32( _____ , __X__ , __X__ , _X_X_ , _X_X_ , _X___ ),
    PACK_6_TO_32( _____ , _____ , _____ , _____ , _____ , _____ ),
    PACK_6_TO_32( _XXX_ , _XXX_ , _XXX_ , _XXX_ , _XXX_ , XX___ ),
    PACK_6_TO_32( X____ , X___X , X___X , X___X , X___X , _X___ ),
    PACK_6_TO_32( X____ , XXXX_ , XXXX_ , XXXX_ , XXXX_ , _X___ ),
    PACK_6_TO_32( X____ , X____ , X____ , X____ , X____ , _X___ ),
    PACK_6_TO_32( _XXXX , _XXXX , _XXXX , _XXXX , _XXXX , XXX__ ),
    PACK_6_TO_32( __X__ , _____ , _____ , _____ , _____ , _____ ),

    PACK_6_TO_32( _X___ , X_X__ , X_X__ , _X_X_ , X_XX_ , __X__ ),
    PACK_6_TO_32( _____ , _____ , _____ , _XX__ , _____ , _____ ),
    PACK_6_TO_32( XX___ , XX___ , XX___ , ___X_ , XXXX_ , _XXX_ ),
    PACK_6_TO_32( _X___ , _X___ , _X___ , _XXX_ , X___X , X___X ),
    PACK_6_TO_32( _X___ , _X___ , _X___ , X___X , X___X , X___X ),
    PACK_6_TO_32( _X___ , _X___ , _X___ , X___X , X___X , X___X ),
    PACK_6_TO_32( XXX__ , XXX__ , XXX__ , _XXX_ , X___X , _XXX_ ),
    PACK_6_TO_32( _____ , _____ , _____ , _____ , _____ , _____ ),

    PACK_6_TO_32( __X__ , _X_X_ , X_XX_ , _X_X_ , _____ , _____ ),
    PACK_6_TO_32( _____ , _____ , _____ , _____ , _____ , ____X ),
    PACK_6_TO_32( _XXX_ , _XXX_ , _XXX_ , _XXX_ , __X__ , _XXX_ ),
    PACK_6_TO_32( X___X , X___X , X___X , X___X , _____ , X__XX ),
    PACK_6_TO_32( X___X , X___X , X___X , X___X , XXXXX , X_X_X ),
    PACK_6_TO_32( X___X , X___X , X___X , X___X , _____ , XX__X ),
    PACK_6_TO_32( _XXX_ , _XXX_ , _XXX_ , _XXX_ , __X__ , _XXX_ ),
    PACK_6_TO_32( _____ , _____ , _____ , _____ , _____ , X____ ),

    PACK_6_TO_32( __X__ , __X__ , _X_X_ , _X_X_ , __X__ , X____ ),
    PACK_6_TO_32( _____ , _____ , _____ , _____ , _____ , X____ ),
    PACK_6_TO_32( X___X , X___X , X___X , X___X , X___X , X_XX_ ),
    PACK_6_TO_32( X___X , X___X , X___X , X___X , X___X , XX__X ),
    PACK_6_TO_32( X___X , X___X , X___X , X___X , X___X , XX__X ),
    PACK_6_TO_32( X___X , X___X , X___X , X___X , _XXXX , X_XX_ ),
    PACK_6_TO_32( _XXXX , _XXXX , _XXXX , _XXXX , ____X , X____ ),
    PACK_6_TO_32( _____ , _____ , _____ , _____ , _XXX_ , X____ ),

    PACK_6_TO_32( _X_X_ , _____ , _____ , _____ , _____ , _____ ),
    PACK_6_TO_32( _____ , _____ , _____ , _____ , _____ , _____ ),
    PACK_6_TO_32( X___X , _____ , _____ , _____ , _____ , _____ ),
    PACK_6_TO_32( X___X , _____ , _____ , _____ , _____ , _____ ),
    PACK_6_TO_32( X___X , _____ , _____ , _____ , _____ , _____ ),
    PACK_6_TO_32( _XXXX , _____ , _____ , _____ , _____ , _____ ),
    PACK_6_TO_32( ____X , _____ , _____ , _____ , _____ , _____ ),
    PACK_6_TO_32( _XXX_ , _____ , _____ , _____ , _____ , _____ )
};


void graphicsDrawChar6x8(JsGraphics *gfx, int x1, int y1, char ch, unsigned short size, bool solidBackground) {
  int idx = ((unsigned char)ch) - 33;
  if (idx<0 || idx>=LCD_FONT_6X8_CHARS) {
    // no char for this
    if (solidBackground)
      graphicsFillRect(gfx, x1, y1, x1+5*size, y1+7*size, gfx->data.bgColor);
    return;
  }

  int cidx = idx % 6;
  idx = (idx/6)*8;
  int y;
  int s = size-1;
  for (y=0;y<8;y++) {
    unsigned int line = LCD_FONT_6X8[idx + y] >> (cidx*5);
    int ly = y*size + y1;
    if (solidBackground || (line&16)) graphicsFillRect(gfx, x1+0*size, ly, x1+s+0*size, ly+s, (line&16) ? gfx->data.fgColor : gfx->data.bgColor);
    if (solidBackground || (line&8)) graphicsFillRect(gfx, x1+1*size, ly, x1+s+1*size, ly+s, (line&8) ? gfx->data.fgColor : gfx->data.bgColor);
    if (solidBackground || (line&4)) graphicsFillRect(gfx, x1+2*size, ly, x1+s+2*size, ly+s, (line&4) ? gfx->data.fgColor : gfx->data.bgColor);
    if (solidBackground || (line&2)) graphicsFillRect(gfx, x1+3*size, ly, x1+s+3*size, ly+s, (line&2) ? gfx->data.fgColor : gfx->data.bgColor);
    if (solidBackground || (line&1)) graphicsFillRect(gfx, x1+4*size, ly, x1+s+4*size, ly+s, (line&1) ? gfx->data.fgColor : gfx->data.bgColor);
  }
}

#endif // USE_FONT_6X8










