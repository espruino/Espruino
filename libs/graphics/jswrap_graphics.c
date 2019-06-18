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
#ifdef USE_LCD_FSMC
#include "lcd_fsmc.h"
#endif

#include "jswrap_functions.h" // for asURL


/*JSON{
  "type" : "class",
  "class" : "Graphics"
}
This class provides Graphics operations that can be applied to a surface.

Use Graphics.createXXX to create a graphics object that renders in the way you want. See [the Graphics page](/Graphics) for more information.

**Note:** On boards that contain an LCD, there is a built-in 'LCD' object of type Graphics. For instance to draw a line you'd type: ```LCD.drawLine(0,0,100,100)```
*/

/*JSON{
  "type" : "idle",
  "generate" : "jswrap_graphics_idle"
}*/
bool jswrap_graphics_idle() {
  graphicsIdle();
  return false;
}

/*JSON{
  "type" : "init",
  "generate" : "jswrap_graphics_init"
}*/
void jswrap_graphics_init() {
#ifdef USE_LCD_FSMC
  JsVar *parent = jspNewObject("LCD", "Graphics");
  if (parent) {
    JsVar *parentObj = jsvSkipName(parent);
    jsvObjectSetChild(execInfo.hiddenRoot, JS_GRAPHICS_VAR, parentObj);
    JsGraphics gfx;
    graphicsStructInit(&gfx);
    gfx.data.type = JSGRAPHICSTYPE_FSMC;
    gfx.graphicsVar = parentObj;
    gfx.data.width = 320;
    gfx.data.height = 240;
    gfx.data.bpp = 16;
    lcdInit_FSMC(&gfx);
    lcdSetCallbacks_FSMC(&gfx);
    graphicsSplash(&gfx);
    graphicsSetVar(&gfx);
    jsvUnLock2(parentObj, parent);
  }
#endif
}

/*JSON{
  "type" : "staticmethod",
  "class" : "Graphics",
  "name" : "getInstance",
  "generate" : "jswrap_graphics_getInstance",
  "return" : ["JsVar","An instance of `Graphics` or undefined"]
}
On devices like Pixl.js or HYSTM boards that contain a built-in display
this will return an instance of the graphics class that can be used to
access that display.

Internally, this is stored as a member called `gfx` inside the 'hiddenRoot'.
*/
JsVar *jswrap_graphics_getInstance() {
  return jsvObjectGetChild(execInfo.hiddenRoot, JS_GRAPHICS_VAR, 0);
}

static bool isValidBPP(int bpp) {
  return bpp==1 || bpp==2 || bpp==4 || bpp==8 || bpp==16 || bpp==24 || bpp==32; // currently one colour can't ever be spread across multiple bytes
}

/*JSON{
  "type" : "staticmethod",
  "class" : "Graphics",
  "name" : "createArrayBuffer",
  "generate" : "jswrap_graphics_createArrayBuffer",
  "params" : [
    ["width","int32","Pixels wide"],
    ["height","int32","Pixels high"],
    ["bpp","int32","Number of bits per pixel"],
    ["options","JsVar",[
      "An object of other options. ```{ zigzag : true/false(default), vertical_byte : true/false(default), msb : true/false(default), color_order: 'rgb'(default),'bgr',etc }```",
      "zigzag = whether to alternate the direction of scanlines for rows",
      "vertical_byte = whether to align bits in a byte vertically or not",
      "msb = when bits<8, store pixels msb first",
      "interleavex = Pixels 0,2,4,etc are from the top half of the image, 1,3,5,etc from the bottom half. Used for P3 LED panels.",
      "color_order = re-orders the colour values that are supplied via setColor"
    ]]
  ],
  "return" : ["JsVar","The new Graphics object"],
  "return_object" : "Graphics"
}
Create a Graphics object that renders to an Array Buffer. This will have a field called 'buffer' that can get used to get at the buffer itself
*/
JsVar *jswrap_graphics_createArrayBuffer(int width, int height, int bpp, JsVar *options) {
  if (width<=0 || height<=0 || width>32767 || height>32767) {
    jsExceptionHere(JSET_ERROR, "Invalid Size");
    return 0;
  }
  if (!isValidBPP(bpp)) {
    jsExceptionHere(JSET_ERROR, "Invalid BPP");
    return 0;
  }

  JsVar *parent = jspNewObject(0, "Graphics");
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
    if (jsvGetBoolAndUnLock(jsvObjectGetChild(options, "msb", 0)))
      gfx.data.flags = (JsGraphicsFlags)(gfx.data.flags | JSGRAPHICSFLAGS_ARRAYBUFFER_MSB);
    if (jsvGetBoolAndUnLock(jsvObjectGetChild(options, "interleavex", 0)))
      gfx.data.flags = (JsGraphicsFlags)(gfx.data.flags | JSGRAPHICSFLAGS_ARRAYBUFFER_INTERLEAVEX);
    if (jsvGetBoolAndUnLock(jsvObjectGetChild(options, "vertical_byte", 0))) {
      if (gfx.data.bpp==1)
        gfx.data.flags = (JsGraphicsFlags)(gfx.data.flags | JSGRAPHICSFLAGS_ARRAYBUFFER_VERTICAL_BYTE);
      else {
        jsExceptionHere(JSET_ERROR, "vertical_byte only works for 1bpp ArrayBuffers\n");
        return 0;
      }
      if (gfx.data.height&7) {
        jsExceptionHere(JSET_ERROR, "height must be a multiple of 8 when using vertical_byte\n");
        return 0;
      }
    }
    JsVar *colorv = jsvObjectGetChild(options, "color_order", 0);
    if (colorv) {
      if (jsvIsStringEqual(colorv, "rgb")) ; // The default
      else if (!jsvIsStringEqual(colorv, "brg"))
        gfx.data.flags = (JsGraphicsFlags)(gfx.data.flags | JSGRAPHICSFLAGS_COLOR_BRG);
      else if (!jsvIsStringEqual(colorv, "bgr"))
        gfx.data.flags = (JsGraphicsFlags)(gfx.data.flags | JSGRAPHICSFLAGS_COLOR_BGR);
      else if (!jsvIsStringEqual(colorv, "gbr"))
        gfx.data.flags = (JsGraphicsFlags)(gfx.data.flags | JSGRAPHICSFLAGS_COLOR_GBR);
      else if (!jsvIsStringEqual(colorv, "grb"))
        gfx.data.flags = (JsGraphicsFlags)(gfx.data.flags | JSGRAPHICSFLAGS_COLOR_GRB);
      else if (!jsvIsStringEqual(colorv, "rbg"))
        gfx.data.flags = (JsGraphicsFlags)(gfx.data.flags | JSGRAPHICSFLAGS_COLOR_RBG);
      else
        jsWarn("color_order must be 3 characters");
      jsvUnLock(colorv);
    }
  }

  lcdInit_ArrayBuffer(&gfx);
  graphicsSetVar(&gfx);
  return parent;
}

/*JSON{
  "type" : "staticmethod",
  "class" : "Graphics",
  "name" : "createCallback",
  "ifndef" : "SAVE_ON_FLASH",
  "generate" : "jswrap_graphics_createCallback",
  "params" : [
    ["width","int32","Pixels wide"],
    ["height","int32","Pixels high"],
    ["bpp","int32","Number of bits per pixel"],
    ["callback","JsVar","A function of the form ```function(x,y,col)``` that is called whenever a pixel needs to be drawn, or an object with: ```{setPixel:function(x,y,col),fillRect:function(x1,y1,x2,y2,col)}```. All arguments are already bounds checked."]
  ],
  "return" : ["JsVar","The new Graphics object"],
  "return_object" : "Graphics"
}
Create a Graphics object that renders by calling a JavaScript callback function to draw pixels
*/
JsVar *jswrap_graphics_createCallback(int width, int height, int bpp, JsVar *callback) {
  if (width<=0 || height<=0 || width>32767 || height>32767) {
    jsExceptionHere(JSET_ERROR, "Invalid Size");
    return 0;
  }
  if (!isValidBPP(bpp)) {
    jsExceptionHere(JSET_ERROR, "Invalid BPP");
    return 0;
  }
  JsVar *callbackSetPixel = 0;
  JsVar *callbackFillRect = 0;
  if (jsvIsObject(callback)) {
    jsvUnLock(callbackSetPixel);
    callbackSetPixel = jsvObjectGetChild(callback, "setPixel", 0);
    callbackFillRect = jsvObjectGetChild(callback, "fillRect", 0);
  } else
    callbackSetPixel = jsvLockAgain(callback);
  if (!jsvIsFunction(callbackSetPixel)) {
    jsExceptionHere(JSET_ERROR, "Expecting Callback Function or an Object but got %t", callbackSetPixel);
    jsvUnLock2(callbackSetPixel, callbackFillRect);
    return 0;
  }
  if (!jsvIsUndefined(callbackFillRect) && !jsvIsFunction(callbackFillRect)) {
    jsExceptionHere(JSET_ERROR, "Expecting Callback Function or an Object but got %t", callbackFillRect);
    jsvUnLock2(callbackSetPixel, callbackFillRect);
    return 0;
  }

  JsVar *parent = jspNewObject(0, "Graphics");
  if (!parent) return 0; // low memory

  JsGraphics gfx;
  graphicsStructInit(&gfx);
  gfx.data.type = JSGRAPHICSTYPE_JS;
  gfx.graphicsVar = parent;
  gfx.data.width = (unsigned short)width;
  gfx.data.height = (unsigned short)height;
  gfx.data.bpp = (unsigned char)bpp;
  lcdInit_JS(&gfx, callbackSetPixel, callbackFillRect);
  graphicsSetVar(&gfx);
  jsvUnLock2(callbackSetPixel, callbackFillRect);
  return parent;
}

#ifdef USE_LCD_SDL
/*JSON{
  "type" : "staticmethod",
  "class" : "Graphics",
  "name" : "createSDL",
  "ifdef" : "USE_LCD_SDL",
  "generate" : "jswrap_graphics_createSDL",
  "params" : [
    ["width","int32","Pixels wide"],
    ["height","int32","Pixels high"]
  ],
  "return" : ["JsVar","The new Graphics object"],
  "return_object" : "Graphics"
}
Create a Graphics object that renders to SDL window (Linux-based devices only)
*/
JsVar *jswrap_graphics_createSDL(int width, int height) {
  if (width<=0 || height<=0 || width>32767 || height>32767) {
    jsExceptionHere(JSET_ERROR, "Invalid Size");
    return 0;
  }

  JsVar *parent = jspNewObject(0, "Graphics");
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


/*JSON{
  "type" : "staticmethod",
  "class" : "Graphics",
  "name" : "createImage",
  "#if" : "!defined(SAVE_ON_FLASH) && !defined(ESPRUINOBOARD)",
  "generate" : "jswrap_graphics_createImage",
  "params" : [
    ["str","JsVar","A String containing a newline-separated image - space is 0, anything else is 1"]
  ],
  "return" : ["JsVar","An Image object that can be used with `Graphics.drawImage`"]
}
Create a simple Black and White image for use with `Graphics.drawImage`.

Use as follows:

```
var img = Graphics.createImage(`
XXXXXXXXX
X       X
X   X   X
X   X   X
X       X
XXXXXXXXX
`);
g.drawImage(img, x,y);
```

If the characters at the beginning and end of the string are newlines, they
will be ignored. Spaces are treated as `0`, and any other character is a `1`
*/
JsVar *jswrap_graphics_createImage(JsVar *data) {
  if (!jsvIsString(data)) {
    jsExceptionHere(JSET_TYPEERROR, "Expecting a String");
    return 0;
  }
  int x=0,y=0;
  int width=0, height=0;
  size_t startCharacter = 0;
  JsvStringIterator it;
  // First iterate and work out width and height
  jsvStringIteratorNew(&it,data,0);
  while (jsvStringIteratorHasChar(&it)) {
    char ch = jsvStringIteratorGetChar(&it);
    if (ch=='\n') {
      if (x==0 && y==0) startCharacter = 1; // ignore first character
      x=0;
      y++;
    } else {
      if (y>=height) height=y+1;
      x++;
      if (x>width) width=x;
    }
    jsvStringIteratorNext(&it);
  }
  jsvStringIteratorFree(&it);
  // Sorted - now create the object, set it up and create the buffer
  JsVar *img = jsvNewObject();
  if (!img) return 0;
  jsvObjectSetChildAndUnLock(img,"width",jsvNewFromInteger(width));
  jsvObjectSetChildAndUnLock(img,"height",jsvNewFromInteger(height));
  // bpp is 1, no need to set it
  int len = (width*height+7)>>3;
  JsVar *buffer = jsvNewStringOfLength((unsigned)len, NULL);
  if (!buffer) { // not enough memory
    jsvUnLock(img);
    return 0;
  }
  // Now set the characters!
  x=0;
  y=0;
  jsvStringIteratorNew(&it,data,startCharacter);
  while (jsvStringIteratorHasChar(&it)) {
    char ch = jsvStringIteratorGetChar(&it);
    if (ch=='\n') {
      x=0;
      y++;
    } else {
      if (ch!=' ') {
        /* a pixel to set. This'll be slowish for non-flat strings,
         * but this is here for relatively small bitmaps
         * anyway so it's not a big deal */
        size_t idx = (size_t)(y*width + x);
        jsvSetCharInString(buffer, idx>>3, (char)(128>>(idx&7)), true/*OR*/);
      }
      x++;
    }
    jsvStringIteratorNext(&it);
  }
  jsvStringIteratorFree(&it);
  jsvObjectSetChildAndUnLock(img, "buffer", buffer);
  return img;
}

/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "getWidth",
  "generate_full" : "jswrap_graphics_getWidthOrHeight(parent, false)",
  "return" : ["int","The width of the LCD"]
}
The width of the LCD
*/
/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "getHeight",
  "generate_full" : "jswrap_graphics_getWidthOrHeight(parent, true)",
  "return" : ["int","The height of the LCD"]
}
The height of the LCD
*/
int jswrap_graphics_getWidthOrHeight(JsVar *parent, bool height) {
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return 0;
  if (gfx.data.flags & JSGRAPHICSFLAGS_SWAP_XY)
    height=!height;
  return height ? gfx.data.height : gfx.data.width;
}

/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "clear",
  "generate" : "jswrap_graphics_clear",
  "params" : [
    ["reset","bool","If `true`, resets the state of Graphics to the default (eg. Color, Font, etc)"]
  ],
  "return" : ["JsVar","The instance of Graphics this was called on, to allow call chaining"],
  "return_object" : "Graphics"
}
Clear the LCD with the Background Color
*/
JsVar *jswrap_graphics_clear(JsVar *parent, bool resetState) {
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return 0;
  if (resetState) graphicsStructResetState(&gfx);
  graphicsClear(&gfx);
  graphicsSetVar(&gfx); // gfx data changed because modified area
  return jsvLockAgain(parent);
}

/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "fillRect",
  "generate" : "jswrap_graphics_fillRect",
  "params" : [
    ["x1","int32","The left X coordinate"],
    ["y1","int32","The top Y coordinate"],
    ["x2","int32","The right X coordinate"],
    ["y2","int32","The bottom Y coordinate"]
  ],
  "return" : ["JsVar","The instance of Graphics this was called on, to allow call chaining"],
  "return_object" : "Graphics"
}
Fill a rectangular area in the Foreground Color
*/
JsVar *jswrap_graphics_fillRect(JsVar *parent, int x1, int y1, int x2, int y2) {
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return 0;
  graphicsFillRect(&gfx, (short)x1,(short)y1,(short)x2,(short)y2);
  graphicsSetVar(&gfx); // gfx data changed because modified area
  return jsvLockAgain(parent);
}

/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "drawRect",
  "generate" : "jswrap_graphics_drawRect",
  "params" : [
    ["x1","int32","The left X coordinate"],
    ["y1","int32","The top Y coordinate"],
    ["x2","int32","The right X coordinate"],
    ["y2","int32","The bottom Y coordinate"]
  ],
  "return" : ["JsVar","The instance of Graphics this was called on, to allow call chaining"],
  "return_object" : "Graphics"
}
Draw an unfilled rectangle 1px wide in the Foreground Color
*/
JsVar *jswrap_graphics_drawRect(JsVar *parent, int x1, int y1, int x2, int y2) {
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return 0;
  graphicsDrawRect(&gfx, (short)x1,(short)y1,(short)x2,(short)y2);
  graphicsSetVar(&gfx); // gfx data changed because modified area
  return jsvLockAgain(parent);
}

/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "fillCircle",
  "ifndef" : "SAVE_ON_FLASH",
  "generate" : "jswrap_graphics_fillCircle",
  "params" : [
    ["x","int32","The X axis"],
    ["y","int32","The Y axis"],
    ["rad","int32","The circle radius"]
  ],
  "return" : ["JsVar","The instance of Graphics this was called on, to allow call chaining"],
  "return_object" : "Graphics"
}
Draw a filled circle in the Foreground Color
*/
 JsVar *jswrap_graphics_fillCircle(JsVar *parent, int x, int y, int rad) {
   jswrap_graphics_fillEllipse(parent, x-rad, y-rad, x+rad, y+rad);
   return jsvLockAgain(parent);
 }

/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "drawCircle",
  "ifndef" : "SAVE_ON_FLASH",
  "generate" : "jswrap_graphics_drawCircle",
  "params" : [
    ["x","int32","The X axis"],
    ["y","int32","The Y axis"],
    ["rad","int32","The circle radius"]
  ],
  "return" : ["JsVar","The instance of Graphics this was called on, to allow call chaining"],
  "return_object" : "Graphics"
}
Draw an unfilled circle 1px wide in the Foreground Color
*/
JsVar *jswrap_graphics_drawCircle(JsVar *parent, int x, int y, int rad) {
  jswrap_graphics_drawEllipse(parent, x-rad, y-rad, x+rad, y+rad);
  return jsvLockAgain(parent);
}

/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "fillEllipse",
  "ifndef" : "SAVE_ON_FLASH",
  "generate" : "jswrap_graphics_fillEllipse",
  "params" : [
    ["x1","int32","The left X coordinate"],
    ["y1","int32","The top Y coordinate"],
    ["x2","int32","The right X coordinate"],
    ["y2","int32","The bottom Y coordinate"]
  ],
  "return" : ["JsVar","The instance of Graphics this was called on, to allow call chaining"],
  "return_object" : "Graphics"
}
Draw a filled ellipse in the Foreground Color
*/
JsVar *jswrap_graphics_fillEllipse(JsVar *parent, int x, int y, int x2, int y2) {
   JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return 0;
   graphicsFillEllipse(&gfx, (short)x,(short)y,(short)x2,(short)y2);
   graphicsSetVar(&gfx); // gfx data changed because modified area
   return jsvLockAgain(parent);
 }
 
/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "drawEllipse",
  "ifndef" : "SAVE_ON_FLASH",
  "generate" : "jswrap_graphics_drawEllipse",
  "params" : [
    ["x1","int32","The left X coordinate"],
    ["y1","int32","The top Y coordinate"],
    ["x2","int32","The right X coordinate"],
    ["y2","int32","The bottom Y coordinate"]
  ],
  "return" : ["JsVar","The instance of Graphics this was called on, to allow call chaining"],
  "return_object" : "Graphics"
}
Draw an ellipse in the Foreground Color
*/
JsVar *jswrap_graphics_drawEllipse(JsVar *parent, int x, int y, int x2, int y2) {
   JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return 0;
   graphicsDrawEllipse(&gfx, (short)x,(short)y,(short)x2,(short)y2);
   graphicsSetVar(&gfx); // gfx data changed because modified area
   return jsvLockAgain(parent);
 }

 /*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "getPixel",
  "generate" : "jswrap_graphics_getPixel",
  "params" : [
    ["x","int32","The left"],
    ["y","int32","The top"]
  ],
  "return" : ["int32","The color"]
}
Get a pixel's color
*/
int jswrap_graphics_getPixel(JsVar *parent, int x, int y) {
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return 0;
  return (int)graphicsGetPixel(&gfx, (short)x, (short)y);
}

/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "setPixel",
  "generate" : "jswrap_graphics_setPixel",
  "params" : [
    ["x","int32","The left"],
    ["y","int32","The top"],
    ["col","JsVar","The color"]
  ],
  "return" : ["JsVar","The instance of Graphics this was called on, to allow call chaining"],
  "return_object" : "Graphics"
}
Set a pixel's color
*/
JsVar *jswrap_graphics_setPixel(JsVar *parent, int x, int y, JsVar *color) {
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return 0;
  unsigned int col = gfx.data.fgColor;
  if (!jsvIsUndefined(color))
    col = (unsigned int)jsvGetInteger(color);
  graphicsSetPixel(&gfx, (short)x, (short)y, col);
  gfx.data.cursorX = (short)x;
  gfx.data.cursorY = (short)y;
  graphicsSetVar(&gfx); // gfx data changed because modified area
  return jsvLockAgain(parent);
}

/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "setColor",
  "generate_full" : "jswrap_graphics_setColorX(parent, r,g,b, true)",
  "params" : [
    ["r","JsVar","Red (between 0 and 1) OR an integer representing the color in the current bit depth and color order"],
    ["g","JsVar","Green (between 0 and 1)"],
    ["b","JsVar","Blue (between 0 and 1)"]
  ],
  "return" : ["JsVar","The instance of Graphics this was called on, to allow call chaining"],
  "return_object" : "Graphics"
}
Set the color to use for subsequent drawing operations.

If just `r` is specified as an integer, the numeric value will be written directly into a pixel. eg. On a 24 bit `Graphics` instance you set bright blue with either `g.setColor(0,0,1)` or `g.setColor(0x0000FF)`.

The mapping is as follows:

* 32 bit: `r,g,b` => `0xFFrrggbb`
* 24 bit: `r,g,b` => `0xrrggbb`
* 16 bit: `r,g,b` => `0brrrrrggggggbbbbb` (RGB565)
* Other bpp: `r,g,b` => white if `r+g+b > 50%`, otherwise black (use `r` on its own as an integer)

If you specified `color_order` when creating the `Graphics` instance, `r`,`g` and `b` will be swapped as you specified.

**Note:** On devices with low flash memory, `r` **must** be an integer representing the color in the current bit depth. It cannot
be a floating point value, and `g` and `b` are ignored.
*/
/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "setBgColor",
  "generate_full" : "jswrap_graphics_setColorX(parent, r,g,b, false)",
  "params" : [
    ["r","JsVar","Red (between 0 and 1) OR an integer representing the color in the current bit depth and color order"],
    ["g","JsVar","Green (between 0 and 1)"],
    ["b","JsVar","Blue (between 0 and 1)"]
  ],
  "return" : ["JsVar","The instance of Graphics this was called on, to allow call chaining"],
  "return_object" : "Graphics"
}
Set the background color to use for subsequent drawing operations. 

See `Graphics.setColor` for more information on the mapping of `r`, `g`, and `b` to pixel values.

**Note:** On devices with low flash memory, `r` **must** be an integer representing the color in the current bit depth. It cannot
be a floating point value, and `g` and `b` are ignored.
*/
JsVar *jswrap_graphics_setColorX(JsVar *parent, JsVar *r, JsVar *g, JsVar *b, bool isForeground) {
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return 0;
  unsigned int color = 0;
#ifdef SAVE_ON_FLASH
  if (false) {
#else
  JsVarFloat rf, gf, bf;
  rf = jsvGetFloat(r);
  gf = jsvGetFloat(g);
  bf = jsvGetFloat(b);
  if (!jsvIsUndefined(g) && !jsvIsUndefined(b)) {
    int ri = (int)(rf*256);
    int gi = (int)(gf*256);
    int bi = (int)(bf*256);
    if (ri>255) ri=255;
    if (gi>255) gi=255;
    if (bi>255) bi=255;
    if (ri<0) ri=0;
    if (gi<0) gi=0;
    if (bi<0) bi=0;
    // Check if we need to twiddle colors
    int colorMask = gfx.data.flags & JSGRAPHICSFLAGS_COLOR_MASK;
    if (colorMask) {
      int tmpr, tmpg, tmpb;
      tmpr = ri;
      tmpg = gi;
      tmpb = bi;
      switch (colorMask) {
        case JSGRAPHICSFLAGS_COLOR_BRG:
          ri = tmpb;
          gi = tmpr;
          bi = tmpg;
          break;
        case JSGRAPHICSFLAGS_COLOR_BGR:
          ri = tmpb;
          bi = tmpr;
          break;
        case JSGRAPHICSFLAGS_COLOR_GBR:
          ri = tmpg;
          gi = tmpb;
          bi = tmpr;
          break;
        case JSGRAPHICSFLAGS_COLOR_GRB:
          ri = tmpg;
          gi = tmpr;
          break;
        case JSGRAPHICSFLAGS_COLOR_RBG:
          gi = tmpb;
          bi = tmpg;
          break;
        default: break;
      }
    }
    if (gfx.data.bpp==16) {
      color = (unsigned int)((bi>>3) | (gi>>2)<<5 | (ri>>3)<<11);
    } else if (gfx.data.bpp==32) {
      color = 0xFF000000 | (unsigned int)(bi | (gi<<8) | (ri<<16));
    } else if (gfx.data.bpp==24) {
      color = (unsigned int)(bi | (gi<<8) | (ri<<16));
    } else
      color = (unsigned int)(((ri+gi+bi)>=384) ? 0xFFFFFFFF : 0);
#endif
  } else {
    // just rgb
    color = (unsigned int)jsvGetInteger(r);
  }
  if (isForeground)
    gfx.data.fgColor = color;
  else
    gfx.data.bgColor = color;
  graphicsSetVar(&gfx);
  return jsvLockAgain(parent);
}

/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "getColor",
  "generate_full" : "jswrap_graphics_getColorX(parent, true)",
  "return" : ["int","The integer value of the colour"]
}
Get the color to use for subsequent drawing operations
*/
/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "getBgColor",
  "generate_full" : "jswrap_graphics_getColorX(parent, false)",
  "return" : ["int","The integer value of the colour"]
}
Get the background color to use for subsequent drawing operations
*/
JsVarInt jswrap_graphics_getColorX(JsVar *parent, bool isForeground) {
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return 0;
  return (JsVarInt)((isForeground ? gfx.data.fgColor : gfx.data.bgColor) & ((1UL<<gfx.data.bpp)-1));
}

/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "setFontBitmap",
  "generate_full" : "jswrap_graphics_setFontSizeX(parent, JSGRAPHICS_FONTSIZE_4X6, false)",
  "return" : ["JsVar","The instance of Graphics this was called on, to allow call chaining"],
  "return_object" : "Graphics"
}
Make subsequent calls to `drawString` use the built-in 4x6 pixel bitmapped Font
*/
/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "setFontVector",
  "ifndef" : "SAVE_ON_FLASH",
  "generate_full" : "jswrap_graphics_setFontSizeX(parent, size, true)",
  "params" : [
    ["size","int32","The height of the font, as an integer"]
  ],
  "return" : ["JsVar","The instance of Graphics this was called on, to allow call chaining"],
  "return_object" : "Graphics"
}
Make subsequent calls to `drawString` use a Vector Font of the given height
*/
JsVar *jswrap_graphics_setFontSizeX(JsVar *parent, int size, bool isVectorFont) {
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return 0;
#ifdef NO_VECTOR_FONT
  if (isVectorFont)
    jsExceptionHere(JSET_ERROR, "No vector font in this build");
#else
  if (isVectorFont) {
    if (size<1) size=1;
    if (size>1023) size=1023;
  }
  if (gfx.data.fontSize == JSGRAPHICS_FONTSIZE_CUSTOM) {
    jsvObjectRemoveChild(parent, JSGRAPHICS_CUSTOMFONT_BMP);
    jsvObjectRemoveChild(parent, JSGRAPHICS_CUSTOMFONT_WIDTH);
    jsvObjectRemoveChild(parent, JSGRAPHICS_CUSTOMFONT_HEIGHT);
    jsvObjectRemoveChild(parent, JSGRAPHICS_CUSTOMFONT_FIRSTCHAR);
  }
  gfx.data.fontSize = (short)size;
  graphicsSetVar(&gfx);
#endif
  return jsvLockAgain(parent);
}
/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "setFontCustom",
  "generate" : "jswrap_graphics_setFontCustom",
  "params" : [
    ["bitmap","JsVar","A column-first, MSB-first, 1bpp bitmap containing the font bitmap"],
    ["firstChar","int32","The first character in the font - usually 32 (space)"],
    ["width","JsVar","The width of each character in the font. Either an integer, or a string where each character represents the width"],
    ["height","int32","The height as an integer"]
  ],
  "return" : ["JsVar","The instance of Graphics this was called on, to allow call chaining"],
  "return_object" : "Graphics"
}
Make subsequent calls to `drawString` use a Custom Font of the given height. See the [Fonts page](http://www.espruino.com/Fonts) for more
information about custom fonts and how to create them.
*/
JsVar *jswrap_graphics_setFontCustom(JsVar *parent, JsVar *bitmap, int firstChar, JsVar *width, int height) {
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return 0;

  if (!jsvIsString(bitmap)) {
    jsExceptionHere(JSET_ERROR, "Font bitmap must be a String");
    return 0;
  }
  if (firstChar<0 || firstChar>255) {
    jsExceptionHere(JSET_ERROR, "First character out of range");
    return 0;
  }
  if (!jsvIsString(width) && !jsvIsInt(width)) {
    jsExceptionHere(JSET_ERROR, "Font width must be a String or an integer");
    return 0;
  }
  if (height<=0 || height>255) {
   jsExceptionHere(JSET_ERROR, "Invalid height");
   return 0;
 }
  jsvObjectSetChild(parent, JSGRAPHICS_CUSTOMFONT_BMP, bitmap);
  jsvObjectSetChild(parent, JSGRAPHICS_CUSTOMFONT_WIDTH, width);
  jsvObjectSetChildAndUnLock(parent, JSGRAPHICS_CUSTOMFONT_HEIGHT, jsvNewFromInteger(height));
  jsvObjectSetChildAndUnLock(parent, JSGRAPHICS_CUSTOMFONT_FIRSTCHAR, jsvNewFromInteger(firstChar));
  gfx.data.fontSize = JSGRAPHICS_FONTSIZE_CUSTOM;
  graphicsSetVar(&gfx);
  return jsvLockAgain(parent);
}
/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "setFontAlign",
  "ifndef" : "SAVE_ON_FLASH",
  "generate" : "jswrap_graphics_setFontAlign",
  "params" : [
    ["x","int32","X alignment. -1=left (default), 0=center, 1=right"],
    ["y","int32","Y alignment. -1=top (default), 0=center, 1=bottom"],
    ["rotation","int32","Rotation of the text. 0=normal, 1=90 degrees clockwise, 2=180, 3=270"]
  ],
  "return" : ["JsVar","The instance of Graphics this was called on, to allow call chaining"],
  "return_object" : "Graphics"
}
Set the alignment for subsequent calls to `drawString`
*/
JsVar *jswrap_graphics_setFontAlign(JsVar *parent, int x, int y, int r) {
#ifndef SAVE_ON_FLASH
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return 0;
  if (x<-1) x=-1;
  if (x>1) x=1;
  if (y<-1) y=-1;
  if (y>1) y=1;
  if (r<0) r=0;
  if (r>3) r=3;
  gfx.data.fontAlignX = x;
  gfx.data.fontAlignY = y;
  gfx.data.fontRotate = r;
  graphicsSetVar(&gfx);
  return jsvLockAgain(parent);
#else
  return 0;
#endif
}

/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "drawString",
  "generate" : "jswrap_graphics_drawString",
  "params" : [
    ["str","JsVar","The string"],
    ["x","int32","The X position of the leftmost pixel"],
    ["y","int32","The Y position of the topmost pixel"]
  ],
  "return" : ["JsVar","The instance of Graphics this was called on, to allow call chaining"],
  "return_object" : "Graphics"
}
Draw a string of text in the current font
*/
JsVar *jswrap_graphics_drawString(JsVar *parent, JsVar *var, int x, int y) {
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return 0;

  JsVar *customBitmap = 0, *customWidth = 0;
  int customHeight = 0, customFirstChar = 0;
  if (gfx.data.fontSize>0) {
    customHeight = gfx.data.fontSize;
  } else if (gfx.data.fontSize == JSGRAPHICS_FONTSIZE_4X6) {
    customHeight = 6;
  } else if (gfx.data.fontSize == JSGRAPHICS_FONTSIZE_CUSTOM) {
    customBitmap = jsvObjectGetChild(parent, JSGRAPHICS_CUSTOMFONT_BMP, 0);
    customWidth = jsvObjectGetChild(parent, JSGRAPHICS_CUSTOMFONT_WIDTH, 0);
    customHeight = (int)jsvGetIntegerAndUnLock(jsvObjectGetChild(parent, JSGRAPHICS_CUSTOMFONT_HEIGHT, 0));
    customFirstChar = (int)jsvGetIntegerAndUnLock(jsvObjectGetChild(parent, JSGRAPHICS_CUSTOMFONT_FIRSTCHAR, 0));
  }
#ifndef SAVE_ON_FLASH
  // Handle text rotation
  JsGraphicsFlags oldFlags = gfx.data.flags;
  if (gfx.data.fontRotate==1) {
    gfx.data.flags ^= JSGRAPHICSFLAGS_SWAP_XY | JSGRAPHICSFLAGS_INVERT_X;
    int t = gfx.data.width - (x+1);
    x = y;
    y = t;
  } else if (gfx.data.fontRotate==2) {
    gfx.data.flags ^= JSGRAPHICSFLAGS_INVERT_X | JSGRAPHICSFLAGS_INVERT_Y;
    x = gfx.data.width - (x+1);
    y = gfx.data.height - (y+1);
  } else if (gfx.data.fontRotate==3) {
    gfx.data.flags ^= JSGRAPHICSFLAGS_SWAP_XY | JSGRAPHICSFLAGS_INVERT_Y;
    int t = gfx.data.height - (y+1);
    y = x;
    x = t;
  }
  // Handle font alignment
  if (gfx.data.fontAlignX<2) // 0=center, 1=right, 2=undefined, 3=left
    x -= jswrap_graphics_stringWidth(parent, var) * (gfx.data.fontAlignX+1)/2;
  if (gfx.data.fontAlignY<2)  // 0=center, 1=bottom, 2=undefined, 3=top
    y -= customHeight * (gfx.data.fontAlignX+1)/2;
#endif

  int maxX = (gfx.data.flags & JSGRAPHICSFLAGS_SWAP_XY) ? gfx.data.height : gfx.data.width;
  int maxY = (gfx.data.flags & JSGRAPHICSFLAGS_SWAP_XY) ? gfx.data.width : gfx.data.height;
  int startx = x;
  JsVar *str = jsvAsString(var);
  JsvStringIterator it;
  jsvStringIteratorNew(&it, str, 0);
  while (jsvStringIteratorHasChar(&it)) {
    char ch = jsvStringIteratorGetChar(&it);
    if (ch=='\n') {
      x = startx;
      y += customHeight;
      jsvStringIteratorNext(&it);
      continue;
    }
    if (gfx.data.fontSize>0) {
#ifndef NO_VECTOR_FONT
      int w = (int)graphicsVectorCharWidth(&gfx, gfx.data.fontSize, ch);
      if (x>-w && x<maxX  && y>-gfx.data.fontSize && y<maxY)
        graphicsFillVectorChar(&gfx, (short)x, (short)y, gfx.data.fontSize, ch);
      x+=w;
#endif
    } else if (gfx.data.fontSize == JSGRAPHICS_FONTSIZE_4X6) {
      if (x>-4 && x<maxX && y>-6 && y<maxY)
        graphicsDrawChar4x6(&gfx, (short)x, (short)y, ch);
      x+=4;
    } else if (gfx.data.fontSize == JSGRAPHICS_FONTSIZE_CUSTOM) {
      // get char width and offset in string
      int width = 0, bmpOffset = 0;
      if (jsvIsString(customWidth)) {
        if (ch>=customFirstChar) {
          JsvStringIterator wit;
          jsvStringIteratorNew(&wit, customWidth, 0);
          while (jsvStringIteratorHasChar(&wit) && (int)jsvStringIteratorGetIndex(&wit)<(ch-customFirstChar)) {
            bmpOffset += (unsigned char)jsvStringIteratorGetChar(&wit);
            jsvStringIteratorNext(&wit);
          }
          width = (unsigned char)jsvStringIteratorGetChar(&wit);
          jsvStringIteratorFree(&wit);
        }
      } else {
        width = (int)jsvGetInteger(customWidth);
        bmpOffset = width*(ch-customFirstChar);
      }
      if (ch>=customFirstChar && (x>-width) && (x<maxX) && (y>-customHeight) && y<maxY) {
        bmpOffset *= customHeight;
        // now render character
        JsvStringIterator cit;
        jsvStringIteratorNew(&cit, customBitmap, (size_t)bmpOffset>>3);
        bmpOffset &= 7;
        int cx,cy;
        for (cx=0;cx<width;cx++) {
          for (cy=0;cy<customHeight;cy++) {
            if ((jsvStringIteratorGetChar(&cit)<<bmpOffset)&128)
              graphicsSetPixel(&gfx, (short)(cx+x), (short)(cy+y), gfx.data.fgColor);
            bmpOffset++;
            if (bmpOffset==8) {
              bmpOffset=0;
              jsvStringIteratorNext(&cit);
            }
          }
        }
        jsvStringIteratorFree(&cit);
      }
      x += width;
    }
    if (jspIsInterrupted()) break;
    jsvStringIteratorNext(&it);
  }
  jsvStringIteratorFree(&it);
  jsvUnLock3(str, customBitmap, customWidth);
#ifndef SAVE_ON_FLASH
  gfx.data.flags = oldFlags; // restore flags because of text rotation
  graphicsSetVar(&gfx); // gfx data changed because modified area
#endif
  return jsvLockAgain(parent);
}

/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "stringWidth",
  "generate" : "jswrap_graphics_stringWidth",
  "params" : [
    ["str","JsVar","The string"]
  ],
  "return" : ["int","The length of the string in pixels"]
}
Return the size in pixels of a string of text in the current font
*/
JsVarInt jswrap_graphics_stringWidth(JsVar *parent, JsVar *var) {
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return 0;

  JsVar *customWidth = 0;
  int customFirstChar = 0;
  if (gfx.data.fontSize == JSGRAPHICS_FONTSIZE_CUSTOM) {
    customWidth = jsvObjectGetChild(parent, JSGRAPHICS_CUSTOMFONT_WIDTH, 0);
    customFirstChar = (int)jsvGetIntegerAndUnLock(jsvObjectGetChild(parent, JSGRAPHICS_CUSTOMFONT_FIRSTCHAR, 0));
  }

  JsVar *str = jsvAsString(var);
  JsvStringIterator it;
  jsvStringIteratorNew(&it, str, 0);
  int width = 0;
  int maxWidth = 0;
  while (jsvStringIteratorHasChar(&it)) {
    char ch = jsvStringIteratorGetChar(&it);
    if (ch=='\n') {
      if (width>maxWidth) maxWidth=width;
      width = 0;
    }
    if (gfx.data.fontSize>0) {
#ifndef NO_VECTOR_FONT
      width += (int)graphicsVectorCharWidth(&gfx, gfx.data.fontSize, ch);
#endif
    } else if (gfx.data.fontSize == JSGRAPHICS_FONTSIZE_4X6) {
      width += 4;
    } else if (gfx.data.fontSize == JSGRAPHICS_FONTSIZE_CUSTOM) {
      if (jsvIsString(customWidth)) {
        if (ch>=customFirstChar)
          width += (unsigned char)jsvGetCharInString(customWidth, (size_t)(ch-customFirstChar));
      } else
        width += (int)jsvGetInteger(customWidth);
    }
    jsvStringIteratorNext(&it);
  }
  jsvStringIteratorFree(&it);
  jsvUnLock2(str, customWidth);
  return width>maxWidth ? width : maxWidth;
}

/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "drawLine",
  "generate" : "jswrap_graphics_drawLine",
  "params" : [
    ["x1","int32","The left"],
    ["y1","int32","The top"],
    ["x2","int32","The right"],
    ["y2","int32","The bottom"]
  ],
  "return" : ["JsVar","The instance of Graphics this was called on, to allow call chaining"],
  "return_object" : "Graphics"
}
Draw a line between x1,y1 and x2,y2 in the current foreground color
*/
JsVar *jswrap_graphics_drawLine(JsVar *parent, int x1, int y1, int x2, int y2) {
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return 0;
  graphicsDrawLine(&gfx, (short)x1,(short)y1,(short)x2,(short)y2);
  graphicsSetVar(&gfx); // gfx data changed because modified area
  return jsvLockAgain(parent);
}

/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "lineTo",
  "generate" : "jswrap_graphics_lineTo",
  "params" : [
    ["x","int32","X value"],
    ["y","int32","Y value"]
  ],
  "return" : ["JsVar","The instance of Graphics this was called on, to allow call chaining"],
  "return_object" : "Graphics"
}
Draw a line from the last position of lineTo or moveTo to this position
*/
JsVar *jswrap_graphics_lineTo(JsVar *parent, int x, int y) {
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return 0;
  graphicsDrawLine(&gfx, gfx.data.cursorX, gfx.data.cursorY, (short)x, (short)y);
  gfx.data.cursorX = (short)x;
  gfx.data.cursorY = (short)y;
  graphicsSetVar(&gfx);
  return jsvLockAgain(parent);
}

/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "moveTo",
  "generate" : "jswrap_graphics_moveTo",
  "params" : [
    ["x","int32","X value"],
    ["y","int32","Y value"]
  ],
  "return" : ["JsVar","The instance of Graphics this was called on, to allow call chaining"],
  "return_object" : "Graphics"
}
Move the cursor to a position - see lineTo
*/
JsVar *jswrap_graphics_moveTo(JsVar *parent, int x, int y) {
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return 0;
  gfx.data.cursorX = (short)x;
  gfx.data.cursorY = (short)y;
  graphicsSetVar(&gfx);
  return jsvLockAgain(parent);
}

/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "drawPoly",
  "ifndef" : "SAVE_ON_FLASH",
  "generate" : "jswrap_graphics_drawPoly",
  "params" : [
    ["poly","JsVar","An array of vertices, of the form ```[x1,y1,x2,y2,x3,y3,etc]```"],
    ["closed","bool","Draw another line between the last element of the array and the first"]
  ],
  "return" : ["JsVar","The instance of Graphics this was called on, to allow call chaining"],
  "return_object" : "Graphics"
}
Draw a polyline (lines between each of the points in `poly`) in the current foreground color
*/
JsVar *jswrap_graphics_drawPoly(JsVar *parent, JsVar *poly, bool closed) {
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return 0;
  if (!jsvIsIterable(poly)) return 0;
  int x,y;
  int startx, starty;
  int idx = 0;
  JsvIterator it;
  jsvIteratorNew(&it, poly, JSIF_EVERY_ARRAY_ELEMENT);
  while (jsvIteratorHasElement(&it)) {
    int el = jsvIteratorGetIntegerValue(&it);
    if (idx&1) {
      y = el;
      if (idx==1) { // save xy positions of first point
        startx = x;
        starty = y;
      } else {
        // only start drawing between the first 2 points
        graphicsDrawLine(&gfx, gfx.data.cursorX, gfx.data.cursorY, (short)x, (short)y);
      }
      gfx.data.cursorX = (short)x;
      gfx.data.cursorY = (short)y;
    } else x = el;
    idx++;
    jsvIteratorNext(&it);
  }
  jsvIteratorFree(&it);
  // if closed, draw between first and last points
  if (closed)
    graphicsDrawLine(&gfx, gfx.data.cursorX, gfx.data.cursorY, (short)startx, (short)starty);

  graphicsSetVar(&gfx); // gfx data changed because modified area
  return jsvLockAgain(parent);
}

/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "fillPoly",
  "ifndef" : "SAVE_ON_FLASH",
  "generate" : "jswrap_graphics_fillPoly",
  "params" : [
    ["poly","JsVar","An array of vertices, of the form ```[x1,y1,x2,y2,x3,y3,etc]```"]
  ],
  "return" : ["JsVar","The instance of Graphics this was called on, to allow call chaining"],
  "return_object" : "Graphics"
}
Draw a filled polygon in the current foreground color
*/
JsVar *jswrap_graphics_fillPoly(JsVar *parent, JsVar *poly) {
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return 0;
  if (!jsvIsIterable(poly)) return 0;
  const int maxVerts = 128;
  short verts[maxVerts];
  int idx = 0;
  JsvIterator it;
  jsvIteratorNew(&it, poly, JSIF_EVERY_ARRAY_ELEMENT);
  while (jsvIteratorHasElement(&it) && idx<maxVerts) {
    verts[idx++] = (short)jsvIteratorGetIntegerValue(&it);
    jsvIteratorNext(&it);
  }
  jsvIteratorFree(&it);
  if (idx==maxVerts) {
    jsWarn("Maximum number of points (%d) exceeded for fillPoly", maxVerts/2);
  }
  graphicsFillPoly(&gfx, idx/2, verts);

  graphicsSetVar(&gfx); // gfx data changed because modified area
  return jsvLockAgain(parent);
}

/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "setRotation",
  "generate" : "jswrap_graphics_setRotation",
  "params" : [
    ["rotation","int32","The clockwise rotation. 0 for no rotation, 1 for 90 degrees, 2 for 180, 3 for 270"],
    ["reflect","bool","Whether to reflect the image"]
  ],
  "return" : ["JsVar","The instance of Graphics this was called on, to allow call chaining"],
  "return_object" : "Graphics"
}
Set the current rotation of the graphics device.
*/
JsVar *jswrap_graphics_setRotation(JsVar *parent, int rotation, bool reflect) {
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return 0;

  // clear flags
  gfx.data.flags &= (JsGraphicsFlags)~(JSGRAPHICSFLAGS_SWAP_XY | JSGRAPHICSFLAGS_INVERT_X | JSGRAPHICSFLAGS_INVERT_Y);
  // set flags
  switch (rotation) {
    case 0:
      break;
    case 1:
      gfx.data.flags |= JSGRAPHICSFLAGS_SWAP_XY | JSGRAPHICSFLAGS_INVERT_X;
      break;
    case 2:
      gfx.data.flags |= JSGRAPHICSFLAGS_INVERT_X | JSGRAPHICSFLAGS_INVERT_Y;
      break;
    case 3:
      gfx.data.flags |= JSGRAPHICSFLAGS_SWAP_XY | JSGRAPHICSFLAGS_INVERT_Y;
      break;
  }

  if (reflect) {
    if (gfx.data.flags & JSGRAPHICSFLAGS_SWAP_XY)
      gfx.data.flags ^= JSGRAPHICSFLAGS_INVERT_Y;
    else
      gfx.data.flags ^= JSGRAPHICSFLAGS_INVERT_X;
  }

  graphicsSetVar(&gfx);
  return jsvLockAgain(parent);
}

/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "drawImage",
  "generate" : "jswrap_graphics_drawImage",
  "params" : [
    ["image","JsVar","An object with the following fields `{ width : int, height : int, bpp : optional int, buffer : ArrayBuffer/String, transparent: optional int }`. bpp = bits per pixel (default is 1), transparent (if defined) is the colour that will be treated as transparent"],
    ["x","int32","The X offset to draw the image"],
    ["y","int32","The Y offset to draw the image"]
  ],
  "return" : ["JsVar","The instance of Graphics this was called on, to allow call chaining"],
  "return_object" : "Graphics"
}
Draw an image at the specified position. If the image is 1 bit, the graphics foreground/background colours will be used. Otherwise color data will be copied as-is. Bitmaps are rendered MSB-first
*/
JsVar *jswrap_graphics_drawImage(JsVar *parent, JsVar *image, int xPos, int yPos) {
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return 0;
  if (!jsvIsObject(image)) {
    jsExceptionHere(JSET_ERROR, "Expecting first argument to be an object");
    return 0;
  }
  int imageWidth = (int)jsvGetIntegerAndUnLock(jsvObjectGetChild(image, "width", 0));
  int imageHeight = (int)jsvGetIntegerAndUnLock(jsvObjectGetChild(image, "height", 0));
  int imageBpp = (int)jsvGetIntegerAndUnLock(jsvObjectGetChild(image, "bpp", 0));
  if (imageBpp<=0) imageBpp=1;
  unsigned int imageBitMask = (unsigned int)((1L<<imageBpp)-1L);
  JsVar *transpVar = jsvObjectGetChild(image, "transparent", 0);
  bool imageIsTransparent = transpVar!=0;
  unsigned int imageTransparentCol = (unsigned int)jsvGetInteger(transpVar);
  jsvUnLock(transpVar);
  JsVar *imageBuffer = jsvObjectGetChild(image, "buffer", 0);
  if (!(jsvIsArrayBuffer(imageBuffer) || jsvIsString(imageBuffer)) ||
      imageWidth<=0 ||
      imageHeight<=0 ||
      imageBpp>32) {
    jsExceptionHere(JSET_ERROR, "Expecting first argument to a valid Image");
    jsvUnLock(imageBuffer);
    return 0;
  }
  // jsvGetArrayBufferBackingString is fine to be passed a string
  JsVar *imageBufferString = jsvGetArrayBufferBackingString(imageBuffer);
  jsvUnLock(imageBuffer);

  int x=0, y=0;
  int bits=0;
  unsigned int colData = 0;
  JsvStringIterator it;
  jsvStringIteratorNew(&it, imageBufferString, 0);
  while ((bits>=imageBpp || jsvStringIteratorHasChar(&it)) && y<imageHeight) {
    // Get the data we need...
    while (bits < imageBpp) {
      colData = (colData<<8) | ((unsigned char)jsvStringIteratorGetChar(&it));
      jsvStringIteratorNext(&it);
      bits += 8;
    }
    // extract just the bits we want
    unsigned int col = (colData>>(bits-imageBpp))&imageBitMask;
    bits -= imageBpp;
    // Try and write pixel!
    if (!imageIsTransparent || imageTransparentCol!=col) {
      if (imageBpp==1)
        col = col ? gfx.data.fgColor : gfx.data.bgColor;
      graphicsSetPixel(&gfx, (short)(x+xPos), (short)(y+yPos), col);
    }
    // Go to next pixel
    x++;
    if (x>=imageWidth) {
      x=0;
      y++;
      // we don't care about image height - we'll stop next time...
    }

  }
  jsvStringIteratorFree(&it);
  jsvUnLock(imageBufferString);
  graphicsSetVar(&gfx); // gfx data changed because modified area
  return jsvLockAgain(parent);
}

/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "asImage",
  "ifndef" : "SAVE_ON_FLASH",
  "generate" : "jswrap_graphics_asImage",
  "return" : ["JsVar","An Image that can be used with `Graphics.drawImage`"]
}
Return this Graphics object as an Image that can be used with `Graphics.drawImage`.
Will return undefined if data can't be allocated for it.
*/
JsVar *jswrap_graphics_asImage(JsVar *parent) {
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return 0;
  JsVar *img = jsvNewObject();
  if (!img) return 0;
  int w = jswrap_graphics_getWidthOrHeight(parent,false);
  int h = jswrap_graphics_getWidthOrHeight(parent,true);
  int bpp = gfx.data.bpp;
  jsvObjectSetChildAndUnLock(img,"width",jsvNewFromInteger(w));
  jsvObjectSetChildAndUnLock(img,"height",jsvNewFromInteger(h));
  if (bpp!=1) jsvObjectSetChildAndUnLock(img,"bpp",jsvNewFromInteger(bpp));
  int len = (w*h*bpp+7)>>3;
  JsVar *buffer = jsvNewStringOfLength((unsigned)len, NULL);
  if (!buffer) { // not enough memory
    jsvUnLock(img);
    return 0;
  }

  int x=0, y=0;
  unsigned int pixelBits = 0;
  unsigned int pixelBitCnt = 0;
  JsvStringIterator it;
  jsvStringIteratorNew(&it, buffer, 0);
  while (jsvStringIteratorHasChar(&it)) {
    pixelBits = (pixelBits<<bpp) | graphicsGetPixel(&gfx, (short)x, (short)y);
    pixelBitCnt += (unsigned)bpp;
    x++;
    if (x>=w) {
      x=0;
      y++;
    }
    while (pixelBitCnt>=8) {
      jsvStringIteratorSetCharAndNext(&it, (char)(pixelBits>>(pixelBitCnt-8)));
      pixelBits = pixelBits>>8;
      pixelBitCnt -= 8;
    }
  }
  jsvStringIteratorFree(&it);
  jsvObjectSetChildAndUnLock(img,"buffer",buffer);
  return img;
}

/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "getModified",
  "ifndef" : "SAVE_ON_FLASH",
  "generate" : "jswrap_graphics_getModified",
  "params" : [
    ["reset","bool","Whether to reset the modified area or not"]
  ],
  "return" : ["JsVar","An object {x1,y1,x2,y2} containing the modified area, or undefined if not modified"]
}
Return the area of the Graphics canvas that has been modified, and optionally clear
the modified area to 0.

For instance if `g.setPixel(10,20)` was called, this would return `{x1:10, y1:20, x2:10, y2:20}`
*/
JsVar *jswrap_graphics_getModified(JsVar *parent, bool reset) {
#ifndef SAVE_ON_FLASH
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return 0;
  JsVar *obj = 0;
  if (gfx.data.modMinX <= gfx.data.modMaxX) { // do we have a rect?
    obj = jsvNewObject();
    if (obj) {
      jsvObjectSetChildAndUnLock(obj, "x1", jsvNewFromInteger(gfx.data.modMinX));
      jsvObjectSetChildAndUnLock(obj, "y1", jsvNewFromInteger(gfx.data.modMinY));
      jsvObjectSetChildAndUnLock(obj, "x2", jsvNewFromInteger(gfx.data.modMaxX));
      jsvObjectSetChildAndUnLock(obj, "y2", jsvNewFromInteger(gfx.data.modMaxY));
    }
  }
  if (reset) {
    gfx.data.modMaxX = -32768;
    gfx.data.modMaxY = -32768;
    gfx.data.modMinX = 32767;
    gfx.data.modMinY = 32767;
    graphicsSetVar(&gfx);
  }
  return obj;
#else
  return 0;
#endif
}

/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "scroll",
  "ifndef" : "SAVE_ON_FLASH",
  "generate" : "jswrap_graphics_scroll",
  "params" : [
    ["x","int32","X direction. >0 = to right"],
    ["y","int32","Y direction. >0 = down"]
  ],
  "return" : ["JsVar","The instance of Graphics this was called on, to allow call chaining"],
  "return_object" : "Graphics"
}
Scroll the contents of this graphics in a certain direction. The remaining area
is filled with the background color.

Note: This uses repeated pixel reads and writes, so will not work on platforms that
don't support pixel reads.
*/
JsVar *jswrap_graphics_scroll(JsVar *parent, int xdir, int ydir) {
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return 0;
  graphicsScroll(&gfx, xdir, ydir);
  // update modified area
  graphicsSetVar(&gfx);
  return jsvLockAgain(parent);
}

/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "asBMP",
  "#if" : "!defined(SAVE_ON_FLASH) && !defined(ESPRUINOBOARD)",
  "generate" : "jswrap_graphics_asBMP",
  "return" : ["JsVar","A String representing the Graphics as a Windows BMP file (or 'undefined' if not possible)"]
}
Create a Windows BMP file from this Graphics instance, and return it as a String.
*/
JsVar *jswrap_graphics_asBMP(JsVar *parent) {
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return 0;
  if (gfx.data.bpp!=1 && gfx.data.bpp!=24) {
    jsExceptionHere(JSET_ERROR, "asBMP/asURL only works on 1bpp/24bpp Graphics");
    return 0;
  }
  int width = (gfx.data.flags & JSGRAPHICSFLAGS_SWAP_XY) ? gfx.data.height : gfx.data.width;
  int height = (gfx.data.flags & JSGRAPHICSFLAGS_SWAP_XY) ? gfx.data.width : gfx.data.height;
  int rowstride = (((width*gfx.data.bpp)+31) >> 5) << 2; // padded to 32 bits
  bool hasPalette = gfx.data.bpp==1;
  int headerLen = 14+ 12+ (hasPalette?6:0);
  int l = headerLen + height*rowstride;
  JsVar *imgData = jsvNewFlatStringOfLength((unsigned)l);
  if (!imgData) return 0; // not enough memory
  unsigned char *imgPtr = (unsigned char *)jsvGetFlatStringPointer(imgData);
  imgPtr[0]=66;
  imgPtr[1]=77;
  imgPtr[2]=(unsigned char)l;
  imgPtr[3]=(unsigned char)(l>>8);  // plus 2 more bytes for size
  imgPtr[10]=(unsigned char)headerLen;
  // BITMAPCOREHEADER
  imgPtr[14]=12; // sizeof(BITMAPCOREHEADER)
  imgPtr[18]=(unsigned char)width;
  imgPtr[19]=(unsigned char)(width>>8);
  imgPtr[20]=(unsigned char)height;
  imgPtr[21]=(unsigned char)(height>>8);
  imgPtr[22]=1;
  imgPtr[24]=(unsigned char)gfx.data.bpp; // bpp
  if (hasPalette) {
    imgPtr[26]=255;
    imgPtr[27]=255;
    imgPtr[28]=255;
  }
  for (int y=0;y<height;y++) {
    int yi = height-(y+1);
    if (gfx.data.bpp==1) {
      for (int x=0;x<width;) {
        unsigned int b = 0;
        for (int i=0;i<8;i++) {
          b = (b<<1)|(graphicsGetPixel(&gfx, (short)(x++), (short)y)&1);
        }
        imgPtr[headerLen + (yi*rowstride) + (x>>3) - 1] = (unsigned char)b;
      }
    } else {
      for (int x=0;x<width;x++) {
        unsigned int c = graphicsGetPixel(&gfx, (short)x, (short)y);
        int i = headerLen + (yi*rowstride) + (x*(gfx.data.bpp>>3));
        imgPtr[i++] = (unsigned char)(c);
        imgPtr[i++] = (unsigned char)(c>>8);
        imgPtr[i++] = (unsigned char)(c>>16);
      }
    }
  }
  return imgData;
}

/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "asURL",
  "#if" : "!defined(SAVE_ON_FLASH) && !defined(ESPRUINOBOARD)",
  "generate" : "jswrap_graphics_asURL",
  "return" : ["JsVar","A String representing the Graphics as a URL (or 'undefined' if not possible)"]
}
Create a URL of the form `data:image/bmp;base64,...` that can be pasted into the browser.

The Espruino Web IDE can detect this data on the console and render the image inline automatically.
*/
JsVar *jswrap_graphics_asURL(JsVar *parent) {
  JsVar *imgData = jswrap_graphics_asBMP(parent);
  if (!imgData) return 0; // not enough memory
  JsVar *b64 = jswrap_btoa(imgData);
  jsvUnLock(imgData);
  if (!b64) return 0; // not enough memory
  JsVar *r = jsvVarPrintf("data:image/bmp;base64,%v",b64);
  jsvUnLock(b64);
  return r;
}

/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "dump",
  "#if" : "!defined(SAVE_ON_FLASH) && !defined(ESPRUINOBOARD)",
  "generate" : "jswrap_graphics_dump"
}
Output this image as a bitmap URL. The Espruino Web IDE can detect the data on the console and render the image inline automatically.

This is identical to `console.log(g.asURL())` - it is just a convenient function for easy debugging.
*/
void jswrap_graphics_dump(JsVar *parent) {
  JsVar *url = jswrap_graphics_asURL(parent);
  if (url) jsiConsolePrintStringVar(url);
  jsvUnLock(url);
  jsiConsolePrint("\n");
}
