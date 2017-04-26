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

/*JSON{
  "type"          : "function",
  "name"          : "peek8",
  "generate_full" : "jswrap_io_peek(addr,count,1)",
  "params"        : [
    ["addr", "int", "The address in memory to read"],
    ["count", "int", "(optional) the number of items to read. If >1 a Uint8Array will be returned."]
  ],
  "return"        : ["JsVar","The value of memory at the given location"]
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
  ]
}
Write 8 bits of memory at the given location - VERY DANGEROUS!
 */
/*JSON{
  "type" : "function",
  "name" : "peek16",
  "generate_full" : "jswrap_io_peek(addr,count,2)",
  "params" : [
    ["addr","int","The address in memory to read"],
    ["count","int","(optional) the number of items to read. If >1 a Uint16Array will be returned."]
  ],
  "return" : ["JsVar","The value of memory at the given location"]
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
  ]
}
Write 16 bits of memory at the given location - VERY DANGEROUS!
 */
/*JSON{
  "type" : "function",
  "name" : "peek32",
  "generate_full" : "jswrap_io_peek(addr,count,4)",
  "params" : [
    ["addr","int","The address in memory to read"],
    ["count","int","(optional) the number of items to read. If >1 a Uint32Array will be returned."]
  ],
  "return" : ["JsVar","The value of memory at the given location"]
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
  ]
}
Write 32 bits of memory at the given location - VERY DANGEROUS!
 */

uint32_t _jswrap_io_peek(JsVarInt addr, int wordSize) {
  if (wordSize==1) return READ_FLASH_UINT8((char*)(size_t)addr);
  if (wordSize==2) {
    return READ_FLASH_UINT8((char*)(size_t)addr) | (uint32_t)(READ_FLASH_UINT8((char*)(size_t)(addr+1)) << 8);
  }
  if (wordSize==4) return (uint32_t)*(unsigned int*)(size_t)addr;
  return 0;
}

JsVar *jswrap_io_peek(JsVarInt addr, JsVarInt count, int wordSize) {
  if (count<=1) {
    return jsvNewFromLongInteger((long long)_jswrap_io_peek(addr, wordSize));
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
      jsvArrayBufferIteratorSetIntegerValue(&it, (JsVarInt)_jswrap_io_peek(addr, wordSize));
      addr += wordSize;
      jsvArrayBufferIteratorNext(&it);
    }
    jsvArrayBufferIteratorFree(&it);
    return arr;
  }
}

void _jswrap_io_poke(JsVarInt addr, uint32_t data, int wordSize) {
  if (wordSize==1) (*(unsigned char*)(size_t)addr) = (unsigned char)data;
  else if (wordSize==2) (*(unsigned short*)(size_t)addr) = (unsigned short)data;
  else if (wordSize==4) (*(unsigned int*)(size_t)addr) = (unsigned int)data;
}

void jswrap_io_poke(JsVarInt addr, JsVar *data, int wordSize) {
  if (jsvIsNumeric(data)) {
    _jswrap_io_poke(addr, (uint32_t)jsvGetInteger(data), wordSize);
  } else if (jsvIsIterable(data)) {
    JsvIterator it;
    jsvIteratorNew(&it, data);
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
  "return" : ["float","The analog Value of the Pin between 0 and 1"]
}
Get the analog value of the given pin

This is different to Arduino which only returns an integer between 0 and 1023

However only pins connected to an ADC will work (see the datasheet)

 **Note:** if you didn't call `pinMode` beforehand then this function will also reset pin's state to `"analog"`
 */
/*JSON{
  "type" : "function",
  "name" : "analogWrite",
  "generate" : "jswrap_io_analogWrite",
  "params" : [
    ["pin","pin",["The pin to use","You can find out which pins to use by looking at [your board's reference page](#boards) and searching for pins with the `PWM` or `DAC` markers."]],
    ["value","float","A value between 0 and 1"],
    ["options","JsVar",["An object containing options for analog output - see below"]]
  ]
}
Set the analog Value of a pin. It will be output using PWM.

Objects can contain:

* `freq` - pulse frequency in Hz, eg. ```analogWrite(A0,0.5,{ freq : 10 });``` - specifying a frequency will force PWM output, even if the pin has a DAC
* `soft` - boolean, If true software PWM is used if available.
* `forceSoft` - boolean, If true software PWM is used even

 **Note:** if you didn't call `pinMode` beforehand then this function will also reset pin's state to `"output"`
 */
void jswrap_io_analogWrite(Pin pin, JsVarFloat value, JsVar *options) {
  JsVarFloat freq = 0;
  JshAnalogOutputFlags flags = JSAOF_NONE;
  if (jsvIsObject(options)) {
    freq = jsvGetFloatAndUnLock(jsvObjectGetChild(options, "freq", 0));
    if (jsvGetBoolAndUnLock(jsvObjectGetChild(options, "forceSoft", 0)))
          flags |= JSAOF_FORCE_SOFTWARE;
    else if (jsvGetBoolAndUnLock(jsvObjectGetChild(options, "soft", 0)))
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
  ]
}
Pulse the pin with the value for the given time in milliseconds. It uses a hardware timer to produce accurate pulses, and returns immediately (before the pulse has finished). Use `digitalPulse(A0,1,0)` to wait until a previous pulse has finished.

eg. `digitalPulse(A0,1,5);` pulses A0 high for 5ms. `digitalPulse(A0,1,[5,2,4]);` pulses A0 high for 5ms, low for 2ms, and high for 4ms

 **Note:** if you didn't call `pinMode` beforehand then this function will also reset pin's state to `"output"`

digitalPulse is for SHORT pulses that need to be very accurate. If you're doing anything over a few milliseconds, use setTimeout instead.
 */
void jswrap_io_digitalPulse(Pin pin, bool value, JsVar *times) {
  if (jsvIsNumeric(times)) {
    JsVarFloat time = jsvGetFloat(times);
    if (time<0 || isnan(time)) {
      jsExceptionHere(JSET_ERROR, "Pulse Time given for digitalPulse is less than 0, or not a number");
    } else {
      jshPinPulse(pin, value, time);
    }
  } else if (jsvIsIterable(times)) {
    // iterable, so output a square wave
    JsvIterator it;
    jsvIteratorNew(&it, times);
    while (jsvIteratorHasElement(&it)) {
      JsVarFloat time = jsvIteratorGetFloatValue(&it);
      if (time>=0 && !isnan(time))
        jshPinPulse(pin, value, time);
      value = !value;
      jsvIteratorNext(&it);
    }
    jsvIteratorFree(&it);
  } else {
    jsExceptionHere(JSET_ERROR, "Expecting a number or array, got %t", times);
  }
}

/*JSON{
  "type"     : "function",
  "name"     : "digitalWrite",
  "generate" : "jswrap_io_digitalWrite",
  "params"   : [
    ["pin",   "JsVar","The pin to use"],
    ["value", "int","Whether to pulse high (true) or low (false)"]
  ]
}
Set the digital value of the given pin.

 **Note:** if you didn't call `pinMode` beforehand then this function will also reset pin's state to `"output"`

If pin argument is an array of pins (eg. `[A2,A1,A0]`) the value argument will be treated
as an array of bits where the last array element is the least significant bit.

In this case, pin values are set least significant bit first (from the right-hand side
of the array of pins). This means you can use the same pin multiple times, for
example `digitalWrite([A1,A1,A0,A0],0b0101)` would pulse A0 followed by A1.
*/

/**
 * Set the output of a GPIO.
 */
void jswrap_io_digitalWrite(
    JsVar *pinVar, //!< A pin or pins.
    JsVarInt value //!< The value of the output.
  ) {
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
  }
  // Handle the case where it is a single pin.
  else {
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
  "return"   : ["int","The digital Value of the Pin"]
}
Get the digital value of the given pin.

 **Note:** if you didn't call `pinMode` beforehand then this function will also reset pin's state to `"input"`

If the pin argument is an array of pins (eg. `[A2,A1,A0]`) the value returned will be an number where
the last array element is the least significant bit, for example if `A0=A1=1` and `A2=0`, `digitalRead([A2,A1,A0]) == 0b011`
*/

/**
 * Read the value of a GPIO pin.
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
  }
  // Handle the case where it is a single pin.
  else {
    Pin pin = jshGetPinFromVar(pinVar);
    return jshPinInput(pin);
  }
}

/*JSON{
  "type"     : "function",
  "name"     : "pinMode",
  "generate" : "jswrap_io_pinMode",
  "params"   : [
    ["pin", "pin", "The pin to set pin mode for"],
    ["mode", "JsVar", "The mode - a string that is either 'analog', 'input', 'input_pullup', 'input_pulldown', 'output', 'opendrain', 'af_output' or 'af_opendrain'. Do not include this argument or use 'auto' if you want to revert to automatic pin mode setting."],
    ["automatic", "bool", "Optional, default is false. If true, subsequent commands will automatically change the state (see notes below)"]
  ]
}
Set the mode of the given pin.

 * `auto`/`undefined` - Don't change state, but allow `digitalWrite`/etc to automatically change state as appropriate
 * `analog` - Analog input
 * `input` - Digital input
 * `input_pullup` - Digital input with internal ~40k pull-up resistor
 * `input_pulldown` - Digital input with internal ~40k pull-down resistor
 * `output` - Digital output
 * `opendrain` - Digital output that only ever pulls down to 0v. Sending a logical `1` leaves the pin open circuit
 * `opendrain_pullup` - Digital output that pulls down to 0v. Sending a logical `1` enables internal ~40k pull-up resistor
 * `af_output` - Digital output from built-in peripheral
 * `af_opendrain` - Digital output from built-in peripheral that only ever pulls down to 0v. Sending a logical `1` leaves the pin open circuit

 **Note:** `digitalRead`/`digitalWrite`/etc set the pin mode automatically *unless* `pinMode` has been called first.
If you want `digitalRead`/etc to set the pin mode automatically after you have called `pinMode`, simply call it again
with no mode argument (`pinMode(pin)`), `auto` as the argument (`pinMode(pin, "auto")`), or with the 3rd 'automatic'
argument set to true (`pinMode(pin, "output", true)`).
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
  "return" : ["JsVar","The pin mode, as a string"]
}
Return the current mode of the given pin. See `pinMode` for more information on returned values.
 */
JsVar *jswrap_io_getPinMode(Pin pin) {
  if (!jshIsPinValid(pin)) {
    jsExceptionHere(JSET_ERROR, "Invalid pin");
    return 0;
  }
  JshPinState m = jshPinGetState(pin)&JSHPINSTATE_MASK;
  const char *text = 0;
  switch (m) {
  case JSHPINSTATE_ADC_IN :             text = "analog"; break;
  case JSHPINSTATE_GPIO_IN :            text = "input"; break;
  case JSHPINSTATE_GPIO_IN_PULLUP :     text = "input_pullup"; break;
  case JSHPINSTATE_GPIO_IN_PULLDOWN :   text = "input_pulldown"; break;
  case JSHPINSTATE_GPIO_OUT :           text = "output"; break;
  case JSHPINSTATE_GPIO_OUT_OPENDRAIN : text = "opendrain"; break;
  case JSHPINSTATE_GPIO_OUT_OPENDRAIN_PULLUP : text = "opendrain_pullup"; break;
  case JSHPINSTATE_AF_OUT :             text = "af_output"; break;
  case JSHPINSTATE_AF_OUT_OPENDRAIN :   text = "af_opendrain"; break;
  default: break;
  }
  if (text) return jsvNewFromString(text);
  return 0;
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
    ["data","JsVar","The data to shift out"]
  ]
}
Shift an array of data out using the pins supplied *least significant bit first*,
for example:

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

Each item in the `data` array will be output to the pins, with the first
pin in the array being the MSB and the last the LSB, then the clock will be
pulsed in the polarity given.

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
        jsExceptionHere(JSET_ERROR, "Too many pins! %d Maximum.", jswrap_io_shiftOutDataMax);
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
  jsvIterateCallback(data, jswrap_io_shiftOutCallback, &d);
}

/*JSON{
  "type" : "function",
  "name" : "setWatch",
  "generate" : "jswrap_interface_setWatch",
  "params" : [
    ["function", "JsVar", "A Function or String to be executed"],
    ["pin", "pin", "The pin to watch"],
    ["options", "JsVar",[ "If this is a boolean or integer, it determines whether to call this once (false = default) or every time a change occurs (true)","If this is an object, it can contain the following information: ```{ repeat: true/false(default), edge:'rising'/'falling'/'both'(default), debounce:10}```. `debounce` is the time in ms to wait for bounces to subside, or 0."]]
  ],
  "return" : ["JsVar","An ID that can be passed to clearWatch"]
}
Call the function specified when the pin changes. Watches set with `setWatch` can be removed using `clearWatch`.

The function may also take an argument, which is an object of type `{state:bool, time:float, lastTime:float}`.

 * `state` is whether the pin is currently a `1` or a `0`
 * `time` is the time in seconds at which the pin changed state
 * `lastTime` is the time in seconds at which the **pin last changed state**. When using `edge:'rising'` or `edge:'falling'`, this is not the same as when the function was last called.

For instance, if you want to measure the length of a positive pulse you could use `setWatch(function(e) { console.log(e.time-e.lastTime); }, BTN, { repeat:true, edge:'falling' });`.
This will only be called on the falling edge of the pulse, but will be able to measure the width of the pulse because `e.lastTime` is the time of the rising edge.

Internally, an interrupt writes the time of the pin's state change into a queue, and the function
supplied to `setWatch` is executed only from the main message loop. However, if the callback is a
native function `void (bool state)` then you can add `irq:true` to options, which will cause the
function to be called from within the IRQ. When doing this, interrupts will happen on both edges
and there will be no debouncing.

**Note:** The STM32 chip (used in the [Espruino Board](/EspruinoBoard) and [Pico](/Pico)) cannot
watch two pins with the same number - eg `A0` and `B0`.

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
    jsWarn("Unable to set watch. You may already have a watch on a pin with the same number (eg. A0 and B0)");
    return 0;
  }

  bool repeat = false;
  JsVarFloat debounce = 0;
  int edge = 0;
  bool isIRQ = false;
  if (jsvIsObject(repeatOrObject)) {
    JsVar *v;
    repeat = jsvGetBoolAndUnLock(jsvObjectGetChild(repeatOrObject, "repeat", 0));
    debounce = jsvGetFloatAndUnLock(jsvObjectGetChild(repeatOrObject, "debounce", 0));
    if (isnan(debounce) || debounce<0) debounce=0;
    v = jsvObjectGetChild(repeatOrObject, "edge", 0);
    edge = -1000;
    if (jsvIsUndefined(v)) {
      edge = 0;
    } else if (jsvIsString(v)) {
      if (jsvIsStringEqual(v, "rising")) edge=1;
      else if (jsvIsStringEqual(v, "falling")) edge=-1;
      else if (jsvIsStringEqual(v, "both")) edge=0;
    }
    jsvUnLock(v);
    if (edge < -1 || edge > 1) {
      jsExceptionHere(JSET_TYPEERROR, "'edge' in setWatch should be a string - either 'rising', 'falling' or 'both'");
      return 0;
    }
    isIRQ = jsvGetBoolAndUnLock(jsvObjectGetChild(repeatOrObject, "irq", 0));
  } else
    repeat = jsvGetBool(repeatOrObject);

  JsVarInt itemIndex = -1;
  if (!jsvIsFunction(func) && !jsvIsString(func)) {
    jsExceptionHere(JSET_ERROR, "Function or String not supplied!");
  } else {
    // Create a new watch object which may contain:
    //
    // o pin      - The pin being watched
    // o recur    - ?
    // o debounce - ?
    // o edge     - ?
    // o callback - The function to be invoked when the IO changes
    JsVar *watchPtr = jsvNewObject();
    if (watchPtr) {
      jsvObjectSetChildAndUnLock(watchPtr, "pin", jsvNewFromPin(pin));
      if (repeat) jsvObjectSetChildAndUnLock(watchPtr, "recur", jsvNewFromBool(repeat));
      if (debounce>0) jsvObjectSetChildAndUnLock(watchPtr, "debounce", jsvNewFromInteger((JsVarInt)jshGetTimeFromMilliseconds(debounce)));
      if (edge) jsvObjectSetChildAndUnLock(watchPtr, "edge", jsvNewFromInteger(edge));
      jsvObjectSetChild(watchPtr, "callback", func); // no unlock intentionally
    }

    // If nothing already watching the pin, set up a watch
    IOEventFlags exti = EV_NONE;
    if (!jsiIsWatchingPin(pin))
      exti = jshPinWatch(pin, true);
    // disable event callbacks by default
    if (exti) {
      jshSetEventCallback(exti, 0);
      if (isIRQ) {
        if (jsvIsNativeFunction(func)) {
          jshSetEventCallback(exti, (JshEventCallbackCallback)jsvGetNativeFunctionPtr(func));
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
    ["id","JsVar","The id returned by a previous call to setWatch"]
  ]
}
Clear the Watch that was created with setWatch. If no parameter is supplied, all watches will be removed.
 */
void jswrap_interface_clearWatch(JsVar *idVar) {

  if (jsvIsUndefined(idVar)) {
    JsVar *watchArrayPtr = jsvLock(watchArray);
    JsvObjectIterator it;
    jsvObjectIteratorNew(&it, watchArrayPtr);
    while (jsvObjectIteratorHasValue(&it)) {
      JsVar *watchPtr = jsvObjectIteratorGetValue(&it);
      JsVar *watchPin = jsvObjectGetChild(watchPtr, "pin", 0);
      jshPinWatch(jshGetPinFromVar(watchPin), false);
      jsvUnLock2(watchPin, watchPtr);
      jsvObjectIteratorNext(&it);
    }
    jsvObjectIteratorFree(&it);
    // remove all items
    jsvRemoveAllChildren(watchArrayPtr);
    jsvUnLock(watchArrayPtr);
  } else {
    JsVar *watchArrayPtr = jsvLock(watchArray);
    JsVar *watchNamePtr = jsvFindChildFromVar(watchArrayPtr, idVar, false);
    jsvUnLock(watchArrayPtr);
    if (watchNamePtr) { // child is a 'name'
      JsVar *watchPtr = jsvSkipName(watchNamePtr);
      Pin pin = jshGetPinFromVarAndUnLock(jsvObjectGetChild(watchPtr, "pin", 0));
      jsvUnLock(watchPtr);

      JsVar *watchArrayPtr = jsvLock(watchArray);
      jsvRemoveChild(watchArrayPtr, watchNamePtr);
      jsvUnLock2(watchNamePtr, watchArrayPtr);

      // Now check if this pin is still being watched
      if (!jsiIsWatchingPin(pin))
        jshPinWatch(pin, false); // 'unwatch' pin
    } else {
      jsExceptionHere(JSET_ERROR, "Unknown Watch");
    }
  }
}
