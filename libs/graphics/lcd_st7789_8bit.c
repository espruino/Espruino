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
 * Graphics Backend for drawing to ST7789
 * ----------------------------------------------------------------------------
 */

#include "platform_config.h"
#include "jsutils.h"
#include "jshardware.h"
#include "lcd_st7789_8bit.h"
#include "nrf_gpio.h"

#define CMDINDEX_CMD   0
#define CMDINDEX_DELAY 1
#define CMDINDEX_DATALEN  2
static const uint8_t ST7789_INIT_CODE[] = {
  // CMD,DELAY,DATA_LEN,D0,D1,D2...
    0x11,0,0,
    // This is an unrotated screen
    0x36,0,1,0, // MADCTL
    // These 2 rotate the screen by 180 degrees
    //0x36,0,1,0xC0, // MADCTL
    //0x37,0,2,0,80, // Vertical scroll

    0x3A,0,1,0x55, // COLMOD - interface pixel format - 16bpp
    0xB2,0,5,0xC,0xC,0,0x33,0x33,
    0xB7,0,1,0,
    0xBB,0,1,0x3E,
    0xC2,0,1,1,
    0xC3,0,1,0x19,
    0xC4,0,1,0x20,
    0xC5,0,1,0xF,
    0xD0,0,2,0xA4,0xA1,
    0xe0,0,14,0xd0,6,0xc,9,9,0x25,0x2e,0x33,0x45,0x36,0x12,0x12,0x2e,0x34,
    0xe1,0,14,0xd0,6,0xc,9,9,0x25,0x2e,0x33,0x45,0x36,0x12,0x12,0x2e,0x34,
    0x29,0,0,
    0x21,0,0,
    // End
    0, 0, 255/*DATA_LEN = 255 => END*/
};
const int LCD_BUFFER_HEIGHT = 320;
LCDST7789Mode lcdMode;
int lcdNextX, lcdNextY;
int lcdScrollY;


/* We have to be careful about this since we can
write faster than the LCD is happy about

#define LCD_SCK_CLR() NRF_P0->OUTCLR = 1<<LCD_PIN_SCK
#define LCD_SCK_SET() NRF_P0->OUTSET = 1<<LCD_PIN_SCK
#define LCD_CS_CLR() nrf_gpio_pin_clear(LCD_PIN_CS);
#define LCD_CS_SET() nrf_gpio_pin_set(LCD_PIN_CS);
#define LCD_DC_COMMAND() nrf_gpio_pin_clear(LCD_PIN_DC);
#define LCD_DC_DATA() nrf_gpio_pin_set(LCD_PIN_DC);
#define LCD_SCK_CLR() nrf_gpio_pin_clear(LCD_PIN_SCK);
#define LCD_SCK_SET() nrf_gpio_pin_set(LCD_PIN_SCK);
#define LCD_CS_CLR() jshPinSetValue(LCD_PIN_CS,0);
#define LCD_CS_SET() jshPinSetValue(LCD_PIN_CS,1);
#define LCD_DC_COMMAND() jshPinSetValue(LCD_PIN_DC,0);
#define LCD_DC_DATA() jshPinSetValue(LCD_PIN_DC,1);
*/
#define LCD_CS_CLR() NRF_P0->OUTCLR = 1<<LCD_PIN_CS
#define LCD_CS_SET() NRF_P0->OUTSET = 1<<LCD_PIN_CS
#define LCD_DC_COMMAND() NRF_P0->OUTCLR = 1<<LCD_PIN_DC
#define LCD_DC_DATA() NRF_P0->OUTSET = 1<<LCD_PIN_DC
#define LCD_SCK_CLR() jshPinSetValue(LCD_PIN_SCK,0);
#define LCD_SCK_SET() jshPinSetValue(LCD_PIN_SCK,1);
#define LCD_SCK_CLR_FAST() NRF_P0->OUTCLR = 1<<LCD_PIN_SCK
#define LCD_SCK_SET_FAST() NRF_P0->OUTSET = 1<<LCD_PIN_SCK
#define LCD_DATA(data) (*((uint8_t*)&NRF_P0->OUT)=data)
#define LCD_WR8(data) {LCD_DATA(data);LCD_SCK_CLR();LCD_SCK_SET();}


void lcd_send_cmd(uint8_t cmd) {
  LCD_CS_CLR();
  LCD_DC_COMMAND(); // command
  LCD_WR8(cmd);
  LCD_CS_SET();
}
void lcd_send_data(uint8_t cmd) {
  LCD_CS_CLR();
  LCD_DC_DATA(); // data
  LCD_WR8(cmd);
  LCD_CS_SET();
}

void lcdST7789_cmd(int cmd, int dataLen, const uint8_t *data) {
  LCD_CS_CLR();
  LCD_DC_COMMAND(); // command
  LCD_WR8(cmd);
  if (dataLen) {
    LCD_DC_DATA(); // data
    while (dataLen) {
      LCD_WR8(*(data++));
      dataLen--;
    }
  }
  LCD_CS_SET();
}

void lcdST7789_setMode(LCDST7789Mode mode) {
  if (lcdMode != mode) {
    uint8_t buf[4];
    lcdMode = mode;
    switch (lcdMode) {
    case LCDST7789_MODE_UNBUFFERED:
      lcdST7789_cmd(0x13,0,NULL);
      buf[0] = 0;
      buf[1] = 0;
      lcdST7789_cmd(0x37,2,buf);
      break;
    case LCDST7789_MODE_DOUBLEBUFFERED:
      buf[0] = 0;
      buf[1] = 40;
      buf[2] = 0;
      buf[3] = 199;
      lcdST7789_cmd(0x30,4,buf);
      lcdST7789_cmd(0x12,0,NULL);
      lcdScrollY = 0;
      buf[0] = 0;
      buf[1] = 120;
      lcdST7789_cmd(0x37,2,buf);
      break;
    }
  }
}

void lcdST7789_flip() {
  if (lcdMode!=LCDST7789_MODE_DOUBLEBUFFERED) return;
  unsigned short offs;
  if (lcdScrollY==0) {
    lcdScrollY = 160;
    offs = 280;
  } else {
    lcdScrollY = 0;
    offs = 120;
  }
  unsigned char buf[2];
  buf[0] = offs>>8;
  buf[1] = offs;
  lcdST7789_cmd(0x37,2,buf);
}

/// Starts a blit operation - call this, then blitPixel (a lot) then blitEnd. No bounds checking
void lcdST7789_blitStart(int x, int y, int w, int h) {
  lcdNextY=-1;
  lcdNextX=-1;
  int x1 = x;
  int y1 = y+lcdScrollY;
  if (y1>=LCD_BUFFER_HEIGHT) y1-=LCD_BUFFER_HEIGHT;
  int x2 = x+w;
  int y2 = y+h+lcdScrollY;
  if (y2>=LCD_BUFFER_HEIGHT) y2-=LCD_BUFFER_HEIGHT;
  y += lcdScrollY;
  LCD_CS_CLR();
  LCD_DC_COMMAND(); // command
  LCD_WR8(0x2A);
  LCD_DC_DATA(); // data
  LCD_WR8(0);
  LCD_WR8(x1);
  LCD_WR8(0);
  LCD_WR8(x2);
  LCD_DC_COMMAND(); // command
  LCD_WR8(0x2B);
  LCD_DC_DATA(); // data
  LCD_WR8(y1 >> 8);
  LCD_WR8(y1);
  LCD_WR8(y2 >> 8);
  LCD_WR8(y2);
  LCD_DC_COMMAND(); // command
  LCD_WR8(0x2C);
  LCD_DC_DATA(); // data
}
void lcdST7789_blitPixel(unsigned int col) {
  LCD_DATA(col>>8);
  asm("nop");asm("nop");
  LCD_SCK_CLR_FAST();
  LCD_SCK_SET_FAST();
  LCD_DATA(col);
  asm("nop");asm("nop");
  LCD_SCK_CLR_FAST();
  LCD_SCK_SET_FAST();
}
void lcdST7789_blitEnd() {
  LCD_CS_SET();
}


void lcdST7789_setPixel(JsGraphics *gfx, int x, int y, unsigned int col) {
  LCD_CS_CLR();
  if ((y!=lcdNextY) || (x!=lcdNextX)) {
    lcdNextY=y;
    lcdNextX=x+1;
    y += lcdScrollY;
    if (y>=LCD_BUFFER_HEIGHT) y-=LCD_BUFFER_HEIGHT;
    LCD_DC_COMMAND(); // command
    LCD_WR8(0x2A);
    LCD_DC_DATA(); // data
    LCD_WR8(0);
    LCD_WR8(x);
    LCD_WR8(0);
    LCD_WR8(LCD_WIDTH-1);
    LCD_DC_COMMAND(); // command
    LCD_WR8(0x2B);
    LCD_DC_DATA(); // data
    LCD_WR8(y>>8);
    LCD_WR8(y);
    LCD_WR8(y >> 8);
    LCD_WR8(y);
    LCD_DC_COMMAND(); // command
    LCD_WR8(0x2C);
    LCD_DC_DATA(); // data
  } else
    lcdNextX++;
  LCD_DATA(col>>8);
  asm("nop");asm("nop");
  LCD_SCK_CLR_FAST();
  LCD_SCK_SET_FAST();
  LCD_DATA(col);
  asm("nop");asm("nop");
  LCD_SCK_CLR_FAST();
  LCD_SCK_SET_FAST();
  LCD_CS_SET();
}

void lcdST7789_fillRect(JsGraphics *gfx, int x1, int y1, int x2, int y2, unsigned int col) {
  int pixels = (1+x2-x1)*(1+y2-y1);
  y1 += lcdScrollY;
  if (y1>=LCD_BUFFER_HEIGHT) y1-=LCD_BUFFER_HEIGHT;
  y2 += lcdScrollY;
  if (y2>=LCD_BUFFER_HEIGHT) y2-=LCD_BUFFER_HEIGHT;
  LCD_DC_COMMAND(); // command
  LCD_CS_CLR();
  LCD_WR8(0x2A);
  LCD_DC_DATA(); // data
  LCD_WR8(0);
  LCD_WR8(x1);
  LCD_WR8(0);
  LCD_WR8(x2);
  LCD_DC_COMMAND(); // command
  LCD_WR8(0x2B);
  LCD_DC_DATA(); // data
  LCD_WR8(y1>>8);
  LCD_WR8(y1);
  LCD_WR8(y2>>8);
  LCD_WR8(y2);
  LCD_DC_COMMAND(); // command
  LCD_WR8(0x2C);
  LCD_DC_DATA(); // data
  lcdNextY=-1;
  lcdNextX=-1;
  if ((col&255) == (col>>8)) {
    // top and bottom bits are the same, we can just mash SCK
    LCD_DATA(col);
    asm("nop");asm("nop");
    while (pixels--) {
      LCD_SCK_CLR_FAST();LCD_SCK_SET_FAST();
      asm("nop");
      LCD_SCK_CLR_FAST();LCD_SCK_SET_FAST();
    }
  } else {
    // colors different, send manually
    while (pixels--) {
      LCD_DATA(col>>8);
      asm("nop");asm("nop");
      LCD_SCK_CLR_FAST();LCD_SCK_SET_FAST();
      LCD_DATA(col);
      asm("nop");asm("nop");
      LCD_SCK_CLR_FAST();LCD_SCK_SET_FAST();
    }
  }
  LCD_CS_SET();
}

void lcdST7789_scroll(JsGraphics *gfx, int xdir, int ydir) {
  if (lcdMode != LCDST7789_MODE_UNBUFFERED) {
    // No way this is going to work double buffered!
    return;
  }
  /* We can't read data back, so we can't do left/right scrolling!
  However we can change our index in the memory buffer window
  which allows us to use the LCD itself for scrolling */
  lcdScrollY-=ydir;
  while (lcdScrollY<0) lcdScrollY+=LCD_BUFFER_HEIGHT;
  while (lcdScrollY>=LCD_BUFFER_HEIGHT) lcdScrollY-=LCD_BUFFER_HEIGHT;

  unsigned char buf[2];
  buf[0] = lcdScrollY>>8;
  buf[1] = lcdScrollY;
  lcdST7789_cmd(0x37, 2, buf);
}

void lcdST7789_init(JsGraphics *gfx) {
  assert(gfx->data.bpp == 16);
  // LCD pins need initialising and LCD needs reset beforehand - done in jswrap_bangle.c

  // Send initialization commands to ST7789
  const uint8_t *cmd = ST7789_INIT_CODE;
  while(cmd[CMDINDEX_DATALEN]!=255) {
    lcdST7789_cmd(cmd[CMDINDEX_CMD], cmd[CMDINDEX_DATALEN], &cmd[3]);
    if (cmd[CMDINDEX_DELAY])
      jshDelayMicroseconds(1000*cmd[CMDINDEX_DELAY]);
    cmd += 3 + cmd[CMDINDEX_DATALEN];
  }

  lcdNextX = -1;
  lcdNextY = -1;
  lcdScrollY = 0;
  lcdMode = LCDST7789_MODE_UNBUFFERED;
}

void lcdST7789_setCallbacks(JsGraphics *gfx) {
  gfx->setPixel = lcdST7789_setPixel;
  gfx->fillRect = lcdST7789_fillRect;
  gfx->scroll = lcdST7789_scroll;
}

