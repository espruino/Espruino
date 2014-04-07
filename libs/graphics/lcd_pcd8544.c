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
 * Graphics Backend for drawing to simple black and white SPI displays
 * (PCD8544 - Nokia 5110 LCD)
 * FIXME: UNFINISHED
 * ----------------------------------------------------------------------------
 */

#include "platform_config.h"
#include "jsutils.h"
#include "lcd.h"

#define LCD_WIDTH 84
#define LCD_HEIGHT 48

typedef struct {
  unsigned char changeX1,changeY1,changeX2,changeY2;
  unsigned char pixels[LCD_WIDTH*LCD_HEIGHT/8];  
} LCDDataPCD8544 PACKED_FLAGS;

unsigned int lcdGetPixel_PCD8544(JsGraphics *gfx, short x, short y) {
  int yp = y>>3;
  int addr = x + (yp*gfx->data.width);
  return (pixels[addr]>>(y&7)) & 1;
}


void lcdSetPixel_PCD8544(JsGraphics *gfx, short x, short y, unsigned int col) {
  int yp = y>>3;
  int addr = x + (yp*gfx->data.width);
  if (col) pixels[addr] |= 1<<(y&7);
  else pixels[addr] &= ~(1<<(y&7));
  if (x<changeX1) changeX1=x;
  if (y<changeY1) changeY1=y;
  if (x>changeX2) changeX2=x;
  if (y>changeY2) changeY2=y;
}

void lcdIdle_PCD8544(JsGraphics *gfx) {
  if (changeX1>=changeX2 && changeY1>=changeY2) {
    // write...
    int cy1 = changeY1 >> 3;
    int cy2 = changeY2 >> 3;
    int x,y;
    jshPinOutput(CE, 0);
    for (y=cy1;y<=cy2;y++) {
      jshPinOutput(DC, 0); // command
      jshSPISend(dev, 0x40 | y); // Y addr
      jshSPISend(dev, 0x80); // X addr
      jshPinOutput(DC, 1); // data
      for (x=changeX1;x<=changeX2;x++) 
        jshSPISend(dev, pixels[x+y*LCD_WIDTH]);
    }      
    jshPinOutput(DC, 0); // command
    jshSPISend(dev, 0x40); // reset?
    jshPinOutput(CE, 1);

    changeX1 = LCD_WIDTH;
    changeY1 = LCD_HEIGHT;
    changeX2 = 0;
    changeY2 = 0;
  }
}

void lcdInit_PCD8544(JsGraphics *gfx) {
  assert(gfx->data.bpp == 1);

  IOEventFlags dev = EV_SPI1;
  jshPinOutput(CE, 0);
  // pulse reset 10ms?
  jshPinOutput(DC, 0); // command
  jshSPISend(dev, 0x21); // fnset extended
  jshSPISend(dev, 0x80 | 0x40); // setvop (experiment with 2nd val to get the right contrast)
  jshSPISend(dev, 0x14); // setbias 4
  jshSPISend(dev, 0x04 | 0x02); // temp control
  jshSPISend(dev, 0x20); // fnset normal
  jshSPISend(dev, 0x08 | 0x04); // dispctl normal
  jshPinOutput(CE, 1);
}

void lcdSetCallbacks_PCD8544(JsGraphics *gfx) {
  gfx->data.width = LCD_WIDTH;
  gfx->data.height = LCD_HEIGHT;
  gfx->data.bpp = 1;
  gfx->setPixel = lcdSetPixel_PCD8544;
  gfx->getPixel = lcdGetPixel_PCD8544;
  gfx->idle = lcdIdle_PCD8544;

}

