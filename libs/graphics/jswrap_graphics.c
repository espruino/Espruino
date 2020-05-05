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
#ifdef USE_LCD_ST7789_8BIT
#include "lcd_st7789_8bit.h"
#endif

#include "jswrap_functions.h" // for asURL

#include "bitmap_font_4x6.h"
#include "bitmap_font_6x8.h"

#ifdef GRAPHICS_PALETTED_IMAGES
// 16 color MAC OS palette
const uint16_t PALETTE_4BIT[16] = { 0x0,0x4228,0x8c51,0xbdd7,0x9b26,0x6180,0x320,0x540,0x4df,0x19,0x3013,0xf813,0xd800,0xfb20,0xffe0,0xffff };
// 256 color 16 bit Web-safe palette
const uint16_t PALETTE_8BIT[256] = {
    0x0,0x6,0xc,0x13,0x19,0x1f,0x180,0x186,0x18c,0x193,0x199,0x19f,0x320,0x326,0x32c,0x333,0x339,0x33f,0x4c0,
    0x4c6,0x4cc,0x4d3,0x4d9,0x4df,0x660,0x666,0x66c,0x673,0x679,0x67f,0x7e0,0x7e6,0x7ec,0x7f3,0x7f9,0x7ff,
    0x3000,0x3006,0x300c,0x3013,0x3019,0x301f,0x3180,0x3186,0x318c,0x3193,0x3199,0x319f,0x3320,0x3326,0x332c,
    0x3333,0x3339,0x333f,0x34c0,0x34c6,0x34cc,0x34d3,0x34d9,0x34df,0x3660,0x3666,0x366c,0x3673,0x3679,0x367f,
    0x37e0,0x37e6,0x37ec,0x37f3,0x37f9,0x37ff,0x6000,0x6006,0x600c,0x6013,0x6019,0x601f,0x6180,0x6186,0x618c,
    0x6193,0x6199,0x619f,0x6320,0x6326,0x632c,0x6333,0x6339,0x633f,0x64c0,0x64c6,0x64cc,0x64d3,0x64d9,0x64df,
    0x6660,0x6666,0x666c,0x6673,0x6679,0x667f,0x67e0,0x67e6,0x67ec,0x67f3,0x67f9,0x67ff,0x9800,0x9806,0x980c,
    0x9813,0x9819,0x981f,0x9980,0x9986,0x998c,0x9993,0x9999,0x999f,0x9b20,0x9b26,0x9b2c,0x9b33,0x9b39,0x9b3f,
    0x9cc0,0x9cc6,0x9ccc,0x9cd3,0x9cd9,0x9cdf,0x9e60,0x9e66,0x9e6c,0x9e73,0x9e79,0x9e7f,0x9fe0,0x9fe6,0x9fec,
    0x9ff3,0x9ff9,0x9fff,0xc800,0xc806,0xc80c,0xc813,0xc819,0xc81f,0xc980,0xc986,0xc98c,0xc993,0xc999,0xc99f,
    0xcb20,0xcb26,0xcb2c,0xcb33,0xcb39,0xcb3f,0xccc0,0xccc6,0xcccc,0xccd3,0xccd9,0xccdf,0xce60,0xce66,0xce6c,
    0xce73,0xce79,0xce7f,0xcfe0,0xcfe6,0xcfec,0xcff3,0xcff9,0xcfff,0xf800,0xf806,0xf80c,0xf813,0xf819,0xf81f,
    0xf980,0xf986,0xf98c,0xf993,0xf999,0xf99f,0xfb20,0xfb26,0xfb2c,0xfb33,0xfb39,0xfb3f,0xfcc0,0xfcc6,0xfccc,
    0xfcd3,0xfcd9,0xfcdf,0xfe60,0xfe66,0xfe6c,0xfe73,0xfe79,0xfe7f,0xffe0,0xffe6,0xffec,0xfff3,0xfff9,0xffff,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xffff
 };
// map Mac to web-safe palette. Must be uint16_t because drawImage uses uint16_t* for palette
const uint16_t PALETTE_4BIT_TO_8BIT[16] = { 0, 43, 129, 172, 121, 78, 12, 18, 23, 4, 39, 183, 144, 192, 210, 215 };
#endif


/*JSON{
  "type" : "class",
  "class" : "Graphics"
}
This class provides Graphics operations that can be applied to a surface.

Use Graphics.createXXX to create a graphics object that renders in the way you want. See [the Graphics page](https://www.espruino.com/Graphics) for more information.

**Note:** On boards that contain an LCD, there is a built-in 'LCD' object of type Graphics. For instance to draw a line you'd type: ```LCD.drawLine(0,0,100,100)```
*/

/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "params" : [ ["all","bool","(only on some devices) If `true` then copy all pixels, not just those that have changed."] ],
  "name" : "flip"
}
On instances of graphics that drive a display with
an offscreen buffer, calling this function will
copy the contents of the offscreen buffer to the
screen.

Call this when you have drawn something to Graphics
and you want it shown on the screen.

If a display does not have an offscreen buffer,
it may not have a `g.flip()` method.

On Bangle.js, there are different graphics modes
chosen with `Bangle.setLCDMode()`. The default mode
is unbuffered and in this mode `g.flip()` does not
affect the screen contents, however it will cause the
screen to light up if it was previously off due
to inactivity.

On some devices, this command will attempt to
only update the areas of the screen that have
changed in order to increase speed. If you have
accessed the `Graphics.buffer` directly then you
may need to use `Graphics.flip(true)` to force
a full update of the screen.
*/
/*JSON{
  "type" : "property",
  "class" : "Graphics",
  "name" : "buffer"
}
On Graphics instances with an offscreen buffer, this
is an `ArrayBuffer` that provides access to the underlying
pixel data.

```
g=Graphics.createArrayBuffer(8,8,8)
g.drawLine(0,0,7,7)
print(new Uint8Array(g.buffer))
new Uint8Array([
255, 0, 0, 0, 0, 0, 0, 0,
0, 255, 0, 0, 0, 0, 0, 0,
0, 0, 255, 0, 0, 0, 0, 0,
0, 0, 0, 255, 0, 0, 0, 0,
0, 0, 0, 0, 255, 0, 0, 0,
0, 0, 0, 0, 0, 255, 0, 0,
0, 0, 0, 0, 0, 0, 255, 0,
0, 0, 0, 0, 0, 0, 0, 255])
```
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
    graphicsStructInit(&gfx,320,240,16);
    gfx.data.type = JSGRAPHICSTYPE_FSMC;
    gfx.graphicsVar = parentObj;
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
  graphicsStructInit(&gfx,width,height,bpp);
  gfx.data.type = JSGRAPHICSTYPE_ARRAYBUFFER;
  gfx.data.flags = JSGRAPHICSFLAGS_NONE;
  gfx.graphicsVar = parent;

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
  graphicsStructInit(&gfx,width,height,bpp);
  gfx.data.type = JSGRAPHICSTYPE_JS;
  gfx.graphicsVar = parent;
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
    ["height","int32","Pixels high"],
    ["bpp","int32","Bits per pixel (8,16,24 or 32 supported)"]
  ],
  "return" : ["JsVar","The new Graphics object"],
  "return_object" : "Graphics"
}
Create a Graphics object that renders to SDL window (Linux-based devices only)
*/
JsVar *jswrap_graphics_createSDL(int width, int height, int bpp) {
  if (width<=0 || height<=0 || width>32767 || height>32767) {
    jsExceptionHere(JSET_ERROR, "Invalid Size");
    return 0;
  }

  JsVar *parent = jspNewObject(0, "Graphics");
  if (!parent) return 0; // low memory
  JsGraphics gfx;
  graphicsStructInit(&gfx,width,height,bpp);
  gfx.data.type = JSGRAPHICSTYPE_SDL;
  gfx.graphicsVar = parent;
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
  "name" : "reset",
  "generate" : "jswrap_graphics_reset",
  "return" : ["JsVar","The instance of Graphics this was called on, to allow call chaining"],
  "return_object" : "Graphics"
}
Reset the state of Graphics to the defaults (eg. Color, Font, etc)
that would have been used when Graphics was initialised.
*/
JsVar *jswrap_graphics_reset(JsVar *parent) {
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return 0;
  // reset font, which will unreference any custom fonts stored inside the instance
  jswrap_graphics_setFontSizeX(parent, 1+JSGRAPHICS_FONTSIZE_4X6, false);
  // properly reset state
  graphicsStructResetState(&gfx);
  graphicsSetVar(&gfx); // gfx data changed because modified area
  return jsvLockAgain(parent);
}

/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "clear",
  "generate" : "jswrap_graphics_clear",
  "params" : [
    ["reset","bool","If `true`, resets the state of Graphics to the default (eg. Color, Font, etc) as if calling `Graphics.reset`"]
  ],
  "return" : ["JsVar","The instance of Graphics this was called on, to allow call chaining"],
  "return_object" : "Graphics"
}
Clear the LCD with the Background Color
*/
JsVar *jswrap_graphics_clear(JsVar *parent, bool resetState) {
  if (resetState) jsvUnLock(jswrap_graphics_reset(parent));
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return 0;
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
  graphicsFillRect(&gfx, x1,y1,x2,y2,gfx.data.fgColor);
  graphicsSetVar(&gfx); // gfx data changed because modified area
  return jsvLockAgain(parent);
}

/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "clearRect",
  "ifndef" : "SAVE_ON_FLASH",
  "generate" : "jswrap_graphics_clearRect",
  "params" : [
    ["x1","int32","The left X coordinate"],
    ["y1","int32","The top Y coordinate"],
    ["x2","int32","The right X coordinate"],
    ["y2","int32","The bottom Y coordinate"]
  ],
  "return" : ["JsVar","The instance of Graphics this was called on, to allow call chaining"],
  "return_object" : "Graphics"
}
Fill a rectangular area in the Background Color
*/
JsVar *jswrap_graphics_clearRect(JsVar *parent, int x1, int y1, int x2, int y2) {
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return 0;
  graphicsFillRect(&gfx, x1,y1,x2,y2,gfx.data.bgColor);
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
  graphicsDrawRect(&gfx, x1,y1,x2,y2);
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
   graphicsFillEllipse(&gfx, x,y,x2,y2);
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
   graphicsDrawEllipse(&gfx, x,y,x2,y2);
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
  return (int)graphicsGetPixel(&gfx, x, y);
}

/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "setPixel",
  "generate" : "jswrap_graphics_setPixel",
  "params" : [
    ["x","int32","The left"],
    ["y","int32","The top"],
    ["col","JsVar","The color (if `undefined`, the foreground color is useD)"]
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
    col = jswrap_graphics_toColor(parent,color,0,0);
  graphicsSetPixel(&gfx, x, y, col);
  gfx.data.cursorX = (short)x;
  gfx.data.cursorY = (short)y;
  graphicsSetVar(&gfx); // gfx data changed because modified area
  return jsvLockAgain(parent);
}

/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "toColor",
  "ifndef" : "SAVE_ON_FLASH",
  "generate" : "jswrap_graphics_toColor",
  "params" : [
    ["r","JsVar","Red (between 0 and 1) **OR** an integer representing the color in the current bit depth and color order **OR** a hexidecimal color string of the form `'#012345'`"],
    ["g","JsVar","Green (between 0 and 1)"],
    ["b","JsVar","Blue (between 0 and 1)"]
  ],
  "return" : ["int","The color index represented by the arguments"]
}
Work out the color value to be used in the current bit depth based on the arguments.

This is used internally by setColor and setBgColor

```
// 1 bit
g.toColor(1,1,1) => 1
// 16 bit
g.toColor(1,0,0) => 0xF800
```
*/

unsigned int jswrap_graphics_toColor(JsVar *parent, JsVar *r, JsVar *g, JsVar *b) {
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return 0;
  unsigned int color = 0;
#ifdef SAVE_ON_FLASH
  if (false) {
#else
  JsVarFloat rf, gf, bf;
  if (jsvIsString(r)) {
    char buf[9];
    memset(buf,0,sizeof(buf));
    jsvGetString(r,buf,sizeof(buf));
    rf = hexToByte(buf[1],buf[2])/256.0;
    gf = hexToByte(buf[3],buf[4])/256.0;
    bf = hexToByte(buf[5],buf[6])/256.0;
    if (rf<0 || gf<0 || bf<0 || buf[7]!=0) {
      jsExceptionHere(JSET_ERROR, "If Color is a String, it must be of the form '#rrggbb'");
      return 0;
    }
  } else {
    rf = jsvGetFloat(r);
    gf = jsvGetFloat(g);
    bf = jsvGetFloat(b);
  }
  if (isfinite(rf) && isfinite(gf) && isfinite(bf)) {
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
#if defined(GRAPHICS_PALETTED_IMAGES)
    } else if (gfx.data.bpp==4) {
      // LCD is paletted - look up in our palette to find the best match
      int d = 0x7FFFFFFF;
      color = 0;
      for (unsigned int i=0;i<16;i++) {
        int p = PALETTE_4BIT[i];
        int pr = (p>>8)&0xF8;
        int pg = (p>>3)&0xFC;
        int pb = (p<<3)&0xF8;
        pr |= pr>>5;
        pg |= pb>>6;
        pb |= pb>>5;
        int dr = pr-ri;
        int dg = pg-gi;
        int db = pb-bi;
        int dd = dr*dr+dg*dg+db*db;
        if (dd<d) {
          d=dd;
          color=i;
        }
      }
    } else if (gfx.data.bpp==8) {
      // LCD is paletted - look up in our palette to find the best match
      // TODO: For web palette we should be able to cheat without searching...
      int d = 0x7FFFFFFF;
      color = 0;
      for (int i=0;i<255;i++) {
        int p = PALETTE_8BIT[i];
        int pr = (p>>8)&0xF8;
        int pg = (p>>3)&0xFC;
        int pb = (p<<3)&0xF8;
        pr |= pr>>5;
        pg |= pb>>6;
        pb |= pb>>5;
        int dr = pr-ri;
        int dg = pg-gi;
        int db = pb-bi;
        int dd = dr*dr+dg*dg+db*db;
        if (dd<d) {
          d=dd;
          color=(unsigned int)i;
        }
      }
#endif
    } else
      color = (unsigned int)(((ri+gi+bi)>=384) ? 0xFFFFFFFF : 0);
#endif
  } else {
    // just rgb
    color = (unsigned int)jsvGetInteger(r);
  }
  return color;
}

/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "setColor",
  "generate_full" : "jswrap_graphics_setColorX(parent, r,g,b, true)",
  "params" : [
    ["r","JsVar","Red (between 0 and 1) **OR** an integer representing the color in the current bit depth and color order **OR** a hexidecimal color string of the form `'#012345'`"],
    ["g","JsVar","Green (between 0 and 1)"],
    ["b","JsVar","Blue (between 0 and 1)"]
  ],
  "return" : ["JsVar","The instance of Graphics this was called on, to allow call chaining"],
  "return_object" : "Graphics"
}
Set the color to use for subsequent drawing operations.

If just `r` is specified as an integer, the numeric value will be written directly into a pixel. eg. On a 24 bit `Graphics` instance you set bright blue with either `g.setColor(0,0,1)` or `g.setColor(0x0000FF)`.

A good shortcut to ensure you get white on all platforms is to use `g.setColor(-1)`

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
    ["r","JsVar","Red (between 0 and 1) **OR** an integer representing the color in the current bit depth and color order **OR** a hexidecimal color string of the form `'#012345'`"],
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
  unsigned int color = jswrap_graphics_toColor(parent,r,g,b);
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
  "name" : "setClipRect",
  "ifndef" : "SAVE_ON_FLASH",
  "generate" : "jswrap_graphics_setClipRect",
  "params" : [
    ["x1","int","Top left X coordinate"],
    ["y1","int","Top left Y coordinate"],
    ["x2","int","Bottom right X coordinate"],
    ["y2","int","Bottom right Y coordinate"]
  ],
  "return" : ["JsVar","The instance of Graphics this was called on, to allow call chaining"],
  "return_object" : "Graphics"
}
This sets the 'clip rect' that subsequent drawing operations are clipped to
sit between.

These values are inclusive - eg `g.setClipRect(1,0,5,0)` will ensure that only
pixel rows 1,2,3,4,5 are touched on column 0.

**Note:** For maximum flexibility, the values here are not range checked. For normal
use, unsure X and Y are between 0 and `getWidth`/`getHeight`.
*/
JsVar *jswrap_graphics_setClipRect(JsVar *parent, int x1, int y1, int x2, int y2) {
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return 0;
#ifndef SAVE_ON_FLASH
  gfx.data.clipRect.x1 = (unsigned short)x1;
  gfx.data.clipRect.y1 = (unsigned short)y1;
  gfx.data.clipRect.x2 = (unsigned short)x2;
  gfx.data.clipRect.y2 = (unsigned short)y2;
  graphicsSetVar(&gfx);
#endif
  return jsvLockAgain(parent);
}


/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "setFontBitmap",
  "generate_full" : "jswrap_graphics_setFontSizeX(parent, 1+JSGRAPHICS_FONTSIZE_4X6, false)",
  "return" : ["JsVar","The instance of Graphics this was called on, to allow call chaining"],
  "return_object" : "Graphics"
}
Make subsequent calls to `drawString` use the built-in 4x6 pixel bitmapped Font

It is recommended that you use `Graphics.setFont("4x6")` for more flexibility.
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
Make subsequent calls to `drawString` use a Vector Font of the given height.

It is recommended that you use `Graphics.setFont("Vector", size)` for more flexibility.
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
  if ((gfx.data.fontSize&JSGRAPHICS_FONTSIZE_FONT_MASK) == JSGRAPHICS_FONTSIZE_CUSTOM &&
      (size&JSGRAPHICS_FONTSIZE_FONT_MASK) != JSGRAPHICS_FONTSIZE_CUSTOM) {
    jsvObjectRemoveChild(parent, JSGRAPHICS_CUSTOMFONT_BMP);
    jsvObjectRemoveChild(parent, JSGRAPHICS_CUSTOMFONT_WIDTH);
    jsvObjectRemoveChild(parent, JSGRAPHICS_CUSTOMFONT_HEIGHT);
    jsvObjectRemoveChild(parent, JSGRAPHICS_CUSTOMFONT_FIRSTCHAR);
  }
  gfx.data.fontSize = (unsigned short)size;
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
    ["height","int32","The height as an integer (max 255). Bits 8-15 represent the scale factor (eg. `2<<8` is twice the size)"]
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
  int scale = height>>8;
  if (scale<1) scale=1;
  height = height&255;
  jsvObjectSetChild(parent, JSGRAPHICS_CUSTOMFONT_BMP, bitmap);
  jsvObjectSetChild(parent, JSGRAPHICS_CUSTOMFONT_WIDTH, width);
  jsvObjectSetChildAndUnLock(parent, JSGRAPHICS_CUSTOMFONT_HEIGHT, jsvNewFromInteger(height));
  jsvObjectSetChildAndUnLock(parent, JSGRAPHICS_CUSTOMFONT_FIRSTCHAR, jsvNewFromInteger(firstChar));
  gfx.data.fontSize = (unsigned short)(scale + JSGRAPHICS_FONTSIZE_CUSTOM);
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
  "name" : "setFont",
  "ifndef" : "SAVE_ON_FLASH",
  "generate" : "jswrap_graphics_setFont",
  "params" : [
    ["name","JsVar","The name of the font to use (if undefined, the standard 4x6 font will be used)"],
    ["size","int","The size of the font (or undefined)"]
  ],
  "return" : ["JsVar","The instance of Graphics this was called on, to allow call chaining"],
  "return_object" : "Graphics"
}
Set the font by name, eg "4x6", or "Vector12".

For bitmap fonts you can also specify a size multiplier, for example `g.setFont("4x6",2)` will double the size of the standard 4x6 bitmap font to 8x12.
*/
JsVar *jswrap_graphics_setFont(JsVar *parent, JsVar *name, int size) {
#ifndef SAVE_ON_FLASH
  if (!jsvIsString(name)) return 0;
  unsigned short sz = 0xFFFF;
  bool isVector = false;
#ifndef NO_VECTOR_FONT
  if (jsvIsStringEqualOrStartsWith(name, "Vector", true)) {
    sz = (unsigned short)jsvGetIntegerAndUnLock(jsvNewFromStringVar(name, 6, JSVAPPENDSTRINGVAR_MAXLENGTH));
    if (size>0) sz = (unsigned short)size;
    isVector = true;
  }
#endif
  if (size<1) size=1;
  if (size>JSGRAPHICS_FONTSIZE_SCALE_MASK) size=JSGRAPHICS_FONTSIZE_SCALE_MASK;
  if (jsvIsUndefined(name) || jsvIsStringEqual(name, "4x6"))
    sz = (unsigned short)(size + JSGRAPHICS_FONTSIZE_4X6);
#ifdef USE_FONT_6X8
  if (jsvIsStringEqual(name, "6x8"))
    sz = (unsigned short)(size + JSGRAPHICS_FONTSIZE_6X8);
#endif
  // TODO: if function named 'setFontXYZ' exists, run it
  if (sz==0xFFFF) {
    JsVar *setterName = jsvVarPrintf("setFont%v",name);
    JsVar *fontSetter = jspGetVarNamedField(parent,setterName,false);
    if (fontSetter) {
      jsvUnLock(jspExecuteFunction(fontSetter,parent,0,NULL));
      sz = (unsigned short)(size + JSGRAPHICS_FONTSIZE_CUSTOM);
    }
    jsvUnLock2(fontSetter,setterName);
  }
  if (sz==0xFFFF) {
    jsExceptionHere(JSET_ERROR, "Unknown font %j", name);
  }
  return jswrap_graphics_setFontSizeX(parent, sz, isVector);
#else
  return 0;
#endif
}

/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "getFont",
  "ifndef" : "SAVE_ON_FLASH",
  "generate" : "jswrap_graphics_getFont",
  "return" : ["JsVar","Get the name of the current font"],
  "return_object" : "String"
}
Get the font by name - can be saved and used with `Graphics.setFont`
*/
JsVar *jswrap_graphics_getFont(JsVar *parent) {
#ifndef SAVE_ON_FLASH
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return 0;
  JsGraphicsFontSize f = gfx.data.fontSize & JSGRAPHICS_FONTSIZE_FONT_MASK;
#ifndef NO_VECTOR_FONT
  if (f == JSGRAPHICS_FONTSIZE_VECTOR)
    return jsvVarPrintf("Vector%d",gfx.data.fontSize & JSGRAPHICS_FONTSIZE_SCALE_MASK);
#endif
  if (f == JSGRAPHICS_FONTSIZE_4X6)
    return jsvNewFromString("4x6");
#ifdef USE_FONT_6X8
  if (f == JSGRAPHICS_FONTSIZE_6X8)
    return jsvNewFromString("6x8");
#endif
  if (f == JSGRAPHICS_FONTSIZE_CUSTOM) {
    // not implemented yet because it's painful trying to pass 5 arguments into setFontCustom
    /*JsVar *n = jsvObjectGetChild(parent, JSGRAPHICS_CUSTOMFONT_NAME, 0);
    if (n) return n;*/
    return jsvNewFromString("Custom");
  }
  return jsvNewFromInteger(gfx.data.fontSize);
#else
  return 0;
#endif
}
/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "getFonts",
  "ifndef" : "SAVE_ON_FLASH",
  "generate" : "jswrap_graphics_getFonts",
  "return" : ["JsVar","And array of font names"],
  "return_object" : "Array"
}
Return an array of all fonts currently in the Graphics library.

**Note:** Vector fonts are specified as `Vector#` where `#` is the font height. As there
are effectively infinite fonts, just `Vector` is included in the list.
*/
JsVar *jswrap_graphics_getFonts(JsVar *parent) {
#ifndef SAVE_ON_FLASH
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return 0;
  JsVar *arr = jsvNewEmptyArray();
  if (!arr) return 0;
  jsvArrayPushAndUnLock(arr, jsvNewFromString("4x6"));
#ifdef USE_FONT_6X8
  jsvArrayPushAndUnLock(arr, jsvNewFromString("6x8"));
#endif
#ifndef NO_VECTOR_FONT
  jsvArrayPushAndUnLock(arr, jsvNewFromString("Vector"));
#endif
  JsVar *c = jspGetPrototype(parent);
  JsvObjectIterator it;
  jsvObjectIteratorNew(&it, c);
  while (jsvObjectIteratorHasValue(&it)) {
    JsVar *k = jsvObjectIteratorGetKey(&it);
    if (jsvIsStringEqualOrStartsWith(k,"setFont",true))
      jsvArrayPushAndUnLock(arr, jsvNewFromStringVar(k,7,JSVAPPENDSTRINGVAR_MAXLENGTH));
    jsvUnLock(k);
    jsvObjectIteratorNext(&it);
  }
  jsvObjectIteratorFree(&it);
  jsvUnLock(c);
  return arr;
#else
  return 0;
#endif
}

/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "getFontHeight",
  "ifndef" : "SAVE_ON_FLASH",
  "generate" : "jswrap_graphics_getFontHeight",
  "return" : ["int","The height in pixels of the current font"]
}
Return the height in pixels of the current font
*/
static int jswrap_graphics_getFontHeightInternal(JsGraphics *gfx) {
  JsGraphicsFontSize f = gfx->data.fontSize & JSGRAPHICS_FONTSIZE_FONT_MASK;
  unsigned short scale = gfx->data.fontSize & JSGRAPHICS_FONTSIZE_SCALE_MASK;
  if (f == JSGRAPHICS_FONTSIZE_VECTOR) {
    return scale;
  } else if (f == JSGRAPHICS_FONTSIZE_4X6) {
    return 6*scale;
#ifdef USE_FONT_6X8
  } else if (f == JSGRAPHICS_FONTSIZE_6X8) {
    return 8*scale;
#endif
  } else if (f == JSGRAPHICS_FONTSIZE_CUSTOM) {
    return scale*(int)jsvGetIntegerAndUnLock(jsvObjectGetChild(gfx->graphicsVar, JSGRAPHICS_CUSTOMFONT_HEIGHT, 0));
  }
  return 0;
}
int jswrap_graphics_getFontHeight(JsVar *parent) {
#ifndef SAVE_ON_FLASH
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return 0;
  return jswrap_graphics_getFontHeightInternal(&gfx);
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
    ["y","int32","The Y position of the topmost pixel"],
    ["solid","bool","For bitmap fonts, should empty pixels be filled with the background color?"]
  ],
  "return" : ["JsVar","The instance of Graphics this was called on, to allow call chaining"],
  "return_object" : "Graphics"
}
Draw a string of text in the current font
*/
JsVar *jswrap_graphics_drawString(JsVar *parent, JsVar *var, int x, int y, bool solidBackground) {
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return 0;

  JsVar *customBitmap = 0, *customWidth = 0;
  int customHeight = jswrap_graphics_getFontHeightInternal(&gfx);
  int customFirstChar = 0;
  JsGraphicsFontSize font = gfx.data.fontSize & JSGRAPHICS_FONTSIZE_FONT_MASK;
  unsigned short scale = gfx.data.fontSize & JSGRAPHICS_FONTSIZE_SCALE_MASK;

  if (font == JSGRAPHICS_FONTSIZE_CUSTOM) {
    customBitmap = jsvObjectGetChild(parent, JSGRAPHICS_CUSTOMFONT_BMP, 0);
    customWidth = jsvObjectGetChild(parent, JSGRAPHICS_CUSTOMFONT_WIDTH, 0);
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
    y -= customHeight * (gfx.data.fontAlignY+1)/2;

  int minX = (gfx.data.flags & JSGRAPHICSFLAGS_SWAP_XY) ? gfx.data.clipRect.y1 : gfx.data.clipRect.x1;
  int minY = (gfx.data.flags & JSGRAPHICSFLAGS_SWAP_XY) ? gfx.data.clipRect.x1 : gfx.data.clipRect.y1;
  int maxX = (gfx.data.flags & JSGRAPHICSFLAGS_SWAP_XY) ? gfx.data.clipRect.y2 : gfx.data.clipRect.x2;
  int maxY = (gfx.data.flags & JSGRAPHICSFLAGS_SWAP_XY) ? gfx.data.clipRect.x2 : gfx.data.clipRect.y2;
#else
  int minX = 0;
  int minY = 0;
  int maxX = ((gfx.data.flags & JSGRAPHICSFLAGS_SWAP_XY) ? gfx.data.width : gfx.data.height) - 1;
  int maxY = ((gfx.data.flags & JSGRAPHICSFLAGS_SWAP_XY) ? gfx.data.height : gfx.data.width) - 1;
#endif
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
    if (font == JSGRAPHICS_FONTSIZE_VECTOR) {
#ifndef NO_VECTOR_FONT
      int w = (int)graphicsVectorCharWidth(&gfx, gfx.data.fontSize, ch);
      if (x>minX-w && x<maxX  && y>minY-gfx.data.fontSize && y<maxY)
        graphicsFillVectorChar(&gfx, x, y, gfx.data.fontSize, ch);
      x+=w;
#endif
    } else if (font == JSGRAPHICS_FONTSIZE_4X6) {
      if (x>minX-4 && x<maxX && y>minY-6 && y<maxY)
        graphicsDrawChar4x6(&gfx, x, y, ch, scale, solidBackground);
      x+=4*scale;
#ifdef USE_FONT_6X8
    } else if (font == JSGRAPHICS_FONTSIZE_6X8) {
      if (x>minX-6 && x<maxX && y>minY-8 && y<maxY)
        graphicsDrawChar6x8(&gfx, x, y, ch, scale, solidBackground);
      x+=6*scale;
#endif
    } else if (font == JSGRAPHICS_FONTSIZE_CUSTOM) {
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
      if (ch>=customFirstChar && (x>minX-width) && (x<maxX) && (y>minY-customHeight) && y<maxY) {
        int ch = customHeight/scale;
        bmpOffset *= ch;
        // now render character
        JsvStringIterator cit;
        jsvStringIteratorNew(&cit, customBitmap, (size_t)bmpOffset>>3);
        bmpOffset &= 7;
        int cx,cy;
        for (cx=0;cx<width;cx++) {
          for (cy=0;cy<ch;cy++) {
            bool set = (jsvStringIteratorGetChar(&cit)<<bmpOffset)&128;
            if (solidBackground || set)
              graphicsFillRect(&gfx,
                  (x + cx*scale),
                  (y + cy*scale),
                  (x + cx*scale + scale-1),
                  (y + cy*scale + scale-1),
                  set ? gfx.data.fgColor : gfx.data.bgColor);
            bmpOffset++;
            if (bmpOffset==8) {
              bmpOffset=0;
              jsvStringIteratorNext(&cit);
            }
          }
        }
        jsvStringIteratorFree(&cit);
      }
      x += width*scale;
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

/// Convenience function for using drawString from C code
void jswrap_graphics_drawCString(JsGraphics *gfx, int x, int y, char *str) {
  JsVar *s = jsvNewFromString(str);
  jsvUnLock2(jswrap_graphics_drawString(gfx->graphicsVar, s, x, y, false),s);
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
  JsGraphicsFontSize font = gfx.data.fontSize & JSGRAPHICS_FONTSIZE_FONT_MASK;
  unsigned short scale = gfx.data.fontSize & JSGRAPHICS_FONTSIZE_SCALE_MASK;
  if (font == JSGRAPHICS_FONTSIZE_CUSTOM) {
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
    if (font == JSGRAPHICS_FONTSIZE_VECTOR) {
#ifndef NO_VECTOR_FONT
      width += (int)graphicsVectorCharWidth(&gfx, scale, ch);
#endif
    } else if (font == JSGRAPHICS_FONTSIZE_4X6) {
      width += 4*scale;
#ifdef USE_FONT_6X8
    } else if (font == JSGRAPHICS_FONTSIZE_6X8) {
      width += 6*scale;
#endif
    } else if (font == JSGRAPHICS_FONTSIZE_CUSTOM) {
      if (jsvIsString(customWidth)) {
        if (ch>=customFirstChar)
          width += scale*(unsigned char)jsvGetCharInString(customWidth, (size_t)(ch-customFirstChar));
      } else
        width += scale*(int)jsvGetInteger(customWidth);
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
  graphicsDrawLine(&gfx, x1,y1,x2,y2);
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
  graphicsDrawLine(&gfx, gfx.data.cursorX, gfx.data.cursorY, x, y);
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
        graphicsDrawLine(&gfx, gfx.data.cursorX, gfx.data.cursorY, x, y);
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
    graphicsDrawLine(&gfx, gfx.data.cursorX, gfx.data.cursorY, startx, starty);

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
    ["image","JsVar","An image to draw, either a String or an Object (see below)"],
    ["x","int32","The X offset to draw the image"],
    ["y","int32","The Y offset to draw the image"],
    ["options","JsVar","options for scaling,rotation,etc (see below)"]
  ],
  "return" : ["JsVar","The instance of Graphics this was called on, to allow call chaining"],
  "return_object" : "Graphics"
}
Image can be:

* An object with the following fields `{ width : int, height : int, bpp : optional int, buffer : ArrayBuffer/String, transparent: optional int, palette : optional Uint16Array(2/4/16) }`. bpp = bits per pixel (default is 1), transparent (if defined) is the colour that will be treated as transparent, and palette is a color palette that each pixel will be looked up in first
* A String where the the first few bytes are: `width,height,bpp,[transparent,]image_bytes...`. If a transparent colour is specified the top bit of `bpp` should be set.

Draw an image at the specified position.

* If the image is 1 bit, the graphics foreground/background colours will be used.
* If `img.palette` is a Uint16Array or 2/4/16 elements, color data will be looked from the supplied palette
* On Bangle.js, 2 bit images blend from background(0) to foreground(1) colours
* On Bangle.js, 4 bit images use the Apple Mac 16 color palette
* On Bangle.js, 8 bit images use the Web Safe 216 color palette
* Otherwise color data will be copied as-is. Bitmaps are rendered MSB-first

If `options` is supplied, `drawImage` will allow images to be rendered at any scale or angle. If `options.rotate` is set it will
center images at `x,y`. `options` must be an object of the form:

```
{
  rotate : float, // the amount to rotate the image in radians (default 0)
  scale : float, // the amount to scale the image up (default 1)
}
```

For example:

```
// In the top left of the screen
g.drawImage(img,0,0);
// In the top left of the screen, twice as big
g.drawImage(img,0,0,{scale:2});
// In the center of the screen, twice as big, 45 degrees
g.drawImage(img, g.getWidth()/2, g.getHeight()/2,
            {scale:2, rotate:Math.PI/4});
```
*/
JsVar *jswrap_graphics_drawImage(JsVar *parent, JsVar *image, int xPos, int yPos, JsVar *options) {
  JsGraphics gfx; if (!graphicsGetFromVar(&gfx, parent)) return 0;

  int imageWidth, imageHeight, imageBpp;
  bool imageIsTransparent = false;
  unsigned int imageTransparentCol;
  JsVar *imageBuffer;
  int imageBufferOffset;
  const uint16_t *palettePtr = 0;
  uint32_t paletteMask = 0;
  uint16_t simplePalette[4];

  if (jsvIsObject(image)) {
    imageWidth = (int)jsvGetIntegerAndUnLock(jsvObjectGetChild(image, "width", 0));
    imageHeight = (int)jsvGetIntegerAndUnLock(jsvObjectGetChild(image, "height", 0));
    imageBpp = (int)jsvGetIntegerAndUnLock(jsvObjectGetChild(image, "bpp", 0));
    if (imageBpp<=0) imageBpp=1;
    JsVar *v;
    v = jsvObjectGetChild(image, "transparent", 0);
    imageIsTransparent = v!=0;
    imageTransparentCol = (unsigned int)jsvGetIntegerAndUnLock(v);
    v = jsvObjectGetChild(image, "palette", 0);
    if (v) {
      if (jsvIsArrayBuffer(v) && v->varData.arraybuffer.type==ARRAYBUFFERVIEW_UINT16) {
        size_t l = 0;
        palettePtr = (uint16_t *)jsvGetDataPointer(v, &l);
        jsvUnLock(v);
        if (l==2 || l==4 || l==16 || l==256)
          paletteMask = (uint32_t)(l-1);
        else {
          palettePtr = 0;
        }
      } else
        jsvUnLock(v);
      if (!palettePtr) {
        jsExceptionHere(JSET_ERROR, "palette specified, but must be a flat Uint16Array of 2,4,16,256 elements");
        return 0;
      }
    }

    imageBuffer = jsvObjectGetChild(image, "buffer", 0);
    imageBufferOffset = 0;
  } else if (jsvIsString(image) || jsvIsArrayBuffer(image)) {
    if (jsvIsArrayBuffer(image)) {
      imageBuffer = jsvGetArrayBufferBackingString(image);
    } else {
      imageBuffer = jsvLockAgain(image);
    }
    if (!jsvIsString(imageBuffer)) {
      jsvUnLock(imageBuffer);
      return 0;
    }
    imageWidth = (unsigned char)jsvGetCharInString(imageBuffer,0);
    imageHeight = (unsigned char)jsvGetCharInString(imageBuffer,1);
    imageBpp = (unsigned char)jsvGetCharInString(imageBuffer,2);
    if (imageBpp & 128) {
      imageBpp = imageBpp&127;
      imageIsTransparent = true;
      imageTransparentCol = (unsigned char)jsvGetCharInString(imageBuffer,3);
      imageBufferOffset = 4;
    } else {
      imageBufferOffset = 3;
    }
  } else {
    jsExceptionHere(JSET_ERROR, "Expecting first argument to be an object or a String");
    return 0;
  }
  if (!imageIsTransparent)
    imageTransparentCol = 0xFFFFFFFF;

  if (palettePtr==0) {
    if (imageBpp==1) {
      simplePalette[0] = (uint16_t)gfx.data.bgColor;
      simplePalette[1] = (uint16_t)gfx.data.fgColor;
      palettePtr = simplePalette;
      paletteMask = 1;
  #ifdef GRAPHICS_PALETTED_IMAGES
    } else if (gfx.data.bpp==16 && imageBpp==2) { // Blend from bg to fg
      unsigned int b = gfx.data.bgColor;
      unsigned int br = (b>>8)&0xF8;
      unsigned int bg = (b>>3)&0xFC;
      unsigned int bb = (b<<3)&0xF8;
      unsigned int f = gfx.data.fgColor;
      unsigned int fr = (f>>8)&0xF8;
      unsigned int fg = (f>>3)&0xFC;
      unsigned int fb = (f<<3)&0xF8;
      simplePalette[0] = (uint16_t)gfx.data.bgColor;
      unsigned int ri,gi,bi;
      ri = (br*2 + fr)/3;
      gi = (bg*2 + fg)/3;
      bi = (bb*2 + fb)/3;
      simplePalette[1] = (uint16_t)((bi>>3) | (gi>>2)<<5 | (ri>>3)<<11);
      ri = (br + fr*2)/3;
      gi = (bg + fg*2)/3;
      bi = (bb + fb*2)/3;
      simplePalette[2] = (uint16_t)((bi>>3) | (gi>>2)<<5 | (ri>>3)<<11);
      simplePalette[3] = (uint16_t)gfx.data.fgColor;
      palettePtr = simplePalette;
      paletteMask = 3;
    } else if (gfx.data.bpp==16 && imageBpp==4) { // palette is 16 bits, so don't use it for other things
      palettePtr = PALETTE_4BIT;
      paletteMask = 15;
    } else if (gfx.data.bpp==16 && imageBpp==8) { // palette is 16 bits, so don't use it for other things
      palettePtr = PALETTE_8BIT;
      paletteMask = 255;
    } else if (gfx.data.bpp==8 && imageBpp==4) {
      palettePtr = PALETTE_4BIT_TO_8BIT;
      paletteMask = 15;
  #endif
    }
  }

  if (!(jsvIsArrayBuffer(imageBuffer) || jsvIsString(imageBuffer)) ||
      imageWidth<=0 ||
      imageHeight<=0 ||
      imageBpp>32) {
    jsExceptionHere(JSET_ERROR, "Expecting first argument to a valid Image");
    jsvUnLock(imageBuffer);
    return 0;
  }
  unsigned int imageBitMask = (unsigned int)((1L<<imageBpp)-1L);
  unsigned int imagePixelsPerByteMask = (unsigned int)((imageBpp<8)?(8/imageBpp)-1:0);
  // jsvGetArrayBufferBackingString is fine to be passed a string
  JsVar *imageBufferString = jsvGetArrayBufferBackingString(imageBuffer);
  jsvUnLock(imageBuffer);

  int x=0, y=0;
  int bits=0;
  unsigned int colData = 0;
  JsvStringIterator it;
  jsvStringIteratorNew(&it, imageBufferString, (size_t)imageBufferOffset);

  if (jsvIsUndefined(options)) {
    // Standard 1:1 blitting
#ifdef GRAPHICS_FAST_PATHS
    bool fastPath =
        (gfx.data.flags & (JSGRAPHICSFLAGS_SWAP_XY|JSGRAPHICSFLAGS_INVERT_X|JSGRAPHICSFLAGS_INVERT_Y))==0; // no messing with coordinates
#ifdef USE_LCD_ST7789_8BIT // can we blit directly to the display?
    uint8_t *pixelPtr;
    size_t pixelLen;
    if (fastPath &&
        gfx.data.type==JSGRAPHICSTYPE_ST7789_8BIT &&
        gfx.data.bpp==16 &&
        (imageBpp==8 || imageBpp==1) &&
        !imageIsTransparent &&
        xPos>=0 && yPos>=0 && // check it's all on-screen
        (xPos+imageWidth)<=LCD_WIDTH && (yPos+imageHeight)<=LCD_HEIGHT &&
        (pixelPtr=(uint8_t*)jsvGetDataPointer(imageBufferString,&pixelLen)) &&
        (int)(pixelLen-imageBufferOffset)*8 >= imageWidth*imageHeight*imageBpp) {
      if (imageBpp==1) lcdST7789_blit1Bit(xPos, yPos, imageWidth, imageHeight, 1, &pixelPtr[imageBufferOffset], palettePtr);
      else if (imageBpp==8) lcdST7789_blit8Bit(xPos, yPos, imageWidth, imageHeight, 1, &pixelPtr[imageBufferOffset], palettePtr);
    } else
#endif
    if (fastPath) { // fast path for standard blit
      int yp = yPos;
      for (y=0;y<imageHeight;y++) {
        int xp = xPos;
        for (x=0;x<imageWidth;x++) {
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
          if (imageTransparentCol!=col) {
            if (palettePtr) col = palettePtr[col&paletteMask];
            if (xp>=gfx.data.clipRect.x1 && xp<=gfx.data.clipRect.x2 &&
                yp>=gfx.data.clipRect.y1 && yp<=gfx.data.clipRect.y2)
              gfx.setPixel(&gfx, xp, yp, col);
          }
          xp++;
        }
        yp++;
      }
      // update modified area since we went direct
      int x1=xPos, y1=yPos, x2=xPos+imageWidth, y2=yPos+imageHeight;
      if (x1<gfx.data.clipRect.x1) x1 = gfx.data.clipRect.x1;
      if (y1<gfx.data.clipRect.y1) y1 = gfx.data.clipRect.y1;
      if (x2>gfx.data.clipRect.x2) x2 = gfx.data.clipRect.x2;
      if (y2>gfx.data.clipRect.y2) y2 = gfx.data.clipRect.y2;
      if (x1 < gfx.data.modMinX) gfx.data.modMinX=(short)x1;
      if (x2 > gfx.data.modMaxX) gfx.data.modMaxX=(short)x2;
      if (y1 < gfx.data.modMinY) gfx.data.modMinY=(short)y1;
      if (y2 > gfx.data.modMaxY) gfx.data.modMaxY=(short)y2;
    } else { // handle rotation, and default to center the image
#else
    if (true) {
#endif
      for (y=0;y<imageHeight;y++) {
        for (x=0;x<imageWidth;x++) {
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
          if (imageTransparentCol!=col) {
            if (palettePtr) col = palettePtr[col&paletteMask];
            graphicsSetPixel(&gfx, x+xPos, y+yPos, col);
          }
        }
      }
    }
  } else if (jsvIsObject(options)) {
#ifndef GRAPHICS_DRAWIMAGE_ROTATED
    jsExceptionHere(JSET_ERROR,"Image rotation not implemented on this device");
#else
    // fancy rotation/scaling
    int imageStride = (imageWidth*imageBpp + 7)>>3;
    // rotate, scale
    double scale = jsvGetFloatAndUnLock(jsvObjectGetChild(options,"scale",0));
    if (!isfinite(scale) || scale<=0) scale=1;
    double rotate = jsvGetFloatAndUnLock(jsvObjectGetChild(options,"rotate",0));
    bool rotateIsSet = isfinite(rotate);
    if (!rotateIsSet) rotate = 0;
#ifdef GRAPHICS_FAST_PATHS
    bool fastPath =
        (!rotateIsSet) &&  // not rotating
        (scale-floor(scale))==0 && // integer scale
        (gfx.data.flags & (JSGRAPHICSFLAGS_SWAP_XY|JSGRAPHICSFLAGS_INVERT_X|JSGRAPHICSFLAGS_INVERT_Y))==0; // no messing with coordinates
    if (fastPath) { // fast path for non-rotated, integer scale
      int s = (int)scale;
      // Scaled blitting
      /* Output as scanlines rather than small fillrects so
       * that on direct-coupled displays we can optimise away
       * coordinate setting
       */
#ifdef USE_LCD_ST7789_8BIT // can we blit directly to the display?
      uint8_t *pixelPtr;
      size_t pixelLen;
      if (gfx.data.type==JSGRAPHICSTYPE_ST7789_8BIT &&
          gfx.data.bpp==16 &&
          s>=1 &&
          (imageBpp==8 || imageBpp==1) &&
          !imageIsTransparent &&
          xPos>=0 && yPos>=0 && // check it's all on-screen
          (xPos+imageWidth*s)<=LCD_WIDTH && (yPos+imageHeight*s)<=LCD_HEIGHT &&
          (pixelPtr=(uint8_t*)jsvGetDataPointer(imageBufferString,&pixelLen)) &&
          (int)(pixelLen-imageBufferOffset)*8 >= imageWidth*imageHeight*imageBpp) {
        if (imageBpp==1) lcdST7789_blit1Bit(xPos, yPos, imageWidth, imageHeight, s, &pixelPtr[imageBufferOffset], palettePtr);
        else lcdST7789_blit8Bit(xPos, yPos, imageWidth, imageHeight, s, &pixelPtr[imageBufferOffset], palettePtr);
      } else
#endif
      {
        int yp = yPos;
        for (y=0;y<imageHeight;y++) {
          // Store current pos as we need to rewind
          size_t lastIt = jsvStringIteratorGetIndex(&it);
          int lastBits = bits;
          unsigned int lastColData = colData;
          // do a new iteration for each line we're scaling
          for (int iy=0;iy<s;iy++) {
            if (iy) { // rewind for all but the first line of scaling
              jsvStringIteratorGoto(&it, imageBufferString, lastIt);
              bits = lastBits;
              colData = lastColData;
            }
            // iterate over x
            int xp = xPos;
            for (x=0;x<imageWidth;x++) {
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
              if (imageTransparentCol!=col && yp>=gfx.data.clipRect.y1 && yp<=gfx.data.clipRect.y2) {
                if (palettePtr) col = palettePtr[col&paletteMask];
                for (int ix=0;ix<s;ix++) {
                  if (xp>=gfx.data.clipRect.x1 && xp<=gfx.data.clipRect.x2)
                    gfx.setPixel(&gfx, xp, yp, col);
                  xp++;
                }
              } else xp += s;
            }
            yp++;
          }
        }
        // update modified area since we went direct
        int x1=xPos, y1=yPos, x2=xPos+s*imageWidth, y2=yPos+s*imageHeight;
        if (x1<gfx.data.clipRect.x1) x1 = gfx.data.clipRect.x1;
        if (y1<gfx.data.clipRect.y1) y1 = gfx.data.clipRect.y1;
        if (x2>gfx.data.clipRect.x2) x2 = gfx.data.clipRect.x2;
        if (y2>gfx.data.clipRect.y2) y2 = gfx.data.clipRect.y2;
        if (x1 < gfx.data.modMinX) gfx.data.modMinX=(short)x1;
        if (x2 > gfx.data.modMaxX) gfx.data.modMaxX=(short)x2;
        if (y1 < gfx.data.modMinY) gfx.data.modMinY=(short)y1;
        if (y2 > gfx.data.modMaxY) gfx.data.modMaxY=(short)y2;
      }
    } else { // handle rotation, and default to center the image
#else
    if (true) {
#endif
      int centerx = imageWidth*128;
      int centery = imageHeight*128;
      // step values for blitting rotated image
      double vcos = cos(rotate);
      double vsin = sin(rotate);
      int sx = (int)((vcos/scale)*256 + 0.5);
      int sy = (int)((vsin/scale)*256 + 0.5);
      // work out actual image width and height
      int iw = (int)(0.5 + scale*(imageWidth*fabs(vcos) + imageHeight*fabs(vsin)));
      int ih = (int)(0.5 + scale*(imageWidth*fabs(vsin) + imageHeight*fabs(vcos)));
      // if rotating, offset our start position from center
      if (rotateIsSet) {
        xPos -= iw/2;
        yPos -= ih/2;
      }
      // work out start position in the image
      int px = centerx - (1 + (sx*iw) + (sy*ih)) / 2;
      int py = centery - (1 + (sx*ih) - (sy*iw)) / 2;
      // scan across image
      for (y=0;y<ih;y++) {
        int qx = px;
        int qy = py;
        for (x=0;x<iw;x++) {
          int imagex = (qx+127)>>8;
          int imagey = (qy+127)>>8;
          if (imagex>=0 && imagey>=0 && imagex<imageWidth && imagey<imageHeight) {
            if (imageBpp==8) { // fast path for 8 bits
              jsvStringIteratorGoto(&it, imageBufferString, (size_t)(imageBufferOffset+imagex+(imagey*imageStride)));
              colData = (unsigned char)jsvStringIteratorGetChar(&it);
            } else {
              int bitOffset = (imagex+(imagey*imageWidth))*imageBpp;
              jsvStringIteratorGoto(&it, imageBufferString, (size_t)(imageBufferOffset+(bitOffset>>3)));
              colData = (unsigned char)jsvStringIteratorGetChar(&it);
              for (int b=8;b<imageBpp;b+=8) {
                jsvStringIteratorNext(&it);
                colData = (colData<<8) | (unsigned char)jsvStringIteratorGetChar(&it);
              }
              //jsiConsolePrintf("%d %d %d\n", bitOffset, imagePixelsPerByteMask, (imagePixelsPerByteMask-(bitOffset&imagePixelsPerByteMask))*imageBpp);
              colData = (colData>>((imagePixelsPerByteMask-(bitOffset&imagePixelsPerByteMask))*imageBpp)) & imageBitMask;
            }
            if (imageTransparentCol!=colData) {
              if (palettePtr) colData = palettePtr[colData&paletteMask];
              graphicsSetPixel(&gfx, x+xPos, y+yPos, colData);
            }
          }
          qx += sx;
          qy -= sy;
        }
        px += sy;
        py += sx;
      }
    }
#endif
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
    pixelBits = (pixelBits<<bpp) | graphicsGetPixel(&gfx, x, y);
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
  "#if" : "!defined(SAVE_ON_FLASH) && !defined(ESPRUINOBOARD)",
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
          b = (b<<1)|(graphicsGetPixel(&gfx, x++, y)&1);
        }
        imgPtr[headerLen + (yi*rowstride) + (x>>3) - 1] = (unsigned char)b;
      }
    } else {
      for (int x=0;x<width;x++) {
        unsigned int c = graphicsGetPixel(&gfx, x, y);
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

/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "quadraticBezier",
  "#if" : "!defined(SAVE_ON_FLASH) && !defined(ESPRUINOBOARD) && !defined(BANGLEJS)",
  "generate" : "jswrap_graphics_quadraticBezier",
  "params" : [
    ["arr","JsVar","An array of three vertices, six enties in form of ```[x0,y0,x1,y1,x2,y2]```"],
    ["options","JsVar","number of points to calulate"]
  ],
  "return" : ["JsVar", "Array with calculated points" ]
}
 Calculate the square area under a Bezier curve.

 x0,y0: start point
 x1,y1: control point
 y2,y2: end point

 Max 10 points without start point.
*/
JsVar *jswrap_graphics_quadraticBezier( JsVar *parent, JsVar *arr, JsVar *options ){
  NOT_USED(parent);
  JsVar *result = jsvNewEmptyArray();
  if (!result) return 0;

  if (jsvGetArrayLength(arr) != 6) return result; 

  double s,t,t2,tp2, tpt;
  int sn = 5;
  int dx, dy;
  int x0, x1, x2, y0, y1, y2;
  int count = 0;

  JsvIterator it;
  jsvIteratorNew(&it, arr, JSIF_EVERY_ARRAY_ELEMENT);
  x0 = jsvIteratorGetIntegerValue(&it); jsvIteratorNext(&it);
  y0 = jsvIteratorGetIntegerValue(&it); jsvIteratorNext(&it);
  x1 = jsvIteratorGetIntegerValue(&it); jsvIteratorNext(&it);
  y1 = jsvIteratorGetIntegerValue(&it); jsvIteratorNext(&it);
  x2 = jsvIteratorGetIntegerValue(&it); jsvIteratorNext(&it);
  y2 = jsvIteratorGetIntegerValue(&it); jsvIteratorFree(&it);

  if (jsvIsObject(options)) count = jsvGetIntegerAndUnLock(jsvObjectGetChild(options,"count",0));

  dx = (x0 - x2) < 0 ? (x2-x0):(x0-x2);
  dy = (y0 - y2) < 0 ? (y2-y0):(y0-y2); 
  s =  1 / (double) (((dx < dy) ? dx : dy ) / sn );
  if ( s >= 1)  s = 0.33;
  if ( s < 0.1) s = 0.1;
  if (count > 0) s = 1.0 / count;

  jsvArrayPushAndUnLock(result, jsvNewFromInteger(x0));
  jsvArrayPushAndUnLock(result, jsvNewFromInteger(y0));

  for ( t = s; t <= 1; t += s ) {
    t2 = t*t;
    tp2 = (1 - t) * (1 - t);
    tpt = 2 * (1 - t) * t;
    jsvArrayPushAndUnLock(result, jsvNewFromInteger((int)(x0 * tp2 + x1 * tpt + x2 * t2 + 0.5)));
    jsvArrayPushAndUnLock(result, jsvNewFromInteger((int)(y0 * tp2 + y1 * tpt + y2 * t2 + 0.5)));
  }

  jsvArrayPushAndUnLock(result, jsvNewFromInteger(x2));
  jsvArrayPushAndUnLock(result, jsvNewFromInteger(y2));

  return  result;
}
