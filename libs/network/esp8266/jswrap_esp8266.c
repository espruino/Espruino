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
 * ----------------------------------------------------------------------------
 */

#include "jswrap_wiznet.h"
#include "jshardware.h"
#include "jsinteractive.h"
#include "network.h"
// wiznet driver
#include "wizchip_conf.h"

#include "network_esp8266.h"
#include "DHCP/dhcp.h"



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
  "generate" : "jswrap_esp8266_connect",
  "params" : [
    ["serial","JsVar","The Serial port used for communications with the ESP8266 (must already be setup)"],
    ["callback","JsVar","A callback to use when connection is complete"]
  ],
  "return" : ["JsVar","An ESP8266 Object"],
  "return_object" : "ESP8266"
}
Initialise the WIZnet module and return an Ethernet object
*/
JsVar *jswrap_esp8266_connect() {
  JsVar *ethObj = jspNewObject(0, "Ethernet");

  // SPI config
  JshSPIInfo inf;
  jshSPIInitInfo(&inf);
  inf.pinSCK =  ETH_CLK_PIN;
  inf.pinMISO = ETH_MISO_PIN;
  inf.pinMOSI = ETH_MOSI_PIN;
  inf.baudRate = 1000000;
  inf.spiMode = SPIF_SPI_MODE_0;
  jshSPISetup(ETH_SPI, &inf);

  // CS Configuration
  jshSetPinStateIsManual(ETH_CS_PIN, false);
  jshPinOutput(ETH_CS_PIN, 1); // de-assert CS

  // Wiznet
  reg_wizchip_cs_cbfunc(wizchip_select, wizchip_deselect);
  reg_wizchip_spi_cbfunc(wizchip_read, wizchip_write);

  /* wizchip initialize*/
  uint8_t tmp;
  uint8_t memsize[2][8] = { {2,2,2,2,2,2,2,2},{2,2,2,2,2,2,2,2}};

  if(ctlwizchip(CW_INIT_WIZCHIP,(void*)memsize) == -1)
  {
    jsiConsolePrint("WIZCHIP Initialized fail.\r\n");
    return 0;
  }

  /* PHY link status check */
  do {
    if(ctlwizchip(CW_GET_PHYLINK, (void*)&tmp) == -1) {
      jsiConsolePrint("Unknown PHY Link status.\r\n");
      return 0;
    }
  } while (tmp == PHY_LINK_OFF);

  JsNetwork net;
  networkCreate(&net, JSNETWORKTYPE_W5500);
  networkFree(&net);

  networkState = NETWORKSTATE_ONLINE;

  return ethObj;
}

/*JSON{
  "type" : "class",
  "class" : "ESP8266"
}
An instantiation of an ESP8266 network adaptor
*/

/*JSON{
  "type" : "method",
  "class" : "Ethernet",
  "name" : "getIP",
  "generate" : "jswrap_ethernet_getIP",
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

  wiz_NetInfo gWIZNETINFO;
  ctlnetwork(CN_GET_NETINFO, (void*)&gWIZNETINFO);

  /* If byte 1 is 0 we don't have a valid address */
  JsVar *data = jsvNewWithFlags(JSV_OBJECT);
  networkPutAddressAsString(data, "ip", &gWIZNETINFO.ip[0], 4, 10, '.');
  networkPutAddressAsString(data, "subnet", &gWIZNETINFO.sn[0], 4, 10, '.');
  networkPutAddressAsString(data, "gateway", &gWIZNETINFO.gw[0], 4, 10, '.');
  networkPutAddressAsString(data, "dns", &gWIZNETINFO.dns[0], 4, 10, '.');
  networkPutAddressAsString(data, "mac", &gWIZNETINFO.mac[0], 6, 16, 0);
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
  "class" : "Ethernet",
  "name" : "setIP",
  "generate" : "jswrap_ethernet_setIP",
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
  wiz_NetInfo gWIZNETINFO;

  ctlnetwork(CN_GET_NETINFO, (void*)&gWIZNETINFO);

  if (jsvIsObject(options)) {
    _eth_getIP_set_address(options, "ip", &gWIZNETINFO.ip[0]);
    _eth_getIP_set_address(options, "subnet", &gWIZNETINFO.sn[0]);
    _eth_getIP_set_address(options, "gateway", &gWIZNETINFO.gw[0]);
    _eth_getIP_set_address(options, "dns", &gWIZNETINFO.dns[0]);
    gWIZNETINFO.dhcp = NETINFO_STATIC;
    success = true;
  } else {
    // DHCP
    uint8_t DHCPisSuccess = getIP_DHCPS(net_wiznet_getFreeSocket(), &gWIZNETINFO);
    if (DHCPisSuccess == 1) {
      // info in lease_time.lVal
      success = true;
    } else {
      jsWarn("DHCP failed");
      success = false;
    }
  }

  ctlnetwork(CN_SET_NETINFO, (void*)&gWIZNETINFO);
  return success;
}

