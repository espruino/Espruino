/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2015 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * This file is designed to be parsed during the build process
 *
 * Contains ESP8266 board specific functions.
 * ----------------------------------------------------------------------------
 */

/* DO_NOT_INCLUDE_IN_DOCS - this is a special token for common.py,
 so we don't put this into espruino.com/Reference until this is out
 of beta.  */

// Because the ESP8266 JS wrapper is assured to be running on an ESP8266 we
// can assume that inclusion of ESP8266 headers will be acceptable.
#include <c_types.h>
#include <user_interface.h>
#include <mem.h>
#include <osapi.h>
#include <ping.h>
#include <espconn.h>
#include <espmissingincludes.h>
#include <uart.h>

#define _GCC_WRAP_STDINT_H
typedef long long int64_t;

#include <jswrap_esp8266.h>
#include "jsinteractive.h" // Pull inn the jsiConsolePrint function

#define _BV(bit) (1 << (bit))

static uint32_t _getCycleCount(void) __attribute__((always_inline));

static inline uint32_t _getCycleCount(void) {
  uint32_t ccount;
  __asm__ __volatile__("rsr %0,ccount":"=a" (ccount));
  return ccount;
}

/*JSON{
 "type"     : "staticmethod",
 "class"    : "ESP8266",
 "name"     : "neopixelWrite",
 "generate" : "jswrap_ESP8266_neopixelWrite",
 "params"   : [
   ["pin", "pin", "Pin for output signal."],
   ["arrayOfData", "JsVar", "Array of LED data."]
 ]
}*/
__attribute__((section(".force.text"))) void jswrap_ESP8266_neopixelWrite(Pin pin, JsVar *jsArrayOfData) {
  if (!jshIsPinValid(pin)) {
    jsExceptionHere(JSET_ERROR, "Pin is not valid.");
    return;
  }
  if (jsArrayOfData == NULL) {
    jsExceptionHere(JSET_ERROR, "No data to send to LEDs.");
    return;
  }
  if (!jsvIsArray(jsArrayOfData)) {
    jsExceptionHere(JSET_ERROR, "Data must be an array.");
    return;
  }

  JSV_GET_AS_CHAR_ARRAY(pixels, dataLength, jsArrayOfData);
  if (!pixels) {
    return;
  }

  if (dataLength == 0) {
    jsExceptionHere(JSET_ERROR, "Data must be a non empty array.");
    return;
  }
  if (dataLength % 3 != 0) {
    jsExceptionHere(JSET_ERROR, "Data length must multiples of RGB bytes (3).");
    return;
  }

  for (size_t i=0; i<dataLength; i++) {
    JsVar *jsItem = jsvGetArrayItem(jsArrayOfData, i);
    pixels[i] = jsvGetInteger(jsItem);
    jsvUnLock(jsItem);
  }

  uint32_t numBytes = dataLength;

  uint8_t *p, *end, pix, mask;
  uint32_t t, time0, time1, period, c, startTime, pinMask;
  pinMask = _BV(pin);
  p = (uint8_t *)pixels;
  end = p + numBytes;
  pix = *p++;
  mask = 0x80;
  c=0;
  startTime = 0;
  time0 = 14; // 14 cycles = (measured)
  //time0 = 28; // 28 cycles = 0.35us
  //time0 = 32; // 0.4us
  //time1 = 108; // 108 cycles = 1.36us
  time1 = 56; // 56 cycles = 0.7us
  //time1 = 64; // 64 cycles = 0.8us
  // Cycles/usec = 80
  // Period = cycles/usec * usecDuration
  //period = 136; // 136 cycles = 1.71us
  period = 100; // cycles = 1.25us
  //period = 104; // 1.3us
  while(1) {
    if (pix & mask)
      t = time1; // Bit high duration
    else
      t = time0;
    GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, pinMask);  // Set high
    startTime = _getCycleCount();                    // Save start time
    while (((c = _getCycleCount()) - startTime) < t)
      ;      // Wait high duration
    GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, pinMask);  // Set low
    if (!(mask >>= 1)) {                             // Next bit/byte
      if (p >= end)
        break;
      pix = *p++;
      mask = 0x80;
    }
    while (((c = _getCycleCount()) - startTime) < period)
      ; // Wait for bit start
  }
  while ((_getCycleCount() - startTime) < period)
    ; // Wait for last bit
}
