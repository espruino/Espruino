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
    JsVar *setPixel = jsvObjectGetChild(gfx->graphicsVar/*lcdProto*/, "iSetPixel", 0);
    if (setPixel) {
      JsVar *args[3];
      args[0] = jsvNewFromInteger(x);
      args[1] = jsvNewFromInteger(y);
      args[2] = jsvNewFromInteger((JsVarInt)col);
      jsvUnLock(jspExecuteFunction(setPixel, gfx->graphicsVar, 3, args));
      jsvUnLockMany(3, args);
      jsvUnLock(setPixel);
    }
//    jsvUnLock(lcdProto);
//  }
}

void  lcdFillRect_JS(struct JsGraphics *gfx, short x1, short y1, short x2, short y2) {
  JsVar *fillRect = jsvObjectGetChild(gfx->graphicsVar/*lcdProto*/, "iFillRect", 0);
  if (fillRect) {
    JsVar *args[5];
    args[0] = jsvNewFromInteger(x1);
    args[1] = jsvNewFromInteger(y1);
    args[2] = jsvNewFromInteger(x2);
    args[3] = jsvNewFromInteger(y2);
    args[4] = jsvNewFromInteger((JsVarInt)gfx->data.fgColor);
    jsvUnLock(jspExecuteFunction(fillRect, gfx->graphicsVar, 5, args));
    jsvUnLockMany(5, args);
    jsvUnLock(fillRect);
  } else
    graphicsFallbackFillRect(gfx, x1,y1,x2,y2);
}

void lcdInit_JS(JsGraphics *gfx, JsVar *setPixelCallback, JsVar *fillRectCallback) {
  jsvObjectSetChild(gfx->graphicsVar, "iSetPixel", setPixelCallback);
  jsvObjectSetChild(gfx->graphicsVar, "iFillRect", fillRectCallback);
}

void lcdSetCallbacks_JS(JsGraphics *gfx) {
  gfx->setPixel = lcdSetPixel_JS;
  gfx->fillRect = lcdFillRect_JS;
}
