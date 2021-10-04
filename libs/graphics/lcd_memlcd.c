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

#define LCD_SPI EV_SPI1
#define LCD_ROWHEADER 2
#define LCD_STRIDE (LCD_ROWHEADER+((LCD_WIDTH*LCD_BPP+7)>>3)) // data in required BPP, plus 2 bytes LCD command

unsigned char lcdBuffer[LCD_STRIDE*LCD_HEIGHT +2/*2 bytes end of transfer*/ +4/*allow extra for fast scroll*/];
bool isBacklightOn;

#ifdef EMULATED
bool EMSCRIPTEN_GFX_CHANGED;

bool jsGfxChanged() {
  bool b = EMSCRIPTEN_GFX_CHANGED;
  EMSCRIPTEN_GFX_CHANGED = false;
  return b;
}
char *jsGfxGetPtr(int line) {
  if (line<0 || line>=LCD_HEIGHT) return 0;
  return &lcdBuffer[LCD_ROWHEADER + (line*LCD_STRIDE)];
}
#endif

// bayer dithering pattern
#define BAYER_RGBSHIFT(b) (b<<13) | (b<<8) | (b<<2)
const unsigned short BAYER2[2][2] = {
    { BAYER_RGBSHIFT(1), BAYER_RGBSHIFT(5) },
    { BAYER_RGBSHIFT(7), BAYER_RGBSHIFT(3) }
};

static unsigned int lcdMemLCD_convert16to3(unsigned int c, int x, int y) {
  c = (c&0b1110011100011100) + BAYER2[y&1][x&1];
  return
      ((c&0x10000)?4:0) |
      ((c&0x00800)?2:0) |
      ((c&0x00020)?1:0);
}

// ======================================================================

unsigned int lcdMemLCD_getPixel(JsGraphics *gfx, int x, int y) {
#if LCD_BPP==3
  int bitaddr = LCD_ROWHEADER*8 + (x*3) + (y*LCD_STRIDE*8);
  int bit = bitaddr&7;
  uint16_t b = __builtin_bswap16(*(uint16_t*)&lcdBuffer[bitaddr>>3]); // get in MSB format
  unsigned int c = ((b<<bit) & 0xE000) >> 13;
#endif
#if LCD_BPP==4
  int addr = LCD_ROWHEADER + (x>>1) + (y*LCD_STRIDE);
  unsigned char b = lcdBuffer[addr];
  unsigned int c = (x&1) ? ((b>>1)&7) : (b>>5);
#endif
  return GRAPHICS_COL_3_TO_16(c);
}


void lcdMemLCD_setPixel(JsGraphics *gfx, int x, int y, unsigned int col) {
#ifdef EMULATED
  EMSCRIPTEN_GFX_CHANGED = true;
#endif
  col = lcdMemLCD_convert16to3(col,x,y);
#if LCD_BPP==3
  int bitaddr = LCD_ROWHEADER*8 + (x*3) + (y*LCD_STRIDE*8);
  int bit = bitaddr&7;
  uint16_t b = __builtin_bswap16(*(uint16_t*)&lcdBuffer[bitaddr>>3]);
  b = (b & (0xFF1FFF>>bit)) | (col<<(13-bit));
  *(uint16_t*)&lcdBuffer[bitaddr>>3] = __builtin_bswap16(b);
#endif
#if LCD_BPP==4
  int addr = LCD_ROWHEADER + (x>>1) + (y*LCD_STRIDE);
  if (x&1) lcdBuffer[addr] = (lcdBuffer[addr] & 0xF0) | (col<<1);
  else lcdBuffer[addr] = (lcdBuffer[addr] & 0x0F) | (col << 5);
#endif
}

#if LCD_BPP==3
void lcdMemLCD_fillRect(struct JsGraphics *gfx, int x1, int y1, int x2, int y2, unsigned int col) {
#ifdef EMULATED
  EMSCRIPTEN_GFX_CHANGED = true;
#endif
  for (int y=y1;y<=y2;y++) {
    int bitaddr = LCD_ROWHEADER*8 + (x1*3) + (y*LCD_STRIDE*8);
    for (int x=x1;x<=x2;x++) {
      int bit = bitaddr&7;
      unsigned int c = lcdMemLCD_convert16to3(col,x,y);
      uint16_t b = __builtin_bswap16(*(uint16_t*)&lcdBuffer[bitaddr>>3]);
      b = (b & (0xFF1FFF>>bit)) | (c<<(13-bit));
      *(uint16_t*)&lcdBuffer[bitaddr>>3] = __builtin_bswap16(b);
      bitaddr += 3;
    }
  }
}
#endif

void lcdMemLCD_scrollX(struct JsGraphics *gfx, unsigned char *dst, unsigned char *src, int xdir) {
  uint32_t *dw = (uint32_t*)&dst[LCD_ROWHEADER];
  uint32_t *sw = (uint32_t*)&src[LCD_ROWHEADER];

  if (xdir==0) {
    memcpy(dst, src, LCD_STRIDE);
  } else if (xdir<0) {
    int shiftBits = -xdir * LCD_BPP;
    int shiftWords = shiftBits>>5;
    shiftBits &= 31;
    int wordLen = (LCD_WIDTH*LCD_BPP - shiftBits)>>5;
    for (int x=0;x<=wordLen;x++)
      dw[x] = __builtin_bswap32((__builtin_bswap32(sw[x+shiftWords])<<shiftBits) | __builtin_bswap32(sw[x+shiftWords+1])>>(32-shiftBits));
  } else { // >0
    int shiftBits = xdir * LCD_BPP;
    int shiftWords = shiftBits>>5;
    shiftBits &= 31;
    int wordLen = (LCD_WIDTH*LCD_BPP + 15 - shiftBits)>>5;
    for (int x=0;x<=wordLen;x++)
      dw[x] = __builtin_bswap32((__builtin_bswap32(sw[x-shiftWords])>>shiftBits) | __builtin_bswap32(sw[x-(shiftWords+1)])<<(32-shiftBits));
  }
}

void lcdMemLCD_scroll(struct JsGraphics *gfx, int xdir, int ydir, int x1, int y1, int x2, int y2) {
#ifdef EMULATED
  EMSCRIPTEN_GFX_CHANGED = true;
#endif
  // if we can't shift entire line in one go, go with the slow method as this case would be a nightmare in 3 bits
  if (x1!=0 || x2!=LCD_WIDTH-1)
    return graphicsFallbackScroll(gfx, xdir, ydir, x1,y1,x2,y2);
  // otherwise...
  unsigned char lineBuffer[LCD_STRIDE+4]; // allow out of bounds write
  if (ydir<=0) {
    for (int y=y1;y<=y2+ydir;y++) {
      int yx = y-ydir;
      lcdMemLCD_scrollX(gfx, lineBuffer, &lcdBuffer[yx*LCD_STRIDE], xdir);
      memcpy(&lcdBuffer[y*LCD_STRIDE + LCD_ROWHEADER],&lineBuffer[LCD_ROWHEADER],LCD_STRIDE-LCD_ROWHEADER);
    }
  } else if (ydir>0) {
    for (int y=y2-ydir-1;y>=y1;y--) {
      int yx = y+ydir;
      lcdMemLCD_scrollX(gfx, lineBuffer, &lcdBuffer[y*LCD_STRIDE], xdir);
      memcpy(&lcdBuffer[yx*LCD_STRIDE + LCD_ROWHEADER],&lineBuffer[LCD_ROWHEADER],LCD_STRIDE-LCD_ROWHEADER);
    }
  }
}

// -----------------------------------------------------------------------------

void lcdMemLCD_flip(JsGraphics *gfx) {
  if (gfx->data.modMinY > gfx->data.modMaxY) return; // nothing to do!

  int y1 = gfx->data.modMinY;
  int y2 = gfx->data.modMaxY;
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
  gfx->data.bpp = 16; // take color as 16 bit even though we only use 3
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

// toggle EXTCOMIN to avoid burn-in
void lcdMemLCD_extcominToggle() {
  if (!isBacklightOn) {
    jshPinSetValue(LCD_EXTCOMIN, 1);
    jshPinSetValue(LCD_EXTCOMIN, 0);
  }
}

// If backlight is on, we need to raise EXTCOMIN freq (use HW PWM)
void lcdMemLCD_extcominBacklight(bool isOn) {
  if (isBacklightOn != isOn) {
    isBacklightOn = isOn;
    if (isOn) {
      jshPinAnalogOutput(LCD_EXTCOMIN, 0.05, 120, JSAOF_NONE);
    } else {
      jshPinOutput(LCD_EXTCOMIN, 0);
    }
  }

}

void lcdMemLCD_setCallbacks(JsGraphics *gfx) {
  gfx->setPixel = lcdMemLCD_setPixel;
#if LCD_BPP==3
  gfx->fillRect = lcdMemLCD_fillRect;
#endif
  gfx->getPixel = lcdMemLCD_getPixel;
  gfx->scroll = lcdMemLCD_scroll;
}

