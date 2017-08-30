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
 * Contains JavaScript interface for the hexagonal Espruino badge
 * ----------------------------------------------------------------------------
 */

/* DO_NOT_INCLUDE_IN_DOCS - this is a special token for common.py */

#include <jswrap_hexbadge.h>
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

#include "jswrap_graphics.h"
#include "lcd_arraybuffer.h"

/*

 var BTNA = D19;
var BTNB = D20;
var CORNER = [D25,D26,D27,D28,D29,D30];
var CAPSENSE = D22;
var BTNU = D31;
var BTND = D16;
var BTNL = D17;
var BTNR = D18;

 */

const Pin PIN_BTNA = 19;
const Pin PIN_BTNB = 20;
const Pin PIN_BTNU = 31;
const Pin PIN_BTND = 16;
const Pin PIN_BTNL = 17;
const Pin PIN_BTNR = 18;
const Pin PIN_CAPSENSE  = 22;
const Pin PIN_CORNERS[] = {25,26,27,28,29,30};

const Pin LCD_DC = 13;
const Pin LCD_CS = 12;
const Pin LCD_RST = 11;
const Pin LCD_SCK = 14;
const Pin LCD_MOSI = 15;


/*JSON{
  "type" : "variable",
  "name" : "BTNA",
  "generate_full" : "19",
  "return" : ["pin",""]
}*/
/*JSON{
  "type" : "variable",
  "name" : "BTNB",
  "generate_full" : "20",
  "return" : ["pin",""]
}*/
/*JSON{
  "type" : "variable",
  "name" : "BTNU",
  "generate_full" : "31",
  "return" : ["pin",""]
}*/
/*JSON{
  "type" : "variable",
  "name" : "BTND",
  "generate_full" : "16",
  "return" : ["pin",""]
}*/
/*JSON{
  "type" : "variable",
  "name" : "BTNL",
  "generate_full" : "17",
  "return" : ["pin",""]
}*/
/*JSON{
  "type" : "variable",
  "name" : "BTNR",
  "generate_full" : "18",
  "return" : ["pin",""]
}*/



/*JSON{
    "type": "class",
    "class" : "Badge"
}
Class containing utility functions for accessing IO on the hexagonal badge
*/
/*JSON{
    "type" : "staticmethod",
    "class" : "Badge",
    "name" : "capSense",
    "generate" : "jswrap_badge_capSense",
    "params" : [
      ["corner","int","The corner to use"]
    ],
    "return" : ["int", "Capacitive sense counter" ]
}
Capacitive sense - the higher the capacitance, the higher the number returned.

Supply a corner between 1 and 6, and a
*/
int jswrap_badge_capSense(int corner) {
  if (corner>=1 && corner<=6) {
    return (int)nrf_utils_cap_sense(PIN_CAPSENSE, PIN_CORNERS[corner-1]);
  }
  return 0;
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Badge",
    "name" : "getBatteryPercentage",
    "generate" : "jswrap_badge_getBatteryPercentage",
    "return" : ["int", "A percentage between 0 and 100" ]
}
Return an approximate battery percentage remaining based on
a normal CR2032 battery (2.8 - 2.2v)
*/
int jswrap_badge_getBatteryPercentage() {
  JsVarFloat v = jswrap_nrf_bluetooth_getBattery();
  int pc = (v-2.2)*100/0.6;
  if (pc>100) pc=100;
  if (pc<0) pc=0;
  return pc;
}



void badge_lcd_wr(int data) {
  int bit;
  for (bit=7;bit>=0;bit--) {
    jshPinSetValue(LCD_MOSI, (data>>bit)&1 );
    jshPinSetValue(LCD_SCK, 1 );
    jshPinSetValue(LCD_SCK, 0 );
  }
}

void badge_lcd_flip(JsVar *g) {
  JsVar *buf = jsvObjectGetChild(g,"buffer",0);
  if (!buf) return;
  JSV_GET_AS_CHAR_ARRAY(bPtr, bLen, buf);
  if (!bPtr || bLen<128*8) return;

  jshPinSetValue(LCD_CS,0);
  for (int y=0;y<8;y++) {
    jshPinSetValue(LCD_DC,0);
    badge_lcd_wr(0xB0|y/* page */);
    badge_lcd_wr(0x00/* x lower*/);
    badge_lcd_wr(0x10/* x upper*/);
    jshPinSetValue(LCD_DC,1);
    for (int x=0;x<128;x++)
      badge_lcd_wr(*(bPtr++));
  }
  jshPinSetValue(LCD_CS,1);
  jsvUnLock(buf);
}


/*JSON{
    "type" : "staticmethod",
    "class" : "Badge",
    "name" : "setContrast",
    "generate" : "jswrap_badge_setContrast",
    "params" : [
      ["c","float","Contrast between 0 and 1"]
    ]
}
Set the LCD's contrast */
void jswrap_badge_setContrast(JsVarFloat c) {
  if (c<0) c=0;
  if (c>1) c=1;
  jshPinSetValue(LCD_CS,0);
  jshPinSetValue(LCD_DC,0);
  badge_lcd_wr(0x81);
  badge_lcd_wr((int)(63*c));
  //badge_lcd_wr(0x20|div); div = 0..7
  jshPinSetValue(LCD_CS,1);
}

/*JSON{
  "type" : "init",
  "generate" : "jswrap_badge_init"
}*/
void jswrap_badge_init() {
  // LCD Init 1
  jshPinOutput(LCD_CS,0);
  jshPinOutput(LCD_DC,0);
  jshPinOutput(LCD_SCK,0);
  jshPinOutput(LCD_MOSI,0);
  jshPinOutput(LCD_RST,0);
  // Create backing graphics for LCD
  JsVar *graphics = jspNewObject(0, "Graphics");
  if (!graphics) return; // low memory
  JsGraphics gfx;
  graphicsStructInit(&gfx);
  gfx.data.type = JSGRAPHICSTYPE_ARRAYBUFFER;
  gfx.data.flags = JSGRAPHICSFLAGS_ARRAYBUFFER_VERTICAL_BYTE | JSGRAPHICSFLAGS_INVERT_X;
  gfx.graphicsVar = graphics;
  gfx.data.width = 128;
  gfx.data.height = 64;
  gfx.data.bpp = 1;
  lcdInit_ArrayBuffer(&gfx);
  graphicsSetVar(&gfx);
  jsvObjectSetChild(execInfo.root,"g",graphics);
  // Set initial image
  const unsigned char LCD_INIT_IMG[] = {
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 128, 128, 128, 128, 128, 192, 96, 176, 120, 60, 30, 14, 14, 12, 12, 12, 12, 12,
      12, 12, 28, 24, 24, 24, 152, 216, 120, 216, 240, 240, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 128, 128, 128, 128, 128, 128, 0, 0, 0, 128, 128, 128, 128, 128, 128, 128, 128, 0, 240, 240,
      208, 240, 0, 128, 128, 128, 0, 0, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 0, 0, 128, 128, 128, 128,
      128, 128, 128, 128, 0, 0, 128, 128, 128, 128, 128, 128, 0, 0, 112, 80, 80, 80, 80, 80, 208, 16, 240, 0, 0, 0, 0,
      255, 253, 57, 125, 231, 255, 2, 7, 7, 6, 6, 7, 7, 7, 15, 13, 13, 13, 15, 14, 14, 30, 30, 22, 22, 9, 6, 3, 0, 255,
      1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 62, 99, 136, 190, 195, 195, 190, 136,
      99, 62, 254, 131, 128, 254, 3, 2, 255, 128, 255, 0, 255, 128, 128, 255, 0, 255, 128, 255, 224, 192, 191, 128, 192,
      63, 3, 2, 7, 253, 128, 255, 8, 127, 129, 190, 98, 195, 255, 201, 0, 255, 48, 223, 142, 246, 213, 215, 186, 169,
      103, 192, 231, 165, 165, 165, 165, 165, 189, 128, 255, 0, 0, 0, 0, 0, 1, 1, 227, 255, 255, 0, 128, 128, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 192, 224, 248, 255, 248, 56, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 7, 15, 8, 15, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 7, 5, 7, 15, 11, 15, 15, 11, 15, 31, 30, 30, 30, 22, 30, 30, 8, 8, 12, 15,
      11, 15, 7, 1

  };
  JsVar *buf = jsvObjectGetChild(graphics,"buffer",0);
  JSV_GET_AS_CHAR_ARRAY(bPtr, bLen, buf);
  if (bPtr) memcpy(bPtr, LCD_INIT_IMG, sizeof(LCD_INIT_IMG));
  jsvUnLock(buf);
  // Create 'flip' fn
  JsVar *fn = jsvNewNativeFunction((void (*)(void))badge_lcd_flip, JSWAT_VOID|JSWAT_THIS_ARG);
  jsvObjectSetChildAndUnLock(graphics,"flip",fn);
  // LCD init 2
  jshDelayMicroseconds(10000);
  jshPinSetValue(LCD_RST,1);
  jshDelayMicroseconds(10000);
  const unsigned char LCD_INIT_DATA[] = {
       0xE2,
       0xA3,
       0xA1,
       0xC8,
       0x25,
       0x81,
       0x17,
       0x2F,
       0xAF
  };
  for (unsigned int i=0;i<sizeof(LCD_INIT_DATA);i++)
    badge_lcd_wr(LCD_INIT_DATA[i]);
  jshPinSetValue(LCD_CS,1);
  // actually flip the LCD contents
  badge_lcd_flip(graphics);
  jsvUnLock(graphics);
}

/*JSON{
  "type" : "kill",
  "generate" : "jswrap_badge_kill"
}*/
void jswrap_badge_kill() {

}

/*JSON{
  "type" : "idle",
  "generate" : "jswrap_badge_idle"
}*/
bool jswrap_badge_idle() {
  return false;
}
