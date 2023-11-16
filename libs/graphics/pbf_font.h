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
 * Pebble font format parser
 * ----------------------------------------------------------------------------
 */

#ifdef ESPR_PBF_FONTS
#ifndef PBF_FONT_H
#define PBF_FONT_H

#include "jsvar.h"
#include "jsvariterator.h"
#include "graphics.h"

typedef struct {
  JsVar *var;
  JsvStringIterator it;
  uint8_t version;
  uint8_t lineHeight;
  uint16_t glyphCount;
  uint16_t hashTableOffset;
  uint8_t hashTableSize;
  uint8_t codepointSize;
  uint8_t offsetTableEntrySize;
  uint32_t offsetTableOffset;
  uint32_t glyphTableOffset;
  bool hashTableValueAsTopBits;
} PbfFontLoaderInfo;

typedef struct {
  uint8_t w;
  uint8_t h;
  int8_t x;
  int8_t y;
  int8_t advance; // how wide is the character itself?
  uint8_t bpp; // bits per pixel
} PbfFontLoaderGlyph;

/// Load a .pbf file
void jspbfFontNew(PbfFontLoaderInfo *info, JsVar *font);
void jspbfFontFree(PbfFontLoaderInfo *info);

// Find the font glyph, fill PbfFontLoaderGlyph with info. Iterator is left pointing to glyph
bool jspbfFontFindGlyph(PbfFontLoaderInfo *info, int codepoint, PbfFontLoaderGlyph *result);

void jspbfFontRenderGlyph(PbfFontLoaderInfo *info, PbfFontLoaderGlyph *glyph, JsGraphics *gfx, int x, int y, bool solidBackground, int scalex, int scaley);

#endif // PBF_FONT_H
#endif // ESPR_PBF_FONTS