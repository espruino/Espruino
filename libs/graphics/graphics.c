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


#ifndef NO_VECTOR_FONT
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
#ifdef USE_LCD_SPI
#include "lcd_spilcd.h"
#endif
#ifdef USE_LCD_ST7789_8BIT
#include "lcd_st7789_8bit.h"
#endif

// ----------------------------------------------------------------------------------------------

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
  return 0;
}

void graphicsFallbackFillRect(JsGraphics *gfx, int x1, int y1, int x2, int y2) {
  int x,y;
  for (y=y1;y<=y2;y++)
    for (x=x1;x<=x2;x++)
      graphicsSetPixelDevice(gfx,x,y, gfx->data.fgColor);
}

void graphicsFallbackScrollX(JsGraphics *gfx, int xdir, int yfrom, int yto) {
  int x;
  if (xdir<=0) {
    int w = gfx->data.width+xdir;
    for (x=0;x<w;x++)
      gfx->setPixel(gfx, (int)x,(int)yto,
          gfx->getPixel(gfx, (int)(x-xdir),(int)yfrom));
  } else { // >0
    for (x=gfx->data.width-xdir-1;x>=0;x--)
      gfx->setPixel(gfx, (int)(x+xdir),(int)yto,
          gfx->getPixel(gfx, (int)x,(int)yfrom));
  }
}

void graphicsFallbackScroll(JsGraphics *gfx, int xdir, int ydir) {
  if (xdir==0 && ydir==0) return;
  int y;
  if (ydir<=0) {
    int h = gfx->data.height+ydir;
    for (y=0;y<h;y++)
      graphicsFallbackScrollX(gfx, xdir, y-ydir, y);
  } else { // >0
    for (y=gfx->data.height-ydir-1;y>=0;y--)
      graphicsFallbackScrollX(gfx, xdir, y, y+ydir);
  }
#ifndef SAVE_ON_FLASH
  gfx->data.modMinX=0;
  gfx->data.modMinY=0;
  gfx->data.modMaxX=gfx->data.width-1;
  gfx->data.modMaxY=gfx->data.height-1;
#endif
}

// ----------------------------------------------------------------------------------------------

void graphicsStructResetState(JsGraphics *gfx) {
  gfx->data.fgColor = 0xFFFFFFFF;
  gfx->data.bgColor = 0;
  gfx->data.fontSize = 1+JSGRAPHICS_FONTSIZE_4X6;
#ifndef SAVE_ON_FLASH
  gfx->data.fontAlignX = 3;
  gfx->data.fontAlignY = 3;
  gfx->data.fontRotate = 0;
#endif
  gfx->data.cursorX = 0;
  gfx->data.cursorY = 0;
}

void graphicsStructInit(JsGraphics *gfx) {
  // type/width/height/bpp should be set elsewhere...
  gfx->data.flags = JSGRAPHICSFLAGS_NONE;
  graphicsStructResetState(gfx);
#ifndef SAVE_ON_FLASH
  gfx->data.modMaxX = -32768;
  gfx->data.modMaxY = -32768;
  gfx->data.modMinX = 32767;
  gfx->data.modMinY = 32767;
#endif
}

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
    data = jsvNewStringOfLength(sizeof(JsGraphicsData), NULL);
    jsvSetValueOfName(dataname, data);
  }
  jsvUnLock(dataname);
  assert(data);
  jsvSetString(data, (char*)&gfx->data, sizeof(JsGraphicsData));
  jsvUnLock(data);
}

/// Get the memory requires for this graphics's pixels if everything was packed as densely as possible
size_t graphicsGetMemoryRequired(const JsGraphics *gfx) {
  return (gfx->data.width * gfx->data.height * gfx->data.bpp + 7) >> 3;
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

// ----------------------------------------------------------------------------------------------

static void graphicsSetPixelDevice(JsGraphics *gfx, int x, int y, unsigned int col) {
  if (x<0 || y<0 || x>=gfx->data.width || y>=gfx->data.height) return;
#ifndef SAVE_ON_FLASH
  if (x < gfx->data.modMinX) gfx->data.modMinX=(short)x;
  if (x > gfx->data.modMaxX) gfx->data.modMaxX=(short)x;
  if (y < gfx->data.modMinY) gfx->data.modMinY=(short)y;
  if (y > gfx->data.modMaxY) gfx->data.modMaxY=(short)y;
#endif
  gfx->setPixel(gfx,(int)x,(int)y,col & (unsigned int)((1L<<gfx->data.bpp)-1));
}

static unsigned int graphicsGetPixelDevice(JsGraphics *gfx, int x, int y) {
  if (x<0 || y<0 || x>=gfx->data.width || y>=gfx->data.height) return 0;
  return gfx->getPixel(gfx, x, y);
}

static void graphicsFillRectDevice(JsGraphics *gfx, int x1, int y1, int x2, int y2) {
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
  if (x1<0) x1=0;
  if (y1<0) y1=0;
  if (x2>=gfx->data.width) x2 = gfx->data.width - 1;
  if (y2>=gfx->data.height) y2 = gfx->data.height - 1;
  if (x2<x1 || y2<y1) return; // nope
#ifndef SAVE_ON_FLASH
  if (x1 < gfx->data.modMinX) gfx->data.modMinX=(short)x1;
  if (x2 > gfx->data.modMaxX) gfx->data.modMaxX=(short)x2;
  if (y1 < gfx->data.modMinY) gfx->data.modMinY=(short)y1;
  if (y2 > gfx->data.modMaxY) gfx->data.modMaxY=(short)y2;
#endif
  if (x1==x2 && y1==y2) {
    gfx->setPixel(gfx,(int)x1,(int)y1,gfx->data.fgColor);
    return;
  }

  return gfx->fillRect(gfx, (int)x1, (int)y1, (int)x2, (int)y2);
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

void graphicsFillRect(JsGraphics *gfx, int x1, int y1, int x2, int y2) {
  graphicsToDeviceCoordinates(gfx, &x1, &y1);
  graphicsToDeviceCoordinates(gfx, &x2, &y2);
  graphicsFillRectDevice(gfx, x1, y1, x2, y2);
}

void graphicsClear(JsGraphics *gfx) {
  unsigned int c = gfx->data.fgColor;
  gfx->data.fgColor = gfx->data.bgColor;
  graphicsFillRectDevice(gfx,0,0,(int)(gfx->data.width-1),(int)(gfx->data.height-1));
  gfx->data.fgColor = c;
}

// ----------------------------------------------------------------------------------------------


void graphicsDrawRect(JsGraphics *gfx, int x1, int y1, int x2, int y2) {
  graphicsToDeviceCoordinates(gfx, &x1, &y1);
  graphicsToDeviceCoordinates(gfx, &x2, &y2);
  // rather than writing pixels, we use fillrect - as it is faster
  graphicsFillRectDevice(gfx,x1,y1,x2,y1);
  graphicsFillRectDevice(gfx,x2,y1,x2,y2);
  graphicsFillRectDevice(gfx,x1,y2,x2,y2);
  graphicsFillRectDevice(gfx,x1,y2,x1,y1);
}

void graphicsDrawEllipse(JsGraphics *gfx, int posX1, int posY1, int posX2, int posY2){
  graphicsToDeviceCoordinates(gfx, &posX1, &posY1);
  graphicsToDeviceCoordinates(gfx, &posX2, &posY2);
  int posX = (posX1+posX2)/2;
  int posY = (posY1+posY2)/2;
  int width = (posX2-posX1)/2;
  int height = (posY2-posY1)/2;
  if (width<0) width=-width;
  if (height<0) height=-height;
  int hh = height * height;
  int ww = width * width;
  int hhww = hh * ww;
  int x0 = width;
  int dx = 0;
  int y;
  graphicsSetPixelDevice(gfx,posX - width,posY,gfx->data.fgColor);
  graphicsSetPixelDevice(gfx,posX + width,posY,gfx->data.fgColor);
  for (y = 1; y <= height; y++) {
    int x1 = x0 - (dx - 1);
    for(; x1> 0; x1--)
      if (x1 * x1 * hh + y * y * ww <= hhww)
        break;
    dx = x0 - x1;
    x0 = x1;
    if(dx<2){
      graphicsSetPixelDevice(gfx,posX - x0, posY - y,gfx->data.fgColor);
      graphicsSetPixelDevice(gfx,posX + x0, posY - y,gfx->data.fgColor);
      graphicsSetPixelDevice(gfx,posX - x0, posY + y,gfx->data.fgColor);
      graphicsSetPixelDevice(gfx,posX + x0, posY + y,gfx->data.fgColor);
    } else {
      graphicsFillRectDevice(gfx,posX - x0, posY - y, posX - x0 - dx + 1, posY - y);
      graphicsFillRectDevice(gfx,posX + x0, posY - y, posX + x0 + dx - 1, posY	- y);
      graphicsFillRectDevice(gfx,posX - x0, posY + y, posX - x0 - dx + 1, posY + y);
      graphicsFillRectDevice(gfx,posX + x0, posY + y, posX + x0 + dx - 1, posY + y); 
    }
  }
}

void graphicsFillEllipse(JsGraphics *gfx, int posX1, int posY1, int posX2, int posY2){
  graphicsToDeviceCoordinates(gfx, &posX1, &posY1);
  graphicsToDeviceCoordinates(gfx, &posX2, &posY2);
  int posX = (posX1+posX2)/2;
  int posY = (posY1+posY2)/2;
  int width = (posX2-posX1)/2;
  int height = (posY2-posY1)/2;
  if (width<0) width=-width;
  if (height<0) height=-height;
  int hh = height * height;
  int ww = width * width;
  int hhww = hh * ww;
  int x0 = width;
  int dx = 0;
  graphicsFillRectDevice(gfx, posX - width, posY, posX + width, posY);  
  for (int y = 1; y <= height; y++) {
    int x1 = x0 - (dx - 1);
    for ( ; x1 > 0; x1--)
      if (x1*x1*hh + y*y*ww <= hhww)
        break;
    dx = x0 - x1;  
    x0 = x1;
	  graphicsFillRectDevice(gfx, posX - x0, posY - y, posX + x0, posY - y);
	  graphicsFillRectDevice(gfx, posX - x0, posY + y, posX + x0, posY + y);
  }
}

static void graphicsDrawString(JsGraphics *gfx, int x1, int y1, const char *str) {
  // no need to modify coordinates as setPixel does that
  while (*str) {
    graphicsDrawChar4x6(gfx,x1,y1,*(str++),1);
    x1 = (int)(x1 + 4);
  }
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

static inline void graphicsFillPolyCreateScanLines(JsGraphics *gfx, short *minx, short *maxx, short x1, short y1,short x2, short y2) {
  if (y2 < y1) {
    short t;
    t=x1;x1=x2;x2=t;
    t=y1;y1=y2;y2=t;
  }
  int xh = x1*256 + 128/*do rounding here rather than when we >>8*/;
  int yl = (1+y2)-y1;
  int stepx = ((x2-x1)*256 + (yl/2)/*rounding*/) / yl;
  short y;
  int x = xh>>8;
  if (x<-32768) x=-32768;
  if (x>32767) x=32767;
  for (y=y1;y<=y2;y++) {
    int oldx = x;
    xh += stepx;
    x = xh>>8;
    if (x<-32768) x=-32768;
    if (x>32767) x=32767;
    if (y>=0 && y<gfx->data.height) {
      if (oldx<minx[y]) minx[y] = (short)oldx;
      if (oldx>maxx[y]) maxx[y] = (short)oldx;
      if (x<minx[y]) minx[y] = (short)x;
      if (x>maxx[y]) maxx[y] = (short)x;
    }
  }
}

void graphicsFillPoly(JsGraphics *gfx, int points, short *vertices) {
  int i;
  int miny = (int)(gfx->data.height-1);
  int maxy = 0;
  for (i=0;i<points*2;i+=2) {
    // convert into device coordinates...
    graphicsToDeviceCoordinates(gfx, &vertices[i], &vertices[i+1]);
    short y = vertices[i+1];
    if (y<miny) miny=y;
    if (y>maxy) maxy=y;
  }
  if (miny<0) miny=0;
  if (maxy>=gfx->data.height) maxy=(int)(gfx->data.height-1);
  short minx[gfx->data.height];
  short maxx[gfx->data.height];
  int y;
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
      int oldy = y;
      while (y<maxy && minx[y+1]==minx[oldy] && maxx[y+1]==maxx[oldy])
        y++;
      // actually fill
      graphicsFillRectDevice(gfx,minx[y],oldy,maxx[y],y);
      if (jspIsInterrupted()) break;
    }
  }
}

#ifndef NO_VECTOR_FONT
// prints character, returns width
unsigned int graphicsFillVectorChar(JsGraphics *gfx, int x1, int y1, int size, char ch) {
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
unsigned int graphicsVectorCharWidth(JsGraphics *gfx, unsigned int size, char ch) {
  NOT_USED(gfx);
  if (size<0) return 0;
  if (ch<vectorFontOffset || ch-vectorFontOffset>=vectorFontCount) return 0;
  unsigned char width = READ_FLASH_UINT8(&vectorFonts[ch-vectorFontOffset].width);
  return (width * (unsigned int)size)/(VECTOR_FONT_POLY_SIZE*2);
}
#endif

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
  // do the scrolling
  gfx->scroll(gfx, xdir, ydir);
  // fill the new area
  unsigned int c = gfx->data.fgColor;
  gfx->data.fgColor = gfx->data.bgColor;
  if (xdir>0) gfx->fillRect(gfx,0,0,xdir-1,gfx->data.height-1);
  else if (xdir<0) gfx->fillRect(gfx,gfx->data.width+xdir,0,gfx->data.width-1,gfx->data.height-1);
  if (ydir>0) gfx->fillRect(gfx,0,0,gfx->data.width-1,ydir-1);
  else if (ydir<0) gfx->fillRect(gfx,0,gfx->data.height+ydir,gfx->data.width-1,gfx->data.height-1);
  gfx->data.fgColor = c;
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

