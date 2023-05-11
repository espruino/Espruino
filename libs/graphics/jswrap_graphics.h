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
 * Contains JavaScript Graphics Draw Functions
 * ----------------------------------------------------------------------------
 */

#include "jsvar.h"
#include "graphics.h"
#include "jsvariterator.h"

#ifdef GRAPHICS_PALETTED_IMAGES
// 16 color MAC OS palette
extern const uint16_t PALETTE_4BIT[16];
// 256 color 16 bit Web-safe palette
extern const uint16_t PALETTE_8BIT[256];
#endif

bool jswrap_graphics_idle();
void jswrap_graphics_init();

JsVar *jswrap_graphics_getInstance();
// For creating graphics classes
JsVar *jswrap_graphics_createArrayBuffer(int width, int height, int bpp,  JsVar *options);
JsVar *jswrap_graphics_createCallback(int width, int height, int bpp, JsVar *callback);
#ifdef USE_LCD_SDL
JsVar *jswrap_graphics_createSDL(int width, int height, int bpp);
#endif
JsVar *jswrap_graphics_createImage(JsVar *data);

int jswrap_graphics_getWidthOrHeight(JsVar *parent, bool height);
int jswrap_graphics_getBPP(JsVar *parent);
JsVar *jswrap_graphics_reset(JsVar *parent);
JsVar *jswrap_graphics_clear(JsVar *parent, bool resetState);
JsVar *jswrap_graphics_fillRect(JsVar *parent, JsVar *opt, int y1, int x2, int y2);
JsVar *jswrap_graphics_clearRect(JsVar *parent, JsVar *opt, int y1, int x2, int y2);
JsVar *jswrap_graphics_drawRect(JsVar *parent, JsVar *opt, int y1, int x2, int y2);
JsVar *jswrap_graphics_drawCircle(JsVar *parent, int x, int y, int rad);
JsVar *jswrap_graphics_drawCircleAA(JsVar *parent, int x, int y, int r);
JsVar *jswrap_graphics_fillCircle(JsVar *parent, int x, int y, int rad);
JsVar *jswrap_graphics_drawEllipse(JsVar *parent, int x, int y, int x2, int y2);
JsVar *jswrap_graphics_fillEllipse(JsVar *parent, int x, int y, int x2, int y2);
int jswrap_graphics_getPixel(JsVar *parent, int x, int y);
JsVar *jswrap_graphics_setPixel(JsVar *parent, int x, int y, JsVar *color);
unsigned int jswrap_graphics_toColor(JsVar *parent, JsVar *r, JsVar *g, JsVar *b);
unsigned int jswrap_graphics_blendColor(JsVar *parent, JsVar *ca, JsVar *cb, JsVar* amt);
JsVar *jswrap_graphics_setColorX(JsVar *parent, JsVar *r, JsVar *g, JsVar *b, bool isForeground);
JsVarInt jswrap_graphics_getColorX(JsVar *parent, bool isForeground);
JsVar *jswrap_graphics_setClipRect(JsVar *parent, int x1, int y1, int x2, int y2);
JsVar *jswrap_graphics_setFontSizeX(JsVar *parent, int size, bool isVectorFont);
JsVar *jswrap_graphics_setFontCustom(JsVar *parent, JsVar *bitmap, int firstChar, JsVar *width, int height);
JsVar *jswrap_graphics_setFontAlign(JsVar *parent, int x, int y, int r);
JsVar *jswrap_graphics_setFont(JsVar *parent, JsVar *name, int size);
JsVar *jswrap_graphics_getFont(JsVar *parent);
JsVar *jswrap_graphics_getFonts(JsVar *parent);
int jswrap_graphics_getFontHeight(JsVar *parent);
JsVar *jswrap_graphics_wrapString(JsVar *parent, JsVar *str, int maxWidth);
JsVar *jswrap_graphics_drawString(JsVar *parent, JsVar *str, int x, int y, bool solidBackground);
void jswrap_graphics_drawCString(JsGraphics *gfx, int x, int y, char *str); /// Convenience function for using drawString from C code
JsVarInt jswrap_graphics_stringWidth(JsVar *parent, JsVar *var);
JsVar* jswrap_graphics_stringMetrics(JsVar *parent, JsVar *var);
JsVar *jswrap_graphics_getVectorFontPolys(JsGraphics *gfx, JsVar *var, JsVar *options);
JsVar *jswrap_graphics_drawLine(JsVar *parent, int x1, int y1, int x2, int y2);
JsVar *jswrap_graphics_drawLineAA(JsVar *parent, double x1, double y1, double x2, double y2);
JsVar *jswrap_graphics_lineTo(JsVar *parent, int x, int y);
JsVar *jswrap_graphics_moveTo(JsVar *parent, int x, int y);
JsVar *jswrap_graphics_drawPoly_X(JsVar *parent, JsVar *poly, bool closed, bool antiAlias);
JsVar *jswrap_graphics_fillPoly_X(JsVar *parent, JsVar *poly, bool antiAlias);
JsVar *jswrap_graphics_setRotation(JsVar *parent, int rotation, bool reflect);
JsVar *jswrap_graphics_imageMetrics(JsVar *parent, JsVar *var);
JsVar *jswrap_graphics_drawImage(JsVar *parent, JsVar *image, int xPos, int yPos, JsVar *options);
JsVar *jswrap_graphics_drawImages(JsVar *parent, JsVar *layersVar, JsVar *options);
JsVar *jswrap_graphics_asImage(JsVar *parent, JsVar *imgType);
JsVar *jswrap_graphics_getModified(JsVar *parent, bool reset);
JsVar *jswrap_graphics_scroll(JsVar *parent, int x, int y);
JsVar *jswrap_graphics_blit(JsVar *parent, JsVar *options);
JsVar *jswrap_graphics_asBMP(JsVar *parent);
JsVar *jswrap_graphics_asURL(JsVar *parent);
void jswrap_graphics_dump(JsVar *parent);
JsVar *jswrap_graphics_quadraticBezier(JsVar *parent, JsVar * arr, JsVar *options);
JsVar *jswrap_graphics_transformVertices(JsVar *parent, JsVar *verts, JsVar *transformation);
JsVar *jswrap_graphics_floodFill(JsVar *parent, int x, int y, JsVar *col);
JsVar *jswrap_graphics_theme();
JsVar *jswrap_graphics_setTheme(JsVar *parent, JsVar *theme);


/// Info about an image to be used for rendering
typedef struct {
  int width, height, bpp;
  bool isTransparent;
  unsigned int transparentCol;
  JsVar *buffer; // must be unlocked!
  uint32_t bitmapOffset; // start offset in imageBuffer
  const uint16_t *palettePtr;
  uint32_t paletteMask;
  unsigned int bitMask;
  unsigned int pixelsPerByteMask;
  int stride; ///< bytes per line
  unsigned short headerLength; ///< size of header (inc palette)
  unsigned short bitmapLength; ///< size of data (excl header)

  uint16_t _simplePalette[16]; // used when a palette is created for rendering
} GfxDrawImageInfo;

void _jswrap_graphics_freeImageInfo(GfxDrawImageInfo *info);

/** Parse an image into GfxDrawImageInfo. See drawImage for image format docs. Returns true on success.
 * if 'image' is a string or ArrayBuffer, imageOffset is the offset within that (usually 0)
 */
bool _jswrap_graphics_parseImage(JsGraphics *gfx, JsVar *image, unsigned int imageOffset, GfxDrawImageInfo *info);


/// This is for rotating and scaling layers
typedef struct {
  int x1,y1,x2,y2; //x2/y2 is exclusive
  double rotate; // radians
  double scale; // 1 = 1:1, 2 = big
  bool center; // center on x1/y1 (which are then offset)
  bool repeat; // tile the image
  GfxDrawImageInfo img;
  // for rendering
  JsvStringIterator it;
  int mx,my; //< max - width and height << 8
  int sx,sy; //< iterator X increment
  int px,py; //< y iterator position
  int qx,qy; //< x iterator position
} GfxDrawImageLayer;

bool _jswrap_drawImageLayerGetPixel(GfxDrawImageLayer *l, unsigned int *result);
void _jswrap_drawImageLayerInit(GfxDrawImageLayer *l);
void _jswrap_drawImageLayerSetStart(GfxDrawImageLayer *l, int x, int y);
void _jswrap_drawImageLayerStartX(GfxDrawImageLayer *l);
void _jswrap_drawImageLayerNextX(GfxDrawImageLayer *l);
void _jswrap_drawImageLayerNextXRepeat(GfxDrawImageLayer *l);
void _jswrap_drawImageLayerNextY(GfxDrawImageLayer *l);
void _jswrap_drawImageSimple(JsGraphics *gfx, int xPos, int yPos, GfxDrawImageInfo *img, JsvStringIterator *it);
