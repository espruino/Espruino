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
 * Contains built-in functions for CC3000 WiFi Access
  * ----------------------------------------------------------------------------
 */

#include "jswrap_cc3000.h"
#include "jshardware.h"
#include "jsinteractive.h"
#include "cc3000/board_spi.h"
#include "network.h"
// ti driver
#include "cc3000/wlan.h"
#include "cc3000/netapp.h"
#include "cc3000/hci.h"


// ------------------------------ defaults
#define WLAN_SPI          EV_SPI3
#define WLAN_CLK_PIN      (Pin)(JSH_PORTB_OFFSET + 3)
#define WLAN_MISO_PIN     (Pin)(JSH_PORTB_OFFSET + 4)
#define WLAN_MOSI_PIN     (Pin)(JSH_PORTB_OFFSET + 5)
#define WLAN_EN_PIN       (Pin)(JSH_PORTB_OFFSET + 7) // active high
#define WLAN_IRQ_PIN      (Pin)(JSH_PORTB_OFFSET + 8) // active low
#define WLAN_CS_PIN       (Pin)(JSH_PORTB_OFFSET + 6) // active low
// -------------------------------------------

/*JSON{
  "type" : "library",
  "class" : "CC3000"
}

*/
/*JSON{
  "type" : "staticmethod",
  "class" : "CC3000",
  "name" : "connect",
  "generate" : "jswrap_cc3000_connect",
  "params" : [
    ["spi", "JsVar", "Device to use for SPI (or undefined to use the default). SPI should be 1,000,000 baud, and set to 'mode 1'"],
    ["cs", "pin", "The pin to use for Chip Select"],
    ["en", "pin", "The pin to use for Enable"],
    ["irq", "pin", "The pin to use for Interrupts"]
  ],
  "return" : ["JsVar","A WLAN Object"],
  "return_object" : "WLAN"
}
Initialise the CC3000 and return a WLAN object
*/
JsVar *jswrap_cc3000_connect(JsVar *spi, Pin cs, Pin en, Pin irq) {
  IOEventFlags spiDevice;
  if (spi) {
    spiDevice = jsiGetDeviceFromClass(spi);
    if (!DEVICE_IS_SPI(spiDevice)) {
      jsExceptionHere(JSET_ERROR, "Expecting SPI device, got %q", spi);
      return 0;
    }
  } else {
    // SPI config
    // SPI config
    JshSPIInfo inf;
    jshSPIInitInfo(&inf);
    inf.pinSCK =  WLAN_CLK_PIN;
    inf.pinMISO = WLAN_MISO_PIN;
    inf.pinMOSI = WLAN_MOSI_PIN;
    inf.baudRate = 1000000;
    inf.spiMode = SPIF_SPI_MODE_1;  // Mode 1   CPOL= 0  CPHA= 1
    jshSPISetup(WLAN_SPI, &inf);
    spiDevice = WLAN_SPI;
  }
  if (!jshIsPinValid(cs))
    cs = WLAN_CS_PIN;
  if (!jshIsPinValid(en))
    en = WLAN_EN_PIN;
  if (!jshIsPinValid(irq))
    irq = WLAN_IRQ_PIN;

  JsNetwork net;
  networkCreate(&net, JSNETWORKTYPE_CC3000);
  net.data.device = spiDevice;
  net.data.pinCS = cs;
  net.data.pinEN = en;
  net.data.pinIRQ = irq;
  networkSet(&net);

  JsVar *wlanObj = jspNewObject(0, "WLAN");
  cc3000_initialise(wlanObj);

  networkFree(&net);

  return wlanObj;
}

/*JSON{
  "type" : "class",
  "class" : "WLAN"
}
An instantiation of a WiFi network adaptor
*/

/*JSON{
  "type" : "method",
  "class" : "WLAN",
  "name" : "connect",
  "generate" : "jswrap_wlan_connect",
  "params" : [
    ["ap","JsVar","Access point name"],
    ["key","JsVar","WPA2 key (or undefined for unsecured connection)"],
    ["callback","JsVar","Function to call back with connection status. It has one argument which is one of 'connect'/'disconnect'/'dhcp'"]
  ],
  "return" : ["bool","True if connection succeeded, false if it didn't."]
}
Connect to a wireless network
*/
bool jswrap_wlan_connect(JsVar *wlanObj, JsVar *vAP, JsVar *vKey, JsVar *callback) {
  if (!(jsvIsUndefined(callback) || jsvIsFunction(callback))) {
    jsError("Expecting callback Function but got %t", callback);
    return 0;
  }

  JsNetwork net;
  if (!networkGetFromVar(&net)) return false;

  // if previously completely disconnected, try and reconnect
  if (jsvGetBoolAndUnLock(jsvObjectGetChildIfExists(wlanObj,JS_HIDDEN_CHAR_STR"DIS"))) {
    cc3000_initialise(wlanObj);
    jsvObjectSetChildAndUnLock(wlanObj,JS_HIDDEN_CHAR_STR"DIS", jsvNewFromBool(false));
  }

  if (jsvIsFunction(callback)) {
    jsvObjectSetChild(wlanObj, CC3000_ON_STATE_CHANGE, callback);
  }

  jsvObjectSetChild(wlanObj,JS_HIDDEN_CHAR_STR"AP", vAP); // no unlock intended
  jsvObjectSetChild(wlanObj,JS_HIDDEN_CHAR_STR"KEY", vKey); // no unlock intended

  char ap[32];
  char key[32];
  unsigned long security = WLAN_SEC_UNSEC;
  jsvGetString(vAP, ap, sizeof(ap));
  if (jsvIsString(vKey)) {
    security = WLAN_SEC_WPA2;
    jsvGetString(vKey, key, sizeof(key));
  }
  // might want to set wlan_ioctl_set_connection_policy
  bool connected =  wlan_connect(security, ap, (long)strlen(ap), NULL, (unsigned char*)key, (long)strlen(key))==0;

  networkFree(&net);
  // note that we're only online (for networkState) when DHCP succeeds
  return connected;
}

/*JSON{
  "type" : "method",
  "class" : "WLAN",
  "name" : "disconnect",
  "generate" : "jswrap_wlan_disconnect"
}
Completely uninitialise and power down the CC3000. After this you'll have to use
```require("CC3000").connect()``` again.
*/
void jswrap_wlan_disconnect(JsVar *wlanObj) {
  JsNetwork net;
  if (!networkGetFromVar(&net)) return;

  jsvObjectSetChildAndUnLock(wlanObj,JS_HIDDEN_CHAR_STR"DIS", jsvNewFromBool(true));
  networkState = NETWORKSTATE_OFFLINE; // force offline
  //wlan_disconnect();
  wlan_stop();

  networkFree(&net);
}

/*JSON{
  "type" : "method",
  "class" : "WLAN",
  "name" : "reconnect",
  "generate" : "jswrap_wlan_reconnect"
}
Completely uninitialise and power down the CC3000, then reconnect to the old
access point.
*/
void jswrap_wlan_reconnect(JsVar *wlanObj) {
  JsNetwork net;
  if (!networkGetFromVar(&net)) return;

  JsVar *ap = jsvObjectGetChildIfExists(wlanObj,JS_HIDDEN_CHAR_STR"AP");
  JsVar *key = jsvObjectGetChildIfExists(wlanObj,JS_HIDDEN_CHAR_STR"KEY");
  JsVar *cb = jsvObjectGetChildIfExists(wlanObj,CC3000_ON_STATE_CHANGE);
  jswrap_wlan_disconnect(wlanObj);
  jswrap_wlan_connect(wlanObj, ap, key, cb);
  jsvUnLock3(ap, key, cb);

  networkFree(&net);
}



/*JSON{
  "type" : "method",
  "class" : "WLAN",
  "name" : "getIP",
  "generate" : "jswrap_wlan_getIP",
  "return" : ["JsVar",""]
}
Get the current IP address
*/
JsVar *jswrap_wlan_getIP(JsVar *wlanObj) {
  NOT_USED(wlanObj);

  if (networkState != NETWORKSTATE_ONLINE) {
    jsError("Not connected to the internet");
    return 0;
  }

  JsNetwork net;
  if (!networkGetFromVar(&net)) return 0;

  tNetappIpconfigRetArgs ipconfig;
  netapp_ipconfig(&ipconfig);

  networkFree(&net);
  /* If byte 1 is 0 we don't have a valid address */
  if (ipconfig.aucIP[3] == 0) return 0;
  JsVar *data = jsvNewObject();
  networkPutAddressAsString(data, "ip", &ipconfig.aucIP[0], -4, 10, '.');
  networkPutAddressAsString(data, "subnet", &ipconfig.aucSubnetMask[0], -4, 10, '.');
  networkPutAddressAsString(data, "gateway", &ipconfig.aucDefaultGateway[0], -4, 10, '.');
  networkPutAddressAsString(data, "dhcp", &ipconfig.aucDHCPServer[0], -4, 10, '.');
  networkPutAddressAsString(data, "dns", &ipconfig.aucDNSServer[0], -4, 10, '.');
  networkPutAddressAsString(data, "mac", &ipconfig.uaMacAddr[0], -6, 16, 0);

  return data;
}


static void _wlan_getIP_set_address(JsVar *options, char *name, unsigned char *ptr) {
  JsVar *info = jsvObjectGetChildIfExists(options, name);
  if (info) {
    char buf[64];
    jsvGetString(info, buf, sizeof(buf));
    *(unsigned long*)ptr = networkParseIPAddress(buf);
    jsvUnLock(info);
  }
}

/*JSON{
  "type" : "method",
  "class" : "WLAN",
  "name" : "setIP",
  "generate" : "jswrap_wlan_setIP",
  "params" : [
    ["options","JsVar","Object containing IP address options `{ ip : '1,2,3,4', subnet, gateway, dns }`, or do not supply an object in otder to force DHCP."]
  ],
  "return" : ["bool","True on success"]
}
Set the current IP address for get an IP from DHCP (if no options object is
specified).

**Note:** Changes are written to non-volatile memory, but will only take effect
after calling `wlan.reconnect()`
*/
bool jswrap_wlan_setIP(JsVar *wlanObj, JsVar *options) {
  NOT_USED(wlanObj);

  if (networkState != NETWORKSTATE_ONLINE) {
    jsError("Not connected to the internet");
    return false;
  }

  JsNetwork net;
  if (!networkGetFromVar(&net)) return false;

  tNetappIpconfigRetArgs ipconfig;
  netapp_ipconfig(&ipconfig);

  if (jsvIsObject(options)) {
    _wlan_getIP_set_address(options, "ip", &ipconfig.aucIP[0]);
    _wlan_getIP_set_address(options, "subnet", &ipconfig.aucSubnetMask[0]);
    _wlan_getIP_set_address(options, "gateway", &ipconfig.aucDefaultGateway[0]);
    _wlan_getIP_set_address(options, "dns", &ipconfig.aucDNSServer[0]);
  } else {
    // DHCP - just set all values to 0
    *((unsigned long*)&ipconfig.aucIP[0]) = 0;
    *((unsigned long*)&ipconfig.aucSubnetMask) = 0;
    *((unsigned long*)&ipconfig.aucDefaultGateway) = 0;
  }

  bool result =  netapp_dhcp(
      (unsigned long *)&ipconfig.aucIP[0],
      (unsigned long *)&ipconfig.aucSubnetMask[0],
      (unsigned long *)&ipconfig.aucDefaultGateway[0],
      (unsigned long *)&ipconfig.aucDNSServer[0]) == 0;

  networkFree(&net);

  return result;
}

