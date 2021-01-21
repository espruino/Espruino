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
 * Device specific information for SPI LCDs
 * ----------------------------------------------------------------------------
 */

#include "platform_config.h"

#define CMDINDEX_CMD   0
#define CMDINDEX_DELAY 1
#define CMDINDEX_DATALEN  2

#ifdef LCD_CONTROLLER_ST7735
static const unsigned char SPILCD_INIT_CODE[] = {
  // CMD,DELAY,DATA_LEN,D0,D1,D2...
  // SWRESET Software reset - but we have hardware reset
  // 0x01, 20, 0,
  // SLPOUT Leave sleep mode
  0x11, 100, 0,
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
   0x29, 10, 0,
  // NORON Normal on
   0x13, 10, 0,
  // End
   0, 0, 255/*DATA_LEN = 255 => END*/
};
const unsigned char SPILCD_CMD_WINDOW_X = 0x2A;
const unsigned char SPILCD_CMD_WINDOW_Y = 0x2B;
const unsigned char SPILCD_CMD_DATA = 0x2C;
#endif
#ifdef LCD_CONTROLLER_ST7789V
static const unsigned char SPILCD_INIT_CODE[] = {
// CMD,DELAY,DATA_LEN,D0,D1,D2...
0x11, 100, 0, // SLPOUT Leave sleep mode
0x36,0,1,/*data*/0, // memory access control
0x3a,0,1,/*data*/3, // pixel format 12bpp
//0x3a,0,1,/*data*/5, // pixel format 16bpp
0x3a,0,0,/*data*/ //
0x21,0,0,/*data*/ // display invert
0xe7,0,1,/*data*/0, // disable SPI2
0x37,0,2,/*data*/0,0, // disable SPI2
0x2a,0,4,/*data*/0,0,0,0xef, // column address
0x2b,0,4,/*data*/0,0,0,0xef, // row address
0xb2,0,5,/*data*/0xc,0xc,0,0x33,0x33, // porch control

0xb7,0,1,/*data*/0x35, // gate control

0xbb,0,1,/*data*/0x2a, // VCOMS
0xc0,0,1,/*data*/0x2c, // LCM Control
0xc2,0,1,/*data*/1, // VDV VRH enable
0xc3,0,1,/*data*/0xb, // VRH set
0xc4,0,1,/*data*/0x20, // VDV set
0xc6,0,1,/*data*/0xf, // FR Control 2
0xd0,0,2,/*data*/0xa4,0xa1, // Power control 1
0xe9,0,3,/*data*/0x11,0x11,3, // Equalise time
0xe0,0,14,/*data*/0xf0,9,0x13,10,0xb,6,0x38,0x33,0x4f,4,0xd,0x19,0x2e,0x2f, // gamma
0xe1,0x14,14,/*data*/0xf0,9,0x13,10,0xb,6,0x38,0x33,0x4f,4,0xd,0x19,0x2e,0x2f, // gamma
0x29,0x14,0,/*data*/ // display on
// End
0, 0, 255/*DATA_LEN = 255 => END*/
};

const unsigned char SPILCD_CMD_WINDOW_X = 0x2A;
const unsigned char SPILCD_CMD_WINDOW_Y = 0x2B;
const unsigned char SPILCD_CMD_DATA = 0x2C;
#endif
#ifdef LCD_CONTROLLER_GC9A01
static const unsigned char SPILCD_INIT_CODE[] = {
// CMD,DELAY,DATA_LEN,D0,D1,D2...
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
    0x36,0,1,  0x88, // Memory Access Control (0x48 flips upside-down)
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
    // End
    0, 0, 255/*DATA_LEN = 255 => END*/
  };

const unsigned char SPILCD_CMD_WINDOW_X = 0x2A;
const unsigned char SPILCD_CMD_WINDOW_Y = 0x2B;
const unsigned char SPILCD_CMD_DATA = 0x2C;
#endif

