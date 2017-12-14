/* Copyright (c) 2017 Lambor Fang. All Rights Reserved.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */
/* Software-based WS2812 driver */

#include "stm32_ws2812b_driver.h"

bool stm32_neopixelWrite(Pin pin, unsigned char *rgbData, size_t rgbSize)
{
  uint16_t c;
  unsigned char *p = (uint8_t *)rgbData;

  if (!jshIsPinValid(pin)) {
    jsExceptionHere(JSET_ERROR, "Pin is not valid.");
    return false;
  }

  // Set neopixel output pin 
  jshPinSetState(pin, JSHPINSTATE_GPIO_OUT);
  
  jshInterruptOff();
  for(c = 0; c < rgbSize; c++) {
    for(int i = 7; i >= 0; i--)
    {
      if(p[c] & (0x01 << i)) {
        PATTERN_1_CODE(pin);
      } 
      else {
        PATTERN_0_CODE(pin);
      }    
    }
  }
  jshDelayMicroseconds(50);
  jshInterruptOn();

  return true;
}
