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

#include <jswrap_jslcd.h>
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

const Pin LCD_DC = 13;
const Pin LCD_CS = 12;
const Pin LCD_RST = 11;
const Pin LCD_SCK = 14;
const Pin LCD_MOSI = 15;

/*JSON{
    "type": "class",
    "class" : "LCD"
}
Class containing utility functions for accessing IO on the hexagonal badge
*/

/*JSON{
    "type" : "staticmethod",
    "class" : "LCD",
    "name" : "getBatteryPercentage",
    "generate" : "jswrap_jslcd_getBatteryPercentage",
    "return" : ["int", "A percentage between 0 and 100" ]
}
Return an approximate battery percentage remaining based on
a normal CR2032 battery (2.8 - 2.2v)
*/
int jswrap_jslcd_getBatteryPercentage() {
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
    "class" : "LCD",
    "name" : "setContrast",
    "generate" : "jswrap_jslcd_setContrast",
    "params" : [
      ["c","float","Contrast between 0 and 1"]
    ]
}
Set the LCD's contrast */
void jswrap_jslcd_setContrast(JsVarFloat c) {
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
    "type" : "staticmethod",
    "class" : "LCD",
    "name" : "lcdw",
    "generate" : "jswrap_jslcd_lcdw",
    "params" : [
      ["c","int",""]
    ]
}
Set the LCD's contrast */
void jswrap_jslcd_lcdw(JsVarInt c) {
  jshPinSetValue(LCD_CS,0);
  jshPinSetValue(LCD_DC,0);
  badge_lcd_wr(c);
  jshPinSetValue(LCD_CS,1);
}

/*JSON{
  "type" : "init",
  "generate" : "jswrap_jslcd_init"
}*/
void jswrap_jslcd_init() {
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
  gfx.data.flags = JSGRAPHICSFLAGS_ARRAYBUFFER_VERTICAL_BYTE;
  gfx.graphicsVar = graphics;
  gfx.data.width = 128;
  gfx.data.height = 64;
  gfx.data.bpp = 1;
  lcdInit_ArrayBuffer(&gfx);
  graphicsSetVar(&gfx);
  jsvObjectSetChild(execInfo.root,"g",graphics);
  // Set initial image
  const unsigned int LCD_IMIT_IMG_OFFSET = 266;
  const unsigned char LCD_INIT_IMG[] = {
      240, 240, 152, 56, 216, 24, 24, 24, 24, 8, 12, 12, 12, 12, 12, 12, 12, 6, 14, 30, 52, 88, 176, 96, 192, 128, 128, 128, 128, 128,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 1, 6, 9, 22, 22, 30, 30, 14, 14, 13, 13, 13, 13, 15, 7, 7, 6, 6, 6, 6, 3, 2, 255,
      199, 125, 56, 129, 255, 0, 0, 0, 0, 240, 240, 240, 48, 48, 48, 48, 48, 48, 0, 0, 0, 128, 128, 128, 128, 0, 0, 0, 0, 128, 128, 0,
      128, 128, 128, 0, 0, 0, 0, 128, 128, 0, 128, 128, 0, 128, 128, 0, 0, 0, 128, 128, 128, 0, 0, 176, 176, 0, 0, 128, 128, 0, 0, 128,
      128, 128, 0, 0, 0, 0, 0, 128, 128, 128, 128, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 56, 120,
      255, 248, 224, 192, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 128, 128, 0, 255, 255, 226, 1, 1, 0, 0, 0, 0, 0, 255, 255,
      255, 198, 198, 198, 198, 198, 192, 0, 0, 103, 207, 205, 217, 217, 251, 114, 0, 0, 255, 255, 99, 193, 193, 227, 127, 62, 0, 0, 255,
      255, 3, 1, 1, 0, 127, 255, 192, 192, 192, 127, 255, 255, 0, 0, 255, 255, 0, 0, 255, 255, 255, 1, 1, 3, 255, 255, 0, 0, 62, 127, 227,
      193, 193, 227, 127, 62, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 3, 15, 11, 15, 12, 8, 8, 30,
      30, 22, 30, 30, 22, 31, 15, 11, 15, 15, 11, 15, 7, 5, 7, 1
  };
  JsVar *buf = jsvObjectGetChild(graphics,"buffer",0);
  JSV_GET_AS_CHAR_ARRAY(bPtr, bLen, buf);
  if (bPtr) memcpy(&bPtr[LCD_IMIT_IMG_OFFSET], LCD_INIT_IMG, sizeof(LCD_INIT_IMG));
  jsvUnLock(buf);
  // Create 'flip' fn
  JsVar *fn = jsvNewNativeFunction((void (*)(void))badge_lcd_flip, JSWAT_VOID|JSWAT_THIS_ARG);
  jsvObjectSetChildAndUnLock(graphics,"flip",fn);
  // LCD init 2
  jshDelayMicroseconds(10000);
  jshPinSetValue(LCD_RST,1);
  jshDelayMicroseconds(10000);
  const unsigned char LCD_INIT_DATA[] = {
       //0xE2,  // soft reset
       0xA3,   // bias 1/7
       0xC8,   // reverse scan dir
       0x25,   // regulation resistor ratio (0..7)
       0x81,   // contrast control
       0x12,
       0x2F,   // control power circuits - last 3 bits = VB/VR/VF
       0xA0,   // start at column 128
       0xAF    // disp on
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
  "generate" : "jswrap_jslcd_kill"
}*/
void jswrap_jslcd_kill() {

}

/*JSON{
  "type" : "idle",
  "generate" : "jswrap_jslcd_idle"
}*/
bool jswrap_jslcd_idle() {
  return false;
}
