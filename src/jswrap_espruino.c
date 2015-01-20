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
#include "jswrapper.h"
#include "jsinteractive.h"
#include "jstimer.h"

/*JSON{
  "type" : "class",
  "class" : "E"
}
This is the built-in JavaScript class for Espruino utility functions.
*/

/*JSON{
  "type" : "staticmethod",
  "ifndef" : "SAVE_ON_FLASH",
  "class" : "E",
  "name" : "getTemperature",
  "generate_full" : "jshReadTemperature()",
  "return" : ["float","The temperature in degrees C"]
}
Use the STM32's internal thermistor to work out the temperature.

While this is implemented on Espruino boards, it may not be implemented on other devices. If so it'll return NaN.

**Note:** This is not entirely accurate and varies by a few degrees from chip to chip. It measures the **die temperature**, so when connected to USB it could be reading 10 over degrees C above ambient temperature. When running from battery with `setDeepSleep(true)` it is much more accurate though.
*/

/*JSON{
  "type" : "staticmethod",
  "ifndef" : "SAVE_ON_FLASH",
  "class" : "E",
  "name" : "getAnalogVRef",
  "generate_full" : "jshReadVRef()",
  "return" : ["float","The voltage (in Volts) that a reading of 1 from `analogRead` actually represents"]
}
Check the internal voltage reference. To work out an actual voltage of an input pin, you can use `analogRead(pin)*E.getAnalogVRef()` 

While this is implemented on Espruino boards, it may not be implemented on other devices. If so it'll return NaN.
*/


int nativeCallGetCType(JsLex *lex) {
  if (lex->tk == LEX_R_VOID) {
    jslMatch(lex, LEX_R_VOID);
    return JSWAT_VOID;
  }
  if (lex->tk == LEX_ID) {
    int t = -1;
    char *name = jslGetTokenValueAsString(lex);
    if (strcmp(name,"int")==0) t=JSWAT_INT32;
    if (strcmp(name,"double")==0) t=JSWAT_JSVARFLOAT;
    if (strcmp(name,"bool")==0) t=JSWAT_BOOL;
    if (strcmp(name,"Pin")==0) t=JSWAT_PIN;
    if (strcmp(name,"JsVar")==0) t=JSWAT_JSVAR;
    jslMatch(lex, LEX_ID);
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
  "return" : ["JsVar","The native function"]
}
ADVANCED: This is a great way to crash Espruino if you're not sure what you are doing

Create a native function that executes the code at the given address. Eg. `E.nativeCall(0x08012345,'double (double,double)')(1.1, 2.2)` 

If you're executing a thumb function, you'll almost certainly need to set the bottom bit of the address to 1.

Note it's not guaranteed that the call signature you provide can be used - there are limits on the number of arguments allowed.

When supplying `data`, if it is a 'flat string' then it will be used directly, otherwise it'll be converted to a flat string and used.
*/
JsVar *jswrap_espruino_nativeCall(JsVarInt addr, JsVar *signature, JsVar *data) {
  unsigned int argTypes = 0;
  if (jsvIsUndefined(signature)) {
    // Nothing to do
  } else if (jsvIsString(signature)) {
    JsLex lex;
    jslInit(&lex, signature);
    int argType;
    bool ok = true;
    int argNumber = 0;
    argType = nativeCallGetCType(&lex);
    if (argType>=0) argTypes |= (unsigned)argType << (JSWAT_BITS * argNumber++);
    else ok = false;
    if (ok) ok = jslMatch(&lex, '(');
    while (ok && lex.tk!=LEX_EOF && lex.tk!=')') {
      argType = nativeCallGetCType(&lex);
      if (argType>=0) {
        argTypes |= (unsigned)argType << (JSWAT_BITS * argNumber++);
        if (lex.tk!=')') ok = jslMatch(&lex, ',');
      } else ok = false;
    }
    if (ok) ok = jslMatch(&lex, ')');
    jslKill(&lex);
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
    jsvUnLock(jsvAddNamedChild(fn, flat, JSPARSE_FUNCTION_CODE_NAME));
    jsvUnLock(flat);
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
  "return" : ["float","The sum of the given buffer"]
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
  jsvIteratorNew(&itsrc, arr);
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
  "return" : ["float","The variance of the given buffer"]
}
Work out the variance of the contents of the given Array, String or ArrayBuffer and return the result. This is equivalent to `v=0;for (i in arr) v+=Math.pow(mean-arr[i],2)`
*/
JsVarFloat jswrap_espruino_variance(JsVar *arr, JsVarFloat mean) {
  if (!(jsvIsIterable(arr))) {
    jsExceptionHere(JSET_ERROR, "Expecting first argument to be iterable, not %t", arr);
    return NAN;
  }
  JsVarFloat variance = 0;

  JsvIterator itsrc;
  jsvIteratorNew(&itsrc, arr);
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
  "return" : ["float","The variance of the given buffer"]
}
Convolve arr1 with arr2. This is equivalent to `v=0;for (i in arr1) v+=arr1[i] * arr2[(i+offset) % arr2.length]`
*/
JsVarFloat jswrap_espruino_convolve(JsVar *arr1, JsVar *arr2, int offset) {
  if (!(jsvIsIterable(arr1)) ||
      !(jsvIsIterable(arr2))) {
    jsExceptionHere(JSET_ERROR, "Expecting first 2 arguments to be iterable, not %t and %t", arr1, arr2);
    return NAN;
  }
  JsVarFloat conv = 0;

  JsvIterator it1;
  jsvIteratorNew(&it1, arr1);
  JsvIterator it2;
  jsvIteratorNew(&it2, arr2);

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
      jsvIteratorNew(&it2, arr2);
    }
  }
  jsvIteratorFree(&it1);
  jsvIteratorFree(&it2);
  return conv;
}

// http://paulbourke.net/miscellaneous/dft/
/*
   This computes an in-place complex-to-complex FFT
   x and y are the real and imaginary arrays of 2^m points.
   dir =  1 gives forward transform
   dir = -1 gives reverse transform
*/
short FFT(short int dir,long m,double *x,double *y)
{
   long n,i,i1,j,k,i2,l,l1,l2;
   double c1,c2,tx,ty,t1,t2,u1,u2,z;

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
      c2 = jswrap_math_sqrt((1.0 - c1) / 2.0);
      if (dir == 1)
         c2 = -c2;
      c1 = jswrap_math_sqrt((1.0 + c1) / 2.0);
   }

   /* Scaling for forward transform */
   if (dir == 1) {
      for (i=0;i<n;i++) {
         x[i] /= (double)n;
         y[i] /= (double)n;
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
  ]
}
Performs a Fast Fourier Transform (fft) on the supplied data and writes it back into the original arrays. Note that if only one array is supplied, the data written back is the modulus of the complex result `sqrt(r*r+i*i)`.
*/
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

  if (jsuGetFreeStack() < 100+sizeof(double)*pow2*2) {
    jsExceptionHere(JSET_ERROR, "Insufficient stack for computing FFT");
    return;
  }

  double *vReal = (double*)alloca(sizeof(double)*pow2);
  double *vImag = (double*)alloca(sizeof(double)*pow2);

  unsigned int i;
  for (i=0;i<pow2;i++) {
    vReal[i]=0;
    vImag[i]=0;
  }

  // load data
  JsvIterator it;
  jsvIteratorNew(&it, arrReal);
  i=0;
  while (jsvIteratorHasElement(&it)) {
    vReal[i++] = jsvIteratorGetFloatValue(&it);
    jsvIteratorNext(&it);
  }
  jsvIteratorFree(&it);

  if (jsvIsIterable(arrImag)) {
    jsvIteratorNew(&it, arrImag);
    i=0;
    while (i<pow2 && jsvIteratorHasElement(&it)) {
      vImag[i++] = jsvIteratorGetFloatValue(&it);
      jsvIteratorNext(&it);
    }
    jsvIteratorFree(&it);
  }

  // do FFT
  FFT(inverse ? -1 : 1, order, vReal, vImag);

  // Put the results back
  bool useModulus = jsvIsIterable(arrImag);

  jsvIteratorNew(&it, arrReal);
  i=0;
  while (jsvIteratorHasElement(&it)) {
    JsVarFloat f;
    if (useModulus)
      f = jswrap_math_sqrt(vReal[i]*vReal[i] + vImag[i]*vImag[i]);
    else
      f = vReal[i];

    jsvUnLock(jsvIteratorSetValue(&it, jsvNewFromFloat(f)));
    i++;
    jsvIteratorNext(&it);
  }
  jsvIteratorFree(&it);
  if (jsvIsIterable(arrImag)) {
    jsvIteratorNew(&it, arrImag);
    i=0;
    while (jsvIteratorHasElement(&it)) {
      jsvUnLock(jsvIteratorSetValue(&it, jsvNewFromFloat(vImag[i++])));
      jsvIteratorNext(&it);
    }
    jsvIteratorFree(&it);
  }
}

/*JSON{
  "type" : "staticmethod",
  "class" : "E",
  "name" : "interpolate",
  "ifndef" : "SAVE_ON_FLASH",
  "generate" : "jswrap_espruino_interpolate",
  "params" : [
    ["array","JsVar","A Typed Array to interpolate between"],
    ["index","float","Floating point index to access"]
  ],
  "return" : ["float","The result of interpolating between (int)index and (int)(index+1)"]
}
Interpolate between two adjacent values in the Typed Array
*/
JsVarFloat jswrap_espruino_interpolate(JsVar *array, JsVarFloat findex) {
  if (!jsvIsArrayBuffer(array)) return 0;
  size_t idx = (size_t)findex;
  JsVarFloat a = findex - (int)idx;
  if (findex<0) {
    idx = 0;
    a = 0;
  }
  if (findex>=jsvGetArrayBufferLength(array)-1) {
    idx = jsvGetArrayBufferLength(array)-1;
    a = 0;
  }
  JsvArrayBufferIterator it;
  jsvArrayBufferIteratorNew(&it, array, idx);
  JsVarFloat fa = jsvArrayBufferIteratorGetFloatValue(&it);
  jsvArrayBufferIteratorNext(&it);
  JsVarFloat fb = jsvArrayBufferIteratorGetFloatValue(&it);
  jsvArrayBufferIteratorFree(&it);
  return fa*(1-a) + fb*a;
}

/*JSON{
  "type" : "staticmethod",
  "class" : "E",
  "name" : "interpolate2d",
  "ifndef" : "SAVE_ON_FLASH",
  "generate" : "jswrap_espruino_interpolate2d",
  "params" : [
    ["array","JsVar","A Typed Array to interpolate between"],
    ["width","int32","Integer 'width' of 2d array"],
    ["x","float","Floating point X index to access"],
    ["y","float","Floating point Y index to access"]
  ],
  "return" : ["float","The result of interpolating in 2d between the 4 surrounding cells"]
}
Interpolate between four adjacent values in the Typed Array, in 2D.
*/
JsVarFloat jswrap_espruino_interpolate2d(JsVar *array, int width, JsVarFloat x, JsVarFloat y) {
  if (!jsvIsArrayBuffer(array)) return 0;
  int yidx = (int)y;
  JsVarFloat ay = y-yidx;
  if (y<0) {
    yidx = 0;
    ay = 0;
  }

  JsVarFloat findex = x + (JsVarFloat)(yidx*width);
  size_t idx = (size_t)findex;
  JsVarFloat ax = findex-(int)idx;
  if (x<0) {
    idx = (size_t)(yidx*width);
    ax = 0;
  }

  JsvArrayBufferIterator it;
  jsvArrayBufferIteratorNew(&it, array, idx);

  JsVarFloat xa,xb;
  int i;

  xa = jsvArrayBufferIteratorGetFloatValue(&it);
  jsvArrayBufferIteratorNext(&it);
  xb = jsvArrayBufferIteratorGetFloatValue(&it);
  JsVarFloat ya = xa*(1-ax) + xb*ax;

  for (i=1;i<width;i++) jsvArrayBufferIteratorNext(&it);

  xa = jsvArrayBufferIteratorGetFloatValue(&it);
  jsvArrayBufferIteratorNext(&it);
  xb = jsvArrayBufferIteratorGetFloatValue(&it);
  jsvArrayBufferIteratorFree(&it);
  JsVarFloat yb = xa*(1-ax) + xb*ax;

  return ya*(1-ay) + yb*ay;
}

/*JSON{
  "type" : "staticmethod",
  "ifndef" : "SAVE_ON_FLASH",
  "class" : "E",
  "name" : "enableWatchdog",
  "generate" : "jswrap_espruino_enableWatchdog",
  "params" : [
    ["timeout","float","The timeout in seconds before a watchdog reset"]
  ]
}
Enable the watchdog timer. This will reset Espruino if it isn't able to return to the idle loop within the timeout. NOTE: This will not work with `setDeepSleep` unless you explicitly wake Espruino up with an interval of less than the timeout.
*/
void jswrap_espruino_enableWatchdog(JsVarFloat time) {
  if (time<0 || isnan(time)) time=1;
  jshEnableWatchDog(time);
}

/*JSON{
  "type" : "staticmethod",
  "ifndef" : "SAVE_ON_FLASH",
  "class" : "E",
  "name" : "getErrorFlags",
  "generate" : "jswrap_espruino_getErrorFlags",
  "return" : ["JsVar","An array of error flags"]
}
Get and reset the error flags. Returns an array that can contain:

`'FIFO_FULL'`: The receive FIFO filled up and data was lost. This could be state transitions for setWatch, or received characters.

`'BUFFER_FULL'`: A buffer for a stream filled up and characters were lost. This can happen to any stream - Serial,HTTP,etc.

`'CALLBACK'`: A callback (s`etWatch`, `setInterval`, `on('data',...)`) caused an error and so was removed.

`'LOW_MEMORY'`: Memory is running low - Espruino had to run a garbage collection pass or remove some of the command history

`'MEMORY'`: Espruino ran out of memory and was unable to allocate some data that it needed.
*/
JsVar *jswrap_espruino_getErrorFlags() {
  JsVar *arr = jsvNewWithFlags(JSV_ARRAY);
  if (!arr) return 0;
  if (jsErrorFlags&JSERR_RX_FIFO_FULL) jsvArrayPushAndUnLock(arr, jsvNewFromString("FIFO_FULL"));
  if (jsErrorFlags&JSERR_BUFFER_FULL) jsvArrayPushAndUnLock(arr, jsvNewFromString("BUFFER_FULL"));
  if (jsErrorFlags&JSERR_CALLBACK) jsvArrayPushAndUnLock(arr, jsvNewFromString("CALLBACK"));
  if (jsErrorFlags&JSERR_LOW_MEMORY) jsvArrayPushAndUnLock(arr, jsvNewFromString("LOW_MEMORY"));
  if (jsErrorFlags&JSERR_MEMORY) jsvArrayPushAndUnLock(arr, jsvNewFromString("MEMORY"));
  jsErrorFlags = JSERR_NONE;
  return arr;
}

/*JSON{
  "type" : "staticmethod",
  "class" : "E",
  "name" : "toArrayBuffer",
  "generate" : "jswrap_espruino_toArrayBuffer",
  "params" : [
    ["str","JsVar","The string to convert to an ArrayBuffer"]
  ],
  "return" : ["JsVar","An ArrayBuffer that uses the given string"],
  "return_object" : "ArrayBufferView"
}
Create an ArrayBuffer from the given string. This is done via a reference, not a copy - so it is very fast and memory efficient.

Note that this is an ArrayBuffer, not a Uint8Array. To get one of those, do: `new Uint8Array(E.toArrayBuffer('....'))`.
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
  "return" : ["JsVar","A String"],
  "return_object" : "String"
}
This creates a string from the given arguments. If an argument is a String or an Array,
each element is traversed and added as an 8 bit character. If it is anything else, it is
converted to a character directly.
*/
void (_jswrap_espruino_toString_char)(int ch,  JsvStringIterator *it) {
  jsvStringIteratorAppend(it, (char)ch);
}

JsVar *jswrap_espruino_toString(JsVar *args) {
  JsVar *str = jsvNewFromEmptyString();
  if (!str) return 0;
  JsvStringIterator it;
  jsvStringIteratorNew(&it, str, 0);
  jsvIterateCallback(args, (void (*)(int,  void *))_jswrap_espruino_toString_char, &it);
  jsvStringIteratorFree(&it);

  return str;
}

/*JSON{
  "type" : "staticmethod",
  "class" : "E",
  "name" : "toUint8Array",
  "generate" : "jswrap_espruino_toUint8Array",
  "params" : [
    ["args","JsVarArray","The arguments to convert to a Uint8Array"]
  ],
  "return" : ["JsVar","A String"],
  "return_object" : "Uint8Array"
}
This creates a Uint8Array from the given arguments. If an argument is a String or an Array,
each element is traversed and added as if it were an 8 bit value. If it is anything else, it is
converted to an 8 bit value directly.
*/
void (_jswrap_espruino_toUint8Array_char)(int ch,  JsvArrayBufferIterator *it) {
  jsvArrayBufferIteratorSetByteValue(it, (char)ch);
  jsvArrayBufferIteratorNext(it);
}

JsVar *jswrap_espruino_toUint8Array(JsVar *args) {
  JsVar *arr = jsvNewTypedArray(ARRAYBUFFERVIEW_UINT8, jsvIterateCallbackCount(args));
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
  "name" : "reverseByte",
  "generate" : "jswrap_espruino_reverseByte",
  "params" : [
    ["x","int32","A byte value to reverse the bits of"]
  ],
  "return" : ["int32","The byte with reversed bits"]
}
Reverse the 8 bits in a byte, swapping MSB and LSB.

For example, `E.reverseByte(0b10010000) == 0b00001001`.

Note that you can reverse all the bytes in an array with: `arr = arr.map(E.reverseByte)`
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
  "ifndef" : "RELEASE",
  "generate" : "jswrap_espruino_dumpTimers"
}
Output the current list of Utility Timer Tasks - for debugging only
*/
void jswrap_espruino_dumpTimers() {
  jstDumpUtilityTimers();
}

/*JSON{
  "type" : "staticmethod",
  "ifndef" : "SAVE_ON_FLASH",
  "class" : "E",
  "name" : "getSizeOf",
  "generate" : "jswrap_espruino_getSizeOf",
  "params" : [
    ["v","JsVar","A variable to get the size of"]
  ],
  "return" : ["int32","The number of variable 'blocks' as an integer"]
}
Return the number of Variable Blocks used by the supplied variable. This is useful if you're running out of memory and you want to be able to see what is taking up most of the available space.

See http://www.espruino.com/Internals for more information
*/
int jswrap_espruino_getSizeOf(JsVar *v) {
  return (int)jsvCountJsVarsUsed(v);
}
