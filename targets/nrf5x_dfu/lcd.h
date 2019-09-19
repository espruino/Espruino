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

#ifdef LCD_CONTROLLER_ST7567 // Pixl
#define LCD
#define LCD_DATA_WIDTH 128
#define LCD_DATA_HEIGHT 64
#endif
#ifdef LCD_CONTROLLER_ST7789V // iD205
#define LCD
#define LCD_DATA_WIDTH 120
#define LCD_DATA_HEIGHT 120
#define LCD_STORE_MODIFIED
#endif
#ifdef LCD_CONTROLLER_ST7735 // HackStrap
#define LCD
#define LCD_DATA_WIDTH 128
#define LCD_DATA_HEIGHT 96
#define LCD_STORE_MODIFIED
#endif

void lcd_init();
void lcd_kill();
void lcd_print(char *ch);
void lcd_println(char *ch);

