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
 * Graphics Backend for drawing to SPI displays
 * ----------------------------------------------------------------------------
 */

#include "platform_config.h"
#include "jsutils.h"
#include "jshardware.h"
#include "lcd_spilcd.h"
#include "lcd_spilcd_info.h"
#include "lcd_spilcd_palette.h"

// ======================================================================

#define LCD_STRIDE ((LCD_WIDTH*LCD_BPP+7)>>3)
unsigned char lcdBuffer[LCD_STRIDE*LCD_HEIGHT];
#if LCD_BPP==4
unsigned short lcdPalette[16];
#endif
#if LCD_BPP==8
unsigned short lcdPalette[256];
#endif

#define LCD_SPI EV_SPI1

// ======================================================================


// ======================================================================

void lcdCmd_SPILCD(int cmd, int dataLen, const char *data) {
  jshPinSetValue(LCD_SPI_CS, 0);
  jshPinSetValue(LCD_SPI_DC, 0); // command
  jshSPISend(LCD_SPI, cmd);
  if (dataLen) {
    jshPinSetValue(LCD_SPI_DC, 1); // data
    while (dataLen) {
      jshSPISend(LCD_SPI, *(data++));
      dataLen--;
    }
  }
  jshPinSetValue(LCD_SPI_CS, 1);
}
void lcdSendInitCmd_SPILCD() {
  // Send initialization commands to ST7735
  const char *cmd = SPILCD_INIT_CODE;
  while(cmd[CMDINDEX_DATALEN]!=255) {
    lcdCmd_SPILCD(cmd[CMDINDEX_CMD], cmd[CMDINDEX_DATALEN], &cmd[3]);
    if (cmd[CMDINDEX_DELAY])
      jshDelayMicroseconds(1000*cmd[CMDINDEX_DELAY]);
    cmd += 3 + cmd[CMDINDEX_DATALEN];
  }
}
void lcdSetPalette_SPILCD(const char *pal) {
  memcpy(lcdPalette, pal ? pal : SPILCD_PALETTE, sizeof(lcdPalette));
}

// ======================================================================


unsigned int lcdGetPixel_SPILCD(JsGraphics *gfx, int x, int y) {
#if LCD_BPP==4
  int addr = (x + (y*LCD_WIDTH)) >> 1;
  unsigned char b = lcdBuffer[addr];
  return (x&1) ? (b&15) : (b>>4);
#endif
#if LCD_BPP==8
  int addr = x + (y*LCD_WIDTH);
  return lcdBuffer[addr];
#endif
}


void lcdSetPixel_SPILCD(JsGraphics *gfx, int x, int y, unsigned int col) {
#if LCD_BPP==4
  int addr = (x + (y*LCD_WIDTH)) >> 1;
  if (x&1) lcdBuffer[addr] = (lcdBuffer[addr] & 0xF0) | (col&0x0F);
  else lcdBuffer[addr] = (lcdBuffer[addr] & 0x0F) | (col << 4);
#endif
#if LCD_BPP==8
  int addr = x + (y*LCD_WIDTH);
  lcdBuffer[addr] = col;
#endif
}

void lcdFlip_SPILCD_callback() {
  // just an empty stub for SPIsend - we'll just push data as fast as we can
}

void lcdFlip_SPILCD(JsGraphics *gfx) {
  if (gfx->data.modMinX > gfx->data.modMaxX) return; // nothing to do!

  unsigned char buffer1[LCD_WIDTH*2]; // 16 bits per pixel
  unsigned char buffer2[LCD_WIDTH*2]; // 16 bits per pixel

  // use nearest 2 pixels as we're sending 12 bits
  gfx->data.modMinX = (gfx->data.modMinX)&~1;
  gfx->data.modMaxX = (gfx->data.modMaxX+2)&~1;
  int xlen = gfx->data.modMaxX - gfx->data.modMinX;
  int xstart = gfx->data.modMinX;

  jshPinSetValue(LCD_SPI_CS, 0);
  jshPinSetValue(LCD_SPI_DC, 0); // command
  buffer1[0] = SPILCD_CMD_WINDOW_X;
  jshSPISendMany(LCD_SPI, buffer1, NULL, 1, NULL);
  jshPinSetValue(LCD_SPI_DC, 1); // data
  buffer1[0] = 0;
  buffer1[1] = gfx->data.modMinX;
  buffer1[2] = 0;
  buffer1[3] = gfx->data.modMaxX;
  jshSPISendMany(LCD_SPI, buffer1, NULL, 4, NULL);
  jshPinSetValue(LCD_SPI_DC, 0); // command
  buffer1[0] = SPILCD_CMD_WINDOW_Y;
  jshSPISendMany(LCD_SPI, buffer1, NULL, 1, NULL);
  jshPinSetValue(LCD_SPI_DC, 1); // data
  buffer1[0] = 0;
  buffer1[1] = gfx->data.modMinY;
  buffer1[2] = 0;
  buffer1[3] = gfx->data.modMaxY+1;
  jshSPISendMany(LCD_SPI, buffer1, NULL, 4, NULL);
  jshPinSetValue(LCD_SPI_DC, 0); // command
  buffer1[0] = SPILCD_CMD_DATA;
  jshSPISendMany(LCD_SPI, buffer1, NULL, 1, NULL);
  jshPinSetValue(LCD_SPI_DC, 1); // data

  for (int y=gfx->data.modMinY;y<=gfx->data.modMaxY;y++) {
    unsigned char *buffer = (y&1)?buffer1:buffer2;
    // skip any lines that don't need updating
#if LCD_BPP==4
    unsigned char *px = &lcdBuffer[y*LCD_STRIDE + (xstart>>1)];
#endif
#if LCD_BPP==8
    unsigned char *px = &lcdBuffer[y*LCD_STRIDE + xstart];
#endif
    unsigned char *bufPtr = (unsigned char*)buffer;
    for (int x=0;x<xlen;x+=2) {
#if LCD_BPP==4
      unsigned char c = *(px++);
      unsigned int a = lcdPalette[c >> 4];
      unsigned int b = lcdPalette[c & 15];
#endif
#if LCD_BPP==8
      unsigned int a = lcdPalette[*(px++)];
      unsigned int b = lcdPalette[*(px++)];
#endif
      *(bufPtr++) = a>>4;
      *(bufPtr++) = (a<<4) | (b>>8);
      *(bufPtr++) = b;
    }
    size_t len = ((unsigned char*)bufPtr)-buffer;
    jshSPISendMany(LCD_SPI, buffer, 0, len, lcdFlip_SPILCD_callback);
  }
  jshSPIWait(LCD_SPI);
  jshPinSetValue(LCD_SPI_CS,1);
  // Reset modified-ness
  gfx->data.modMaxX = -32768;
  gfx->data.modMaxY = -32768;
  gfx->data.modMinX = 32767;
  gfx->data.modMinY = 32767;
}


void lcdInit_SPILCD(JsGraphics *gfx) {
  gfx->data.width = LCD_WIDTH;
  gfx->data.height = LCD_HEIGHT;
  gfx->data.bpp = LCD_BPP;

  lcdSetPalette_SPILCD(0);

  jshPinOutput(LCD_SPI_CS,1);
  jshPinOutput(LCD_SPI_DC,1);
  jshPinOutput(LCD_SPI_SCK,1);
  jshPinOutput(LCD_SPI_MOSI,1);
  jshPinOutput(LCD_SPI_RST,1);
  jshDelayMicroseconds(10000);
  jshPinOutput(LCD_SPI_RST, 1);
  jshDelayMicroseconds(10000);

  JshSPIInfo inf;
  jshSPIInitInfo(&inf);
  inf.baudRate = 8000000;
  inf.pinMOSI = LCD_SPI_MOSI;
  inf.pinSCK = LCD_SPI_SCK;
  jshSPISetup(LCD_SPI, &inf);

  lcdSendInitCmd_SPILCD();
}

void lcdSetCallbacks_SPILCD(JsGraphics *gfx) {
  gfx->setPixel = lcdSetPixel_SPILCD;
  gfx->getPixel = lcdGetPixel_SPILCD;
  //gfx->idle = lcdIdle_PCD8544;
}

