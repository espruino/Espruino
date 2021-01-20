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
#include "hardware.h"
#include "lcd.h"

#ifdef LCD

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
    PACK_5_TO_16( XX_ , _X_ , _X_ , _X_ , X_X ),

    PACK_5_TO_16( X_X , X_X , XXX , _XX , ___ ), // XYZ[\ end
    PACK_5_TO_16( X_X , X_X , __X , _X_ , ___ ), // \ is used as .
    PACK_5_TO_16( _X_ , _X_ , _X_ , _X_ , ___ ),
    PACK_5_TO_16( X_X , _X_ , X__ , _X_ , ___ ),
    PACK_5_TO_16( X_X , _X_ , XXX , _XX , _X_ ),
};

int lcdx = LCD_START_X, lcdy = LCD_START_Y;
#define LCD_ROWSTRIDE (LCD_DATA_WIDTH>>3)
char lcd_data[LCD_ROWSTRIDE*LCD_DATA_HEIGHT];
#ifdef LCD_STORE_MODIFIED
int ymin=0,ymax=LCD_DATA_HEIGHT-1;
#endif

void lcd_flip();

#ifdef LCD_CONTROLLER_ST7567
void lcd_pixel(int x, int y) {
  // each byte is vertical
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
#if defined(LCD_CONTROLLER_ST7789V) || defined(LCD_CONTROLLER_ST7735) || defined(LCD_CONTROLLER_LPM013M126) || defined(LCD_CONTROLLER_GC9A01)

void lcd_pixel(int x, int y) {
  // each byte is horizontal
  lcd_data[(x>>3)+(y*LCD_ROWSTRIDE)] |= 1<<(x&7);
#ifdef LCD_STORE_MODIFIED
  // update changed area
  if (y<ymin) ymin=y;
  if (y>ymax) ymax=y;
#endif
}

#ifdef NRF52_SERIES
// Enable fast SPI by writing direct to registers
#if LCD_SPI_SCK<32
#define LCD_SPI_SCK_SET() (*(volatile uint32_t*)0x50000508)=1<<LCD_SPI_SCK
#define LCD_SPI_SCK_CLEAR() (*(volatile uint32_t*)0x5000050C)=1<<LCD_SPI_SCK
#else
#define LCD_SPI_SCK_SET() (*(volatile uint32_t*)0x50000808)=1<<(LCD_SPI_SCK-32)
#define LCD_SPI_SCK_CLEAR() (*(volatile uint32_t*)0x5000080C)=1<<(LCD_SPI_SCK-32)
#endif
#if LCD_SPI_MOSI<32
#define LCD_SPI_MOSI_SET() (*(volatile uint32_t*)0x50000508)=1<<LCD_SPI_MOSI
#define LCD_SPI_MOSI_CLEAR() (*(volatile uint32_t*)0x5000050C)=1<<LCD_SPI_MOSI
#else
#define LCD_SPI_MOSI_SET() (*(volatile uint32_t*)0x50000808)=1<<(LCD_SPI_MOSI-32)
#define LCD_SPI_MOSI_CLEAR() (*(volatile uint32_t*)0x5000080C)=1<<(LCD_SPI_MOSI-32)
#endif

void lcd_wr(int data) {
  for (int bit=7;bit>=0;bit--) {
    LCD_SPI_SCK_CLEAR();
    if ((data>>bit)&1) LCD_SPI_MOSI_SET();
    else LCD_SPI_MOSI_CLEAR();
    LCD_SPI_SCK_SET();
  }
}
void lcd_wr16(bool allFF) {
  if (allFF) LCD_SPI_MOSI_SET();
  else LCD_SPI_MOSI_CLEAR();
  for (int bit=0;bit<16;bit++) {
    LCD_SPI_SCK_CLEAR();
    LCD_SPI_SCK_SET();
  }
}
#else
void lcd_wr(int data) {
  for (int bit=7;bit>=0;bit--) {
    jshPinSetValue(LCD_SPI_SCK, 0 );
    jshPinSetValue(LCD_SPI_MOSI, ((data>>bit)&1) );
    jshPinSetValue(LCD_SPI_SCK, 1 );
  }
}
void lcd_wr16(bool allFF) {
  jshPinSetValue(LCD_SPI_MOSI, allFF);
  for (int bit=0;bit<16;bit++) {
    jshPinSetValue(LCD_SPI_SCK, 0 );
    jshPinSetValue(LCD_SPI_SCK, 1 );
  }
}
#endif
#endif
#if defined(LCD_CONTROLLER_ST7789_8BIT)

void lcd_pixel(int x, int y) {
  // each byte is horizontal
  lcd_data[(x>>3)+(y*LCD_ROWSTRIDE)] |= 1<<(x&7);
#ifdef LCD_STORE_MODIFIED
  // update changed area
  if (y<ymin) ymin=y;
  if (y>ymax) ymax=y;
#endif
}

void lcd_wr(int data) {
  *((uint8_t*)&NRF_P0->OUT) = data;
  asm("nop");asm("nop");asm("nop");asm("nop");
  jshPinSetValue(LCD_PIN_SCK, 0 );
  asm("nop");asm("nop");asm("nop");asm("nop");
  jshPinSetValue(LCD_PIN_SCK, 1 );
}

// very tiny I2C implementation
// for IO expander
void dly() {
  volatile int i;
  for (i=0;i<10;i++);
}
void sda1() {
  nrf_gpio_pin_set(I2C_SDA);
  nrf_gpio_cfg_output(I2C_SDA);
  dly();
}
void sda0() {
  nrf_gpio_pin_clear(I2C_SDA);
  nrf_gpio_cfg_output(I2C_SDA);
  dly();
}
void scl1() {
  nrf_gpio_pin_set(I2C_SCL);
  nrf_gpio_cfg_output(I2C_SCL);
  dly();
}
void scl0() {
  nrf_gpio_pin_clear(I2C_SCL);
  nrf_gpio_cfg_output(I2C_SCL);
  dly();
}
void i2c_wr_bit(bool b) {
  if (b) sda1(); else sda0();
  scl1();
  scl0();
}

void i2c_wr(uint8_t data) {
  int i;
  for (i=0;i<8;i++) {
    i2c_wr_bit(data&128);
    data <<= 1;
  }
  scl1();
  scl0();
}
// I2C write. Address is 8 bit (not 7 as on normal Espruino functions) (ignore value if <0)
void i2c_wrreg(int addr, int reg, int value) {
  // start
  sda0();
  scl0();
  // write
  i2c_wr(addr);
  i2c_wr(reg);
  if (value>=0) i2c_wr(value);
  // stop
  sda0();
  scl1();
  sda1();
}
void ioexpander_write(int mask, bool value) {
  static uint8_t state = 0;
  if (value) state|=mask;
  else state&=~mask;
  i2c_wrreg(0x20<<1,state,-1);
}
#endif

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
  jshDelayMicroseconds(10000);
  jshPinSetValue(LCD_SPI_RST,1);
  jshDelayMicroseconds(10000);
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

void lcd_cmd(int cmd, int dataLen, char *data) {
  jshPinSetValue(LCD_SPI_CS, 0);
  jshPinSetValue(LCD_SPI_DC, 0); // command
  lcd_wr(cmd);
  if (dataLen) {
    jshPinSetValue(LCD_SPI_DC, 1); // data
    while (dataLen) {
      lcd_wr(*(data++));
      dataLen--;
    }
  }
  jshPinSetValue(LCD_SPI_CS, 1);
}

void lcd_flip() {
  if (ymin<=ymax) {
    ymin=ymin*2;
    ymax=ymax*2+1;
    jshPinSetValue(LCD_SPI_CS, 0);
    jshPinSetValue(LCD_SPI_DC, 0); // command
    lcd_wr(0x2A);
    jshPinSetValue(LCD_SPI_DC, 1); // data
    lcd_wr(0);
    lcd_wr(0);
    lcd_wr(0);
    lcd_wr(LCD_DATA_WIDTH*2);
    jshPinSetValue(LCD_SPI_DC, 0); // command
    lcd_wr(0x2B);
    jshPinSetValue(LCD_SPI_DC, 1); // data
    lcd_wr(0);
    lcd_wr(ymin);
    lcd_wr(0);
    lcd_wr(ymax+1);
    jshPinSetValue(LCD_SPI_DC, 0); // command
    lcd_wr(0x2C);
    jshPinSetValue(LCD_SPI_DC, 1); // data
    for (int y=ymin;y<=ymax;y++) {
      for (int x=0;x<LCD_DATA_WIDTH;x++) { // send 2 pixels at once
        int c = (lcd_data[(x>>3)+((y>>1)*LCD_ROWSTRIDE)]&1<<(x&7)) ? 0xFF:0;
        lcd_wr(c);
        lcd_wr(c);
        lcd_wr(c);
      }
    }
    jshPinSetValue(LCD_SPI_CS,1);
  }
  ymin=LCD_HEIGHT;
  ymax=0;
}
void lcd_init() {
  jshPinOutput(3,1); // general VDD power?
  jshPinOutput(LCD_BL, LCD_BL_ON); // backlight
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

void lcd_kill() {
  jshPinOutput(LCD_BL, !LCD_BL_ON);
  lcd_cmd(0xAE, 0, NULL); // DISPOFF
}
#endif
#ifdef LCD_CONTROLLER_ST7789_8BIT

#define CMDINDEX_CMD   0
#define CMDINDEX_DATALEN  1
static const char ST7789_INIT_CODE[] = {
  // CMD,DATA_LEN,D0,D1,D2...
    0x11,0,
    0x36,1,0, // MADCTL
    0x3A,1,0x55, // COLMOD - interface pixel format - 16bpp
    0xB2,5,0xC,0xC,0,0x33,0x33,
    0xB7,1,0,
    0xBB,1,0x3E,
    0xC2,1,1,
    0xC3,1,0x19,
    0xC4,1,0x20,
    0xC5,1,0xF,
    0xD0,2,0xA4,0xA1,
    0x29,0,
    0x21,0,
    // End
    0, 255/*DATA_LEN = 255 => END*/
};

void lcd_cmd(int cmd, int dataLen, char *data) {
  jshPinSetValue(LCD_PIN_CS, 0);
  jshPinSetValue(LCD_PIN_DC, 0); // command
  lcd_wr(cmd);
  if (dataLen) {
    jshPinSetValue(LCD_PIN_DC, 1); // data
    while (dataLen) {
      lcd_wr(*(data++));
      dataLen--;
    }
  }
  jshPinSetValue(LCD_PIN_CS, 1);
}

void lcd_flip() {
#if LCD_STORE_MODIFIED
  if (ymin<=ymax) {
    ymin=ymin*2;
    ymax=ymax*2+1;
#else
    const int ymin = 0;
    const int ymax = (LCD_DATA_HEIGHT*2)-1;
#endif
    jshPinSetValue(LCD_PIN_CS, 0);
    jshPinSetValue(LCD_PIN_DC, 0); // command
    lcd_wr(0x2A);
    jshPinSetValue(LCD_PIN_DC, 1); // data
    lcd_wr(0);
    lcd_wr(0);
    lcd_wr(0);
    lcd_wr(LCD_DATA_WIDTH*2);
    jshPinSetValue(LCD_PIN_DC, 0); // command
    lcd_wr(0x2B);
    jshPinSetValue(LCD_PIN_DC, 1); // data
    lcd_wr(0);
    lcd_wr(ymin);
    lcd_wr(0);
    lcd_wr(ymax+1);
    jshPinSetValue(LCD_PIN_DC, 0); // command
    lcd_wr(0x2C);
    jshPinSetValue(LCD_PIN_DC, 1); // data
    for (int y=ymin;y<=ymax;y++) {
      for (int x=0;x<LCD_DATA_WIDTH;x++) { // send 2 pixels at once
        bool c = (lcd_data[(x>>3)+((y>>1)*LCD_ROWSTRIDE)]&1<<(x&7)) ? 0xFF:0;
        lcd_wr(c);
        lcd_wr(c);
        lcd_wr(c);
        lcd_wr(c);
      }
    }
    jshPinSetValue(LCD_PIN_CS,1);
#if LCD_STORE_MODIFIED
  }
  ymin=LCD_HEIGHT;
  ymax=0;
#endif
}

void lcd_send_cmd(uint8_t cmd) {
  jshPinSetValue(LCD_PIN_CS, 0);
  jshPinSetValue(LCD_PIN_DC, 0); // command
  lcd_wr(cmd);
  jshPinSetValue(LCD_PIN_CS, 1);
}
void lcd_send_data(uint8_t cmd) {
  jshPinSetValue(LCD_PIN_CS, 0);
  jshPinSetValue(LCD_PIN_DC, 1); // data
  lcd_wr(cmd);
  jshPinSetValue(LCD_PIN_CS, 1);
}

void lcd_init() {
  nrf_gpio_cfg_input(I2C_SDA, NRF_GPIO_PIN_PULLUP);
  nrf_gpio_cfg_input(I2C_SCL, NRF_GPIO_PIN_PULLUP);

  jshPinOutput(LCD_PIN_CS,1);
  jshPinOutput(LCD_PIN_DC,1);
  jshPinOutput(LCD_PIN_SCK,1);
  for (int i=0;i<8;i++)
    nrf_gpio_pin_write_output(i, 0);

  jshPinOutput(28,0); // IO expander reset
  nrf_delay_ms(10);
  jshPinSetValue(28,1);
  nrf_delay_ms(0x32);

  ioexpander_write(0,1);
  ioexpander_write(0,0);
  ioexpander_write(0x80,1); // HRM off
  nrf_delay_ms(100);
  ioexpander_write(0x40,1); // LCD reset off
  // ioexpander_write(0x20,0); // backlight on (default)
  nrf_delay_ms(0x78);

  // Send initialization commands to ST7789
  const char *cmd = ST7789_INIT_CODE;
  while(cmd[CMDINDEX_DATALEN]!=255) {
    lcd_cmd(cmd[CMDINDEX_CMD], cmd[CMDINDEX_DATALEN], &cmd[2]);
    cmd += 2 + cmd[CMDINDEX_DATALEN];
  }
}

void lcd_kill() {
  lcd_send_cmd(0x28); // DISPOFF
  lcd_send_cmd(0x10); // SLPIN
  ioexpander_write(0x20,1); // backlight off
  /* TODO: not convinced accelerometer is being turned off - power consumption
  is higher if we just rebooted from Bangle.js. Maybe this I2C impl. is too
  noncompliant for it. */
  i2c_wrreg(ACCEL_ADDR<<1, 0x18,0x0a);  // accelerometer off
  i2c_wrreg(MAG_ADDR<<1, 0x31,0); // compass off
}
#endif

#if defined(LCD_CONTROLLER_ST7735) || defined(LCD_CONTROLLER_GC9A01)

#define CMDINDEX_CMD   0
#define CMDINDEX_DELAY 1
#define CMDINDEX_DATALEN  2
static const char SPILCD_INIT_CODE[] = {
#ifdef LCD_CONTROLLER_ST7735
  // CMD,DELAY,DATA_LEN,D0,D1,D2...
  // SWRESET Software reset
  0x01, 150, 0,
  // SLPOUT Leave sleep mode
  0x11, 150, 0,
  // FRMCTR1 , FRMCTR2 Frame Rate configuration -- Normal mode, idle
  // frame rate = fosc / (1 x 2 + 40) * (LINE + 2C + 2D)
  0xB1, 0, 3,  /*data*/0x01, 0x2C, 0x2D ,
  0xB2, 0, 3,  /*data*/0x01, 0x2C, 0x2D ,
  // FRMCTR3 Frame Rate configureation -- partial mode
  0xB3, 0, 6, /*data*/0x01, 0x2C, 0x2D, 0x01, 0x2C, 0x2D ,
  // INVCTR Display inversion (no inversion)
   0xB4, 0, 1, /*data*/0x07 ,
  // PWCTR1 Power control -4.6V, Auto mode
   0xC0, 0, 3,  /*data*/0xA2, 0x02, 0x84,
  // PWCTR2 Power control VGH25 2.4C, VGSEL -10, VGH = 3 * AVDD
   0xC1, 0, 1,  /*data*/0xC5,
  // PWCTR3 Power control , opamp current smal, boost frequency
   0xC2, 0, 2,  /*data*/0x0A, 0x00 ,
  // PWCTR4 Power control , BLK/2, opamp current small and medium low
   0xC3, 0, 2,  /*data*/0x8A, 0x2A,
  // PWRCTR5 , VMCTR1 Power control
   0xC4, 0, 2,  /*data*/0x8A, 0xEE,
   0xC5, 0, 1,  /*data*/0x0E ,
  // INVOFF Don't invert display
   0x20, 0, 0,
  // MADCTL row address/col address, bottom to top refesh (10.1.27)
   0x36, 0, 1, /*data*/0xC8,
  // COLMOD, Color mode 12 bit
   0x3A, 0, 1, /*data*/0x03,
  // GMCTRP1 Gamma correction
   0xE0, 0, 16, /*data*/0x02, 0x1C, 0x07, 0x12, 0x37, 0x32, 0x29, 0x2D, 0x29, 0x25, 0x2B, 0x39, 0x00, 0x01, 0x03, 0x10 ,
  // GMCTRP2 Gamma Polarity correction
   0xE1, 0, 16, /*data*/0x03, 0x1d, 0x07, 0x06, 0x2E, 0x2C, 0x29, 0x2D, 0x2E, 0x2E, 0x37, 0x3F, 0x00, 0x00, 0x02, 0x10 ,
  // DISPON Display on
   0x29, 100, 0,
  // NORON Normal on
   0x13, 10, 0,
#endif
#ifdef LCD_CONTROLLER_GC9A01
   0xfe,0,0,
   0xef,0,0,
   0xeb,0,1,  0x14,
   0x84,0,1,  0x40,
   0x88,0,1,  10,
   0x89,0,1,  0x21,
   0x8a,0,1,  0,
   0x8b,0,1,  0x80,
   0x8c,0,1,  1,
   0x8d,0,1,  1,
   0xb6,0,1,  0x20,
   0x36,0,1,  0x48, // Memory Access Control
   0x3a,0,1,  5, // could be 16/12 bit?
   0x90,0,4,  8,  8,  8,  8,
   0xbd,0,1,  6,
   0xbc,0,1,  0,
   0xff,0,3,  0x60,  1,  4,
   0xc3,0,1,  0x13,
   0xc4,0,1,  0x13,
   0xc9,0,1,  0x22,
   0xbe,0,1,  0x11,
   0xe1,0,2,  0x10,  0xe,
   0xdf,0,3,  0x21,  0xc,  2,
   0xf0,0,6,  0x45,  9,  8,  8,  0x26,  0x2a,
   0xf1,0,6,  0x43,  0x70,  0x72,  0x36,  0x37,  0x6f,
   0xf2,0,6,  0x45,  9,  8,  8,  0x26,  0x2a,
   0xf3,0,6,  0x43,  0x70,  0x72,  0x36,  0x37,  0x6f,
   0xed,0,2,  0x1b,  0xb,
   0xae,0,1,  0x74,
   0xcd,0,1,  99,
   0x70,0,9,  7,  9,  4,  0xe,  0xf,  9,  7,  8,  3,
   0xe8,0,1,  0x34,
   0x62,0,12,  0x18,  0xd,  0x71,  0xed,  0x70,  0x70,  0x18,  0xf,  0x71,  0xef,  0x70,  0x70,
   99,0,12,  0x18,  0x11,  0x71,  0xf1,  0x70,  0x70,  0x18,  0x13,  0x71,  0xf3,  0x70,  0x70,
   100,0,7,  0x28,  0x29,  0xf1,  1,  0xf1,  0,  7,
   0x66,0,10,  0x3c,  0,  0xcd,  0x67,  0x45,  0x45,  0x10,  0,  0,  0,
   0x67,0,10,  0,  0x3c,  0,  0,  0,  1,  0x54,  0x10,  0x32,  0x98,
   0x74,0,7,  0x10,  0x85,  0x80,  0,  0,  0x4e,  0,
   0x98,0,2,  0x3e,  7,
   0x35,0,0,
   0x21,10,0,
   0x11,20,0,
   0x29,10,0,
   0x2c,0,0,
#endif
   // End
   0, 0, 255/*DATA_LEN = 255 => END*/
};
void lcd_cmd(int cmd, int dataLen, const char *data) {
  jshPinSetValue(LCD_SPI_CS, 0);
  jshPinSetValue(LCD_SPI_DC, 0); // command
  lcd_wr(cmd);
  if (dataLen) {
    jshPinSetValue(LCD_SPI_DC, 1); // data
    while (dataLen) {
      lcd_wr(*(data++));
      dataLen--;
    }
  }
  jshPinSetValue(LCD_SPI_CS, 1);
}

void lcd_flip() {
#ifndef LCD_STORE_MODIFIED
  int ymin=0;
  int ymax=LCD_HEIGHT-1;
#endif
  if (ymin<=ymax) {
    jshPinSetValue(LCD_SPI_DC, 0); // command
    jshPinSetValue(LCD_SPI_CS, 0);
    lcd_wr(0x2A);
    jshPinSetValue(LCD_SPI_CS, 1);
    jshPinSetValue(LCD_SPI_DC, 1); // data
    jshPinSetValue(LCD_SPI_CS, 0);
    lcd_wr(0);
    lcd_wr(0);
    lcd_wr(0);
    lcd_wr(LCD_WIDTH-1);
    jshPinSetValue(LCD_SPI_CS, 1);
    jshPinSetValue(LCD_SPI_DC, 0); // command
    jshPinSetValue(LCD_SPI_CS, 0);
    lcd_wr(0x2B);
    jshPinSetValue(LCD_SPI_CS, 1);
    jshPinSetValue(LCD_SPI_DC, 1); // data
    jshPinSetValue(LCD_SPI_CS, 0);
    lcd_wr(0);
    lcd_wr(ymin);
    lcd_wr(0);
    lcd_wr(ymax);
    jshPinSetValue(LCD_SPI_CS, 1);
    jshPinSetValue(LCD_SPI_DC, 0); // command
    jshPinSetValue(LCD_SPI_CS, 0);
    lcd_wr(0x2C);
    jshPinSetValue(LCD_SPI_CS, 1);
    jshPinSetValue(LCD_SPI_DC, 1); // data
    jshPinSetValue(LCD_SPI_CS, 0);
    for (int y=ymin;y<=ymax;y++) {
#ifdef LCD_CONTROLLER_ST7735
      for (int x=LCD_WIDTH-1;x>=0;) {
        bool a = lcd_data[(x>>3)+(y*LCD_ROWSTRIDE)]&1<<(x&7);x--;
        bool b = lcd_data[(x>>3)+(y*LCD_ROWSTRIDE)]&1<<(x&7);x--;
        lcd_wr(a?0xFF:0);
        lcd_wr((a?0xF0:0)|(b?0x0F:0));
        lcd_wr(b?0xFF:0);
      }
#endif
#ifdef LCD_CONTROLLER_GC9A01
      for (int x=0;x<LCD_WIDTH;x++) {
        bool a = lcd_data[(x>>3)+(y*LCD_ROWSTRIDE)]&1<<(x&7);
        lcd_wr16(a);
      }
#endif
    }
    jshPinSetValue(LCD_SPI_CS,1);
  }
  ymin=LCD_HEIGHT;
  ymax=0;
}

void lcd_init() {
#ifdef GPS_PIN_EN
  jshPinOutput(GPS_PIN_EN,1); // GPS off
#endif
  jshPinOutput(LCD_BL, LCD_BL_ON); // backlight on
#ifdef LCD_EN
  jshPinOutput(LCD_EN,1); // enable on
#endif
  // LCD Init 1
  jshPinOutput(LCD_SPI_CS,1);
  jshPinOutput(LCD_SPI_DC,1);
  jshPinOutput(LCD_SPI_SCK,1);
  jshPinOutput(LCD_SPI_MOSI,1);
  jshPinOutput(LCD_SPI_RST,0);
  jshDelayMicroseconds(10000);
  jshPinOutput(LCD_SPI_RST, 1);
  jshDelayMicroseconds(10000);

  // Send initialization commands
  const char *cmd = SPILCD_INIT_CODE;
  while(cmd[CMDINDEX_DATALEN]!=255) {
    lcd_cmd(cmd[CMDINDEX_CMD], cmd[CMDINDEX_DATALEN], &cmd[3]);
    if (cmd[CMDINDEX_DELAY])
      jshDelayMicroseconds(1000*cmd[CMDINDEX_DELAY]);
    cmd += 3 + cmd[CMDINDEX_DATALEN];
  }
}
void lcd_kill() {
  jshPinOutput(LCD_BL,!LCD_BL_ON); // backlight off
  lcd_cmd(0x28, 0, NULL); // display off
#ifdef LCD_EN
  jshPinOutput(LCD_EN,0); // enable off
#endif
}

#endif

#ifdef LCD_CONTROLLER_LPM013M126

void lcd_flip() {
  if (ymin>ymax) return;
  jshPinSetValue(LCD_SPI_CS,1);
  /* // undoubled
  for (int y=ymin;y<=ymax;y++) {
    lcd_wr(0b10010000);
    lcd_wr(y + 1);
    for (int x=0;x<LCD_DATA_WIDTH;x+=2) {
      unsigned char d = lcd_data[(x>>3)+(y*LCD_ROWSTRIDE)] >> (x&7);
      lcd_wr(((d&1)?0:0xF0) | ((d&2)?0:0x0F));
    }
  }*/
  // pixel doubled
  for (int y=ymin;y<=ymax;y++) {
    for (int yy=0;yy<2;yy++) {
      lcd_wr(0b10010000);
      lcd_wr((y*2) + yy + 1);
      for (int x=0;x<LCD_DATA_WIDTH;x++) {
        unsigned char d = lcd_data[(x>>3)+(y*LCD_ROWSTRIDE)] >> (x&7);
        lcd_wr((d&1)?0:0xFF); // doubled
      }
    }
  }
  lcd_wr(0);
  lcd_wr(0);
  jshPinSetValue(LCD_SPI_MOSI,0);
  jshPinOutput(LCD_SPI_SCK,0);
  jshPinSetValue(LCD_SPI_CS,0);
  static bool lcdToggle;
  jshPinSetValue(LCD_EXTCOMIN, lcdToggle = !lcdToggle);
  ymin=LCD_HEIGHT;
  ymax=0;
}

void lcd_init() {
  jshPinOutput(LCD_SPI_CS,0);
  jshPinOutput(LCD_SPI_SCK,0);
  jshPinOutput(LCD_SPI_MOSI,1);
  jshPinOutput(LCD_DISP,1);
  jshPinOutput(LCD_EXTCOMIN,1);
  jshPinOutput(LCD_BL, LCD_BL_ON); // backlight on
}
void lcd_kill() {
  jshPinOutput(LCD_BL, !LCD_BL_ON); // backlight off
  jshPinOutput(LCD_DISP,0); // display off
}

#endif


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
        ymax=LCD_HEIGHT-1;
#endif
      }
    } else if ('\r'==*ch) {
      lcdx = LCD_START_X;
    } else lcdx += 4;
    ch++;
  }
  lcd_flip();
}
void lcd_println(char *ch) {
  lcd_print(ch);
  lcd_print("\r\n");
}
void lcd_clear() {
  memset(lcd_data,0,sizeof(lcd_data));
  lcdx=LCD_START_X;
  lcdy=LCD_START_Y;
#ifdef LCD_STORE_MODIFIED
  ymin=0;
  ymax=LCD_HEIGHT-1;
#endif
  lcd_flip();
}

#else
// No LCD
void lcd_init() {}
void lcd_kill() {}
void lcd_clear() {}; // clear screen
void lcd_print(char *ch) {}
void lcd_println(char *ch) {}
#endif
