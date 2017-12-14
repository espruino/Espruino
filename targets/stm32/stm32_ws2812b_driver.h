/* Copyright (c) 2017 Lambor Fang. All Rights Reserved.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

/* Software-based WS2812 driver - timings are designed for STM32F405 */

#ifndef STM32_WS2812B_DRIVER_H__
#define STM32_WS2812B_DRIVER_H__

#include "jspin.h"
#include "jshardware.h"

// pattern ws2812b pulses holding time
#define PATTERN_PULSE_T0H() for(uint32_t i = 0; i < 17; i++) {__NOP();}
#define PATTERN_PULSE_T0L() for(uint32_t i = 0; i < 33; i++) {__NOP();}
#define PATTERN_PULSE_T1H() for(uint32_t i = 0; i < 30; i++) {__NOP();}
#define PATTERN_PULSE_T1L() for(uint32_t i = 0; i < 20; i++) {__NOP();}

#define PATTERN_0_CODE(pin)  do { \
								jshPinSetValue(pin, true); \
								PATTERN_PULSE_T0H() \
								jshPinSetValue(pin, false); \
								PATTERN_PULSE_T0L() \
							} while(0)


#define PATTERN_1_CODE(pin) do { \
								jshPinSetValue(pin, true); \
								PATTERN_PULSE_T1H() \
								jshPinSetValue(pin, false); \
								PATTERN_PULSE_T1L() \
							}while(0)	


bool stm32_neopixelWrite(Pin pin, unsigned char *rgbData, size_t rgbSize);


#endif  // STM32_WS2812B_DRIVER_H__
