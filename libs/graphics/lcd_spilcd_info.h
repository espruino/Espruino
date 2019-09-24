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
 * Graphics Backend for drawing to simple black and white SPI displays
 * (PCD8544 - Nokia 5110 LCD)
 * ----------------------------------------------------------------------------
 */

#include "platform_config.h"

#define CMDINDEX_CMD   0
#define CMDINDEX_DELAY 1
#define CMDINDEX_DATALEN  2

#ifdef LCD_CONTROLLER_ST7735
static const char SPILCD_INIT_CODE[] = {
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
