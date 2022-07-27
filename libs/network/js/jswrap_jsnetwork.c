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
Initialise the network using the callbacks given and return the first argument.
For instance:

```
require("NetworkJS").create({
  create : function(host, port, socketType, options) {
    // Create a socket and return its index, host is a string, port is an integer.
    // If host isn't defined, create a server socket
    console.log("Create",host,port);
    return 1;
  },
  close : function(sckt) {
    // Close the socket. returns nothing
  },
  accept : function(sckt) {
    // Accept the connection on the server socket. Returns socket number or -1 if no connection
    return -1;
  },
  recv : function(sckt, maxLen, socketType) {
    // Receive data. Returns a string (even if empty).
    // If non-string returned, socket is then closed
    return null;//or "";
  },
  send : function(sckt, data, socketType) {
    // Send data (as string). Returns the number of bytes sent - 0 is ok.
    // Less than 0
    return data.length;
  }
});
```

`socketType` is an integer - 2 for UDP, or see SocketType in
https://github.com/espruino/Espruino/blob/master/libs/network/network.h for more
information.
*/
JsVar *jswrap_networkjs_create(JsVar *obj) {
  JsNetwork net;
  networkCreate(&net, JSNETWORKTYPE_JS);
  networkSet(&net);
  networkFree(&net);

  net_js_setObj(obj);

  networkState = NETWORKSTATE_ONLINE;
  return jsvLockAgain(obj);
}
