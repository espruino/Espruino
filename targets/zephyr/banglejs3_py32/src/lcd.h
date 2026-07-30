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

#include <stdint.h>

#define LCD_DATA_WIDTH 120 // pixel doubled
#define LCD_DATA_HEIGHT 120 // pixel doubled
#define LCD_BL_ON 1
#define LCD_START_X 20
#define LCD_START_Y 20

#ifndef LCD_START_X
#define LCD_START_X 0
#endif
#ifndef LCD_START_Y
#define LCD_START_Y 0
#endif

#define LCD_ENB(v) HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, v?GPIO_PIN_SET:GPIO_PIN_RESET)
#define LCD_XRST(v) HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, v?GPIO_PIN_SET:GPIO_PIN_RESET)
#define LCD_VCK(v) HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, v?GPIO_PIN_SET:GPIO_PIN_RESET)
#define LCD_HST(v) HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, v?GPIO_PIN_SET:GPIO_PIN_RESET)
#define LCD_VST(v) HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, v?GPIO_PIN_SET:GPIO_PIN_RESET)
//#define LCD_HCK(v) HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, v?GPIO_PIN_SET:GPIO_PIN_RESET)
#define LCD_HCK(v) if (v) GPIOA->BSRR = (1<<6); else GPIOA->BRR = (1<<6);

#define LCD_COL(v) \
  GPIOA->ODR = (GPIOA->ODR & 0xFFFFFFC0) | ((v)&0x3F);
/*HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, ((v)&1)?GPIO_PIN_SET:GPIO_PIN_RESET); \
HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, ((v)&2)?GPIO_PIN_SET:GPIO_PIN_RESET); \
HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, ((v)&4)?GPIO_PIN_SET:GPIO_PIN_RESET); \
HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, ((v)&8)?GPIO_PIN_SET:GPIO_PIN_RESET); \
HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, ((v)&16)?GPIO_PIN_SET:GPIO_PIN_RESET); \
HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, ((v)&32)?GPIO_PIN_SET:GPIO_PIN_RESET)*/

#define LCD_ROWSTRIDE (LCD_DATA_WIDTH>>3)
extern uint8_t lcd_data[LCD_ROWSTRIDE*LCD_DATA_HEIGHT];

void lcd_init();
void lcd_kill();
void lcd_clear();
void lcd_print(char *ch); // prints text, doesn't flip
void lcd_print_hex(unsigned int v); // just for debugging - print a number, doesn't flip
void lcd_println(char *ch); // prints a line and flips screen
void lcd_flip();


