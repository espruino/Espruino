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
static const char ST7789_INIT_CODE[] = {
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
int lcdNextX, lcdNextY;

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

void lcdCmd_ST7789(int cmd, int dataLen, const char *data) {
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

void lcdSetPixel_ST7789(JsGraphics *gfx, short x, short y, unsigned int col) {
  LCD_CS_CLR();
  if ((y!=lcdNextY) || (x!=lcdNextX)) {
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
    LCD_WR8(0);
    LCD_WR8(y);
    LCD_WR8(0);
    LCD_WR8(LCD_HEIGHT-1);
    LCD_DC_COMMAND(); // command
    LCD_WR8(0x2C);
    lcdNextY=y;
    lcdNextX=x+1;
  } else
    lcdNextX++;
  LCD_DC_DATA(); // data
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

void lcdFillRect_ST7789(JsGraphics *gfx, short x1, short y1, short x2, short y2) {
  LCD_DC_COMMAND(); // command
  LCD_CS_CLR();
  LCD_WR8(0x2A);
  LCD_CS_SET();
  LCD_DC_DATA(); // data
  LCD_CS_CLR();
  LCD_WR8(0);
  LCD_WR8(x1);
  LCD_WR8(0);
  LCD_WR8(x2);
  LCD_CS_SET();
  LCD_DC_COMMAND(); // command
  LCD_CS_CLR();
  LCD_WR8(0x2B);
  LCD_CS_SET();
  LCD_DC_DATA(); // data
  LCD_CS_CLR();
  LCD_WR8(0);
  LCD_WR8(y1);
  LCD_WR8(0);
  LCD_WR8(y2);
  LCD_CS_SET();
  LCD_DC_COMMAND(); // command
  LCD_CS_CLR();
  LCD_WR8(0x2C);
  LCD_CS_SET();
  LCD_DC_DATA(); // data
  LCD_CS_CLR();
  lcdNextY=-1;
  lcdNextX=-1;
  unsigned int col = gfx->data.fgColor & 0xFFFF;
  int i = (1+x2-x1)*(1+y2-y1);
  if ((col&255) == (col>>8)) {
    // top and bottom bits are the same, we can just mash SCK
    LCD_DATA(col);
    asm("nop");asm("nop");
    while (i--) {
      LCD_SCK_CLR_FAST();LCD_SCK_SET_FAST();
      asm("nop");
      LCD_SCK_CLR_FAST();LCD_SCK_SET_FAST();
    }
  } else {
    // colors different, send manually
    while (i--) {
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

void lcdScroll_ST7789(JsGraphics *gfx, int xdir, int ydir) {
  // ignore for now - could be painful!
  // can vertically scroll in mem buffer
  // c=310;Strap.lcdWr(0x37,[c>>8,c])
}

void lcdInit_ST7789(JsGraphics *gfx) {
  assert(gfx->data.bpp == 1);

  // LCD pins need initialising and LCD needs reset beforehand - done in jswrap_hackstrap.c

  // Send initialization commands to ST7789
  const char *cmd = ST7789_INIT_CODE;
  while(cmd[CMDINDEX_DATALEN]!=255) {
    lcdCmd_ST7789(cmd[CMDINDEX_CMD], cmd[CMDINDEX_DATALEN], &cmd[3]);
    if (cmd[CMDINDEX_DELAY])
      jshDelayMicroseconds(1000*cmd[CMDINDEX_DELAY]);
    cmd += 3 + cmd[CMDINDEX_DATALEN];
  }

  lcdNextX = -1;
  lcdNextY = -1;
}

void lcdSetCallbacks_ST7789(JsGraphics *gfx) {
  gfx->data.width = LCD_WIDTH;
  gfx->data.height = LCD_HEIGHT;
  gfx->data.bpp = LCD_BPP;
  gfx->setPixel = lcdSetPixel_ST7789;
  gfx->fillRect = lcdFillRect_ST7789;
  gfx->scroll = lcdScroll_ST7789;
}

