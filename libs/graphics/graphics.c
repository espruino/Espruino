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
#ifndef SAVE_ON_FLASH
#include "vector_font.h"
#endif
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

// ----------------------------------------------------------------------------------------------

static void graphicsSetPixelDevice(JsGraphics *gfx, short x, short y, unsigned int col);

void graphicsFallbackSetPixel(JsGraphics *gfx, short x, short y, unsigned int col) {
  NOT_USED(gfx);
  NOT_USED(x);
  NOT_USED(y);
  NOT_USED(col);
}

unsigned int graphicsFallbackGetPixel(JsGraphics *gfx, short x, short y) {
  NOT_USED(gfx);
  NOT_USED(x);
  NOT_USED(y);
  return 0;
}

void graphicsFallbackFillRect(JsGraphics *gfx, short x1, short y1, short x2, short y2) {
  short x,y;
  for (y=y1;y<=y2;y++)
    for (x=x1;x<=x2;x++)
      graphicsSetPixelDevice(gfx,x,y, gfx->data.fgColor);
}

// ----------------------------------------------------------------------------------------------

bool graphicsGetFromVar(JsGraphics *gfx, JsVar *parent) {
  gfx->graphicsVar = parent;
  JsVar *data = jsvObjectGetChild(parent, JS_HIDDEN_CHAR_STR"gfx", 0);
  assert(data);
  if (data) {
    jsvGetString(data, (char*)&gfx->data, sizeof(JsGraphicsData)+1/*trailing zero*/);
    jsvUnLock(data);
    gfx->setPixel = graphicsFallbackSetPixel;
    gfx->getPixel = graphicsFallbackGetPixel;
    gfx->fillRect = graphicsFallbackFillRect;
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
    } else if (gfx->data.type == JSGRAPHICSTYPE_JS) {
      lcdSetCallbacks_JS(gfx);
    } else {
      jsExceptionHere(JSET_INTERNALERROR, "Unknown graphics type\n");
      assert(0);
    }

    return true;
  } else
    return false;
}

void graphicsSetVar(JsGraphics *gfx) {
  JsVar *dataname = jsvFindChildFromString(gfx->graphicsVar, JS_HIDDEN_CHAR_STR"gfx", true);
  JsVar *data = jsvSkipName(dataname);
  if (!data) {
    data = jsvNewStringOfLength(sizeof(JsGraphicsData));
    jsvSetValueOfName(dataname, data);
  }
  jsvUnLock(dataname);
  assert(data);
  jsvSetString(data, (char*)&gfx->data, sizeof(JsGraphicsData));
  jsvUnLock(data);
}

// ----------------------------------------------------------------------------------------------

// If graphics is flipped or rotated then the coordinates need modifying
void graphicsToDeviceCoordinates(const JsGraphics *gfx, short *x, short *y) {
  if (gfx->data.flags & JSGRAPHICSFLAGS_SWAP_XY) {
    short t = *x;
    *x = *y;
    *y = t;
  }
  if (gfx->data.flags & JSGRAPHICSFLAGS_INVERT_X) *x = (short)(gfx->data.width - (*x+1));
  if (gfx->data.flags & JSGRAPHICSFLAGS_INVERT_Y) *y = (short)(gfx->data.height - (*y+1));
}

// ----------------------------------------------------------------------------------------------

static void graphicsSetPixelDevice(JsGraphics *gfx, short x, short y, unsigned int col) {
  if (x<0 || y<0 || x>=gfx->data.width || y>=gfx->data.height) return;
  if (x < gfx->data.modMinX) gfx->data.modMinX=x;
  if (x > gfx->data.modMaxX) gfx->data.modMaxX=x;
  if (y < gfx->data.modMinY) gfx->data.modMinY=y;
  if (y > gfx->data.modMaxY) gfx->data.modMaxY=y;
  gfx->setPixel(gfx,x,y,col & (unsigned int)((1L<<gfx->data.bpp)-1));
}

static unsigned int graphicsGetPixelDevice(JsGraphics *gfx, short x, short y) {
  if (x<0 || y<0 || x>=gfx->data.width || y>=gfx->data.height) return 0;
  return gfx->getPixel(gfx, x, y);
}

static void graphicsFillRectDevice(JsGraphics *gfx, short x1, short y1, short x2, short y2) {
  if (x1>x2) {
    short t = x1;
    x1 = x2;
    x2 = t;
  }
  if (y1>y2) {
    short t = y1;
    y1 = y2;
    y2 = t;
  }
  if (x1<0) x1=0;
  if (y1<0) y1=0;
  if (x2>=gfx->data.width) x2 = (short)(gfx->data.width - 1);
  if (y2>=gfx->data.height) y2 = (short)(gfx->data.height - 1);
  if (x2<x1 || y2<y1) return; // nope

  if (x1 < gfx->data.modMinX) gfx->data.modMinX=x1;
  if (x2 > gfx->data.modMaxX) gfx->data.modMaxX=x2;
  if (y1 < gfx->data.modMinY) gfx->data.modMinY=y1;
  if (y2 > gfx->data.modMaxY) gfx->data.modMaxY=y2;

  if (x1==x2 && y1==y2) {
    graphicsSetPixelDevice(gfx,x1,y1,gfx->data.fgColor);
    return;
  }

  return gfx->fillRect(gfx, x1, y1, x2, y2);
}

// ----------------------------------------------------------------------------------------------

void graphicsSetPixel(JsGraphics *gfx, short x, short y, unsigned int col) {
  graphicsToDeviceCoordinates(gfx, &x, &y);
  graphicsSetPixelDevice(gfx, x, y, col);
}

unsigned int graphicsGetPixel(JsGraphics *gfx, short x, short y) {
  graphicsToDeviceCoordinates(gfx, &x, &y);
  return graphicsGetPixelDevice(gfx, x, y);
}

void graphicsFillRect(JsGraphics *gfx, short x1, short y1, short x2, short y2) {
  graphicsToDeviceCoordinates(gfx, &x1, &y1);
  graphicsToDeviceCoordinates(gfx, &x2, &y2);
  graphicsFillRectDevice(gfx, x1, y1, x2, y2);
}

void graphicsClear(JsGraphics *gfx) {
  unsigned int c = gfx->data.fgColor;
  gfx->data.fgColor = gfx->data.bgColor;
  graphicsFillRectDevice(gfx,0,0,(short)(gfx->data.width-1),(short)(gfx->data.height-1));
  gfx->data.fgColor = c;
}

// ----------------------------------------------------------------------------------------------


void graphicsDrawRect(JsGraphics *gfx, short x1, short y1, short x2, short y2) {
  graphicsToDeviceCoordinates(gfx, &x1, &y1);
  graphicsToDeviceCoordinates(gfx, &x2, &y2);
  // rather than writing pixels, we use fillrect - as it is faster
  graphicsFillRectDevice(gfx,x1,y1,x2,y1);
  graphicsFillRectDevice(gfx,x2,y1,x2,y2);
  graphicsFillRectDevice(gfx,x1,y2,x2,y2);
  graphicsFillRectDevice(gfx,x1,y2,x1,y1);
}

void graphicsDrawString(JsGraphics *gfx, short x1, short y1, const char *str) {
  // no need to modify coordinates as setPixel does that
  while (*str) {
    graphicsDrawChar4x6(gfx,x1,y1,*(str++));
    x1 = (short)(x1 + 4);
  }
}

void graphicsDrawLine(JsGraphics *gfx, short x1, short y1, short x2, short y2) {
  graphicsToDeviceCoordinates(gfx, &x1, &y1);
  graphicsToDeviceCoordinates(gfx, &x2, &y2);

  int xl = x2-x1;
  int yl = y2-y1;
  if (xl<0) xl=-xl; else if (xl==0) xl=1;
  if (yl<0) yl=-yl; else if (yl==0) yl=1;
  if (xl > yl) { // longer in X - scan in X
    if (x1>x2) {
      short t;
      t = x1; x1 = x2; x2 = t;
      t = y1; y1 = y2; y2 = t;
    }
    int pos = (y1<<8) + 128; // rounding!
    int step = ((y2-y1)<<8) / xl;
    short x;
    for (x=x1;x<=x2;x++) {
      graphicsSetPixelDevice(gfx, x, (short)(pos>>8), gfx->data.fgColor);
      pos += step;
    }
  } else {
    if (y1>y2) {
      short t;
      t = x1; x1 = x2; x2 = t;
      t = y1; y1 = y2; y2 = t;
    }
    int pos = (x1<<8) + 128; // rounding!
    int step = ((x2-x1)<<8) / yl;
    short y;
    for (y=y1;y<=y2;y++) {
      graphicsSetPixelDevice(gfx, (short)(pos>>8), y, gfx->data.fgColor);
      pos += step;
    }
  }
}

static inline void graphicsFillPolyCreateScanLines(JsGraphics *gfx, short *minx, short *maxx, short x1, short y1,short x2, short y2) {
    if (y2 < y1) {
        short t;
        t=x1;x1=x2;x2=t;
        t=y1;y1=y2;y2=t;
    }
    int xh = x1*256;
    int yl = y2-y1;
    if (yl==0) yl=1;
    int stepx = (x2-x1)*256 / yl;
    short y;
    for (y=y1;y<=y2;y++) {
        int x = xh>>8;
        if (x<-32768) x=-32768;
        if (x>32767) x=32767;
        if (y>=0 && y<gfx->data.height) {
            if (x<minx[y]) {
                minx[y] = (short)x;
            }
            if (x>maxx[y]) {
                maxx[y] = (short)x;
            }
        }
        xh += stepx;
    }
}

void graphicsFillPoly(JsGraphics *gfx, int points, short *vertices) {
  int i;
  short miny = (short)(gfx->data.height-1);
  short maxy = 0;
  for (i=0;i<points*2;i+=2) {
    // convert into device coordinates...
    graphicsToDeviceCoordinates(gfx, &vertices[i], &vertices[i+1]);
    short y = vertices[i+1];
    if (y<miny) miny=y;
    if (y>maxy) maxy=y;
  }
  if (miny<0) miny=0;
  if (maxy>=gfx->data.height) maxy=(short)(gfx->data.height-1);
  short minx[gfx->data.height];
  short maxx[gfx->data.height];
  short y;
  for (y=miny;y<=maxy;y++) {
    minx[y] = (short)(gfx->data.width);
    maxx[y] = -1;
  }
  int j = (points-1)*2;
  for (i=0;i<points*2;i+=2) {
    graphicsFillPolyCreateScanLines(gfx, minx, maxx, vertices[j+0], vertices[j+1], vertices[i+0], vertices[i+1]);
    j = i;
  }
  for (y=miny;y<=maxy;y++) {
    if (maxx[y]>=minx[y] && maxx[y]>=0 && minx[y]<gfx->data.width) {
      // clip
      if (minx[y]<0) minx[y]=0;
      if (maxx[y]>=gfx->data.width) maxx[y]=(short)(gfx->data.width-1);
      // try and expand the rect that we fill
      short oldy = y;
      while (y<maxy && minx[y+1]==minx[oldy] && maxx[y+1]==maxx[oldy])
        y++;
      // actually fill
      graphicsFillRectDevice(gfx,minx[y],oldy,maxx[y],y);
      if (jspIsInterrupted()) break;
    }
  }
}

#ifndef SAVE_ON_FLASH
// prints character, returns width
unsigned int graphicsFillVectorChar(JsGraphics *gfx, short x1, short y1, short size, char ch) {
  // no need to modify coordinates as graphicsFillPoly does that
  if (size<0) return 0;
  if (ch<vectorFontOffset || ch-vectorFontOffset>=vectorFontCount) return 0;
  int vertOffset = 0;
  int i;
  /* compute offset (I figure a ~50 iteration FOR loop is preferable to
   * a 200 byte array) */
  int fontOffset = ch-vectorFontOffset;
  for (i=0;i<fontOffset;i++)
    vertOffset += READ_FLASH_UINT8(&vectorFonts[i].vertCount);
  VectorFontChar vector;
  vector.vertCount = READ_FLASH_UINT8(&vectorFonts[fontOffset].vertCount);
  vector.width = READ_FLASH_UINT8(&vectorFonts[fontOffset].width);
  short verts[VECTOR_FONT_MAX_POLY_SIZE*2];
  int idx=0;
  for (i=0;i<vector.vertCount;i+=2) {
    verts[idx+0] = (short)(x1 + (((READ_FLASH_UINT8(&vectorFontPolys[vertOffset+i+0])&0x7F)*size + (VECTOR_FONT_POLY_SIZE/2)) / VECTOR_FONT_POLY_SIZE));
    verts[idx+1] = (short)(y1 + (((READ_FLASH_UINT8(&vectorFontPolys[vertOffset+i+1])&0x7F)*size + (VECTOR_FONT_POLY_SIZE/2)) / VECTOR_FONT_POLY_SIZE));
    idx+=2;
    if (READ_FLASH_UINT8(&vectorFontPolys[vertOffset+i+1]) & VECTOR_FONT_POLY_SEPARATOR) {
      graphicsFillPoly(gfx,idx/2, verts);

      if (jspIsInterrupted()) break;
      idx=0;
    }
  }
  return (vector.width * (unsigned int)size)/(VECTOR_FONT_POLY_SIZE*2);
}

// returns the width of a character
unsigned int graphicsVectorCharWidth(JsGraphics *gfx, short size, char ch) {
  NOT_USED(gfx);
  if (size<0) return 0;
  if (ch<vectorFontOffset || ch-vectorFontOffset>=vectorFontCount) return 0;
  unsigned char width = READ_FLASH_UINT8(&vectorFonts[ch-vectorFontOffset].width);
  return (width * (unsigned int)size)/(VECTOR_FONT_POLY_SIZE*2);
}
#endif

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

