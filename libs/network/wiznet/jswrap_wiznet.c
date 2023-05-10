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
 * Contains built-in functions for WIZnet Ethernet Access
 * ----------------------------------------------------------------------------
 */

#include "jswrap_wiznet.h"
#include "jshardware.h"
#include "jsinteractive.h"
#include "network.h"
// wiznet driver
#include "wizchip_conf.h"

#include "network_wiznet.h"
#include "DHCP/dhcp.h"

// -------------------- defaults...
#ifdef STM32
#define ETH_SPI          EV_SPI3
#define ETH_CS_PIN       (Pin)(JSH_PORTB_OFFSET + 2) // active low
#define ETH_CLK_PIN      (Pin)(JSH_PORTB_OFFSET + 3)
#define ETH_MISO_PIN     (Pin)(JSH_PORTB_OFFSET + 4)
#define ETH_MOSI_PIN     (Pin)(JSH_PORTB_OFFSET + 5)
#endif
// -------------------------------

static char *state[] = { "offline", "connected", "online", "disconnect" };
static char *phylink[] = {"down","up"};
static char *phypowm[] = {"down","normal"};

void  wizchip_select(void) {
  assert(networkGetCurrent());
  jshPinOutput(networkGetCurrent()->data.pinCS, 0); // active low
}

void  wizchip_deselect(void) {
  assert(networkGetCurrent());
  jshPinOutput(networkGetCurrent()->data.pinCS, 1); // active low
}

static uint8_t wizchip_rw(uint8_t data) {
  assert(networkGetCurrent());
  int r = jshSPISend(networkGetCurrent()->data.device, data);
  if (r<0) r = jshSPISend(networkGetCurrent()->data.device, -1);
  return (uint8_t)r;
}

void  wizchip_write(uint8_t wb) {
  wizchip_rw(wb);
}

uint8_t wizchip_read() {
   return wizchip_rw(0xFF);
}


/*JSON{
  "type" : "library",
  "class" : "WIZnet",
  "ifdef" : "USE_WIZNET"
}
Library for communication with the WIZnet Ethernet module
*/
/*JSON{
  "type" : "staticmethod",
  "class" : "WIZnet",
  "ifdef" : "USE_WIZNET",
  "name" : "connect",
  "generate" : "jswrap_wiznet_connect",
  "params" : [
    ["spi", "JsVar", "Device to use for SPI (or undefined to use the default)"],
    ["cs", "pin", "The pin to use for Chip Select"]
  ],
  "return" : ["JsVar","An Ethernet Object"],
  "return_object" : "Ethernet"
}
Initialise the WIZnet module and return an Ethernet object
*/
JsVar *jswrap_wiznet_connect(JsVar *spi, Pin cs) {

  IOEventFlags spiDevice;
  if (spi) {
    spiDevice = jsiGetDeviceFromClass(spi);
    if (!DEVICE_IS_SPI(spiDevice)) {
      jsExceptionHere(JSET_ERROR, "Expecting SPI device, got %q", spi);
      return 0;
    }
  } else {
#ifdef ETH_SPI
    // SPI config
    JshSPIInfo inf;
    jshSPIInitInfo(&inf);
    inf.pinSCK =  ETH_CLK_PIN;
    inf.pinMISO = ETH_MISO_PIN;
    inf.pinMOSI = ETH_MOSI_PIN;
    inf.baudRate = 1000000;
    inf.spiMode = SPIF_SPI_MODE_0;
    jshSPISetup(ETH_SPI, &inf);
    spiDevice = ETH_SPI;
#else
    jsExceptionHere(JSET_ERROR, "No default SPI on this platform - you must specify one.");
    return 0;
#endif
  }
#ifdef ETH_CS_PIN
  if (!jshIsPinValid(cs))
    cs = ETH_CS_PIN;
#endif

  JsNetwork net;
  networkCreate(&net, JSNETWORKTYPE_W5500);
  net.data.device = spiDevice;
  net.data.pinCS = cs;
  networkSet(&net);

  JsVar *ethObj = jspNewObject(0, "Ethernet");

  // CS Configuration
  jshSetPinStateIsManual(net.data.pinCS, false);
  jshPinOutput(net.data.pinCS, 1); // de-assert CS

  // Initialise WIZnet functions
  reg_wizchip_cs_cbfunc(wizchip_select, wizchip_deselect);
  reg_wizchip_spi_cbfunc(wizchip_read, wizchip_write);

  /* wizchip initialize*/
  uint8_t tmp;
  uint8_t memsize[2][8] = { {2,2,2,2,2,2,2,2}, {2,2,2,2,2,2,2,2}};

  if(ctlwizchip(CW_INIT_WIZCHIP,(void*)memsize) == -1)
  {
    jsiConsolePrint("WIZnet Initialize failed.\r\n");
    networkFree(&net);
    return 0;
  }

#if _WIZCHIP_ == 5500
  /* PHY link status check - W5100 doesn't have this */
  do {
    if(ctlwizchip(CW_GET_PHYLINK, (void*)&tmp) == -1) {
      jsiConsolePrint("Unknown PHY Link status.\r\n");
      networkFree(&net);
      return 0;
    }
  } while (tmp == PHY_LINK_OFF);
#endif

  networkFree(&net);

  networkState = NETWORKSTATE_ONLINE;

  return ethObj;
}

/*JSON{
  "type" : "class",
  "class" : "Ethernet",
  "ifdef" : "USE_WIZNET"
}
An instantiation of an Ethernet network adaptor
*/

/*JSON{
  "type" : "method",
  "class" : "Ethernet",
  "ifdef" : "USE_WIZNET",
  "name" : "getIP",
  "generate" : "jswrap_ethernet_getIP",
  "params" : [
    ["options","JsVar","[optional] An `callback(err, ipinfo)` function to be called back with the IP information."]
  ],
  "return" : ["JsVar",""]
}
Get the current IP address, subnet, gateway and mac address.
*/
JsVar *jswrap_ethernet_getIP(JsVar *wlanObj, JsVar *callback) {
  NOT_USED(wlanObj);

  if (networkState != NETWORKSTATE_ONLINE) {
    jsExceptionHere(JSET_ERROR, "Not connected to the internet");
    return 0;
  }

  JsNetwork net;
  if (!networkGetFromVar(&net)) return 0;

  wiz_NetInfo gWIZNETINFO;
  ctlnetwork(CN_GET_NETINFO, (void*)&gWIZNETINFO);

  /* If byte 1 is 0 we don't have a valid address */
  JsVar *data = jsvNewObject();
  networkPutAddressAsString(data, "ip", &gWIZNETINFO.ip[0], 4, 10, '.');
  networkPutAddressAsString(data, "subnet", &gWIZNETINFO.sn[0], 4, 10, '.');
  networkPutAddressAsString(data, "gateway", &gWIZNETINFO.gw[0], 4, 10, '.');
  if ( !gWIZNETINFO.gw[0] ) {
    gWIZNETINFO.dns[0] = gWIZNETINFO.dns[1] = gWIZNETINFO.dns[2] = gWIZNETINFO.dns[3] = 0;
  }
  networkPutAddressAsString(data, "dns", &gWIZNETINFO.dns[0], 4, 10, '.');
  networkPutAddressAsString(data, "mac", &gWIZNETINFO.mac[0], 6, 16, ':');

  networkFree(&net);

  // Schedule callback if a function was provided
  if (jsvIsFunction(callback)) {
    JsVar *params[2];
    params[0] = jsvNewWithFlags(JSV_NULL);
    params[1] = data;
    jsiQueueEvents(NULL, callback, params, 2);
    jsvUnLock(params[0]);
  }

  return data;
}


static void _eth_getIP_set_address(JsVar *options, char *name, unsigned char *ptr) {
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
  "class" : "Ethernet",
  "ifdef" : "USE_WIZNET",
  "name" : "setIP",
  "generate" : "jswrap_ethernet_setIP",
  "params" : [
    ["options","JsVar","Object containing IP address options `{ ip : '1.2.3.4', subnet : '...', gateway: '...', dns:'...', mac:':::::'  }`, or do not supply an object in order to force DHCP."],
    ["callback","JsVar","[optional] An `callback(err)` function to invoke when ip is set. `err==null` on success, or a string on failure."]
  ],
  "return" : ["bool","True on success"]
}
Set the current IP address or get an IP from DHCP (if no options object is
specified)

If 'mac' is specified as an option, it must be a string of the form
`"00:01:02:03:04:05"` The default mac is 00:08:DC:01:02:03.
*/
bool jswrap_ethernet_setIP(JsVar *wlanObj, JsVar *options, JsVar *callback) {
  NOT_USED(wlanObj);

  if (networkState != NETWORKSTATE_ONLINE) {
    jsExceptionHere(JSET_ERROR, "Not connected to the internet");
    return false;
  }

  JsNetwork net;
  if (!networkGetFromVar(&net)) return false;

  const char *errorMessage = 0;
  wiz_NetInfo gWIZNETINFO;

  ctlnetwork(CN_GET_NETINFO, (void*)&gWIZNETINFO);
  if (!gWIZNETINFO.mac[0] && !gWIZNETINFO.mac[1] &&
      !gWIZNETINFO.mac[2] && !gWIZNETINFO.mac[3] &&
      !gWIZNETINFO.mac[4] && !gWIZNETINFO.mac[5]) {
    // wow - no mac address - WIZ550BoB? Set up a simple one
    // in WIZnet's range of addresses
    gWIZNETINFO.mac[0]=0x00;
    gWIZNETINFO.mac[1]=0x08;
    gWIZNETINFO.mac[2]=0xDC;
    gWIZNETINFO.mac[3]=0x01;
    gWIZNETINFO.mac[4]=0x02;
    gWIZNETINFO.mac[5]=0x03;
  }

  if (jsvIsObject(options)) {
    _eth_getIP_set_address(options, "ip", &gWIZNETINFO.ip[0]);
    _eth_getIP_set_address(options, "subnet", &gWIZNETINFO.sn[0]);
    _eth_getIP_set_address(options, "gateway", &gWIZNETINFO.gw[0]);
    _eth_getIP_set_address(options, "dns", &gWIZNETINFO.dns[0]);

    JsVar *info = jsvObjectGetChildIfExists(options, "mac");
    if (info) {
      char buf[64];
      jsvGetString(info, buf, sizeof(buf));
      networkParseMACAddress(&gWIZNETINFO.mac[0], buf);
      // TODO: check failure?
      jsvUnLock(info);
    }

    gWIZNETINFO.dhcp = NETINFO_STATIC;
    errorMessage = 0; // all ok
  } else {
    // DHCP
    uint8_t DHCPisSuccess = getIP_DHCPS(net_wiznet_getFreeSocket(), &gWIZNETINFO);
    if (DHCPisSuccess == 1) {
      // info in lease_time.lVal
      errorMessage = 0; // all ok
    } else {
      errorMessage = "DHCP failed";
      jsWarn(errorMessage);
    }
  }

  ctlnetwork(CN_SET_NETINFO, (void*)&gWIZNETINFO);

  networkFree(&net);

  // Schedule callback if a function was provided
  if (jsvIsFunction(callback)) {
    JsVar *params[1];
    params[0] = errorMessage ? jsvNewFromString(errorMessage) : jsvNewWithFlags(JSV_NULL);
    jsiQueueEvents(NULL, callback, params, 1);
    jsvUnLock(params[0]);
  }
  return errorMessage==0;
}

/*JSON{
  "type" : "method",
  "class" : "Ethernet",
  "ifdef" : "USE_WIZNET",
  "name" : "setHostname",
  "generate" : "jswrap_ethernet_setHostname",
  "params" : [
    ["hostname","JsVar","hostname as string"],
    ["callback","JsVar","[optional] An `callback(err)` function to be called back with null or error text."]
  ],
  "return" : ["bool","True on success"]
}
Set hostname used during the DHCP request. Minimum 8 and maximum 12 characters,
best set before calling `eth.setIP()`. Default is WIZnet010203, 010203 is the
default nic as part of the mac.
*/
bool jswrap_ethernet_setHostname(JsVar *wlanObj, JsVar *jsHostname, JsVar *callback){
  NOT_USED(wlanObj);
  char hostname[13];
  const char *errorMessage = 0;

  jsvGetString(jsHostname, hostname, sizeof(hostname));

  if (strlen(hostname) < 8) {
    errorMessage = "hostname too short (min 8 and max 12 char )";
  }
  else {
    setHostname(hostname);
  }

  // Schedule callback if a function was provided
  if (jsvIsFunction(callback)) {
    JsVar *params[1];
    params[0] = errorMessage ? jsvNewFromString(errorMessage) : jsvNewWithFlags(JSV_NULL);
    jsiQueueEvents(NULL, callback, params, 1);
    jsvUnLock(params[0]);
  }
  return errorMessage==0;
}

/*JSON{
  "type" : "method",
  "class" : "Ethernet",
  "ifdef" : "USE_WIZNET",
  "name" : "getHostname",
  "generate" : "jswrap_ethernet_getHostname",
  "params" : [
    ["callback","JsVar","[optional] An `callback(err,hostname)` function to be called back with the status information."]
  ],
  "return" : ["JsVar" ]
}
Returns the hostname
*/
JsVar * jswrap_ethernet_getHostname(JsVar *wlanObj, JsVar *callback) {
  NOT_USED(wlanObj);
  const char *errorMessage = 0;
  char *hostname = getHostname();

  // Schedule callback if a function was provided
  if (jsvIsFunction(callback)) {
    JsVar *params[2];
    params[0] = errorMessage ? jsvNewFromString(errorMessage) : jsvNewWithFlags(JSV_NULL);
    params[1] = jsvNewFromString(hostname);
    jsiQueueEvents(NULL, callback, params, 2);
    jsvUnLock(params[0]);
  }
  return jsvNewFromString(hostname);
}

/*JSON{
  "type" : "method",
  "class" : "Ethernet",
  "ifdef" : "USE_WIZNET",
  "name" : "getStatus",
  "generate" : "jswrap_ethernet_getStatus",
  "params" : [
    ["options","JsVar","[optional] An `callback(err, status)` function to be called back with the status information."]
  ],
  "return" : ["JsVar" ]
}
Get the current status of the ethernet device

*/
JsVar * jswrap_ethernet_getStatus( JsVar *wlanObj, JsVar *callback) {
  NOT_USED(wlanObj);

  uint8_t tmp;
  uint8_t tmpstr[6] = {0,};

  JsNetwork net;
  if (!networkGetFromVar(&net)) return 0;

  JsVar *jsStatus = jsvNewObject();

  jsvObjectSetChildAndUnLock(jsStatus, "state", jsvNewFromString(state[networkState]));

  ctlwizchip(CW_GET_ID,(void*)tmpstr);  
  jsvObjectSetChildAndUnLock(jsStatus, "id",jsvNewFromString(tmpstr));

  ctlwizchip(CW_GET_PHYLINK, (void*)&tmp);
  jsvObjectSetChildAndUnLock(jsStatus, "phylink", jsvNewFromString(phylink[tmp]));
  
  ctlwizchip(CW_GET_PHYPOWMODE,(void*)&tmp);
  jsvObjectSetChildAndUnLock(jsStatus, "phypowmode", jsvNewFromString(phypowm[tmp]));

#if  _WIZCHIP_ == 5500 
  wiz_PhyConf tmpPhycConf;
  ctlwizchip(CW_GET_PHYCONF,(void*)&tmpPhycConf);
  jsvObjectSetChildAndUnLock(jsStatus, "by", jsvNewFromInteger(tmpPhycConf.by));
  jsvObjectSetChildAndUnLock(jsStatus, "mode", jsvNewFromInteger(tmpPhycConf.mode));
  jsvObjectSetChildAndUnLock(jsStatus, "speed",  tmpPhycConf.speed ? jsvNewFromInteger(10) :  jsvNewFromInteger(100));
  jsvObjectSetChildAndUnLock(jsStatus, "duplex", jsvNewFromInteger(tmpPhycConf.duplex));
#endif

  networkFree(&net);

  // Schedule callback if a function was provided
  if (jsvIsFunction(callback)) {
    JsVar *params[2];
    params[0] = jsvNewWithFlags(JSV_NULL);
    params[1] = jsStatus;
    jsiQueueEvents(NULL, callback, params, 2);
    jsvUnLock(params[0]);
  } 
  return jsStatus;
}
