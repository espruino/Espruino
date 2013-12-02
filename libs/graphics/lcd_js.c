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
 * Graphics Backend for drawing via JavaScript callback
 * ----------------------------------------------------------------------------
 */

#include "lcd_arraybuffer.h"
#include "jsvar.h"
#include "jsparse.h"
#include "jsinteractive.h"


void lcdSetPixel_JS(JsGraphics *gfx, short x, short y, unsigned int col) {
  // look up setPixel and execute it!
//  JsVar *lcdProto = jsvObjectGetChild(gfx->graphicsVar, JSPARSE_PROTOTYPE_VAR, 0);
 // if (lcdProto) {
    JsVar *setPixel = jsvObjectGetChild(gfx->graphicsVar/*lcdProto*/, "setPixel", 0);
    if (setPixel) {
      JsVar *args[3];
      args[0] = jsvNewFromInteger(x);
      args[1] = jsvNewFromInteger(y);
      args[2] = jsvNewFromInteger(col);
      jspExecuteFunction(jsiGetParser(), setPixel, gfx->graphicsVar, 3, args);
      jsvUnLock(args[0]);
      jsvUnLock(args[1]);
      jsvUnLock(args[2]);
      jsvUnLock(setPixel);
    }
//    jsvUnLock(lcdProto);
//  }
}

void lcdInit_JS(JsGraphics *gfx, JsVar *setPixelCallback) {
  jsvAddNamedChild(gfx->graphicsVar, setPixelCallback, "setPixel");
}

void lcdSetCallbacks_JS(JsGraphics *gfx) {
  gfx->setPixel = lcdSetPixel_JS;
}
