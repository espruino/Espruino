/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2015 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * This file is designed to be parsed during the build process
 *
 * TV output capability on STM32 devices
 * ----------------------------------------------------------------------------
 */
#include "jsutils.h"
#include "jsvar.h"
#include "jspin.h"
#include "jswrap_tv.h"
#include "tv.h"

#ifndef STM32
#error TV output wont work on non-STM32 devices
#endif

/*JSON{
  "type" : "library",
  "class" : "tv",
  "#if" : "STM32"
}
This library provides TV out capability on the Espruino and Espruino Pico
*/




/*JSON{
  "type" : "staticmethod",
  "class" : "tv",
  "name" : "setup",
  "generate" : "jswrap_tv_setup",
  "params" : [
    ["options","JsVar","Various options for the TV output"],
    ["width","int",""]
  ],
  "return" : ["JsVar","A graphics object"]
}
This initialises the TV output. Options for PAL are as follows:

```
var g = require('tv').setup({ type : "pal",
  video : A7, // Pin - SPI MOSI Pin for Video output
  sync : A6, // Pin - Timer output pin to use for video sync
  width : 384,
  height : 270, // max 270
});
```
*/

JsVar *jswrap_tv_setup(JsVar *options) {
  if (!jsvIsObject(options)) {
    jsExceptionHere(JSET_ERROR, "Expecting an options object, got %t", options);
    return 0;
  }
  JsVar *tvType = jsvObjectGetChild(options, "type",0);
  if (jsvIsStringEqual(tvType, "pal")) {
    jsvUnLock(tvType);
    Pin pinVideo, pinSync;
    int width,height;
    jsvConfigObject configs[] = {
        {"type", 0, 0},
        {"video", JSV_PIN, &pinVideo},
        {"sync", JSV_PIN, &pinSync},
        {"width", JSV_INTEGER, &width},
        {"height", JSV_INTEGER, &height},
    };
    if (jsvReadConfigObject(options, configs, sizeof(configs) / sizeof(jsvConfigObject))) {
      return tv_setup_pal(pinVideo, pinSync, width, height);
    }
    return 0;
  }

  jsExceptionHere(JSET_ERROR, "Unknown TV output type %q", tvType);
  jsvUnLock(tvType);
  return 0;
}
