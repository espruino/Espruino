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
 * This file is designed to be parsed during the build process
 *
 * Contains JavaScript interface for Neopixel/WS281x/APA10x devices
 * ----------------------------------------------------------------------------
 */

#ifdef NRF52
#include "i2s_ws2812b_drive.h"
#endif
#ifdef ESP8266
#include <c_types.h>
#include <espmissingincludes.h>
#endif

#include <jswrap_neopixel.h>
#include "jsvariterator.h"
#include "jspininfo.h"
#include "jshardware.h"
#include "jsspi.h"

/** Send the given RGB pixel data out to neopixel/WS2811/APA104/etc */
bool neopixelWrite(Pin pin, unsigned char *rgbData, size_t rgbSize);


/*JSON{
  "type" : "library",
  "class" : "neopixel"
}
This library allows you to write to Neopixel/WS281x/APA10x LED strips

These use a high speed single-wire protocol which needs platform-specific
implementation on some devices - hence this library to simplify things.
*/

/*JSON{
  "type" : "staticmethod",
  "class" : "neopixel",
  "name" : "write",
  "generate" : "jswrap_neopixel_write",
  "params" : [
    ["pin", "pin", "The Pin the LEDs are connected to"],
    ["data","JsVar","The data to write to the LED strip"]
  ]
}
Write to NeoPixel/WS281x/APA10x-style LEDs attached to the given pin

Different types of LED have the data in different orders - so don't
be surprised by RGB or BGR orderings!

```
// set just one pixel
require("neopixel").write(B15, [255,0,0]);

// Produce an animated rainbow over 25 LEDs
var rgb = new Uint8ClampedArray(25*3);
var pos = 0;

function getPattern() {
  pos++;
  for (var i=0;i<rgb.length;) {
    rgb[i++] = (1 + Math.sin((i+pos)*0.1324)) * 127;
    rgb[i++] = (1 + Math.sin((i+pos)*0.1654)) * 127;
    rgb[i++] = (1 + Math.sin((i+pos)*0.1)) * 127;
  }
  return rgb;
}

setInterval(function() {
  require("neopixel").write(B15, getPattern());
}, 100);
```

**Note:** Espruino devices tend to have 3.3v IO, while WS2812/etc run
off of 5v. Many WS2812 will only register a logic '1' at 70%
of their input voltage - so if powering them off 5v you will not
be able to send them data reliably. You can work around this by
powering the LEDs off a lower voltage (for example 3.7v from a LiPo
battery), or can put the output into the `af_opendrain` state and use
a pullup resistor to 5v on STM32 based boards (nRF52 are not 5v tolerant
so you can't do this).
*/
void jswrap_neopixel_write(Pin pin, JsVar *data) {
  JSV_GET_AS_CHAR_ARRAY(rgbData, rgbSize, data);
  if (!rgbData) {
    jsExceptionHere(JSET_ERROR, "Couldn't convert %t to data to send to LEDs", data);
    return;
  }
  if (rgbSize == 0) {
    jsExceptionHere(JSET_ERROR, "Data must be a non empty array.");
    return;
  }
  if (rgbSize % 3 != 0) {
    jsExceptionHere(JSET_ERROR, "Data length must be a multiple of 3 (RGB).");
    return;
  }

  neopixelWrite(pin, (unsigned char *)rgbData, rgbSize);
}


// -----------------------------------------------------------------------------------
// -------------------------------------------------------------- Platform specific
// -----------------------------------------------------------------------------------

#if defined(STM32) // ----------------------------------------------------------------

// this one could potentially work on other platforms as well...
bool neopixelWrite(Pin pin, unsigned char *rgbData, size_t rgbSize) {
  if (!jshIsPinValid(pin)) {
    jsExceptionHere(JSET_ERROR, "Pin is not valid.");
    return false;
  }
  JshPinFunction spiDevice = 0;
  unsigned int i;
  for (i=0;i<JSH_PININFO_FUNCTIONS;i++) {
    if (JSH_PINFUNCTION_IS_SPI(pinInfo[pin].functions[i]) &&
        ((pinInfo[pin].functions[i] & JSH_MASK_INFO)==JSH_SPI_MOSI)) {
      spiDevice = pinInfo[pin].functions[i];
    }
  }
  IOEventFlags device = jshGetFromDevicePinFunction(spiDevice);
  if (!spiDevice || !device) {
    jsExceptionHere(JSET_ERROR, "No suitable SPI device found for this pin\n");
    return false;
  }
  JshSPIInfo inf;
  jshSPIInitInfo(&inf);
  inf.baudRate = 3200000;
  inf.pinMOSI = pin;
  jshSPISetup(device, &inf);
  jshSPISet16(device, true); // 16 bit output
  // we're just sending (no receive)
  jshSPISetReceive(device, false);
  jshInterruptOff();
  for (i=0;i<rgbSize;i++)
    jsspiSend4bit(device, rgbData[i], 1, 3);
  jshInterruptOn();
  jshSPIWait(device); // wait until SPI send finished and clear the RX buffer
  jshSPISet16(device, false); // back to 8 bit
  return true;
}


#elif defined(NRF52) // ----------------------------------------------------------------

bool neopixelWrite(Pin pin, unsigned char *rgbData, size_t rgbSize) {
#ifdef NRF52
  if (!jshIsPinValid(pin)) {
    jsExceptionHere(JSET_ERROR, "Pin is not valid.");
    return false;
  }
  return !i2s_ws2812b_drive_xfer((rgb_led_t *)rgbData, rgbSize/3, pinInfo[pin].pin);
#else
  jsExceptionHere(JSET_ERROR, "Neopixel writing not implemented");
  return false;
#endif
}


#elif defined(ESP8266) // ----------------------------------------------------------------

static inline uint32_t _getCycleCount(void) {
  uint32_t ccount;
  __asm__ __volatile__("rsr %0,ccount":"=a" (ccount));
  return ccount;
}

bool neopixelWrite(Pin pin, unsigned char *rgbData, size_t rgbSize) {
  if (!jshIsPinValid(pin)) {
    jsExceptionHere(JSET_ERROR, "Pin is not valid.");
    return false;
  }
  if (!jshGetPinStateIsManual(pin))
    jshPinSetState(pin, JSHPINSTATE_GPIO_OUT);

  // values for 160Mhz clock
  uint8_t tOne =  90;  // one bit, high typ 800ns
  uint8_t tZero = 40;  // zero bit, high typ 300ns
  uint8_t tLow = 170;  // total cycle, typ 1.2us
  if (system_get_cpu_freq() < 100) {
    tOne = 56;  // 56 cyc = 700ns
    tZero = 14; // 14 cycl = 175ns
    tLow = 100;
  }
#define _BV(bit) (1 << (bit))

#if 1

  // the loop over the RGB pixel bits below is loaded into the instruction cache from flash
  // with the result that dependeing on the cache line alignment the first loop iteration
  // takes too long and thereby messes up the first LED.
  // The fix is to make it so the first loop iteration does nothing, i.e. just outputs the
  // same "low" for the full loop as we had before entering this function. This way no LED
  // gets set to the wrong value and we load the cache line so the second iteration, i.e.,
  // first real LED bit, runs at full speed.
  uint32_t pinMask = _BV(pin);    // bit mask for GPIO pin to write to reg
  uint8_t *p = (uint8_t *)rgbData; // pointer to walk through pixel array
  uint8_t *end = p + rgbSize;  // pointer to end of array
  uint8_t pix = *p++;             // current byte being shifted out
  uint8_t mask = 0x80;            // mask for current bit
  uint32_t start;                 // start time of bit
  // adjust things for the pre-roll
  p--;                            // next byte we fetch will be the first byte again
  mask = 0x01;                    // fetch the next byte at the end of the first loop iteration
  pinMask = 0;                    // zero mask means we set or clear no I/O pin
  // iterate through all bits
  ets_intr_lock();                // disable most interrupts
  while(1) {
    uint32_t t;
    if (pix & mask) t = tOne;
    else            t = tZero;
    GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, pinMask);  // Set high
    start = _getCycleCount();                        // get start time of this bit
    while (_getCycleCount()-start < t) ;             // busy-wait
    GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, pinMask);  // Set low
    if (!(mask >>= 1)) {                             // Next bit/byte?
      if (p >= end) break;                           // at end, we're done
      pix = *p++;
      mask = 0x80;
      pinMask = _BV(pin);
    }
    while (_getCycleCount()-start < tLow) ;          // busy-wait
  }
  while (_getCycleCount()-start < tLow) ;            // wait for last bit
  ets_intr_unlock();

#else

  // this is the code without preloading the first bit
  uint32_t pinMask = _BV(pin);    // bit mask for GPIO pin to write to reg
  uint8_t *p = (uint8_t *)rgbData; // pointer to walk through pixel array
  uint8_t *end = p + rgbSize;  // pointer to end of array
  uint8_t pix = *p++;             // current byte being shifted out
  uint8_t mask = 0x80;            // mask for current bit
  uint32_t start;                 // start time of bit
  // iterate through all bits
  while(1) {
    uint32_t t;
    if (pix & mask) t = 56; // one bit, high typ 800ns (56 cyc = 700ns)
    else            t = 14; // zero bit, high typ 300ns (14 cycl = 175ns)
    GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, pinMask);  // Set high
    start = _getCycleCount();                        // get start time of this bit
    while (_getCycleCount()-start < t) ;             // busy-wait
    GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, pinMask);  // Set low
    if (!(mask >>= 1)) {                             // Next bit/byte?
      if (p >= end) break;                           // at end, we're done
      pix = *p++;
      mask = 0x80;
    }
    while (_getCycleCount()-start < 100) ;           // busy-wait, 100 cyc = 1.25us
  }
  while (_getCycleCount()-start < 100) ;             // Wait for last bit

#endif
#undef _BV
  return true;
}

#else

bool neopixelWrite(Pin pin, unsigned char *rgbData, size_t rgbSize) {
  jsExceptionHere(JSET_ERROR, "Neopixel writing not implemented");
  return false;
}

#endif
