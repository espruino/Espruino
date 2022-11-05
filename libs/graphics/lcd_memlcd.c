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
#include "jswrap_graphics.h"
#include "jswrap_espruino.h" // for reversebyte

// ======================================================================

#define LCD_SPI EV_SPI1
#define LCD_ROWHEADER 2
#define LCD_STRIDE (LCD_ROWHEADER+((LCD_WIDTH*LCD_BPP+7)>>3)) // data in required BPP, plus 2 bytes LCD command

/** Buffer for our LCD data.
  - We add 2 extra lines (LCD_HEIGHT+2) as a scratch area for doing the overlay (if enabled)
  - 2 bytes at end of transfer (needed by LCD) included in that extra line
  - 4 bytes at end (needed to allow fast scrolling) also handled by the extra line
 */
unsigned char lcdBuffer[LCD_STRIDE*(LCD_HEIGHT+2)];
bool isBacklightOn; ///< is LCD backlight on? If so we need to toggle EXTCOMIN faster
JsVar *lcdOverlayImage; ///< if set, an Image to use for overlays
short lcdOverlayX,lcdOverlayY; ///< coordinates of the graphics instance
volatile bool lcdIsBusy; ///< We're now allowing SPI send in the background - if we're sending, block execution until it finishes

#ifdef EMULATED
bool EMSCRIPTEN_GFX_CHANGED;

unsigned char fakeLCDBuffer[LCD_STRIDE*LCD_HEIGHT];

bool jsGfxChanged() {
  bool b = EMSCRIPTEN_GFX_CHANGED;
  EMSCRIPTEN_GFX_CHANGED = false;
  return b;
}
char *jsGfxGetPtr(int line) {
  if (line<0 || line>=LCD_HEIGHT) return 0;
  return &fakeLCDBuffer[LCD_ROWHEADER + (line*LCD_STRIDE)];
}
#endif

// bayer dithering pattern
#define BAYER_RGBSHIFT(b) (b<<13) | (b<<8) | (b<<2)
const unsigned short BAYER2[2][2] = {
    { BAYER_RGBSHIFT(1), BAYER_RGBSHIFT(5) },
    { BAYER_RGBSHIFT(7), BAYER_RGBSHIFT(3) }
};

static ALWAYS_INLINE unsigned int lcdMemLCD_convert16toLCD(unsigned int c, int x, int y) {
  c = (c&0b1110011100011100) + BAYER2[y&1][x&1];
  return
      ((c&0x10000)?1:0) |
      ((c&0x00800)?2:0) |
      ((c&0x00020)?4:0);
}

/** 'Flip' now ends while data is still sending to the LCD. This
 * function allows us to wait until the flip has finished (or a timeout
 * has occurred) which we'll need to do before we next modify what
 * is on the LCD.
 */
static ALWAYS_INLINE void lcdMemLCD_waitForSendComplete() {
  int timeout = 1000000;
  while (lcdIsBusy && --timeout) {};
  if (lcdIsBusy) {
    // LCD timeout! Do we want to log this?
    lcdIsBusy = false;
  }
}

// ======================================================================

unsigned int lcdMemLCD_getPixel(JsGraphics *gfx, int x, int y) {
#if LCD_BPP==3
  int bitaddr = LCD_ROWHEADER*8 + (x*3) + (y*LCD_STRIDE*8);
  int bit = bitaddr&7;
  uint16_t b = *(uint16_t*)&lcdBuffer[bitaddr>>3]; // get in MSB format
  unsigned int c = (b>>bit) & 0x7;
#endif
#if LCD_BPP==4
  int addr = LCD_ROWHEADER + (x>>1) + (y*LCD_STRIDE);
  unsigned char b = lcdBuffer[addr];
  unsigned int c = (x&1) ? ((b>>1)&7) : (b>>5);
#endif
  return  ((((c)&1)?0xF800:0)|(((c)&2)?0x07E0:0)|(((c)&4)?0x001F:0));
}

/*


 0123456701234567
 RGB               bpp=0,
              RGB  bpp=13




 */
void lcdMemLCD_setPixel(JsGraphics *gfx, int x, int y, unsigned int col) {
  col =  lcdMemLCD_convert16toLCD(col,x,y);
  lcdMemLCD_waitForSendComplete();
#if LCD_BPP==3
  int bitaddr = LCD_ROWHEADER*8 + (x*3) + (y*LCD_STRIDE*8);
  int bit = bitaddr&7;
  uint16_t b = *(uint16_t*)&lcdBuffer[bitaddr>>3];
  *(uint16_t*)&lcdBuffer[bitaddr>>3] = (b & ~(7<<bit)) | (col<<bit);
#endif
#if LCD_BPP==4
  int addr = LCD_ROWHEADER + (x>>1) + (y*LCD_STRIDE);
  if (x&1) lcdBuffer[addr] = (lcdBuffer[addr] & 0x0F) | (col << 4);
  else lcdBuffer[addr] = (lcdBuffer[addr] & 0xF0) | col;
#endif
}

void lcdMemLCD_fillRect(struct JsGraphics *gfx, int x1, int y1, int x2, int y2, unsigned int col) {
  lcdMemLCD_waitForSendComplete();
  // Super-fast fill if whole width
  if (x1==0 && x2==LCD_WIDTH-1 && (col==0 || col==0xFFFF)) {
    int addr = LCD_ROWHEADER + y1*LCD_STRIDE;
    for (int y=y1;y<=y2;y++) {
      memset(&lcdBuffer[addr], (col==0)?0:0xFF, LCD_STRIDE-LCD_ROWHEADER);
      addr += LCD_STRIDE;
    }
    return;
  }
  /* Otherwise go line-by line. 2x2 dithering means we're not writing the
   * same color to every pixel, so we usually work out what a 2x2 block
   * or 2x1 row is beforehand to save time
   */

#if LCD_BPP==3
  /* On 3bpp if we're filling a small width of pixels, just set them
  individually - it's possible to do with a mask too, but it hurts
  my head and it's not quite as big a performance improvement. */
  if (x2-x1 < 8) {
    // For 3 bit, precalculate what the 2 pixels go to
    unsigned int cols[2][2] = {
        { lcdMemLCD_convert16toLCD(col,0,0), lcdMemLCD_convert16toLCD(col,1,0) }, // even row
        { lcdMemLCD_convert16toLCD(col,0,1), lcdMemLCD_convert16toLCD(col,1,1) } // odd row
    };
    // write pixels individually
    for (int y=y1;y<=y2;y++) {
      unsigned int *c = cols[y&1];
      int bitaddr = LCD_ROWHEADER*8 + (x1*3) + (y*LCD_STRIDE*8);
      for (int x=x1;x<=x2;x++) {
        int bit = bitaddr&7;
        uint16_t b = *(uint16_t*)&lcdBuffer[bitaddr>>3];
        *(uint16_t*)&lcdBuffer[bitaddr>>3] = (b & ~(7<<bit)) | (c[x&1]<<bit);
        bitaddr += 3;
      }
    }
    return;
  }
#endif

  for (int y=y1;y<=y2;y++) {
#if LCD_BPP==3
    // For 3 bit with a lot of pixels, precalculate what the 2 pixels go to, then build up a 24 bit block
    unsigned int c = lcdMemLCD_convert16toLCD(col,0,y) | lcdMemLCD_convert16toLCD(col,1,y)<<3;
    c |= c<<6;
    c |= c<<12;
    /* Now things get a bit crazy - instead of doing every pixel we just mask off
     what we need from the 24 bit block of colour (8 pixels) and write it in  */
    uint8_t *row = (uint8_t*)&lcdBuffer[LCD_ROWHEADER + y*LCD_STRIDE];
    if (x1&7) { // do the first block before we're aligned (up to 8 pixels)
      int byteAddr = (x1>>3)*3;
      int bit = x1&7;
      uint32_t *pixels = (uint32_t*)&row[byteAddr];
      uint32_t mask = (0xFFFFFF<<(bit*3)) & 0xFFFFFF;
      *pixels = (*pixels&~mask) | (c&mask);
    }
    for (int x=(x1+7)>>3;x<(x2+1)>>3;x++) { // do the middle blocks of 24 bits (8 pixels)
      uint32_t *pixels = (uint32_t*)&row[x*3];
      uint32_t mask = 0xFFFFFF;
      *pixels = (*pixels&~mask) | (c&mask);
    }
    if ((x2+1)&7) { // do the final block after we're aligned (up to 8 pixels)
      int byteAddr = ((x2+1)>>3)*3;
      int bit = (x2+1)&7;
      uint32_t *pixels = (uint32_t*)&row[byteAddr];
      uint32_t mask = (0xFFFFFF<<(bit*3)) | 0xFF000000;
      *pixels = (*pixels&mask) | (c&~mask);
    }
#endif
#if LCD_BPP==4
    // For 4 bit, we can pack the 2 pixels into a byte
    unsigned char ditheredCol =
        lcdMemLCD_convert16toLCD(col,0,y) |
        (lcdMemLCD_convert16toLCD(col,1,y)<<4);
    int x=x1;
    int addr = LCD_ROWHEADER + (x>>1) + (y*LCD_STRIDE);
    if (x&1) { // first pixel on odd coordinate, unaligned
      lcdBuffer[addr] = (lcdBuffer[addr] & 0x0F) | (ditheredCol&0xF0);
      addr++;x++;
    }
    for (;x<x2;x+=2) // middle in blocks of 2, aligned so just a copy
      lcdBuffer[addr++] = ditheredCol;
    if (!(x2&1)) // final pixel on an even coordinate, unaligned
      lcdBuffer[addr] = (lcdBuffer[addr] & 0xF0) | (ditheredCol&0x0F);
#endif
  }
}


static void lcdMemLCD_scrollX(struct JsGraphics *gfx, unsigned char *dst, unsigned char *src, int xdir) {
  uint32_t *dw = (uint32_t*)&dst[LCD_ROWHEADER];
  uint32_t *sw = (uint32_t*)&src[LCD_ROWHEADER];

  if (xdir==0) {
    memcpy(dst, src, LCD_STRIDE);
  } else if (xdir<0) {
    int shiftBits = -xdir * LCD_BPP; // shiftBits positive
    int shiftWords = shiftBits>>5;
    shiftBits &= 31;
    int wordLen = (LCD_WIDTH*LCD_BPP - shiftBits)>>5;
    for (int x=0;x<=wordLen;x++)
      dw[x] = (sw[x+shiftWords]>>shiftBits) | (sw[x+shiftWords+1]<<(32-shiftBits));
  } else { // >0
    int shiftBits = xdir * LCD_BPP;
    int shiftWords = shiftBits>>5;
    shiftBits &= 31;
    int wordLen = (LCD_WIDTH*LCD_BPP + 15 - shiftBits)>>5;
    for (int x=0;x<=wordLen;x++)
      dw[x] = (sw[x-shiftWords]<<shiftBits) | (sw[x-(shiftWords+1)]>>(32-shiftBits));
  }
}

void lcdMemLCD_scroll(struct JsGraphics *gfx, int xdir, int ydir, int x1, int y1, int x2, int y2) {
  lcdMemLCD_waitForSendComplete();
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
    for (int y=y2-ydir;y>=y1;y--) {
      int yx = y+ydir;
      lcdMemLCD_scrollX(gfx, lineBuffer, &lcdBuffer[y*LCD_STRIDE], xdir);
      memcpy(&lcdBuffer[yx*LCD_STRIDE + LCD_ROWHEADER],&lineBuffer[LCD_ROWHEADER],LCD_STRIDE-LCD_ROWHEADER);
    }
  }
}

// -----------------------------------------------------------------------------
// used to allow SPI send to work async WHEN DOING OVERLAYS (we don't care when it finishes)
void lcdMemLCD_flip_spi_ovr_callback() {}
// used to allow SPI send to work async for normal sends.
void lcdMemLCD_flip_spi_callback() {
  jshPinSetValue(LCD_SPI_CS, 0);
  lcdIsBusy = false;
}
// send the data to the screen
void lcdMemLCD_flip(JsGraphics *gfx) {
  if (gfx->data.modMinY > gfx->data.modMaxY) return; // nothing to do!
#ifdef EMULATED
  EMSCRIPTEN_GFX_CHANGED = true;
#endif
  lcdMemLCD_waitForSendComplete();

  int y1 = gfx->data.modMinY;
  int y2 = gfx->data.modMaxY;
  int l = 1+y2-y1;

  bool hasOverlay = false;
  GfxDrawImageInfo overlayImg;
  if (lcdOverlayImage)
    hasOverlay = _jswrap_graphics_parseImage(gfx, lcdOverlayImage, 0, &overlayImg);

  jshPinSetValue(LCD_SPI_CS, 1);
  if (hasOverlay) {
    /* If lcdOverlayImage is defined, we want to overlay this image
     * on top of what we have in our LCD buffer. Do this line by
     * line. It's slower but it won't use a bunch of memory.
     *
     * We use an extra line added to the end of lcdBuffer for this, which
     * allows us to use lcdMemLCD_setPixel to do color conversion and dither
     * without loads of duplicate code.
     *
     * Optimisation: we could just send any non-overlaid stuff above or below
     * the overlay...
     */

    // initialise image layer
    GfxDrawImageLayer l;
    l.x1 = 0;
    l.y1 = lcdOverlayY;
    l.img = overlayImg;
    l.rotate = 0;
    l.scale = 1;
    l.center = false;
    l.repeat = false;
    jsvStringIteratorNew(&l.it, l.img.buffer, (size_t)l.img.bitmapOffset);
    _jswrap_drawImageLayerInit(&l);
    _jswrap_drawImageLayerSetStart(&l, 0, y1);
    for (int y=y1;y<=y2;y++) {
      int bufferLine = LCD_HEIGHT + (y&1); // alternate lines so we still get dither AND we can send while
      unsigned char *buf = &lcdBuffer[LCD_STRIDE*bufferLine]; // point to line right on the end of gfx
      // copy original line in
      memcpy(buf, &lcdBuffer[LCD_STRIDE*y], LCD_STRIDE);
      // overwrite areas with overlay image
      if (y>=lcdOverlayY && y<lcdOverlayY+overlayImg.height) {
        _jswrap_drawImageLayerStartX(&l);
        for (int x=0;x<overlayImg.width;x++) {
          unsigned int c;
          int ox = x+lcdOverlayX;
          if (_jswrap_drawImageLayerGetPixel(&l, &c) && (ox < LCD_WIDTH) && (ox >= 0))
            lcdMemLCD_setPixel(NULL, ox, bufferLine, c);
          _jswrap_drawImageLayerNextX(&l);
        }
      }
      _jswrap_drawImageLayerNextY(&l);
      // send the line
#ifdef EMULATED
      memcpy(&fakeLCDBuffer[LCD_STRIDE*y], buf, LCD_STRIDE);
#else
      jshSPISendMany(LCD_SPI, buf, NULL, LCD_STRIDE, lcdMemLCD_flip_spi_ovr_callback);
#endif
    }
    jsvStringIteratorFree(&l.it);
    _jswrap_graphics_freeImageInfo(&overlayImg);
    // any 2 final bytes to finish the transfer
#ifndef EMULATED
    jshSPISendMany(LCD_SPI, lcdBuffer, NULL, 2, NULL);
    lcdMemLCD_flip_spi_callback();
#endif
  } else { // standard, non-overlay
#ifdef EMULATED
    memcpy(fakeLCDBuffer, lcdBuffer, LCD_HEIGHT*LCD_STRIDE);
#else
    lcdIsBusy = true;
    if (!jshSPISendMany(LCD_SPI, &lcdBuffer[LCD_STRIDE*y1], NULL, (l*LCD_STRIDE)+2, lcdMemLCD_flip_spi_callback))
      lcdMemLCD_flip_spi_callback();
    // lcdMemLCD_flip_spi_callback will call jshPinSetValue(LCD_SPI_CS, 0); when done and set lcdIsBusy=false
#endif
  }
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
    lcdBuffer[y*LCD_STRIDE]=jswrap_espruino_reverseByte(0b10000000);
#endif
#if LCD_BPP==4
    lcdBuffer[y*LCD_STRIDE]=jswrap_espruino_reverseByte(0b10010000);
#endif
    lcdBuffer[(y*LCD_STRIDE)+1]=jswrap_espruino_reverseByte(y+1);
  }

  jshPinOutput(LCD_SPI_CS,0);
  jshPinOutput(LCD_SPI_SCK,1);
  jshPinOutput(LCD_SPI_MOSI,1);
  jshPinOutput(LCD_DISP,1);
  jshPinOutput(LCD_EXTCOMIN,1);

  JshSPIInfo inf;
  jshSPIInitInfo(&inf);
  inf.baudRate = 4000000; // it seems 8000000 is too fast to be reliable
  inf.pinMOSI = LCD_SPI_MOSI;
  inf.pinSCK = LCD_SPI_SCK;
  inf.spiMSB = false; // LSB first!
  jshSPISetup(LCD_SPI, &inf);
}

// toggle EXTCOMIN to avoid burn-in
void lcdMemLCD_extcominToggle() {
  if (!isBacklightOn) {
    jshPinSetValue(LCD_EXTCOMIN, 1);
    jshDelayMicroseconds(2); // datasheet saus 2uS min rise time
    jshPinSetValue(LCD_EXTCOMIN, 0);
  }
}

// If backlight is on, we need to raise EXTCOMIN freq (use HW PWM)
void lcdMemLCD_extcominBacklight(bool isOn) {
  if (isBacklightOn != isOn) {
    isBacklightOn = isOn;
    if (isOn) {
      jshPinAnalogOutput(LCD_EXTCOMIN, 0.0003, 120, JSAOF_NONE); // ~3us
    } else {
      jshPinOutput(LCD_EXTCOMIN, 0);
    }
  }
}

// Enable overlay mode (to overlay a graphics instance on top of the LCD contents)
void lcdMemLCD_setOverlay(JsVar *imgVar, int x, int y) {
  if (lcdOverlayImage) jsvUnLock(lcdOverlayImage);
  if (imgVar) {
    lcdOverlayImage = jsvLockAgain(imgVar);
    lcdOverlayX = (short)x;
    lcdOverlayY = (short)y;
  } else {
    lcdOverlayImage = 0;
    lcdOverlayX = 0;
    lcdOverlayY = 0;
  }
}

void lcdMemLCD_setCallbacks(JsGraphics *gfx) {
  gfx->setPixel = lcdMemLCD_setPixel;
  gfx->fillRect = lcdMemLCD_fillRect;
  gfx->getPixel = lcdMemLCD_getPixel;
  gfx->scroll = lcdMemLCD_scroll;
}

