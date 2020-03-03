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
#include "lcd_st7789_8bit.h"
#include "jswrap_graphics.h"
#ifndef EMSCRIPTEN
#include "jshardware.h"
#include "nrf_gpio.h"
#endif

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
    // https://github.com/espruino/Espruino/issues/1758
    //0xe0,0,14,0xd0,6,0xc,9,9,0x25,0x2e,0x33,0x45,0x36,0x12,0x12,0x2e,0x34,
    //0xe1,0,14,0xd0,6,0xc,9,9,0x25,0x2e,0x33,0x45,0x36,0x12,0x12,0x2e,0x34,
    0xe0,0,14,0x70,0x15,0x20,0x15,0x10,0x09,0x48,0x33,0x53,0x0B,0x19,0x15,0x2a,0x2f,
    0xe1,0,14,0x70,0x15,0x20,0x15,0x10,0x09,0x48,0x33,0x53,0x0B,0x19,0x15,0x2a,0x2f,
    0x29,0,0,
    0x21,0,0,
    // End
    0, 0, 255/*DATA_LEN = 255 => END*/
};
const int LCD_BUFFER_HEIGHT = 320;
LCDST7789Mode lcdMode;
int lcdNextX, lcdNextY;
/// Vertical scrolling (change the scan start address on the LCD)
int lcdScrollY;
/// For notifications, we keep lcdScroll the same but then shift the LCD start address anyway
int lcdOffsetY;

#ifdef EMSCRIPTEN
#define EMSCRIPTEN_GFX_STRIDE (240*2)
int EMSCRIPTEN_GFX_YSTART = 0;
char EMSCRIPTEN_GFX_BUFFER[240*320*2];
bool EMSCRIPTEN_GFX_CHANGED;
bool EMSCRIPTEN_GFX_WIDESCREEN = false; // are we 160px high, not 240?

int EMSCRIPTEN_GFX_BLIT_X;
int EMSCRIPTEN_GFX_BLIT_Y;
int EMSCRIPTEN_GFX_BLIT_X1;
int EMSCRIPTEN_GFX_BLIT_X2;
int EMSCRIPTEN_GFX_BLIT_Y1;
int EMSCRIPTEN_GFX_BLIT_Y2;
#endif

#ifndef EMSCRIPTEN
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
#endif // EMSCRIPTEN

void lcdST7789_cmd(int cmd, int dataLen, const uint8_t *data) {
#ifndef EMSCRIPTEN
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
#else
  if (cmd == 0x37) {
    EMSCRIPTEN_GFX_YSTART = (data[0]<<8)|data[1];
    if (EMSCRIPTEN_GFX_WIDESCREEN)
      EMSCRIPTEN_GFX_YSTART += 40;
    if (EMSCRIPTEN_GFX_YSTART>=320)
      EMSCRIPTEN_GFX_YSTART -= 320;
    EMSCRIPTEN_GFX_CHANGED = true;
  }
#endif
}

// Update LCD scroll position
void lcdST7789_scrollCmd() {
  int offs = lcdScrollY + lcdOffsetY;
  if (lcdMode == LCDST7789_MODE_DOUBLEBUFFERED)
    offs += 120;
  if (offs>=320) offs -= 320;
  if (offs<0) offs += 320;
  uint8_t buf[2];
  buf[0] = offs>>8;
  buf[1] = offs;
  lcdST7789_cmd(0x37,2,buf);
}

/** Allow the LCD to be shifted vertically while still drawing in the normal position.
 * Use this to display notifications while keeping the original data on the screen */
void lcdST7789_setYOffset(int y) {
  if (y<-80) y=-80;
  if (y>80) y=80;
  lcdOffsetY = y;
  lcdST7789_scrollCmd();
}

void lcdST7789_setMode(LCDST7789Mode mode) {
  if (lcdMode != mode) {
    uint8_t buf[4];
    lcdMode = mode;
    switch (lcdMode) {
    case LCDST7789_MODE_UNBUFFERED:
    case LCDST7789_MODE_BUFFER_120x120:
    case LCDST7789_MODE_BUFFER_80x80:
#ifdef EMSCRIPTEN
      EMSCRIPTEN_GFX_WIDESCREEN = false;
#endif
      lcdST7789_cmd(0x13,0,NULL);
      lcdScrollY = 0;
      lcdST7789_scrollCmd();
      break;
    case LCDST7789_MODE_DOUBLEBUFFERED:
#ifdef EMSCRIPTEN
      EMSCRIPTEN_GFX_WIDESCREEN = true;
#endif
      buf[0] = 0;
      buf[1] = 40;
      buf[2] = 0;
      buf[3] = 199;
      lcdST7789_cmd(0x30,4,buf);
      lcdST7789_cmd(0x12,0,NULL);
      lcdScrollY = 0;
      lcdST7789_scrollCmd();
      break;
    }
  }
}

LCDST7789Mode lcdST7789_getMode() {
  return lcdMode;
}

void lcdST7789_flip(JsGraphics *gfx) {
  unsigned char buf[2];
  switch (lcdMode) {
    case LCDST7789_MODE_UNBUFFERED:
      // unbuffered - flip has no effect
    break;
    case LCDST7789_MODE_DOUBLEBUFFERED: {
      // buffered - flip using LCD itself
      unsigned short offs;
      if (lcdScrollY==0) {
        lcdScrollY = 160;
      } else {
        lcdScrollY = 0;
      }
      lcdST7789_scrollCmd();
    } break;
    case LCDST7789_MODE_BUFFER_120x120: {
      // offscreen buffer - BLIT
      JsVar *buffer = jsvObjectGetChild(gfx->graphicsVar, "buffer", 0);
      size_t len = 0;
      unsigned char *dataPtr = (unsigned char*)jsvGetDataPointer(buffer, &len);
      jsvUnLock(buffer);
      if (dataPtr && len>=(120*120)) {
        // reset scroll to 0
        lcdScrollY = 0;
        lcdST7789_scrollCmd();
        // blit
        lcdST7789_blitStart(0,0,239,239);
        for (int y=0;y<240;y++) {
          for (int x=0;x<120;x++) {
            uint16_t c = PALETTE_8BIT[*(dataPtr++)];
            lcdST7789_blitPixel(c);
            lcdST7789_blitPixel(c);
          }
          // display the same row twice
          if (!(y&1)) dataPtr -= 120;
        }
        lcdST7789_blitEnd();
      }
    } break;
    case LCDST7789_MODE_BUFFER_80x80: {
      // offscreen buffer - BLIT
      JsVar *buffer = jsvObjectGetChild(gfx->graphicsVar, "buffer", 0);
      size_t len = 0;
      unsigned char *dataPtr = (unsigned char*)jsvGetDataPointer(buffer, &len);
      jsvUnLock(buffer);
      if (dataPtr && len>=(80*80)) {
        // reset scroll to 0
        lcdScrollY = 0;
        lcdST7789_scrollCmd();
        // blit
        lcdST7789_blitStart(0,0,239,239);
        for (int y=0;y<80;y++) {
          for (int n=0;n<3;n++) {
            for (int x=0;x<80;x++) {
              uint16_t c = PALETTE_8BIT[*(dataPtr++)];
              lcdST7789_blitPixel(c);
              lcdST7789_blitPixel(c);
              lcdST7789_blitPixel(c);
            }
            if (n<2) dataPtr -= 80;
          }
        }
        lcdST7789_blitEnd();
      }
    } break;
  }
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
#ifndef EMSCRIPTEN
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
#else
  EMSCRIPTEN_GFX_BLIT_X = x1;
  EMSCRIPTEN_GFX_BLIT_Y = y1;
  EMSCRIPTEN_GFX_BLIT_X1 = x1;
  EMSCRIPTEN_GFX_BLIT_Y1 = y1;
  EMSCRIPTEN_GFX_BLIT_X2 = x2;
  EMSCRIPTEN_GFX_BLIT_Y2 = y2;
#endif
}
inline void lcdST7789_blitPixel(unsigned int col) {
#ifndef EMSCRIPTEN
  /* FIXME: Handle case where scrolling means
   * we wrap around the memory area - see what
   * lcdST7789_fillRect does */
  LCD_DATA(col>>8);
  asm("nop");asm("nop");
  LCD_SCK_CLR_FAST();
  LCD_SCK_SET_FAST();
  LCD_DATA(col);
  asm("nop");asm("nop");
  LCD_SCK_CLR_FAST();
  LCD_SCK_SET_FAST();
#else
  ((uint16_t*)EMSCRIPTEN_GFX_BUFFER)[EMSCRIPTEN_GFX_BLIT_X+EMSCRIPTEN_GFX_BLIT_Y*240] = col;
  EMSCRIPTEN_GFX_CHANGED = true;

  EMSCRIPTEN_GFX_BLIT_X++;
  if (EMSCRIPTEN_GFX_BLIT_X>EMSCRIPTEN_GFX_BLIT_X2) {
    EMSCRIPTEN_GFX_BLIT_X = EMSCRIPTEN_GFX_BLIT_X1;
    EMSCRIPTEN_GFX_BLIT_Y++;
    if (EMSCRIPTEN_GFX_BLIT_Y>EMSCRIPTEN_GFX_BLIT_Y2) {
      EMSCRIPTEN_GFX_BLIT_Y = EMSCRIPTEN_GFX_BLIT_Y1;
    }
  }
#endif
}
void lcdST7789_blitEnd() {
#ifndef EMSCRIPTEN
  LCD_CS_SET();
#endif
}


void lcdST7789_setPixel(JsGraphics *gfx, int x, int y, unsigned int col) {
#ifndef EMSCRIPTEN
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
#else // EMSCRIPTEN
  y += lcdScrollY;
  if (y>=LCD_BUFFER_HEIGHT) y-=LCD_BUFFER_HEIGHT;
  ((uint16_t*)EMSCRIPTEN_GFX_BUFFER)[x+y*240] = col;
  EMSCRIPTEN_GFX_CHANGED = true;
#endif
}

#ifndef EMSCRIPTEN
void lcdST7789_fillRect(JsGraphics *gfx, int x1, int y1, int x2, int y2, unsigned int col) {
  y1 += lcdScrollY;
  if (y1>=LCD_BUFFER_HEIGHT) y1-=LCD_BUFFER_HEIGHT;
  y2 += lcdScrollY;
  if (y2>=LCD_BUFFER_HEIGHT) y2-=LCD_BUFFER_HEIGHT;
  // if scrolling meant we wrapped, we have to write it two separate calls
  int nexty = -1;
  if (y2<y1) {
    nexty = y2;
    y2 = LCD_BUFFER_HEIGHT-1;
  }
  // Output to the screen
  while (true) {
    int pixels = (1+x2-x1)*(1+y2-y1);
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
    // We might have to wrap over and start again
    if (nexty<0) return;
    y1 = 0;
    y2 = nexty;
    nexty = -1;
  }
}
#endif

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
  lcdST7789_scrollCmd();
}


// ====================================================================================
void lcdST7789buf_setPixel(JsGraphics *gfx, int x, int y, unsigned int col) {
  if (!gfx->backendData) return;
  ((uint8_t*)gfx->backendData)[x + y*gfx->data.width] = col;
}

unsigned int lcdST7789buf_getPixel(struct JsGraphics *gfx, int x, int y) {
  if (!gfx->backendData) return 0;
  return ((uint8_t*)gfx->backendData)[x + y*gfx->data.width];
}

void lcdST7789buf_scroll(JsGraphics *gfx, int xdir, int ydir) {
  if (!gfx->backendData) return;
  int pixels = -(xdir + ydir*gfx->data.width);
  int l = gfx->data.width*gfx->data.height;
  if (pixels>0) memcpy(&((uint8_t*)gfx->backendData)[0],&((uint8_t*)gfx->backendData)[pixels],l-pixels);
  else if (pixels<0) memcpy(&((uint8_t*)gfx->backendData)[-pixels],&((uint8_t*)gfx->backendData)[0],l+pixels);
}
// ====================================================================================

void lcdST7789_init(JsGraphics *gfx) {
  assert(gfx->data.bpp == 16);
  // LCD pins need initialising and LCD needs reset beforehand - done in jswrap_bangle.c

#ifndef EMSCRIPTEN
  // Send initialization commands to ST7789
  const uint8_t *cmd = ST7789_INIT_CODE;
  while(cmd[CMDINDEX_DATALEN]!=255) {
    lcdST7789_cmd(cmd[CMDINDEX_CMD], cmd[CMDINDEX_DATALEN], &cmd[3]);
    if (cmd[CMDINDEX_DELAY])
      jshDelayMicroseconds(1000*cmd[CMDINDEX_DELAY]);
    cmd += 3 + cmd[CMDINDEX_DATALEN];
  }
#endif

  lcdNextX = -1;
  lcdNextY = -1;
  lcdScrollY = 0;
  lcdOffsetY = 0;
  lcdMode = LCDST7789_MODE_UNBUFFERED;
}

void lcdST7789_setCallbacks(JsGraphics *gfx) {
  if (lcdMode==LCDST7789_MODE_NULL) {
    // nothing - ignores all draw commands!
  } else if (lcdMode==LCDST7789_MODE_BUFFER_120x120 ||
      lcdMode==LCDST7789_MODE_BUFFER_80x80) {
    size_t expectedLen = (lcdMode==LCDST7789_MODE_BUFFER_120x120) ? (120*120) : (80*80);
    JsVar *buf = jsvObjectGetChild(gfx->graphicsVar, "buffer", 0);
    size_t len = 0;
    char *dataPtr = jsvGetDataPointer(buf, &len);
    jsvUnLock(buf);
    if (dataPtr && len>=expectedLen) {
      gfx->backendData = dataPtr;
      gfx->setPixel = lcdST7789buf_setPixel;
      gfx->getPixel = lcdST7789buf_getPixel;
      // gfx->fillRect = lcdST7789buf_fillRect; // maybe later...
      gfx->scroll = lcdST7789buf_scroll;
    }
  } else {
    gfx->setPixel = lcdST7789_setPixel;
#ifndef EMSCRIPTEN
    gfx->fillRect = lcdST7789_fillRect;
#endif
    gfx->scroll = lcdST7789_scroll;
  }
}

