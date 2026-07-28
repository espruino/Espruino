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
#include <string.h>
#include "py32f07x_hal.h"
#include "lcd.h"

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

    PACK_5_TO_16( _X_ , ___ , _X_ , XX_ , _XX ), // ?@ABC
    PACK_5_TO_16( X_X , _X_ , X_X , X_X , X__ ), // @ is used as +
    PACK_5_TO_16( __X , XXX , XXX , XX_ , X__ ),
    PACK_5_TO_16( ___ , _X_ , X_X , X_X , X__ ),
    PACK_5_TO_16( _X_ , ___ , X_X , XX_ , _XX ),

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
    PACK_5_TO_16( XX_ , _X_ , XXX , _X_ , X_X ),

    PACK_5_TO_16( X_X , X_X , XXX , _XX , ___ ), // XYZ[\ end
    PACK_5_TO_16( X_X , X_X , __X , _X_ , ___ ), // \ is used as .
    PACK_5_TO_16( _X_ , _X_ , _X_ , _X_ , ___ ),
    PACK_5_TO_16( X_X , _X_ , X__ , _X_ , ___ ),
    PACK_5_TO_16( X_X , _X_ , XXX , _XX , _X_ ),
};

int lcdx = LCD_START_X, lcdy = LCD_START_Y;
uint8_t lcd_data[LCD_ROWSTRIDE*LCD_DATA_HEIGHT];
#ifdef LCD_STORE_MODIFIED
int ymin=0,ymax=LCD_DATA_HEIGHT-1;
#endif

static void D() { /*for (volatile int i=0;i<0;i++);*/ }
static void DX() { /*for (volatile int i=0;i<10;i++);*/ }

void lcd_pixel(int x, int y) {
  int idx = (x+y*120)>>3, o = x&7;
  lcd_data[idx] |= (1<<o);
}

void lcd_flip() {
  LCD_XRST(0);DX();DX();DX();DX();DX();
  LCD_XRST(1);D();
  LCD_ENB(1);DX(); // def needed - but datasheet shows for LSB only?
  LCD_VCK(0);DX();
  LCD_VST(1);DX();
  LCD_VCK(1);DX();
  LCD_VST(0);D();
  LCD_VCK(0);D();
  LCD_VCK(1);D();

  for (int y=0;y<240;y++) {
    uint8_t *px;
    LCD_HST(1);D();
    LCD_HCK(1);D();
    LCD_HST(0);D();
    LCD_HCK(0);D(); // MSB
    px = &lcd_data[((y>>1)*120)>>3];
    for (int x=0;x<120;x+=8) {
      int c = *(px++);
      LCD_COL((c&1)?63:0);
      LCD_HCK(1);
      LCD_COL((c&2)?63:0);
      LCD_HCK(0);
      LCD_COL((c&4)?63:0);
      LCD_HCK(1);
      LCD_COL((c&8)?63:0);
      LCD_HCK(0);
      LCD_COL((c&16)?63:0);
      LCD_HCK(1);
      LCD_COL((c&32)?63:0);
      LCD_HCK(0);
      LCD_COL((c&64)?63:0);
      LCD_HCK(1);
      LCD_COL((c&128)?63:0);
      LCD_HCK(0);
    }
    D();
    LCD_VCK(0);D();
    LCD_HST(1);D();
    LCD_HCK(1);D();
    LCD_HST(0);D();
    LCD_HCK(0);D(); // LSB
    px = &lcd_data[((y>>1)*120)>>3];
    for (int x=0;x<120;x+=8) {
      int c = *(px++);
      LCD_COL((c&1)?63:0);
      LCD_HCK(1);
      LCD_COL((c&2)?63:0);
      LCD_HCK(0);
      LCD_COL((c&4)?63:0);
      LCD_HCK(1);
      LCD_COL((c&8)?63:0);
      LCD_HCK(0);
      LCD_COL((c&16)?63:0);
      LCD_HCK(1);
      LCD_COL((c&32)?63:0);
      LCD_HCK(0);
      LCD_COL((c&64)?63:0);
      LCD_HCK(1);
      LCD_COL((c&128)?63:0);
      LCD_HCK(0);
    }
    D();
    LCD_VCK(1);D();
  }
  // datasheet shows 488 clocks (so maybe clock a few more out?)
  LCD_ENB(0);
}

void lcd_init() {
}

void lcd_char(int x1, int y1, char ch) {
  // char replacements so we don't waste font space
  if (ch=='.') ch='\\';
  if (ch=='+') ch='@';
  if (ch>='a') ch-='a'-'A';
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

void lcd_print(char *ch) {
  while (*ch) {
    lcd_char(lcdx,lcdy,*ch);
    if ('\n'==*ch) {
      lcdy += 6;
      if (lcdy>=LCD_DATA_HEIGHT-4) {
        memcpy(lcd_data,&lcd_data[LCD_ROWSTRIDE*8],LCD_ROWSTRIDE*(LCD_DATA_HEIGHT-8)); // shift up 8 pixels
        memset(&lcd_data[LCD_ROWSTRIDE*(LCD_DATA_HEIGHT-8)],0,LCD_ROWSTRIDE*8); // fill bottom 8 rows
        lcdy-=8;
#ifdef LCD_STORE_MODIFIED
        ymin=0;
        ymax=LCD_DATA_HEIGHT-1;
#endif
      }
    } else if ('\r'==*ch) {
      lcdx = LCD_START_X;
    } else lcdx += 4;
    ch++;
  }
}
void lcd_print_hex(unsigned int v) {
 char buf[11] = "0x";
 for (int i=0;i<8;i++) {
   int n = (v>>((7-i)*4)) & 0x0F;
   buf[2+i] = (n<10) ? ('0'+n) : ('A'+n-10);
 }
 buf[10] = 0;
 lcd_print(buf);
}
void lcd_println(char *ch) {
  lcd_print(ch);
  lcd_print("\r\n");
  lcd_flip();
}
void lcd_clear() {
  memset(lcd_data,0,sizeof(lcd_data));
  lcdx=LCD_START_X;
  lcdy=LCD_START_Y;
#ifdef LCD_STORE_MODIFIED
  ymin=0;
  ymax=LCD_DATA_HEIGHT-1;
#endif
  lcd_flip();
}
