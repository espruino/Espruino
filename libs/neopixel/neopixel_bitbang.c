/* Copyright (c) 2017 Lambor Fang. All Rights Reserved.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */
/* Software-based WS2812 driver */

#include "neopixel_bitbang.h"
#include <cmsis_core.h>

/*

PATTERN_PULSE_T0H 0.25uS
PATTERN_PULSE_T0L 1uS
PATTERN_PULSE_T1H 0.6uS
PATTERN_PULSE_T1L 0.65uS

*/

#ifdef BANGLEJS3
#define NRFX
#endif

#ifdef NRFX
#include <nrfx.h>
#include <hal/nrf_gpio.h>
#define PATTERN_PULSE_T0H() for(volatile uint32_t i = 0; i < 2; i++);
#define PATTERN_PULSE_T0L() for(volatile uint32_t i = 0; i < 11; i++);
#define PATTERN_PULSE_T1H() for(volatile uint32_t i = 0; i < 6; i++);
#define PATTERN_PULSE_T1L() for(volatile uint32_t i = 0; i < 6; i++);
#define PIN_ON() port->OUTSET = mask
#define PIN_OFF() port->OUTCLR = mask
#else
// pattern ws2812b pulses holding time for STM32F405
#define PATTERN_PULSE_T0H() for(uint32_t i = 0; i < 17; i++) {__NOP();}
#define PATTERN_PULSE_T0L() for(uint32_t i = 0; i < 33; i++) {__NOP();}
#define PATTERN_PULSE_T1H() for(uint32_t i = 0; i < 30; i++) {__NOP();}
#define PATTERN_PULSE_T1L() for(uint32_t i = 0; i < 20; i++) {__NOP();}
#define PIN_ON() jshPinSetValue(pin, true)
#define PIN_OFF() jshPinSetValue(pin, false)
#endif

#define PATTERN_0_CODE(pin)  do { \
								PIN_ON(); \
								PATTERN_PULSE_T0H() \
								PIN_OFF(); \
								PATTERN_PULSE_T0L() \
							} while(0)


#define PATTERN_1_CODE(pin) do { \
								PIN_ON(); \
								PATTERN_PULSE_T1H() \
								PIN_OFF(); \
								PATTERN_PULSE_T1L() \
							}while(0)



bool neopixelWrite_bitbang(Pin pin, unsigned char *rgbData, size_t rgbSize)
{
  uint16_t c;
  unsigned char *p = (uint8_t *)rgbData;

  if (!jshIsPinValid(pin)) {
    jsExceptionHere(JSET_ERROR, "Pin is not valid");
    return false;
  }

  // Set neopixel output pin
  jshPinSetState(pin, JSHPINSTATE_GPIO_OUT);

#ifdef NRFX
  int mask = 1 << pinInfo[pin].pin;
  NRF_GPIO_Type *ports[] = { NRF_P0, NRF_P1, NRF_P2 };
  NRF_GPIO_Type *port = ports[(pinInfo[pin].port & JSH_PORT_MASK) - JSH_PORTA];
#endif

  jshInterruptOff();
  for(c = 0; c < rgbSize; c++) {
    for(int i = 7; i >= 0; i--) {
      if(p[c] & (0x01 << i)) {
        PATTERN_1_CODE(pin);
      } else {
        PATTERN_0_CODE(pin);
      }
    }
  }
  jshDelayMicroseconds(50);
  jshInterruptOn();

  return true;
}
