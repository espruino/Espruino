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
    if (lEventType == HCI_EVNT_WLAN_ASYNC_SIMPLE_CONFIG_DONE)
    {
        //ulSmartConfigFinished = 1;
        jsiConsolePrint("ulSmartConfigFinished\n");
    }

    if (lEventType == HCI_EVNT_WLAN_UNSOL_CONNECT)
    {
        //ulCC3000Connected = 1;
        /* Turn On LED */
        SmartConfigLedOn(TRUE);
    }

    if (lEventType == HCI_EVNT_WLAN_UNSOL_DISCONNECT)
    {
        //ulCC3000Connected = 0;
        /*  Turn Off LED */
        SmartConfigLedOn(FALSE);
    }
        if (lEventType == HCI_EVNT_WLAN_UNSOL_DHCP)
        {
          //ulCC3000DHCP = 1;
          jsiConsolePrint("ulCC3000DHCP\n");
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
  wlan_init(CC3000_UsynchCallback, (tFWPatches )sendWLFWPatch, (tDriverPatches )sendDriverPatch, (tBootLoaderPatches)sendBootLoaderPatch, ReadWlanInterruptPin, WlanInterruptEnable, WlanInterruptDisable, WriteWlanPin);
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
  wlan_set_event_mask(HCI_EVNT_WLAN_KEEPALIVE|HCI_EVNT_WLAN_UNSOL_INIT|HCI_EVNT_WLAN_UNSOL_DHCP|HCI_EVNT_WLAN_ASYNC_PING_REPORT);
}

/*JSON{ "type":"staticmethod",
         "class" : "WLAN", "name" : "connect",
         "generate" : "jswrap_wlan_connect",
         "description" : "",
         "params" : [  ],
         "return" : ["int", ""]
}*/
JsVarInt jswrap_wlan_connect() {
  const char *ap = "Iles Farm TPLink";
  return wlan_connect(WLAN_SEC_UNSEC, ap, strlen(ap), NULL, NULL, 0);
}
