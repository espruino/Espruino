/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2019 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * Graphics Backend for drawing to LPM013M126 displays
 * ----------------------------------------------------------------------------
 */

#include "platform_config.h"
#include "jsutils.h"
#include "jshardware.h"
#include "jsinteractive.h"
#include "lcd_memlcd.h"

// ======================================================================

#define LCD_STRIDE (2+((LCD_WIDTH*LCD_BPP+7)>>3)) // data in required BPP, plus 2 bytes LCD command
unsigned char lcdBuffer[LCD_STRIDE*LCD_HEIGHT +2/*2 bytes end of transfer*/];
lcdMemLCDMode lcdMode = MEMLCD_MODE_NORMAL;

#define LCD_SPI EV_SPI1



// ======================================================================

unsigned int lcdMemLCD_getPixel(JsGraphics *gfx, int x, int y) {
#if LCD_BPP==3
  int bitaddr = 2*8 + (x*3) + (y*LCD_STRIDE*8);
  int bit = bitaddr&7;
  uint16_t b = __builtin_bswap16(*(uint16_t*)&lcdBuffer[bitaddr>>3]); // get in MSB format
  return ((b<<bit) & 0xE000) >> 13;
#endif
#if LCD_BPP==4
  int addr = 2 + (x>>1) + (y*LCD_STRIDE);
  unsigned char b = lcdBuffer[addr];
  return (x&1) ? (b&15) : (b>>4);
#endif
}


void lcdMemLCD_setPixel(JsGraphics *gfx, int x, int y, unsigned int col) {
#if LCD_BPP==3
  int bitaddr = 2*8 + (x*3) + (y*LCD_STRIDE*8);
  int bit = bitaddr&7;
  uint16_t b = __builtin_bswap16(*(uint16_t*)&lcdBuffer[bitaddr>>3]);
  b = (b & (~(0xE000>>bit))) | ((col&7)<<(13-bit));
  *(uint16_t*)&lcdBuffer[bitaddr>>3] = __builtin_bswap16(b);
#endif
#if LCD_BPP==4
  int addr = 2 + (x>>1) + (y*LCD_STRIDE);
  if (x&1) lcdBuffer[addr] = (lcdBuffer[addr] & 0xF0) | (col&0x0F);
  else lcdBuffer[addr] = (lcdBuffer[addr] & 0x0F) | (col << 4);
#endif
}

// -----------------------------------------------------------------------------

void lcdMemLCD_setPixel240(JsGraphics *gfx, int x, int y, unsigned int col) {
  x = (x*LCD_WIDTH) / 240;
  y = (y*LCD_HEIGHT) / 240;
  if (x<0 || y<0 || x>=LCD_WIDTH || y>=LCD_HEIGHT) return;
  unsigned int b = col;
  unsigned int br = (b>>11)&0x1F;
  unsigned int bg = (b>>5)&0x3F;
  unsigned int bb = b&0x1F;
  // TODO: dither?
  col = ((br>15)?4:0) | ((bg>31)?2:0) | ((bb>15)?1:0);
  lcdMemLCD_setPixel(gfx,x,y,col);
}

// -----------------------------------------------------------------------------

void lcdMemLCD_flip(JsGraphics *gfx) {
  if (gfx->data.modMinY > gfx->data.modMaxY) return; // nothing to do!

  int y1 = gfx->data.modMinY;
  int y2 = gfx->data.modMaxY;

  if (lcdMode==MEMLCD_MODE_240x240) {
    y1 = (y1*LCD_HEIGHT) / 240;
    y2 = (y2*LCD_HEIGHT) / 240;
  }

  int l = 1+y2-y1;

  jshPinSetValue(LCD_SPI_CS, 1);
  //jshDelayMicroseconds(10000);
  jshSPISendMany(LCD_SPI, &lcdBuffer[LCD_STRIDE*y1], NULL, (l*LCD_STRIDE)+2, NULL);
  //jshDelayMicroseconds(10000);
  jshPinSetValue(LCD_SPI_CS, 0);
  // Reset modified-ness
  gfx->data.modMaxX = -32768;
  gfx->data.modMaxY = -32768;
  gfx->data.modMinX = 32767;
  gfx->data.modMinY = 32767;
}

void lcdMemLCD_init(JsGraphics *gfx) {
  gfx->data.width = LCD_WIDTH;
  gfx->data.height = LCD_HEIGHT;
  gfx->data.bpp = LCD_BPP;
  memset(lcdBuffer,0,sizeof(lcdBuffer));
  for (int y=0;y<LCD_HEIGHT;y++) {
#if LCD_BPP==3
    lcdBuffer[y*LCD_STRIDE]=0b10000000;
#endif
#if LCD_BPP==4
    lcdBuffer[y*LCD_STRIDE]=0b10010000;
#endif
    lcdBuffer[(y*LCD_STRIDE)+1]=y+1;
  }

  jshPinOutput(LCD_SPI_CS,0);
  jshPinOutput(LCD_SPI_SCK,0);
  jshPinOutput(LCD_SPI_MOSI,1);
  jshPinOutput(LCD_DISP,1);
  jshPinOutput(LCD_EXTCOMIN,1);

  JshSPIInfo inf;
  jshSPIInitInfo(&inf);
  inf.baudRate = 4000000; // it seems 8000000 is too fast to be reliable
  inf.pinMOSI = LCD_SPI_MOSI;
  inf.pinSCK = LCD_SPI_SCK;
  jshSPISetup(LCD_SPI, &inf);
}

void lcdMemLCD_setMode(lcdMemLCDMode mode) {
  lcdMode = mode;
}

// toggle EXTCOMIN to avoid burn-in
void lcdMemLCD_extcomin() {
  static bool extcomin = false;
  extcomin = !extcomin;
  jshPinSetValue(LCD_EXTCOMIN, extcomin);
}

void lcdMemLCD_setCallbacks(JsGraphics *gfx) {
  if (lcdMode==MEMLCD_MODE_NORMAL) {
    gfx->setPixel = lcdMemLCD_setPixel;
    gfx->getPixel = lcdMemLCD_getPixel;
  } else if (lcdMode==MEMLCD_MODE_240x240) {
    gfx->setPixel = lcdMemLCD_setPixel240;
  }
}

