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
 * This file is designed to be parsed during the build process
 *
 * Contains JavaScript Graphics Draw Functions
 * ----------------------------------------------------------------------------
 */
#include "jswrap_graphics.h"
#include "jsutils.h"
#include "jsinteractive.h"

#include "lcd_arraybuffer.h"
#include "lcd_js.h"
#ifdef USE_LCD_SDL
#include "lcd_sdl.h"
#endif

/*JSON{ "type":"class",
        "class" : "Graphics",
        "description" : ["This class provides Graphics operations that can be applied to a surface.",
                         "Use Graphics.createXXX to create a graphics object that renders in the way you want.",
                         "NOTE: On boards that contain an LCD, there is a built-in 'LCD' object of type Graphics. For instance to draw a line you'd type: ```LCD.drawLine(0,0,100,100)```" ]
}*/

/*JSON{ "type":"staticmethod", "class": "Graphics", "name" : "createArrayBuffer",
         "description" : "Create a Graphics object that renders to an Array Buffer. This will have a field called 'buffer' that can get used to get at the buffer itself",
         "generate" : "jswrap_graphics_createArrayBuffer",
         "params" : [ [ "width", "int", "Pixels wide" ],
                      [ "height", "int", "Pixels high" ],
                      [ "bpp", "int", "Number of bits per pixel" ],
                      [ "options", "JsVar", ["An object of other options. ```{ zigzag : true/false(default), vertical_byte : true/false(default) }```",
                                             "zigzag = whether to alternate the direction of scanlines for rows",
                                             "vertical_byte = whether to align bits in a byte vertically or not"] ] ],
         "return" : [ "JsVar", "The new Graphics object" ]
}*/
JsVar *jswrap_graphics_createArrayBuffer(int width, int height, int bpp, JsVar *options) {
  if (width<=0 || height<=0 || width>1023 || height>1023) {
    jsWarn("Invalid Size");
    return 0;
  }
  if (!(bpp==1 || bpp==8 || bpp==24 || bpp==32)) {
    jsWarn("Invalid BPP");
    return 0;
  }

  JsVar *parent = jspNewObject(jsiGetParser(), 0, "Graphics");
  if (!parent) return 0; // low memory

  JsGraphics gfx;
  graphicsStructInit(&gfx);
  gfx.data.type = JSGRAPHICSTYPE_ARRAYBUFFER;
  gfx.data.flags = JSGRAPHICSFLAGS_NONE;
  gfx.graphicsVar = parent;
  gfx.data.width = (unsigned short)width;
  gfx.data.height = (unsigned short)height;
  gfx.data.bpp = (unsigned char)bpp;

  if (jsvIsObject(options)) {
    if (jsvGetBoolAndUnLock(jsvObjectGetChild(options, "zigzag", 0)))
      gfx.data.flags = (JsGraphicsFlags)(gfx.data.flags | JSGRAPHICSFLAGS_ARRAYBUFFER_ZIGZAG);
    if (jsvGetBoolAndUnLock(jsvObjectGetChild(options, "vertical_byte", 0))) {
      if (gfx.data.bpp==1)
        gfx.data.flags = (JsGraphicsFlags)(gfx.data.flags | JSGRAPHICSFLAGS_ARRAYBUFFER_VERTICAL_BYTE);
      else
        jsWarn("vertical_byte only works for 1bpp ArrayBuffers\n");
    }
  }

  lcdInit_ArrayBuffer(&gfx);
  graphicsSetVar(&gfx);
  return parent;
}

/*JSON{ "type":"staticmethod", "class": "Graphics", "name" : "createCallback",
         "description" : "Create a Graphics object that renders by calling a JavaScript callback function",
         "generate" : "jswrap_graphics_createCallback",
         "params" : [ [ "width", "int", "Pixels wide" ],
                      [ "height", "int", "Pixels high" ],
                      [ "bpp", "int", "Number of bits per pixel" ],
                      [ "callback", "JsVar", "A function of the form ```function(x,y,col)``` that is called whenever a pixel needs to be drawn" ] ],
         "return" : [ "JsVar", "The new Graphics object" ]
}*/
JsVar *jswrap_graphics_createCallback(int width, int height, int bpp, JsVar *callback) {
  if (width<=0 || height<=0 || width>1023 || height>1023) {
    jsWarn("Invalid Size");
    return 0;
  }
  if (!(bpp==1 || bpp==8 || bpp==24 || bpp==32)) {
    jsWarn("Invalid BPP");
    return 0;
  }
  JsVar *callbackFn = jsvSkipName(callback);
  if (!jsvIsFunction(callbackFn)) {
    jsvUnLock(callbackFn);
    jsError("Expecting Callback Function but got %t", callbackFn);
    return 0;
  }

  JsVar *parent = jspNewObject(jsiGetParser(), 0, "Graphics");
  if (!parent) return 0; // low memory

  JsGraphics gfx;
  graphicsStructInit(&gfx);
  gfx.data.type = JSGRAPHICSTYPE_JS;
  gfx.graphicsVar = parent;
  gfx.data.width = (unsigned short)width;
  gfx.data.height = (unsigned short)height;
  gfx.data.bpp = (unsigned char)bpp;
  lcdInit_JS(&gfx, callbackFn);
  graphicsSetVar(&gfx);
  jsvUnLock(callbackFn);
  return parent;
}

#ifdef USE_LCD_SDL
/*JSON{ "type":"staticmethod", "class": "Graphics", "name" : "createSDL", "ifdef" : "USE_LCD_SDL",
         "description" : "Create a Graphics object that renders to SDL window (Linux-based devices only)",
         "generate" : "jswrap_graphics_createSDL",
         "params" : [ [ "width", "int", "Pixels wide" ],
                      [ "height", "int", "Pixels high" ] ],
         "return" : [ "JsVar", "The new Graphics object" ]
}*/
JsVar *jswrap_graphics_createSDL(int width, int height) {
  if (width<=0 || height<=0 || width>1023 || height>1023) {
    jsWarn("Invalid Size");
    return 0;
  }

  JsVar *parent = jspNewObject(jsiGetParser(), 0, "Graphics");
  if (!parent) return 0; // low memory
  JsGraphics gfx;
  graphicsStructInit(&gfx);
  gfx.data.type = JSGRAPHICSTYPE_SDL;
  gfx.graphicsVar = parent;
  gfx.data.width = (unsigned short)width;
  gfx.data.height = (unsigned short)height;
  gfx.data.bpp = 32;
  lcdInit_SDL(&gfx);
  graphicsSetVar(&gfx);
  return parent;
}
#endif

/*JSON{ "type":"method", "class": "Graphics", "name" : "getWidth",
         "description" : "The width of the LCD",
         "generate_full" : "jswrap_graphics_getWidthOrHeight(parent, false)",
         "return" : [ "int", "The width of the LCD" ]
}*/
/*JSON{ "type":"method", "class": "Graphics", "name" : "getHeight",
         "description" : "The height of the LCD",
         "generate_full" : "jswrap_graphics_getWidthOrHeight(parent, true)",
         "return" : [ "int", "The height of the LCD" ]
}*/
int jswrap_graphics_getWidthOrHeight(JsVar *parent, bool height) {
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return 0;
  return height ? gfx.data.height : gfx.data.width;
}

/*JSON{ "type":"method", "class": "Graphics", "name" : "clear",
         "description" : "Clear the LCD with the Background Color",
         "generate" : "jswrap_graphics_clear"
}*/
void jswrap_graphics_clear(JsVar *parent) {
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return;
  graphicsClear(&gfx);
}

/*JSON{ "type":"method", "class": "Graphics", "name" : "fillRect",
         "description" : "Fill a rectangular area in the Foreground Color",
         "generate" : "jswrap_graphics_fillRect",
         "params" : [ [ "x1", "int", "The left" ],
                      [ "y1", "int", "The top" ],
                      [ "x2", "int", "The right" ],
                      [ "y2", "int", "The bottom" ] ]
}*/
void jswrap_graphics_fillRect(JsVar *parent, int x1, int y1, int x2, int y2) {
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return;
  graphicsFillRect(&gfx, (short)x1,(short)y1,(short)x2,(short)y2);
}

/*JSON{ "type":"method", "class": "Graphics", "name" : "drawRect",
         "description" : "Draw an unfilled rectangle 1px wide in the Foreground Color",
         "generate" : "jswrap_graphics_drawRect",
         "params" : [ [ "x1", "int", "The left" ],
                      [ "y1", "int", "The top" ],
                      [ "x2", "int", "The right" ],
                      [ "y2", "int", "The bottom" ]]
}*/
void jswrap_graphics_drawRect(JsVar *parent, int x1, int y1, int x2, int y2) {
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return;
  graphicsDrawRect(&gfx, (short)x1,(short)y1,(short)x2,(short)y2);
}

/*JSON{ "type":"method", "class": "Graphics", "name" : "getPixel",
         "description" : "Get a pixel's color",
         "generate" : "jswrap_graphics_getPixel",
         "params" : [ [ "x", "int", "The left" ],
                      [ "y", "int", "The top" ] ],
         "return" : [ "int", "The color" ]
}*/
int jswrap_graphics_getPixel(JsVar *parent, int x, int y) {
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return 0;
  return (int)graphicsGetPixel(&gfx, (short)x, (short)y);
}

/*JSON{ "type":"method", "class": "Graphics", "name" : "setPixel",
         "description" : "Set a pixel's color",
         "generate" : "jswrap_graphics_setPixel",
         "params" : [ [ "x", "int", "The left" ],
                      [ "y", "int", "The top" ],
                      [ "col", "int", "The color" ] ]
}*/
void jswrap_graphics_setPixel(JsVar *parent, int x, int y, int color) {
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return;
  graphicsSetPixel(&gfx, (short)x, (short)y, (unsigned int)color);
  gfx.data.cursorX = (short)x;
  gfx.data.cursorY = (short)y;
}


/*JSON{ "type":"method", "class": "Graphics", "name" : "setColor",
         "description" : "Set the color to use for subsequent drawing operations",
         "generate_full" : "jswrap_graphics_setColorX(parent, r,g,b, true)",
         "params" : [ [ "r", "JsVar", "Red (between 0 and 1) OR an integer representing the color in the current bit depth" ],
                      [ "g", "JsVar", "Green (between 0 and 1)" ],
                      [ "b", "JsVar", "Blue (between 0 and 1)" ] ]
}*/
/*JSON{ "type":"method", "class": "Graphics", "name" : "setBgColor",
         "description" : "Set the background color to use for subsequent drawing operations",
         "generate_full" : "jswrap_graphics_setColorX(parent, r,g,b, false)",
         "params" : [ [ "r", "JsVar", "Red (between 0 and 1) OR an integer representing the color in the current bit depth" ],
                      [ "g", "JsVar", "Green (between 0 and 1)" ],
                      [ "b", "JsVar", "Blue (between 0 and 1)" ] ]
}*/
void jswrap_graphics_setColorX(JsVar *parent, JsVar *r, JsVar *g, JsVar *b, bool isForeground) {
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return;
  unsigned int color = 0;
  if (!jsvIsUndefined(g) && !jsvIsUndefined(b)) {
    int ri = (int)(jsvGetFloat(r)*256);
    int gi = (int)(jsvGetFloat(g)*256);
    int bi = (int)(jsvGetFloat(b)*256);
    if (ri>255) ri=255;
    if (gi>255) gi=255;
    if (bi>255) bi=255;
    if (ri<0) ri=0;
    if (gi<0) gi=0;
    if (bi<0) bi=0;
    if (gfx.data.bpp==16) {
      color = (unsigned int)((bi>>3) | (gi>>2)<<5 | (ri>>3)<<11);
    } else if (gfx.data.bpp==32) {
      color = 0xFF000000 | (unsigned int)(bi | (gi<<8) | (ri<<16));
    } else if (gfx.data.bpp==24) {
      color = (unsigned int)(bi | (gi<<8) | (ri<<16));
    } else
      color = (unsigned int)(((ri+gi+bi)>=384) ? 0xFFFFFFFF : 0);
  } else {
    // just rgb
    color = (unsigned int)jsvGetInteger(r);
  }
  if (isForeground)
    gfx.data.fgColor = color;
  else
    gfx.data.bgColor = color;
  graphicsSetVar(&gfx);
}

/*JSON{ "type":"method", "class": "Graphics", "name" : "setFontBitmap",
         "description" : "Set Graphics to draw with a Bitmapped Font",
         "generate_full" : "jswrap_graphics_setFontSizeX(parent, JSGRAPHICS_FONTSIZE_8X8, false)"
}*/
/*JSON{ "type":"method", "class": "Graphics", "name" : "setFontVector",
         "description" : "Set Graphics to draw with a Vector Font of the given size",
         "generate_full" : "jswrap_graphics_setFontSizeX(parent, jsvGetInteger(size), true)",
         "params" : [ [ "size", "int", "The size as an integer" ] ]
}*/
void jswrap_graphics_setFontSizeX(JsVar *parent, JsVarInt size, bool checkValid) {
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return;

  if (checkValid) {
    if (size<1) size=1;
    if (size>1023) size=1023;
  }
  gfx.data.fontSize = (short)size;
  graphicsSetVar(&gfx);
}


/*JSON{ "type":"method", "class": "Graphics", "name" : "drawString",
         "description" : "Draw a string of text in the current font",
         "generate" : "jswrap_graphics_drawString",
         "params" : [ [ "str", "JsVar", "The string" ],
                      [ "x", "int", "The left" ],
                      [ "y", "int", "The top" ] ]
}*/
void jswrap_graphics_drawString(JsVar *parent, JsVar *var, int x, int y) {
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return;
  JsVar *str = jsvAsString(var, false);
  JsvStringIterator it;
  jsvStringIteratorNew(&it, str, 0);
  while (jsvStringIteratorHasChar(&it)) {
    char ch = jsvStringIteratorGetChar(&it);
    if (gfx.data.fontSize>0) {
      int w = (int)graphicsFillVectorChar(&gfx, (short)x, (short)y, gfx.data.fontSize, ch);
      x+=w;
    } else if (gfx.data.fontSize == JSGRAPHICS_FONTSIZE_8X8) {
      graphicsDrawChar(&gfx, (short)x, (short)y, ch);
      x+=8;
    }
    jsvStringIteratorNext(&it);
  }
  jsvStringIteratorFree(&it);
  jsvUnLock(str);
}

/*JSON{ "type":"method", "class": "Graphics", "name" : "stringWidth",
         "description" : "Return the size in pixels of a string of text in the current font",
         "generate" : "jswrap_graphics_stringWidth",
         "params" : [ [ "str", "JsVar", "The string" ] ],
         "return" : [ "int", "The length of the string in pixels" ]
}*/
JsVarInt jswrap_graphics_stringWidth(JsVar *parent, JsVar *var) {
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return 0;
  JsVar *str = jsvAsString(var, false);
  JsvStringIterator it;
  jsvStringIteratorNew(&it, str, 0);
  int width = 0;
  while (jsvStringIteratorHasChar(&it)) {
    char ch = jsvStringIteratorGetChar(&it);
    if (gfx.data.fontSize>0) {
      width += (int)graphicsVectorCharWidth(&gfx, gfx.data.fontSize, ch);
    } else if (gfx.data.fontSize == JSGRAPHICS_FONTSIZE_8X8) {
      width += 8;
    }
    jsvStringIteratorNext(&it);
  }
  jsvStringIteratorFree(&it);
  jsvUnLock(str);
  return width;
}

/*JSON{ "type":"method", "class": "Graphics", "name" : "drawLine",
         "description" : "Draw a line between x1,y1 and x2,y2 in the current foreground color",
         "generate" : "jswrap_graphics_drawLine",
         "params" : [ [ "x1", "int", "The left" ],
                      [ "y1", "int", "The top" ],
                      [ "x2", "int", "The right" ],
                      [ "y2", "int", "The bottom" ] ]
}*/
void jswrap_graphics_drawLine(JsVar *parent, int x1, int y1, int x2, int y2) {
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return;
  graphicsDrawLine(&gfx, (short)x1,(short)y1,(short)x2,(short)y2);
}

/*JSON{ "type":"method", "class": "Graphics", "name" : "lineTo",
         "description" : "Draw a line from the last position of lineTo or moveTo to this position",
         "generate" : "jswrap_graphics_lineTo",
         "params" : [ [ "x", "int", "X value" ],
                      [ "y", "int", "Y value" ] ]
}*/
void jswrap_graphics_lineTo(JsVar *parent, int x, int y) {
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return;
  graphicsDrawLine(&gfx, gfx.data.cursorX, gfx.data.cursorY, (short)x, (short)y);
  gfx.data.cursorX = (short)x;
  gfx.data.cursorY = (short)y;
  graphicsSetVar(&gfx);
}

/*JSON{ "type":"method", "class": "Graphics", "name" : "moveTo",
         "description" : "Move the cursor to a position - see lineTo",
         "generate" : "jswrap_graphics_moveTo",
         "params" : [ [ "x", "int", "X value" ],
                      [ "y", "int", "Y value" ] ]
}*/
void jswrap_graphics_moveTo(JsVar *parent, int x, int y) {
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return;
  gfx.data.cursorX = (short)x;
  gfx.data.cursorY = (short)y;
  graphicsSetVar(&gfx);
}

/*JSON{ "type":"method", "class": "Graphics", "name" : "fillPoly",
         "description" : "Draw a filled polygon in the current foreground color",
         "generate" : "jswrap_graphics_fillPoly",
         "params" : [ [ "poly", "JsVar", "An array of vertices, of the form ```[x1,y1,x2,y2,x3,y3,etc]```" ] ]
}*/
void jswrap_graphics_fillPoly(JsVar *parent, JsVar *poly) {
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return;
  if (!jsvIsArray(poly)) return;
  const int maxVerts = 128;
  short verts[maxVerts];
  int idx = 0;
  JsVarRef item = poly->firstChild;
  while (item && idx<maxVerts) {
    JsVar *val = jsvLock(item);
    verts[idx++] = (short)jsvGetIntegerAndUnLock(jsvSkipName(val));
    item = val->nextSibling;
    jsvUnLock(val);
  }
  if (idx==maxVerts) {
    jsWarn("Maximum number of points (%d) exceeded for fillPoly", maxVerts/2);
  }
  graphicsFillPoly(&gfx, idx/2, verts);
}

