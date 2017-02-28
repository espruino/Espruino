/* Copyright (c) 2016 Takafumi Naka. All Rights Reserved.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

#ifndef I2S_WS2812B_DRIVER_H__
#define I2S_WS2812B_DRIVER_H__

#include <stdio.h>
#include "nrf_drv_i2s.h"

typedef struct
{
    uint8_t   green; // Brightness of green (0 to 255)
    uint8_t   red;   // Brightness of red   (0 to 255)
    uint8_t   blue;  // Brightness of blue  (0 to 255)
} rgb_led_t;


#define I2S_WS2812B_DRIVE_PATTERN_0 ((uint8_t)0x08)			// Bit pattern for data "0" is "HLLL".
#define I2S_WS2812B_DRIVE_PATTERN_1 ((uint8_t)0x0e)      // Bit pattern for data "1" is "HHHL".
#define	I2S_WS2812B_DRIVE_BUF_SIZE_PER_LED	(12)	// buffer size for each LED (8bit * 4 * 3 )

void i2s_ws2812b_drive_set_buff(rgb_led_t* rgb_led, uint8_t *p_xfer, uint16_t xbuff_length);

ret_code_t i2s_ws2812b_drive_xfer(rgb_led_t *led_array, uint16_t num_leds, uint8_t drive_pin);

#endif // I2S_WS2812B_DRIVER_H__
