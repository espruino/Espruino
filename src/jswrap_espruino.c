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
 * JavaScript methods for Espruino utility functions
 * ----------------------------------------------------------------------------
 */
#include "jswrap_espruino.h"
#include "jswrap_math.h"
#include "jswrap_arraybuffer.h"
#include "jswrap_json.h"
#include "jsflash.h"
#include "jswrapper.h"
#include "jsinteractive.h"
#include "jstimer.h"
#ifdef PUCKJS
#include "jswrap_puck.h" // jswrap_puck_getTemperature
#endif

/*JSON{
    "type" : "variable",
    "name" : "__FILE__",
    "generate" : false,
    "return" : ["JsVar","The filename of the JavaScript that is currently executing"]
}
The filename of the JavaScript that is currently executing.

If `load` has been called with a filename (eg `load("myfile.js")`) then
`__FILE__` is set to that filename. Otherwise (eg `load()`) or immediately
after booting, `__FILE__` is not set.
*//*Documentation only*/

/*JSON{
  "type" : "class",
  "class" : "E"
}
This is the built-in JavaScript class for Espruino utility functions.
 */

/*JSON{
  "type" : "event",
  "class" : "E",
  "name" : "init"
}
This event is called right after the board starts up, and has a similar effect
to creating a function called `onInit`.

For example to write `"Hello World"` every time Espruino starts, use:

```
E.on('init', function() {
  console.log("Hello World!");
});
```

**Note:** that subsequent calls to `E.on('init', ` will **add** a new handler,
rather than replacing the last one. This allows you to write modular code -
something that was not possible with `onInit`.
 */
/*JSON{
  "type" : "event",
  "class" : "E",
  "name" : "kill"
}
This event is called just before the device shuts down for commands such as
`reset()`, `load()`, `save()`, `E.reboot()` or `Bangle.off()`

For example to write `"Bye!"` just before shutting down use:

```
E.on('kill', function() {
  console.log("Bye!");
});
```

**NOTE:** This event is not called when the device is 'hard reset' - for example
by removing power, hitting an actual reset button, or via a Watchdog timer
reset.
*/

/*JSON{
  "type" : "event",
  "class" : "E",
  "name" : "errorFlag",
  "params" : [
    ["errorFlags","JsVar","An array of new error flags, as would be returned by `E.getErrorFlags()`. Error flags that were present before won't be reported."]
  ],
  "typescript" : "on(event: \"errorFlag\", callback: (errorFlags: ErrorFlag[]) => void): void;"
}
This event is called when an error is created by Espruino itself (rather than JS
code) which changes the state of the error flags reported by `E.getErrorFlags()`

This could be low memory, full buffers, UART overflow, etc. `E.getErrorFlags()`
has a full description of each type of error.

This event will only be emitted when error flag is set. If the error flag was
already set nothing will be emitted. To clear error flags so that you do get a
callback each time a flag is set, call `E.getErrorFlags()`.
*/
/*JSON{
  "type" : "event",
  "class" : "E",
  "name" : "touch",
  "params" : [
    ["x","int","X coordinate in display coordinates"],
    ["y","int","Y coordinate in display coordinates"],
    ["b","int","Touch count - 0 for released, 1 for pressed"]
  ]
}
This event is called when a full touchscreen device on an Espruino is interacted
with.

**Note:** This event is not implemented on Bangle.js because it only has a two
area touchscreen.

To use the touchscreen to draw lines, you could do:

```
var last;
E.on('touch',t=>{
  if (last) g.lineTo(t.x, t.y);
  else g.moveTo(t.x, t.y);
  last = t.b;
});
```
*/

/*JSON{
  "type" : "staticmethod",
  "class" : "E",
  "name" : "getTemperature",
  "generate" : "jswrap_espruino_getTemperature",
  "return" : ["float","The temperature in degrees C"]
}
Use the microcontroller's internal thermistor to work out the temperature.

On Puck.js v2.0 this will use the on-board PCT2075TP temperature sensor, but on
other devices it may not be desperately well calibrated.

While this is implemented on Espruino boards, it may not be implemented on other
devices. If so it'll return NaN.

 **Note:** This is not entirely accurate and varies by a few degrees from chip
 to chip. It measures the **die temperature**, so when connected to USB it could
 be reading 10 over degrees C above ambient temperature. When running from
 battery with `setDeepSleep(true)` it is much more accurate though.
*/
JsVarFloat jswrap_espruino_getTemperature() {
#ifdef PUCKJS
  return jswrap_puck_getTemperature();
#else
  return jshReadTemperature();
#endif
}

/*JSON{
  "type" : "staticmethod",
  "ifndef" : "SAVE_ON_FLASH",
  "class" : "E",
  "name" : "getAnalogVRef",
  "generate_full" : "jshReadVRef()",
  "return" : ["float","The voltage (in Volts) that a reading of 1 from `analogRead` actually represents - usually around 3.3v"]
}
Check the internal voltage reference. To work out an actual voltage of an input
pin, you can use `analogRead(pin)*E.getAnalogVRef()`

 **Note:** This value is calculated by reading the voltage on an internal
voltage reference with the ADC. It will be slightly noisy, so if you need this
for accurate measurements we'd recommend that you call this function several
times and average the results.

While this is implemented on Espruino boards, it may not be implemented on other
devices. If so it'll return NaN.
 */


int nativeCallGetCType() {
  if (lex->tk == LEX_R_VOID) {
    jslMatch(LEX_R_VOID);
    return JSWAT_VOID;
  }
  if (lex->tk == LEX_ID) {
    int t = -1;
    char *name = jslGetTokenValueAsString();
    if (strcmp(name,"int")==0) t=JSWAT_INT32;
    if (strcmp(name,"double")==0) t=JSWAT_JSVARFLOAT;
    if (strcmp(name,"bool")==0) t=JSWAT_BOOL;
    if (strcmp(name,"Pin")==0) t=JSWAT_PIN;
    if (strcmp(name,"JsVar")==0) t=JSWAT_JSVAR;
    jslMatch(LEX_ID);
    return t;
  }
  return -1; // unknown
}

/*JSON{
  "type" : "staticmethod",
  "ifndef" : "SAVE_ON_FLASH",
  "class" : "E",
  "name" : "nativeCall",
  "generate" : "jswrap_espruino_nativeCall",
  "params" : [
    ["addr","int","The address in memory of the function (or offset in `data` if it was supplied"],
    ["sig","JsVar","The signature of the call, `returnType (arg1,arg2,...)`. Allowed types are `void`,`bool`,`int`,`double`,`Pin`,`JsVar`"],
    ["data","JsVar","(Optional) A string containing the function itself. If not supplied then 'addr' is used as an absolute address."]
  ],
  "return" : ["JsVar","The native function"],
  "typescript" : "nativeCall(addr: number, sig: string, data?: string): any;"
}
ADVANCED: This is a great way to crash Espruino if you're not sure what you are
doing

Create a native function that executes the code at the given address, e.g.
`E.nativeCall(0x08012345,'double (double,double)')(1.1, 2.2)`

If you're executing a thumb function, you'll almost certainly need to set the
bottom bit of the address to 1.

Note it's not guaranteed that the call signature you provide can be used - there
are limits on the number of arguments allowed.

When supplying `data`, if it is a 'flat string' then it will be used directly,
otherwise it'll be converted to a flat string and used.
 */
JsVar *jswrap_espruino_nativeCall(JsVarInt addr, JsVar *signature, JsVar *data) {
  unsigned int argTypes = 0;
  if (jsvIsUndefined(signature)) {
    // Nothing to do
  } else if (jsvIsString(signature)) {
    JsLex lex;
    JsLex *oldLex = jslSetLex(&lex);
    jslInit(signature);
    int argType;
    bool ok = true;
    int argNumber = 0;
    argType = nativeCallGetCType();
    if (argType>=0) argTypes |= (unsigned)argType << (JSWAT_BITS * argNumber++);
    else ok = false;
    if (ok) ok = jslMatch('(');
    while (ok && lex.tk!=LEX_EOF && lex.tk!=')') {
      argType = nativeCallGetCType();
      if (argType>=0) {
        argTypes |= (unsigned)argType << (JSWAT_BITS * argNumber++);
        if (lex.tk!=')') ok = jslMatch(',');
      } else ok = false;
    }
    if (ok) ok = jslMatch(')');
    jslKill();
    jslSetLex(oldLex);
    if (argTypes & (unsigned int)~0xFFFF)
      ok = false;
    if (!ok) {
      jsExceptionHere(JSET_ERROR, "Error Parsing signature at argument number %d", argNumber);
      return 0;
    }
  } else {
    jsExceptionHere(JSET_ERROR, "Invalid Signature");
    return 0;
  }

  JsVar *fn = jsvNewNativeFunction((void *)(size_t)addr, (unsigned short)argTypes);
  if (data) {
    JsVar *flat = jsvAsFlatString(data);
    jsvUnLock2(jsvAddNamedChild(fn, flat, JSPARSE_FUNCTION_CODE_NAME), flat);
  }
  return fn;
}


/*JSON{
  "type" : "staticmethod",
  "ifndef" : "SAVE_ON_FLASH",
  "class" : "E",
  "name" : "clip",
  "generate" : "jswrap_espruino_clip",
  "params" : [
    ["x","float","A floating point value to clip"],
    ["min","float","The smallest the value should be"],
    ["max","float","The largest the value should be"]
  ],
  "return" : ["float","The value of x, clipped so as not to be below min or above max."]
}
Clip a number to be between min and max (inclusive)
 */
JsVarFloat jswrap_espruino_clip(JsVarFloat x, JsVarFloat min, JsVarFloat max) {
  if (x<min) x=min;
  if (x>max) x=max;
  return x;
}


/*JSON{
  "type" : "staticmethod",
  "ifndef" : "SAVE_ON_FLASH",
  "class" : "E",
  "name" : "sum",
  "generate" : "jswrap_espruino_sum",
  "params" : [
    ["arr","JsVar","The array to sum"]
  ],
  "return" : ["float","The sum of the given buffer"],
  "typescript" : "sum(arr: string | number[] | ArrayBuffer): number;"
}
Sum the contents of the given Array, String or ArrayBuffer and return the result
 */
JsVarFloat jswrap_espruino_sum(JsVar *arr) {
  if (!(jsvIsString(arr) || jsvIsArray(arr) || jsvIsArrayBuffer(arr))) {
    jsExceptionHere(JSET_ERROR, "Expecting first argument to be an array, not %t", arr);
    return NAN;
  }
  JsVarFloat sum = 0;

  JsvIterator itsrc;
  jsvIteratorNew(&itsrc, arr, JSIF_DEFINED_ARRAY_ElEMENTS);
  while (jsvIteratorHasElement(&itsrc)) {
    sum += jsvIteratorGetFloatValue(&itsrc);
    jsvIteratorNext(&itsrc);
  }
  jsvIteratorFree(&itsrc);
  return sum;
}

/*JSON{
  "type" : "staticmethod",
  "ifndef" : "SAVE_ON_FLASH",
  "class" : "E",
  "name" : "variance",
  "generate" : "jswrap_espruino_variance",
  "params" : [
    ["arr","JsVar","The array to work out the variance for"],
    ["mean","float","The mean value of the array"]
  ],
  "return" : ["float","The variance of the given buffer"],
  "typescript" : "variance(arr: string | number[] | ArrayBuffer, mean: number): number;"
}
Work out the variance of the contents of the given Array, String or ArrayBuffer
and return the result. This is equivalent to `v=0;for (i in arr)
v+=Math.pow(mean-arr[i],2)`
 */
JsVarFloat jswrap_espruino_variance(JsVar *arr, JsVarFloat mean) {
  if (!(jsvIsIterable(arr))) {
    jsExceptionHere(JSET_ERROR, "Expecting first argument to be iterable, not %t", arr);
    return NAN;
  }
  JsVarFloat variance = 0;

  JsvIterator itsrc;
  jsvIteratorNew(&itsrc, arr, JSIF_EVERY_ARRAY_ELEMENT);
  while (jsvIteratorHasElement(&itsrc)) {
    JsVarFloat val = jsvIteratorGetFloatValue(&itsrc);
    val -= mean;
    variance += val*val;
    jsvIteratorNext(&itsrc);
  }
  jsvIteratorFree(&itsrc);
  return variance;
}

/*JSON{
  "type" : "staticmethod",
  "ifndef" : "SAVE_ON_FLASH",
  "class" : "E",
  "name" : "convolve",
  "generate" : "jswrap_espruino_convolve",
  "params" : [
    ["arr1","JsVar","An array to convolve"],
    ["arr2","JsVar","An array to convolve"],
    ["offset","int32","The mean value of the array"]
  ],
  "return" : ["float","The variance of the given buffer"],
  "typescript" : "convolve(arr1: string | number[] | ArrayBuffer, arr2: string | number[] | ArrayBuffer, offset: number): number;"
}
Convolve arr1 with arr2. This is equivalent to `v=0;for (i in arr1) v+=arr1[i] *
arr2[(i+offset) % arr2.length]`
 */
JsVarFloat jswrap_espruino_convolve(JsVar *arr1, JsVar *arr2, int offset) {
  if (!(jsvIsIterable(arr1)) ||
      !(jsvIsIterable(arr2))) {
    jsExceptionHere(JSET_ERROR, "Expecting first 2 arguments to be iterable, not %t and %t", arr1, arr2);
    return NAN;
  }
  JsVarFloat conv = 0;

  JsvIterator it1;
  jsvIteratorNew(&it1, arr1, JSIF_EVERY_ARRAY_ELEMENT);
  JsvIterator it2;
  jsvIteratorNew(&it2, arr2, JSIF_EVERY_ARRAY_ELEMENT);

  // get iterator2 at the correct offset
  int l = (int)jsvGetLength(arr2);
  offset = offset % l;
  if (offset<0) offset += l;
  while (offset-->0)
    jsvIteratorNext(&it2);


  while (jsvIteratorHasElement(&it1)) {
    conv += jsvIteratorGetFloatValue(&it1) * jsvIteratorGetFloatValue(&it2);
    jsvIteratorNext(&it1);
    jsvIteratorNext(&it2);
    // restart iterator if it hit the end
    if (!jsvIteratorHasElement(&it2)) {
      jsvIteratorFree(&it2);
      jsvIteratorNew(&it2, arr2, JSIF_EVERY_ARRAY_ELEMENT);
    }
  }
  jsvIteratorFree(&it1);
  jsvIteratorFree(&it2);
  return conv;
}

#if defined(SAVE_ON_FLASH_MATH) || defined(BANGLEJS)
#define FFTDATATYPE double
#else
#define FFTDATATYPE float
#endif

// http://paulbourke.net/miscellaneous/dft/
/*
   This computes an in-place complex-to-complex FFT
   x and y are the real and imaginary arrays of 2^m points.
   dir =  1 gives forward transform
   dir = -1 gives reverse transform
 */
short FFT(short int dir,long m,FFTDATATYPE *x,FFTDATATYPE *y)
{
  long n,i,i1,j,k,i2,l,l1,l2;
  FFTDATATYPE c1,c2,tx,ty,t1,t2,u1,u2,z;

  /* Calculate the number of points */
  n = 1;
  for (i=0;i<m;i++)
    n *= 2;

  /* Do the bit reversal */
  i2 = n >> 1;
  j = 0;
  for (i=0;i<n-1;i++) {
    if (i < j) {
      tx = x[i];
      ty = y[i];
      x[i] = x[j];
      y[i] = y[j];
      x[j] = tx;
      y[j] = ty;
    }
    k = i2;
    while (k <= j) {
      j -= k;
      k >>= 1;
    }
    j += k;
  }

  /* Compute the FFT */
  c1 = -1.0;
  c2 = 0.0;
  l2 = 1;
  for (l=0;l<m;l++) {
    l1 = l2;
    l2 <<= 1;
    u1 = 1.0;
    u2 = 0.0;
    for (j=0;j<l1;j++) {
      for (i=j;i<n;i+=l2) {
        i1 = i + l1;
        t1 = u1 * x[i1] - u2 * y[i1];
        t2 = u1 * y[i1] + u2 * x[i1];
        x[i1] = x[i] - t1;
        y[i1] = y[i] - t2;
        x[i] += t1;
        y[i] += t2;
      }
      z =  u1 * c1 - u2 * c2;
      u2 = u1 * c2 + u2 * c1;
      u1 = z;
    }
    c2 = (FFTDATATYPE)jswrap_math_sqrt((1.0 - c1) / 2.0);
    if (dir == 1)
      c2 = -c2;
    c1 = (FFTDATATYPE)jswrap_math_sqrt((1.0 + c1) / 2.0);
  }

  /* Scaling for forward transform */
  if (dir == 1) {
    for (i=0;i<n;i++) {
      x[i] /= (FFTDATATYPE)n;
      y[i] /= (FFTDATATYPE)n;
    }
  }

  return(TRUE);
}

/*JSON{
  "type" : "staticmethod",
  "ifndef" : "SAVE_ON_FLASH",
  "class" : "E",
  "name" : "FFT",
  "generate" : "jswrap_espruino_FFT",
  "params" : [
    ["arrReal","JsVar","An array of real values"],
    ["arrImage","JsVar","An array of imaginary values (or if undefined, all values will be taken to be 0)"],
    ["inverse","bool","Set this to true if you want an inverse FFT - otherwise leave as 0"]
  ],
  "typescript" : "FFT(arrReal: string | number[] | ArrayBuffer, arrImage?: string | number[] | ArrayBuffer, inverse?: boolean): any;"
}
Performs a Fast Fourier Transform (FFT) in 32 bit floats on the supplied data
and writes it back into the original arrays. Note that if only one array is
supplied, the data written back is the modulus of the complex result
`sqrt(r*r+i*i)`.

In order to perform the FFT, there has to be enough room on the stack to
allocate two arrays of 32 bit floating point numbers - this will limit the
maximum size of FFT possible to around 1024 items on most platforms.

**Note:** on the Original Espruino board, FFTs are performed in 64bit arithmetic
as there isn't space to include the 32 bit maths routines (2x more RAM is
required).
 */
void _jswrap_espruino_FFT_getData(FFTDATATYPE *dst, JsVar *src, size_t length) {
  JsvIterator it;
  size_t i=0;
  if (jsvIsIterable(src)) {
    jsvIteratorNew(&it, src, JSIF_EVERY_ARRAY_ELEMENT);
    while (i<length && jsvIteratorHasElement(&it)) {
      dst[i++] = (FFTDATATYPE)jsvIteratorGetFloatValue(&it);
      jsvIteratorNext(&it);
    }
    jsvIteratorFree(&it);
  }
  while (i<length)
    dst[i++]=0;
}
void _jswrap_espruino_FFT_setData(JsVar *dst, FFTDATATYPE *src, FFTDATATYPE *srcModulus, size_t length) {
  JsvIterator it;
  jsvIteratorNew(&it, dst, JSIF_EVERY_ARRAY_ELEMENT);
  size_t i=0;
  while (i<length && jsvIteratorHasElement(&it)) {
    JsVarFloat f;
    if (srcModulus)
      f = jswrap_math_sqrt(src[i]*src[i] + srcModulus[i]*srcModulus[i]);
    else
      f = src[i];

    jsvUnLock(jsvIteratorSetValue(&it, jsvNewFromFloat(f)));
    i++;
    jsvIteratorNext(&it);
  }
  jsvIteratorFree(&it);
}
void jswrap_espruino_FFT(JsVar *arrReal, JsVar *arrImag, bool inverse) {
  if (!(jsvIsIterable(arrReal)) ||
      !(jsvIsUndefined(arrImag) || jsvIsIterable(arrImag))) {
    jsExceptionHere(JSET_ERROR, "Expecting first 2 arguments to be iterable or undefined, not %t and %t", arrReal, arrImag);
    return;
  }

  // get length and work out power of 2
  size_t l = (size_t)jsvGetLength(arrReal);
  size_t pow2 = 1;
  int order = 0;
  while (pow2 < l) {
    pow2 <<= 1;
    order++;
  }

  if (jsuGetFreeStack() < 256+sizeof(FFTDATATYPE)*pow2*2) {
    jsExceptionHere(JSET_ERROR, "Insufficient stack for computing FFT");
    return;
  }

  FFTDATATYPE *vReal = (FFTDATATYPE*)alloca(sizeof(FFTDATATYPE)*pow2*2);
  FFTDATATYPE *vImag = &vReal[pow2];

  // load data
  _jswrap_espruino_FFT_getData(vReal, arrReal, pow2);
  _jswrap_espruino_FFT_getData(vImag, arrImag, pow2);

  // do FFT
  FFT(inverse ? -1 : 1, order, vReal, vImag);

  // Put the results back
  // If we had imaginary data then DON'T modulus the result
  bool hasImagResult = jsvIsIterable(arrImag);
  _jswrap_espruino_FFT_setData(arrReal, vReal, hasImagResult?0:vImag, pow2);
  if (hasImagResult)
    _jswrap_espruino_FFT_setData(arrImag, vImag, 0, pow2);
}

/*JSON{
  "type" : "staticmethod",
  "ifndef" : "SAVE_ON_FLASH",
  "class" : "E",
  "name" : "enableWatchdog",
  "generate" : "jswrap_espruino_enableWatchdog",
  "params" : [
    ["timeout","float","The timeout in seconds before a watchdog reset"],
    ["isAuto","JsVar","If undefined or true, the watchdog is kicked automatically. If not, you must call `E.kickWatchdog()` yourself"]
  ],
  "typescript" : "enableWatchdog(timeout: number, isAuto?: boolean): void;"
}
Enable the watchdog timer. This will reset Espruino if it isn't able to return
to the idle loop within the timeout.

If `isAuto` is false, you must call `E.kickWatchdog()` yourself every so often
or the chip will reset.

```
E.enableWatchdog(0.5); // automatic mode                                                        
while(1); // Espruino will reboot because it has not been idle for 0.5 sec
```

```
E.enableWatchdog(1, false);                                                         
setInterval(function() {
  if (everything_ok)
    E.kickWatchdog();
}, 500);
// Espruino will now reset if everything_ok is false,
// or if the interval fails to be called 
```

**NOTE:** This is only implemented on STM32 and nRF5x devices (all official
Espruino boards).

**NOTE:** On STM32 (Pico, WiFi, Original) with `setDeepSleep(1)` you need to
explicitly wake Espruino up with an interval of less than the watchdog timeout
or the watchdog will fire and the board will reboot. You can do this with
`setInterval("", time_in_milliseconds)`.
 */
void jswrap_espruino_enableWatchdog(JsVarFloat time, JsVar *isAuto) {
  if (time<0 || isnan(time)) time=1;
  if (jsvIsUndefined(isAuto) || jsvGetBool(isAuto))
    jsiStatus |= JSIS_WATCHDOG_AUTO;
  else
    jsiStatus &= ~JSIS_WATCHDOG_AUTO;
  jshEnableWatchDog(time);
}

/*JSON{
  "type" : "staticmethod",
  "ifndef" : "SAVE_ON_FLASH",
  "class" : "E",
  "name" : "kickWatchdog",
  "generate" : "jswrap_espruino_kickWatchdog"
}
Kicks a Watchdog timer set up with `E.enableWatchdog(..., false)`. See
`E.enableWatchdog` for more information.

**NOTE:** This is only implemented on STM32 and nRF5x devices (all official
Espruino boards).
 */
void jswrap_espruino_kickWatchdog() {
  jshKickWatchDog();
}

/// Return an array of errors based on the current flags
JsVar *jswrap_espruino_getErrorFlagArray(JsErrorFlags flags) {
  JsVar *arr = jsvNewEmptyArray();
  if (!arr) return 0;
  if (flags&JSERR_RX_FIFO_FULL) jsvArrayPushAndUnLock(arr, jsvNewFromString("FIFO_FULL"));
  if (flags&JSERR_BUFFER_FULL) jsvArrayPushAndUnLock(arr, jsvNewFromString("BUFFER_FULL"));
  if (flags&JSERR_CALLBACK) jsvArrayPushAndUnLock(arr, jsvNewFromString("CALLBACK"));
  if (flags&JSERR_LOW_MEMORY) jsvArrayPushAndUnLock(arr, jsvNewFromString("LOW_MEMORY"));
  if (flags&JSERR_MEMORY) jsvArrayPushAndUnLock(arr, jsvNewFromString("MEMORY"));
  if (flags&JSERR_MEMORY_BUSY) jsvArrayPushAndUnLock(arr, jsvNewFromString("MEMORY_BUSY"));
  if (flags&JSERR_UART_OVERFLOW) jsvArrayPushAndUnLock(arr, jsvNewFromString("UART_OVERFLOW"));

  return arr;
}

/*TYPESCRIPT
type ErrorFlag =
  | "FIFO_FULL"
  | "BUFFER_FULL"
  | "CALLBACK"
  | "LOW_MEMORY"
  | "MEMORY"
  | "UART_OVERFLOW";
*/
/*JSON{
  "type" : "staticmethod",
  "ifndef" : "SAVE_ON_FLASH",
  "class" : "E",
  "name" : "getErrorFlags",
  "generate" : "jswrap_espruino_getErrorFlags",
  "return" : ["JsVar","An array of error flags"],
  "typescript" : "getErrorFlags(): ErrorFlag[]"
}
Get and reset the error flags. Returns an array that can contain:

`'FIFO_FULL'`: The receive FIFO filled up and data was lost. This could be state
transitions for setWatch, or received characters.

`'BUFFER_FULL'`: A buffer for a stream filled up and characters were lost. This
can happen to any stream - Serial,HTTP,etc.

`'CALLBACK'`: A callback (`setWatch`, `setInterval`, `on('data',...)`) caused an
error and so was removed.

`'LOW_MEMORY'`: Memory is running low - Espruino had to run a garbage collection
pass or remove some of the command history

`'MEMORY'`: Espruino ran out of memory and was unable to allocate some data that
it needed.

`'UART_OVERFLOW'` : A UART received data but it was not read in time and was
lost
 */
JsVar *jswrap_espruino_getErrorFlags() {
  JsErrorFlags flags = jsErrorFlags;
  jsErrorFlags = JSERR_NONE;
  return jswrap_espruino_getErrorFlagArray(flags);
}


/*TYPESCRIPT
type Flag =
  | "deepSleep"
  | "pretokenise"
  | "unsafeFlash"
  | "unsyncFiles";
*/
/*JSON{
  "type" : "staticmethod",
  "class" : "E",
  "name" : "getFlags",
  "generate" : "jsfGetFlags",
  "return" : ["JsVar","An object containing flag names and their values"],
  "typescript" : "getFlags(): { [key in Flag]: boolean }"
}
Get Espruino's interpreter flags that control the way it handles your JavaScript
code.

* `deepSleep` - Allow deep sleep modes (also set by setDeepSleep)
* `pretokenise` - When adding functions, pre-minify them and tokenise reserved
  words
* `unsafeFlash` - Some platforms stop writes/erases to interpreter memory to
  stop you bricking the device accidentally - this removes that protection
* `unsyncFiles` - When writing files, *don't* flush all data to the SD card
  after each command (the default is *to* flush). This is much faster, but can
  cause filesystem damage if power is lost without the filesystem unmounted.
*/
/*JSON{
  "type" : "staticmethod",
  "class" : "E",
  "name" : "setFlags",
  "generate" : "jsfSetFlags",
  "params" : [
    ["flags","JsVar","An object containing flag names and boolean values. You need only specify the flags that you want to change."]
  ],
  "typescript" : "setFlags(flags: { [key in Flag]?: boolean }): void"
}
Set the Espruino interpreter flags that control the way it handles your
JavaScript code.

Run `E.getFlags()` and check its description for a list of available flags and
their values.
*/

// TODO Improve TypeScript declaration
/*JSON{
  "type" : "staticmethod",
  "class" : "E",
  "name" : "pipe",
  "ifndef" : "SAVE_ON_FLASH",
  "generate" : "jswrap_pipe",
  "params" : [
    ["source","JsVar","The source file/stream that will send content."],
    ["destination","JsVar","The destination file/stream that will receive content from the source."],
    ["options","JsVar",["An optional object `{ chunkSize : int=64, end : bool=true, complete : function }`","chunkSize : The amount of data to pipe from source to destination at a time","complete : a function to call when the pipe activity is complete","end : call the 'end' function on the destination when the source is finished"]]
  ],
  "typescript" : "pipe(source: any, destination: any, options?: { chunkSize?: number, end?: boolean, complete?: () => void }): void"
}*/

/*JSON{
  "type" : "staticmethod",
  "class" : "E",
  "name" : "toArrayBuffer",
  "generate" : "jswrap_espruino_toArrayBuffer",
  "params" : [
    ["str","JsVar","The string to convert to an ArrayBuffer"]
  ],
  "return" : ["JsVar","An ArrayBuffer that uses the given string"],
  "return_object" : "ArrayBufferView",
  "typescript" : "toArrayBuffer(str: string): ArrayBuffer;"
}
Create an ArrayBuffer from the given string. This is done via a reference, not a
copy - so it is very fast and memory efficient.

Note that this is an ArrayBuffer, not a Uint8Array. To get one of those, do:
`new Uint8Array(E.toArrayBuffer('....'))`.
 */
JsVar *jswrap_espruino_toArrayBuffer(JsVar *str) {
  if (!jsvIsString(str)) return 0;
  return jsvNewArrayBufferFromString(str, 0);
}

/*JSON{
  "type" : "staticmethod",
  "class" : "E",
  "name" : "toString",
  "generate" : "jswrap_espruino_toString",
  "params" : [
    ["args","JsVarArray","The arguments to convert to a String"]
  ],
  "return" : ["JsVar","A String (or `undefined` if a Flat String cannot be created)"],
  "return_object" : "String",
  "typescript" : "toString(...args: any[]): string | undefined;"
}
Returns a 'flat' string representing the data in the arguments, or return
`undefined` if a flat string cannot be created.

This creates a string from the given arguments. If an argument is a String or an
Array, each element is traversed and added as an 8 bit character. If it is
anything else, it is converted to a character directly.

In the case where there's one argument which is an 8 bit typed array backed by a
flat string of the same length, the backing string will be returned without
doing a copy or other allocation. The same applies if there's a single argument
which is itself a flat string.
 */
void (_jswrap_espruino_toString_char)(int ch,  JsvStringIterator *it) {
  jsvStringIteratorSetCharAndNext(it, (char)ch);
}

JsVar *jswrap_espruino_toString(JsVar *args) {
  // One argument
  if (jsvGetArrayLength(args)==1) {
    JsVar *arg = jsvGetArrayItem(args,0);
    // Is it a flat string? If so we're there already - just return it
    if (jsvIsFlatString(arg))
      return arg;
    // In the case where we have a Uint8Array,etc, return it directly
    if (jsvIsArrayBuffer(arg) &&
        JSV_ARRAYBUFFER_GET_SIZE(arg->varData.arraybuffer.type)==1 &&
        arg->varData.arraybuffer.byteOffset==0) {
      JsVar *backing = jsvGetArrayBufferBackingString(arg, NULL);
      if (jsvIsFlatString(backing) &&
          jsvGetCharactersInVar(backing) == arg->varData.arraybuffer.length) {
        jsvUnLock(arg);
        return backing;
      }
      jsvUnLock(backing);
    }
    jsvUnLock(arg);
  }

  unsigned int len = (unsigned int)jsvIterateCallbackCount(args);
  JsVar *str = jsvNewFlatStringOfLength(len);
  if (!str) {
    // if we couldn't do it, try again after garbage collecting
    jsvGarbageCollect();
    str = jsvNewFlatStringOfLength(len);
  }
  if (!str) return 0;
  JsvStringIterator it;
  jsvStringIteratorNew(&it, str, 0);
  jsvIterateCallback(args, (void (*)(int,  void *))_jswrap_espruino_toString_char, &it);
  jsvStringIteratorFree(&it);

  return str;
}

/*TYPESCRIPT
type Uint8ArrayResolvable =
  | number
  | string
  | Uint8ArrayResolvable[]
  | ArrayBuffer
  | ArrayBufferView
  | { data: Uint8ArrayResolvable, count: number }
  | { callback: () => Uint8ArrayResolvable }
*/
/*JSON{
  "type" : "staticmethod",
  "class" : "E",
  "name" : "toUint8Array",
  "generate" : "jswrap_espruino_toUint8Array",
  "params" : [
    ["args","JsVarArray","The arguments to convert to a Uint8Array"]
  ],
  "return" : ["JsVar","A Uint8Array"],
  "return_object" : "Uint8Array",
  "typescript" : "toUint8Array(...args: Uint8ArrayResolvable[]): Uint8Array;"
}
This creates a Uint8Array from the given arguments. These are handled as
follows:

 * `Number` -> read as an integer, using the lowest 8 bits
 * `String` -> use each character's numeric value (e.g.
   `String.charCodeAt(...)`)
 * `Array` -> Call itself on each element
 * `ArrayBuffer` or Typed Array -> use the lowest 8 bits of each element
 * `Object`:
   * `{data:..., count: int}` -> call itself `object.count` times, on
     `object.data`
   * `{callback : function}` -> call the given function, call itself on return
     value

For example:

```
E.toUint8Array([1,2,3])
=new Uint8Array([1, 2, 3])
E.toUint8Array([1,{data:2,count:3},3])
=new Uint8Array([1, 2, 2, 2, 3])
E.toUint8Array("Hello")
=new Uint8Array([72, 101, 108, 108, 111])
E.toUint8Array(["hi",{callback:function() { return [1,2,3] }}])
=new Uint8Array([104, 105, 1, 2, 3])
```
*/
void (_jswrap_espruino_toUint8Array_char)(int ch,  JsvArrayBufferIterator *it) {
  jsvArrayBufferIteratorSetByteValue(it, (char)ch);
  jsvArrayBufferIteratorNext(it);
}

JsVar *jswrap_espruino_toUint8Array(JsVar *args) {
  JsVar *arr = jsvNewTypedArray(ARRAYBUFFERVIEW_UINT8, (JsVarInt)jsvIterateCallbackCount(args));
  if (!arr) return 0;

  JsvArrayBufferIterator it;
  jsvArrayBufferIteratorNew(&it, arr, 0);
  jsvIterateCallback(args, (void (*)(int,  void *))_jswrap_espruino_toUint8Array_char, &it);
  jsvArrayBufferIteratorFree(&it);

  return arr;
}

/*JSON{
  "type" : "staticmethod",
  "class" : "E",
  "name" : "toJS",
  "generate" : "jswrap_espruino_toJS",
  "params" : [
    ["arg","JsVar","The JS variable to convert to a string"]
  ],
  "return" : ["JsVar","A String"],
  "return_object" : "String"
}
This performs the same basic function as `JSON.stringify`, however
`JSON.stringify` adds extra characters to conform to the JSON spec which aren't
required if outputting JS.

`E.toJS` will also stringify JS functions, whereas `JSON.stringify` ignores
them.

For example:

* `JSON.stringify({a:1,b:2}) == '{"a":1,"b":2}'`
* `E.toJS({a:1,b:2}) == '{a:1,b:2}'`

**Note:** Strings generated with `E.toJS` can't be reliably parsed by
`JSON.parse` - however they are valid JS so will work with `eval` (but this has
security implications if you don't trust the source of the string).

On the desktop [JSON5 parsers](https://github.com/json5/json5) will parse the
strings produced by `E.toJS` without trouble.
 */
JsVar *jswrap_espruino_toJS(JsVar *v) {
  JSONFlags flags = JSON_DROP_QUOTES|JSON_NO_UNDEFINED|JSON_ARRAYBUFFER_AS_ARRAY;
  JsVar *result = jsvNewFromEmptyString();
  if (result) {// could be out of memory
    jsfGetJSON(v, result, flags);
  }
  return result;
}

/*JSON{
  "type" : "staticmethod",
  "class" : "E",
  "name" : "memoryArea",
  "generate" : "jswrap_espruino_memoryArea",
  "params" : [
    ["addr","int","The address of the memory area"],
    ["len","int","The length (in bytes) of the memory area"]
  ],
  "return" : ["JsVar","A String"],
  "return_object" : "String"
}
This creates and returns a special type of string, which actually references a
specific memory address. It can be used in order to use sections of Flash memory
directly in Espruino (for example to execute code straight from flash memory
with `eval(E.memoryArea( ... ))`)

**Note:** This is only tested on STM32-based platforms (Espruino Original and
Espruino Pico) at the moment.
*/
JsVar *jswrap_espruino_memoryArea(int addr, int len) {
  if (len<0) return 0;
  // hack for ESP8266/ESP32 where the address can be different
  size_t mappedAddr = jshFlashGetMemMapAddress((size_t)addr);
  return jsvNewNativeString((char*)mappedAddr, (size_t)len);
}

/*JSON{
  "type" : "staticmethod",
  "class" : "E",
  "name" : "setBootCode",
  "generate" : "jswrap_espruino_setBootCode",
  "params" : [
    ["code","JsVar","The code to execute (as a string)"],
    ["alwaysExec","bool","Whether to always execute the code (even after a reset)"]
  ],
  "typescript" : "setBootCode(code: string, alwaysExec?: boolean): void;"
}
This writes JavaScript code into Espruino's flash memory, to be executed on
startup. It differs from `save()` in that `save()` saves the whole state of the
interpreter, whereas this just saves JS code that is executed at boot.

Code will be executed before `onInit()` and `E.on('init', ...)`.

If `alwaysExec` is `true`, the code will be executed even after a call to
`reset()`. This is useful if you're making something that you want to program,
but you want some code that is always built in (for instance setting up a
display or keyboard).

To remove boot code that has been saved previously, use `E.setBootCode("")`

**Note:** this removes any code that was previously saved with `save()`
*/
void jswrap_espruino_setBootCode(JsVar *code, bool alwaysExec) {
  if (jsvIsString(code)) code = jsvLockAgain(code);
  else code = jsvNewFromEmptyString();
  jsfSaveBootCodeToFlash(code, alwaysExec);
  jsvUnLock(code);
}


/*JSON{
  "type" : "staticmethod",
  "ifndef" : "SAVE_ON_FLASH",
  "class" : "E",
  "name" : "setClock",
  "generate" : "jswrap_espruino_setClock",
  "params" : [
    ["options","JsVar","Platform-specific options for setting clock speed"]
  ],
  "return" : ["int","The actual frequency the clock has been set to"],
  "typescript" : "setClock(options: number | { M: number, N: number, P: number, Q: number, latency?: number, PCLK?: number, PCLK2?: number }): number;"
}
This sets the clock frequency of Espruino's processor. It will return `0` if it
is unimplemented or the clock speed cannot be changed.

**Note:** On pretty much all boards, UART, SPI, I2C, PWM, etc will change
frequency and will need setting up again in order to work.

### STM32F4

Options is of the form `{ M: int, N: int, P: int, Q: int }` - see the 'Clocks'
section of the microcontroller's reference manual for what these mean.

* System clock = 8Mhz * N / ( M * P )
* USB clock (should be 48Mhz) = 8Mhz * N / ( M * Q )

Optional arguments are:

* `latency` - flash latency from 0..15
* `PCLK1` - Peripheral clock 1 divisor (default: 2)
* `PCLK2` - Peripheral clock 2 divisor (default: 4)

The Pico's default is `{M:8, N:336, P:4, Q:7, PCLK1:2, PCLK2:4}`, use `{M:8,
N:336, P:8, Q:7, PCLK:1, PCLK2:2}` to halve the system clock speed while keeping
the peripherals running at the same speed (omitting PCLK1/2 will lead to the
peripherals changing speed too).

On STM32F4 boards (e.g. Espruino Pico), the USB clock needs to be kept at 48Mhz
or USB will fail to work. You'll also experience USB instability if the
processor clock falls much below 48Mhz.

### ESP8266

Just specify an integer value, either 80 or 160 (for 80 or 160Mhz)

*/
int jswrap_espruino_setClock(JsVar *options) {
  return (int)jshSetSystemClock(options);
}

/*JSON{
  "type" : "staticmethod",
  "ifndef" : "SAVE_ON_FLASH",
  "class" : "E",
  "name" : "setConsole",
  "generate" : "jswrap_espruino_setConsole",
  "params" : [
    ["device","JsVar",""],
    ["options","JsVar","(optional) object of options, see below"]
  ],
  "typescript" : "setConsole(device: \"Serial1\" | \"USB\" | \"Bluetooth\" | \"Telnet\" | \"Terminal\" | Serial | null, options?: { force?: boolean }): void;"
}
Changes the device that the JS console (otherwise known as the REPL) is attached
to. If the console is on a device, that device can be used for programming
Espruino.

Rather than calling `Serial.setConsole` you can call
`E.setConsole("DeviceName")`.

This is particularly useful if you just want to remove the console.
`E.setConsole(null)` will make the console completely inaccessible.

`device` may be `"Serial1"`,`"USB"`,`"Bluetooth"`,`"Telnet"`,`"Terminal"`, any
other *hardware* `Serial` device, or `null` to disable the console completely.

`options` is of the form:

```
{
  force : bool // default false, force the console onto this device so it does not move
               //   if false, changes in connection state (e.g. USB/Bluetooth) can move
               //   the console automatically.
}
```
*/
void jswrap_espruino_setConsole(JsVar *deviceVar, JsVar *options) {
  bool force = false;
  jsvConfigObject configs[] = {
    {"force", JSV_BOOLEAN, &force}
  };
  if (!jsvReadConfigObject(options, configs, sizeof(configs) / sizeof(jsvConfigObject)))
    return;

  IOEventFlags device = EV_NONE;
  if (jsvIsObject(deviceVar)) {
    device = jsiGetDeviceFromClass(deviceVar);
  } else if (jsvIsString(deviceVar)) {
    char name[JSLEX_MAX_TOKEN_LENGTH];
    jsvGetString(deviceVar, name, JSLEX_MAX_TOKEN_LENGTH);
    device = jshFromDeviceString(name);
  }

  if (device==EV_NONE && !jsvIsNull(deviceVar)) {
    jsExceptionHere(JSET_ERROR, "Unknown device type %q", device);
    return;
  }

  if (device!=EV_NONE && !DEVICE_IS_SERIAL(device)) {
    jsExceptionHere(JSET_ERROR, "setConsole can't be used on 'soft' or non-Serial devices");
    return;
  }
  jsiSetConsoleDevice(device, force);
}

/*JSON{
  "type" : "staticmethod",
  "class" : "E",
  "name" : "getConsole",
  "generate" : "jswrap_espruino_getConsole",
  "return" : ["JsVar","The current console device as a string, or just `null` if the console is null"],
  "typescript" : "getConsole(): string | null"
}
Returns the current console device - see `E.setConsole` for more information.
*/
JsVar *jswrap_espruino_getConsole() {
  IOEventFlags dev = jsiGetConsoleDevice();
  if (dev == EV_NONE) return jsvNewNull();
  return jsvNewFromString(jshGetDeviceString(dev));
}


/*JSON{
  "type" : "staticmethod",
  "ifndef" : "SAVE_ON_FLASH",
  "class" : "E",
  "name" : "reverseByte",
  "generate" : "jswrap_espruino_reverseByte",
  "params" : [
    ["x","int32","A byte value to reverse the bits of"]
  ],
  "return" : ["int32","The byte with reversed bits"]
}
Reverse the 8 bits in a byte, swapping MSB and LSB.

For example, `E.reverseByte(0b10010000) == 0b00001001`.

Note that you can reverse all the bytes in an array with: `arr =
arr.map(E.reverseByte)`
 */
int jswrap_espruino_reverseByte(int v) {
  unsigned int b = v&0xFF;
  // http://graphics.stanford.edu/~seander/bithacks.html#ReverseByteWith64Bits
  return (((b * 0x0802LU & 0x22110LU) | (b * 0x8020LU & 0x88440LU)) * 0x10101LU >> 16) & 0xFF;
}


/*JSON{
  "type" : "staticmethod",
  "class" : "E",
  "name" : "dumpTimers",
  "ifndef" : "SAVE_ON_FLASH",
  "generate" : "jswrap_espruino_dumpTimers"
}
Output the current list of Utility Timer Tasks - for debugging only
 */
void jswrap_espruino_dumpTimers() {
  jstDumpUtilityTimers();
}

/*JSON{
  "type" : "staticmethod",
  "class" : "E",
  "name" : "dumpLockedVars",
  "ifndef" : "RELEASE",
  "generate" : "jswrap_espruino_dumpLockedVars"
}
Dump any locked variables that aren't referenced from `global` - for debugging
memory leaks only.
*/
#ifndef RELEASE
void jswrap_espruino_dumpLockedVars() {
  jsvDumpLockedVars();
}
#endif

/*JSON{
  "type" : "staticmethod",
  "class" : "E",
  "name" : "dumpFreeList",
  "ifndef" : "RELEASE",
  "generate" : "jswrap_espruino_dumpFreeList"
}
Dump any locked variables that aren't referenced from `global` - for debugging
memory leaks only.
*/
#ifndef RELEASE
void jswrap_espruino_dumpFreeList() {
  jsvDumpFreeList();
}
#endif

/*JSON{
  "type" : "staticmethod",
  "class" : "E",
  "name" : "dumpFragmentation",
  "ifndef" : "SAVE_ON_FLASH",
  "generate" : "jswrap_e_dumpFragmentation"
}
Show fragmentation.

* ` ` is free space
* `#` is a normal variable
* `L` is a locked variable (address used, cannot be moved)
* `=` represents data in a Flat String (must be contiguous)
 */
void jswrap_e_dumpFragmentation() {
  int l = 0;
  for (unsigned int i=0;i<jsvGetMemoryTotal();i++) {
    JsVar *v = _jsvGetAddressOf(i+1);
    if ((v->flags&JSV_VARTYPEMASK)==JSV_UNUSED) {
      jsiConsolePrint(" ");
      if (l++>80) { jsiConsolePrint("\n");l=0; }
    } else {
      if (jsvGetLocks(v)) jsiConsolePrint("L");
      else jsiConsolePrint("#");
      if (l++>80) { jsiConsolePrint("\n");l=0; }
      if (jsvIsFlatString(v)) {
        unsigned int b = (unsigned int)jsvGetFlatStringBlocks(v);
        i += b; // skip forward
        while (b--) {
          jsiConsolePrint("=");
          if (l++>80) { jsiConsolePrint("\n");l=0; }
        }
      }
    }
  }
  jsiConsolePrint("\n");
}

/*JSON{
  "type" : "staticmethod",
  "ifndef" : "SAVE_ON_FLASH",
  "class" : "E",
  "name" : "dumpVariables",
  "generate" : "jswrap_e_dumpVariables"
}
Dumps a comma-separated list of all allocated variables along with the variables
they link to. Can be used to visualise where memory is used.
 */
void jswrap_e_dumpVariables() {
  jsiConsolePrintf("ref,size,flags,name,links...\n");
  for (unsigned int i=0;i<jsvGetMemoryTotal();i++) {
    JsVarRef ref = i+1;
    JsVar *v = _jsvGetAddressOf(ref);
    if ((v->flags&JSV_VARTYPEMASK)==JSV_UNUSED) continue;
    if (jsvIsStringExt(v)) continue;
    unsigned int size = 1;
    if (jsvIsFlatString(v)) {
      unsigned int b = (unsigned int)jsvGetFlatStringBlocks(v);
      i += b; // skip forward
      size += b;
    } else if (jsvHasCharacterData(v)) {
      JsVarRef childref = jsvGetLastChild(v);
      while (childref) {
        JsVar *child = jsvLock(childref);
        size++;
        childref = jsvGetLastChild(child);
        jsvUnLock(child);
      }
    }
    jsiConsolePrintf("%d,%d,%d,",ref,size,v->flags&JSV_VARTYPEMASK);
    if (jsvIsName(v)) jsiConsolePrintf("%q,",v);
    else if (jsvIsNumeric(v)) jsiConsolePrintf("Number %j,",v);
    else if (jsvIsString(v)) {
      JsVar *s;
      if (jsvGetStringLength(v)>20) {
        s = jsvNewFromStringVar(v, 0, 17);
        jsvAppendString(s,"...");
      } else
        s = jsvLockAgain(v);
      jsiConsolePrintf("String %j,",s);
      jsvUnLock(s);
    } else if (jsvIsObject(v)) jsiConsolePrintf("Object,");
    else if (jsvIsArray(v)) jsiConsolePrintf("Array,");
    else jsiConsolePrintf(",");

    if (jsvHasSingleChild(v) || jsvHasChildren(v)) {
      JsVarRef childref = jsvGetFirstChild(v);
      while (childref) {
        JsVar *child = jsvLock(childref);
        jsiConsolePrintf("%d,",childref);
        if (jsvHasChildren(v)) childref = jsvGetNextSibling(child);
        else childref = 0;
        jsvUnLock(child);
      }
    }
    jsiConsolePrint("\n");
  }
}

/*JSON{
  "type" : "staticmethod",
  "ifndef" : "SAVE_ON_FLASH",
  "class" : "E",
  "name" : "defrag",
  "generate" : "jsvDefragment"
}
BETA: defragment memory!
*/

/*TYPESCRIPT
type VariableSizeInformation = {
  name: string;
  size: number;
  more?: VariableSizeInformation;
};
*/
/*JSON{
  "type" : "staticmethod",
  "ifndef" : "SAVE_ON_FLASH",
  "class" : "E",
  "name" : "getSizeOf",
  "generate" : "jswrap_espruino_getSizeOf",
  "params" : [
    ["v","JsVar","A variable to get the size of"],
    ["depth","int","The depth that detail should be provided for. If depth<=0 or undefined, a single integer will be returned"]
  ],
  "return" : ["JsVar","Information about the variable size - see below"],
  "typescript" : [
    "getSizeOf(v: any, depth?: 0): number;",
    "getSizeOf(v: any, depth: number): VariableSizeInformation;"
  ]
}
Return the number of variable blocks used by the supplied variable. This is
useful if you're running out of memory and you want to be able to see what is
taking up most of the available space.

If `depth>0` and the variable can be recursed into, an array listing all
property names (including internal Espruino names) and their sizes is returned.
If `depth>1` there is also a `more` field that inspects the objects' children's
children.

For instance `E.getSizeOf(function(a,b) { })` returns `5`.

But `E.getSizeOf(function(a,b) { }, 1)` returns:

```
 [
  {
    "name": "a",
    "size": 1 },
  {
    "name": "b",
    "size": 1 },
  {
    "name": "\xFFcod",
    "size": 2 }
 ]
```

In this case setting depth to `2` will make no difference as there are no more
children to traverse.

See http://www.espruino.com/Internals for more information
 */
JsVar *jswrap_espruino_getSizeOf(JsVar *v, int depth) {
  if (depth>0 && jsvHasChildren(v)) {
    JsVar *arr = jsvNewEmptyArray();
    if (!arr) return 0;
    JsvObjectIterator it;
    jsvObjectIteratorNew(&it, v);
    while (jsvObjectIteratorHasValue(&it)) {
      JsVar *key = jsvObjectIteratorGetKey(&it);
      JsVar *val = jsvSkipName(key);
      JsVar *item = jsvNewObject();
      if (item) {
        jsvObjectSetChildAndUnLock(item, "name", jsvAsString(key));
        jsvObjectSetChildAndUnLock(item, "size", jswrap_espruino_getSizeOf(key, 0));
        if (depth>1 && jsvHasChildren(val))
          jsvObjectSetChildAndUnLock(item, "more", jswrap_espruino_getSizeOf(val, depth-1));
        jsvArrayPushAndUnLock(arr, item);
      }
      jsvUnLock2(val, key);
      jsvObjectIteratorNext(&it);
    }
    jsvObjectIteratorFree(&it);
    return arr;
  }
  return jsvNewFromInteger((JsVarInt)jsvCountJsVarsUsed(v));
}


/*JSON{
  "type" : "staticmethod",
  "ifndef" : "SAVE_ON_FLASH",
  "class" : "E",
  "name" : "getAddressOf",
  "generate" : "jswrap_espruino_getAddressOf",
  "params" : [
    ["v","JsVar","A variable to get the address of"],
    ["flatAddress","bool","(boolean) If `true` and a Flat String or Flat ArrayBuffer is supplied, return the address of the data inside it - otherwise 0. If `false` (the default) return the address of the JsVar itself."]
  ],
  "return" : ["int","The address of the given variable"]
}
Return the address in memory of the given variable. This can then be used with
`peek` and `poke` functions. However, changing data in JS variables directly
(flatAddress=false) will most likely result in a crash.

This functions exists to allow embedded targets to set up peripherals such as
DMA so that they write directly to JS variables.

See http://www.espruino.com/Internals for more information
 */
JsVarInt jswrap_espruino_getAddressOf(JsVar *v, bool flatAddress) {
  if (flatAddress) {
    size_t len=0;
    return (JsVarInt)(size_t)jsvGetDataPointer(v, &len);
  }
  return (JsVarInt)(size_t)v;
}

/*JSON{
  "type" : "staticmethod",
    "ifndef" : "SAVE_ON_FLASH",
  "class" : "E",
  "name" : "mapInPlace",
  "generate" : "jswrap_espruino_mapInPlace",
  "params" : [
    ["from","JsVar","An ArrayBuffer to read elements from"],
    ["to","JsVar","An ArrayBuffer to write elements too"],
    ["map","JsVar","An array or `function(value,index)` to use to map one element to another, or `undefined` to provide no mapping"],
    ["bits","int","If specified, the number of bits per element (MSB first) - otherwise use a 1:1 mapping. If negative, use LSB first."]
  ],
  "typescript" : "mapInPlace(from: ArrayBuffer, to: ArrayBuffer, map?: number[] | ((value: number, index: number) => number) | undefined, bits?: number): void;"
}
Take each element of the `from` array, look it up in `map` (or call
`map(value,index)` if it is a function), and write it into the corresponding
element in the `to` array.

You can use an array to map:

```
var a = new Uint8Array([1,2,3,1,2,3]);
var lut = new Uint8Array([128,129,130,131]);
E.mapInPlace(a, a, lut);
// a = [129, 130, 131, 129, 130, 131]
```

Or `undefined` to pass straight through, or a function to do a normal 'mapping':

```
var a = new Uint8Array([0x12,0x34,0x56,0x78]);
var b = new Uint8Array(8);
E.mapInPlace(a, b, undefined); // straight through
// b = [0x12,0x34,0x56,0x78,0,0,0,0]
E.mapInPlace(a, b, (value,index)=>index); // write the index in the first 4 (because a.length==4)
// b = [0,1,2,3,4,0,0,0]
E.mapInPlace(a, b, undefined, 4); // 4 bits from 8 bit input -> 2x as many outputs, msb-first
// b = [1, 2, 3, 4, 5, 6, 7, 8]
 E.mapInPlace(a, b, undefined, -4); // 4 bits from 8 bit input -> 2x as many outputs, lsb-first
// b = [2, 1, 4, 3, 6, 5, 8, 7]
E.mapInPlace(a, b, a=>a+2, 4);
// b = [3, 4, 5, 6, 7, 8, 9, 10]
var b = new Uint16Array(4);
E.mapInPlace(a, b, undefined, 12); // 12 bits from 8 bit input, msb-first
// b = [0x123, 0x456, 0x780, 0]
E.mapInPlace(a, b, undefined, -12); // 12 bits from 8 bit input, lsb-first
// b = [0x412, 0x563, 0x078, 0]
```
 */
void jswrap_espruino_mapInPlace(JsVar *from, JsVar *to, JsVar *map, JsVarInt bits) {
  if (!jsvIsArrayBuffer(from) || !jsvIsArrayBuffer(to)) {
    jsExceptionHere(JSET_ERROR, "First 2 arguments should be array buffers");
    return;
  }
  if (map && !jsvIsArray(map) && !jsvIsArrayBuffer(map) && !jsvIsFunction(map)) {
    jsExceptionHere(JSET_ERROR, "Third argument should be a function or array");
    return;
  }
  bool isFn = jsvIsFunction(map);
  int bitsFrom = 8*(int)JSV_ARRAYBUFFER_GET_SIZE(from->varData.arraybuffer.type);
  bool msbFirst = true;
  if (bits<0) {
    bits=-bits;
    msbFirst = false;
  }
  if (bits==0) bits = bitsFrom;

  JsvArrayBufferIterator itFrom,itTo;
  jsvArrayBufferIteratorNew(&itFrom, from, 0);
  JsVarInt el = 0;
  int b = 0;

  jsvArrayBufferIteratorNew(&itTo, to, 0);
  while ((jsvArrayBufferIteratorHasElement(&itFrom) || b>=bits) &&
         jsvArrayBufferIteratorHasElement(&itTo)) {

    JsVar *index = isFn?jsvArrayBufferIteratorGetIndex(&itFrom):0;
    while (b < bits) {
      if (msbFirst) el = (el<<bitsFrom) | jsvArrayBufferIteratorGetIntegerValue(&itFrom);
      else el |= jsvArrayBufferIteratorGetIntegerValue(&itFrom) << b;
      jsvArrayBufferIteratorNext(&itFrom);
      b += bitsFrom;
    }

    JsVarInt v;
    if (msbFirst) {
      v = (el>>(b-bits)) & ((1<<bits)-1);
    } else {
      v = el & ((1<<bits)-1);
      el >>= bits;
    }
    b -= bits;


    if (map) {
      JsVar *v2 = 0;
      if (isFn) {
        JsVar *args[2];
        args[0] = jsvNewFromInteger(v);
        args[1] = index; // child is a variable name, create a new variable for the index
        v2 = jspeFunctionCall(map, 0, 0, false, 2, args);
        jsvUnLock(args[0]);
      } else if (jsvIsArray(map)) {
        v2 = jsvGetArrayItem(map, v);
      } else {
        assert(jsvIsArrayBuffer(map));
        v2 = jsvArrayBufferGet(map, (size_t)v);
      }
      jsvArrayBufferIteratorSetValue(&itTo, v2);
      jsvUnLock(v2);
    } else { // no map - push right through
      jsvArrayBufferIteratorSetIntegerValue(&itTo, v);
    }
    jsvUnLock(index);
    jsvArrayBufferIteratorNext(&itTo);
  }
  jsvArrayBufferIteratorFree(&itFrom);
  jsvArrayBufferIteratorFree(&itTo);
}

/*JSON{
  "type" : "staticmethod",
  "class" : "E",
  "name" : "lookupNoCase",
  "generate" : "jswrap_espruino_lookupNoCase",
  "params" : [
    ["haystack","JsVar","The Array/Object/Function to search"],
    ["needle","JsVar","The key to search for"],
    ["returnKey","bool","If true, return the key, else return the value itself"]
  ],
  "return" : ["JsVar","The value in the Object matching 'needle', or if `returnKey==true` the key's name - or undefined"],
  "typescript" : [
    "lookupNoCase(haystack: any[] | object | Function, needle: string, returnKey?: false): any;",
    "lookupNoCase<T>(haystack: any[] | object | Function, needle: T, returnKey: true): T | undefined;"
  ]
}
Search in an Object, Array, or Function
 */
JsVar *jswrap_espruino_lookupNoCase(JsVar *haystack, JsVar *needle, bool returnKey) {
  if (!jsvHasChildren(haystack)) return 0;
  char needleBuf[64];
  if (jsvGetString(needle, needleBuf, sizeof(needleBuf))==sizeof(needleBuf)) {
    jsExceptionHere(JSET_ERROR, "Search string is too long (>=%d chars)", sizeof(needleBuf));
  }

  if (returnKey) {
    JsVar *key = jsvFindChildFromStringI(haystack, needleBuf);
    if (key) return jsvAsStringAndUnLock(key);
    return 0;
  } else return jsvObjectGetChildI(haystack, needleBuf);
}

/*JSON{
  "type" : "staticmethod",
  "ifndef" : "SAVE_ON_FLASH",
  "class" : "E",
  "name" : "dumpStr",
  "generate" : "jswrap_e_dumpStr",
  "return" : ["JsVar","A String"],
  "return_object" : "String"
}
Get the current interpreter state in a text form such that it can be copied to a
new device
 */
JsVar *jswrap_e_dumpStr() {
  JsVar *result = jsvNewFromEmptyString();
  if (!result) return 0;
  JsvStringIterator it;
  jsvStringIteratorNew(&it, result, 0);
  jsiDumpState((vcbprintf_callback)&jsvStringIteratorPrintfCallback, &it);
  jsvStringIteratorFree(&it);
  return result;
}

/*JSON{
  "type" : "staticmethod",
  "ifndef" : "SAVE_ON_FLASH",
  "class" : "E",
  "name" : "srand",
  "generate" : "srand",
  "params" : [
    ["v","int","The 32 bit integer seed to use for the random number generator"]
  ]
}
Set the seed for the random number generator used by `Math.random()`.
 */

/*JSON{
  "type" : "staticmethod",
  "ifndef" : "SAVE_ON_FLASH",
  "class" : "E",
  "name" : "hwRand",
  "generate" : "jshGetRandomNumber",
  "return" : ["int32","A random number"]
}
Unlike 'Math.random()' which uses a pseudo-random number generator, this method
reads from the internal voltage reference several times, XOR-ing and rotating to
try and make a relatively random value from the noise in the signal.
 */

/*JSON{
  "type" : "staticmethod",
  "ifndef" : "SAVE_ON_FLASH",
  "class" : "E",
  "name" : "CRC32",
  "generate" : "jswrap_espruino_CRC32",
  "params" : [
    ["data","JsVar","Iterable data to perform CRC32 on (each element treated as a byte)"]
  ],
  "return" : ["JsVar","The CRC of the supplied data"]
}
Perform a standard 32 bit CRC (Cyclic redundancy check) on the supplied data
(one byte at a time) and return the result as an unsigned integer.
 */
JsVar *jswrap_espruino_CRC32(JsVar *data) {
  JsvIterator it;
  jsvIteratorNew(&it, data, JSIF_EVERY_ARRAY_ELEMENT);
  uint32_t crc = 0xFFFFFFFF;
  while (jsvIteratorHasElement(&it)) {
    crc ^= (uint8_t)jsvIteratorGetIntegerValue(&it);
    for (int t=0;t<8;t++)
      crc = (crc>>1) ^ (0xEDB88320 & -(crc & 1));
    jsvIteratorNext(&it);
  }
  jsvIteratorFree(&it);
  return jsvNewFromLongInteger(~crc);
}

/*JSON{
  "type" : "staticmethod",
  "ifndef" : "SAVE_ON_FLASH",
  "class" : "E",
  "name" : "HSBtoRGB",
  "generate" : "jswrap_espruino_HSBtoRGB",
  "params" : [
    ["hue","float","The hue, as a value between 0 and 1"],
    ["sat","float","The saturation, as a value between 0 and 1"],
    ["bri","float","The brightness, as a value between 0 and 1"],
    ["format","int","If `true` or `1`, return an array of [R,G,B] values betwen 0 and 255. If `16`, return a 16 bit number. `undefined`/`24` is the same as normal (returning a 24 bit number)"]
  ],
  "return" : ["JsVar","A 24 bit number containing bytes representing red, green, and blue `0xBBGGRR`. Or if `asArray` is true, an array `[R,G,B]`"],
  "typescript" : [
    "HSBtoRGB(hue: number, sat: number, bri: number, format?: false): number;",
    "HSBtoRGB(hue: number, sat: number, bri: number, format: 16): number;",
    "HSBtoRGB(hue: number, sat: number, bri: number, format: 24): number;",
    "HSBtoRGB(hue: number, sat: number, bri: number, format: true): [number, number, number];"
  ]
}
Convert hue, saturation and brightness to red, green and blue (packed into an
integer if `asArray==false` or an array if `asArray==true`).

This replaces `Graphics.setColorHSB` and `Graphics.setBgColorHSB`. On devices
with 24 bit colour it can be used as: `Graphics.setColor(E.HSBtoRGB(h, s, b))`,
or on devices with 26 bit colour use `Graphics.setColor(E.HSBtoRGB(h, s, b, 16))`

You can quickly set RGB items in an Array or Typed Array using
`array.set(E.HSBtoRGB(h, s, b, true), offset)`, which can be useful with arrays
used with `require("neopixel").write`.
 */
int jswrap_espruino_HSBtoRGB_int(JsVarFloat hue, JsVarFloat sat, JsVarFloat bri) {
  int   r, g, b, hi, bi, x, y, z;
  JsVarFloat hfrac;

  if ( bri == 0.0 ) return 0;
  else if ( sat == 0.0 ) {
    r = (int)(bri * 255);
    return (r<<16) | (r<<8) | r;
  }
  else {
    hue = (hue-floor(hue))*6; // auto-wrap hue
    hi = (int)hue;
    hfrac = hue - hi;
    hi = hi % 6;

    bri *= 255;
    bi = (int)bri;

    x = (int) ((1 - sat) * bri);
    y = (int) ((1 - sat*hfrac) * bri);
    z = (int) ((1 - sat*(1 - hfrac)) * bri);

    if  ( hi == 0 ) { r = bi;   g = z;    b = x; } else
      if  ( hi == 1 ) { r = y;    g = bi;   b = x; } else
        if  ( hi == 2 ) { r = x;    g = bi;   b = z; } else
          if  ( hi == 3 ) { r = x;    g = y;    b = bi; } else
            if  ( hi == 4 ) { r = z;    g = x;    b = bi; } else
              if  ( hi == 5 ) { r = bi;   g = x;    b = y; } else
              {  r = 0;    g = 0;    b = 0; }

    return (b<<16) | (g<<8) | r;
  }
}
JsVar *jswrap_espruino_HSBtoRGB(JsVarFloat hue, JsVarFloat sat, JsVarFloat bri, int format) {
  int rgb = jswrap_espruino_HSBtoRGB_int(hue, sat, bri);
  if (format==0 || format==24) return jsvNewFromInteger(rgb);
  int r = rgb&0xFF;
  int g = (rgb>>8)&0xFF;
  int b = (rgb>>16)&0xFF;
  if (format==16) return jsvNewFromInteger((unsigned int)((b>>3) | (g>>2)<<5 | (r>>3)<<11));
  if (format!=1) {
    jsExceptionHere(JSET_ERROR, "HSBtoRGB's format arg expects undefined/1/16/24");
    return 0;
  }
  JsVar *arrayElements[] = {
      jsvNewFromInteger(r),
      jsvNewFromInteger(g),
      jsvNewFromInteger(b)
  };
  JsVar *arr = jsvNewArray(arrayElements, 3);
  jsvUnLockMany(3, arrayElements);
  return arr;
}

/*JSON{
  "type" : "staticmethod",
  "class" : "E",
  "name" : "setPassword",
  "generate" : "jswrap_espruino_setPassword",
  "params" : [
    ["password","JsVar","The password - max 20 chars"]
  ],
  "typescript" : "setPassword(password: string): void;"
}
Set a password on the console (REPL). When powered on, Espruino will then demand
a password before the console can be used. If you want to lock the console
immediately after this you can call `E.lockConsole()`

To remove the password, call this function with no arguments.

**Note:** There is no protection against multiple password attempts, so someone
could conceivably try every password in a dictionary.

**Note:** This password is stored in memory in plain text. If someone is able to
execute arbitrary JavaScript code on the device (e.g., you use `eval` on input
from unknown sources) or read the device's firmware then they may be able to
obtain it.
 */
void jswrap_espruino_setPassword(JsVar *pwd) {
  if (pwd)
    pwd = jsvAsString(pwd);
  jsvUnLock(jsvObjectSetChild(execInfo.hiddenRoot, PASSWORD_VARIABLE_NAME, pwd));
}

/*JSON{
  "type" : "staticmethod",
  "class" : "E",
  "name" : "lockConsole",
  "generate" : "jswrap_espruino_lockConsole"
}
If a password has been set with `E.setPassword()`, this will lock the console so
the password needs to be entered to unlock it.
*/
void jswrap_espruino_lockConsole() {
  JsVar *pwd = jsvObjectGetChild(execInfo.hiddenRoot, PASSWORD_VARIABLE_NAME, 0);
  if (pwd)
    jsiStatus |= JSIS_PASSWORD_PROTECTED;
  jsvUnLock(pwd);
}

/*JSON{
  "type" : "staticmethod",
  "class" : "E",
  "name" : "setTimeZone",
  "generate" : "jswrap_espruino_setTimeZone",
  "params" : [
    ["zone","float","The time zone in hours"]
  ]
}
Set the time zone to be used with `Date` objects.

For example `E.setTimeZone(1)` will be GMT+0100

Note that `E.setTimeZone()` will have no effect when daylight savings time rules
have been set with `E.setDST()`. The timezone value will be stored, but never
used so long as DST settings are in effect.

Time can be set with `setTime`.
*/
void jswrap_espruino_setTimeZone(JsVarFloat zone) {
  jsvObjectSetChildAndUnLock(execInfo.hiddenRoot, JS_TIMEZONE_VAR,
      jsvNewFromInteger((int)(zone*60)));
}

#ifndef ESPR_NO_DAYLIGHT_SAVING
/*JSON{
  "type" : "staticmethod",
  "ifndef" : "ESPR_NO_DAYLIGHT_SAVING",
  "class" : "E",
  "name" : "setDST",
  "generate" : "jswrap_espruino_setDST",
  "params" : [
      ["params","JsVarArray","An array containing the settings for DST"]
  ],
  "typescript" : "setDST(dstOffset: number, timezone: number, startDowNumber: number, startDow: number, startMonth: number, startDayOffset: number, startTimeOfDay: number, endDowNumber: number, endDow: number, endMonth: number, endDayOffset: number, endTimeOfDay: number): void"
}
Set the daylight savings time parameters to be used with `Date` objects.

The parameters are
- dstOffset: The number of minutes daylight savings time adds to the clock
  (usually 60) - set to 0 to disable DST
- timezone: The time zone, in minutes, when DST is not in effect - positive east
  of Greenwich
- startDowNumber: The index of the day-of-week in the month when DST starts - 0
  for first, 1 for second, 2 for third, 3 for fourth and 4 for last
- startDow: The day-of-week for the DST start calculation - 0 for Sunday, 6 for
  Saturday
- startMonth: The number of the month that DST starts - 0 for January, 11 for
  December
- startDayOffset: The number of days between the selected day-of-week and the
  actual day that DST starts - usually 0
- startTimeOfDay: The number of minutes elapsed in the day before DST starts
- endDowNumber: The index of the day-of-week in the month when DST ends - 0 for
  first, 1 for second, 2 for third, 3 for fourth and 4 for last
- endDow: The day-of-week for the DST end calculation - 0 for Sunday, 6 for
  Saturday
- endMonth: The number of the month that DST ends - 0 for January, 11 for
  December
- endDayOffset: The number of days between the selected day-of-week and the
  actual day that DST ends - usually 0
- endTimeOfDay: The number of minutes elapsed in the day before DST ends

To determine what the `dowNumber, dow, month, dayOffset, timeOfDay` parameters
should be, start with a sentence of the form "DST starts on the last Sunday of
March (plus 0 days) at 03:00". Since it's the last Sunday, we have
startDowNumber = 4, and since it's Sunday, we have startDow = 0. That it is
March gives us startMonth = 2, and that the offset is zero days, we have
startDayOffset = 0. The time that DST starts gives us startTimeOfDay = 3*60.

"DST ends on the Friday before the second Sunday in November at 02:00" would
give us endDowNumber=1, endDow=0, endMonth=10, endDayOffset=-2 and
endTimeOfDay=120.

Using Ukraine as an example, we have a time which is 2 hours ahead of GMT in
winter (EET) and 3 hours in summer (EEST). DST starts at 03:00 EET on the last
Sunday in March, and ends at 04:00 EEST on the last Sunday in October. So
someone in Ukraine might call `E.setDST(60,120,4,0,2,0,180,4,0,9,0,240);`

Note that when DST parameters are set (i.e. when `dstOffset` is not zero),
`E.setTimeZone()` has no effect.
*/
void jswrap_espruino_setDST(JsVar *params) {
  JsVar *dst;
  JsvIterator it;
  unsigned int i = 0;

  if (!jsvIsArray(params)) return;
  if (jsvGetLength(params) != 12) return;
  dst = jswrap_typedarray_constructor(ARRAYBUFFERVIEW_INT16, params, 0, 0);
  jsvObjectSetChildAndUnLock(execInfo.hiddenRoot, JS_DST_SETTINGS_VAR, dst);
}
#endif

/*JSON{
  "type" : "staticmethod",
  "ifndef" : "SAVE_ON_FLASH",
  "class" : "E",
  "name" : "memoryMap",
  "generate" : "jswrap_espruino_memoryMap",
  "params" : [
    ["baseAddress","JsVar","The base address (added to every address in `registers`)"],
    ["registers","JsVar","An object containing `{name:address}`"]
  ],
  "return" : ["JsVar","An object where each field is memory-mapped to a register."],
  "typescript" : "memoryMap<T extends string>(baseAddress: number, registers: { [key in T]: number }): { [key in T]: number };"
}
Create an object where every field accesses a specific 32 bit address in the
microcontroller's memory. This is perfect for accessing on-chip peripherals.

```
// for NRF52 based chips
var GPIO = E.memoryMap(0x50000000,{OUT:0x504, OUTSET:0x508, OUTCLR:0x50C, IN:0x510, DIR:0x514, DIRSET:0x518, DIRCLR:0x51C});
GPIO.DIRSET = 1; // set GPIO0 to output
GPIO.OUT ^= 1; // toggle the output state of GPIO0
```
*/
JsVar *jswrap_espruino_memoryMap(JsVar *baseAddress, JsVar *registers) {
  /* Do this in JS - it's safer and more readable, and doesn't
   * have to be super fast. */
  JsVar *args[2] = {baseAddress, registers};
  return jspExecuteJSFunction("(function(base,j) {"
    "var o={},addr;"
    "for (var reg in j) {"
      "addr=base+j[reg];"
      "Object.defineProperty(o,reg,{"
        "get:peek32.bind(undefined,addr),"
        "set:poke32.bind(undefined,addr)"
      "});"
    "}"
    "return o;"
  "})",0,2,args);
}

/*JSON{
  "type" : "staticmethod",
  "ifndef" : "SAVE_ON_FLASH",
  "class" : "E",
  "name" : "asm",
  "generate" : "jswrap_espruino_asm",
  "params" : [
    ["callspec","JsVar","The arguments this assembly takes - e.g. `void(int)`"],
    ["assemblycode","JsVarArray","One of more strings of assembler code"]
  ],
  "typescript" : "asm(callspec: string, ...assemblycode: string[]): any;"
}
Provide assembly to Espruino.

**This function is not part of Espruino**. Instead, it is detected by the
Espruino IDE (or command-line tools) at upload time and is replaced with machine
code and an `E.nativeCall` call.

See [the documentation on the Assembler](http://www.espruino.com/Assembler) for
more information.
*/
void jswrap_espruino_asm(JsVar *callspec, JsVar *args) {
  NOT_USED(callspec);
  NOT_USED(args);
  jsExceptionHere(JSET_ERROR, "'E.asm' calls should have been replaced by the Espruino tools before upload");
}

/*JSON{
  "type" : "staticmethod",
  "ifndef" : "SAVE_ON_FLASH",
  "class" : "E",
  "name" : "compiledC",
  "generate" : "jswrap_espruino_compiledC",
  "params" : [
    ["code","JsVar","A Templated string of C code"]
  ],
  "typescript": "compiledC(code: string): any;"
}
Provides the ability to write C code inside your JavaScript file.

**This function is not part of Espruino**. Instead, it is detected by the
Espruino IDE (or command-line tools) at upload time, is sent to our web service
to be compiled, and is replaced with machine code and an `E.nativeCall` call.

See [the documentation on Inline C](http://www.espruino.com/InlineC) for more
information and examples.
*/
void jswrap_espruino_compiledC(JsVar *code) {
  NOT_USED(code);
  jsExceptionHere(JSET_ERROR, "'E.compiledC' calls should have been replaced by the Espruino tools before upload");
}

/*JSON{
  "type" : "staticmethod",
  "class" : "E",
  "name" : "reboot",
  "generate" : "jswrap_espruino_reboot"
}
Forces a hard reboot of the microcontroller - as close as possible to if the
reset pin had been toggled.

**Note:** This is different to `reset()`, which performs a software reset of
Espruino (resetting the interpreter and pin states, but not all the hardware)
*/
void jswrap_espruino_reboot() {
  // ensure `E.on('kill',...` gets called and everything is torn down correctly
  jsiKill();
  jsvKill();
  jshKill();

  jshReboot();
}


// ----------------------------------------- USB Specific Stuff

#ifdef USE_USB_HID
#include "usbd_cdc_hid.h"

/*JSON{
  "type" : "staticmethod",
  "ifdef" : "USE_USB_HID",
  "class" : "E",
  "name" : "setUSBHID",
  "generate" : "jswrap_espruino_setUSBHID",
  "params" : [
    ["opts","JsVar","An object containing at least reportDescriptor, an array representing the report descriptor. Pass undefined to disable HID."]
  ],
  "typescript" : "setUSBHID(opts?: { reportDescriptor: any[] }): void;"
}
USB HID will only take effect next time you unplug and re-plug your Espruino. If
you're disconnecting it from power you'll have to make sure you have `save()`d
after calling this function.
 */
void jswrap_espruino_setUSBHID(JsVar *arr) {
  if (jsvIsUndefined(arr)) {
    // Disable HID
    jsvObjectRemoveChild(execInfo.hiddenRoot, JS_USB_HID_VAR_NAME);
    return;
  }
  if (!jsvIsObject(arr)) {
    jsExceptionHere(JSET_TYPEERROR, "Expecting Object, got %t", arr);
    return;
  }

  JsVar *report = jsvObjectGetChild(arr, "reportDescriptor", 0);
  if (!(jsvIsArray(report) || jsvIsArrayBuffer(report) || jsvIsString(report))) {
    jsExceptionHere(JSET_TYPEERROR, "Object.reportDescriptor should be an Array or String, got %t", arr);
    jsvUnLock(report);
    return;
  }
  JsVar *s = jswrap_espruino_toString(report);
  jsvUnLock(report);
  // Enable HID
  jsvObjectSetChildAndUnLock(execInfo.hiddenRoot, JS_USB_HID_VAR_NAME, s);
}

/*JSON{
  "type" : "staticmethod",
  "ifdef" : "USE_USB_HID",
  "class" : "E",
  "name" : "sendUSBHID",
  "generate" : "jswrap_espruino_sendUSBHID",
  "params" : [
    ["data","JsVar","An array of bytes to send as a USB HID packet"]
  ],
  "return" : ["bool","1 on success, 0 on failure"],
  "typescript" : "sendUSBHID(data: string | ArrayBuffer | number[]): boolean;"
}
 */
bool jswrap_espruino_sendUSBHID(JsVar *arr) {
  unsigned char data[HID_DATA_IN_PACKET_SIZE];
  unsigned int l = jsvIterateCallbackToBytes(arr, data, HID_DATA_IN_PACKET_SIZE);
  if (l>HID_DATA_IN_PACKET_SIZE) return 0;

  return USBD_HID_SendReport(data, l) == USBD_OK;
}

#endif


/*JSON{
  "type" : "staticmethod",
  "#if" : "defined(PUCKJS) || defined(PIXLJS) || defined(BANGLEJS)",
  "class" : "E",
  "name" : "getBattery",
  "generate" : "jswrap_espruino_getBattery",
  "return" : ["int","A percentage between 0 and 100"]
}
In devices that come with batteries, this function returns the battery charge
percentage as an integer between 0 and 100.

**Note:** this is an estimation only, based on battery voltage. The temperature
of the battery (as well as the load being drawn from it at the time
`E.getBattery` is called) will affect the readings.
*/
JsVarInt jswrap_espruino_getBattery() {
#if defined(CUSTOM_GETBATTERY)
  JsVarInt CUSTOM_GETBATTERY();
  return CUSTOM_GETBATTERY();
#else
  return 0;
#endif
}

/*JSON{
  "type" : "staticmethod",
  "#if" : "defined(PICO) || defined(ESPRUINOWIFI) || defined(ESPRUINOBOARD)",
  "class" : "E",
  "name" : "setRTCPrescaler",
  "generate" : "jswrap_espruino_setRTCPrescaler",
  "params" : [
    ["prescaler","int","The amount of counts for one second of the RTC - this is a 15 bit integer value (0..32767)"]
  ]
}
Sets the RTC's prescaler's maximum value. This is the counter that counts up on
each oscillation of the low speed oscillator. When the prescaler counts to the
value supplied, one second is deemed to have passed.

By default this is set to the oscillator's average speed as specified in the
datasheet, and usually that is fine. However on early [Espruino Pico](/Pico)
boards the STM32F4's internal oscillator could vary by as much as 15% from the
value in the datasheet. In that case you may want to alter this value to reflect
the true RTC speed for more accurate timekeeping.

To change the RTC's prescaler value to a computed value based on comparing
against the high speed oscillator, just run the following command, making sure
it's done a few seconds after the board starts up:

```
E.setRTCPrescaler(E.getRTCPrescaler(true));
```

When changing the RTC prescaler, the RTC 'follower' counters are reset and it
can take a second or two before readings from getTime are stable again.

To test, you can connect an input pin to a known frequency square wave and then
use `setWatch`. If you don't have a frequency source handy, you can check
against the high speed oscillator:

```
// connect pin B3 to B4
analogWrite(B3, 0.5, {freq:0.5});
setWatch(function(e) {
  print(e.time - e.lastTime);
}, B4, {repeat:true});
```

**Note:** This is only used on official Espruino boards containing an STM32
microcontroller. Other boards (even those using an STM32) don't use the RTC and
so this has no effect.
 */
void jswrap_espruino_setRTCPrescaler(int prescale) {
#ifdef STM32
  if (prescale<0 || prescale>32767) {
    jsExceptionHere(JSET_ERROR, "Out of range");
    return;
  }
  jshSetupRTCPrescalerValue((unsigned)prescale);
  jshResetRTCTimer();
#else
  NOT_USED(prescale);
#endif
}

/*JSON{
  "type" : "staticmethod",
  "#if" : "defined(PICO) || defined(ESPRUINOWIFI) || defined(ESPRUINOBOARD)",
  "class" : "E",
  "name" : "getRTCPrescaler",
  "generate" : "jswrap_espruino_getRTCPrescaler",
  "params" : [
    ["calibrate","bool","If `false`, the current value. If `true`, the calculated 'correct' value"]
  ],
  "return" : ["int","The RTC prescaler's current value"]
}
Gets the RTC's current prescaler value if `calibrate` is undefined or false.

If `calibrate` is true, the low speed oscillator's speed is calibrated against
the high speed oscillator (usually +/- 20 ppm) and a suggested value to be fed
into `E.setRTCPrescaler(...)` is returned.

See `E.setRTCPrescaler` for more information.
 */
int jswrap_espruino_getRTCPrescaler(bool calibrate) {
#ifdef STM32
  return jshGetRTCPrescalerValue(calibrate);
#else
  NOT_USED(calibrate);
  return 0;
#endif
}

/*JSON{
  "type" : "staticmethod",
  "ifndef" : "SAVE_ON_FLASH",
  "class" : "E",
  "name" : "decodeUTF8",
  "generate" : "jswrap_espruino_decodeUTF8",
  "params" : [
    ["str","JsVar","A string of UTF8-encoded data"],
    ["lookup","JsVar","An array containing a mapping of character code -> replacement string"],
    ["replaceFn","JsVar","If not in lookup, `replaceFn(charCode)` is called and the result used if it's a function, *or* if it's a string, the string value is used"]
  ],
  "return" : ["JsVar","A string containing all UTF8 sequences flattened to 8 bits"],
  "typescript" : "decodeUTF8(str: string, lookup: string[], replaceFn: string | ((charCode: number) => string)): string;"
}
Decode a UTF8 string.

* Any decoded character less than 256 gets passed straight through
* Otherwise if `lookup` is an array and an item with that char code exists in `lookup` then that is used
* Otherwise if `lookup` is an object and an item with that char code (as lowercase hex) exists in `lookup` then that is used
* Otherwise `replaceFn(charCode)` is called and the result used if `replaceFn` is a function
* If `replaceFn` is a string, that is used
* Or finally if nothing else matches, the character is ignored

For instance:

```
let unicodeRemap = {
  0x20ac:"\u0080", // Euro symbol
  0x2026:"\u0085", // Ellipsis
};
E.decodeUTF8("UTF-8 Euro: \u00e2\u0082\u00ac", unicodeRemap, '[?]') == "UTF-8 Euro: \u0080"
```

 */
JsVar *jswrap_espruino_decodeUTF8(JsVar *str, JsVar *lookup, JsVar *replaceFn) {
  if (!(jsvIsString(str))) {
    jsExceptionHere(JSET_ERROR, "Expecting first argument to be a string, not %t", str);
    return 0;
  }
  JsvStringIterator it, dit;
  JsVar *dst = jsvNewFromEmptyString();
  jsvStringIteratorNew(&it, str, 0);
  jsvStringIteratorNew(&dit, dst, 0);
  while (jsvStringIteratorHasChar(&it)) {
    unsigned char c = (unsigned char)jsvStringIteratorGetCharAndNext(&it);
    int cp=c; // Code point defaults to ASCII
    int ra=0; // Read ahead
    if (c>127) {
      if ((c&0xE0)==0xC0) { // 2-byte code starts with 0b110xxxxx
        cp=c&0x1F;ra=1;
      } else if ((c&0xF0)==0xE0) { // 3-byte code starts with 0b1110xxxx
        cp=c&0x0F;ra=2;
      } else if ((c&0xF8)==0xF0) { // 4-byte code starts with 0b11110xxx
        cp=c&0x07;ra=3;
      }
      while (ra--) {
        cp=(cp<<6)|((unsigned char)jsvStringIteratorGetCharAndNext(&it) & 0x3F);
      }
    }
    if (cp<=255){
      jsvStringIteratorAppend(&dit, (char)cp); // ASCII (including extended)
    } else {
      JsVar* replace = 0;
      if (jsvIsArray(lookup))
        replace = jsvGetArrayItem(lookup, cp);
      else if (jsvIsObject(lookup)) {
        JsVar *index = jsvNewFromInteger(cp);
        replace = jsvSkipNameAndUnLock(jsvFindChildFromVar(lookup, index, false));
        jsvUnLock(index);
      }
      if (!replace && jsvIsFunction(replaceFn)) {
        JsVar *v = jsvNewFromInteger(cp);
        replace = jspExecuteFunction(replaceFn, NULL, 1, &v);
        jsvUnLock(v);
      }
      if (!replace && jsvIsString(replaceFn))
        replace = jsvLockAgain(replaceFn);
      if (replace) {
        replace = jsvAsStringAndUnLock(replace);
        jsvStringIteratorAppendString(&dit, replace, 0, JSVAPPENDSTRINGVAR_MAXLENGTH);
        jsvUnLock(replace);
      }
    }
  }
  jsvStringIteratorFree(&it);
  jsvStringIteratorFree(&dit);
  return dst;
}

