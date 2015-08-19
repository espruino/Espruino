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
 * Contains built-in functions for Espressif ESP8266 WiFi Access
 *
 * DEPRECATED - YOU SHOULD NOW USE THE ESP8266.js MODULE
 * ----------------------------------------------------------------------------
 */
/* DO_NOT_INCLUDE_IN_DOCS - this is a special token for common.py */

#include "jswrap_esp8266.h"
#include "jshardware.h"
#include "jsinteractive.h"
#include "network.h"
#include "network_esp8266.h"


/*JSON{
  "type" : "library",
  "class" : "ESP8266"
}
Library for the Espressif ESP8266 WiFi Module
*/
/*JSON{
  "type" : "staticmethod",
  "class" : "ESP8266",
  "name" : "connect",
  "generate" : "jswrap_esp8266_connect_device",
  "params" : [
    ["serial","JsVar","The Serial port used for communications with the ESP8266 (must already be setup)"],
    ["callback","JsVar","Function to call back when connected"]
  ],
  "return" : ["JsVar","An ESP8266 Object"],
  "return_object" : "ESP8266"
}
Initialise the WIZnet module and return an Ethernet object
*/
JsVar *jswrap_esp8266_connect_device(JsVar *usart, JsVar *callback) {

  IOEventFlags usartDevice;
  usartDevice = jsiGetDeviceFromClass(usart);
  if (!DEVICE_IS_USART(usartDevice)) {
    jsExceptionHere(JSET_ERROR, "Expecting USART device, got %q", usart);
    return 0;
  }

  JsNetwork net;
  networkCreate(&net, JSNETWORKTYPE_ESP8266);
  net.data.device = usartDevice;
  networkSet(&net);

  JsVar *wifiObj = jspNewObject(0, "ESPWifi");

  net_esp8266_initialise(callback);

  networkFree(&net);

  networkState = NETWORKSTATE_ONLINE;

  return wifiObj;
}

/*JSON{
  "type" : "class",
  "class" : "ESPWifi"
}
An instantiation of an ESP8266 network adaptor
*/

/*JSON{
  "type" : "method",
  "class" : "ESPWifi",
  "name" : "connect",
  "generate" : "jswrap_esp8266_connect",
  "params" : [
    ["ap","JsVar","Access point name"],
    ["key","JsVar","WPA2 key (or undefined for unsecured connection)"],
    ["callback","JsVar","Function to call back with connection status. It has one argument which is one of 'connect'/'disconnect'/'dhcp'"]
  ],
  "return" : ["bool",""]
}
Connect to an access point
*/
bool jswrap_esp8266_connect(JsVar *wlanObj, JsVar *vAP, JsVar *vKey, JsVar *callback) {
  NOT_USED(wlanObj);

  JsNetwork net;
  if (!networkGetFromVar(&net)) return false;

  net_esp8266_connect(vAP, vKey, callback);

  networkFree(&net);

  return true;
}

/*JSON{
  "type" : "method",
  "class" : "ESPWifi",
  "name" : "getIP",
  "generate" : "jswrap_esp8266_getIP",
  "return" : ["JsVar",""]
}
Get the current IP address
*/
JsVar *jswrap_esp8266_getIP(JsVar *wlanObj) {
  NOT_USED(wlanObj);

  if (networkState != NETWORKSTATE_ONLINE) {
    jsError("Not connected to the internet");
    return 0;
  }


  /* If byte 1 is 0 we don't have a valid address */
  JsVar *data = jsvNewWithFlags(JSV_OBJECT);
/*  networkPutAddressAsString(data, "ip", &gWIZNETINFO.ip[0], 4, 10, '.');
  networkPutAddressAsString(data, "subnet", &gWIZNETINFO.sn[0], 4, 10, '.');
  networkPutAddressAsString(data, "gateway", &gWIZNETINFO.gw[0], 4, 10, '.');
  networkPutAddressAsString(data, "dns", &gWIZNETINFO.dns[0], 4, 10, '.');
  networkPutAddressAsString(data, "mac", &gWIZNETINFO.mac[0], 6, 16, 0);*/
  return data;
}


static void _eth_getIP_set_address(JsVar *options, char *name, unsigned char *ptr) {
  JsVar *info = jsvObjectGetChild(options, name, 0);
  if (info) {
    char buf[64];
    jsvGetString(info, buf, sizeof(buf));
    *(unsigned long*)ptr = networkParseIPAddress(buf);
    jsvUnLock(info);
  }
}

/*JSON{
  "type" : "method",
  "class" : "ESPWifi",
  "name" : "setIP",
  "generate" : "jswrap_esp8266_setIP",
  "params" : [
    ["options","JsVar","Object containing IP address options `{ ip : '1,2,3,4', subnet, gateway, dns  }`, or do not supply an object in otder to force DHCP."]
  ],
  "return" : ["bool","True on success"]
}
Set the current IP address for get an IP from DHCP (if no options object is specified)
*/
bool jswrap_esp8266_setIP(JsVar *wlanObj, JsVar *options) {
  NOT_USED(wlanObj);

  if (networkState != NETWORKSTATE_ONLINE) {
    jsError("Not connected to the internet");
    return false;
  }

  bool success = false;

  return success;
}

