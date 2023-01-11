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
 * Unistroke handling functionality
 * ----------------------------------------------------------------------------
 */
#include "jswrap_unistroke.h"
#include "unistroke.h"

/*JSON{
    "type" : "class",
    "class" : "Unistroke",
    "ifdef" : "BANGLEJS2"
}
This class provides functionality to recognise gestures drawn on a touchscreen.
It is only built into Bangle.js 2.

Usage:

```
var strokes = {
  stroke1 : Unistroke.new(new Uint8Array([x1, y1, x2, y2, x3, y3, ...])),
  stroke2 : Unistroke.new(new Uint8Array([x1, y1, x2, y2, x3, y3, ...])),
  stroke3 : Unistroke.new(new Uint8Array([x1, y1, x2, y2, x3, y3, ...]))
};
var r = Unistroke.recognise(strokes,new Uint8Array([x1, y1, x2, y2, x3, y3, ...]))
print(r); // stroke1/stroke2/stroke3
```

*/

/*JSON{
    "type" : "staticmethod",
    "class" : "Unistroke",
    "name" : "new",
    "ifdef" : "BANGLEJS2",
    "generate" : "jswrap_unistroke_new",
    "params" : [
      ["xy","JsVar","An array of interleaved XY coordinates"]
    ],
    "return" : ["JsVar","A string of data representing this unistroke"]
}
Create a new Unistroke based on XY coordinates
*/
JsVar *jswrap_unistroke_new(JsVar *xy) {
  return unistroke_convert(xy);
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Unistroke",
    "name" : "recognise",
    "ifdef" : "BANGLEJS2",
    "generate" : "jswrap_unistroke_recognise",
    "params" : [
      ["strokes","JsVar","An object of named strokes : `{arrow:..., circle:...}`"],
      ["xy","JsVar","An array of interleaved XY coordinates"]
    ],
    "return" : ["JsVar","The key name of the matched stroke"]
}
Recognise based on an object of named strokes, and a list of XY coordinates
*/
JsVar *jswrap_unistroke_recognise(JsVar *strokes, JsVar *xy) {
  return unistroke_recognise_xy(strokes, xy);
}
