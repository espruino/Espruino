/* Copyright (c) 2017 Lambor Fang. All Rights Reserved.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

#ifndef STM32_WS2812B_DRIVER_H__
#define STM32_WS2812B_DRIVER_H__

#include "jspin.h"
#include "jshardware.h"

// 18 33
#define PATERN_PULSE_T0H() for(uint32_t i = 0; i < 17; i++) {__NOP();}
#define PATERN_PULSE_T0L() for(uint32_t i = 0; i < 33; i++) {__NOP();}
#define PATERN_PULSE_T1H() for(uint32_t i = 0; i < 30; i++) {__NOP();}
#define PATERN_PULSE_T1L() for(uint32_t i = 0; i < 20; i++) {__NOP();}

#define PATERN_0_CODE(pin)  do { \
								jshPinSetValue(pin, true); \
								PATERN_PULSE_T0H() \
								jshPinSetValue(pin, false); \
								PATERN_PULSE_T0L() \
							} while(0)


#define PATERN_1_CODE(pin) do { \
								jshPinSetValue(pin, true); \
								PATERN_PULSE_T1H() \
								jshPinSetValue(pin, false); \
								PATERN_PULSE_T1L() \
							}while(0)										

#endif  // STM32_WS2812B_DRIVER_H__