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
  "ifdef" : "STM32"
}
This library provides TV out capability on the Espruino and Espruino Pico.

See the [[Television]] page for more information.
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
  video : A7, // Pin - SPI MOSI Pin for Video output (MUST BE SPI1)
  sync : A6, // Pin - Timer pin to use for video sync
  width : 384,
  height : 270, // max 270
});
```

and for VGA:

```
var g = require('tv').setup({ type : "vga",
  video : A7, // Pin - SPI MOSI Pin for Video output (MUST BE SPI1)
  hsync : A6, // Pin - Timer pin to use for video sync
  vsync : A5, // Pin - pin to use for video sync
  width : 220,
  height : 240,
  repeat : 2, // amount of times to repeat each line
});
```

or

```
var g = require('tv').setup({ type : "vga",
  video : A7, // Pin - SPI MOSI Pin for Video output (MUST BE SPI1)
  hsync : A6, // Pin - Timer pin to use for video sync
  vsync : A5, // Pin - pin to use for video sync
  width : 220,
  height : 480,
  repeat : 1, // amount of times to repeat each line
});
```

See the [[Television]] page for more information.
*/

JsVar *jswrap_tv_setup(JsVar *options) {
  if (!jsvIsObject(options)) {
    jsExceptionHere(JSET_ERROR, "Expecting an options object, got %t", options);
    return 0;
  }
  JsVar *tvType = jsvObjectGetChild(options, "type",0);
  if (jsvIsStringEqual(tvType, "pal")) {
    jsvUnLock(tvType);
    tv_info_pal inf;
    tv_info_pal_init(&inf);
    jsvConfigObject configs[] = {
        {"type", 0, 0},
        {"video", JSV_PIN, &inf.pinVideo},
        {"sync", JSV_PIN, &inf.pinSync},
        {"width", JSV_INTEGER, &inf.width},
        {"height", JSV_INTEGER, &inf.height},
    };
    if (jsvReadConfigObject(options, configs, sizeof(configs) / sizeof(jsvConfigObject))) {
      return tv_setup_pal(&inf);
    }
    return 0;
  } else if (jsvIsStringEqual(tvType, "vga")) {
    jsvUnLock(tvType);
    tv_info_vga inf;
    tv_info_vga_init(&inf);
    jsvConfigObject configs[] = {
        {"type", 0, 0},
        {"video", JSV_PIN, &inf.pinVideo},
        {"hsync", JSV_PIN, &inf.pinSync},
        {"vsync", JSV_PIN, &inf.pinSyncV},
        {"width", JSV_INTEGER, &inf.width},
        {"height", JSV_INTEGER, &inf.height},
        {"repeat", JSV_INTEGER, &inf.lineRepeat},
    };
    if (jsvReadConfigObject(options, configs, sizeof(configs) / sizeof(jsvConfigObject))) {
      return tv_setup_vga(&inf);
    }
    return 0;
  }

  jsExceptionHere(JSET_ERROR, "Unknown TV output type %q", tvType);
  jsvUnLock(tvType);
  return 0;
}
