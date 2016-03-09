/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2014 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * This file is designed to be parsed during the build process
 *
 * NodeMCU-specific pin namings
 * ----------------------------------------------------------------------------
 */

#include "jswrap_nodemcu.h"

/*JSON{
  "type" : "object",
  "name" : "NodeMCU"
}
This is a built-in class to allow you to use the ESP8266 NodeMCU boards's pin namings to access pins. It is only available on ESP8266-based boards.
*/

// TODO: Sigh - because everyone is using `Pin(..)` now, we can't have a proper 'A0' pin defined because it'd shift all the pins created by`Pin(..)`

/*JSON{
  "type" : "variable",
  "name" : "A0",
  "memberOf" : "NodeMCU",
  "thisParam" : false,
  "generate_full" : "0",
  "return" : ["pin","A Pin"]
}

*/

/*JSON{
  "type" : "variable",
  "name" : "D0",
  "memberOf" : "NodeMCU",
  "thisParam" : false,
  "generate_full" : "16",
  "return" : ["pin","A Pin"]
}

*/
/*JSON{
  "type" : "variable",
  "name" : "D1",
  "memberOf" : "NodeMCU",
  "thisParam" : false,
  "generate_full" : "5",
  "return" : ["pin","A Pin"]
}

*/
/*JSON{
  "type" : "variable",
  "name" : "D2",
  "memberOf" : "NodeMCU",
  "thisParam" : false,
  "generate_full" : "4",
  "return" : ["pin","A Pin"]
}

*/
/*JSON{
  "type" : "variable",
  "name" : "D3",
  "memberOf" : "NodeMCU",
  "thisParam" : false,
  "generate_full" : "0",
  "return" : ["pin","A Pin"]
}

*/
/*JSON{
  "type" : "variable",
  "name" : "D4",
  "memberOf" : "NodeMCU",
  "thisParam" : false,
  "generate_full" : "2",
  "return" : ["pin","A Pin"]
}

*/
/*JSON{
  "type" : "variable",
  "name" : "D5",
  "memberOf" : "NodeMCU",
  "thisParam" : false,
  "generate_full" : "14",
  "return" : ["pin","A Pin"]
}

*/
/*JSON{
  "type" : "variable",
  "name" : "D6",
  "memberOf" : "NodeMCU",
  "thisParam" : false,
  "generate_full" : "12",
  "return" : ["pin","A Pin"]
}

*/
/*JSON{
  "type" : "variable",
  "name" : "D7",
  "memberOf" : "NodeMCU",
  "thisParam" : false,
  "generate_full" : "13",
  "return" : ["pin","A Pin"]
}

*/
/*JSON{
  "type" : "variable",
  "name" : "D8",
  "memberOf" : "NodeMCU",
  "thisParam" : false,
  "generate_full" : "15",
  "return" : ["pin","A Pin"]
}

*/
/*JSON{
  "type" : "variable",
  "name" : "D9",
  "memberOf" : "NodeMCU",
  "thisParam" : false,
  "generate_full" : "3",
  "return" : ["pin","A Pin"]
}

*/
/*JSON{
  "type" : "variable",
  "name" : "D10",
  "memberOf" : "NodeMCU",
  "thisParam" : false,
  "generate_full" : "1",
  "return" : ["pin","A Pin"]
}

*/

