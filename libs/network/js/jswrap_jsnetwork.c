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
 * Implementation of JsNetwork that calls into JavaScript
 * ----------------------------------------------------------------------------
 */

#include "jswrap_jsnetwork.h"
#include "jshardware.h"
#include "jsinteractive.h"
#include "network.h"
#include "network_js.h"


/*JSON{
  "type" : "library",
  "class" : "NetworkJS"
}
Library that initialises a network device that calls into JavaScript
*/

/*JSON{
  "type" : "staticmethod",
  "class" : "NetworkJS",
  "name" : "create",
  "generate" : "jswrap_networkjs_create",
  "params" : [
    ["obj","JsVar","An object containing functions to access the network device"]
  ],
  "return" : ["JsVar","The object passed in"]
}
Initialise the WIZnet module and return an Ethernet object
*/
JsVar *jswrap_networkjs_create(JsVar *obj) {
  JsNetwork net;
  networkCreate(&net, JSNETWORKTYPE_JS);
  networkSet(&net);

  networkState = NETWORKSTATE_ONLINE;
  return obj;
}
