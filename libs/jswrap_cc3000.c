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


/**
  * @brief  This function turns the connection LED ON/OFF
  * @param  None
  * @retval None
  */
void SmartConfigLedOn(uint32_t ulTrueFalse)
{
  jshPinOutput(LED1_PININDEX, ulTrueFalse);
}

/**
  * @brief  This function handles asynchronous events that come from CC3000 device
  *         and operates to indicate exchange of data
  * @param  The type of event we just received.
  * @retval None
  */

void CC3000_UsynchCallback(long lEventType, char *pcData, unsigned char ucLength)
{
    if (lEventType == HCI_EVNT_WLAN_ASYNC_SIMPLE_CONFIG_DONE) {
      //ulSmartConfigFinished = 1;
      jsiConsolePrint("HCI_EVNT_WLAN_ASYNC_SIMPLE_CONFIG_DONE\n");
    }

    if (lEventType == HCI_EVNT_WLAN_UNSOL_CONNECT) {
      jsiConsolePrint("HCI_EVNT_WLAN_UNSOL_CONNECT\n");
      //ulCC3000Connected = 1;
      /* Turn On LED */
      SmartConfigLedOn(TRUE);
    }

    if (lEventType == HCI_EVNT_WLAN_UNSOL_DISCONNECT) {
      jsiConsolePrint("HCI_EVNT_WLAN_UNSOL_DISCONNECT\n");
      //ulCC3000Connected = 0;
      /*  Turn Off LED */
      SmartConfigLedOn(FALSE);
    }
    if (lEventType == HCI_EVNT_WLAN_UNSOL_DHCP) {
      //ulCC3000DHCP = 1;
      jsiConsolePrint("HCI_EVNT_WLAN_UNSOL_DHCP\n");
    }
    if (lEventType == HCI_EVNT_WLAN_ASYNC_PING_REPORT) {
      jsiConsolePrint("HCI_EVNT_WLAN_ASYNC_PING_REPORT\n");
    }
}

/**
  * @brief  This function returns a pointer to the driver patch.
  * @param  The length of the patch.
  * @retval None
  */
const unsigned char *sendDriverPatch(unsigned long *Length)
{
    *Length = 0;
    return NULL;
}


/**
  * @brief  This function returns a pointer to the bootloader patch.
  * @param  The length of the patch.
  * @retval None
  */
const unsigned char  *sendBootLoaderPatch(unsigned long *Length)
{
    *Length = 0;
    return NULL;
}


/**
  * @brief  This function returns a pointer to the firmware patch.
  * @param  The length of the patch.
  * @retval None
  */
const unsigned char *sendWLFWPatch(unsigned long *Length)
{
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
         "params" : [  ]
}*/
void jswrap_wlan_init() {
  SpiInit();
  wlan_init(CC3000_UsynchCallback, sendWLFWPatch, sendDriverPatch, sendBootLoaderPatch, ReadWlanInterruptPin, WlanInterruptEnable, WlanInterruptDisable, WriteWlanPin);
}

/*JSON{ "type":"staticmethod",
         "class" : "WLAN", "name" : "start",
         "generate" : "jswrap_wlan_start",
         "description" : "",
         "params" : [  ]
}*/
void jswrap_wlan_start() {
  wlan_start(0);
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
                      [ "key", "JsVar", "WPA2 key (or undefined for unsecured connection)" ] ],
         "return" : ["int", ""]
}*/
JsVarInt jswrap_wlan_connect(JsVar *vAP, JsVar *vKey) {
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
  // unsigned char uaSSID[32];

  return data;
}
