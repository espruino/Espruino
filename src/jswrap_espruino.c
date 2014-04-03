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
#include "libs/jswrap_math.h"
#include "jswrapper.h"
#include "jsinteractive.h"

/*JSON{ "type":"class",
        "class" : "E",
        "description" : ["This is the built-in JavaScript class for Espruino utility functions." ]
}*/

/*JSON{ "type":"staticmethod", "ifdef" : "STM32F1", "ifndef" : "SAVE_ON_FLASH",
         "class": "E", "name" : "getTemperature",
         "generate_full" : "jshReadTemperature()",
         "description" : ["Use the STM32's internal thermistor to work out the temperature.",
                          "**Note:** This is very inaccurate (+/- 20 degrees C) and varies from chip to chip. It can be used to work out when temperature rises or falls, but don't expect absolute temperature readings to be useful."],
         "return" : ["float", "The temperature in degrees C"]
}*/

/*JSON{ "type":"staticmethod", "ifdef" : "STM32F1", "ifndef" : "SAVE_ON_FLASH",
         "class": "E", "name" : "getAnalogVRef",
         "generate_full" : "jshReadVRef()",
         "description" : "Check the internal voltage reference. To work out an actual voltage of an input pin, you can use `analogRead(pin)*E.getAnalogVRef()` ",
         "return" : ["float", "The voltage (in Volts) that a reading of 1 from `analogRead` actually represents"]
}*/


int nativeCallGetCType(JsLex *lex) {
  if (lex->tk == LEX_R_VOID) {
    jslMatch(lex, LEX_R_VOID);
    return JSWAT_VOID;
  }
  if (lex->tk == LEX_ID) {
    int t = -1;
    char *name = jslGetTokenValueAsString(lex);
    if (strcmp(name,"int")==0) t=JSWAT_INT32;
    if (strcmp(name,"long")==0) t=JSWAT_JSVARINT;
    if (strcmp(name,"double")==0) t=JSWAT_JSVARFLOAT;
    if (strcmp(name,"bool")==0) t=JSWAT_BOOL;
    if (strcmp(name,"Pin")==0) t=JSWAT_PIN;
    if (strcmp(name,"JsVar")==0) t=JSWAT_JSVAR;
    jslMatch(lex, LEX_ID);
    return t;
  }
  return -1; // unknown
}

/*JSON{ "type":"staticmethod", "ifndef" : "SAVE_ON_FLASH",
         "class" : "E", "name" : "nativeCall",
         "generate" : "jswrap_espruino_nativeCall",
         "description" : ["ADVANCED: This is a great way to crash Espruino if you're not sure what you are doing",
                          "Create a native function that executes the code at the given address. Eg. `E.nativeCall(0x08012345,'double (double,double)')(1.1, 2.2)` ",
                          "If you're executing a thumb function, you'll almost certainly need to set the bottom bit of the address to 1.",
                          "Note it's not guaranteed that the call signature you provide can be used - it has to be something that a function in Espruino already uses."],
         "params" : [ [ "addr", "int", "The address in memory of the function"],
                      [ "sig", "JsVar", "The signature of the call, `returnType (arg1,arg2,...)`. Allowed types are `void`,`bool`,`int`,`long`,`double`,`Pin`,`JsVar`"] ],
         "return" : ["JsVar", "The native function"]
}*/
JsVar *jswrap_espruino_nativeCall(JsVarInt addr, JsVar *signature) {
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
    if (!ok) {
      jsError("Error Parsing signature at argument number %d", argNumber);
      return 0;
    }
  } else {
    jsError("Invalid Signature");
    return 0;
  }

  return jsvNewNativeFunction((void *)addr, argTypes);
}


/*JSON{ "type":"staticmethod", "ifndef" : "SAVE_ON_FLASH",
         "class" : "E", "name" : "clip",
         "generate" : "jswrap_espruino_clip",
         "description" : "Clip a number to be between min and max (inclusive)",
         "params" : [ [ "x", "float", "A floating point value to clip"],
                      [ "min", "float", "The smallest the value should be"],
                      [ "max", "float", "The largest the value should be"] ],
         "return" : ["float", "The value of x, clipped so as not to be below min or above max."]
}*/
JsVarFloat jswrap_espruino_clip(JsVarFloat x, JsVarFloat min, JsVarFloat max) {
  if (x<min) x=min;
  if (x>max) x=max;
  return x;
}


/*JSON{ "type":"staticmethod", "ifndef" : "SAVE_ON_FLASH",
         "class" : "E", "name" : "sum",
         "generate" : "jswrap_espruino_sum",
         "description" : "Sum the contents of the given Array, String or ArrayBuffer and return the result",
         "params" : [ [ "arr", "JsVar", "The array to sum"] ],
         "return" : ["float", "The sum of the given buffer"]
}*/
JsVarFloat jswrap_espruino_sum(JsVar *arr) {
  if (!(jsvIsString(arr) || jsvIsArray(arr) || jsvIsArrayBuffer(arr))) {
    jsError("Expecting first argument to be an array, not %t", arr);
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

/*JSON{ "type":"staticmethod", "ifndef" : "SAVE_ON_FLASH",
         "class" : "E", "name" : "variance",
         "generate" : "jswrap_espruino_variance",
         "description" : "Work out the variance of the contents of the given Array, String or ArrayBuffer and return the result. This is equivalent to `v=0;for (i in arr) v+=Math.pow(mean-arr[i],2)`",
         "params" : [ [ "arr", "JsVar", "The array to work out the variance for"], ["mean", "float", "The mean value of the array" ] ],
         "return" : ["float", "The variance of the given buffer"]
}*/
JsVarFloat jswrap_espruino_variance(JsVar *arr, JsVarFloat mean) {
  if (!(jsvIsIterable(arr))) {
    jsError("Expecting first argument to be iterable, not %t", arr);
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

/*JSON{ "type":"staticmethod", "ifndef" : "SAVE_ON_FLASH",
         "class" : "E", "name" : "convolve",
         "generate" : "jswrap_espruino_convolve",
         "description" : "Convolve arr1 with arr2. This is equivalent to `v=0;for (i in arr1) v+=arr1[i] * arr2[(i+offset) % arr2.length]`",
         "params" : [ [ "arr1", "JsVar", "An array to convolve"],
                      [ "arr2", "JsVar", "An array to convolve"],
                      [ "offset", "int32", "The mean value of the array" ] ],
         "return" : ["float", "The variance of the given buffer"]
}*/
JsVarFloat jswrap_espruino_convolve(JsVar *arr1, JsVar *arr2, int offset) {
  if (!(jsvIsIterable(arr1)) ||
      !(jsvIsIterable(arr2))) {
    jsError("Expecting first 2 arguments to be iterable, not %t and %t", arr1, arr2);
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

/*JSON{ "type":"staticmethod", "ifndef" : "SAVE_ON_FLASH",
         "class" : "E", "name" : "FFT",
         "generate" : "jswrap_espruino_FFT",
         "description" : "Performs a Fast Fourier Transform (fft) on the supplied data and writes it back into the original arrays. Note that if only one array is supplied, the data written back is the modulus of the complex result `sqrt(r*r+i*i)`.",
         "params" : [ [ "arrReal", "JsVar", "An array of real values"],
                      [ "arrImage", "JsVar", "An array of imaginary values (or if undefined, all values will be taken to be 0)"],
                      [ "inverse", "bool", "Set this to true if you want an inverse FFT - otherwise leave as 0" ] ]
}*/
void jswrap_espruino_FFT(JsVar *arrReal, JsVar *arrImag, bool inverse) {
  if (!(jsvIsIterable(arrReal)) ||
      !(jsvIsUndefined(arrImag) || jsvIsIterable(arrImag))) {
    jsError("Expecting first 2 arguments to be iterable or undefined, not %t and %t", arrReal, arrImag);
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
    jsError("Insufficient stack for computing FFT");
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


/*JSON{ "type":"staticmethod", "ifndef" : "SAVE_ON_FLASH",
         "class" : "E", "name" : "enableWatchdog",
         "generate" : "jswrap_espruino_enableWatchdog",
         "description" : "Enable the watchdog timer. This will reset Espruino if it isn't able to return to the idle loop within the timeout. NOTE: This will not work with `setDeepSleep` unless you explicitly wake Espruino up with an interval of less than the timeout.",
         "params" : [ [ "timeout", "float", "The timeout in seconds before a watchdog reset"] ]
}*/
void jswrap_espruino_enableWatchdog(JsVarFloat time) {
  if (time<0 || isnan(time)) time=1;
  jshEnableWatchDog(time);
}
