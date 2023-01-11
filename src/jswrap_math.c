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
 * Contains built-in functions for Maths
 * ----------------------------------------------------------------------------
 */
#include "jswrap_math.h"
#include "jsvariterator.h"

static bool isNegativeZero(double x) {
  double NEGATIVE_ZERO = -0.0;
  return *((long long*)&x) == *((long long*)&NEGATIVE_ZERO);
}

#ifdef SAVE_ON_FLASH_EXTREME
// Replace `dsub` with a negate + add
// GCC requires '-ffreestanding' option for this to work
/*
00039540 g     F .text	000006ec .hidden __aeabi_dadd
0003a938 g     F .text	00000730 .hidden __aeabi_dsub
*/
double __aeabi_dsub(double a, double b) {
  // flip top bit of 64 bit number (the sign bit)
  ((uint32_t*)&b)[1] ^= 0x80000000; // assume little endian
  return a + b;
}
#endif

double jswrap_math_sin(double x) {
#ifdef SAVE_ON_FLASH_MATH
  /* To save on flash, do our own sin function that's slower/nastier
   * but is smaller! If we pull in gcc's it adds:
   * __kernel_rem_pio2    2054 bytes
   * __ieee754_rem_pio2   1060 bytes
   * __kernel_cos         584 bytes
   * __kernel_sin         364 bytes
   *
   * So about 3k, just for sin.
   * */
  // exploit symmetry - we're only accurate when x is small
  int xi = (int)(x/PI);
  x -= xi*PI;
  if (x>PI/2) x=PI-x;
  // Taylor series expansion of 'sin'
  double r = x; // running total
  double x2 = x*x; // precalculate x^2
  double xpow = x; // running power
  unsigned int factorial = 1; // running factorial
  unsigned int i;
  for (i=1;i<10;i++) {
    xpow = xpow*x2;
    factorial *= (i*2)*((i*2)+1);
    double term = xpow / factorial;
    if (i&1) r-=term; else r+=term;
  }
  // symmetry
  if (xi&1) r=-r;
  return r;
#else
  return sin(x);
#endif
}

/*JSON{
  "type" : "class",
  "class" : "Math"
}
This is a standard JavaScript class that contains useful Maths routines
*/

// -------------------------------------------------------------------- Math
/*JSON{
  "type" : "staticproperty",
  "class" : "Math",
  "name" : "E",
  "generate_full" : "2.718281828459045",
  "return" : ["float","The value of E - 2.718281828459045"]
}*/
/*JSON{
  "type" : "staticproperty",
  "class" : "Math",
  "name" : "PI",
  "generate_full" : "PI",
  "return" : ["float","The value of PI - 3.141592653589793"]
}*/
/*JSON{
  "type" : "staticproperty",
  "class" : "Math",
  "name" : "LN2",
  "generate_full" : "0.6931471805599453",
  "return" : ["float","The natural logarithm of 2 - 0.6931471805599453"]
}*/
/*JSON{
  "type" : "staticproperty",
  "class" : "Math",
  "name" : "LN10",
  "generate_full" : "2.302585092994046",
  "return" : ["float","The natural logarithm of 10 - 2.302585092994046"]
}*/
/*JSON{
  "type" : "staticproperty",
  "class" : "Math",
  "name" : "LOG2E",
  "generate_full" : "1.4426950408889634",
  "return" : ["float","The base 2 logarithm of e - 1.4426950408889634"]
}*/
/*JSON{
  "type" : "staticproperty",
  "class" : "Math",
  "name" : "LOG10E",
  "generate_full" : "0.4342944819032518",
  "return" : ["float","The base 10 logarithm of e - 0.4342944819032518"]
}*/
/*JSON{
  "type" : "staticproperty",
  "class" : "Math",
  "name" : "SQRT2",
  "generate_full" : "1.4142135623730951",
  "return" : ["float","The square root of 2 - 1.4142135623730951"]
}*/
/*JSON{
  "type" : "staticproperty",
  "class" : "Math",
  "name" : "SQRT1_2",
  "generate_full" : "0.7071067811865476",
  "return" : ["float","The square root of 1/2 - 0.7071067811865476"]
}*/


/*JSON{
  "type" : "staticmethod",
  "class" : "Math",
  "name" : "abs",
  "generate" : "jswrap_math_abs",
  "params" : [
    ["x","float","A floating point value"]
  ],
  "return" : ["float","The absolute value of x (eg, ```Math.abs(2)==2```, but also ```Math.abs(-2)==2```)"]
}*/
JsVarFloat jswrap_math_abs(JsVarFloat x) {
  return (x<0)?-x:x;
}
/*JSON{
  "type" : "staticmethod",
  "ifndef" : "SAVE_ON_FLASH_EXTREME",
  "class" : "Math",
  "name" : "acos",
  "generate_full" : "(PI/2) - jswrap_math_asin(x)",
  "params" : [
    ["x","float","The value to get the arc cosine of"]
  ],
  "return" : ["float","The arc cosine of x, between 0 and PI"]
}*/
/*JSON{
  "type" : "staticmethod",
  "ifndef" : "SAVE_ON_FLASH_EXTREME",
  "class" : "Math",
  "name" : "asin",
  "generate" : "jswrap_math_asin",
  "params" : [
    ["x","float","The value to get the arc sine of"]
  ],
  "return" : ["float","The arc sine of x, between -PI/2 and PI/2"]
}*/
JsVarFloat jswrap_math_asin(JsVarFloat x) {
  return jswrap_math_atan(x / jswrap_math_sqrt(1-x*x));
}
/*JSON{
  "type" : "staticmethod",
  "class" : "Math",
  "name" : "atan",
  "generate" : "jswrap_math_atan",
  "params" : [
    ["x","float","The value to get the arc tangent of"]
  ],
  "return" : ["float","The arc tangent of x, between -PI/2 and PI/2"]
}*/
double jswrap_math_atan(double x) {
#ifdef SAVE_ON_FLASH_MATH
  /* To save on flash, do our own atan function that's slower/nastier
   * but is smaller! */
  // exploit symmetry - we're only accurate when x is small
  double ox = x;
  bool negate = false;
  bool offset = false;
  if (x<0) {
    x = -x;
    negate = true;
  }
  if (x>1) {
    x = 1/x;
    offset = true;
  }

  // Taylor series expansion of 'atan'
  double r = x; // running total
  double x2 = x*x; // precalculate x^2
  double xpow = x; // running power
  unsigned int i;
  for (i=1;i<20;i++) {
    xpow = xpow*x2;
    double term = xpow / ((i*2)+1);
    if (i&1) r-=term; else r+=term;
  }
  // symmetry
  if (offset) r=(PI/2)-r;
  if (negate) r=-r;
  return r;
#else
  return atan(x);
#endif
}

/*JSON{
  "type" : "staticmethod",
  "ifndef" : "SAVE_ON_FLASH",
  "class" : "Math",
  "name" : "atan2",
  "generate" : "jswrap_math_atan2",
  "params" : [
    ["y","float","The Y-part of the angle to get the arc tangent of"],
    ["x","float","The X-part of the angle to get the arc tangent of"]
  ],
  "return" : ["float","The arctangent of Y/X, between -PI and PI"]
}*/
double jswrap_math_atan2(double y, double x) {
#ifdef SAVE_ON_FLASH_MATH
  if (x>0) return jswrap_math_atan(y/x);
  if (x<0) {
    if (y>=0) return jswrap_math_atan(y/x)+PI;
    else return jswrap_math_atan(y/x)-PI;
  } else { // X==0
    if (y>0) return PI/2;
    else if (y<0) return -PI/2;
    else return NAN;
  }
#else
  return atan2(y, x);
#endif
}

/* we use sin here, not cos, to try and save a bit of code space */
/*JSON{
  "type" : "staticmethod",
  "class" : "Math",
  "name" : "cos",
  "generate_full" : "jswrap_math_sin(theta + (PI/2))",
  "params" : [
    ["theta","float","The angle to get the cosine of"]
  ],
  "return" : ["float","The cosine of theta"]
}*/

double jswrap_math_mod(double x, double y) {
  double a, b;
  const double c = x;

  if (!isfinite(x) || isnan(y))
    return NAN;

  if (0 > c) {
    x = -x;
  }
  if (0 > y) {
    y = -y;
  }
  if (y != 0 && DBL_MAX >= y && DBL_MAX >= x) {
    while (x >= y) {
      a = x / 2;
      b = y;
      while (a >= b) {
        b *= 2;
      }
      x -= b;
    }
  } else {
    x = 0;
  }
  return 0 > c ? -x : x;
}

double jswrap_math_pow(double x, double y) {
  double p;
  /* quick hack for raising to a small integer power.
   * exp/log aren't accurate and are relatively slow, so
   * it's probably better to bash through small integer
   * powers in a stupid way. */
  int yi = (int)y;
  if (yi>=0 && yi<10 && yi==y) {
    if (yi==0) return 1.0;
    p = x;
    while (yi>1) {
      p *= x;
      yi--;
    }
    return p;
  }

  /* do proper floating point pow. Not as accurate as a
   * proper pow implementation but this saves a *lot*
   * of flash */
  if (x < 0 && jswrap_math_mod(y, 1) == 0) {
    if (jswrap_math_mod(y, 2) == 0) {
      p = exp(log(-x) * y);
    } else {
      p = -exp(log(-x) * y);
    }
  } else {
    if (x != 0 || 0 >= y) {
      p = exp(log( x) * y);
    } else {
      p = 0;
    }
  }
  return p;
}


/*JSON{
  "type" : "staticmethod",
  "ifndef" : "SAVE_ON_FLASH_EXTREME",
  "class" : "Math",
  "name" : "pow",
  "generate" : "jswrap_math_pow",
  "params" : [
    ["x","float","The value to raise to the power"],
    ["y","float","The power x should be raised to"]
  ],
  "return" : ["float","x raised to the power y (x^y)"]
}*/
/*JSON{
  "type" : "staticmethod",
  "class" : "Math",
  "name" : "random",
  "generate_full" : "(JsVarFloat)rand() / (JsVarFloat)RAND_MAX",
  "return" : ["float","A random number between 0 and 1"]
}*/
/*JSON{
  "type" : "staticmethod",
  "class" : "Math",
  "name" : "round",
  "generate" : "jswrap_math_round",
  "params" : [
    ["x","float","The value to round"]
  ],
  "return" : ["JsVar","x, rounded to the nearest integer"]
}*/
JsVar *jswrap_math_round(double x) {
  if (!isfinite(x) || isNegativeZero(x)) return jsvNewFromFloat(x);
  x += (x<0) ? -0.5 : 0.5;
  long long i = (long long)x;
  if (i==0 && (x<0))
    return jsvNewFromFloat(-0.0); // pass -0 through
  return jsvNewFromLongInteger(i);
}

/*JSON{
  "type" : "staticmethod",
  "class" : "Math",
  "name" : "sin",
  "generate" : "jswrap_math_sin",
  "params" : [
    ["theta","float","The angle to get the sine of"]
  ],
  "return" : ["float","The sine of theta"]
}*/
/*JSON{
  "type" : "staticmethod",
  "class" : "Math",
  "name" : "tan",
  "generate_full" : "jswrap_math_sin(theta) / jswrap_math_sin(theta+(PI/2))",
  "params" : [
    ["theta","float","The angle to get the tangent of"]
  ],
  "return" : ["float","The tangent of theta"]
}*/ // note - we're trying to avoid having to use cos here

/* we could use the real sqrt - but re-use pow to save on code space */
/*JSON{
  "type" : "staticmethod",
  "ifndef" : "SAVE_ON_FLASH_EXTREME",
  "class" : "Math",
  "name" : "sqrt",
  "generate" : "jswrap_math_sqrt",
  "params" : [
    ["x","float","The value to take the square root of"]
  ],
  "return" : ["float","The square root of x"]
}*/

double jswrap_math_sqrt(double x) {
  return (x>=0) ? exp(log(x) * 0.5) : NAN;
}

/*JSON{
  "type" : "staticmethod",
  "class" : "Math",
  "name" : "ceil",
  "generate" : "ceil",
  "params" : [
    ["x","float","The value to round up"]
  ],
  "return" : ["float","x, rounded upwards to the nearest integer"]
}*/
/*JSON{
  "type" : "staticmethod",
  "class" : "Math",
  "name" : "floor",
  "generate" : "floor",
  "params" : [
    ["x","float","The value to round down"]
  ],
  "return" : ["float","x, rounded downwards to the nearest integer"]
}*/

/*JSON{
  "type" : "staticmethod",
  "ifndef" : "SAVE_ON_FLASH_EXTREME",
  "class" : "Math",
  "name" : "exp",
  "generate" : "exp",
  "params" : [
    ["x","float","The value raise E to the power of"]
  ],
  "return" : ["float","E^x"]
}*/
/*JSON{
  "type" : "staticmethod",
  "ifndef" : "SAVE_ON_FLASH_EXTREME",
  "class" : "Math",
  "name" : "log",
  "generate" : "log",
  "params" : [
    ["x","float","The value to take the logarithm (base E) root of"]
  ],
  "return" : ["float","The log (base E) of x"]
}*/

/*JSON{
  "type" : "staticmethod",
  "ifndef" : "SAVE_ON_FLASH",
  "class" : "Math",
  "name" : "clip",
  "generate" : "jswrap_math_clip",
  "params" : [
    ["x","float","A floating point value to clip"],
    ["min","float","The smallest the value should be"],
    ["max","float","The largest the value should be"]
  ],
  "return" : ["float","The value of x, clipped so as not to be below min or above max."]
}
DEPRECATED - Please use `E.clip()` instead. Clip a number to be between min and
max (inclusive)
*/
JsVarFloat jswrap_math_clip(JsVarFloat x, JsVarFloat min, JsVarFloat max) {
  if (x<min) x=min;
  if (x>max) x=max;
  return x;
}

/*JSON{
  "type" : "staticmethod",
  "ifndef" : "SAVE_ON_FLASH",
  "class" : "Math",
  "name" : "wrap",
  "generate" : "wrapAround",
  "params" : [
    ["x","float","A floating point value to wrap"],
    ["max","float","The largest the value should be"]
  ],
  "return" : ["float","The value of x, wrapped so as not to be below min or above max."]
}
DEPRECATED - This is not part of standard JavaScript libraries

Wrap a number around if it is less than 0 or greater than or equal to max. For
instance you might do: ```Math.wrap(angleInDegrees, 360)```
*/

/*JSON{
  "type" : "staticmethod",
  "class" : "Math",
  "name" : "min",
  "generate_full" : "jswrap_math_minmax(args, false)",
  "params" : [
    ["args","JsVarArray","Floating point values to clip"]
  ],
  "return" : ["float","The minimum of the supplied values"]
}
Find the minimum of a series of numbers
*/
/*JSON{
  "type" : "staticmethod",
  "class" : "Math",
  "name" : "max",
  "generate_full" : "jswrap_math_minmax(args, true)",
  "params" : [
    ["args","JsVarArray","Floating point values to clip"]
  ],
  "return" : ["float","The maximum of the supplied values"]
}
Find the maximum of a series of numbers
*/
JsVarFloat jswrap_math_minmax(JsVar *args, bool isMax) {
  JsVarFloat v = isMax ? -INFINITY : INFINITY;

  JsvObjectIterator it;
  jsvObjectIteratorNew(&it, args);
  while (jsvObjectIteratorHasValue(&it)) {
    JsVarFloat arg = jsvGetFloatAndUnLock(jsvObjectIteratorGetValue(&it));
    if ((isMax && arg > v) || (!isMax && arg < v) || isnan(arg))
      v = arg;
    jsvObjectIteratorNext(&it);
  }
  jsvObjectIteratorFree(&it);

  return v;
}

/*JSON{
  "type" : "staticmethod",
  "ifndef" : "SAVE_ON_FLASH_EXTREME",
  "class" : "Math",
  "name" : "sign",
  "generate" : "jswrap_math_sign",
  "params" : [
    ["x","float","The value to get the sign from"]
  ],
  "return" : ["int","sign on x - -1, 1, or 0"]
}*/
int jswrap_math_sign(double x)
{
  if (x == 0 || isNegativeZero(x))
    return 0;
  return x > 0 ? 1 : -1;
}
