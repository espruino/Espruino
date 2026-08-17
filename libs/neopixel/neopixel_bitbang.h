/* Copyright (c) 2017 Lambor Fang. All Rights Reserved.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

/* Software-based WS2812 driver - timings are designed for STM32F405 */

#ifndef NEOPIXEL_BITBANG_H__
#define NEOPIXEL_BITBANG_H__

#include "jspin.h"
#include "jshardware.h"


bool neopixelWrite_bitbang(Pin pin, unsigned char *rgbData, size_t rgbSize);


#endif  // NEOPIXEL_BITBANG_H__
