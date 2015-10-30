/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2015 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * This file is designed to be parsed during the build process
 *
 * Contains ESP8266 board specific functions.
 * ----------------------------------------------------------------------------
 */

/* DO_NOT_INCLUDE_IN_DOCS - this is a special token for common.py,
so we don't put this into espruino.com/Reference until this is out
of beta.  */

// Because the ESP8266 JS wrapper is assured to be running on an ESP8266 we
// can assume that inclusion of ESP8266 headers will be acceptable.
#include <c_types.h>
#include <user_interface.h>
#include <mem.h>
#include <osapi.h>
#include <ping.h>
#include <espconn.h>
#include <espmissingincludes.h>
#include <uart.h>

#define ESP8266_ON_ACCESS_POINTS "#accessPoints"

#define _GCC_WRAP_STDINT_H
typedef long long int64_t;

#include "jswrap_esp8266.h"
#include "jsinteractive.h" // Pull inn the jsiConsolePrint function
#include "network.h"
#include "network_esp8266.h"
#include "jswrap_net.h"

// Forward declaration of functions.
static void   scanCB(void *arg, STATUS status);
static void   wifiEventHandler(System_Event_t *event);
static void   ipAddrToString(struct ip_addr addr, char *string);
static char  *nullTerminateString(char *target, char *source, int sourceLength);
static void   setupJsNetwork();
static void   pingRecvCB();
static char  *wifiConnectStatusToString(uint8 status);
static JsVar *createErrorVar(int errorCode, char *errorMessage);


// #NOTE: For callback functions, be sure and unlock them in the `kill` handler.

// A callback function to be invoked when we find a new access point.
static JsVar *g_jsScanCallback;

// A callback function to be invoked when we receive a WiFi event.
static JsVar *g_jsWiFiEventCallback;

// A callback function to be invoked when we have an IP address.
static JsVar *g_jsGotIpCallback;

// A callback function to be invoked on ping responses.
static JsVar *g_jsPingCallback;

// A callback function to be invoked on gethostbyname responses.
static JsVar *g_jsHostByNameCallback;

// A callback function to be invoked on a disconnect response.
static JsVar *g_jsDisconnectCallback;

// A callback function to be invoked on a createAP response.
static JsVar *g_jsCreateAPCallback;

// A callback function to be invoked on a stopAP response.
static JsVar *g_jsStopAPCallback;

// Global data structure for ping request
static struct ping_option pingOpt;

// Value of WiFi mode pre-scan so that it can be restored.
static uint8 g_preWiFiScanMode;

// Reasons for which a connection failed
static char *wifiReasons[] = {
  "0 - <Not Used>",           // 0
  "unspecified",              // 1 - REASON_UNSPECIFIED
  "auth_expire",              // 2 - REASON_AUTH_EXPIRE
  "auth_leave",               // 3 - REASON_AUTH_LEAVE
  "assoc_expire",             // 4 - REASON_ASSOC_EXPIRE
  "assoc_toomany",            // 5 - REASON_ASSOC_TOOMANY
  "not_authed",               // 6 - REASON_NOT_AUTHED
  "not_assoced",              // 7 - REASON_NOT_ASSOCED
  "assoc_leave",              // 8 - REASON_ASSOC_LEAVE
  "assoc_not_authed",         // 9 - REASON_ASSOC_NOT_AUTHED
  "disassoc_pwrcap_bad",      // 10 - REASON_DISASSOC_PWRCAP_BAD
  "disassoc_supchan_bad",     // 11 - REASON_DISASSOC_SUPCHAN_BAD
  "12 - <Not Used>",          // 12
  "ie_invalid",               // 13 - REASON_IE_INVALID
  "mic_failure",              // 14 - REASON_MIC_FAILURE
  "4way_handshake_timeout",   // 15 - REASON_4WAY_HANDSHAKE_TIMEOUT
  "group_key_update_timeout", // 16 - REASON_GROUP_KEY_UPDATE_TIMEOUT
  "ie_in_4way_differs",       // 17 - REASON_IE_IN_4WAY_DIFFERS
  "group_cipher_invalid",     // 18 - REASON_GROUP_CIPHER_INVALID
  "pairwise_cipher_invalid",  // 19 - REASON_PAIRWISE_CIPHER_INVALID
  "akmp_invalid",             // 20 - REASON_AKMP_INVALID
  "unsupp_rsn_ie_version",    // 21 - REASON_UNSUPP_RSN_IE_VERSION
  "invalid_rsn_ie_cap",       // 22 - REASON_UNSUPP_RSN_IE_VERSION
  "802_1x_auth_failed",       // 23 - REASON_802_1X_AUTH_FAILED
  "cipher_suite_rejected",    // 24 - REASON_CIPHER_SUITE_REJECTED
  "beacon_timeout",           // 200 - REASON_BEACON_TIMEOUT
  "no_ap_found"               // 201 - REASON_NO_AP_FOUND
};

static char *wifiGetReason(uint8 wifiReason) {
  if (wifiReason <= 24) return wifiReasons[wifiReason];
  if (wifiReason >= 200 && wifiReason <= 201) return wifiReasons[wifiReason-200+24];
  return wifiReasons[1];
}

static char *wifiMode[] = { 0, "STA", "AP", "AP+STA" };
static char *wifiPhy[]  = { 0, "11b", "11g", "11n" };


/*JSON{
   "type": "library",
   "class": "ESP8266"
}*/

/*JSON{
  "type"     : "staticmethod",
  "class"    : "ESP8266",
  "name"     : "getAddressAsString",
  "generate" : "jswrap_ESP8266_getAddressAsString",
  "params"   : [
    ["address", "JsVar", "An integer value representing an IP address."]
  ],
  "return"   : ["JsVar", "A String"]
}*/

JsVar *jswrap_ESP8266_getAddressAsString(
    JsVar *address //!< An integer value representing an IP address.
  ) {
  if (!jsvIsInt(address)) {
    jsExceptionHere(JSET_ERROR, "No address.");
    return NULL;
  }
  uint32 iAddress = (uint32)jsvGetInteger(address);
  return networkGetAddressAsString((uint8 *)&iAddress, 4, 10, '.');
}

/*JSON{
   "type": "library",
   "class": "wifi"
}*/

/*JSON{
  "type"     : "staticmethod",
  "class"    : "wifi",
  "name"     : "disconnect",
  "generate" : "jswrap_ESP8266_wifi_disconnect",
  "params"   : [
    ["options", "JsVar", "Options controlling the function (optional)."],
    ["callback", "JsVar", "A function to be called back on completion (optional)."]
  ]
}
* The options is a JavaScript object that contains the following properties:
* * `default` - If set to true, then auto connect of this device as a station will be
* disabled.  The default of `default` is false.
*/
void jswrap_ESP8266_wifi_disconnect(JsVar *jsOptions, JsVar *jsCallback) {
  os_printf("> jswrap_ESP8266_wifi_disconnect\n");
  if (g_jsDisconnectCallback != NULL) {
    jsvUnLock(g_jsDisconnectCallback);
    g_jsDisconnectCallback = NULL;
  }
  g_jsDisconnectCallback = jsvLockAgainSafe(jsCallback);

  bool setDefault = false;
  if (jsOptions != NULL && jsvIsObject(jsOptions)) {
    JsVar *jsDefault = jsvObjectGetChild(jsOptions, "default", 0);
    if (jsvIsBoolean(jsDefault)) {
      if(jsvGetBool(jsDefault)) {
        wifi_station_set_auto_connect(0);
      }
    }
    jsvUnLock(jsDefault);
  }

  bool rc = wifi_station_disconnect();
  JsVar *params[1];

  if (g_jsDisconnectCallback != NULL) {
    // Set the return error as a function of the return code returned from the call to
    // the ESP8266 API to disconnect from the access point.
    if (rc == true) {
      params[0] = jsvNewNull();
    } else {
      params[0] = createErrorVar(-1, "Error");
    }

    jsiQueueEvents(NULL, g_jsDisconnectCallback, params, 1);
  }

  os_printf("< jswrap_ESP8266_wifi_disconnect\n");
}

/*JSON{
  "type"     : "staticmethod",
  "class"    : "wifi",
  "name"     : "stopAP",
  "generate" : "jswrap_ESP8266_wifi_stopAP",
  "params"   : [
    ["options", "JsVar", "Options controlling the stop of an access point."],
    ["callback", "JsVar", "A function to be called back on completion (optional)."]
  ]
}
* Stop being an access point.
* The options contain:
* * default - A boolean.  If set to true, then the access point will be stopped at boot time.
* The default for `default` is false which means that the start/stop of an access point at boot time
* will not be changed.
*/
void jswrap_ESP8266_wifi_stopAP(JsVar *jsOptions, JsVar *jsCallback) {
  os_printf("> jswrap_ESP8266_wifi_stopAP\n");

  if (g_jsStopAPCallback != NULL) {
    jsvUnLock(g_jsStopAPCallback);
  }
  g_jsStopAPCallback = jsvLockAgainSafe(jsCallback);

  uint8 currentMode = wifi_get_opmode();

  // Current         Resulting
  // --------------  --------------
  // NULL_MODE       NULL_MODE
  // STATION_MODE    STATION_MODE
  // SOFTAP_MODE     NULL_MODE
  // STATIONAP_MODE  STATION_MODE

  bool setDefault = false;
  if (jsOptions != NULL && jsvIsObject(jsOptions)) {
    JsVar *jsDefault = jsvObjectGetChild(jsOptions, "default", 0);
    if (jsvIsBoolean(jsDefault)) {
      setDefault = jsvGetBool(jsDefault);
    }
    jsvUnLock(jsDefault);
  }

  if (currentMode == STATION_MODE || currentMode == STATIONAP_MODE) {
    if (setDefault == false) {
      wifi_set_opmode_current(STATION_MODE);
    } else {
      wifi_set_opmode(STATION_MODE);
    }
  } else {
    if (setDefault == false) {
      wifi_set_opmode_current(NULL_MODE);
    } else {
      wifi_set_opmode(NULL_MODE);
    }
  }


  // No real return code to check, so hard code to be true.
  bool rc = true;

  if (g_jsStopAPCallback != NULL) {
    JsVar *params[1];
    // Set the return error as a function of the return code returned from the call to
    // the ESP8266 API to disconnect from the access point.
    if (rc == true) {
      params[0] = jsvNewNull();
    } else {
      params[0] = createErrorVar(-1, "Error");
    }
    jsiQueueEvents(NULL, g_jsStopAPCallback, params, 1);
  }

  os_printf("< jswrap_ESP8266_wifi_stopAP\n");
}

/*JSON{
  "type"     : "staticmethod",
  "class"    : "wifi",
  "name"     : "connect",
  "generate" : "jswrap_ESP8266_wifi_connect",
  "params"   : [
    ["ssid", "JsVar", "The access point network id."],
    ["password", "JsVar", "The password to be used to access the network (optional)."],
    ["options", "JsVar", "Connection options (optional)."],
    ["callback", "JsVar", "A function to be called back on completion (optional)."]
  ]
}
* Connect to an access point as a station.
* When the outcome is known, the callback function is invoked.  The options object contains
* properties that control the connection.  Included in this object are:
* * default (Boolean) - When true, these settings will be used to connect on next boot.
* * dnsServers (array of String) - An array of up to two DNS servers in dotted decimal format string.
*/
void jswrap_ESP8266_wifi_connect(
    JsVar *jsSsid,
    JsVar *jsPassword,
    JsVar *jsOptions,
    JsVar *jsCallback
  ) {

  // Notes:
  // The callback function is saved in the file local variable called g_jsGotIpCallback.  The
  // callback will be made when the WiFi callback found in the function called wifiEventHandler.

  os_printf("> jswrap_ESP8266_wifi_connect() Called!\n");

 // Check that the ssid and password values aren't obviously in error.
 if (jsSsid == NULL || !jsvIsString(jsSsid)) {
   jsExceptionHere(JSET_ERROR, "No SSID.");
   return;
 }

 if (jsPassword != NULL && !jsvIsString(jsPassword)) {
   jsExceptionHere(JSET_ERROR, "Invalid password");
   return;
 }

 // Check that if a callback function was supplied that we actually have a callback function.
 if (jsCallback != NULL && !jsvIsUndefined(jsCallback) && !jsvIsFunction(jsCallback)) {
   jsCallback = NULL;
   jsExceptionHere(JSET_ERROR, "A callback function was supplied that is not a function.");
   return;
 }
 if (jsvIsUndefined(jsCallback) || jsvIsNull(jsCallback)) {
   jsCallback = NULL;
 }

 // Set the global which is the gotIP callback to null but first unlock it.
 if (g_jsGotIpCallback != NULL) {
   jsvUnLock(g_jsGotIpCallback);
   g_jsGotIpCallback = NULL;
 }

 // If we have a callback, save it for later invocation.
 if (jsCallback != NULL) {
   g_jsGotIpCallback = jsvLockAgainSafe(jsCallback);
 }

 // Debug
 // os_printf("jsCallback=%p\n", jsCallback);

 // Create strings from the JsVars for the ESP8266 API calls.
 char ssid[33];
 int len = jsvGetString(jsSsid, ssid, sizeof(ssid)-1);
 ssid[len]='\0';

 char password[65];
 if (jsPassword != NULL) {
   len = jsvGetString(jsPassword, password, sizeof(password)-1);
   password[len]='\0';
 } else {
   password[0] = '\0';
 }

 os_printf(">  - ssid=%s, password=\"%s\"\n", ssid, password);



 struct station_config stationConfig;
 memset(&stationConfig, 0, sizeof(stationConfig));
 os_strncpy((char *)stationConfig.ssid, ssid, 32);
 os_strncpy((char *)stationConfig.password, password, 64);

 // Set the WiFi configuration
 wifi_station_set_config_current(&stationConfig);

 uint8 wifiConnectStatus = wifi_station_get_connect_status();
 os_printf(" - Current connect status: %s\n", wifiConnectStatusToString(wifiConnectStatus));

 if (wifiConnectStatus == STATION_GOT_IP) {
   // See issue #618.  There are currently three schools of thought on what should happen
   // when a connect is issued and we are already connected.
   //
   // Option #1 - Always perform a disconnect.
   // Option #2 - Perform a disconnect if the SSID or PASSWORD are different from current
   // Option #3 - Fail the connect if we are already connected.
   //
#define ISSUE_618 1

#if ISSUE_618 == 1
   wifi_station_disconnect();
#elif ISSUE_618 == 2
   struct station_config existingConfig;
   wifi_station_get_config(&existingConfig);
   if (os_strncmp((char *)existingConfig.ssid, (char *)stationConfig.ssid, 32) == 0 &&
       os_strncmp((char *)existingConfig.password, (char *)stationConfig.password, 64) == 0) {
     if (jsGotIpCallback != NULL) {
       JsVar *params[2];
       params[0] = jsvNewFromInteger(STATION_GOT_IP);
       params[1] = jsvNewNull();
       jsiQueueEvents(NULL, jsCallback, params, 2);
     }
     return;

   } else {
     wifi_station_disconnect();
   }
#elif ISSUE_618 == 3
   // Add a return code to the function and return an already connected error.
#endif
  }
  bool isDefault = false;
  if (jsOptions != NULL && jsvIsObject(jsOptions)) {
    // Do we have a child property called autoConnect?
    JsVar *jsDefault = jsvObjectGetChild(jsOptions, "default", 0);
    if (jsvIsBoolean(jsDefault)) {
      isDefault = jsvGetBool(jsDefault);
      if (isDefault) {
        // autoConnect == true
        os_printf(" - Setting auto connect to true\n");
        wifi_station_set_auto_connect(true);
        // Set the WiFi configuration (includes setting default)
        wifi_station_set_config(&stationConfig);
      }
   }
   jsvUnLock(jsDefault);
   // End of default processing

   // Do we have a child property called dnsServers?
   JsVar *jsDNSServers = jsvObjectGetChild(jsOptions, "dnsServers", 0);
   if (jsvIsArray(jsDNSServers) != false) {
     os_printf(" - We have DNS servers!!\n");
     JsVarInt numDNSServers = jsvGetArrayLength(jsDNSServers);
     ip_addr_t dnsAddresses[2];
     int count;
     if (numDNSServers == 0) {
       os_printf("No servers!!");
       count = 0;
     }
     if (numDNSServers > 0) {
       // One server
       count = 1;
       JsVar *jsCurrentDNSServer = jsvGetArrayItem(jsDNSServers, 0);
       char buffer[50];
       size_t size = jsvGetString(jsCurrentDNSServer, buffer, sizeof(buffer)-1);
       buffer[size] = '\0';
       jsvUnLock(jsCurrentDNSServer);
       dnsAddresses[0].addr = networkParseIPAddress(buffer);
     }
     if (numDNSServers > 1) {
       // Two servers
       count = 2;
       JsVar *jsCurrentDNSServer = jsvGetArrayItem(jsDNSServers, 1);
       char buffer[50];
       size_t size = jsvGetString(jsCurrentDNSServer, buffer, sizeof(buffer)-1);
       buffer[size] = '\0';
       jsvUnLock(jsCurrentDNSServer);
       dnsAddresses[1].addr = networkParseIPAddress(buffer);
     }
     if (numDNSServers > 2) {
       os_printf("Ignoring DNS servers after first 2.");
     }
     if (count > 0) {
       espconn_dns_setserver((char)count, dnsAddresses);
     }
   }
   jsvUnLock(jsDNSServers);
 } // End of options processing

 // Set the WiFi mode of the ESP8266

  // Current         Resulting
  // --------------  --------------
  // NULL_MODE       STATION_MODE
  // STATION_MODE    STATION_MODE
  // SOFTAP_MODE     STATIONAP_MODE
  // STATIONAP_MODE  STATIONAP_MODE

  uint8 currentMode = wifi_get_opmode();
  if (currentMode == SOFTAP_MODE || currentMode == STATIONAP_MODE) {
    if(isDefault) {
      wifi_set_opmode(STATIONAP_MODE);
    } else {
      wifi_set_opmode_current(STATIONAP_MODE);
    }
  } else {
    if (isDefault) {
      wifi_set_opmode(STATION_MODE);
    } else {
      wifi_set_opmode_current(STATION_MODE);
    }
  }

 // Perform the network level connection.
 wifi_station_connect();
 os_printf("< jswrap_ESP8266_wifi_connect\n");
}

/*JSON{
  "type"     : "staticmethod",
  "class"    : "wifi",
  "name"     : "scan",
  "generate" : "jswrap_ESP8266_wifi_scan",
  "params"   : [
    ["callback", "JsVar", "A function to be called back on completion (optional)."]
  ]
}*/
void jswrap_ESP8266_wifi_scan(
    JsVar *jsCallback
  ) {
  os_printf("> jswrap_ESP8266_wifi_scan\n");
   if (jsCallback == NULL || !jsvIsFunction(jsCallback)) {
       jsExceptionHere(JSET_ERROR, "No callback.");
     return;
   }

   // If we had saved a previous scan callback function, release it.
   if (g_jsScanCallback != NULL) {
     jsvUnLock(g_jsScanCallback);
   }

   // Save the callback for the scan in the global variable called jsScanCallback.
   g_jsScanCallback = jsvLockAgainSafe(jsCallback);

   // Ask the ESP8266 to perform a network scan after first entering
   // station mode.  The network scan will eventually result in a callback
   // being executed (scanCB) which will contain the results.

   // Ensure we are in station mode
   if (g_preWiFiScanMode == -1) {
     g_preWiFiScanMode = wifi_get_opmode();
   }

   // Current         Resulting
   // --------------  --------------
   // NULL_MODE       STATION_MODE
   // STATION_MODE    STATION_MODE
   // SOFTAP_MODE     STATIONAP_MODE
   // STATIONAP_MODE  STATIONAP_MODE

   if (g_preWiFiScanMode == SOFTAP_MODE || g_preWiFiScanMode == STATIONAP_MODE) {
     wifi_set_opmode_current(STATIONAP_MODE);
   } else {
     wifi_set_opmode_current(STATION_MODE);
   }

   // Request a scan of the network calling "scanCB" on completion
   wifi_station_scan(NULL, scanCB);

   os_printf("< jswrap_ESP8266_wifi_scan\n");
}

/*JSON{
  "type"     : "staticmethod",
  "class"    : "wifi",
  "name"     : "createAP",
  "generate" : "jswrap_ESP8266_wifi_createAP",
  "params"   : [
    ["ssid", "JsVar", "The network id."],
    ["password", "JsVar", "A password for connecting stations (optional)."],
    ["options", "JsVar", "Configuration options (optional)."],
    ["callback", "JsVar", "A function to be called back on completion (optional)."]
  ]
}
* Create a logical access point allowing WiFi stations to connect.
* The `options` object can contain the following properties.
* * authMode - The authentication mode to use.  Can be one of "open", "wpa2", "wpa", "wpa_wpa2".
* * `default` - When true, sets the creation of an access point as the default at boot.
*/
void jswrap_ESP8266_wifi_createAP(
    JsVar *jsSsid,     //!< The network SSID that we will use to listen as.
    JsVar *jsPassword, //!< The password that stations will use to connect.
    JsVar *jsOptions,  //!< Configuration options.
    JsVar *jsCallback  //!< A callback to be invoked when completed.
  ) {
  os_printf("> jswrap_ESP8266_wifi_createAP\n");

  // If we had a previous createAP callback then release it.
  if (g_jsCreateAPCallback != NULL) {
    jsvUnLock(g_jsCreateAPCallback);
  }
  g_jsCreateAPCallback = jsCallback;


  // Validate that the SSID is provided and is a string.
  if (jsSsid == NULL || !jsvIsString(jsSsid)) {
      jsExceptionHere(JSET_ERROR, "No SSID.");
    return;
  }

  // Build our SoftAP configuration details
  struct softap_config softApConfig;
  memset(&softApConfig, 0, sizeof(softApConfig));

  softApConfig.authmode = (AUTH_MODE)-1;

  bool isDefault = false;
  // Handle any options that may have been supplied.
  if (jsOptions != NULL && jsvIsObject(jsOptions)) {

    // Handle "authMode" processing.  Here we check that "authMode", if supplied, is
    // one of the allowed values and set the softApConfig object property appropriately.
    JsVar *jsAuth = jsvObjectGetChild(jsOptions, "authMode", 0);
    if (jsAuth != NULL) {
      if (jsvIsString(jsAuth)) {
        if (jsvIsStringEqual(jsAuth, "open")) {
          softApConfig.authmode = AUTH_OPEN;
        } else if (jsvIsStringEqual(jsAuth, "wpa2")) {
          softApConfig.authmode = AUTH_WPA2_PSK;
        } else if (jsvIsStringEqual(jsAuth, "wpa")) {
          softApConfig.authmode = AUTH_WPA_PSK;
        } else if (jsvIsStringEqual(jsAuth, "wpa_wpa2")) {
          softApConfig.authmode = AUTH_WPA_WPA2_PSK;
        } else {
          jsvUnLock(jsAuth);
          jsExceptionHere(JSET_ERROR, "Unknown authMode value.");
          return;
        }
      } else {
        jsvUnLock(jsAuth);
        jsExceptionHere(JSET_ERROR, "The authMode must be a string.");
        return;
      }
      jsvUnLock(jsAuth);
      assert(softApConfig.authmode != (AUTH_MODE)-1);
    } // End of jsAuth is present.

    JsVar *jsDefault = jsvObjectGetChild(jsOptions, "default", 0);
    if (jsvIsBoolean(jsDefault)) {
      isDefault = jsvGetBool(jsDefault);
    }
  } // End of jsOptions is present

  // If no password has been supplied, then be open.  Otherwise, use WPA2 and the
  // password supplied.  Also check that the password is at least 8 characters long.
  if (jsPassword == NULL || !jsvIsString(jsPassword)) {
    if (softApConfig.authmode == (AUTH_MODE)-1) {
      softApConfig.authmode = AUTH_OPEN;
    }
    if (softApConfig.authmode != AUTH_OPEN) {
      jsExceptionHere(JSET_ERROR, "Password not set but authMode not 'open'.");
      return;
    }
  } else {
    if (jsvGetStringLength(jsPassword) < 8) {
      jsExceptionHere(JSET_ERROR, "Password must be 8 characters or more in length.");
      return;
    }
    if (softApConfig.authmode == (AUTH_MODE)-1) {
      softApConfig.authmode = AUTH_WPA2_PSK;
    }
    if (softApConfig.authmode == AUTH_OPEN) {
      jsWarn("wifi.connection: Auth mode set to open but password supplied!");
    }
    int len = jsvGetString(jsPassword, (char *)softApConfig.password, sizeof(softApConfig.password)-1);
    softApConfig.password[len]='\0';
  }

  int len = jsvGetString(jsSsid, (char *)softApConfig.ssid, sizeof(softApConfig.ssid)-1);
  softApConfig.ssid[len]='\0';

  // Define that we are in Soft AP mode including station mode if required.
  os_printf("Wifi: switching to soft-AP mode, authmode=%d\n", softApConfig.authmode);
  uint8 currentMode = wifi_get_opmode();

  // Current         Resulting
  // --------------  --------------
  // NULL_MODE       SOFTAP_MODE
  // STATION_MODE    STATIONAP_MODE
  // SOFTAP_MODE     SOFTAP_MODE
  // STATIONAP_MODE  STATIONAP_MODE

  if (currentMode == STATION_MODE || currentMode == STATIONAP_MODE) {
    if (isDefault) {
      wifi_set_opmode(STATIONAP_MODE);
    } else {
      wifi_set_opmode_current(STATIONAP_MODE);
    }
  } else {
    if (isDefault) {
      wifi_set_opmode(SOFTAP_MODE);
    } else {
      wifi_set_opmode_current(SOFTAP_MODE);
    }
  }

  softApConfig.ssid_len       = 0; // Null terminated SSID
  softApConfig.ssid_hidden    = 0; // Not hidden.
  softApConfig.max_connection = 4; // Maximum number of connections.

  // Set the WiFi configuration.
  int rc = wifi_softap_set_config_current(&softApConfig);
  // We should really check that becoming an access point works, however as of SDK 1.4, we
  // are finding that if we are currently connected to an access point and we switch to being
  // an access point, it works ... but returns 1 indicating an error.
  /*
  if (rc != 1) {
      os_printf(" - Error returned from wifi_softap_set_config_current=%d\n", rc);
      jsExceptionHere(JSET_ERROR, "Error setting ESP8266 softap config.");
  }
  */
  if (g_jsCreateAPCallback != NULL) {
    // Set the return error as a function of the return code returned from the call to
    // the ESP8266 API to disconnect from the access point.
    JsVar *params[1];
    params[0] = jsvNewNull();
    jsiQueueEvents(NULL, g_jsCreateAPCallback, params, 1);
  }
  os_printf("< jswrap_ESP8266_wifi_createAP\n");
}

/*JSON{
  "type"     : "staticmethod",
  "class"    : "wifi",
  "name"     : "getIP",
  "generate" : "jswrap_ESP8266_wifi_getIP",
  "return"   : ["JsVar", "A boolean representing our auto connect status"]
}
* Return IP information in an object which contains:
* * ip - IP address
* * netmask - The interface netmask
* * gw - The network gateway
*/
JsVar *jswrap_ESP8266_wifi_getIP() {
  os_printf("> jswrap_ESP8266_wifi_getIP\n");
  struct ip_info info;
  wifi_get_ip_info(0, &info);

  JsVar *ipInfo = jspNewObject(NULL, "Restart");
  jsvObjectSetChildAndUnLock(ipInfo, "ip", jsvNewFromInteger(info.ip.addr));
  jsvObjectSetChildAndUnLock(ipInfo, "netmask", jsvNewFromInteger(info.netmask.addr));
  jsvObjectSetChildAndUnLock(ipInfo, "gw", jsvNewFromInteger(info.gw.addr));
  os_printf("< jswrap_ESP8266_wifi_getIP\n");
  return ipInfo;
}


// Let's define the JavaScript class that will contain our `world()` method. We'll call it `Hello`
/*JSON{
  "type"  : "class",
  "class" : "ESP8266WiFi"
}*/

/*JSON{
    "type": "init",
    "generate": "jswrap_ESP8266_init"
}*/
void jswrap_ESP8266_init() {
  os_printf("> jswrap_ESP8266_init\n");
  jswrap_ESP8266WiFi_init();
  os_printf("< jswrap_ESP8266_init\n");
}

/*JSON{
    "type":     "kill",
    "generate": "jswrap_ESP8266WiFi_kill"
}
*
* Register a handler to be called when the ESP8266 is reset.
*/
void jswrap_ESP8266WiFi_kill() {
  // When the Espruino environment is reset, this callback function will be invoked.
  // The purpose is to reset the environment by cleaning up whatever might be needed
  // to be cleaned up.
  os_printf("> jswrap_esp8266_kill\n");

  // Handle the g_jsGotIpCallback - If it is not null, then it contains a reference
  // to a locked JS variable that is a function pointer for the "got ip" callback.
  // We want to unlock this and delete the reference.
  if (g_jsGotIpCallback != NULL) {
    jsvUnLock(g_jsGotIpCallback);
    g_jsGotIpCallback = NULL;
  }

  // Handle g_jsPingCallback release.
  if (g_jsPingCallback != NULL) {
    jsvUnLock(g_jsPingCallback);
    g_jsPingCallback = NULL;
  }

  // Handle g_jsWiFiEventCallback release.
  if (g_jsWiFiEventCallback != NULL) {
    jsvUnLock(g_jsWiFiEventCallback);
    g_jsWiFiEventCallback = NULL;
  }

  // Handle g_jsScanCallback release.
  if (g_jsScanCallback != NULL) {
    jsvUnLock(g_jsScanCallback);
    g_jsScanCallback = NULL;
  }

  // Handle g_jsHostByNameCallback release.
  if (g_jsHostByNameCallback != NULL) {
    jsvUnLock(g_jsHostByNameCallback);
    g_jsHostByNameCallback = NULL;
  }

  // Handle g_jsDisconnectCallback release.
  if (g_jsDisconnectCallback != NULL) {
    jsvUnLock(g_jsDisconnectCallback);
    g_jsDisconnectCallback = NULL;
  }

  // Handle g_jsStopAPCallback release.
  if (g_jsStopAPCallback != NULL) {
    jsvUnLock(g_jsStopAPCallback);
    g_jsStopAPCallback = NULL;
  }

  // Handle g_jsCreateAPCallback release.
  if (g_jsCreateAPCallback != NULL) {
    jsvUnLock(g_jsCreateAPCallback);
    g_jsCreateAPCallback = NULL;
  }

  os_printf("< jswrap_esp8266_kill\n");
}

/*JSON{
  "type"     : "staticmethod",
  "class"    : "ESP8266WiFi",
  "name"     : "connect",
  "generate" : "jswrap_ESP8266WiFi_connect",
  "params"   : [
    ["ssid","JsVar","The network id of the access point."],
    ["password","JsVar","The password to the access point"],
    ["gotIpCallback", "JsVar", "An optional callback invoked when we have an IP"]
  ]
}
 * Deprecated by jswrap_ESP8266_wifi_connect
 * Connect the station to an access point.
 */
void jswrap_ESP8266WiFi_connect(
    JsVar *jsv_ssid,     //!< The SSID of the access point to connect.
    JsVar *jsv_password, //!< The password for the access point.
    JsVar *gotIpCallback //!< The Callback function to be called when we are connected.
  ) {
    os_printf("> jswrap_ESP8266WiFi_connect\n");

  // Check that the ssid and password values aren't obviously in error.
  if (jsv_ssid == NULL || !jsvIsString(jsv_ssid)) {
      jsExceptionHere(JSET_ERROR, "No SSID.");
    return;
  }
  if (jsv_password == NULL || !jsvIsString(jsv_password)) {
      jsExceptionHere(JSET_ERROR, "No password.");
    return;
  }

  // Check that if a callback function was supplied that we actually have a callback function.
  if (gotIpCallback != NULL && !jsvIsUndefined(gotIpCallback) && !jsvIsFunction(gotIpCallback)) {
    gotIpCallback = NULL;
      jsExceptionHere(JSET_ERROR, "A callback function was supplied that is not a function.");
    return;
  }
  if (jsvIsUndefined(gotIpCallback) || jsvIsNull(gotIpCallback)) {
    gotIpCallback = NULL;
  }

  // Set the global which is the gotIP callback to null but first unlock it.
  if (g_jsGotIpCallback != NULL) {
    jsvUnLock(g_jsGotIpCallback);
    g_jsGotIpCallback = NULL;
  }

  // If we have a callback, save it for later invocation.
  if (gotIpCallback != NULL) {
    g_jsGotIpCallback = jsvLockAgainSafe(gotIpCallback);
  }

  // Debug
  // os_printf("jsGotIpCallback=%p\n", jsGotIpCallback);

  // Create strings from the JsVars for the ESP8266 API calls.
  char ssid[33];
  int len = jsvGetString(jsv_ssid, ssid, sizeof(ssid)-1);
  ssid[len]='\0';
  char password[65];
  len = jsvGetString(jsv_password, password, sizeof(password)-1);
  password[len]='\0';

  os_printf(">  - ssid=%s, password=%s\n", ssid, password);

  // Set the WiFi mode of the ESP8266
  wifi_set_opmode_current(STATION_MODE);

  struct station_config stationConfig;
  memset(&stationConfig, 0, sizeof(stationConfig));
  os_strncpy((char *)stationConfig.ssid, ssid, 32);
  if (password != NULL) {
    os_strncpy((char *)stationConfig.password, password, 64);
  } else {
    os_strcpy((char *)stationConfig.password, "");
  }

  // Set the WiFi configuration
  wifi_station_set_config(&stationConfig);

  uint8 wifiConnectStatus = wifi_station_get_connect_status();
  os_printf(" - Current connect status: %s\n", wifiConnectStatusToString(wifiConnectStatus));

  if (wifiConnectStatus == STATION_GOT_IP) {
    // See issue #618.  There are currently three schools of thought on what should happen
    // when a connect is issued and we are already connected.
    //
    // Option #1 - Always perform a disconnect.
    // Option #2 - Perform a disconnect if the SSID or PASSWORD are different from current
    // Option #3 - Fail the connect if we are already connected.
    //
#define ISSUE_618 1

#if ISSUE_618 == 1
    wifi_station_disconnect();
#elif ISSUE_618 == 2
    struct station_config existingConfig;
    wifi_station_get_config(&existingConfig);
    if (os_strncmp((char *)existingConfig.ssid, (char *)stationConfig.ssid, 32) == 0 &&
        os_strncmp((char *)existingConfig.password, (char *)stationConfig.password, 64) == 0) {
      if (jsGotIpCallback != NULL) {
        JsVar *params[2];
        params[0] = jsvNewFromInteger(STATION_GOT_IP);
         params[1] = jsvNewNull();
        jsiQueueEvents(NULL, jsGotIpCallback, params, 2);
      }
      return;

    } else {
      wifi_station_disconnect();
    }
#elif ISSUE_618 == 3
    // Add a return code to the function and return an already connected error.
#endif
  }
  // Perform the network level connection.
  wifi_station_connect();
  os_printf("< jswrap_ESP8266WiFi_connect\n");
}

/*JSON{
  "type"     : "staticmethod",
  "class"    : "ESP8266WiFi",
  "name"     : "stopAP",
  "generate" : "jswrap_ESP8266WiFi_stopAP"
}
 * Stop being an access point.
*/
// DEPRECATED by wifi.stopAP()
void jswrap_ESP8266WiFi_stopAP() {
  os_printf("Wifi: switching to Station mode.");
  wifi_set_opmode_current(STATION_MODE);
}


/**
 * Become an access point.
 * When we call this function we are instructing the ESP8266 to set itself up as an
 * access point to allow other WiFi stations to connect to it.  In order to be an access
 * point, the ESP8266 needs to know the SSID it should use as well as the password used
 * to allow clients to connect.
 *
 * Parameters:
 *  - `jsv_ssid` - The network identity that the access point will advertize itself as.
 *  - `jsv_password` - The password a station will need to connect to the
 * access point.
 *
 * Notes:
 *  - How about if the password is not supplied, NULL or empty then we set ourselves
 * up using an Open authentication mechanism?
 *  - Add support for hidden SSIDs.
 *
 */
/*JSON{
  "type"     : "staticmethod",
  "class"    : "ESP8266WiFi",
  "name"     : "beAccessPoint",
  "generate" : "jswrap_ESP8266WiFi_beAccessPoint",
  "params"   : [
    ["jsv_ssid", "JsVar", "The network SSID"],
    ["jsv_password", "JsVar", "The password to allow stations to connect to the access point"]
  ]
}
* Deprecated by jswrap_ESP8266_wifi_createAP
*/
void jswrap_ESP8266WiFi_beAccessPoint(
    JsVar *jsv_ssid,    //!< The network identity that the access point will advertize itself as.
    JsVar *jsv_password //!< The password a station will need to connect to the access point.
  ) {
  os_printf("> jswrap_ESP8266WiFi_beAccessPoint\n");
  // Validate that the SSID and password are somewhat useful.
  if (jsv_ssid == NULL || !jsvIsString(jsv_ssid)) {
      jsExceptionHere(JSET_ERROR, "No SSID.");
    return;
  }

  // Build our SoftAP configuration details
  struct softap_config softApConfig;
  memset(&softApConfig, 0, sizeof(softApConfig));

  // If no password has been supplied, then be open.  Otherwise, use WPA2 and the
  // password supplied.  Also check that the password is at least 8 characters long.
  if (jsv_password == NULL || !jsvIsString(jsv_password)) {
    softApConfig.authmode = AUTH_OPEN;
  } else {
    if (jsvGetStringLength(jsv_password) < 8) {
      jsExceptionHere(JSET_ERROR, "Password must be 8 characters or more in length.");
      return;
    }
    softApConfig.authmode = AUTH_WPA2_PSK;
    int len = jsvGetString(jsv_password, (char *)softApConfig.password, sizeof(softApConfig.password)-1);
    softApConfig.password[len]='\0';
  }

  int len = jsvGetString(jsv_ssid, (char *)softApConfig.ssid, sizeof(softApConfig.ssid)-1);
  softApConfig.ssid[len]='\0';

  // Define that we are in Soft AP mode.
  os_printf("Wifi: switching to soft-AP mode, authmode=%d\n", softApConfig.authmode);
  wifi_set_opmode_current(SOFTAP_MODE);

  softApConfig.ssid_len       = 0; // Null terminated SSID
  softApConfig.ssid_hidden    = 0; // Not hidden.
  softApConfig.max_connection = 4; // Maximum number of connections.

  // Set the WiFi configuration.
  int rc = wifi_softap_set_config_current(&softApConfig);
  // We should really check that becoming an access point works, however as of SDK 1.4, we
  // are finding that if we are currently connected to an access point and we switch to being
  // an access point, it works ... but returns 1 indicating an error.
  /*
  if (rc != 1) {
      os_printf(" - Error returned from wifi_softap_set_config_current=%d\n", rc);
      jsExceptionHere(JSET_ERROR, "Error setting ESP8266 softap config.");
  }
  */
  os_printf("< jswrap_ESP8266WiFi_beAccessPoint\n");
}


/**
 * Determine the list of access points available to us.
 */
/*JSON{
  "type"     : "staticmethod",
  "class"    : "ESP8266WiFi",
  "name"     : "getAccessPoints",
  "generate" : "jswrap_ESP8266WiFi_getAccessPoints",
  "params"   : [
    ["callback","JsVar","Function to call back when access points retrieved."]
  ]
}
* Deprecated by jswrap_ESP8266_wifi_scan
*/
void jswrap_ESP8266WiFi_getAccessPoints(
    JsVar *callback //!< Function to call back when access points retrieved.
  ) {
  os_printf("> ESP8266WiFi_getAccessPoints\n");
  if (callback == NULL || !jsvIsFunction(callback)) {
      jsExceptionHere(JSET_ERROR, "No callback.");
    return;
  }

  // If we had saved a previous scan callback function, release it.
  if (g_jsScanCallback != NULL) {
    jsvUnLock(g_jsScanCallback);
  }

  // Save the callback for the scan in the global variable called jsScanCallback.
  g_jsScanCallback = jsvLockAgainSafe(callback);

  // Ask the ESP8266 to perform a network scan after first entering
  // station mode.  The network scan will eventually result in a callback
  // being executed (scanCB) which will contain the results.

  // Ensure we are in station mode
  wifi_set_opmode_current(STATION_MODE);

  // Request a scan of the network calling "scanCB" on completion
  wifi_station_scan(NULL, scanCB);

  os_printf("< ESP8266WiFi_getAccessPoints\n");
}

/*JSON{
  "type"     : "staticmethod",
  "class"    : "ESP8266WiFi",
  "name"     : "mdnsInit",
  "generate" : "jswrap_ESP8266WiFi_mdnsInit"
}
 * Initial testing for mDNS support
 */
void jswrap_ESP8266WiFi_mdnsInit() {
  os_printf("> jswrap_ESP8266WiFi_mdnsInit\n");
  struct mdns_info mdnsInfo;
  os_memset(&mdnsInfo, 0, sizeof(struct mdns_info));
  // Populate the mdns structure

  struct ip_info ipInfo;
  wifi_get_ip_info(0, &ipInfo);

  mdnsInfo.host_name   = "myhostname";
  mdnsInfo.ipAddr      = ipInfo.ip.addr;
  mdnsInfo.server_name = "myservername";
  mdnsInfo.server_port = 80;
  //espconn_mdns_init(&mdnsInfo);
  os_printf("< jswrap_ESP8266WiFi_mdnsInit\n");
}

/**
 * Disconnect the station from the access point.
 */
/*JSON{
  "type"     : "staticmethod",
  "class"    : "ESP8266WiFi",
  "name"     : "disconnect",
  "generate" : "jswrap_ESP8266WiFi_disconnect"
}
* Deprecated by jswrap_ESP8266_wifi_disconnect
*/
void jswrap_ESP8266WiFi_disconnect() {
  wifi_station_disconnect();
}


/*JSON{
  "type"     : "staticmethod",
  "class"    : "ESP8266",
  "name"     : "restart",
  "generate" : "jswrap_ESP8266_restart"
}
 * Ask the physical ESP8266 device to restart itself.
 */
void jswrap_ESP8266_restart() {
  os_printf("> jswrap_ESP8266_restart\n");
  system_restart();
}


/**
 * Register a callback function that will be invoked when a WiFi event is detected.
 */
/*JSON{
  "type"     : "staticmethod",
  "class"    : "ESP8266WiFi",
  "name"     : "onWiFiEvent",
  "generate" : "jswrap_ESP8266WiFi_onWiFiEvent",
  "params"   : [
    ["callback","JsVar","WiFi event callback"]
  ]
}*/
void jswrap_ESP8266WiFi_onWiFiEvent(
    JsVar *callback //!< WiFi event callback.
  ) {
  // If the callback is null
  if (callback == NULL || jsvIsNull(callback)) {
    if (g_jsWiFiEventCallback != NULL) {
      jsvUnLock(g_jsWiFiEventCallback);
    }
    g_jsWiFiEventCallback = NULL;
    return;
  }

  if (!jsvIsFunction(callback)) {
      jsExceptionHere(JSET_ERROR, "No callback.");
    return;
  }

  // We are about to save a new global WiFi even callback handler.  If we have previously
  // had one, we need to unlock it so that we don't leak memory.
  if (g_jsWiFiEventCallback != NULL) {
    jsvUnLock(g_jsWiFiEventCallback);
  }

  // Save the global WiFi event callback handler.
  g_jsWiFiEventCallback = jsvLockAgainSafe(callback);
}


/**
 * Set whether or not the ESP8266 will perform an auto connect on startup.
 * A value of true means it will while a value of false means it won't.
 */
/*JSON{
  "type"     : "staticmethod",
  "class"    : "ESP8266WiFi",
  "name"     : "setAutoConnect",
  "generate" : "jswrap_ESP8266WiFi_setAutoConnect",
  "params"   : [
    ["autoconnect","JsVar","True if we wish to auto connect."]
  ]
}*/
void jswrap_ESP8266WiFi_setAutoConnect(
    JsVar *autoconnect //!< True if we wish to auto connect.
  ) {
  os_printf("Auto connect is: %d\n", (int)autoconnect);
  // Check that we have been passed a boolean ... if not, nothing to do here.
  if (!jsvIsBoolean(autoconnect)) {
    return;
  }

  uint8 newValue = jsvGetBool(autoconnect);
  os_printf("New value: %d\n", newValue);

  wifi_station_set_auto_connect(newValue);
  os_printf("Autoconnect changed\n");
}


/**
 * Retrieve whether or not the ESP8266 will perform an auto connect on startup.
 * A value of 1 means it will while a value of zero means it won't.
 */
/*JSON{
  "type"     : "staticmethod",
  "class"    : "ESP8266WiFi",
  "name"     : "getAutoConnect",
  "generate" : "jswrap_ESP8266WiFi_getAutoConnect",
  "return"   : ["JsVar","A boolean representing our auto connect status"]
}*/
JsVar *jswrap_ESP8266WiFi_getAutoConnect() {
  uint8 result = wifi_station_get_auto_connect();
  return jsvNewFromBool(result);
}


/**
 * Retrieve the reset information that is stored when event the ESP8266 resets.
 * The result will be a JS object containing the details.
 */
/*JSON{
  "type"     : "staticmethod",
  "class"    : "ESP8266",
  "name"     : "getRstInfo",
  "generate" : "jswrap_ESP8266_getRstInfo",
  "return"   : ["JsVar","A Restart Object"],
  "return_object" : "Restart"
}*/
JsVar *jswrap_ESP8266_getRstInfo() {
  struct rst_info* info = system_get_rst_info();
  JsVar *restartInfo = jspNewObject(NULL, "Restart");
  jsvObjectSetChildAndUnLock(restartInfo, "reason",   jsvNewFromInteger(info->reason));
  jsvObjectSetChildAndUnLock(restartInfo, "exccause", jsvNewFromInteger(info->exccause));
  jsvObjectSetChildAndUnLock(restartInfo, "epc1",     jsvNewFromInteger(info->epc1));
  jsvObjectSetChildAndUnLock(restartInfo, "epc2",     jsvNewFromInteger(info->epc2));
  jsvObjectSetChildAndUnLock(restartInfo, "epc3",     jsvNewFromInteger(info->epc3));
  jsvObjectSetChildAndUnLock(restartInfo, "excvaddr", jsvNewFromInteger(info->excvaddr));
  jsvObjectSetChildAndUnLock(restartInfo, "depc",     jsvNewFromInteger(info->depc));
  return restartInfo;
}

/*JSON{
  "type"     : "staticmethod",
  "class"    : "ESP8266",
  "name"     : "logDebug",
  "generate" : "jswrap_ESP8266_logDebug",
  "params"   : [
    ["enable", "JsVar", "Enable or disable the debug logging."]
  ]
}
 * Enable or disable the logging of debug information.  A value of `true` enables
 * debug logging while a value of `false` disables debug logging.  Debug output is sent
 * to UART1.
 */
void jswrap_ESP8266_logDebug(
    JsVar *jsDebug
  ) {
  uint8 enable = (uint8)jsvGetBool(jsDebug);
  os_printf("> jswrap_ESP8266_logDebug, enable=%d\n", enable);
  system_set_os_print((uint8)jsvGetBool(jsDebug));
  os_printf("< jswrap_ESP8266_logDebug\n");
}

/*JSON{
  "type"     : "staticmethod",
  "class"    : "ESP8266",
  "name"     : "updateCPUFreq",
  "generate" : "jswrap_ESP8266_updateCPUFreq",
  "params"   : [
    ["freq", "JsVar", "Desired frequency - either 80 or 160."]
  ]
}
 * Update the operating frequency of the ESP8266 processor.
 */
void jswrap_ESP8266_updateCPUFreq(
    JsVar *jsFreq //!< Operating frequency of the processor.  Either 80 or 160.
  ) {
  if (!jsvIsInt(jsFreq)) {
    jsExceptionHere(JSET_ERROR, "Invalid frequency.");
    return;
  }
  int newFreq = jsvGetInteger(jsFreq);
  if (newFreq != 80 && newFreq != 160) {
    jsExceptionHere(JSET_ERROR, "Invalid frequency value, must be 80 or 160.");
    return;
  }
  system_update_cpu_freq(newFreq);
}


/**
 * Return an object that contains details about the state of the ESP8266.
 *  - `sdkVersion`   - Version of the SDK.
 *  - `cpuFrequency` - CPU operating frequency.
 *  - `freeHeap`     - Amount of free heap.
 *  - `maxCon`       - Maximum number of concurrent connections.
 *
 */
/*JSON{
  "type"     : "staticmethod",
  "class"    : "ESP8266",
  "name"     : "getState",
  "generate" : "jswrap_ESP8266_getState",
  "return"   : ["JsVar", "The state of the ESP8266"]
}*/
JsVar *jswrap_ESP8266_getState() {
  // Create a new variable and populate it with the properties of the ESP8266 that we
  // wish to return.
  JsVar *esp8266State = jspNewObject(NULL, "ESP8266State");
  jsvObjectSetChildAndUnLock(esp8266State, "sdkVersion",   jsvNewFromString(system_get_sdk_version()));
  jsvObjectSetChildAndUnLock(esp8266State, "cpuFrequency", jsvNewFromInteger(system_get_cpu_freq()));
  jsvObjectSetChildAndUnLock(esp8266State, "freeHeap",     jsvNewFromInteger(system_get_free_heap_size()));
  jsvObjectSetChildAndUnLock(esp8266State, "maxCon",       jsvNewFromInteger(espconn_tcp_get_max_con()));
  return esp8266State;
}

/**
 * Return the value of an integer representation (4 bytes) of IP address
 * as a string.
 */
/*JSON{
  "type"     : "staticmethod",
  "class"    : "ESP8266WiFi",
  "name"     : "getAddressAsString",
  "generate" : "jswrap_ESP8266WiFi_getAddressAsString",
  "params"   : [
    ["address","JsVar","An integer value representing an IP address."]
  ],
  "return"   : ["JsVar","A String"]
}
* Deprecated by jswrap_ESP8266_getAddressAsString
*/
JsVar *jswrap_ESP8266WiFi_getAddressAsString(
    JsVar *address //!< An integer value representing an IP address.
  ) {
  if (!jsvIsInt(address)) {
    jsExceptionHere(JSET_ERROR, "No SSID.");
    return NULL;
  }
  uint32 iAddress = (uint32)jsvGetInteger(address);
  return networkGetAddressAsString((uint8 *)&iAddress, 4, 10, '.');
}


/**
 * Retrieve the IP information about this network interface and return a JS
 * object that contains its details.
 * The object will have the following properties defined upon it:
 *  - `ip` - The IP address of the interface.
 *  - `netmask` - The netmask of the interface.
 *  - `gw` - The gateway to reach when transmitting through the interface.
 */
/*JSON{
  "type"     : "staticmethod",
  "class"    : "ESP8266WiFi",
  "name"     : "getIPInfo",
  "generate" : "jswrap_ESP8266WiFi_getIPInfo",
  "return"   : ["JsVar","A IPInfo Object"],
  "return_object" : "IPInfo"
}
* Deprecated by jswrap_ESP8266_wifi_getIP
*/
JsVar *jswrap_ESP8266WiFi_getIPInfo() {
  struct ip_info info;
  wifi_get_ip_info(0, &info);

  JsVar *ipInfo = jspNewObject(NULL, "Restart");
  jsvObjectSetChildAndUnLock(ipInfo, "ip", jsvNewFromInteger(info.ip.addr));
  jsvObjectSetChildAndUnLock(ipInfo, "netmask", jsvNewFromInteger(info.netmask.addr));
  jsvObjectSetChildAndUnLock(ipInfo, "gw", jsvNewFromInteger(info.gw.addr));
  return ipInfo;
}


/**
 * Query the station configuration and return a JS object that represents the
 * current settings.
 * The object will have the following properties:
 *
 *  - `ssid` - The network identity of the access point
 *  - `password` - The password to use to connect to the access point
 *
 */
/*JSON{
  "type"     : "staticmethod",
  "class"    : "ESP8266WiFi",
  "name"     : "getStationConfig",
  "generate" : "jswrap_ESP8266WiFi_getStationConfig",
  "return"   : ["JsVar","A Station Config"],
  "return_object" : "StationConfig"
}*/
JsVar *jswrap_ESP8266WiFi_getStationConfig() {
  struct station_config config;
  wifi_station_get_config(&config);
  JsVar *jsConfig = jspNewObject(NULL, "StationConfig");
  //char ssid[33];
  //nullTerminateString(ssid, (char *)config.ssid, 32);
  jsvObjectSetChildAndUnLock(jsConfig, "ssid", jsvNewFromString((char *)config.ssid));
  //char password[65];
  //nullTerminateString(password, (char *)config.password, 64);
  jsvObjectSetChildAndUnLock(jsConfig, "password", jsvNewFromString((char *)config.password));
  return jsConfig;
}


/**
 * Determine the list of connected stations and return them.
 */
/*JSON{
  "type"     : "staticmethod",
  "class"    : "ESP8266WiFi",
  "name"     : "getConnectedStations",
  "generate" : "jswrap_ESP8266WiFi_getConnectedStations",
  "return"   : ["JsVar","An array of connected stations."]
}*/
JsVar *jswrap_ESP8266WiFi_getConnectedStations() {
  uint8 stationCount = wifi_softap_get_station_num();
  struct station_info *stationInfo = wifi_softap_get_station_info();
  JsVar *jsArray = jsvNewArray(NULL, 0);
  if (stationInfo != NULL) {
    while (stationInfo != NULL) {
      os_printf("Station IP: %d.%d.%d.%d\n", IP2STR(&(stationInfo->ip)));
      JsVar *jsStation = jsvNewWithFlags(JSV_OBJECT);
      jsvObjectSetChildAndUnLock(jsStation, "ip", jsvNewFromInteger(stationInfo->ip.addr));
      jsvArrayPush(jsArray, jsStation);
      stationInfo = STAILQ_NEXT(stationInfo, next);
    }
    wifi_softap_free_station_info();
  }
  return jsArray;
}

/**
 * Handle a response from espconn_gethostbyname.
 * Invoke the callback function to inform the caller that a hostname has been converted to
 * an IP address.  The callback function should take a parameter that is the IP address.
 */
static void dnsFoundCallback(
    const char *hostname, //!< The hostname that was converted to an IP address.
    ip_addr_t *ipAddr,    //!< The ip address retrieved.  This may be 0.
    void *arg             //!< Parameter passed in from espconn_gethostbyname.
  ) {
  os_printf(">> dnsFoundCallback - %s %x\n", hostname, ipAddr->addr);
  if (g_jsHostByNameCallback != NULL) {
    JsVar *params[1];
    params[0] = jsvNewFromInteger(ipAddr->addr);
    jsiQueueEvents(NULL, g_jsHostByNameCallback, params, 1);
    jsvUnLock(params[0]);
    jsvUnLock(g_jsHostByNameCallback);
    g_jsHostByNameCallback = NULL;
  }
  os_printf("<< dnsFoundCallback\n");
}


/*JSON{
  "type"     : "staticmethod",
  "class"    : "ESP8266",
  "name"     : "getHostByName",
  "generate" : "jswrap_ESP8266_getHostByName",
    "params"   : [
    ["hostname", "JsVar", "The hostname to lookup."],
    ["callback", "JsVar", "The callback to invoke when the hostname is returned."]
  ]
}
 * Lookup the hostname and invoke a callback when the IP address is known.
*/

void jswrap_ESP8266_getHostByName(
    JsVar *jsHostname,
    JsVar *jsCallback
  ) {
  ip_addr_t ipAddr;
  char hostname[256];

  if (jsvIsString(jsHostname) == false) {
    jsExceptionHere(JSET_ERROR, "Not a valid hostname.");
    return;
  }
  if (jsvIsFunction(jsCallback) == false) {
    jsExceptionHere(JSET_ERROR, "Not a valid callback function.");
    return;
  }
  os_printf("> jswrap_ESP8266_getHostByName\n");
  // Save the callback unlocking an old callback if needed.
  if (g_jsHostByNameCallback != NULL) {
    jsvUnLock(g_jsHostByNameCallback);
  }
  g_jsHostByNameCallback = jsCallback;
  jsvLockAgainSafe(g_jsHostByNameCallback);

  jsvGetString(jsHostname, hostname, sizeof(hostname));
  err_t err = espconn_gethostbyname(NULL, hostname, &ipAddr, dnsFoundCallback);
  if (err == ESPCONN_OK) {
    os_printf("Already got\n");
    dnsFoundCallback(hostname, &ipAddr, NULL);
  } else if (err != ESPCONN_INPROGRESS) {
    os_printf("Error: %d from espconn_gethostbyname\n", err);
  }
  os_printf("< jswrap_ESP8266_getHostByName\n");
}


/*JSON{
  "type"     : "staticmethod",
  "class"    : "ESP8266WiFi",
  "name"     : "getDHCPHostname",
  "generate" : "jswrap_ESP8266WiFi_getDHCPHostname",
  "return"   : ["JsVar", "The current DHCP hostname."]
}
 * Get the current DHCP hostname.
*/
JsVar *jswrap_ESP8266WiFi_getDHCPHostname() {
  char *hostname = wifi_station_get_hostname();
  if (hostname == NULL) {
    hostname = "";
  }
  return jsvNewFromString(hostname);
}

/*JSON{
  "type"     : "staticmethod",
  "class"    : "ESP8266WiFi",
  "name"     : "setDHCPHostname",
  "generate" : "jswrap_ESP8266WiFi_setDHCPHostname",
  "params"   : [
    ["hostname", "JsVar", "The new DHCP hostname."]
  ]
}
 * Set the DHCP hostname.
*/
void jswrap_ESP8266WiFi_setDHCPHostname(
    JsVar *jsHostname //!< The hostname to set for device.
  ) {
  char hostname[256];
  jsvGetString(jsHostname, hostname, sizeof(hostname));
  os_printf("> jswrap_ESP8266WiFi_setDHCPHostname: %s\n", hostname);
  wifi_station_set_hostname(hostname);
}


/**
 * Get the signal strength.
 */
/*JSON{
  "type"     : "staticmethod",
  "class"    : "ESP8266WiFi",
  "name"     : "getRSSI",
  "generate" : "jswrap_ESP8266WiFi_getRSSI",
  "return"   : ["JsVar","An integer representing the signal strength."]
}*/
JsVar *jswrap_ESP8266WiFi_getRSSI() {
  int rssi = wifi_station_get_rssi();
  return jsvNewFromInteger(rssi);
}



/**
 * Initialize the ESP8266 environment.
 */
/*JSON{
  "type"     : "staticmethod",
  "class"    : "ESP8266WiFi",
  "name"     : "init",
  "generate" : "jswrap_ESP8266WiFi_init"
}*/
void jswrap_ESP8266WiFi_init() {
  os_printf("> jswrap_ESP8266WiFi_init\n");

  // Record that we don't know the current preWiFiScan mode.
  g_preWiFiScanMode = -1;

  // register the state change handler so we get debug printout for sure
  wifi_set_phy_mode(2);
  wifi_set_event_handler_cb(wifiEventHandler);
  os_printf("Wifi init, mode=%d\n", wifi_get_opmode());
  wifi_station_set_hostname("espruino");

  netInit_esp8266_board();
  setupJsNetwork();
  networkState = NETWORKSTATE_ONLINE;
  os_printf("< jswrap_ESP8266WiFi_init\n");
}


/**
 * Return the ESP8266 connection status.
 * This is an integer value defined as:
 *  - 0 - STATION_IDLE
 *  - 1 - STATION_CONNECTING
 *  - 2 - STATION_WRONG_PASSWORD
 *  - 3 - STATION_NO_AP_FOUND
 *  - 4 - STATION_CONNECT_FAIL
 *  - 5 - STATION_GOT_IP
 */
/*JSON{
  "type"     : "staticmethod",
  "class"    : "ESP8266WiFi",
  "name"     : "getConnectStatus",
  "generate" : "jswrap_ESP8266WiFi_getConnectStatus",
  "return"   : ["JsVar","A connection status"]
}

Retrieve the connection status.  The return is an object that contains:

* status - The status code from ESP8266
* statusMsg - The description of the code

*/
JsVar *jswrap_ESP8266WiFi_getConnectStatus() {
  // Ask ESP8266 for the connection status
  uint8 status = wifi_station_get_connect_status();

  // Create a JS variable to return
  JsVar *var = jsvNewWithFlags(JSV_OBJECT);

  // Populate the return JS variable with a property called "status"
  JsVar *jsStatus = jsvNewFromInteger(status);
  //jsvUnLock(jsStatus);
  jsvObjectSetChildAndUnLock(var, "status", jsStatus);

  // Populate the return JS variable with a property called "statusMsg"
  char *statusMsg;
  switch(status) {
  case STATION_IDLE:
    statusMsg = "STATION_IDLE";
    break;
  case STATION_CONNECTING:
    statusMsg = "STATION_CONNECTING";
    break;
  case STATION_WRONG_PASSWORD:
    statusMsg = "STATION_WRONG_PASSWORD";
    break;
  case STATION_NO_AP_FOUND:
    statusMsg = "STATION_NO_AP_FOUND";
    break;
  case STATION_CONNECT_FAIL:
    statusMsg = "STATION_CONNECT_FAIL";
    break;
  case STATION_GOT_IP:
    statusMsg = "STATION_GOT_IP";
    break;
  default:
    statusMsg = "*** Unknown ***";
  }
  JsVar *jsStatusMsg = jsvNewFromString(statusMsg);
  //jsvUnLock(jsStatusMsg);
  jsvObjectSetChildAndUnLock(var, "statusMsg", jsStatusMsg);
  //jsvUnLock(var);
  return var;
}


/**
 * Test: Perform a socket connection to a partner system.
 */
/*JSON{
  "type"     : "staticmethod",
  "class"    : "ESP8266WiFi",
  "name"     : "socketConnect",
  "generate" : "jswrap_ESP8266WiFi_socketConnect",
  "params"   : [
    ["options","JsVar","Some kind of options."],
    ["callback","JsVar","Some kind of callback."]
  ],
  "return"   : ["JsVar","A connection object"]
}*/
JsVar *jswrap_ESP8266WiFi_socketConnect(
    JsVar *options, //!< Some kind of options.
    JsVar *callback //!< Some kind of callback.
  ) {
  os_printf("Network state = %d\n", networkState);
  JsVar *ret = jswrap_net_connect(options, callback, ST_NORMAL);
  return ret;
}


/*JSON{
  "type"     : "staticmethod",
  "class"    : "ESP8266WiFi",
  "name"     : "socketEnd",
  "generate" : "jswrap_ESP8266WiFi_socketEnd",
  "params"   : [
    ["socket","JsVar","The socket to be closed."],
    ["data","JsVar","Optional data to be sent before close."]
  ]
}*/
void jswrap_ESP8266WiFi_socketEnd(
    JsVar *socket, //!< The socket to be closed.
    JsVar *data    //!< Optional data to be sent before close.
  ) {
  jswrap_net_socket_end(socket, data);
}


/**
 * Perform a network ping request.
 * The parameter can be either a String or a numeric IP address.
 */
/*JSON{
  "type"     : "staticmethod",
  "class"    : "ESP8266",
  "name"     : "ping",
  "generate" : "jswrap_ESP8266_ping",
  "params"   : [
    ["ipAddr", "JsVar", "A string or integer representation of an IP address."],
    ["pingCallback", "JsVar", "Optional callback function."]
  ]
}*/
void jswrap_ESP8266_ping(
    JsVar *ipAddr,      //!< A string or integer representation of an IP address.
    JsVar *pingCallback //!< Optional callback function.
  ) {
  // If the parameter is a string, get the IP address from the string
  // representation.
  if (jsvIsString(ipAddr)) {
    char ipString[20];
    int len = jsvGetString(ipAddr, ipString, sizeof(ipString)-1);
    ipString[len] = '\0';
    pingOpt.ip = networkParseIPAddress(ipString);
    if (pingOpt.ip == 0) {
        jsExceptionHere(JSET_ERROR, "Not a valid IP address.");
      return;
    }
  } else
  // If the parameter is an integer, treat it as an IP address.
  if (jsvIsInt(ipAddr)) {
    pingOpt.ip = jsvGetInteger(ipAddr);
  } else
  // The parameter was neither a string nor an IP address and hence we don't
  // know how to get the IP address of the partner to ping so throw an
  // exception.
  {
      jsExceptionHere(JSET_ERROR, "IP address must be string or integer.");
    return;
  }

  if (jsvIsUndefined(pingCallback) || jsvIsNull(pingCallback)) {
    if (g_jsPingCallback != NULL) {
      jsvUnLock(g_jsPingCallback);
    }
    g_jsPingCallback = NULL;
  } else if (!jsvIsFunction(pingCallback)) {
      jsExceptionHere(JSET_ERROR, "Callback is not a function.");
    return;
  } else {
    if (g_jsPingCallback != NULL) {
      jsvUnLock(g_jsPingCallback);
    }
    g_jsPingCallback = pingCallback;
    jsvLockAgainSafe(g_jsPingCallback);
  }

  // We now have an IP address to ping ... so ping.
  memset(&pingOpt, 0, sizeof(pingOpt));
  pingOpt.count = 5;
  pingOpt.recv_function = pingRecvCB;
  ping_start(&pingOpt);
}


/**
 * Dump the data in the socket.
 */
/*JSON{
  "type"     : "staticmethod",
  "class"    : "ESP8266",
  "name"     : "dumpSocket",
  "generate" : "jswrap_ESP8266_dumpSocket",
  "params"   : [
    ["socketId","JsVar","The socket to be dumped."]
  ]
}*/
void jswrap_ESP8266_dumpSocket(
    JsVar *socketId //!< The socket to be dumped.
  ) {
  esp8266_dumpSocket(jsvGetInteger(socketId)-1);
}

/*JSON{
  "type"     : "staticmethod",
  "class"    : "ESP8266",
  "name"     : "dumpAllSocketData",
  "generate" : "jswrap_ESP8266_dumpAllSocketData"
}
* Write all the socket data structures to the debug log.
* This is purely a diagnostic function.
*/
void jswrap_ESP8266_dumpAllSocketData() {
  esp8266_dumpAllSocketData();
}

/**
 * Null terminate a string.
 */
static char *nullTerminateString(char *target, char *source, int sourceLength) {
  os_strncpy(target, source, sourceLength);
  target[sourceLength-1] = '\0';
  return target;
}

/**
 *
 */
static void setupJsNetwork() {
  JsNetwork net;
  networkCreate(&net, JSNETWORKTYPE_ESP8266_BOARD);
  networkSet(&net);
}


/**
 * Handle receiving a response from a ping reply.
 * If a callback function has been supplied we invoked that callback by queuing it for future
 * execution.  A parameter is supplied to the callback which is a JavaScript object that contains:
 *  - totalCount
 *  - totalBytes
 *  - totalTime
 *  - respTime
 *  - seqNo
 *  - timeoutCount
 *  - bytes
 *  - error
 */
static void pingRecvCB(void *pingOpt, void *pingResponse) {
  struct ping_resp *pingResp = (struct ping_resp *)pingResponse;
  os_printf("Received a ping response!\n");
  if (g_jsPingCallback != NULL) {
    JsVar *jsPingResponse = jspNewObject(NULL, "PingResponse");
    jsvObjectSetChildAndUnLock(jsPingResponse, "totalCount",   jsvNewFromInteger(pingResp->total_count));
    jsvObjectSetChildAndUnLock(jsPingResponse, "totalBytes",   jsvNewFromInteger(pingResp->total_bytes));
    jsvObjectSetChildAndUnLock(jsPingResponse, "totalTime",    jsvNewFromInteger(pingResp->total_time));
    jsvObjectSetChildAndUnLock(jsPingResponse, "respTime",     jsvNewFromInteger(pingResp->resp_time));
    jsvObjectSetChildAndUnLock(jsPingResponse, "seqNo",        jsvNewFromInteger(pingResp->seqno));
    jsvObjectSetChildAndUnLock(jsPingResponse, "timeoutCount", jsvNewFromInteger(pingResp->timeout_count));
    jsvObjectSetChildAndUnLock(jsPingResponse, "bytes",        jsvNewFromInteger(pingResp->bytes));
    jsvObjectSetChildAndUnLock(jsPingResponse, "error",        jsvNewFromInteger(pingResp->ping_err));
    JsVar *params[1];
    params[0] = jsPingResponse;
    jsiQueueEvents(NULL, g_jsPingCallback, params, 1);
  }
}


/**
 * Callback function that is invoked at the culmination of a scan.
 */
static void scanCB(void *arg, STATUS status) {
  /**
   * Create a JsVar that is an array of JS objects where each JS object represents a
   * retrieved access point set of information.   The structure of a record will be:
   * o authMode
   * o isHidden
   * o rssi
   * o channel
   * o ssid
   * When the array has been built, invoke the callback function passing in the array
   * of records.
   */

  os_printf(">> scanCB\n");

  // Set the opmode back to the value it was prior to the request for a scan.
  assert(g_preWiFiScanMode != -1);
  wifi_set_opmode_current(g_preWiFiScanMode);
  g_preWiFiScanMode = -1;

  // Create the Empty JS array that will be passed as a parameter to the callback.
  JsVar *accessPointArray = jsvNewArray(NULL, 0);
  struct bss_info *bssInfo;

  bssInfo = (struct bss_info *)arg;
  while(bssInfo != NULL) {
    // Add a new object to the JS array that will be passed as a parameter to
    // the callback.  The ESP8266 bssInfo structure contains the following:
    // ---
    // uint8 bssid[6]
    // uint8 ssid[32]
    // uint8 channel
    // sint8 rssi \96 The received signal strength indication
    // AUTH_MODE authmode
    //  Open = 0
    //  WEP = 1
    //  WPA_PSK = 2
    //  WPA2_PSK = 3
    //  WPA_WPA2_PSK = 4
    // uint8 is_hidden
    // sint16 freq_offset
    // ---
    // Create, populate and add a child ...
    JsVar *currentAccessPoint = jspNewObject(NULL, "AccessPoint");
    jsvObjectSetChildAndUnLock(currentAccessPoint, "rssi", jsvNewFromInteger(bssInfo->rssi));
    jsvObjectSetChildAndUnLock(currentAccessPoint, "channel", jsvNewFromInteger(bssInfo->channel));
    jsvObjectSetChildAndUnLock(currentAccessPoint, "authMode", jsvNewFromInteger(bssInfo->authmode));
    jsvObjectSetChildAndUnLock(currentAccessPoint, "isHidden", jsvNewFromBool(bssInfo->is_hidden));
    // The SSID may **NOT** be NULL terminated ... so handle that.
    char ssid[sizeof(bssInfo->ssid) + 1];
    os_strncpy((char *)ssid, (char *)bssInfo->ssid, sizeof(bssInfo->ssid));
    ssid[sizeof(ssid)-1] = '\0';
    jsvObjectSetChildAndUnLock(currentAccessPoint, "ssid", jsvNewFromString(ssid));

    // Add the new record to the array
    jsvArrayPush(accessPointArray, currentAccessPoint);

    os_printf(" - ssid: %s\n", bssInfo->ssid);
    bssInfo = STAILQ_NEXT(bssInfo, next);
  }

  // We have now completed the scan callback, so now we can invoke the JS callback.
  JsVar *params[1];
  params[0] = accessPointArray;
  jsiQueueEvents(NULL, g_jsScanCallback, params, 1);
  jsvUnLock(g_jsScanCallback);
  os_printf("<< scanCB\n");
}


/**
 * Invoke the JavaScript callback to notify the program that an ESP8266
 * WiFi event has occurred.
 */
static void sendWifiEvent(
    uint32 eventType, //!< The ESP8266 WiFi event type.
    JsVar *jsDetails  //!< The JS object to be passed as a parameter to the callback.
  ) {
  jsvUnLock(jsDetails);

  // We need to check that we actually have an event callback handler because
  // it might have been disabled/removed.
  if (g_jsWiFiEventCallback != NULL) {
    // Build a callback event.
    JsVar *params[2];
    params[0] = jsvNewFromInteger(eventType);
    params[1] = jsDetails;
    jsiQueueEvents(NULL, g_jsWiFiEventCallback, params, 2);
  }

  if (g_jsGotIpCallback != NULL && eventType == EVENT_STAMODE_GOT_IP) {
    JsVar *params[2];
    params[0] = jsvNewNull();
    params[1] = jswrap_ESP8266_wifi_getIP();
    jsiQueueEvents(NULL, g_jsGotIpCallback, params, 2);
    // Once we have invoked the callback, we can unlock and release
    // the variable as we are only calling it once.
    jsvUnLock(g_jsGotIpCallback);
    g_jsGotIpCallback = NULL;
  }

  if (g_jsGotIpCallback != NULL && eventType == EVENT_STAMODE_DISCONNECTED) {
    uint8 stationCurrentStatus = wifi_station_get_connect_status();
    if (stationCurrentStatus == STATION_WRONG_PASSWORD || stationCurrentStatus == STATION_NO_AP_FOUND) {
      JsVar *params[2];
      switch(stationCurrentStatus) {
      case STATION_WRONG_PASSWORD:
        params[0] = createErrorVar(STATION_WRONG_PASSWORD, "Wrong password");
        break;
      case STATION_NO_AP_FOUND:
        params[0] = createErrorVar(STATION_NO_AP_FOUND, "No access point found");
        break;
      default:
        params[0] = createErrorVar(stationCurrentStatus, "Unknown error!!");
      }
      params[1] = jswrap_ESP8266_wifi_getIP();
      jsiQueueEvents(NULL, g_jsGotIpCallback, params, 2);
      // Once we have invoked the callback, we can unlock and release
      // the variable as we are only calling it once.
      jsvUnLock(g_jsGotIpCallback);
      g_jsGotIpCallback = NULL;

      // Current         Resulting
      // --------------  --------------
      // NULL_MODE       NULL_MODE
      // STATION_MODE    NULL_MODE
      // SOFTAP_MODE     SOFTAP_MODE
      // STATIONAP_MODE  SOFTAP_MODE
      uint8 currentMode = wifi_get_opmode();
      if (currentMode == SOFTAP_MODE || currentMode == STATIONAP_MODE) {
        wifi_set_opmode(SOFTAP_MODE);
      } else {
        wifi_set_opmode(NULL_MODE);
      }
    }
  }
}


/**
 * ESP8266 WiFi Event handler.
 * This function is called by the ESP8266
 * environment when significant events happen related to the WiFi environment.
 * The event handler is registered with a call to wifi_set_event_handler_cb()
 * that is provided by the ESP8266 SDK.
 */
static void wifiEventHandler(System_Event_t *evt) {
  switch(evt->event) {
  // We have connected to an access point.
  case EVENT_STAMODE_CONNECTED:
    os_printf("Wifi connected to ssid %s, ch %d\n", evt->event_info.connected.ssid,
      evt->event_info.connected.channel);
    sendWifiEvent(evt->event, jsvNewNull());
    break;

  // We have disconnected or been disconnected from an access point.
  case EVENT_STAMODE_DISCONNECTED:
    os_printf("Wifi disconnected from ssid %s, reason %s (%d)\n",
      evt->event_info.disconnected.ssid,
      wifiGetReason(evt->event_info.disconnected.reason),
      evt->event_info.disconnected.reason);

    // Get the current connect status
    uint8 stationCurrentStatus = wifi_station_get_connect_status();
    os_printf("Current status: %s\n", wifiConnectStatusToString(stationCurrentStatus));

    JsVar *details = jspNewObject(NULL, "EventDetails");
    jsvObjectSetChildAndUnLock(details, "reason", jsvNewFromInteger(evt->event_info.disconnected.reason));
    char ssid[33];
    memcpy(ssid, evt->event_info.disconnected.ssid, evt->event_info.disconnected.ssid_len);
    ssid[ evt->event_info.disconnected.ssid_len] = '\0';
    sendWifiEvent(evt->event, details);
    break;

  // The authentication information at the access point has changed.
  case EVENT_STAMODE_AUTHMODE_CHANGE:
    os_printf("Wifi auth mode: %d -> %d\n",
      evt->event_info.auth_change.old_mode, evt->event_info.auth_change.new_mode);
    sendWifiEvent(evt->event, jsvNewNull());
    break;

  // We have been allocated an IP address.
  case EVENT_STAMODE_GOT_IP:
    os_printf("Wifi got ip:" IPSTR ",mask:" IPSTR ",gw:" IPSTR "\n",
      IP2STR(&evt->event_info.got_ip.ip), IP2STR(&evt->event_info.got_ip.mask),
      IP2STR(&evt->event_info.got_ip.gw));
    sendWifiEvent(evt->event, jsvNewNull());
    break;

  case EVENT_STAMODE_DHCP_TIMEOUT:
    os_printf("Wifi DHCP timeout");
    sendWifiEvent(evt->event, jsvNewNull());
    break;

  case EVENT_SOFTAPMODE_STACONNECTED:
    os_printf("Wifi AP: station " MACSTR " joined, AID = %d\n",
      MAC2STR(evt->event_info.sta_connected.mac), evt->event_info.sta_connected.aid);
    sendWifiEvent(evt->event, jsvNewNull());
    break;

  case EVENT_SOFTAPMODE_STADISCONNECTED:
    os_printf("Wifi AP: station " MACSTR " left, AID = %d\n",
      MAC2STR(evt->event_info.sta_disconnected.mac), evt->event_info.sta_disconnected.aid);
    sendWifiEvent(evt->event, jsvNewNull());
    break;

  case EVENT_SOFTAPMODE_PROBEREQRECVED:
    os_printf("Wifi AP: probe request from station " MACSTR ", rssi = %d\n",
      MAC2STR(evt->event_info.ap_probereqrecved.mac), evt->event_info.ap_probereqrecved.rssi);
    sendWifiEvent(evt->event, jsvNewNull());
    break;

  default:
    os_printf("Wifi: unexpected event %d\n", evt->event);
    sendWifiEvent(evt->event, jsvNewNull());
    break;
  }
}

/**
 * Write an IP address as a dotted decimal string.
 */
// Note: This may be a duplicate ... it appears that we may have an existing function
// in network.c which does exactly this and more!!
//
static void ipAddrToString(struct ip_addr addr, char *string) {
  os_sprintf(string, "%d.%d.%d.%d", ((char *)&addr)[0], ((char *)&addr)[1], ((char *)&addr)[2], ((char *)&addr)[3]);
}

/**
 * Convert an ESP8266 WiFi connect status to a string.
 *
 * Convert the status (as returned by `wifi_station_get_connect_status()`) to a string
 * representation.
 * \return A string representation of a WiFi connect status.
 */
static char *wifiConnectStatusToString(uint8 status) {
  switch(status) {
  case STATION_IDLE:
    return "STATION_IDLE";
  case STATION_CONNECTING:
    return "STATION_CONNECTING";
  case STATION_WRONG_PASSWORD:
    return "STATION_WRONG_PASSWORD";
  case STATION_NO_AP_FOUND:
    return "STATION_NO_AP_FOUND";
  case STATION_CONNECT_FAIL:
    return "STATION_CONNECT_FAIL";
  case STATION_GOT_IP:
    return "STATION_GOT_IP";
  default:
    return "Unknown connect status!!";
  }
}

/**
 * Create a JsVar from an error code and and error message.
 * The returned JsVar contains the following fields:
 * * `errorCode` - The code representing the error.
 * * `errorMessage` - A string describing the error.
 * \return A populated instance of a JsVar object.
 */
static JsVar *createErrorVar(int errorCode, char *errorMessage) {
  JsVar *jsErrorVar = jsvNewWithFlags(JSV_OBJECT);
  jsvObjectSetChildAndUnLock(jsErrorVar, "errorCode", jsvNewFromInteger(errorCode));
  jsvObjectSetChildAndUnLock(jsErrorVar, "errorMessage", jsvNewFromString(errorMessage));
  return jsErrorVar;
}
