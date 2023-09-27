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
    0x84,0,1,  0x60, // 0x40->0x60 0xb5 en  20200924  james
    0x85,0,1,  0xFF,
    0x86,0,1,  0xFF,
    0x87,0,1,  0xFF,
    0x8e,0,1,  0xFF,
    0x8f,0,1,  0xFF,
    0x88,0,1,  10,
    0x89,0,1,  0x23, // 0x21->0x23 spi 2data reg en
    0x8a,0,1,  0,
    0x8b,0,1,  0x80,
    0x8c,0,1,  1,
    0x8d,0,1,  3,   // 1->3  99 en
    0xb5,0,4,  0x08, 0x09, 0x14, 0x08,
    0xb6,0,2,  0, 0, // Positive sweep 0x20->0  GS SS 0x20
#ifdef LCD_ROTATION
 #if (LCD_ROTATION == 90)
    0x36,0,1,  0x78, // Memory Access Control (rotated 90 degrees)
 #elif (LCD_ROTATION == 180)
    0x36,0,1,  0x48, // Memory Access Control (rotated 180 degrees)
 #elif (LCD_ROTATION == 270)
    0x36,0,1,  0xB8, // Memory Access Control (rotated 270 degrees)
 #else
    0x36,0,1,  0x88, // Memory Access Control (no rotation)
 #endif   
#else
    0x36,0,1,  0x88, // Memory Access Control (no rotation)
#endif
    0x3a,0,1,  5, // could be 16/12 bit?
    0x90,0,4,  8,  8,  8,  8,
    0xba,0,1,  1, // TE width
    0xbd,0,1,  6,
    0xbc,0,1,  0,
    0xff,0,3,  0x60,  1,  4,
    0xc3,0,1,  0x13, // Power control 2: 0x13->0x1d->0x13 (again)
    0xc4,0,1,  0x13, // Power control 3: 0x13->0x1d->0x13 (again)
    0xc9,0,1,  0x25, // Power control 4: 0x22->0x25
    0xbe,0,1,  0x11,
    0xe1,0,2,  0x10,  0xe,
    0xdf,0,3,  0x21,  0xc,  2,
    0xf0,0,6,  0x45,  9,  8,  8,  0x26,  0x2a,          // Gamma 1
    0xf1,0,6,  0x43,  0x70,  0x72,  0x36,  0x37,  0x6f, // Gamma 2
    0xf2,0,6,  0x45,  9,  8,  8,  0x26,  0x2a,          // Gamma 3
    0xf3,0,6,  0x43,  0x70,  0x72,  0x36,  0x37,  0x6f, // Gamma 4
    0xed,0,2,  0x1b,  0xb,
    0xae,0,1,  0x77,  // 0x74->0x77
    0xcd,0,1,  0x63,
    0x70,0,9,  7,  7,  4,  0xe,  0xf,  9,  7,  8,  3,  // 7,9,4... -> 7,7,4...
    0xe8,0,1,  0x34,
    0x60,0,8,  0x38, 0x0b, 0x6d, 0x6d, 0x39, 0xf0, 0x6d, 0x6d,
    0x61,0,8,  0x38, 0xf4, 0x6d, 0x6d, 0x38, 0xf7, 0x6d, 0x6d,
    0x62,0,12,  0x38,  0xd,  0x71,  0xed,  0x70,  0x70,  0x38,  0xf,  0x71,  0xef,  0x70,  0x70,
    0x63,0,12,  0x38,  0x11, 0x71,  0xf1,  0x70,  0x70,  0x38,  0x13,  0x71,  0xf3,  0x70,  0x70,
    0x64,0,7,  0x28,  0x29,  0xf1,  1,  0xf1,  0,  7,
    0x66,0,10,  0x3c,  0,  0xcd,  0x67,  0x45,  0x45,  0x10,  0,  0,  0,
    0x67,0,10,  0,  0x3c,  0,  0,  0,  1,  0x54,  0x10,  0x32,  0x98,
    0x74,0,7,  0x10,  0x68,  0x80,  0,  0,  0x4e,  0, // 0x85->0x68
    0x98,0,2,  0x3e,  7,
    0x99,0,2,  0x3e,  7, // bvee 2x
    0x35,0,1,  0,   // Tearing effect (TE) line ON, with V-blanking only
    0x21,5,0,       // Display inversion ON
    0x11,5,0,       // Sleep out
    0x29,5,0,       // Display ON
    0x2c,0,0,       // Memory write
    // End
    0, 0, 255/*DATA_LEN = 255 => END*/
  };

const unsigned char SPILCD_CMD_WINDOW_X = 0x2A;
const unsigned char SPILCD_CMD_WINDOW_Y = 0x2B;
const unsigned char SPILCD_CMD_DATA = 0x2C;
#endif

