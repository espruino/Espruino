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
  if (x<0 || y<0 || x>=gfx->data.width || y>=gfx->data.height) return;
  // look up setPixel and execute it!
//  JsVar *lcdProto = jsvObjectGetChild(gfx->graphicsVar, JSPARSE_PROTOTYPE_VAR, 0);
 // if (lcdProto) {
    JsVar *setPixel = jsvObjectGetChild(gfx->graphicsVar/*lcdProto*/, "setPixel", 0);
    if (setPixel) {
      JsVar *args[3];
      args[0] = jsvNewFromInteger(x);
      args[1] = jsvNewFromInteger(y);
      args[2] = jsvNewFromInteger(col);
      jspExecuteFunction(setPixel, gfx->graphicsVar, 3, args);
      jsvUnLock(args[0]);
      jsvUnLock(args[1]);
      jsvUnLock(args[2]);
      jsvUnLock(setPixel);
    }
//    jsvUnLock(lcdProto);
//  }
}

void  lcdFillRect_JS(struct JsGraphics *gfx, short x1, short y1, short x2, short y2) {
  if (x1>x2) {
    short t = x1;
    x1 = x2;
    x2 = t;
  }
  if (y1>y2) {
    short t = y1;
    y1 = y2;
    y2 = t;
  }
  if (x1<0) x1=0;
  if (y1<0) y1=0;
  if (x2>=gfx->data.width) x2 = (short)(gfx->data.width - 1);
  if (y2>=gfx->data.height) y2 = (short)(gfx->data.height - 1);
  if (x2<x1 || y2<y1) return; // nope

  if (x1==x2 && y1==y2) {
    lcdSetPixel_JS(gfx,x1,y1,gfx->data.fgColor);
    return;
  }

  JsVar *fillRect = jsvObjectGetChild(gfx->graphicsVar/*lcdProto*/, "fillRect", 0);
  if (fillRect) {
    JsVar *args[5];
    args[0] = jsvNewFromInteger(x1);
    args[1] = jsvNewFromInteger(y1);
    args[2] = jsvNewFromInteger(x2);
    args[3] = jsvNewFromInteger(y2);
    args[4] = jsvNewFromInteger(gfx->data.fgColor);
    jspExecuteFunction(fillRect, gfx->graphicsVar, 5, args);
    jsvUnLock(args[0]);
    jsvUnLock(args[1]);
    jsvUnLock(args[2]);
    jsvUnLock(args[3]);
    jsvUnLock(args[4]);
    jsvUnLock(fillRect);
  }
}

void lcdInit_JS(JsGraphics *gfx, JsVar *setPixelCallback, JsVar *fillRectCallback) {
  jsvAddNamedChild(gfx->graphicsVar, setPixelCallback, "setPixel");
  jsvAddNamedChild(gfx->graphicsVar, fillRectCallback, "fillRect");
}

void lcdSetCallbacks_JS(JsGraphics *gfx) {
  gfx->setPixel = lcdSetPixel_JS;
  gfx->fillRect = lcdFillRect_JS;
}
