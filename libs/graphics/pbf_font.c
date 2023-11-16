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

#include "pbf_font.h"

// https://github.com/pebble-dev/wiki/wiki/Firmware-Font-Format

/* Notes for improvements in a new format:

 * remove value in hashtable (un-needed)
 * Smaller hashtable, might as well use ~64 or something
 * per-codepoint codepoint size?
 * Allow mix of 1bpp/2bpp/etc glyphs
 * Store all glyphs in one chunk - allow bit-level packing
*/

static uint8_t jspbfGetU8(JsvStringIterator *it) {
  return (uint8_t)jsvStringIteratorGetCharAndNext(it);
}
static uint16_t jspbfGetU16(JsvStringIterator *it) {
  return (uint16_t)((uint16_t)(uint8_t)jsvStringIteratorGetCharAndNext(it) |
                  (((uint16_t)(uint8_t)jsvStringIteratorGetCharAndNext(it))<<8));
}
static uint32_t jspbfGetU32(JsvStringIterator *it) {
  return (uint32_t)((uint32_t)(uint8_t)jsvStringIteratorGetCharAndNext(it) |
                    (((uint32_t)(uint8_t)jsvStringIteratorGetCharAndNext(it))<<8) |
                    (((uint32_t)(uint8_t)jsvStringIteratorGetCharAndNext(it))<<16) |
                    (((uint32_t)(uint8_t)jsvStringIteratorGetCharAndNext(it))<<24));
}

/// Load a .pbf file
void jspbfFontNew(PbfFontLoaderInfo *info, JsVar *font) {
  info->var = jsvLockAgain(font);
  jsvStringIteratorNew(&info->it, font, 0);
  info->version = jspbfGetU8(&info->it); // 0
  info->lineHeight = jspbfGetU8(&info->it); // 1
  info->glyphCount = jspbfGetU16(&info->it); // 2
  jspbfGetU16(&info->it); // wildcard codepoint
  info->hashTableOffset = 6;
  info->hashTableSize = 255;
  info->hashTableValueAsTopBits = false;
  info->offsetTableEntrySize = 6;  // cp=2,address=4
  info->codepointSize = 2;
  if (info->version>=2)  {
    info->hashTableOffset = 8;
    info->hashTableSize = jspbfGetU8(&info->it);
    info->codepointSize = jspbfGetU8(&info->it);
  }
  if (info->version>=3) {
    info->hashTableOffset = jspbfGetU8(&info->it);
    uint8_t features = jspbfGetU8(&info->it);
    // &1 = OffsetTable offset sizes (default 0=32,  1=16)
    // &2 = Storage of Glyphs (default 0=raw,  1=RLE4)
    // &128 = ESPRUINO ONLY: Use redundant HashTable 'value' entry as extra 8 bits of offset
    if (features & 1) info->offsetTableEntrySize = 4;  // cp=2,address=2
    if (features & 128) info->hashTableValueAsTopBits = true;
  }
  info->offsetTableOffset = (uint32_t)(info->hashTableOffset + info->hashTableSize*4);
  info->glyphTableOffset = (uint32_t)(info->offsetTableOffset + (info->glyphCount*info->offsetTableEntrySize));
}

void jspbfFontFree(PbfFontLoaderInfo *info) {
  jsvStringIteratorFree(&info->it);
  jsvUnLock(info->var);
  info->var = NULL;
}

// Find the font glyph, fill PbfFontLoaderGlyph with info. Iterator is left pointing to glyph
bool jspbfFontFindGlyph(PbfFontLoaderInfo *info, int codepoint, PbfFontLoaderGlyph *result) {
  int hash = codepoint % info->hashTableSize;
  /*
  HashTable
  0x00 u8 : Value. In all examples I encountered, this simply counts 0..255
  0x01 u8 : Offset Table Size.
  0x02 u16 : Offset Table Offset. Measured in bytes from the beginning of the offset table.
  */
  jsvStringIteratorGoto(&info->it, info->var, info->hashTableOffset + 4*hash );
  uint8_t value = jspbfGetU8(&info->it);
  uint8_t offsetTableSize = jspbfGetU8(&info->it);
  uint16_t o = jspbfGetU16(&info->it);
  uint32_t offsetTableOffset = o + info->offsetTableOffset;
  if (info->hashTableValueAsTopBits)
    offsetTableOffset += (value<<16);
  jsvStringIteratorGoto(&info->it, info->var, offsetTableOffset);
  int cp;
  while (offsetTableSize--) {
    /* Offset Table
    0x00 u16/u32 : Unicode codepoint (for example, 0x0622 for ∆).
    0x02 or 0x04 u16/u32 : Data offset.
    */
    cp = (info->codepointSize==2) ? jspbfGetU16(&info->it) : jspbfGetU32(&info->it);
    uint32_t dataOffset = (info->offsetTableEntrySize==4) ? jspbfGetU16(&info->it) : jspbfGetU32(&info->it);
    if (cp==codepoint) {
      jsvStringIteratorGoto(&info->it, info->var, info->glyphTableOffset+dataOffset);
      result->w = jspbfGetU8(&info->it);
      result->h = jspbfGetU8(&info->it);
      result->x = (int8_t)jspbfGetU8(&info->it);
      result->y = (int8_t)jspbfGetU8(&info->it);
      result->advance = (int8_t)jspbfGetU8(&info->it);
      result->bpp = 1;
      if (((uint8_t)result->advance)&128) { // extension for Espruino - if advance top bit set (eg. negative), it means we have 2 bpp data
        result->advance &= 127;
        result->bpp = 2;
      }
      return true;
    }
  }
  return false;
}

void jspbfFontRenderGlyph(PbfFontLoaderInfo *info, PbfFontLoaderGlyph *glyph, JsGraphics *gfx, int x, int y, bool solidBackground, int scalex, int scaley) {
  //bmpOffset *= ch * customBPP;
  // now render character
  int bmpOffset = 0;
  int bpp = glyph->bpp;
  int bppRange = (1<<bpp)-1;
  int cx,cy;
  int citdata = jsvStringIteratorGetCharAndNext(&info->it);
  for (cy=0;cy<glyph->h;cy++) {
    for (cx=0;cx<glyph->w;cx++) {
      int col = citdata&bppRange;
      if (solidBackground || col)
        graphicsFillRect(gfx,
            (x + cx*scalex),
            (y + cy*scaley),
            (x + cx*scalex + scalex-1),
            (y + cy*scaley + scaley-1),
            graphicsBlendGfxColor(gfx, (256*col)/bppRange));
      bmpOffset += bpp;
      citdata >>= bpp;
      if (bmpOffset>=8) {
        bmpOffset=0;
        citdata = jsvStringIteratorGetCharAndNext(&info->it);
      }
    }
  }
}

#endif // ESPR_PBF_FONTS