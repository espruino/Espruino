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
 * Contains JavaScript interface for Pixl.js (http://www.espruino.com/Pixl.js)
 * ----------------------------------------------------------------------------
 */

#include <jswrap_pixljs.h>
#include "jsinteractive.h"
#include "jsdevices.h"
#include "jsnative.h"
#include "jshardware.h"
#include "jsdevices.h"
#include "jspin.h"
#include "jstimer.h"
#include "jswrap_bluetooth.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "nrf5x_utils.h"
#include "jsflash.h" // for jsfRemoveCodeFromFlash
#include "bluetooth.h" // for self-test

#include "jswrap_graphics.h"
#include "lcd_arraybuffer.h"

const Pin PIXL_IO_PINS[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19};

/*JSON{
    "type": "class",
    "class" : "Pixl"
}
Class containing utility functions for [Pixl.js](http://www.espruino.com/Pixl.js)
*/

/*JSON{
    "type" : "staticmethod",
    "class" : "Pixl",
    "name" : "getBatteryPercentage",
    "generate" : "jswrap_espruino_getBattery",
    "return" : ["int", "A percentage between 0 and 100" ]
}
DEPRECATED - Please use `E.getBattery()` instead.

Return an approximate battery percentage remaining based on
a normal CR2032 battery (2.8 - 2.2v)
*/

/*JSON{
    "type" : "staticmethod",
    "class" : "Pixl",
    "name" : "menu",
    "generate" : "jswrap_pixljs_menu",
    "params" : [
      ["menu","JsVar","An object containing name->function mappings to to be used in a menu"]
    ],
    "return" : ["JsVar", "A menu object with `draw`, `move` and `select` functions" ]
}
Display a menu on Pixl.js's screen, and set up the buttons to navigate through it.

Supply an object containing menu items. When an item is selected, the
function it references will be executed. For example:

```
// First menu
var mainmenu = {
  "" : {
    "title" : "-- Main Menu --"
  },
  "Backlight On" : function() { LED1.set(); },
  "Backlight Off" : function() { LED1.reset(); },
  "Submenu" : function() { Pixl.menu(submenu); },
  "Exit" : function() { Pixl.menu(); },
};

// Submenu
var submenu = {
  "" : {
    "title" : "-- SubMenu --"
  },
  "One" : undefined, // do nothing
  "Two" : undefined, // do nothing
  "< Back" : function() { Pixl.menu(mainmenu); },
};

Pixl.menu(mainmenu);
```

See http://www.espruino.com/graphical_menu for more detailed information.
*/
JsVar *jswrap_pixljs_menu(JsVar *menu) {
  /* Unminified JS code is:

Pixl.show = function(menudata) {
  if (Pixl.btnWatches) {
    Pixl.btnWatches.forEach(clearWatch);
    Pixl.btnWatches = undefined;
  }
  g.clear();g.flip(); // clear screen if no menu supplied
  if (!menudata) return;
  function im(b) {
    return {
      width:8,height:b.length,bpp:1,buffer:new Uint8Array(b).buffer
    };
  }
  if (!menudata[""]) menudata[""]={};
  g.setFontBitmap();
  var w = g.getWidth()-9;
  var h = g.getHeight();
  menudata[""].x=9;
  menudata[""].x2=w-2;
  menudata[""].preflip=function() {
    g.drawImage(im([
      0b00010000,
      0b00111000,
      0b01111100,
      0b11111110,
      0b00010000,
      0b00010000,
      0b00010000,
      0b00010000,
    ]),0,4);
    g.drawImage(im([
      0b00010000,
      0b00010000,
      0b00010000,
      0b00010000,
      0b11111111,
      0b01111100,
      0b00111000,
      0b00010000,
    ]),0,h-12);
    g.drawImage(im([
      0b00000000,
      0b00001000,
      0b00001100,
      0b00001110,
      0b11111111,
      0b00001110,
      0b00001100,
      0b00001000,
    ]),w+1,h-12);
    //g.drawLine(7,0,7,h);
    //g.drawLine(w,0,w,h);
  };
  var m = require("graphical_menu").list(g, menudata);
  Pixl.btnWatches = [
    setWatch(function() { m.move(-1); }, BTN1, {repeat:1}),
    setWatch(function() { m.move(1); }, BTN4, {repeat:1}),
    setWatch(function() { m.select(); }, BTN3, {repeat:1})
  ];
  return m;
};
*/

  /* TODO: handle this better. Something in build_js_wrapper.py?
   * We want the function to be defined in the docs so using JSMODULESOURCES
   * isn't a great idea - ideally there would be some nice way of including it
   * here.
   */
  JsVar *fn = jspEvaluate("(function(a){function c(a){return{width:8,height:a.length,bpp:1,buffer:(new Uint8Array(a)).buffer}}Pixl.btnWatches&&(Pixl.btnWatches.forEach(clearWatch),Pixl.btnWatches=void 0);"
      "g.clear();g.flip();"
      "if(a){a['']||(a['']={});g.setFontBitmap();var d=g.getWidth()-9,e=g.getHeight();a[''].x=9;a[''].x2=d-2;a[''].preflip=function(){"
      "g.drawImage(c([16,56,124,254,16,16,16,16]),0,4);g.drawImage(c([16,16,16,16,255,124,56,16]),0,e-12);g.drawImage(c([0,8,12,14,255,14,12,8]),d+1,e-12)};"
      "var b=require('graphical_menu').list(g,a);Pixl.btnWatches=[setWatch(function(){b.move(-1)},BTN1,{repeat:1}),setWatch(function(){b.move(1)},BTN4,{repeat:1}),"
      "setWatch(function(){b.select()},BTN3,{repeat:1})];return b}})",true);
  JsVar *result = jspExecuteFunction(fn,0,1,&menu);
  jsvUnLock(fn);
  return result;
}


/*JSON{
  "type" : "variable",
  "name" : "SDA",
  "generate_full" : "4",
  "ifdef" : "PIXLJS",
  "return" : ["pin",""]
}
The pin marked SDA on the Arduino pin footprint. This is connected directly to pin A4.
*/
/*JSON{
  "type" : "variable",
  "name" : "SCL",
  "generate_full" : "5",
  "ifdef" : "PIXLJS",
  "return" : ["pin",""]
}
The pin marked SDA on the Arduino pin footprint. This is connected directly to pin A5.
*/


void lcd_wr(int data) {
  int bit;
  for (bit=7;bit>=0;bit--) {
    // TODO: we could push this faster by accessing IO directly
    jshPinSetValue(LCD_SPI_MOSI, (data>>bit)&1 );
    jshPinSetValue(LCD_SPI_SCK, 1 );
    jshPinSetValue(LCD_SPI_SCK, 0 );
  }
}

void lcd_flip_gfx(JsGraphics *gfx) {
  if (gfx->data.modMinX > gfx->data.modMaxX) return; // nothing to do!

  JsVar *buf = jsvObjectGetChild(gfx->graphicsVar,"buffer",0);
  if (!buf) return;
  JSV_GET_AS_CHAR_ARRAY(bPtr, bLen, buf);
  if (!bPtr || bLen<128*8) return;

  int xcoord = gfx->data.modMinX&~7;
  int xlen = gfx->data.modMaxX+1-xcoord;

  jshPinSetValue(LCD_SPI_CS,0);
  for (int y=0;y<8;y++) {
    // skip any lines that don't need updating
    int ycoord = y*8;
    if (ycoord > gfx->data.modMaxY ||
        ycoord+7 < gfx->data.modMinY) continue;
    // Send only what we need
    jshPinSetValue(LCD_SPI_DC,0);
    lcd_wr(0xB0|y/* page */);
    lcd_wr(0x00|(xcoord&15)/* x lower*/);
    lcd_wr(0x10|(xcoord>>4)/* x upper*/);
    jshPinSetValue(LCD_SPI_DC,1);

    char *px = &bPtr[y*128 + (xcoord>>3)];
    for (int x=0;x<xlen;x++) {
      int bit = 128>>(x&7);
      lcd_wr(
          ((px[  0]&bit)?1:0) |
          ((px[ 16]&bit)?2:0) |
          ((px[ 32]&bit)?4:0) |
          ((px[ 48]&bit)?8:0) |
          ((px[ 64]&bit)?16:0) |
          ((px[ 80]&bit)?32:0) |
          ((px[ 96]&bit)?64:0) |
          ((px[112]&bit)?128:0));
      if ((x&7) == 7) px++;
    }
  }
  jshPinSetValue(LCD_SPI_CS,1);
  jsvUnLock(buf);
  // Reset modified-ness
  gfx->data.modMaxX = -32768;
  gfx->data.modMaxY = -32768;
  gfx->data.modMinX = 32767;
  gfx->data.modMinY = 32767;
}


/// Send buffer contents to the screen. Usually only the modified data will be output, but if all=true then the whole screen contents is sent
void lcd_flip(JsVar *parent, bool all) {
  JsGraphics gfx; 
  if (!graphicsGetFromVar(&gfx, parent)) return;
  if (all) {
    gfx.data.modMinX = 0;
    gfx.data.modMinY = 0;
    gfx.data.modMaxX = 127;
    gfx.data.modMaxY = 63;
  }
  lcd_flip_gfx(&gfx);
  graphicsSetVar(&gfx);
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Pixl",
    "name" : "setContrast",
    "generate" : "jswrap_pixljs_setContrast",
    "params" : [
      ["c","float","Contrast between 0 and 1"]
    ]
}
Set the LCD's contrast
*/
void jswrap_pixljs_setContrast(JsVarFloat c) {
  if (c<0) c=0;
  if (c>1) c=1;
  jshPinSetValue(LCD_SPI_CS,0);
  jshPinSetValue(LCD_SPI_DC,0);
  lcd_wr(0x81);
  lcd_wr((int)(63*c));
  //lcd_wr(0x20|div); div = 0..7
  jshPinSetValue(LCD_SPI_CS,1);
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Pixl",
    "name" : "lcdw",
    "generate" : "jswrap_pixljs_lcdw",
    "params" : [
      ["c","int",""]
    ]
}
Writes a command directly to the ST7567 LCD controller
*/
void jswrap_pixljs_lcdw(JsVarInt c) {
  jshPinSetValue(LCD_SPI_CS,0);
  jshPinSetValue(LCD_SPI_DC,0);
  lcd_wr(c);
  jshPinSetValue(LCD_SPI_CS,1);
}

static bool selftest_check_pin(Pin pin) {
  unsigned int i;
  bool ok = true;
  jshPinSetState(pin, JSHPINSTATE_GPIO_OUT);
  jshPinSetValue(pin, 1);
  jshPinSetState(pin, JSHPINSTATE_GPIO_IN_PULLUP);
  if (!jshPinGetValue(pin)) {
    jsiConsolePrintf("Pin %p forced low\n", pin);
    ok = false;
  }
  for (i=0;i<sizeof(PIXL_IO_PINS)/sizeof(Pin);i++)
    if (PIXL_IO_PINS[i]!=pin)
      jshPinOutput(PIXL_IO_PINS[i], 0);
  if (!jshPinGetValue(pin)) {
    jsiConsolePrintf("Pin %p shorted low\n", pin);
    ok = false;
  }

  jshPinSetState(pin, JSHPINSTATE_GPIO_OUT);
  jshPinSetValue(pin, 0);
  jshPinSetState(pin, JSHPINSTATE_GPIO_IN_PULLDOWN);
  if (jshPinGetValue(pin)) {
    jsiConsolePrintf("Pin %p forced high\n", pin);
    ok = false;
  }
  for (i=0;i<sizeof(PIXL_IO_PINS)/sizeof(Pin);i++)
     if (PIXL_IO_PINS[i]!=pin)
       jshPinOutput(PIXL_IO_PINS[i], 1);
   if (jshPinGetValue(pin)) {
     jsiConsolePrintf("Pin %p shorted high\n", pin);
     ok = false;
   }
  jshPinSetState(pin, JSHPINSTATE_GPIO_IN);
  return ok;
}

static bool pixl_selfTest() {
  unsigned int timeout, i;
  JsVarFloat v;
  bool ok = true;

  // light up all LEDs white
  jshPinOutput(LED1_PININDEX, LED1_ONSTATE);
  jshPinSetState(BTN1_PININDEX, BTN1_PINSTATE);

  if (jshPinGetValue(BTN1_PININDEX)==BTN1_ONSTATE)
    jsiConsolePrintf("Release BTN1\n");
  if (jshPinGetValue(BTN4_PININDEX)==BTN4_ONSTATE)
    jsiConsolePrintf("Release BTN4\n");
  timeout = 2000;
  while ((jshPinGetValue(BTN1_PININDEX)==BTN1_ONSTATE ||
          jshPinGetValue(BTN4_PININDEX)==BTN4_ONSTATE) && timeout--)
    nrf_delay_ms(1);
  if (jshPinGetValue(BTN1_PININDEX)==BTN1_ONSTATE) {
    jsiConsolePrintf("BTN1 stuck down\n");
    ok = false;
  }
  if (jshPinGetValue(BTN2_PININDEX)==BTN1_ONSTATE) {
    jsiConsolePrintf("BTN2 stuck down\n");
    ok = false;
  }
  if (jshPinGetValue(BTN3_PININDEX)==BTN1_ONSTATE) {
    jsiConsolePrintf("BTN3 stuck down\n");
    ok = false;
  }
  if (jshPinGetValue(BTN4_PININDEX)==BTN1_ONSTATE) {
    jsiConsolePrintf("BTN4 stuck down\n");
    ok = false;
  }
  nrf_delay_ms(100);
  jshPinInput(LED1_PININDEX);
  nrf_delay_ms(500);

  jshPinSetState(LED1_PININDEX, JSHPINSTATE_GPIO_IN_PULLUP);
  nrf_delay_ms(1);
  v = jshPinAnalog(LED1_PININDEX);
  jshPinSetState(LED1_PININDEX, JSHPINSTATE_GPIO_IN);
  if (v<0.3 || v>0.65) {
    jsiConsolePrintf("Backlight OOR (%f)\n", v);
    ok = false;
  }

  for (i=0;i<sizeof(PIXL_IO_PINS)/sizeof(Pin);i++) {
    if (!selftest_check_pin(PIXL_IO_PINS[i])) ok = false;
  }

  for (i=0;i<sizeof(PIXL_IO_PINS)/sizeof(Pin);i++)
    jshPinSetState(PIXL_IO_PINS[i], JSHPINSTATE_GPIO_IN);

  if (jshHasEvents()) {
    jsiConsolePrintf("Have events - no BLE test\n");
  } else {
    bool bleWorking = false;
    uint32_t err_code;
    err_code = jsble_set_scanning(true);
    jsble_check_error(err_code);
    int timeout = 20;
    while (timeout-- && !jshHasEvents()) {
      nrf_delay_ms(100);
    }
    err_code = jsble_set_scanning(false);
    jsble_check_error(err_code);
    if (!jshHasEvents()) {
      jsiConsolePrintf("No BLE adverts found in 2s\n");
      ok = false;
    }
  }

  return ok;
}

/*JSON{
  "type" : "init",
  "generate" : "jswrap_pixljs_init"
}*/
void jswrap_pixljs_init() {
  // LCD Init 1
  jshPinOutput(LCD_SPI_CS,0);
  jshPinOutput(LCD_SPI_DC,0);
  jshPinOutput(LCD_SPI_SCK,0);
  jshPinOutput(LCD_SPI_MOSI,0);
  jshPinOutput(LCD_SPI_RST,0);
  // Create backing graphics for LCD
  JsVar *graphics = jspNewObject(0, "Graphics");
  if (!graphics) return; // low memory
  JsGraphics gfx;
  graphicsStructInit(&gfx);
  gfx.data.type = JSGRAPHICSTYPE_ARRAYBUFFER;
  gfx.data.flags = JSGRAPHICSFLAGS_ARRAYBUFFER_MSB;
  gfx.graphicsVar = graphics;
  gfx.data.width = 128;
  gfx.data.height = 64;
  gfx.data.bpp = 1;
  lcdInit_ArrayBuffer(&gfx);
  graphicsSetVar(&gfx);
  jsvObjectSetChild(execInfo.root, "g", graphics);
  jsvObjectSetChild(execInfo.hiddenRoot, JS_GRAPHICS_VAR, graphics);
  graphicsGetFromVar(&gfx, graphics);
  // Set initial image
  const unsigned char PIXLJS_IMG[] = {
        251, 239, 135, 192, 0, 0, 31, 0, 0, 0, 125, 247, 195, 224, 0, 0, 15, 128, 0,
        0, 62, 251, 225, 240, 0, 0, 7, 192, 0, 0, 31, 125, 240, 248, 0, 0, 3, 224, 0,
        0, 15, 190, 248, 124, 0, 0, 1, 240, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3,
        224, 62, 0, 15, 128, 248, 124, 0, 0, 1, 240, 31, 0, 7, 192, 124, 62, 0, 0, 0,
        248, 15, 128, 3, 224, 62, 31, 0, 0, 0, 124, 7, 192, 1, 240, 31, 15, 128, 0,
        0, 62, 3, 224, 0, 248, 15, 135, 192, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 15,
        190, 248, 124, 0, 248, 1, 240, 0, 0, 7, 223, 124, 62, 0, 124, 0, 248, 1, 128,
        3, 239, 190, 31, 0, 62, 0, 124, 1, 224, 1, 247, 223, 15, 128, 31, 0, 62, 0,
        240, 0, 251, 239, 135, 192, 15, 128, 31, 0, 48, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 62, 0, 1, 240, 248, 15, 135, 223, 30, 31, 223, 0, 0, 248, 124, 7, 195, 239,
        143, 31, 255, 128, 0, 124, 62, 3, 225, 247, 199, 158, 55, 192, 0, 62, 31, 1,
        240, 251, 227, 199, 131, 224, 0, 31, 15, 128, 248, 125, 241, 227, 240, 0, 0,
        0, 0, 0, 0, 0, 0, 240, 254, 0, 0, 0, 0, 0, 0, 0, 0, 120, 31, 128, 0, 0, 0, 0,
        0, 0, 12, 60, 3, 192, 0, 0, 0, 0, 0, 0, 15, 30, 49, 224, 0, 0, 0, 0, 0, 0, 7,
        143, 63, 240, 0, 0, 0, 0, 0, 0, 1, 135, 143, 224, 0, 0, 0, 0, 0, 0, 0, 3, 192,
        0, 0, 0, 0, 0, 0, 0, 0, 1, 224, 0, 0, 0, 0, 0, 0, 0, 0, 7, 224, 0, 0, 0, 0, 0,
        0, 0, 0, 3, 240, 0, 0, 0, 0, 0, 0, 0, 0, 1, 240, 0, 63
  };

  // Create 'flip' fn
  JsVar *fn = jsvNewNativeFunction((void (*)(void))lcd_flip, JSWAT_VOID|JSWAT_THIS_ARG|(JSWAT_BOOL << (JSWAT_BITS*1)));
  jsvObjectSetChildAndUnLock(graphics,"flip",fn);
  // LCD init 2
  jshDelayMicroseconds(10000);
  jshPinSetValue(LCD_SPI_RST,1);
  jshDelayMicroseconds(10000);
  const unsigned char LCD_SPI_INIT_DATA[] = {
       //0xE2,  // soft reset
       0xA2,   // bias 1/9
       //0xA3,   // bias 1/7
       0xC8,   // reverse scan dir
       0x81,   // contrast control (next byte)
       35,     // actual contrast (0..63)
       0x25,   // regulation resistor ratio (0..7)
       0x2F,   // control power circuits - last 3 bits = VB/VR/VF
       0xA0,   // start at column 128
       0xAF    // disp on
  };
  for (unsigned int i=0;i<sizeof(LCD_SPI_INIT_DATA);i++)
    lcd_wr(LCD_SPI_INIT_DATA[i]);
  jshPinSetValue(LCD_SPI_CS,1);

  /* If the button is pressed during reset, perform a self test.
   * With bootloader this means apply power while holding button for >3 secs */
  static bool firstStart = true;

  if (firstStart) {
    // animate logo in
    for (int i=128;i>24;i-=4) {
      lcd_flip_gfx(&gfx);
      graphicsClear(&gfx);
      graphicsDrawImage1bpp(&gfx,i,15,81,34,PIXLJS_IMG);
    }
  } else {
    // if a standard reset, just display logo
    graphicsClear(&gfx);
    graphicsDrawImage1bpp(&gfx,24,15,81,34,PIXLJS_IMG);
  }
  graphicsDrawString(&gfx,28,39,JS_VERSION);
  // Write MAC address in bottom right
  JsVar *addr = jswrap_nrf_bluetooth_getAddress();
  char buf[20];
  jsvGetString(addr, buf, sizeof(buf));
  jsvUnLock(addr);
  graphicsDrawString(&gfx,127-strlen(buf)*4,59,buf);
  lcd_flip_gfx(&gfx);


  if (firstStart && (jshPinGetValue(BTN1_PININDEX) == BTN1_ONSTATE || jshPinGetValue(BTN4_PININDEX) == BTN4_ONSTATE)) {
    // don't do it during a software reset - only first hardware reset
    jsiConsolePrintf("SELF TEST\n");
    if (pixl_selfTest()) jsiConsolePrintf("Test passed!\n");
  }

  // If the button is *still* pressed, remove all code from flash memory too!
  if (firstStart && jshPinGetValue(BTN1_PININDEX) == BTN1_ONSTATE) {
    jsfRemoveCodeFromFlash();
    jsiConsolePrintf("Removed saved code from Flash\n");
  }
  graphicsSetVar(&gfx);

  firstStart = false;
  jsvUnLock(graphics);
}

/*JSON{
  "type" : "kill",
  "generate" : "jswrap_pixljs_kill"
}*/
void jswrap_pixljs_kill() {

}

/*JSON{
  "type" : "idle",
  "generate" : "jswrap_pixljs_idle"
}*/
bool jswrap_pixljs_idle() {
  return false;
}
