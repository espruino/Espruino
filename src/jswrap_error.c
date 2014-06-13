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
 * JavaScript methods for Errors
 * ----------------------------------------------------------------------------
 */
#include "jswrap_error.h"
#include "jsparse.h"

/*JSON{ "type":"class",
        "class" : "Error",
        "description" : [ "The base class for runtime errors" ]
}*/

/*JSON{ "type":"constructor",
        "class" : "Error",
        "name" : "Error",
        "generate" : "jswrap_error_constructor",
        "description" : [ "Creates an Error object" ],
        "return" : ["JsVar", "An Error object"]
}*/
JsVar *jswrap_error_constructor() {
  JsVar *d = jspNewObject(0,"Error");
  if (!d) return 0;

  return d;
}
