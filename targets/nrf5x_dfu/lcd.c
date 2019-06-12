/**
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2017 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * Super small LCD driver for Pixl.js
 * ----------------------------------------------------------------------------
 */
#include "platform_config.h"

#ifdef LCD_CONTROLLER_ST7567 // Pixl
#define LCD
#endif
#ifdef LCD_CONTROLLER_ST7789V // iD205
#define LCD
#endif

#ifdef LCD
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "lcd.h"
#include "jspininfo.h"

#define ___ 0
#define __X 1
#define _X_ 2
#define _XX 3
#define X__ 4
#define X_X 5
#define XX_ 6
#define XXX 7
#define PACK_5_TO_16(A,B,C,D,E) ((A) | (B<<3) | (C<<6) | (D<<9) | (E<<12))
 // 48

#define LCD_FONT_3X5_CHARS 95
const unsigned short LCD_FONT_3X5[] = { // from 33 up to 127
    PACK_5_TO_16( XXX , _X_ , XXX , XX_ , X_X ), // 01234
    PACK_5_TO_16( X_X , XX_ , __X , __X , X_X ),
    PACK_5_TO_16( X_X , _X_ , _X_ , XX_ , XXX ),
    PACK_5_TO_16( X_X , _X_ , X__ , __X , __X ),
    PACK_5_TO_16( XXX , XXX , XXX , XX_ , __X ),

    PACK_5_TO_16( XXX , XXX , XXX , XXX , XXX ), // 56789
    PACK_5_TO_16( X__ , X__ , __X , X_X , X_X ),
    PACK_5_TO_16( XXX , XXX , _X_ , XXX , XXX ),
    PACK_5_TO_16( __X , X_X , _X_ , X_X , __X ),
    PACK_5_TO_16( XXX , XXX , _X_ , XXX , XXX ),

    PACK_5_TO_16( ___ , ___ , __X , ___ , X__ ), // :;<=>
    PACK_5_TO_16( _X_ , _X_ , _X_ , XXX , _X_ ),
    PACK_5_TO_16( ___ , ___ , X__ , ___ , __X ),
    PACK_5_TO_16( _X_ , _X_ , _X_ , XXX , _X_ ),
    PACK_5_TO_16( ___ , X__ , __X , ___ , X__ ),

    PACK_5_TO_16( _X_ , _X_ , _X_ , XX_ , _XX ), // ?@ABC
    PACK_5_TO_16( X_X , X_X , X_X , X_X , X__ ),
    PACK_5_TO_16( __X , XXX , XXX , XX_ , X__ ),
    PACK_5_TO_16( ___ , X_X , X_X , X_X , X__ ),
    PACK_5_TO_16( _X_ , _XX , X_X , XX_ , _XX ),

    PACK_5_TO_16( XX_ , XXX , XXX , _XX , X_X ), // DEFGH
    PACK_5_TO_16( X_X , X__ , X__ , X__ , X_X ),
    PACK_5_TO_16( X_X , XX_ , XXX , X_X , XXX ),
    PACK_5_TO_16( X_X , X__ , X__ , X_X , X_X ),
    PACK_5_TO_16( XX_ , XXX , X__ , _XX , X_X ),

    PACK_5_TO_16( XXX , XXX , X_X , X__ , X_X ), // IJKLM
    PACK_5_TO_16( _X_ , __X , X_X , X__ , XXX ),
    PACK_5_TO_16( _X_ , __X , XX_ , X__ , XXX ),
    PACK_5_TO_16( _X_ , __X , X_X , X__ , X_X ),
    PACK_5_TO_16( XXX , XX_ , X_X , XXX , X_X ),

    PACK_5_TO_16( XX_ , _XX , XX_ , _X_ , XX_ ), // NOPQR
    PACK_5_TO_16( X_X , X_X , X_X , X_X , X_X ),
    PACK_5_TO_16( X_X , X_X , XX_ , X_X , XX_ ),
    PACK_5_TO_16( X_X , X_X , X__ , X_X , X_X ),
    PACK_5_TO_16( X_X , _X_ , X__ , _XX , X_X ),

    PACK_5_TO_16( _XX , XXX , X_X , X_X , X_X ), // STUVW
    PACK_5_TO_16( X__ , _X_ , X_X , X_X , X_X ),
    PACK_5_TO_16( _X_ , _X_ , X_X , X_X , XXX ),
    PACK_5_TO_16( __X , _X_ , X_X , _X_ , XXX ),
    PACK_5_TO_16( XX_ , _X_ , _X_ , _X_ , X_X ),

    PACK_5_TO_16( X_X , X_X , XXX , _XX , ___ ), // XYZ[
    PACK_5_TO_16( X_X , X_X , __X , _X_ , ___ ),
    PACK_5_TO_16( _X_ , _X_ , _X_ , _X_ , ___ ),
    PACK_5_TO_16( X_X , _X_ , X__ , _X_ , ___ ),
    PACK_5_TO_16( X_X , _X_ , XXX , _XX , _X_ ),
};

int lcdx = 0, lcdy = 0;

void jshPinSetValue(Pin pin, bool value) {
  nrf_gpio_pin_write((uint32_t)pinInfo[pin].pin, value);
}
void jshPinOutput(Pin pin, bool value) {
  nrf_gpio_pin_write((uint32_t)pinInfo[pin].pin, value);
  nrf_gpio_cfg(
      (uint32_t)pinInfo[pin].pin,
      NRF_GPIO_PIN_DIR_OUTPUT,
      NRF_GPIO_PIN_INPUT_DISCONNECT,
      NRF_GPIO_PIN_NOPULL,
      NRF_GPIO_PIN_H0H1,
      NRF_GPIO_PIN_NOSENSE);
}

#ifdef LCD_CONTROLLER_ST7567
char lcd_data[128*8];

void lcd_pixel(int x, int y) {
  lcd_data[x+((y>>3)<<7)] |= 1<<(y&7);
}

void lcd_wr(int data) {
  int bit;
  for (bit=7;bit>=0;bit--) {
    jshPinSetValue(LCD_SPI_MOSI, (data>>bit)&1 );
    jshPinSetValue(LCD_SPI_SCK, 1 );
    jshPinSetValue(LCD_SPI_SCK, 0 );
  }
}
#endif
#ifdef LCD_CONTROLLER_ST7789V
#define LCD_ROWSTRIDE (120>>3)
char lcd_data[LCD_ROWSTRIDE*120];
int ymin=0,ymax=119;

void lcd_pixel(int x, int y) {
  lcd_data[(x>>3)+(y*LCD_ROWSTRIDE)] |= 1<<(x&7);
  if (y<ymin) ymin=y;
  if (y>ymax) ymax=y;
}

void lcd_wr(int data) {
  int bit;
  for (bit=7;bit>=0;bit--) {
    jshPinSetValue(LCD_SPI_SCK, 0 );
    jshPinSetValue(LCD_SPI_MOSI, (data>>bit)&1 );
    jshPinSetValue(LCD_SPI_SCK, 1 );
  }
}
#endif

void lcd_char(int x1, int y1, char ch) {
  if (ch=='.') ch='\\';
  int idx = ch - '0';
  if (idx<0 || idx>=LCD_FONT_3X5_CHARS) return; // no char for this - just return
  int cidx = idx % 5; // character index
  idx -= cidx;
  int y;
  for (y=0;y<5;y++) {
    unsigned short line = LCD_FONT_3X5[idx + y] >> (cidx*3);
    if (line&4) lcd_pixel(x1+0, y+y1);
    if (line&2) lcd_pixel(x1+1, y+y1);
    if (line&1) lcd_pixel(x1+2, y+y1);
  }
}




void lcd_flip();

void lcd_print(char *ch) {
  while (*ch) {
    lcd_char(lcdx,lcdy,*ch);
    if ('\n'==*ch) {
      lcdy += 6;
      if (lcdy>=60) {
        // scroll - FIXME FOR OTHER LCDs
        memcpy(lcd_data,&lcd_data[128],128*7);
        memset(&lcd_data[128*7],0,128);
        lcdy-=8;
        ymin=0;
        ymax=LCD_HEIGHT-1;
      }
    } else if ('\r'==*ch) {
      lcdx = 0;
    } else lcdx += 4;
    ch++;
  }
  lcd_flip();
}

#ifdef LCD_CONTROLLER_ST7567

void lcd_flip() {
  jshPinSetValue(LCD_SPI_CS,0);
  for (int y=0;y<8;y++) {
    // Send only what we need
    jshPinSetValue(LCD_SPI_DC,0);
    lcd_wr(0xB0|y/* page */);
    lcd_wr(0x00/* x lower*/);
    lcd_wr(0x10/* x upper*/);
    jshPinSetValue(LCD_SPI_DC,1);

    char *px = &lcd_data[y*128];
    for (int x=0;x<128;x++)
      lcd_wr(*(px++));
  }
  jshPinSetValue(LCD_SPI_CS,1);
}

void lcd_init() {
  // LCD Init 1
  jshPinOutput(LCD_SPI_CS,0);
  jshPinOutput(LCD_SPI_DC,0);
  jshPinOutput(LCD_SPI_SCK,0);
  jshPinOutput(LCD_SPI_MOSI,0);
  jshPinOutput(LCD_SPI_RST,0);
  // LCD init 2
  nrf_delay_us(10000);
  jshPinSetValue(LCD_SPI_RST,1);
  nrf_delay_us(10000);
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
    lcd_wr(LCD_INIT_DATA[i]);
  jshPinSetValue(LCD_SPI_CS,1);
}
#endif
#ifdef LCD_CONTROLLER_ST7789V
#define LCD_SPI 0
#define jshDelayMicroseconds nrf_delay_us
#define jshSPISend(x,d) lcd_wr(d)

void lcd_cmd(int cmd, int dataLen, char *data) {
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

void lcd_flip() {
  if (ymin<=ymax) {
    ymin=ymin*2;
    ymax=ymax*2+1;
    jshPinOutput(LCD_BL,0); // testing
    jshPinSetValue(LCD_SPI_CS, 0);
    jshPinSetValue(LCD_SPI_DC, 0); // command
    jshSPISend(LCD_SPI, 0x2A);
    jshPinSetValue(LCD_SPI_DC, 1); // data
    jshSPISend(LCD_SPI, 0);
    jshSPISend(LCD_SPI, 0);
    jshSPISend(LCD_SPI, 0);
    jshSPISend(LCD_SPI, LCD_WIDTH);
    jshPinSetValue(LCD_SPI_DC, 0); // command
    jshSPISend(LCD_SPI, 0x2B);
    jshPinSetValue(LCD_SPI_DC, 1); // data
    jshSPISend(LCD_SPI, 0);
    jshSPISend(LCD_SPI, ymin);
    jshSPISend(LCD_SPI, 0);
    jshSPISend(LCD_SPI, ymax+1);
    jshPinSetValue(LCD_SPI_DC, 0); // command
    jshSPISend(LCD_SPI, 0x2C);
    jshPinSetValue(LCD_SPI_DC, 1); // data
    for (int y=ymin;y<=ymax;y++) {
      for (int x=0;x<LCD_WIDTH>>1;x++) { // send 2 pixels at once
        int c = (lcd_data[(x>>3)+((y>>1)*LCD_ROWSTRIDE)]&1<<(x&7)) ? 0xFF:0;
        jshSPISend(LCD_SPI, c);
        jshSPISend(LCD_SPI, c);
        jshSPISend(LCD_SPI, c);
      }
    }
    jshPinSetValue(LCD_SPI_CS,1);
    jshPinOutput(LCD_BL,1); // testing
  }
  ymin=LCD_HEIGHT;
  ymax=0;
}
void lcd_init() {
  jshPinOutput(3,1); // general VDD power?
  jshPinOutput(LCD_BL,1); // backlight
  // LCD Init 1
  jshPinOutput(LCD_SPI_CS,1);
  jshPinOutput(LCD_SPI_DC,1);
  jshPinOutput(LCD_SPI_SCK,1);
  jshPinOutput(LCD_SPI_MOSI,1);
  jshPinOutput(LCD_SPI_RST,0);
  jshDelayMicroseconds(100000);
  jshPinOutput(LCD_SPI_RST,1);
  jshDelayMicroseconds(150000);
  // LCD init 2
  lcd_cmd(0x11, 0, NULL); // SLPOUT
  jshDelayMicroseconds(150000);
  //lcd_cmd(0x3A, 1, "\x55"); // COLMOD - 16bpp
  lcd_cmd(0x3A, 1, "\x03"); // COLMOD - 12bpp
  jshDelayMicroseconds(10000);
  lcd_cmd(0xC6, 1, "\x01"); // Frame rate control in normal mode, 111Hz
  jshDelayMicroseconds(10000);
  lcd_cmd(0x36, 1, "\x08"); // MADCTL
  jshDelayMicroseconds(10000);
  lcd_cmd(0x21, 0, NULL); // INVON
  jshDelayMicroseconds(10000);
  lcd_cmd(0x13, 0, NULL); // NORON
  jshDelayMicroseconds(10000);
  lcd_cmd(0x36, 1, "\xC0"); // MADCTL
  jshDelayMicroseconds(10000);
  lcd_cmd(0x37, 2, "\0\x50"); // VSCRSADD - vertical scroll
  jshDelayMicroseconds(10000);
  lcd_cmd(0x35, 0, NULL); // Tear on
    jshDelayMicroseconds(10000);
  lcd_cmd(0x29, 0, NULL); // DISPON
  jshDelayMicroseconds(10000);
}
#endif


#else
// No LCD
void lcd_init() {}
void lcd_print(char *ch) {}
#endif
