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
 * Contains JavaScript function just for the Dickens smartwatch
 * ----------------------------------------------------------------------------
 */
/* DO_NOT_INCLUDE_IN_DOCS - this is a special token for common.py */

#include "jswrap_dickens.h"
#include "jswrap_graphics.h"
#include "jswrap_math.h"
#include "jsinteractive.h"
#include "lcd_spilcd.h"

/*JSON{
  "type": "class",
  "class" : "Dickens",
  "ifdef" : "BANGLEJS"
}
*/

/*JSON{
    "type" : "staticmethod", "class" : "Bangle", "name" : "drawWidgets", "patch":true,
    "generate_js" : "libs/js/dickens/Bangle_drawWidgets_DICKENS.min.js",
    "#if" : "defined(BANGLEJS) && defined(DICKENS)"
}
*/
/*JSON{
    "type" : "staticmethod", "class" : "E", "name" : "showMenu", "patch":true,
    "generate_js" : "libs/js/dickens/E_showMenu_DICKENS.min.js",
    "#if" : "defined(BANGLEJS) && defined(DICKENS)"
}
*/
/*JSON{
    "type" : "staticmethod", "class" : "E", "name" : "showPrompt", "patch":true,
    "generate_js" : "libs/js/dickens/E_showPrompt_DICKENS.min.js",
    "#if" : "defined(BANGLEJS) && defined(DICKENS)"
}
*/
/*JSON{
    "type" : "staticmethod", "class" : "E", "name" : "showMessage", "patch":true,
    "generate_js" : "libs/js/dickens/E_showMessage_DICKENS.min.js",
    "#if" : "defined(BANGLEJS) && defined(DICKENS)"
}
*/
/*JSON{
    "type" : "staticmethod", "class" : "Bangle", "name" : "setUI", "patch":true,
    "generate_js" : "libs/js/dickens/Bangle_setUI_DICKENS.min.js",
    "#if" : "defined(BANGLEJS) && defined(DICKENS)"
}
*/

// add a radial point to an array
void addRadialPoint(JsVar *arr, double r, double a) {
  jsvArrayPushAndUnLock(arr, jsvNewFromFloat(119.0 + (r*jswrap_math_sin(a))));
  jsvArrayPushAndUnLock(arr, jsvNewFromFloat(119.0 - (r*jswrap_math_cos(a))));
}

/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "fillArc",
  "ifdef" : "DICKENS",
  "generate" : "jswrap_graphics_fillArc",
  "params" : [
    ["a1","float","Angle 1 (radians)"],
    ["a2","float","Angle 2 (radians)"],
    ["r","float","Radius"]
  ],
  "return" : ["JsVar","The instance of Graphics this was called on, to allow call chaining"],
  "return_object" : "Graphics"
}
Draw a filled arc between two angles
*/
JsVar *jswrap_graphics_fillArc(JsVar *parent, double a1, double a2, double r) {
  if (a2<a1) return jsvLockAgain(parent);
  if ((a2-a1)>6.28) a2=a1+6.28; // No need for more than a full circle - if the polygon has too many points, a memory leak will happen
  double a;
  const int res = 8;
  JsVar *poly = jsvNewEmptyArray();
  for (double i=a1*res;i<a2*res;i++) {
    a = i/res;
    addRadialPoint(poly, r, a);
  }
  addRadialPoint(poly, r, a2);

  jswrap_graphics_fillPoly_X(parent, poly, true/*antialias*/);
  jsvUnLock(poly);
  return parent; // jswrap_graphics_fillPoly_X clready locked it
}


/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "fillSeg",
  "ifdef" : "DICKENS",
  "generate" : "jswrap_graphics_fillSeg",
  "params" : [
    ["a","float","Angle (radians)"],
    ["ar","float","Angle either side (radians)"],
    ["r1","float","Radius"],
    ["r2","float","Radius"]
  ],
  "return" : ["JsVar","The instance of Graphics this was called on, to allow call chaining"],
  "return_object" : "Graphics"
}
Draw rectangle between angles a-ar and a+ar, and radius r1/r2
*/
JsVar *jswrap_graphics_fillSeg(JsVar *parent, double a, double ar, double r1, double r2) {
  double a1 = a-ar;
  double a2 = a+ar;

  JsVar *poly = jsvNewEmptyArray();
  addRadialPoint(poly, r1, a1);
  addRadialPoint(poly, r1, a2);
  addRadialPoint(poly, r2, a2);
  addRadialPoint(poly, r2, a1);
  jswrap_graphics_fillPoly_X(parent, poly, true/*antialias*/);
  jsvUnLock(poly);
  return parent; // jswrap_graphics_fillPoly_X clready locked it
}


/*JSON{
  "type" : "method",
  "class" : "Graphics",
  "name" : "drawSeg",
  "ifdef" : "DICKENS",
  "generate" : "jswrap_graphics_drawSeg",
  "params" : [
    ["a","float","Angle (radians)"],
    ["ar","float","Angle either side (radians)"],
    ["r","float","Radius"]
  ],
  "return" : ["JsVar","The instance of Graphics this was called on, to allow call chaining"],
  "return_object" : "Graphics"
}
Draw A line between angles a-ar and a+ar at radius r
*/
JsVar *jswrap_graphics_drawSeg(JsVar *parent, double a, double ar, double r) {
  double a1 = a-ar;
  double a2 = a+ar;
  return jswrap_graphics_drawLineAA(parent,
    119.0 + (r*jswrap_math_sin(a1)),
    119.0 - (r*jswrap_math_cos(a1)),
    119.0 + (r*jswrap_math_sin(a2)),
    119.0 - (r*jswrap_math_cos(a2))
  );
}


#ifdef DICKENS
/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "setLCDRotation",
    "generate" : "jswrap_banglejs_setLCDRotation",
    "params" : [
      ["d","int","The number of degrees to the LCD display (0, 90, 180 or 270)"]
    ],
    "ifdef" : "BANGLEJS"
}
Sets the rotation of the LCD display (relative to its nominal orientation)
*/
void jswrap_banglejs_setLCDRotation(int d) {
#ifdef LCD_ROTATION
  // If the LCD is already rotated in the board definition file, add this
  d += LCD_ROTATION;
  if (d>=360) d-=360;
#endif
  uint8_t regValue;
  // Register values are OK for GC9A01 on Dickens, but will need to be different on other hardware.
  switch (d) {
    case 0:
      regValue = 0x88;
      break;
    case 90:
      regValue = 0x78;
      break;
    case 180:
      regValue = 0x48;
      break;
    case 270:
      regValue = 0xB8;
      break;
    default:
      jsExceptionHere(JSET_ERROR, "setLCDRotation expects a rotation value of 0, 90, 180 or 270");
  }
  lcdCmd_SPILCD(0x36, 1, (const uint8_t *)&regValue);
}
#endif