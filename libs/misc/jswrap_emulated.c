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
 * Stubs for emulation (eg NRF class)
 * ----------------------------------------------------------------------------
 */
#include <jswrap_emulated.h>

/* DO_NOT_INCLUDE_IN_DOCS - this is a special token for common.py */

/*JSON{
  "type" : "class",
  "class" : "NRF"
}
*/
/*JSON{
  "type" : "class",
  "class" : "Bluetooth",
  "typescript" : null
}
*/

/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "getSecurityStatus",
    "generate_full" : "jsvNewObject()",
    "return" : ["JsVar", "An object" ],
    "return_object" : "NRFSecurityStatus"
}
*/
/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "getAddress",
    "generate_full" : "jsvNewFromString(\"12:34:56:78:90:ab\")",
    "return" : ["JsVar", "An object" ]
}
*/
/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "setServices",
    "generate_full" : "",
    "params" : [
      ["data","JsVar","The service (and characteristics) to advertise"],
      ["options","JsVar","Optional object containing options"]
    ]
}*/
/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "setAdvertising",
    "generate_full" : "",
    "params" : [
      ["data","JsVar","The data to advertise as an object - see below for more info"],
      ["options","JsVar","[optional] An object of options"]
    ]
}*/
/*JSON{
    "type" : "staticmethod",
    "class" : "Bluetooth",
    "name" : "setConsole",
    "generate_full" : "",
    "typescript" : null
}
*/
