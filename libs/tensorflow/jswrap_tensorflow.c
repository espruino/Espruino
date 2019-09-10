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
 * Contains JavaScript interface for Neopixel/WS281x/APA10x devices
 * ----------------------------------------------------------------------------
 */

#include <jswrap_tensorflow.h>
#include "jsvariterator.h"


/*JSON{
  "type" : "library",
  "class" : "tensorflow"
}
*/

/*JSON{
  "type" : "staticmethod",
  "class" : "tensorflow",
  "name" : "test",
  "generate" : "jswrap_tensorflow_test",
  "params" : [
    ["x","float",""]
  ],
  "return" : ["float",""]
}
*/
JsVarFloat jswrap_tensorflow_test(JsVarFloat x) {
  extern float testtensor(float x);
  return testtensor(x);
}
