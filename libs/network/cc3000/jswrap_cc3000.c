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
 * EXTREMELY BETA AND LIKELY TO CHANGE DRASTICALLY
 * ----------------------------------------------------------------------------
 */

#include "jswrap_cc3000.h"
#include "jshardware.h"
#include "jsinteractive.h"
#include "board_spi.h"
// ti driver
#include "wlan.h"
#include "netapp.h"
#include "hci.h"

/*JSON{ "type":"class",
        "class" : "WLAN",
        "description" : ""
}*/

// Bit field containing whether the socket has closed or not
unsigned int cc3000_socket_closed = 0;

/// Check if the cc3000's socket has disconnected (clears flag as soon as is called)
bool cc3000_socket_has_closed(int socketNum) {
  if (cc3000_socket_closed & (1<<socketNum)) {
    cc3000_socket_closed &= ~(1<<socketNum);
    return true;
  } else return false;
}

/**
  * @brief  This function handles asynchronous events that come from CC3000 device
  *         and operates to indicate exchange of data
  * @param  The type of event we just received.
  * @retval None
  */

void cc3000_usynch_callback(long lEventType, char *pcData, unsigned char ucLength)
{
    if (lEventType == HCI_EVNT_WLAN_ASYNC_SIMPLE_CONFIG_DONE) {
      //ulSmartConfigFinished = 1;
      jsiConsolePrint("HCI_EVNT_WLAN_ASYNC_SIMPLE_CONFIG_DONE\n");
    } else if (lEventType == HCI_EVNT_WLAN_UNSOL_CONNECT) {
      jsiConsolePrint("HCI_EVNT_WLAN_UNSOL_CONNECT\n");
      //ulCC3000Connected = 1;
    } else if (lEventType == HCI_EVNT_WLAN_UNSOL_DISCONNECT) {
      jsiConsolePrint("HCI_EVNT_WLAN_UNSOL_DISCONNECT\n");
      //ulCC3000Connected = 0;
    } else if (lEventType == HCI_EVNT_WLAN_UNSOL_DHCP) {
      //ulCC3000DHCP = 1;
      jsiConsolePrint("HCI_EVNT_WLAN_UNSOL_DHCP\n");
    } else if (lEventType == HCI_EVNT_WLAN_ASYNC_PING_REPORT) {
      jsiConsolePrint("HCI_EVNT_WLAN_ASYNC_PING_REPORT\n");
    } else if (lEventType == HCI_EVNT_BSD_TCP_CLOSE_WAIT) {
        uint8_t socketnum = pcData[0];
        cc3000_socket_closed |= 1<<socketnum;
//        jsiConsolePrint("HCI_EVNT_BSD_TCP_CLOSE_WAIT\n");
    } else {
      //jsiConsolePrintHexInt(lEventType);jsiConsolePrint("-usync\n");
    }
}

const unsigned char *sendNoPatch(unsigned long *Length) {
    *Length = 0;
    return NULL;
}

/**
  * @brief  This function returns enables or disables CC3000 .
  * @param  None
  * @retval None
  */
void WriteWlanPin( unsigned char val )
{
  jshPinOutput(WLAN_EN_PIN, val == WLAN_ENABLE);
}


/*JSON{ "type":"staticmethod", 
         "class" : "WLAN", "name" : "init",
         "generate" : "jswrap_wlan_init",
         "description" : "",
         "params" : [ ]
}*/
void jswrap_wlan_init() {
  cc3000_spi_open();
  wlan_init(cc3000_usynch_callback,
            sendNoPatch/*sendWLFWPatch*/,
            sendNoPatch/*sendDriverPatch*/,
            sendNoPatch/*sendBootLoaderPatch*/,
            cc3000_read_irq_pin, cc3000_irq_enable, cc3000_irq_disable, WriteWlanPin);
  wlan_start(0/* No patches */);
  // Mask out all non-required events from CC3000
  wlan_set_event_mask(
      HCI_EVNT_WLAN_KEEPALIVE |
      HCI_EVNT_WLAN_UNSOL_INIT);

  // TODO: check return value !=0
  wlan_ioctl_set_connection_policy(0, 0, 0); // don't auto-connect
  wlan_ioctl_del_profile(255); // delete stored eeprom data
}

/*JSON{ "type":"staticmethod",
         "class" : "WLAN", "name" : "connect",
         "generate" : "jswrap_wlan_connect",
         "description" : "Connect to a wireless network",
         "params" : [ [ "ap", "JsVar", "Access point name" ],
                      [ "key", "JsVar", "WPA2 key (or undefined for unsecured connection)" ],
                      [ "callback", "JsVar", "Function to call back with connection status. It has one argument which is one of 'connected'/'dhcp'" ] ],
         "return" : ["int", ""]
}*/
JsVarInt jswrap_wlan_connect(JsVar *vAP, JsVar *vKey, JsVar *callback) {
  if (!(jsvIsUndefined(callback) || jsvIsFunction(callback))) {
    jsError("Expecting callback function");
    return 0;
  }
  char ap[32];
  char key[32];
  unsigned long security = WLAN_SEC_UNSEC;
  jsvGetString(vAP, ap, sizeof(ap));
  if (jsvIsString(vKey)) {
    security = WLAN_SEC_WPA2;
    jsvGetString(vKey, key, sizeof(key));
  }
  // might want to set wlan_ioctl_set_connection_policy
  return wlan_connect(security, ap, strlen(ap), NULL, key, strlen(key));
}


void _wlan_getIP_get_address(JsVar *object, const char *name,  unsigned char *ip, int nBytes, int base, char separator) {
  char data[64] = "";
  int i, l = 0;
  for (i=nBytes-1;i>=0;i--) {
    itoa(ip[i], &data[l], base);
    l = strlen(data);
    if (i>0 && separator) {
      data[l++] = separator;
      data[l] = 0;
    }
  }

  JsVar *dataVar = jsvNewFromString(data);
  if (!dataVar) return;

  JsVar *v = jsvFindChildFromString(object, name, true);
  if (!v) {
    jsvUnLock(dataVar);
    return; // out of memory
  }
  jsvSetValueOfName(v, dataVar);
  jsvUnLock(dataVar);
  jsvUnLock(v);
}

/*JSON{ "type":"staticmethod",
         "class" : "WLAN", "name" : "getIP",
         "generate" : "jswrap_wlan_getIP",
         "description" : "Get the current IP address",
         "return" : ["JsVar", ""]
}*/
JsVar *jswrap_wlan_getIP() {
  tNetappIpconfigRetArgs ipconfig;
  netapp_ipconfig(&ipconfig);
  /* If byte 1 is 0 we don't have a valid address */
  if (ipconfig.aucIP[3] == 0) return 0;
  JsVar *data = jsvNewWithFlags(JSV_OBJECT);
  _wlan_getIP_get_address(data, "ip", &ipconfig.aucIP, 4, 10, '.');
  _wlan_getIP_get_address(data, "subnet", &ipconfig.aucSubnetMask, 4, 10, '.');
  _wlan_getIP_get_address(data, "gateway", &ipconfig.aucDefaultGateway, 4, 10, '.');
  _wlan_getIP_get_address(data, "dhcp", &ipconfig.aucDHCPServer, 4, 10, '.');
  _wlan_getIP_get_address(data, "dns", &ipconfig.aucDNSServer, 4, 10, '.');
  _wlan_getIP_get_address(data, "mac", &ipconfig.uaMacAddr, 6, 16, 0);
  return data;
}
