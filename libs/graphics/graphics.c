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
 * Graphics Draw Functions
 * ----------------------------------------------------------------------------
 */

#include "graphics.h"
#include "bitmap_font_4x6.h"


#include "jsutils.h"
#include "jsvar.h"
#include "jsparse.h"

#include "lcd_arraybuffer.h"
#include "lcd_js.h"
#ifdef USE_LCD_SDL
#include "lcd_sdl.h"
#endif
#ifdef USE_LCD_FSMC
#include "lcd_fsmc.h"
#endif
#ifdef USE_LCD_SPI
#include "lcd_spilcd.h"
#endif
#ifdef USE_LCD_ST7789_8BIT
#include "lcd_st7789_8bit.h"
#endif
#ifdef USE_LCD_MEMLCD
#include "lcd_memlcd.h"
#endif
#ifdef USE_LCD_SPI_UNBUF
#include "lcd_spi_unbuf.h"
#endif

// ----------------------------------------------------------------------------------------------

#ifdef GRAPHICS_THEME
/// Global color scheme colours
JsGraphicsTheme graphicsTheme;
#endif

#ifdef ESPR_GRAPHICS_INTERNAL
/// Internal instance of Graphics structure (eg for built-in LCD) so we don't have to store all state in a var
JsGraphics graphicsInternal;
#endif

static void graphicsSetPixelDevice(JsGraphics *gfx, int x, int y, unsigned int col);

void graphicsFallbackSetPixel(JsGraphics *gfx, int x, int y, unsigned int col) {
  NOT_USED(gfx);
  NOT_USED(x);
  NOT_USED(y);
  NOT_USED(col);
}

unsigned int graphicsFallbackGetPixel(JsGraphics *gfx, int x, int y) {
  NOT_USED(gfx);
  NOT_USED(x);
  NOT_USED(y);
  return gfx->data.bgColor;
}

void graphicsFallbackFillRect(JsGraphics *gfx, int x1, int y1, int x2, int y2, unsigned int col) {
  int x,y;
  for (y=y1;y<=y2;y++)
    for (x=x1;x<=x2;x++)
      graphicsSetPixelDevice(gfx,x,y, col);
}

void graphicsFallbackBlit(JsGraphics *gfx, int x1, int y1, int w, int h, int x2, int y2) {
  for (int y=0;y<h;y++)
    for (int x=0;x<w;x++)
      gfx->setPixel(gfx, (int)(x+x2),(int)(y+y2),
        gfx->getPixel(gfx, (int)(x+x1),(int)(y+y1)));
}

void graphicsFallbackScrollX(JsGraphics *gfx, int xdir, int yfrom, int yto, int x1, int x2) {
  int x;
  if (xdir<=0) {
    int w = x2+xdir;
    for (x=x1;x<=w;x++)
      gfx->setPixel(gfx, (int)x,(int)yto,
          gfx->getPixel(gfx, (int)(x-xdir),(int)yfrom));
  } else { // >0
    for (x=x2-xdir;x>=x1;x--)
      gfx->setPixel(gfx, (int)(x+xdir),(int)yto,
          gfx->getPixel(gfx, (int)x,(int)yfrom));
  }
}

void graphicsFallbackScroll(JsGraphics *gfx, int xdir, int ydir, int x1, int y1, int x2, int y2) {
  if (xdir==0 && ydir==0) return;
  int y;
  if (ydir<=0) {
    int h = y2+ydir;
    for (y=y1;y<=h;y++)
      graphicsFallbackScrollX(gfx, xdir, y-ydir, y, x1, x2);
  } else { // >0
    for (y=y2-ydir;y>=y1;y--)
      graphicsFallbackScrollX(gfx, xdir, y, y+ydir, x1, x2);
  }
}

// ----------------------------------------------------------------------------------------------

void graphicsStructResetState(JsGraphics *gfx) {

#ifdef GRAPHICS_THEME
  // Only set theme for OS-provided graphics (not arraybuffer/etc)
  if (gfx->data.type!=JSGRAPHICSTYPE_ARRAYBUFFER &&
      gfx->data.type!=JSGRAPHICSTYPE_JS) {
    gfx->data.fgColor = graphicsTheme.fg;
    gfx->data.bgColor = graphicsTheme.bg;
  } else
#endif
  {
    gfx->data.fgColor = 0xFFFFFFFF;
    gfx->data.bgColor = 0;
  }
  gfx->data.fontSize = 1+JSGRAPHICS_FONTSIZE_4X6;
#ifndef SAVE_ON_FLASH
  gfx->data.fontAlignX = 3;
  gfx->data.fontAlignY = 3;
  gfx->data.fontRotate = 0;
#endif
#ifndef NO_MODIFIED_AREA
  gfx->data.clipRect.x1 = 0;
  gfx->data.clipRect.y1 = 0;
  gfx->data.clipRect.x2 = (unsigned short)(gfx->data.width-1);
  gfx->data.clipRect.y2 = (unsigned short)(gfx->data.height-1);
#endif
  gfx->data.cursorX = 0;
  gfx->data.cursorY = 0;
}

void graphicsStructInit(JsGraphics *gfx, int width, int height, int bpp) {
  // type/width/height/bpp should be set elsewhere...
  gfx->data.flags = JSGRAPHICSFLAGS_NONE;
  gfx->data.width = (unsigned short)width;
  gfx->data.height = (unsigned short)height;
  gfx->data.bpp = (unsigned char)bpp;
  graphicsStructResetState(gfx);
#ifndef NO_MODIFIED_AREA
  gfx->data.modMaxX = -32768;
  gfx->data.modMaxY = -32768;
  gfx->data.modMinX = 32767;
  gfx->data.modMinY = 32767;
#endif
}

/// Set up the callbacks for this graphics instance (usually done by graphicsGetFromVar)
bool graphicsSetCallbacks(JsGraphics *gfx) {
  gfx->setPixel = graphicsFallbackSetPixel;
  gfx->getPixel = graphicsFallbackGetPixel;
  gfx->fillRect = graphicsFallbackFillRect;
  gfx->blit = graphicsFallbackBlit;
  gfx->scroll = graphicsFallbackScroll;
#ifdef USE_LCD_SDL
  if (gfx->data.type == JSGRAPHICSTYPE_SDL) {
    lcdSetCallbacks_SDL(gfx);
  } else
#endif
#ifdef USE_LCD_FSMC
  if (gfx->data.type == JSGRAPHICSTYPE_FSMC) {
    lcdSetCallbacks_FSMC(gfx);
  } else
#endif
  if (gfx->data.type == JSGRAPHICSTYPE_ARRAYBUFFER) {
    lcdSetCallbacks_ArrayBuffer(gfx);
#ifndef SAVE_ON_FLASH
  } else if (gfx->data.type == JSGRAPHICSTYPE_JS) {
    lcdSetCallbacks_JS(gfx);
#endif
#ifdef USE_LCD_SPI
  } else if (gfx->data.type == JSGRAPHICSTYPE_SPILCD) {
    lcdSetCallbacks_SPILCD(gfx);
#endif
#ifdef USE_LCD_ST7789_8BIT
  } else if (gfx->data.type == JSGRAPHICSTYPE_ST7789_8BIT) {
    lcdST7789_setCallbacks(gfx);
#endif
#ifdef USE_LCD_MEMLCD
  } else if (gfx->data.type == JSGRAPHICSTYPE_MEMLCD) {
    lcdMemLCD_setCallbacks(gfx);
#endif
#ifdef USE_LCD_SPI_UNBUF
  } else if (gfx->data.type == JSGRAPHICSTYPE_LCD_SPI_UNBUF) {
    lcd_spi_unbuf_setCallbacks(gfx);
#endif
  } else {
    jsExceptionHere(JSET_INTERNALERROR, "Unknown graphics type\n");
    assert(0);
    return false;
  }

  return true;
}

bool graphicsGetFromVar(JsGraphics *gfx, JsVar *parent) {
  gfx->graphicsVar = parent;
  // jsvObjectGetChild can handle parent==NULL
  JsVar *data = jsvObjectGetChild(parent, JS_HIDDEN_CHAR_STR"gfx", 0);
#if ESPR_GRAPHICS_INTERNAL
  if (!data) {
    *gfx = graphicsInternal;
    return true;
  }
#endif
  assert(data);
  if (data) {
    jsvGetStringChars(data,0,(char*)&gfx->data, sizeof(JsGraphicsData));
    jsvUnLock(data);
    return graphicsSetCallbacks(gfx);
  } else
    return false;
}

// Set the data variable for graphics - called initially when a graphics instance is first make
void graphicsSetVarInitial(JsGraphics *gfx) {
  JsVar *dataname = jsvFindChildFromString(gfx->graphicsVar, JS_HIDDEN_CHAR_STR"gfx", true);
  JsVar *data = jsvSkipName(dataname);
  if (!data) {
    data = jsvNewStringOfLength(sizeof(JsGraphicsData), NULL);
    jsvSetValueOfName(dataname, data);
  }
  jsvUnLock(dataname);
  assert(data);
  jsvSetString(data, (char*)&gfx->data, sizeof(JsGraphicsData));
  jsvUnLock(data);
}

// Set the data variable for graphics - graphics data must exist
void graphicsSetVar(JsGraphics *gfx) {
  JsVar *data = jsvSkipNameAndUnLock(jsvFindChildFromString(gfx->graphicsVar, JS_HIDDEN_CHAR_STR"gfx", false));
#if ESPR_GRAPHICS_INTERNAL
  if (!data) {
    graphicsInternal = *gfx;
    return;
  }
#endif
  if (data) {
    jsvSetString(data, (char*)&gfx->data, sizeof(JsGraphicsData));
    jsvUnLock(data);
  }
}

/// Get the memory requires for this graphics's pixels if everything was packed as densely as possible
size_t graphicsGetMemoryRequired(const JsGraphics *gfx) {
  return (size_t)(gfx->data.width * gfx->data.height * gfx->data.bpp + 7) >> 3;
};

// ----------------------------------------------------------------------------------------------

// If graphics is flipped or rotated then the coordinates need modifying
void graphicsToDeviceCoordinates(const JsGraphics *gfx, int *x, int *y) {
  if (gfx->data.flags & JSGRAPHICSFLAGS_SWAP_XY) {
    int t = *x;
    *x = *y;
    *y = t;
  }
  if (gfx->data.flags & JSGRAPHICSFLAGS_INVERT_X) *x = (int)(gfx->data.width - (*x+1));
  if (gfx->data.flags & JSGRAPHICSFLAGS_INVERT_Y) *y = (int)(gfx->data.height - (*y+1));
}

// If graphics is flipped or rotated then the coordinates need modifying. This is to go back - eg for touchscreens
void deviceToGraphicsCoordinates(const JsGraphics *gfx, int *x, int *y) {
  if (gfx->data.flags & JSGRAPHICSFLAGS_INVERT_X) *x = (int)(gfx->data.width - (*x+1));
  if (gfx->data.flags & JSGRAPHICSFLAGS_INVERT_Y) *y = (int)(gfx->data.height - (*y+1));
  if (gfx->data.flags & JSGRAPHICSFLAGS_SWAP_XY) {
    int t = *x;
    *x = *y;
    *y = t;
  }
}

// If graphics is flipped or rotated then the coordinates need modifying
void graphicsToDeviceCoordinates16x(const JsGraphics *gfx, int *x, int *y) {
  if (gfx->data.flags & JSGRAPHICSFLAGS_SWAP_XY) {
    int t = *x;
    *x = *y;
    *y = t;
  }
  if (gfx->data.flags & JSGRAPHICSFLAGS_INVERT_X) *x = (int)((gfx->data.width-1)*16 - *x);
  if (gfx->data.flags & JSGRAPHICSFLAGS_INVERT_Y) *y = (int)((gfx->data.height-1)*16 - *y);
}

unsigned short graphicsGetWidth(const JsGraphics *gfx) {
  return (gfx->data.flags & JSGRAPHICSFLAGS_SWAP_XY) ? gfx->data.height : gfx->data.width;
}
unsigned short graphicsGetHeight(const JsGraphics *gfx) {
  return (gfx->data.flags & JSGRAPHICSFLAGS_SWAP_XY) ? gfx->data.width : gfx->data.height;
}

// Set the area modified by a draw command and also clip to the screen/clipping bounds
bool graphicsSetModifiedAndClip(JsGraphics *gfx, int *x1, int *y1, int *x2, int *y2) {
  bool modified = false;
#ifndef NO_MODIFIED_AREA
  if (*x1<gfx->data.clipRect.x1) { *x1 = gfx->data.clipRect.x1; modified = true; }
  if (*y1<gfx->data.clipRect.y1) { *y1 = gfx->data.clipRect.y1; modified = true; }
  if (*x2>gfx->data.clipRect.x2) { *x2 = gfx->data.clipRect.x2; modified = true; }
  if (*y2>gfx->data.clipRect.y2) { *y2 = gfx->data.clipRect.y2; modified = true; }
  if (*x1 < gfx->data.modMinX) { gfx->data.modMinX=(short)*x1; modified = true; }
  if (*x2 > gfx->data.modMaxX) { gfx->data.modMaxX=(short)*x2; modified = true; }
  if (*y1 < gfx->data.modMinY) { gfx->data.modMinY=(short)*y1; modified = true; }
  if (*y2 > gfx->data.modMaxY) { gfx->data.modMaxY=(short)*y2; modified = true; }
#else
  if (*x1<0) { *x1 = 0; modified = true; }
  if (*y1<0) { *y1 = 0; modified = true; }
  if (*x2>=gfx->data.width) { *x2 = gfx->data.width-1; modified = true; }
  if (*y2>=gfx->data.height) { *y2 = gfx->data.height-1; modified = true; }
#endif
  return modified;
}

// Set the area modified by a draw command
void graphicsSetModified(JsGraphics *gfx, int x1, int y1, int x2, int y2) {
#ifndef NO_MODIFIED_AREA
  if (x1 < gfx->data.modMinX) { gfx->data.modMinX=(short)x1; }
  if (x2 > gfx->data.modMaxX) { gfx->data.modMaxX=(short)x2; }
  if (y1 < gfx->data.modMinY) { gfx->data.modMinY=(short)y1; }
  if (y2 > gfx->data.modMaxY) { gfx->data.modMaxY=(short)y2; }
#endif
}

/// Get a setPixel function (assuming coordinates already clipped with graphicsSetModifiedAndClip) - if all is ok it can choose a faster draw function
JsGraphicsSetPixelFn graphicsGetSetPixelFn(JsGraphics *gfx) {
  if (gfx->data.flags & JSGRAPHICSFLAGS_MAPPEDXY)
    return graphicsSetPixel; // fallback
  else
    return gfx->setPixel; // fast
}

/// Get a setPixel function (assuming no clipping by caller) - if all is ok it can choose a faster draw function
JsGraphicsSetPixelFn graphicsGetSetPixelUnclippedFn(JsGraphics *gfx, int x1, int y1, int x2, int y2) {
  if ((gfx->data.flags & JSGRAPHICSFLAGS_MAPPEDXY) ||
      graphicsSetModifiedAndClip(gfx,&x1,&y1,&x2,&y2))
    return graphicsSetPixel; // fallback
  else
    return gfx->setPixel; // fast
}

/// Merge one color into another based on current bit depth (amt is 0..256)
uint32_t graphicsBlendColor(JsGraphics *gfx, unsigned int fg, unsigned int bg, int iamt) {
  unsigned int amt = (iamt>0) ? (unsigned)iamt : 0;
  if (amt>256) amt=256;
  if (gfx->data.bpp==2 || gfx->data.bpp==4 || gfx->data.bpp==8) {
    // TODO: if our graphics instance is paletted this isn't correct!
    return (bg*(256-amt) + fg*amt) >> 8;
  } else if (gfx->data.bpp==16) { // Blend from bg to fg
    unsigned int b = bg;
    unsigned int br = (b>>11)&0x1F;
    unsigned int bg = (b>>5)&0x3F;
    unsigned int bb = b&0x1F;
    unsigned int f = fg;
    unsigned int fr = (f>>11)&0x1F;
    unsigned int fg = (f>>5)&0x3F;
    unsigned int fb = f&0x1F;
    unsigned int ri = (br*(256-amt) + fr*amt) >> 8;
    unsigned int gi = (bg*(256-amt) + fg*amt) >> 8;
    unsigned int bi = (bb*(256-amt) + fb*amt) >> 8;
    return (bi | gi<<5 | ri<<11);
#ifdef ESPR_GRAPHICS_12BIT
  } else if (gfx->data.bpp==12) { // Blend from bg to fg
    unsigned int b = bg;
    unsigned int br = (b>>8)&0x0F;
    unsigned int bg = (b>>4)&0x0F;
    unsigned int bb = b&0x0F;
    unsigned int f = fg;
    unsigned int fr = (f>>8)&0x0F;
    unsigned int fg = (f>>4)&0x0F;
    unsigned int fb = f&0x0F;
    unsigned int ri = (br*(256-amt) + fr*amt) >> 8;
    unsigned int gi = (bg*(256-amt) + fg*amt) >> 8;
    unsigned int bi = (bb*(256-amt) + fb*amt) >> 8;
    return (bi | gi<<4 | ri<<8);
#endif
  } else if (gfx->data.bpp==24) { // Blend from bg to fg
    unsigned int b = bg;
    unsigned int br = (b>>16)&0xFF;
    unsigned int bg = (b>>8)&0xFF;
    unsigned int bb = b&0xFF;
    unsigned int f = fg;
    unsigned int fr = (f>>16)&0xFF;
    unsigned int fg = (f>>8)&0xFF;
    unsigned int fb = f&0xFF;
    unsigned int ri = (br*(256-amt) + fr*amt) >> 8;
    unsigned int gi = (bg*(256-amt) + fg*amt) >> 8;
    unsigned int bi = (bb*(256-amt) + fb*amt) >> 8;
    return (bi | gi<<8 | ri<<16);
  }
  // Memory LCD is 3 bit, so best bet is just to choose one or the other
  return (amt>=128) ? fg : bg;
}

/// Merge one color into another based on current bit depth (amt is 0..256)
uint32_t graphicsBlendGfxColor(JsGraphics *gfx, int iamt) {
  return graphicsBlendColor(gfx, gfx->data.fgColor, gfx->data.bgColor, iamt);
}

// ----------------------------------------------------------------------------------------------

static void graphicsSetPixelDevice(JsGraphics *gfx, int x, int y, unsigned int col) {
#ifndef NO_MODIFIED_AREA
  if (x<gfx->data.clipRect.x1 ||
      y<gfx->data.clipRect.y1 ||
      x>gfx->data.clipRect.x2 ||
      y>gfx->data.clipRect.y2) return;
  if (x < gfx->data.modMinX) gfx->data.modMinX=(short)x;
  if (x > gfx->data.modMaxX) gfx->data.modMaxX=(short)x;
  if (y < gfx->data.modMinY) gfx->data.modMinY=(short)y;
  if (y > gfx->data.modMaxY) gfx->data.modMaxY=(short)y;
#else
  if (x<0 || y<0 || x>=gfx->data.width || y>=gfx->data.height) return;
#endif
  gfx->setPixel(gfx,(int)x,(int)y,col & (unsigned int)((1L<<gfx->data.bpp)-1));
}

static unsigned int graphicsGetPixelDevice(JsGraphics *gfx, int x, int y) {
  if (x<0 || y<0 || x>=gfx->data.width || y>=gfx->data.height) return 0;
  return gfx->getPixel(gfx, x, y);
}

/// For Antialiasing - blends between FG and BG colors
static void graphicsSetPixelDeviceBlended(JsGraphics *gfx, int x, int y, int amt) {
  unsigned int bg = graphicsGetPixelDevice(gfx, x, y);
  unsigned int col = graphicsBlendColor(gfx, gfx->data.fgColor, bg, amt);
  graphicsSetPixelDevice(gfx, x, y, col);
}

void graphicsFillRectDevice(JsGraphics *gfx, int x1, int y1, int x2, int y2, unsigned int col) {
  if (x1>x2) {
    int t = x1;
    x1 = x2;
    x2 = t;
  }
  if (y1>y2) {
    int t = y1;
    y1 = y2;
    y2 = t;
  }
#ifdef SAVE_ON_FLASH
  if (x1<0) x1 = 0;
  if (y1<0) y1 = 0;
  if (x2>=gfx->data.width) x2 = gfx->data.width - 1;
  if (y2>=gfx->data.height) y2 = gfx->data.height - 1;
#else
  if (x1<gfx->data.clipRect.x1) x1 = gfx->data.clipRect.x1;
  if (y1<gfx->data.clipRect.y1) y1 = gfx->data.clipRect.y1;
  if (x2>gfx->data.clipRect.x2) x2 = gfx->data.clipRect.x2;
  if (y2>gfx->data.clipRect.y2) y2 = gfx->data.clipRect.y2;
#endif
  if (x2<x1 || y2<y1) return; // nope
#ifndef NO_MODIFIED_AREA
  if (x1 < gfx->data.modMinX) gfx->data.modMinX=(short)x1;
  if (x2 > gfx->data.modMaxX) gfx->data.modMaxX=(short)x2;
  if (y1 < gfx->data.modMinY) gfx->data.modMinY=(short)y1;
  if (y2 > gfx->data.modMaxY) gfx->data.modMaxY=(short)y2;
#endif
  if (x1==x2 && y1==y2) {
    gfx->setPixel(gfx,(int)x1,(int)y1,col);
    return;
  }

  return gfx->fillRect(gfx, (int)x1, (int)y1, (int)x2, (int)y2, col);
}

// ----------------------------------------------------------------------------------------------

void graphicsSetPixel(JsGraphics *gfx, int x, int y, unsigned int col) {
  graphicsToDeviceCoordinates(gfx, &x, &y);
  graphicsSetPixelDevice(gfx, x, y, col);
}

unsigned int graphicsGetPixel(JsGraphics *gfx, int x, int y) {
  graphicsToDeviceCoordinates(gfx, &x, &y);
  return graphicsGetPixelDevice(gfx, x, y);
}

void graphicsFillRect(JsGraphics *gfx, int x1, int y1, int x2, int y2, unsigned int col) {
  graphicsToDeviceCoordinates(gfx, &x1, &y1);
  graphicsToDeviceCoordinates(gfx, &x2, &y2);
  graphicsFillRectDevice(gfx, x1, y1, x2, y2, col);
}

void graphicsClear(JsGraphics *gfx) {
  graphicsFillRectDevice(gfx,0,0,(int)(gfx->data.width-1),(int)(gfx->data.height-1), gfx->data.bgColor);
}

// ----------------------------------------------------------------------------------------------


void graphicsDrawRect(JsGraphics *gfx, int x1, int y1, int x2, int y2) {
  graphicsToDeviceCoordinates(gfx, &x1, &y1);
  graphicsToDeviceCoordinates(gfx, &x2, &y2);
  // rather than writing pixels, we use fillrect - as it is faster
  graphicsFillRectDevice(gfx,x1,y1,x2,y1,gfx->data.fgColor);
  graphicsFillRectDevice(gfx,x2,y1,x2,y2,gfx->data.fgColor);
  graphicsFillRectDevice(gfx,x1,y2,x2,y2,gfx->data.fgColor);
  graphicsFillRectDevice(gfx,x1,y2,x1,y1,gfx->data.fgColor);
}

void graphicsDrawEllipse(JsGraphics *gfx, int posX1, int posY1, int posX2, int posY2){
  graphicsToDeviceCoordinates(gfx, &posX1, &posY1);
  graphicsToDeviceCoordinates(gfx, &posX2, &posY2);
  if (posX1>posX2) {
    int t=posX1;posX1=posX2;posX2=t;
  }
  if (posY1>posY2) {
    int t=posY1;posY1=posY2;posY2=t;
  }

  int posX =  (posX1+posX2)/2;
  int posY =  (posY1+posY2)/2;
  int a = (posX2-posX1)/2;
  int b = (posY2-posY1)/2;
  int dx = 0;
  int dy = b;
  int a2 = a*a;
  int b2 = b*b;
  int err = b2-(2*b-1)*a2;
  int e2;
  bool changed = false;

  do {
    changed = false;
    graphicsSetPixelDevice(gfx,posX+dx,posY+dy,gfx->data.fgColor);
    graphicsSetPixelDevice(gfx,posX-dx,posY+dy,gfx->data.fgColor);
    graphicsSetPixelDevice(gfx,posX+dx,posY-dy,gfx->data.fgColor);
    graphicsSetPixelDevice(gfx,posX-dx,posY-dy,gfx->data.fgColor);
    e2 = 2*err;
    if (e2 <  (2*dx+1)*b2) { dx++; err += (2*dx+1)*b2; changed=true; }
    if (e2 > -(2*dy-1)*a2) { dy--; err -= (2*dy-1)*a2; changed=true; }
  } while (changed && dy >= 0);

  while (dx++ < a) { /* erroneous termination in flat ellipses (b=1) */
       graphicsSetPixelDevice(gfx,posX+dx,posY,gfx->data.fgColor);
       graphicsSetPixelDevice(gfx,posX-dx,posY,gfx->data.fgColor);
  }
}

void graphicsFillEllipse(JsGraphics *gfx, int posX1, int posY1, int posX2, int posY2){
  graphicsToDeviceCoordinates(gfx, &posX1, &posY1);
  graphicsToDeviceCoordinates(gfx, &posX2, &posY2);
  if (posX1>posX2) {
    int t=posX1;posX1=posX2;posX2=t;
  }
  if (posY1>posY2) {
    int t=posY1;posY1=posY2;posY2=t;
  }

  int posX =  (posX1+posX2)/2;
  int posY =  (posY1+posY2)/2;
  int a = (posX2-posX1)/2;
  int b = (posY2-posY1)/2;
  int dx = 0;
  int dy = b;
  int a2 = a*a;
  int b2 = b*b;
  int err = b2-(2*b-1)*a2;
  int e2;
  bool changed = false;
  do {
    changed = false;
    e2 = 2*err;
    if (e2 <  (2*dx+1)*b2) { dx++; err += (2*dx+1)*b2; changed=true; }
    if (e2 > -(2*dy-1)*a2) {
      // draw only just before we change Y, to avoid a bunch of overdraw
      graphicsFillRectDevice(gfx,posX+dx,posY+dy,posX-dx,posY+dy,gfx->data.fgColor);
      graphicsFillRectDevice(gfx,posX+dx,posY-dy,posX-dx,posY-dy,gfx->data.fgColor);
      dy--; err -= (2*dy-1)*a2; changed=true;
    }
  } while (changed && dy >= 0);

  while (dx++ < a) { /* erroneous termination in flat ellipses(b=1) */
     graphicsFillRectDevice(gfx,posX+dx,posY,posX-dx,posY,gfx->data.fgColor );
  }
}

void graphicsFillAnnulus(JsGraphics *gfx, int x0, int y0, int r1, int r2, unsigned short quadrants){
  graphicsToDeviceCoordinates(gfx, &x0, &y0);
  int x = 0;
  int y1 = r1;
  int y2 = r2;
  int d1 = 3-2*r1;
  int d2 = 3-2*r2;
  do {
    if (quadrants & 0x01) {   // Will currently overdraw into other quadrants if r1 <= (r2/2)
      graphicsFillRectDevice(gfx, x0+x, y0-y1, x0+x, y0-y2, gfx->data.fgColor);
      graphicsFillRectDevice(gfx, x0+y1, y0-x, x0+y2, y0-x, gfx->data.fgColor);
    }
    if (quadrants & 0x02) {
      graphicsFillRectDevice(gfx, x0+x, y0+y1, x0+x, y0+y2, gfx->data.fgColor);
      graphicsFillRectDevice(gfx, x0+y1, y0+x, x0+y2, y0+x, gfx->data.fgColor);
    }
    if (quadrants & 0x04) {
      graphicsFillRectDevice(gfx, x0-x, y0+y1, x0-x, y0+y2, gfx->data.fgColor);
      graphicsFillRectDevice(gfx, x0-y1, y0+x, x0-y2, y0+x, gfx->data.fgColor);
    }
    if (quadrants & 0x08) {
      graphicsFillRectDevice(gfx, x0-x, y0-y1, x0-x, y0-y2, gfx->data.fgColor);
      graphicsFillRectDevice(gfx, x0-y1, y0-x, x0-y2, y0-x, gfx->data.fgColor);
    }
    x++;
    if (d1 > 0) {
      y1--;
      d1 += 4*(x-y1)+10;
    }
    else d1 += 4*x+6;
    if (d2 > 0) {
      y2--;
      d2 += 4*(x-y2)+10;
    }
    else d2 += 4*x+6;
  } while (y2 >= x);
}

void graphicsDrawLine(JsGraphics *gfx, int x1, int y1, int x2, int y2) {
  graphicsToDeviceCoordinates(gfx, &x1, &y1);
  graphicsToDeviceCoordinates(gfx, &x2, &y2);

  int xl = x2-x1;
  int yl = y2-y1;
  if (xl<0) xl=-xl; else if (xl==0) xl=1;
  if (yl<0) yl=-yl; else if (yl==0) yl=1;
  if (xl > yl) { // longer in X - scan in X
    if (x1>x2) {
      int t;
      t = x1; x1 = x2; x2 = t;
      t = y1; y1 = y2; y2 = t;
    }
    int pos = (y1<<8) + 128; // rounding!
    int step = ((y2-y1)<<8) / xl;
    int x;
    for (x=x1;x<=x2;x++) {
      graphicsSetPixelDevice(gfx, x, pos>>8, gfx->data.fgColor);
      pos += step;
    }
  } else {
    if (y1>y2) {
      int t;
      t = x1; x1 = x2; x2 = t;
      t = y1; y1 = y2; y2 = t;
    }
    int pos = (x1<<8) + 128; // rounding!
    int step = ((x2-x1)<<8) / yl;
    int y;
    for (y=y1;y<=y2;y++) {
      graphicsSetPixelDevice(gfx, pos>>8, y, gfx->data.fgColor);
      pos += step;
    }
  }
}

#ifdef GRAPHICS_ANTIALIAS
// In 16x accuracy
void graphicsDrawLineAA(JsGraphics *gfx, int ix1, int iy1, int ix2, int iy2) {
  // https://en.wikipedia.org/wiki/Xiaolin_Wu%27s_line_algorithm
  graphicsToDeviceCoordinates16x(gfx, &ix1, &iy1);
  graphicsToDeviceCoordinates16x(gfx, &ix2, &iy2);
  int x0 = ix1*16;
  int y0 = iy1*16;
  int x1 = ix2*16;
  int y1 = iy2*16;
  bool steep = abs(y1 - y0) > abs(x1 - x0);
  if (steep) {
    int t;
    t=x0;x0=y0;y0=t;
    t=x1;x1=y1;y1=t;
  }
  if (x0 > x1) {
    int t;
    t=x0;x0=x1;x1=t;
    t=y0;y0=y1;y1=t;
  }
  int dx = x1 - x0;
  int dy = y1 - y0;
  int gradient = dx ? ((dy<<8) / dx) : 256;
  // handle first endpoint
  int xend = x0 & ~255;
  int yend = y0 + ((gradient * (xend - x0)) >> 8);
  int xgap = 255 - (x0 & 255);
  int xpxl1 = xend >> 8; // this will be used in the main loop
  int ypxl1 = yend >> 8;
  int c = yend & 255;
  if (steep) {
    graphicsSetPixelDeviceBlended(gfx, ypxl1,   xpxl1, ((256-c)*xgap)>>8);
    graphicsSetPixelDeviceBlended(gfx, ypxl1+1, xpxl1, (c*xgap)>>8);
  } else {
    graphicsSetPixelDeviceBlended(gfx, xpxl1, ypxl1, ((256-c)*xgap)>>8);
    graphicsSetPixelDeviceBlended(gfx, xpxl1, ypxl1+1, (c*xgap)>>8);
  }

  int intery = yend + gradient; // first y-intersection for the main loop
  // handle second endpoint
  xend = (x1+256) & ~255;
  yend = y1 + ((gradient * (xend - x1)) >> 8);
  xgap = (x1+256) & 255;
  int xpxl2 = xend>>8; //this will be used in the main loop
  int ypxl2 = yend>>8;
  c = yend & 255;
  if (steep) {
    graphicsSetPixelDeviceBlended(gfx, ypxl2  , xpxl2, ((256-c)*xgap)>>8);
    graphicsSetPixelDeviceBlended(gfx, ypxl2+1, xpxl2, (c*xgap)>>8);
  } else {
    graphicsSetPixelDeviceBlended(gfx, xpxl2, ypxl2,  ((256-c)*xgap)>>8);
    graphicsSetPixelDeviceBlended(gfx, xpxl2, ypxl2+1, (c*xgap)>>8);
  }
  // main loop
  for (int x=xpxl1+1;x<xpxl2;x++) {
    int y = intery>>8;
    c = intery & 255;
    if (steep) {
      graphicsSetPixelDeviceBlended(gfx, y  , x, 256-c);
      graphicsSetPixelDeviceBlended(gfx, y+1, x,  c);
    } else {
      graphicsSetPixelDeviceBlended(gfx, x, y,  256-c);
      graphicsSetPixelDeviceBlended(gfx, x, y+1, c);
    }
    intery += gradient;
  }
}

void graphicsDrawCircleAA(JsGraphics *gfx, int x0, int y0, int r){
  graphicsToDeviceCoordinates(gfx, &x0, &y0);
  int x = -r;
  int y = 0;
  int err = 2-2*r;
  int i, x2, e2;
  r = 1-err;
  do {
     i = 255-255*abs(err-2*(x+y)-2)/r;  
     graphicsSetPixelDeviceBlended(gfx, x0-x, y0+y, i);
     graphicsSetPixelDeviceBlended(gfx, x0-y, y0-x, i);
     graphicsSetPixelDeviceBlended(gfx, x0+x, y0-y, i);
     graphicsSetPixelDeviceBlended(gfx, x0+y, y0+x, i);
     e2 = err;
     x2 = x;
     if (err+y > 0) {               // X step
        i = 255-255*(err-2*x-1)/r;  // Outward pixel
        if (i > 0) {
           graphicsSetPixelDeviceBlended(gfx, x0-x, y0+y+1, i);
           graphicsSetPixelDeviceBlended(gfx, x0-y-1, y0-x, i);
           graphicsSetPixelDeviceBlended(gfx, x0+x, y0-y-1, i);
           graphicsSetPixelDeviceBlended(gfx, x0+y+1, y0+x, i);
        }
        err += ++x*2+1;
     }
     if (e2+x2 <= 0) {              // Y step
        i = 255-255*(2*y+3-e2)/r;   // Inward pixel
        if (i > 0) {
           graphicsSetPixelDeviceBlended(gfx, x0-x2-1, y0+y, i);
           graphicsSetPixelDeviceBlended(gfx, x0-y, y0-x2-1, i);
           graphicsSetPixelDeviceBlended(gfx, x0+x2+1, y0-y, i);
           graphicsSetPixelDeviceBlended(gfx, x0+y, y0+x2+1, i);
        }
        err += ++y*2+1;
     }
  } while (x < 0);
}

#endif

// Fill poly - each member of vertices is 1/16th pixel
void graphicsFillPoly(JsGraphics *gfx, int points, short *vertices) {
  typedef struct {
    short x,y;
  } VertXY;
  VertXY *v = (VertXY*)vertices;

  int i,j,y;
  int miny = (int)(gfx->data.height-1);
  int maxy = 0;
  for (i=0;i<points;i++) {
    // convert into device coordinates...
    int vx = v[i].x;
    int vy = v[i].y;
    graphicsToDeviceCoordinates16x(gfx, &vx, &vy);
    v[i].x = (short)vx;
    v[i].y = (short)vy;
    // work out min and max
    short y = v[i].y>>4;
    if (y<miny) miny=y;
    if (y>maxy) maxy=y;
  }
#ifndef SAVE_ON_FLASH
  if (miny < gfx->data.clipRect.y1) miny=gfx->data.clipRect.y1;
  if (maxy > gfx->data.clipRect.y2) maxy=gfx->data.clipRect.y2;
#else
  if (miny<0) miny=0;
  if (maxy>=gfx->data.height) maxy=(int)(gfx->data.height-1);
#endif

  const int MAX_CROSSES = 64;

  // for each scanline
  for (y=miny<<4;y<=maxy<<4;y+=16) {
    int yl = y>>4;
    short cross[MAX_CROSSES];
    bool slopes[MAX_CROSSES];
    int crosscnt = 0;
    // work out all the times lines cross the scanline
    j = points-1;
    for (i=0;i<points;i++) {
      if ((v[i].y<=y && v[j].y>y) || (v[j].y<=y && v[i].y>y)) {
        if (crosscnt < MAX_CROSSES) {
          int l = v[j].y - v[i].y;
          if (l) { // don't do horiz lines - rely on the ends of the lines that join onto them
            cross[crosscnt] = (short)(v[i].x + ((y - v[i].y) * (v[j].x-v[i].x)) / l);
            slopes[crosscnt] = (l>1)?1:0;
            crosscnt++;
          }
        }
      }
      j = i;
    }

    // bubble sort
    for (i=0;i<crosscnt-1;) {
      if (cross[i]>cross[i+1]) {
        short t=cross[i];
        cross[i]=cross[i+1];
        cross[i+1]=t;
        bool ts=slopes[i];
        slopes[i]=slopes[i+1];
        slopes[i+1]=ts;
        if (i) i--;
      } else i++;
    }

    //  Fill the pixels between node pairs.
    int x = 0,s = 0;
    for (i=0;i<crosscnt;i++) {
      if (s==0) x=cross[i];
      if (slopes[i]) s++; else s--;
      if (!s || i==crosscnt-1) {
        int x1 = (x+15)>>4;
        int x2 = (cross[i]+15)>>4;
        if (x2>x1) graphicsFillRectDevice(gfx,x1,yl,x2-1,yl,gfx->data.fgColor);
      }
      if (jspIsInterrupted()) break;
    }
  }
}

/// Draw a simple 1bpp image in foreground colour
void graphicsDrawImage1bpp(JsGraphics *gfx, int x1, int y1, int width, int height, const unsigned char *pixelData) {
  int pixel = 256|*(pixelData++);
  int x,y;
  for (y=y1;y<y1+height;y++) {
    for (x=x1;x<x1+width;x++) {
      if (pixel&128) graphicsSetPixelDevice(gfx, x, y, gfx->data.fgColor);
      pixel = pixel<<1;
      if (pixel&65536) pixel = 256|*(pixelData++);
    }
  }
}

/// Scroll the graphics device (in user coords). X>0 = to right, Y >0 = down
void graphicsScroll(JsGraphics *gfx, int xdir, int ydir) {
  // Ensure we flip coordinate system if needed
  int x1 = 0, y1 = 0;
  int x2 = xdir, y2 = ydir;
  graphicsToDeviceCoordinates(gfx, &x1, &y1);
  graphicsToDeviceCoordinates(gfx, &x2, &y2);
  xdir = x2-x1;
  ydir = y2-y1;
  // range check - if too big no point scrolling
  bool scroll = true;
  if (xdir>gfx->data.width) { xdir=gfx->data.width; scroll=false; }
  if (xdir<-gfx->data.width) { xdir=-gfx->data.width; scroll=false; }
  if (ydir>gfx->data.height) { ydir=gfx->data.height; scroll=false; }
  if (ydir<-gfx->data.height) { ydir=-gfx->data.height; scroll=false; }
  // do the scrolling
#ifdef NO_MODIFIED_AREA
  x1=0;
  y1=0;
  x2=gfx->data.width-1;
  x2=gfx->data.height-1;
#else
  x1=gfx->data.clipRect.x1;
  y1=gfx->data.clipRect.y1;
  x2=gfx->data.clipRect.x2;
  y2=gfx->data.clipRect.y2;
#endif
  if (scroll) gfx->scroll(gfx, xdir, ydir, x1,y1,x2,y2);
  graphicsSetModified(gfx, x1,y1,x2,y2);
  // fill the new area
  if (xdir>0) gfx->fillRect(gfx,x1,y1,x1+xdir-1,y2, gfx->data.bgColor);
  else if (xdir<0) gfx->fillRect(gfx,x2+1+xdir,y1,x2,y2, gfx->data.bgColor);
  if (ydir>0) gfx->fillRect(gfx,x1,y1,x2,y1+ydir-1, gfx->data.bgColor);
  else if (ydir<0) gfx->fillRect(gfx,x1,y2+1+ydir,x2,y2, gfx->data.bgColor);
}

static void graphicsDrawString(JsGraphics *gfx, int x1, int y1, const char *str) {
  // no need to modify coordinates as setPixel does that
  while (*str) {
    graphicsDrawChar4x6(gfx,x1,y1,*(str++),1,1,false);
    x1 = (int)(x1 + 4);
  }
}

// Splash screen
void graphicsSplash(JsGraphics *gfx) {
  graphicsClear(gfx);
  graphicsDrawString(gfx,0,0,"Espruino "JS_VERSION);
  graphicsDrawString(gfx,0,6,"  Embedded JavaScript");
  graphicsDrawString(gfx,0,12,"  www.espruino.com");
}

void graphicsIdle() {
#ifdef USE_LCD_SDL
  lcdIdle_SDL();
#endif
}
