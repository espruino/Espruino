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

#include <jswrap_hackstrap.h>
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
#include "jsi2c.h" // touchscreen

#include "jswrap_graphics.h"
#include "lcd_arraybuffer.h"

#define LCD_SPI EV_SPI1
/*
Full screen refresh is:

12 bit 0.33 sec
16 bit 0.44 sec


a=[0x000000, 0x0000a8, 0x00a800, 0x00a8a8, 0xa80000, 0xa800a8, 0xa85400, 0xa8a8a8, 0x545454, 0x5454fc, 0x54fc54, 0x54fcfc, 0xfc5454, 0xfc54fc, 0xfcfc54, 0xfcfcfc, 0x000000, 0x141414, 0x202020, 0x2c2c2c, 0x383838, 0x444444, 0x505050, 0x606060, 0x707070, 0x808080, 0x909090, 0xa0a0a0, 0xb4b4b4, 0xc8c8c8, 0xe0e0e0, 0xfcfcfc, 0x0000fc, 0x4000fc, 0x7c00fc, 0xbc00fc, 0xfc00fc, 0xfc00bc, 0xfc007c, 0xfc0040, 0xfc0000, 0xfc4000, 0xfc7c00, 0xfcbc00, 0xfcfc00, 0xbcfc00, 0x7cfc00, 0x40fc00, 0x00fc00, 0x00fc40, 0x00fc7c, 0x00fcbc, 0x00fcfc, 0x00bcfc, 0x007cfc, 0x0040fc, 0x7c7cfc, 0x9c7cfc, 0xbc7cfc, 0xdc7cfc, 0xfc7cfc, 0xfc7cdc, 0xfc7cbc, 0xfc7c9c, 0xfc7c7c, 0xfc9c7c, 0xfcbc7c, 0xfcdc7c, 0xfcfc7c, 0xdcfc7c, 0xbcfc7c, 0x9cfc7c, 0x7cfc7c, 0x7cfc9c, 0x7cfcbc, 0x7cfcdc, 0x7cfcfc, 0x7cdcfc, 0x7cbcfc, 0x7c9cfc, 0xb4b4fc, 0xc4b4fc, 0xd8b4fc, 0xe8b4fc, 0xfcb4fc, 0xfcb4e8, 0xfcb4d8, 0xfcb4c4, 0xfcb4b4, 0xfcc4b4, 0xfcd8b4, 0xfce8b4, 0xfcfcb4, 0xe8fcb4, 0xd8fcb4, 0xc4fcb4, 0xb4fcb4, 0xb4fcc4, 0xb4fcd8, 0xb4fce8, 0xb4fcfc, 0xb4e8fc, 0xb4d8fc, 0xb4c4fc, 0x000070, 0x1c0070, 0x380070, 0x540070, 0x700070, 0x700054, 0x700038, 0x70001c, 0x700000, 0x701c00, 0x703800, 0x705400, 0x707000, 0x547000, 0x387000, 0x1c7000, 0x007000, 0x00701c, 0x007038, 0x007054, 0x007070, 0x005470, 0x003870, 0x001c70, 0x383870, 0x443870, 0x543870, 0x603870, 0x703870, 0x703860, 0x703854, 0x703844, 0x703838, 0x704438, 0x705438, 0x706038, 0x707038, 0x607038, 0x547038, 0x447038, 0x387038, 0x387044, 0x387054, 0x387060, 0x387070, 0x386070, 0x385470, 0x384470, 0x505070, 0x585070, 0x605070, 0x685070, 0x705070, 0x705068, 0x705060, 0x705058, 0x705050, 0x705850, 0x706050, 0x706850, 0x707050, 0x687050, 0x607050, 0x587050, 0x507050, 0x507058, 0x507060, 0x507068, 0x507070, 0x506870, 0x506070, 0x505870, 0x000040, 0x100040, 0x200040, 0x300040, 0x400040, 0x400030, 0x400020, 0x400010, 0x400000, 0x401000, 0x402000, 0x403000, 0x404000, 0x304000, 0x204000, 0x104000, 0x004000, 0x004010, 0x004020, 0x004030, 0x004040, 0x003040, 0x002040, 0x001040, 0x202040, 0x282040, 0x302040, 0x382040, 0x402040, 0x402038, 0x402030, 0x402028, 0x402020, 0x402820, 0x403020, 0x403820, 0x404020, 0x384020, 0x304020, 0x284020, 0x204020, 0x204028, 0x204030, 0x204038, 0x204040, 0x203840, 0x203040, 0x202840, 0x2c2c40, 0x302c40, 0x342c40, 0x3c2c40, 0x402c40, 0x402c3c, 0x402c34, 0x402c30, 0x402c2c, 0x40302c, 0x40342c, 0x403c2c, 0x40402c, 0x3c402c, 0x34402c, 0x30402c, 0x2c402c, 0x2c4030, 0x2c4034, 0x2c403c, 0x2c4040, 0x2c3c40, 0x2c3440, 0x2c3040, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0xFFFFFF];

// 12 bit
print(a.map(function(rgb) {
  r = rgb>>16;
  g = (rgb>>8)&0xFF;
  b = rgb&0xFF;
  return ((r&0xF0)<<4) | (g&0xF0) | (b>>4);
}).map(x=>"0x"+x.toString(16)).join(","))
// 16 bit
 print(a.map(function(rgb) {
  r = rgb>>16;
  g = (rgb>>8)&0xFF;
  b = rgb&0xFF;
  return ((r&0xF8)<<8) | ((g&0xFc)<<3) | (b>>3);
}).map(x=>"0x"+x.toString(16)).join(","))
 */
// 16 bit
//const uint16_t PALETTE[256] = { 0x0,0x15,0x540,0x555,0xa800,0xa815,0xaaa0,0xad55,0x52aa,0x52bf,0x57ea,0x57ff,0xfaaa,0xfabf,0xffea,0xffff,0x0,0x10a2,0x2104,0x2965,0x39c7,0x4228,0x528a,0x630c,0x738e,0x8410,0x9492,0xa514,0xb5b6,0xce59,0xe71c,0xffff,0x1f,0x401f,0x781f,0xb81f,0xf81f,0xf817,0xf80f,0xf808,0xf800,0xfa00,0xfbe0,0xfde0,0xffe0,0xbfe0,0x7fe0,0x47e0,0x7e0,0x7e8,0x7ef,0x7f7,0x7ff,0x5ff,0x3ff,0x21f,0x7bff,0x9bff,0xbbff,0xdbff,0xfbff,0xfbfb,0xfbf7,0xfbf3,0xfbef,0xfcef,0xfdef,0xfeef,0xffef,0xdfef,0xbfef,0x9fef,0x7fef,0x7ff3,0x7ff7,0x7ffb,0x7fff,0x7eff,0x7dff,0x7cff,0xb5bf,0xc5bf,0xddbf,0xedbf,0xfdbf,0xfdbd,0xfdbb,0xfdb8,0xfdb6,0xfe36,0xfed6,0xff56,0xfff6,0xeff6,0xdff6,0xc7f6,0xb7f6,0xb7f8,0xb7fb,0xb7fd,0xb7ff,0xb75f,0xb6df,0xb63f,0xe,0x180e,0x380e,0x500e,0x700e,0x700a,0x7007,0x7003,0x7000,0x70e0,0x71c0,0x72a0,0x7380,0x5380,0x3b80,0x1b80,0x380,0x383,0x387,0x38a,0x38e,0x2ae,0x1ce,0xee,0x39ce,0x41ce,0x51ce,0x61ce,0x71ce,0x71cc,0x71ca,0x71c8,0x71c7,0x7227,0x72a7,0x7307,0x7387,0x6387,0x5387,0x4387,0x3b87,0x3b88,0x3b8a,0x3b8c,0x3b8e,0x3b0e,0x3aae,0x3a2e,0x528e,0x5a8e,0x628e,0x6a8e,0x728e,0x728d,0x728c,0x728b,0x728a,0x72ca,0x730a,0x734a,0x738a,0x6b8a,0x638a,0x5b8a,0x538a,0x538b,0x538c,0x538d,0x538e,0x534e,0x530e,0x52ce,0x8,0x1008,0x2008,0x3008,0x4008,0x4006,0x4004,0x4002,0x4000,0x4080,0x4100,0x4180,0x4200,0x3200,0x2200,0x1200,0x200,0x202,0x204,0x206,0x208,0x188,0x108,0x88,0x2108,0x2908,0x3108,0x3908,0x4108,0x4107,0x4106,0x4105,0x4104,0x4144,0x4184,0x41c4,0x4204,0x3a04,0x3204,0x2a04,0x2204,0x2205,0x2206,0x2207,0x2208,0x21c8,0x2188,0x2148,0x2968,0x3168,0x3168,0x3968,0x4168,0x4167,0x4166,0x4166,0x4165,0x4185,0x41a5,0x41e5,0x4205,0x3a05,0x3205,0x3205,0x2a05,0x2a06,0x2a06,0x2a07,0x2a08,0x29e8,0x29a8,0x2988,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xffff };
// 12 bit
const uint16_t PALETTE[256] = { 0x0,0xa,0xa0,0xaa,0xa00,0xa0a,0xa50,0xaaa,0x555,0x55f,0x5f5,0x5ff,0xf55,0xf5f,0xff5,0xfff,0x0,0x111,0x222,0x222,0x333,0x444,0x555,0x666,0x777,0x888,0x999,0xaaa,0xbbb,0xccc,0xeee,0xfff,0xf,0x40f,0x70f,0xb0f,0xf0f,0xf0b,0xf07,0xf04,0xf00,0xf40,0xf70,0xfb0,0xff0,0xbf0,0x7f0,0x4f0,0xf0,0xf4,0xf7,0xfb,0xff,0xbf,0x7f,0x4f,0x77f,0x97f,0xb7f,0xd7f,0xf7f,0xf7d,0xf7b,0xf79,0xf77,0xf97,0xfb7,0xfd7,0xff7,0xdf7,0xbf7,0x9f7,0x7f7,0x7f9,0x7fb,0x7fd,0x7ff,0x7df,0x7bf,0x79f,0xbbf,0xcbf,0xdbf,0xebf,0xfbf,0xfbe,0xfbd,0xfbc,0xfbb,0xfcb,0xfdb,0xfeb,0xffb,0xefb,0xdfb,0xcfb,0xbfb,0xbfc,0xbfd,0xbfe,0xbff,0xbef,0xbdf,0xbcf,0x7,0x107,0x307,0x507,0x707,0x705,0x703,0x701,0x700,0x710,0x730,0x750,0x770,0x570,0x370,0x170,0x70,0x71,0x73,0x75,0x77,0x57,0x37,0x17,0x337,0x437,0x537,0x637,0x737,0x736,0x735,0x734,0x733,0x743,0x753,0x763,0x773,0x673,0x573,0x473,0x373,0x374,0x375,0x376,0x377,0x367,0x357,0x347,0x557,0x557,0x657,0x657,0x757,0x756,0x756,0x755,0x755,0x755,0x765,0x765,0x775,0x675,0x675,0x575,0x575,0x575,0x576,0x576,0x577,0x567,0x567,0x557,0x4,0x104,0x204,0x304,0x404,0x403,0x402,0x401,0x400,0x410,0x420,0x430,0x440,0x340,0x240,0x140,0x40,0x41,0x42,0x43,0x44,0x34,0x24,0x14,0x224,0x224,0x324,0x324,0x424,0x423,0x423,0x422,0x422,0x422,0x432,0x432,0x442,0x342,0x342,0x242,0x242,0x242,0x243,0x243,0x244,0x234,0x234,0x224,0x224,0x324,0x324,0x324,0x424,0x423,0x423,0x423,0x422,0x432,0x432,0x432,0x442,0x342,0x342,0x342,0x242,0x243,0x243,0x243,0x244,0x234,0x234,0x234,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xfff};

/*JSON{
    "type": "class",
    "class" : "ID"
}
Class containing utility functions for [ID205](http://www.espruino.com/ID205)
*/


/*JSON{
  "type" : "variable",
  "name" : "VIBRATE",
  "generate_full" : "8",
  "ifdef" : "Strap",
  "return" : ["pin",""]
}
The ID205's vibration motor.
*/

IOEventFlags touchWatchChannel;
JshI2CInfo internalI2C;
uint8_t touchX, touchY;
bool touchDown, touchWasDown, touchHadEvent;


void lcd_cmd(int cmd, int dataLen, const char *data) {
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

void lcd_flip_spi_callback() {
  // just an empty stub - we'll just push data as fast as we can
}

void lcd_flip_gfx(JsGraphics *gfx) {
  if (gfx->data.modMinX > gfx->data.modMaxX) return; // nothing to do!

  JsVar *buf = jsvObjectGetChild(gfx->graphicsVar,"buffer",0);
  if (!buf) return;
  JSV_GET_AS_CHAR_ARRAY(bPtr, bLen, buf);
  if (!bPtr || bLen<128*8) return;

  unsigned char buffer1[LCD_WIDTH*2]; // 16 bits per pixel
  unsigned char buffer2[LCD_WIDTH*2]; // 16 bits per pixel

  // use nearest 2 pixels as we're sending 12 bits
  gfx->data.modMinX = gfx->data.modMinX&~1;
  gfx->data.modMaxX = (gfx->data.modMaxX+1)&~1;
  int xlen = gfx->data.modMaxX - gfx->data.modMinX;

  jshPinSetValue(LCD_SPI_CS, 0);
  jshPinSetValue(LCD_SPI_DC, 0); // command
  buffer1[0] = 0x2A;
  jshSPISendMany(LCD_SPI, buffer1, NULL, 1, NULL);
  jshPinSetValue(LCD_SPI_DC, 1); // data
  buffer1[0] = 0;
  buffer1[1] = gfx->data.modMinX;
  buffer1[2] = 0;
  buffer1[3] = gfx->data.modMaxX-1;
  jshSPISendMany(LCD_SPI, buffer1, NULL, 4, NULL);
  jshPinSetValue(LCD_SPI_DC, 0); // command
  buffer1[0] = 0x2B;
  jshSPISendMany(LCD_SPI, buffer1, NULL, 1, NULL);
  jshPinSetValue(LCD_SPI_DC, 1); // data
  buffer1[0] = 0;
  buffer1[1] = gfx->data.modMinY;
  buffer1[2] = 0;
  buffer1[3] = gfx->data.modMaxY+1;
  jshSPISendMany(LCD_SPI, buffer1, NULL, 4, NULL);
  jshPinSetValue(LCD_SPI_DC, 0); // command
  buffer1[0] = 0x2C;
  jshSPISendMany(LCD_SPI, buffer1, NULL, 1, NULL);
  jshPinSetValue(LCD_SPI_DC, 1); // data

  for (int y=gfx->data.modMinY;y<=gfx->data.modMaxY;y++) {
    unsigned char *buffer = (y&1)?buffer1:buffer2;
    // skip any lines that don't need updating
    unsigned char *px = (unsigned char *)&bPtr[y*LCD_WIDTH + gfx->data.modMinX];
    unsigned char *bufPtr = (uint16_t*)buffer;
    for (int x=0;x<xlen;x+=2) {
      unsigned int a = PALETTE[*(px++)];
      unsigned int b = PALETTE[*(px++)];
      *(bufPtr++) = a>>4;
      *(bufPtr++) = (a<<4) | (b>>8);
      *(bufPtr++) = b;
    }
    size_t len = ((unsigned char*)bufPtr)-buffer;
    if (len>255) {
      // do biggest bit of buffer last so we can get on and work
      // out the next scanline while it's sent out
      jshSPISendMany(LCD_SPI, buffer, 0, len-254, NULL);
      jshSPISendMany(LCD_SPI, &buffer[len-254], 0, 254, lcd_flip_spi_callback);
    } else {
      jshSPISendMany(LCD_SPI, buffer, 0, len, lcd_flip_spi_callback);
    }
  }
  jshSPIWait(LCD_SPI);
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
    gfx.data.modMaxX = LCD_WIDTH-1;
    gfx.data.modMaxY = LCD_HEIGHT-1;
  }
  lcd_flip_gfx(&gfx);
  graphicsSetVar(&gfx);
}

/*JSON{
    "type" : "staticmethod",
    "class" : "ID",
    "name" : "setLCDPower",
    "generate" : "jswrap_hackstrap_setLCDPower",
    "params" : [
      ["isOn","bool","True if the LCD should be on, false if not"]
    ]
}
This function can be used to turn ID205's LCD off or on.
*/
void jswrap_hackstrap_setLCDPower(bool isOn) {
  if (isOn) {
    lcd_cmd(0x11,0, NULL); // SLPOUT
    jshPinOutput(LCD_BL,0); // backlight
  } else {
    lcd_cmd(0x10,0, NULL); // SLPIN
    jshPinOutput(LCD_BL,1); // backlight
  }
}

/*JSON{
    "type" : "staticmethod",
    "class" : "ID",
    "name" : "lcdw",
    "generate" : "jswrap_hackstrap_lcdw",
    "params" : [
      ["cmd","int",""],
      ["data","JsVar",""]
    ]
}
Writes a command directly to the ST7567 LCD controller
*/
void jswrap_hackstrap_lcdw(JsVarInt cmd, JsVar *data) {
  JSV_GET_AS_CHAR_ARRAY(dPtr, dLen, data);
  lcd_cmd(cmd, dLen, dPtr);
}

// Holding down both buttons will reboot
void watchdogHandler() {
  if (!(jshPinGetValue(BTN1_PININDEX) && jshPinGetValue(BTN2_PININDEX)))
    jshKickWatchDog();
}

void DELAY_MS(int d) {
  jshDelayMicroseconds(1000*d);
}
void LCD_SPI_CMD(int d) {
  jshPinSetValue(LCD_SPI_CS, 0);
  jshPinSetValue(LCD_SPI_DC, 0); // command
  jshSPISend(LCD_SPI, d);
  jshPinSetValue(LCD_SPI_CS, 1);
}
void LCD_SPI_DATA(int d) {
  jshPinSetValue(LCD_SPI_CS, 0);
  jshPinSetValue(LCD_SPI_DC, 1); // data
  jshSPISend(LCD_SPI, d);
  jshPinSetValue(LCD_SPI_CS, 1);
}

typedef struct  {
  uint8_t command; // ST7735 command byte
  uint8_t delay; // ms delay after
  uint8_t len; // length of parameter data
  uint8_t data[16]; // parameter data
} ST7735_cmdBuf;

#define ST7735_CASET 0x2A
#define ST7735_RASET 0x2B
#define ST7735_MADCTL 0x36
#define ST7735_RAMWR 0x2C
#define ST7735_RAMRD 0x2E
#define ST7735_COLMOD 0x3A
static const ST7735_cmdBuf ST7735_init[] = {
  // SWRESET Software reset
  { 0x01, 150, 0, {0}},
  // SLPOUT Leave sleep mode
  { 0x11, 150, 0, {0}},
  // FRMCTR1 , FRMCTR2 Frame Rate configuration -- Normal mode, idle
  // frame rate = fosc / (1 x 2 + 40) * (LINE + 2C + 2D)
  { 0xB1, 0, 3, { 0x01, 0x2C, 0x2D }},
  { 0xB2, 0, 3, { 0x01, 0x2C, 0x2D }},
  // FRMCTR3 Frame Rate configureation -- partial mode
  { 0xB3, 0, 6, { 0x01, 0x2C, 0x2D, 0x01, 0x2C, 0x2D }},
  // INVCTR Display inversion (no inversion)
  { 0xB4, 0, 1, { 0x07 }},
  // PWCTR1 Power control -4.6V, Auto mode
  { 0xC0, 0, 3, { 0xA2, 0x02, 0x84}},
  // PWCTR2 Power control VGH25 2.4C, VGSEL -10, VGH = 3 * AVDD
  { 0xC1, 0, 1, { 0xC5}},
  // PWCTR3 Power control , opamp current smal, boost frequency
  { 0xC2, 0, 2, { 0x0A, 0x00 }},
  // PWCTR4 Power control , BLK/2, opamp current small and medium low
  { 0xC3, 0, 2, { 0x8A, 0x2A}},
  // PWRCTR5 , VMCTR1 Power control
  { 0xC4, 0, 2, { 0x8A, 0xEE}},
  { 0xC5, 0, 1, { 0x0E }},
  // INVOFF Don't invert display
  { 0x20, 0, 0, {0}},
  // Memory access directions. row address/col address , bottom to top refesh (10.1.27)
  { ST7735_MADCTL , 0, 1, {0xC8}},
  // Color mode 12 bit
  { ST7735_COLMOD , 0, 1, {0x03}},
  // Column address set 0..127
  { ST7735_CASET , 0, 4, {0x00, 0x00, 0x00, 0x7F }},
  // Row address set 0..159
  { ST7735_RASET , 0, 4, {0x00, 0x00, 0x00, 0x9F }},
  // GMCTRP1 Gamma correction
  { 0xE0, 0, 16, {0x02, 0x1C, 0x07, 0x12, 0x37, 0x32, 0x29, 0x2D, 0x29, 0x25, 0x2B, 0x39, 0x00, 0x01, 0x03, 0x10 }},
  // GMCTRP2 Gamma Polarity corrction
  { 0xE1, 0, 16, {0x03, 0x1d, 0x07, 0x06, 0x2E, 0x2C, 0x29, 0x2D, 0x2E, 0x2E, 0x37, 0x3F, 0x00, 0x00, 0x02, 0x10 }},
  // DISPON Display on
  { 0x29, 100, 0, {0}},
  // NORON Normal on
  { 0x13, 10, 0, {0}},
  // End
  { 0, 0, 0, {0}}
};

/*JSON{
  "type" : "init",
  "generate" : "jswrap_hackstrap_init"
}*/
void jswrap_hackstrap_init() {
  // Add watchdog timer to ensure watch always stays usable (hopefully!)
  jshEnableWatchDog(10); // 10 second watchdog
  JsSysTime t = jshGetTimeFromMilliseconds(4000);
  jstExecuteFn(watchdogHandler, NULL, jshGetSystemTime()+t, t);

  jshPinOutput(0,1); // GPS off

  jshPinOutput(LCD_BL,0); // backlight
  // LCD Init 1
  jshPinOutput(LCD_SPI_CS,1);
  jshPinOutput(LCD_SPI_DC,1);
  jshPinOutput(LCD_SPI_SCK,1);
  jshPinOutput(LCD_SPI_MOSI,1);
  jshPinOutput(LCD_SPI_RST,1);
  jshDelayMicroseconds(10000);
  jshPinOutput(LCD_SPI_RST, 1);
  jshDelayMicroseconds(10000);
  // Create backing graphics for LCD
  JsVar *graphics = jspNewObject(0, "Graphics");
  if (!graphics) return; // low memory
  JsGraphics gfx;
  graphicsStructInit(&gfx);
  gfx.data.type = JSGRAPHICSTYPE_ARRAYBUFFER;
  gfx.data.flags = JSGRAPHICSFLAGS_INVERT_X | JSGRAPHICSFLAGS_INVERT_Y;
  gfx.graphicsVar = graphics;
  gfx.data.width = LCD_WIDTH;
  gfx.data.height = LCD_HEIGHT;
  gfx.data.bpp = LCD_BPP;

  //gfx.data.fontSize = JSGRAPHICS_FONTSIZE_6X8;
  lcdInit_ArrayBuffer(&gfx);
  graphicsSetVar(&gfx);
  jsvObjectSetChild(execInfo.root, "g", graphics);
  jsvObjectSetChild(execInfo.hiddenRoot, JS_GRAPHICS_VAR, graphics);
  graphicsGetFromVar(&gfx, graphics);


  // Create 'flip' fn
  JsVar *fn;
  fn = jsvNewNativeFunction((void (*)(void))lcd_flip, JSWAT_VOID|JSWAT_THIS_ARG|(JSWAT_BOOL << (JSWAT_BITS*1)));
  jsvObjectSetChildAndUnLock(graphics,"flip",fn);

  JshSPIInfo inf;
  jshSPIInitInfo(&inf);
  inf.baudRate = 8000000;
  inf.pinMOSI = LCD_SPI_MOSI;
  inf.pinSCK = LCD_SPI_SCK;
  jshSPISetup(LCD_SPI, &inf);

  // Send initialization commands to ST7735
  const ST7735_cmdBuf *cmd;
  for (cmd = ST7735_init; cmd->command; cmd++) {
    lcd_cmd(cmd->command, cmd->len, cmd->data);
  }

  /* If the button is pressed during reset, perform a self test.
   * With bootloader this means apply power while holding button for >3 secs */
  static bool firstStart = true;

  graphicsClear(&gfx);
  int h=6;
  jswrap_graphics_drawCString(&gfx,0,h*1," ____                 _ ");
  jswrap_graphics_drawCString(&gfx,0,h*2,"|  __|___ ___ ___ _ _|_|___ ___ ");
  jswrap_graphics_drawCString(&gfx,0,h*3,"|  __|_ -| . |  _| | | |   | . |");
  jswrap_graphics_drawCString(&gfx,0,h*4,"|____|___|  _|_| |___|_|_|_|___|");
  jswrap_graphics_drawCString(&gfx,0,h*5,"         |_| espruino.com");
  jswrap_graphics_drawCString(&gfx,0,h*6," "JS_VERSION" (c) 2019 G.Williams");
  // Write MAC address in bottom right
  JsVar *addr = jswrap_ble_getAddress();
  char buf[20];
  jsvGetString(addr, buf, sizeof(buf));
  jsvUnLock(addr);
  jswrap_graphics_drawCString(&gfx,(LCD_WIDTH-1)-strlen(buf)*6,h*8,buf);
  lcd_flip_gfx(&gfx);

/*
  if (firstStart && (jshPinGetValue(BTN1_PININDEX) == BTN1_ONSTATE || jshPinGetValue(BTN4_PININDEX) == BTN4_ONSTATE)) {
    // don't do it during a software reset - only first hardware reset
    jsiConsolePrintf("SELF TEST\n");
    if (pixl_selfTest()) jsiConsolePrintf("Test passed!\n");
  }*/

  // If the button is *still* pressed, remove all code from flash memory too!
  /*if (firstStart && jshPinGetValue(BTN1_PININDEX) == BTN1_ONSTATE) {
    jsfRemoveCodeFromFlash();
    jsiConsolePrintf("Removed saved code from Flash\n");
  }*/
  graphicsSetVar(&gfx);

  firstStart = false;
  jsvUnLock(graphics);

  // Setup touchscreen I2C
  jshI2CInitInfo(&internalI2C);
  internalI2C.pinSDA = 1;
  internalI2C.pinSCL = 2;
  jshPinSetValue(internalI2C.pinSCL, 1);
  jshPinSetState(internalI2C.pinSCL,  JSHPINSTATE_GPIO_OUT_OPENDRAIN_PULLUP);
  jshPinSetValue(internalI2C.pinSDA, 1);
  jshPinSetState(internalI2C.pinSDA,  JSHPINSTATE_GPIO_OUT_OPENDRAIN_PULLUP);

}

/*JSON{
  "type" : "kill",
  "generate" : "jswrap_hackstrap_kill"
}*/
void jswrap_hackstrap_kill() {

}

/*JSON{
  "type" : "idle",
  "generate" : "jswrap_hackstrap_idle"
}*/
bool jswrap_hackstrap_idle() {

  return false;
}

/*JSON{
  "type" : "event",
  "class" : "ID",
  "name" : "touchDown",
  "ifdef" : "Strap"
}
 */
/*JSON{
  "type" : "event",
  "class" : "ID",
  "name" : "touchUp",
  "ifdef" : "Strap"
}
 */
/*JSON{
  "type" : "event",
  "class" : "ID",
  "name" : "touchMove",
  "ifdef" : "Strap"
}
 */
