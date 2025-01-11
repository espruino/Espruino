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
 * JavaScript Hardware IO Functions
 * ----------------------------------------------------------------------------
 */
#include "jswrap_io.h"
#include "jsvar.h"
#include "jswrap_arraybuffer.h" // for jswrap_io_peek
#include "jswrapper.h" // for JSWAT_VOID
#include "jstimer.h" // for digitalPulse
#include "jspin.h"
#include <stdio.h>

#ifdef ESP32
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#endif

#ifdef USE_LCD_SDL
#include <SDL/SDL.h>

static unsigned int SDL_Backdoor(int x) {
  static SDL_Event event;
  switch (x) {
  case 0: return SDL_PollEvent(&event);
  case 1: return event.type;
  case 2: return (int) event.key.keysym.scancode;
  case 3: return (int) event.button.button;
  case 4: return (int) event.button.state;
  case 5: return (int) event.button.x;
  case 6: return (int) event.button.y;
  }
  printf("Bad backdoor call %d\n", x); fflush(stdout);
  return -1;
}
#endif

/*JSON{
  "type"          : "function",
  "name"          : "peek8",
  "generate_full" : "jswrap_io_peek(addr,count,1)",
  "params"        : [
    ["addr", "int", "The address in memory to read"],
    ["count", "int", "[optional] the number of items to read. If >1 a `Uint8Array` will be returned."]
  ],
  "return"        : ["JsVar","The value of memory at the given location"],
  "typescript"    : [
    "declare function peek8(addr: number, count?: 1): number;",
    "declare function peek8(addr: number, count: number): Uint8Array;"
  ]
}
Read 8 bits of memory at the given location - DANGEROUS!
 */
/*JSON{
  "type"          : "function",
  "name"          : "poke8",
  "generate_full" : "jswrap_io_poke(addr,value,1)",
  "params" : [
    ["addr","int","The address in memory to write"],
    ["value","JsVar","The value to write, or an array of values"]
  ],
  "typescript"    : "declare function poke8(addr: number, value: number | number[]): void;"
}
Write 8 bits of memory at the given location - VERY DANGEROUS!
 */
/*JSON{
  "type" : "function",
  "name" : "peek16",
  "generate_full" : "jswrap_io_peek(addr,count,2)",
  "params" : [
    ["addr","int","The address in memory to read"],
    ["count","int","[optional] the number of items to read. If >1 a `Uint16Array` will be returned."]
  ],
  "return" : ["JsVar","The value of memory at the given location"],
  "typescript" : [
    "declare function peek16(addr: number, count?: 1): number;",
    "declare function peek16(addr: number, count: number): Uint8Array;"
  ]
}
Read 16 bits of memory at the given location - DANGEROUS!
 */
/*JSON{
  "type" : "function",
  "name" : "poke16",
  "generate_full" : "jswrap_io_poke(addr,value,2)",
  "params" : [
    ["addr","int","The address in memory to write"],
    ["value","JsVar","The value to write, or an array of values"]
  ],
  "typescript" : "declare function poke16(addr: number, value: number | number[]): void;"
}
Write 16 bits of memory at the given location - VERY DANGEROUS!
 */
/*JSON{
  "type" : "function",
  "name" : "peek32",
  "generate_full" : "jswrap_io_peek(addr,count,4)",
  "params" : [
    ["addr","int","The address in memory to read"],
    ["count","int","[optional] the number of items to read. If >1 a `Uint32Array` will be returned."]
  ],
  "return" : ["JsVar","The value of memory at the given location"],
  "typescript" : [
    "declare function peek32(addr: number, count?: 1): number;",
    "declare function peek32(addr: number, count: number): Uint8Array;"
  ]
}
Read 32 bits of memory at the given location - DANGEROUS!
 */
/*JSON{
  "type" : "function",
  "name" : "poke32",
  "generate_full" : "jswrap_io_poke(addr,value,4)",
  "params" : [
    ["addr","int","The address in memory to write"],
    ["value","JsVar","The value to write, or an array of values"]
  ],
  "typescript" : "declare function poke32(addr: number, value: number | number[]): void;"
}
Write 32 bits of memory at the given location - VERY DANGEROUS!
 */
uint32_t _jswrap_io_peek(size_t addr, int wordSize) {
  if (wordSize==1) return READ_FLASH_UINT8((char*)addr);
  if (wordSize==2) {
    return READ_FLASH_UINT8((char*)addr) | (uint32_t)(READ_FLASH_UINT8((char*)(addr+1)) << 8);
  }
  if (wordSize==4) return (uint32_t)*(unsigned int*)addr;
  return 0;
}

JsVar *jswrap_io_peek(JsVarInt addr, JsVarInt count, int wordSize) {
#ifdef USE_LCD_SDL
	/* HERE */
	if (wordSize != 1) return ~0;
	uint32_t ret = SDL_Backdoor(addr);
	//printf("io_peek %x %x -> %x\n", addr, wordSize, ret);  fflush(stdout);
	return jsvNewFromLongInteger(ret);
#endif
  // hack for ESP8266/ESP32 where the address can be different
  size_t mappedAddr = jshFlashGetMemMapAddress((size_t)addr);
  if (count<=1) {
    return jsvNewFromLongInteger((long long)_jswrap_io_peek(mappedAddr, wordSize));
  } else {
    JsVarDataArrayBufferViewType aType;
    if (wordSize==1) aType=ARRAYBUFFERVIEW_UINT8;
    if (wordSize==2) aType=ARRAYBUFFERVIEW_UINT16;
    if (wordSize==4) aType=ARRAYBUFFERVIEW_UINT32;
    JsVar *arr = jsvNewTypedArray(aType, count);
    if (!arr) return 0;
    JsvArrayBufferIterator it;
    jsvArrayBufferIteratorNew(&it, arr, 0);
    while (jsvArrayBufferIteratorHasElement(&it)) {
      jsvArrayBufferIteratorSetIntegerValue(&it, (JsVarInt)_jswrap_io_peek(mappedAddr, wordSize));
      mappedAddr += (size_t)wordSize;
      jsvArrayBufferIteratorNext(&it);
    }
    jsvArrayBufferIteratorFree(&it);
    return arr;
  }
}

void _jswrap_io_poke(JsVarInt addr, uint32_t data, int wordSize) {
	/* HERE */
  printf("io_poke %x %x %x\n", addr, data, wordSize);
  fflush(stdout);
  return 1337;
	
  if (wordSize==1) (*(unsigned char*)(size_t)addr) = (unsigned char)data;
  else if (wordSize==2) (*(unsigned short*)(size_t)addr) = (unsigned short)data;
  else if (wordSize==4) (*(unsigned int*)(size_t)addr) = (unsigned int)data;
}

void jswrap_io_poke(JsVarInt addr, JsVar *data, int wordSize) {
  if (jsvIsNumeric(data)) {
    _jswrap_io_poke(addr, (uint32_t)jsvGetInteger(data), wordSize);
  } else if (jsvIsIterable(data)) {
    JsvIterator it;
    jsvIteratorNew(&it, data, JSIF_EVERY_ARRAY_ELEMENT);
    while (jsvIteratorHasElement(&it)) {
      _jswrap_io_poke(addr, (uint32_t)jsvIteratorGetIntegerValue(&it), wordSize);
      addr += wordSize;
      jsvIteratorNext(&it);
    }
    jsvIteratorFree(&it);
  }
}


/*JSON{
  "type" : "function",
  "name" : "analogRead",
  "generate" : "jshPinAnalog",
  "params" : [
    ["pin","pin",["The pin to use","You can find out which pins to use by looking at [your board's reference page](#boards) and searching for pins with the `ADC` markers."]]
  ],
  "return" : ["float","The Analog Value of the Pin between 0(GND) and 1(VCC). See below."]
}
Get the analogue value of the given pin.

This is different to Arduino which only returns an integer between 0 and 1023

However only pins connected to an ADC will work (see the datasheet)

**Note:** if you didn't call `pinMode` beforehand then this function will also
reset pin's state to `"analog"`

**Note:** [Jolt.js](https://www.espruino.com/Jolt.js) motor driver pins with
analog inputs are scaled with a potential divider, and so those pins return a
number which is the actual voltage.
 */
/*JSON{
  "type" : "function",
  "name" : "analogWrite",
  "generate" : "jswrap_io_analogWrite",
  "params" : [
    ["pin","pin",["The pin to use","You can find out which pins to use by looking at [your board's reference page](#boards) and searching for pins with the `PWM` or `DAC` markers."]],
    ["value","float","A value between 0 and 1"],
    ["options","JsVar",["An object containing options for analog output - see below"]]
  ],
  "typescript" : "declare function analogWrite(pin: Pin, value: number, options?: { freq?: number, soft?: boolean, forceSoft?: boolean }): void;"
}
Set the analog Value of a pin. It will be output using PWM.

Objects can contain:

* `freq` - pulse frequency in Hz, e.g. ```analogWrite(A0,0.5,{ freq : 10 });``` -
  specifying a frequency will force PWM output, even if the pin has a DAC
* `soft` - boolean, If true software PWM is used if hardware is not available.
* `forceSoft` - boolean, If true software PWM is used even if hardware PWM or a
  DAC is available

On nRF52-based devices (Puck.js, Pixl.js, MDBT42Q, etc) hardware PWM runs at
16MHz, with a maximum output frequency of 4MHz (but with only 2 bit (0..3) accuracy).
At 1Mhz, you have 4 bits (0..15), 1kHz = 14 bits and so on.

 **Note:** if you didn't call `pinMode` beforehand then this function will also
 reset pin's state to `"output"`
 */
void jswrap_io_analogWrite(Pin pin, JsVarFloat value, JsVar *options) {
  JsVarFloat freq = 0;
  JshAnalogOutputFlags flags = JSAOF_NONE;
  if (jsvIsObject(options)) {
    freq = jsvObjectGetFloatChild(options, "freq");
    if (jsvObjectGetBoolChild(options, "forceSoft"))
          flags |= JSAOF_FORCE_SOFTWARE;
    else if (jsvObjectGetBoolChild(options, "soft"))
      flags |= JSAOF_ALLOW_SOFTWARE;
  }

  jshPinAnalogOutput(pin, value, freq, flags);
}

/*JSON{
  "type" : "function",
  "name" : "digitalPulse",
  "generate" : "jswrap_io_digitalPulse",
  "params" : [
    ["pin","pin","The pin to use"],
    ["value","bool","Whether to pulse high (true) or low (false)"],
    ["time","JsVar","A time in milliseconds, or an array of times (in which case a square wave will be output starting with a pulse of 'value')"]
  ],
  "typescript" : "declare function digitalPulse(pin: Pin, value: boolean, time: number | number[]): void;"
}
Pulse the pin with the value for the given time in milliseconds. It uses a
hardware timer to produce accurate pulses, and returns immediately (before the
pulse has finished). Use `digitalPulse(A0,1,0)` to wait until a previous pulse
has finished.

e.g. `digitalPulse(A0,1,5);` pulses A0 high for 5ms.
`digitalPulse(A0,1,[5,2,4]);` pulses A0 high for 5ms, low for 2ms, and high for
4ms

 **Note:** if you didn't call `pinMode` beforehand then this function will also
 reset pin's state to `"output"`

digitalPulse is for SHORT pulses that need to be very accurate. If you're doing
anything over a few milliseconds, use setTimeout instead.
 */
void jswrap_io_digitalPulse(Pin pin, bool value, JsVar *times) {
  if (!jshIsPinValid(pin)) {
    jsExceptionHere(JSET_ERROR, "Invalid pin");
    return;
  }
  // check for currently running timer tasks
  UtilTimerTask task;
  uint32_t timerOffset = jstGetUtilTimerOffset();
  bool hasTimer = jstGetLastPinTimerTask(pin, &task);
  if (!hasTimer) task.time = 0;
  // now start either one or a series of pulses
  if (jsvIsNumeric(times)) {
    JsVarFloat pulseTime = jsvGetFloat(times);
    if (pulseTime<0 || isnan(pulseTime)) {
      jsExceptionHere(JSET_ERROR, "Pulse Time is less than 0 or not a number");
    } else if (pulseTime>0) {
      if (!hasTimer) jshPinOutput(pin, value);
      task.time += jshGetTimeFromMilliseconds(pulseTime);
      jstPinOutputAtTime(task.time, &timerOffset, &pin, 1, !value);
    } else jstUtilTimerWaitEmpty(); // time==0
  } else if (jsvIsIterable(times)) {
    // iterable, so output a square wave
    if (!hasTimer) jshPinOutput(pin, value);
    JsvIterator it;
    jsvIteratorNew(&it, times, JSIF_EVERY_ARRAY_ELEMENT);
    while (jsvIteratorHasElement(&it)) {
      JsVarFloat pulseTime = jsvIteratorGetFloatValue(&it);
      if (!isnan(pulseTime)) {
        if (pulseTime>0) {
          task.time += jshGetTimeFromMilliseconds(pulseTime);
          jstPinOutputAtTime(task.time, &timerOffset, &pin, 1, !value);
	}
      }
      value = !value;
      jsvIteratorNext(&it);
    }
    jsvIteratorFree(&it);
  } else {
    jsExceptionHere(JSET_ERROR, "Expecting Number or Array, got %t", times);
  }
}

/*JSON{
  "type"     : "function",
  "name"     : "digitalWrite",
  "generate" : "jswrap_io_digitalWrite",
  "params"   : [
    ["pin",   "JsVar","The pin to use"],
    ["value", "JsVar","Whether to write a high (true) or low (false) value"]
  ],
  "typescript" : "declare function digitalWrite(pin: Pin, value: boolean): void;"
}
Set the digital value of the given pin.

```
digitalWrite(LED1, 1); // light LED1
digitalWrite([LED1,LED2,LED3], 0b101); // lights LED1 and LED3
```

 **Note:** if you didn't call `pinMode(pin, ...)` or `Pin.mode(...)` beforehand then this function will also
reset pin's state to `"output"`

If pin argument is an array of pins (e.g. `[A2,A1,A0]`) the value argument will
be treated as an array of bits where the last array element is the least
significant bit.

In this case, pin values are set least significant bit first (from the
right-hand side of the array of pins). This means you can use the same pin
multiple times, for example `digitalWrite([A1,A1,A0,A0],0b0101)` would pulse A0
followed by A1.

In 2v22 and later firmwares, using a boolean for the value will set *all* pins in
the array to the same value, eg `digitalWrite(pins, value?0xFFFFFFFF:0)`. Previously
digitalWrite with a boolean behaved like `digitalWrite(pins, value?1:0)` and would
only set the first pin.

If the pin argument is an object with a `write` method, the `write` method will
be called with the value passed through.
*/
void jswrap_io_digitalWrite(
    JsVar *pinVar, //!< A pin or pins.
    JsVar *valueVar //!< The value of the output.
  ) {
  JsVarInt value;
  if (jsvIsBoolean(valueVar)) value = jsvGetBool(valueVar) ? 0xFFFFFFFF : 0;
  else value = jsvGetInteger(valueVar);
  // Handle the case where it is an array of pins.
  if (jsvIsArray(pinVar)) {
    JsVarRef pinName = jsvGetLastChild(pinVar); // NOTE: start at end and work back!
    while (pinName) {
      JsVar *pinNamePtr = jsvLock(pinName);
      JsVar *pinPtr = jsvSkipName(pinNamePtr);
      jshPinOutput(jshGetPinFromVar(pinPtr), value&1);
      jsvUnLock(pinPtr);
      pinName = jsvGetPrevSibling(pinNamePtr);
      jsvUnLock(pinNamePtr);
      value = value>>1; // next bit down
    }
  } else if (jsvIsObject(pinVar)) {
    JsVar *w = jspGetNamedField(pinVar, "write", false);
    if (jsvIsFunction(w)) {
      JsVar *v = jsvNewFromInteger(value);
      jsvUnLock2(jspeFunctionCall(w,0,pinVar,false,1,&v), v);
    } else jsExceptionHere(JSET_ERROR, "Invalid pin");
    jsvUnLock(w);
  } else {
    // Handle the case where it is a single pin.
    Pin pin = jshGetPinFromVar(pinVar);
    jshPinOutput(pin, value != 0);
  }
}


/*JSON{
  "type"     : "function",
  "name"     : "digitalRead",
  "generate" : "jswrap_io_digitalRead",
  "params"   : [
    ["pin","JsVar","The pin to use"]
  ],
  "return"   : ["int","The digital Value of the Pin"],
  "typescript" : "declare function digitalRead(pin: Pin): number;"
}
Get the digital value of the given pin.

 **Note:** if you didn't call `pinMode` beforehand then this function will also
 reset pin's state to `"input"`

If the pin argument is an array of pins (e.g. `[A2,A1,A0]`) the value returned
will be an number where the last array element is the least significant bit, for
example if `A0=A1=1` and `A2=0`, `digitalRead([A2,A1,A0]) == 0b011`

If the pin argument is an object with a `read` method, the `read` method will be
called and the integer value it returns passed back.
*/
JsVarInt jswrap_io_digitalRead(JsVar *pinVar) {
  // Hadnle the case where it is an array of pins.
  if (jsvIsArray(pinVar)) {
    int pins = 0;
    JsVarInt value = 0;
    JsvObjectIterator it;
    jsvObjectIteratorNew(&it, pinVar);
    while (jsvObjectIteratorHasValue(&it)) {
      JsVar *pinPtr = jsvObjectIteratorGetValue(&it);
      value = (value<<1) | (JsVarInt)jshPinInput(jshGetPinFromVar(pinPtr));
      jsvUnLock(pinPtr);
      jsvObjectIteratorNext(&it);
      pins++;
    }
    jsvObjectIteratorFree(&it);
    if (pins==0) return 0; // return undefined if array empty
    return value;
  } else if (jsvIsObject(pinVar)) {
    JsVarInt v = 0;
    JsVar *r = jspGetNamedField(pinVar, "read", false);
    if (jsvIsFunction(r)) {
      v = jsvGetIntegerAndUnLock(jspeFunctionCall(r,0,pinVar,false,0,0));
    } else jsExceptionHere(JSET_ERROR, "Invalid pin");
    jsvUnLock(r);
    return v;
  } else {
    // Handle the case where it is a single pin.
    Pin pin = jshGetPinFromVar(pinVar);
    return jshPinInput(pin);
  }
}

/*TYPESCRIPT
type PinMode =
  | "analog"
  | "input"
  | "input_pullup"
  | "input_pulldown"
  | "output"
  | "opendrain"
  | "af_output"
  | "af_opendrain";
*/
/*JSON{
  "type"     : "function",
  "name"     : "pinMode",
  "generate" : "jswrap_io_pinMode",
  "params"   : [
    ["pin", "pin", "The pin to set pin mode for"],
    ["mode", "JsVar", "The mode - a string that is either 'analog', 'input', 'input_pullup', 'input_pulldown', 'output', 'opendrain', 'af_output' or 'af_opendrain'. Do not include this argument or use 'auto' if you want to revert to automatic pin mode setting."],
    ["automatic", "bool", "Optional, default is false. If true, subsequent commands will automatically change the state (see notes below)"]
  ],
  "typescript" : "declare function pinMode(pin: Pin, mode?: PinMode | \"auto\", automatic?: boolean): void;"
}
Set the mode of the given pin.

 * `auto`/`undefined` - Don't change state, but allow `digitalWrite`/etc to
   automatically change state as appropriate
 * `analog` - Analog input
 * `input` - Digital input
 * `input_pullup` - Digital input with internal ~40k pull-up resistor
 * `input_pulldown` - Digital input with internal ~40k pull-down resistor
 * `output` - Digital output
 * `opendrain` - Digital output that only ever pulls down to 0v. Sending a
   logical `1` leaves the pin open circuit
 * `opendrain_pullup` - Digital output that pulls down to 0v. Sending a logical
   `1` enables internal ~40k pull-up resistor
 * `af_output` - Digital output from built-in peripheral
 * `af_opendrain` - Digital output from built-in peripheral that only ever pulls
   down to 0v. Sending a logical `1` leaves the pin open circuit

 **Note:** `digitalRead`/`digitalWrite`/etc set the pin mode automatically
*unless* `pinMode` has been called first. If you want `digitalRead`/etc to set
the pin mode automatically after you have called `pinMode`, simply call it again
with no mode argument (`pinMode(pin)`), `auto` as the argument (`pinMode(pin,
"auto")`), or with the 3rd 'automatic' argument set to true (`pinMode(pin,
"output", true)`).
*/
void jswrap_io_pinMode(
    Pin pin,
    JsVar *mode,
    bool automatic
  ) {
  if (!jshIsPinValid(pin)) {
    jsExceptionHere(JSET_ERROR, "Invalid pin");
    return;
  }
  JshPinState m = JSHPINSTATE_UNDEFINED;
  if (jsvIsString(mode)) {
    if (jsvIsStringEqual(mode, "analog"))              m = JSHPINSTATE_ADC_IN;
    else if (jsvIsStringEqual(mode, "input"))          m = JSHPINSTATE_GPIO_IN;
    else if (jsvIsStringEqual(mode, "input_pullup"))   m = JSHPINSTATE_GPIO_IN_PULLUP;
    else if (jsvIsStringEqual(mode, "input_pulldown")) m = JSHPINSTATE_GPIO_IN_PULLDOWN;
    else if (jsvIsStringEqual(mode, "output"))         m = JSHPINSTATE_GPIO_OUT;
    else if (jsvIsStringEqual(mode, "opendrain"))      m = JSHPINSTATE_GPIO_OUT_OPENDRAIN;
    else if (jsvIsStringEqual(mode, "opendrain_pullup")) m = JSHPINSTATE_GPIO_OUT_OPENDRAIN_PULLUP;
    else if (jsvIsStringEqual(mode, "af_output"))      m = JSHPINSTATE_AF_OUT;
    else if (jsvIsStringEqual(mode, "af_opendrain"))   m = JSHPINSTATE_AF_OUT_OPENDRAIN;
  }
  if (m != JSHPINSTATE_UNDEFINED) {
    jshSetPinStateIsManual(pin, !automatic);
    jshPinSetState(pin, m);
  } else {
    jshSetPinStateIsManual(pin, false);
    if (!jsvIsUndefined(mode) && !jsvIsStringEqual(mode,"auto")) {
      jsExceptionHere(JSET_ERROR, "Unknown pin mode");
    }
  }
}

/*JSON{
  "type" : "function",
  "ifndef" : "SAVE_ON_FLASH",
  "name" : "getPinMode",
  "generate" : "jswrap_io_getPinMode",
  "params" : [
    ["pin","pin","The pin to check"]
  ],
  "return" : ["JsVar","The pin mode, as a string"],
  "typescript" : "declare function getPinMode(pin: Pin): PinMode;"
}
Return the current mode of the given pin. See `pinMode` for more information on
returned values.
 */
JsVar *jswrap_io_getPinMode(Pin pin) {
  if (!jshIsPinValid(pin)) {
    jsExceptionHere(JSET_ERROR, "Invalid pin");
    return 0;
  }
  return jshGetPinStateString(jshPinGetState(pin));
}


#define jswrap_io_shiftOutDataMax 8
typedef struct {
  Pin pins[jswrap_io_shiftOutDataMax];
  Pin clk;
#ifdef STM32
  volatile uint32_t *addrs[jswrap_io_shiftOutDataMax];
  volatile uint32_t *clkAddr;
#endif
  bool clkPol; // clock polarity

  int cnt; // number of pins
  int repeat; // iterations to perform per array item
} jswrap_io_shiftOutData;

void jswrap_io_shiftOutCallback(int val, void *data) {
  jswrap_io_shiftOutData *d = (jswrap_io_shiftOutData*)data;
  int n, i;
  for (i=0;i<d->repeat;i++) {
    for (n=d->cnt-1; n>=0; n--) {
  #ifdef STM32
      if (d->addrs[n])
        *d->addrs[n] = val&1;
  #else
      if (jshIsPinValid(d->pins[n]))
          jshPinSetValue(d->pins[n], val&1);
  #endif
      val>>=1;
    }
#ifdef STM32
    if (d->clkAddr) {
        *d->clkAddr = d->clkPol;
        *d->clkAddr = !d->clkPol;
    }
#else
    if (jshIsPinValid(d->clk)) {
      jshPinSetValue(d->clk, d->clkPol);
      jshPinSetValue(d->clk, !d->clkPol);
    }
#endif
  }
}

/*JSON{
  "type" : "function",
  "name" : "shiftOut",
  "generate" : "jswrap_io_shiftOut",
  "params" : [
    ["pins","JsVar","A pin, or an array of pins to use"],
    ["options","JsVar","Options, for instance the clock (see below)"],
    ["data","JsVar","The data to shift out (see `E.toUint8Array` for info on the forms this can take)"]
  ],
  "typescript" : "declare function shiftOut(pins: Pin | Pin[], options: { clk?: Pin, clkPol?: boolean, repeat?: number }, data: Uint8ArrayResolvable): void;"
}
Shift an array of data out using the pins supplied *least significant bit
first*, for example:

```
// shift out to single clk+data
shiftOut(A0, { clk : A1 }, [1,0,1,0]);
```

```
// shift out a whole byte (like software SPI)
shiftOut(A0, { clk : A1, repeat: 8 }, [1,2,3,4]);
```

```
// shift out via 4 data pins
shiftOut([A3,A2,A1,A0], { clk : A4 }, [1,2,3,4]);
```

`options` is an object of the form:

```
{
  clk : pin, // a pin to use as the clock (undefined = no pin)
  clkPol : bool, // clock polarity - default is 0 (so 1 normally, pulsing to 0 to clock data in)
  repeat : int, // number of clocks per array item
}
```

Each item in the `data` array will be output to the pins, with the first pin in
the array being the MSB and the last the LSB, then the clock will be pulsed in
the polarity given.

`repeat` is the amount of times shift data out for each array item. For instance
we may want to shift 8 bits out through 2 pins - in which case we need to set
repeat to 4.
 */
void jswrap_io_shiftOut(JsVar *pins, JsVar *options, JsVar *data) {
  jswrap_io_shiftOutData d;
  d.cnt = 0;
  d.clk = PIN_UNDEFINED;
  d.clkPol = 0;
  d.repeat = 1;

  jsvConfigObject configs[] = {
      {"clk", JSV_PIN, &d.clk},
      {"clkPol", JSV_BOOLEAN, &d.clkPol},
      {"repeat", JSV_INTEGER, &d.repeat}
  };
  if (!jsvReadConfigObject(options, configs, sizeof(configs) / sizeof(jsvConfigObject))) {
    return; // do nothing - error already displayed by jsvReadConfigObject
  }
  d.clkPol = d.clkPol?1:0;
  if (d.repeat<1) d.repeat=1;

  if (jsvIsArray(pins)) {
    JsvObjectIterator it;
    jsvObjectIteratorNew(&it, pins);
    while (jsvObjectIteratorHasValue(&it)) {
      if (d.cnt>=jswrap_io_shiftOutDataMax) {
        jsExceptionHere(JSET_ERROR, "Too many pins! %d Maximum", jswrap_io_shiftOutDataMax);
        return;
      }
      d.pins[d.cnt] = jshGetPinFromVarAndUnLock(jsvObjectIteratorGetValue(&it));
      d.cnt++;
      jsvObjectIteratorNext(&it);

    }
    jsvObjectIteratorFree(&it);
  } else {
    d.pins[d.cnt++] = jshGetPinFromVar(pins);
  }

  // Set pins as outputs
  int i;
  for (i=0;i<d.cnt;i++) {
    if (jshIsPinValid(d.pins[i])) {
      if (!jshGetPinStateIsManual(d.pins[i]))
        jshPinSetState(d.pins[i], JSHPINSTATE_GPIO_OUT);
    }
    // on STM32, try and get the pin's output address
#ifdef STM32
    d.addrs[i] = jshGetPinAddress(d.pins[i], JSGPAF_OUTPUT);
#endif
  }
#ifdef STM32
  d.clkAddr = jshGetPinAddress(d.clk, JSGPAF_OUTPUT);
#endif
  if (jshIsPinValid(d.clk))
    jshPinSetState(d.clk, JSHPINSTATE_GPIO_OUT);

  // Now run through the data, pushing it out
#ifdef ESP32
  vTaskSuspendAll();
#endif
  jsvIterateCallback(data, jswrap_io_shiftOutCallback, &d);
#ifdef ESP32
  xTaskResumeAll();
#endif
}

/*JSON{
  "type" : "function",
  "name" : "setWatch",
  "generate" : "jswrap_interface_setWatch",
  "params" : [
    ["function", "JsVar", "A Function or String to be executed"],
    ["pin", "pin", "The pin to watch"],
    ["options", "JsVar","If a boolean or integer, it determines whether to call this once (false = default) or every time a change occurs (true). Can be an object of the form `{ repeat: true/false(default), edge:'rising'/'falling'/'both', debounce:10}` - see below for more information."]
  ],
  "return" : ["JsVar","An ID that can be passed to clearWatch"],
  "typescript" : "declare function setWatch(func: ((arg: { state: boolean, time: number, lastTime: number }) => void) | string, pin: Pin, options?: boolean | { repeat?: boolean, edge?: \"rising\" | \"falling\" | \"both\", debounce?: number, irq?: boolean, data?: Pin, hispeed?: boolean }): number;"
}
Call the function specified when the pin changes. Watches set with `setWatch`
can be removed using `clearWatch`.

If the `options` parameter is an object, it can contain the following
information (all optional):

```
{
   // Whether to keep producing callbacks, or remove the watch after the first callback
   repeat: true/false(default),
   // Trigger on the rising or falling edge of the signal. Can be a string, or 1='rising', -1='falling', 0='both'
   edge:'rising'(default for built-in buttons)/'falling'/'both'(default for pins),
   // Use software-debouncing to stop multiple calls if a switch bounces
   // This is the time in milliseconds to wait for bounces to subside, or 0 to disable
   debounce:10 (0 is default for pins, 25 is default for built-in buttons),
   // Advanced: If the function supplied is a 'native' function (compiled or assembly)
   // setting irq:true will call that function in the interrupt itself
   irq : false(default)
   // Advanced: If specified, the given pin will be read whenever the watch is called
   // and the state will be included as a 'data' field in the callback (`debounce:0` is required)
   data : pin
   // Advanced: On Nordic devices, a watch may be 'high' or 'low' accuracy. By default low
   // accuracy is used (which is better for power consumption), but this means that
   // high speed pulses (less than 25us) may not be reliably received. Setting hispeed=true
   // allows for detecting high speed pulses at the expense of higher idle power consumption
   hispeed : true
}
```

The `function` callback is called with an argument, which is an object of type
`{state:bool, time:float, lastTime:float}`.

 * `state` is whether the pin is currently a `1` or a `0`
 * `time` is the time in seconds at which the pin changed state
 * `lastTime` is the time in seconds at which the **pin last changed state**.
   When using `edge:'rising'` or `edge:'falling'`, this is not the same as when
   the function was last called.
 * `data` is included if `data:pin` was specified in the options, and can be
   used for reading in clocked data. It will only work if `debounce:0` is used

For instance, if you want to measure the length of a positive pulse you could
use `setWatch(function(e) { console.log(e.time-e.lastTime); }, BTN, {
repeat:true, edge:'falling' });`. This will only be called on the falling edge
of the pulse, but will be able to measure the width of the pulse because
`e.lastTime` is the time of the rising edge.

Internally, an interrupt writes the time of the pin's state change into a queue
with the exact time that it happened, and the function supplied to `setWatch` is
executed only from the main message loop. However, if the callback is a native
function `void (bool state)` then you can add `irq:true` to options, which will
cause the function to be called from within the IRQ. When doing this, interrupts
will happen on both edges and there will be no debouncing.

**Note:** if you didn't call `pinMode` beforehand then this function will reset
pin's state to `"input"`

**Note:** The STM32 chip (used in the [Espruino Board](/EspruinoBoard) and
[Pico](/Pico)) cannot watch two pins with the same number - e.g. `A0` and `B0`.

**Note:** On nRF52 chips (used in Puck.js, Pixl.js, MDBT42Q) `setWatch` disables
the GPIO output on that pin. In order to be able to write to the pin again you
need to disable the watch with `clearWatch`.

 */
JsVar *jswrap_interface_setWatch(
    JsVar *func,           //!< A callback function to be invoked when the pin state changes.
    Pin    pin,            //!< The pin to be watched.
    JsVar *repeatOrObject  //!<
  ) {
  if (!jshIsPinValid(pin)) {
    jsError("Invalid pin");
    return 0;
  }

  if (!jsiIsWatchingPin(pin) && !jshCanWatch(pin)) {
    jsWarn("Unable to set watch. You may already have a watch on a pin with the same number (eg. A0 and B0),\nor this pin cannot be used with watch");
    return 0;
  }

  bool repeat = false;
  JsVarFloat debounce = 0;
  int edge = 0;
  bool isIRQ = false, isHighSpeed = false;
  Pin dataPin = PIN_UNDEFINED;
  if (IS_PIN_A_BUTTON(pin)) {
    edge = 1;
    debounce = 25;
  }
  if (jsvIsObject(repeatOrObject)) {
    JsVar *v;
    v = jsvObjectGetChildIfExists(repeatOrObject, "repeat");
    if (v) repeat = jsvGetBoolAndUnLock(v);
    v = jsvObjectGetChildIfExists(repeatOrObject, "debounce");
    if (v) debounce = jsvGetFloatAndUnLock(v);
    if (isnan(debounce) || debounce<0) debounce=0;
    v = jsvObjectGetChildIfExists(repeatOrObject, "edge");
    if (jsvIsUndefined(v)) {
      // do nothing - use default
    } else if (jsvIsNumeric(v)) {
      JsVarInt i = jsvGetInteger(v);
      edge = (i>0)?1:((i<0)?-1:0);
    } else {
      edge = -1000; // force error unless checks below work
      if (jsvIsString(v)) {
        if (jsvIsStringEqual(v, "rising")) edge=1;
        else if (jsvIsStringEqual(v, "falling")) edge=-1;
        else if (jsvIsStringEqual(v, "both")) edge=0;
      }
    }
    jsvUnLock(v);
    if (edge < -1 || edge > 1) {
      jsExceptionHere(JSET_TYPEERROR, "'edge' in setWatch should be 1, -1, 0, 'rising', 'falling' or 'both'");
      return 0;
    }
    isIRQ = jsvObjectGetBoolChild(repeatOrObject, "irq");
#ifdef NRF5X
    isHighSpeed = jsvObjectGetBoolChild(repeatOrObject, "hispeed");
#endif
    dataPin = jshGetPinFromVarAndUnLock(jsvObjectGetChildIfExists(repeatOrObject, "data"));
  } else
    repeat = jsvGetBool(repeatOrObject);

  JsVarInt itemIndex = -1;
  if (!jsvIsFunction(func) && !jsvIsString(func)) {
    jsExceptionHere(JSET_ERROR, "Function or String not supplied!");
  } else {
    JsVar *watchPtr = jsvNewObject();
    if (watchPtr) {
      jsvObjectSetChildAndUnLock(watchPtr, "pin", jsvNewFromPin(pin));
      if (repeat) jsvObjectSetChildAndUnLock(watchPtr, "recur", jsvNewFromBool(repeat));
      if (debounce>0) jsvObjectSetChildAndUnLock(watchPtr, "debounce", jsvNewFromInteger((JsVarInt)jshGetTimeFromMilliseconds(debounce)));
      if (edge) jsvObjectSetChildAndUnLock(watchPtr, "edge", jsvNewFromInteger(edge));
      jsvObjectSetChild(watchPtr, "cb", func); // no unlock intentionally
      jsvObjectSetChildAndUnLock(watchPtr, "state", jsvNewFromBool(jshPinInput(pin)));
      if (isHighSpeed)
        jsvObjectSetChildAndUnLock(watchPtr, "hispeed", jsvNewFromBool(true));
    }

    // If nothing already watching the pin, set up a watch
    IOEventFlags exti = EV_NONE;
    if (!jsiIsWatchingPin(pin))
      exti = jshPinWatch(pin, true, isHighSpeed ? JSPW_HIGH_SPEED : JSPW_NONE);
    // disable event callbacks by default
    if (exti) {
      jshSetEventCallback(exti, 0);
      if (jshIsPinValid(dataPin))
        jshSetEventDataPin(exti, dataPin);
      if (isIRQ) {
        if (jsvIsNativeFunction(func)) {
          jshSetEventCallback(exti, (JshEventCallbackCallback)jsvGetNativeFunctionPtr(func));
        } else if (jshIsPinValid(dataPin)) {
          jsExceptionHere(JSET_ERROR, "Can't have a data pin and irq:true");
        } else {
          jsExceptionHere(JSET_ERROR, "irq=true set, but function is not a native function");
        }
      }
    } else {
      if (isIRQ)
        jsExceptionHere(JSET_ERROR, "irq=true set, but watch is already used");
    }


    JsVar *watchArrayPtr = jsvLock(watchArray);
    itemIndex = jsvArrayAddToEnd(watchArrayPtr, watchPtr, 1) - 1;
    jsvUnLock2(watchArrayPtr, watchPtr);


  }
  return (itemIndex>=0) ? jsvNewFromInteger(itemIndex) : 0/*undefined*/;
}

/*JSON{
  "type" : "function",
  "name" : "clearWatch",
  "generate" : "jswrap_interface_clearWatch",
  "params" : [
    ["id","JsVarArray","The id returned by a previous call to setWatch. **Only one argument is allowed.** (or pass nothing to clear all watches)"]
  ],
  "typescript" : [
    "declare function clearWatch(id: number): void;",
    "declare function clearWatch(): void;"
  ]
}
Clear the Watch that was created with setWatch. If no parameter is supplied, all watches will be removed.

To avoid accidentally deleting all Watches, if a parameter is supplied but is `undefined` then an Exception will be thrown.
 */
void jswrap_interface_clearWatch(JsVar *idVarArr) {
  if (jsvIsUndefined(idVarArr) || jsvGetArrayLength(idVarArr)==0) {
    JsVar *watchArrayPtr = jsvLock(watchArray);
    JsvObjectIterator it;
    jsvObjectIteratorNew(&it, watchArrayPtr);
    while (jsvObjectIteratorHasValue(&it)) {
      JsVar *watchPtr = jsvObjectIteratorGetValue(&it);
      JsVar *watchPin = jsvObjectGetChildIfExists(watchPtr, "pin");
      Pin pin = jshGetPinFromVar(watchPin);
      if (!jshGetPinShouldStayWatched(pin))
        jshPinWatch(pin, false, JSPW_NONE);
      jsvUnLock2(watchPin, watchPtr);
      jsvObjectIteratorNext(&it);
    }
    jsvObjectIteratorFree(&it);
    // remove all items
    jsvRemoveAllChildren(watchArrayPtr);
    jsvUnLock(watchArrayPtr);
  } else {
    JsVar *idVar = jsvGetArrayItem(idVarArr, 0);
    if (jsvIsUndefined(idVar)) {
      jsExceptionHere(JSET_ERROR, "clearWatch(undefined) not allowed. Use clearWatch() instead");
      return;
    }
    JsVar *watchArrayPtr = jsvLock(watchArray);
    JsVar *watchNamePtr = jsvFindChildFromVar(watchArrayPtr, idVar, false);
    jsvUnLock(watchArrayPtr);
    if (watchNamePtr) { // child is a 'name'
      JsVar *watchPtr = jsvSkipName(watchNamePtr);
      Pin pin = jshGetPinFromVarAndUnLock(jsvObjectGetChildIfExists(watchPtr, "pin"));
      jsvUnLock(watchPtr);

      JsVar *watchArrayPtr = jsvLock(watchArray);
      jsvRemoveChildAndUnLock(watchArrayPtr, watchNamePtr);
      jsvUnLock(watchArrayPtr);

      // Now check if this pin is still being watched
      if (!jsiIsWatchingPin(pin))
        jshPinWatch(pin, false, JSPW_NONE); // 'unwatch' pin
    } else {
      jsExceptionHere(JSET_ERROR, "Unknown Watch %v", idVar);
    }
    jsvUnLock(idVar);
  }
}

/// function for internal use
int jswrap_interface_setWatch_int(void(*callback)(), Pin pin, bool repeat, int edge) {
  JsVar *fn = jsvNewNativeFunction(callback, JSWAT_VOID);
  JsVar *options = jsvNewObject();
  jsvObjectSetChildAndUnLock(options, "repeat", jsvNewFromBool(repeat));
  jsvObjectSetChildAndUnLock(options, "edge", jsvNewFromInteger(edge));
  int id = jsvGetIntegerAndUnLock(jswrap_interface_setWatch(fn, pin, options));
  jsvUnLock2(fn, options);
  return id;
}
/// function for internal use
void jswrap_interface_clearWatch_int(int watchNumber) {
  JsVar *id = jsvNewFromInteger(watchNumber);
  JsVar *idArray = jsvNewArray(&id, 1);
  jswrap_interface_clearWatch(idArray);
  jsvUnLock2(id, idArray);
}
