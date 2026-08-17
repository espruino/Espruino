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
 * Contains ESP32 and Wifi library specific functions.
 *
 * FOR DESCRIPTIONS OF THE WIFI FUNCTIONS IN THIS FILE, SEE
 * libs/network/jswrap_wifi.c (or http://www.espruino.com/Reference#Wifi)
 *
 * IMPORTANT: the functions in this file have tests in ./tests/wifi-test-mode.js
 * please maintain these tests if you make functional changes!
 * ----------------------------------------------------------------------------
 */

// Includes from ESP-IDF
#include "esp_wifi.h"
#include "esp_event_loop.h"
#if ESP_IDF_VERSION_MAJOR>=5
#include "esp_netif.h"
#include "esp_event.h"
#include "mdns.h"
#include "ping/ping.h"
#include "esp_ping.h"
#include "esp_sntp.h"
#include "esp_mac.h"
#include "nvs_flash.h"
#elif ESP_IDF_VERSION_MAJOR>=4

#include "tcpip_adapter.h"
#include "mdns.h"
#include "ping/ping.h"
#include "esp_ping.h"
#include "sntp/sntp.h"
#else

#include "tcpip_adapter.h"
#include "mdns/include/mdns.h"
#include "lwip/include/apps/ping/ping.h"
#include "lwip/include/apps/esp_ping.h"
#include "apps/sntp/sntp.h"
#endif

#include "lwip/dns.h"
#include "jsinteractive.h"
#include "network.h"
#include "jswrap_modules.h"
#include "jswrap_esp32_network.h"
#include "jswrap_storage.h"

#include "jsutils.h"

#define UNUSED(x) (void)(x)

#if ESP_IDF_VERSION_MAJOR>=5
esp_netif_t *sta_netif;
static esp_event_handler_instance_t instance_wifi = NULL;
static esp_event_handler_instance_t instance_ip = NULL;
//static esp_event_handler_instance_t instance_ap_ip = NULL;

#endif

#ifndef TCPIP_ADAPTER_IF_STA
#define TCPIP_ADAPTER_IF_STA 0
#define TCPIP_ADAPTER_IF_AP  1
#endif

static void sendWifiCompletionCB(
  JsVar **g_jsCallback,  //!< Pointer to the global callback variable
  char  *reason          //!< NULL if successful, error string otherwise
);

// A callback function to be invoked on a disconnect response.
static JsVar *g_jsDisconnectCallback;

// A callback function to be invoked when we have an IP address.
static JsVar *g_jsGotIpCallback;

// A callback function to be invoked on ping responses.
static JsVar *g_jsPingCallback;

// A callback function to be invoked on gethostbyname responses.
static JsVar *g_jsHostByNameCallback;

// A callback function to be invoked when we complete an access point scan.
static JsVar *g_jsScanCallback;

// A callback function to be invoked when we are being an access point.
static JsVar *g_jsAPStartedCallback;

#if ESP_IDF_VERSION_MAJOR>=5
// The last time we were connected as a station.
static wifi_event_sta_connected_t g_lastEventStaConnected;
// The last time we were disconnected as a station.
static wifi_event_sta_disconnected_t g_lastEventStaDisconnected;
#else
// The last time we were connected as a station.
static system_event_sta_connected_t g_lastEventStaConnected;

// The last time we were disconnected as a station.
static system_event_sta_disconnected_t g_lastEventStaDisconnected;
#endif
// Are we connected as a station?
static bool g_isStaConnected = false;

// Has the AP started?
static bool g_isAPStarted = false;

#define EXPECT_CB_EXCEPTION(jsCB)   jsExceptionHere(JSET_ERROR, "Expecting callback function but got %v", jsCB)
#define EXPECT_OPT_EXCEPTION(jsOPT) jsExceptionHere(JSET_ERROR, "Expecting Object, got %t", jsOPT)


static const char *TAG = "wifi";

//===== mDNS
static bool mdns_started = 0;

void stopMDNS() {
  jsDebug(DBG_INFO, "Wifi:stopMDNS\n");
  if (mdns_started) {
    mdns_free();
    mdns_started = false;
  }
}

void startMDNS(const char *hostname) {
  jsDebug(DBG_INFO, "Wifi:startMDNS - %s\n", hostname);
  if (mdns_started) stopMDNS();
  esp_err_t err;
  err = mdns_init();
  if (err != ESP_OK) {
    jsDebug(DBG_ERROR, "mDNS init failed: %d\n", err);
    return;
  }
  err = mdns_hostname_set(hostname);
  if (err != ESP_OK) {
    jsDebug(DBG_ERROR, "mDNS hostname_set failed: %d\n", err);
    return;
  }
  err = mdns_service_add(NULL, "_telnet", "_tcp", 23, NULL, 0);
  if (err != ESP_OK) {
    jsDebug(DBG_ERROR, "mDNS service_add failed: %d\n", err);
    return;
  }
  mdns_started = true;
}

// Convert an wifi_auth_mode_t data type to a string value.
static char *authModeToString(wifi_auth_mode_t authMode) {
  switch(authMode) {
    case WIFI_AUTH_OPEN: return "open";
    case WIFI_AUTH_WEP: return "wep";
    case WIFI_AUTH_WPA_PSK: return "wpa";
    case WIFI_AUTH_WPA2_PSK: return "wpa2";
    case WIFI_AUTH_WPA_WPA2_PSK: return "wpa_wpa2";
    default:  return "unknown";
  }
} // End of authModeToString

//Convert an wifi_cipher_type_t data type to a string value.

/**
 * Convert an wifi_cipher_type_t data type to a string value.
 *
static char *cipherTypeToString(wifi_cipher_type_t cipherType) {

  switch(cipherType) {
  case WIFI_CIPHER_TYPE_NONE:
    return "NONE";
  case WIFI_CIPHER_TYPE_WEP40:
    return "WEP40";
  case WIFI_CIPHER_TYPE_WEP104:
    return "WEP104";
  case WIFI_CIPHER_TYPE_TKIP:
    return "TKIP";
  case WIFI_CIPHER_TYPE_CCMP:
    return "CCMP";
  case WIFI_CIPHER_TYPE_TKIP_CCMP:
    return "TKIP+CCMP";
  }
  return "unknown";
} // End of authModeToString
*/

/**
 * check esp function
*/

/**
 * Convert an wifi_second_chan_t data type to a string value.
 */
static char *htModeToString(wifi_second_chan_t htMode) {
  switch(htMode) {
    case WIFI_SECOND_CHAN_NONE: return "HT20";
    case WIFI_SECOND_CHAN_ABOVE: return "HT40+";
    case WIFI_SECOND_CHAN_BELOW: return "HT40-";
    default: return "unknown";
  }
} // End of htModeToString

// Convert a Wifi reason code to a string representation.
const char *wifiReasonToString(int r) {
  switch (r) {
    case WIFI_REASON_UNSPECIFIED: return "UNSPECIFIED";
    case WIFI_REASON_AUTH_EXPIRE: return "AUTH_EXPIRE";
    case WIFI_REASON_AUTH_LEAVE: return "AUTH_LEAVE";
    case WIFI_REASON_ASSOC_EXPIRE: return "ASSOC_EXPIRE";
    case WIFI_REASON_ASSOC_TOOMANY: return "ASSOC_TOOMANY";
    case WIFI_REASON_NOT_AUTHED: return "NOT_AUTHED";
    case WIFI_REASON_NOT_ASSOCED: return "NOT_ASSOCED";
    case WIFI_REASON_ASSOC_LEAVE: return "ASSOC_LEAVE";
    case WIFI_REASON_ASSOC_NOT_AUTHED: return "ASSOC_NOT_AUTHED";
    case WIFI_REASON_DISASSOC_PWRCAP_BAD: return "DISASSOC_PWRCAP_BAD";
    case WIFI_REASON_DISASSOC_SUPCHAN_BAD: return "DISASSOC_SUPCHAN_BAD";
    case WIFI_REASON_IE_INVALID: return "IE_INVALID";
    case WIFI_REASON_MIC_FAILURE: return "MIC_FAILURE";
    case WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT: return "4WAY_HANDSHAKE_TIMEOUT";
    case WIFI_REASON_GROUP_KEY_UPDATE_TIMEOUT: return "GROUP_KEY_UPDATE_TIMEOUT";
    case WIFI_REASON_IE_IN_4WAY_DIFFERS: return "IE_IN_4WAY_DIFFERS";
    case WIFI_REASON_GROUP_CIPHER_INVALID: return "GROUP_CIPHER_INVALID";
    case WIFI_REASON_PAIRWISE_CIPHER_INVALID: return "PAIRWISE_CIPHER_INVALID";
    case WIFI_REASON_AKMP_INVALID: return "AKMP_INVALID";
    case WIFI_REASON_UNSUPP_RSN_IE_VERSION: return "UNSUPP_RSN_IE_VERSION";
    case WIFI_REASON_INVALID_RSN_IE_CAP: return "INVALID_RSN_IE_CAP";
    case WIFI_REASON_802_1X_AUTH_FAILED: return "802_1X_AUTH_FAILED";
    case WIFI_REASON_CIPHER_SUITE_REJECTED: return "CIPHER_SUITE_REJECTED";
    case WIFI_REASON_BEACON_TIMEOUT: return "BEACON_TIMEOUT";
    case WIFI_REASON_NO_AP_FOUND: return "NO_AP_FOUND";
    case WIFI_REASON_HANDSHAKE_TIMEOUT: return "HANDSHAKE_TIMEOUT";
    case WIFI_REASON_AUTH_FAIL: return "AUTH_FAIL";
    default: return "Unknown reason";
  }
}
/**
 * Convery a wifi_mode_t data type to a string value.
 */
static char *wifiModeToString(wifi_mode_t mode) {
  switch(mode) {
    case WIFI_MODE_NULL: return "NULL";
    case WIFI_MODE_STA: return "STA";
    case WIFI_MODE_AP: return "AP";
    case WIFI_MODE_APSTA: return "APSTA";
    default: return "UNKNOWN";
  }
} // End of wifiModeToString

#if ESP_IDF_VERSION_MAJOR>=5
// Convert an wifi event to a string value.
const char *wifiEventToString(int e) {
  switch (e) {
    case WIFI_EVENT_STA_CONNECTED: return "STA_CONNECTED";
    case WIFI_EVENT_STA_DISCONNECTED: return "STA_DISCONNECTED";
    case WIFI_EVENT_STA_AUTHMODE_CHANGE: return "STA_AUTHMODE_CHANGE";
    case WIFI_EVENT_AP_STACONNECTED: return "AP_STACONNECTED";
    case WIFI_EVENT_AP_STADISCONNECTED: return "AP_STADISCONNECTED";
    case WIFI_EVENT_AP_PROBEREQRECVED: return "AP_PROBEREQRECVED";
    case WIFI_EVENT_WIFI_READY: return "WIFI_READY";
    case WIFI_EVENT_SCAN_DONE: return "SCAN_DONE";
    case WIFI_EVENT_STA_START: return "STA_START";
    case WIFI_EVENT_STA_STOP: return "STA_STOP";
    case WIFI_EVENT_STA_WPS_ER_SUCCESS: return "STA_WPS_ER_SUCCESS";
    case WIFI_EVENT_STA_WPS_ER_FAILED: return "STA_WPS_ER_FAILED";
    case WIFI_EVENT_STA_WPS_ER_TIMEOUT: return "STA_WPS_ER_TIMEOUT";
    case WIFI_EVENT_STA_WPS_ER_PIN: return "STA_WPS_ER_PIN";
    case WIFI_EVENT_AP_START: return "AP_START";
    case WIFI_EVENT_AP_STOP: return "AP_STOP";
    case WIFI_EVENT_MAX: return "MAX";
    case WIFI_REASON_IE_IN_4WAY_DIFFERS: return "IEs in 4-way handshake differ";
    default: return "unknown";
  }
}
const char *ipEventToString(int e) {
  switch (e) {
    case IP_EVENT_STA_GOT_IP: return "STA_GOT_IP";
    case IP_EVENT_STA_LOST_IP: return "LOST_IP";
    case IP_EVENT_GOT_IP6: return "GOT_IP6";
    case IP_EVENT_ETH_GOT_IP: return "ETH_GOT_IP";
    default: return "unknown";
  }
}
#else
/**
 * Convert an wifi event to a string value.
 */
static char *wifiEventToString(uint32_t event){
  jsDebug(DBG_INFO, "wifiReasonToString %d\n",event);
  switch(event){
    case SYSTEM_EVENT_STA_CONNECTED:return "STA_CONNECTED";
    case SYSTEM_EVENT_STA_DISCONNECTED:return "STA_DISCONNECTED";
    case SYSTEM_EVENT_STA_AUTHMODE_CHANGE:return "STA_AUTHMODE_CHANGE";
    case SYSTEM_EVENT_STA_GOT_IP:return "STA_GOT_IP";
    case SYSTEM_EVENT_AP_STACONNECTED:return "AP_STACONNECTED";
    case SYSTEM_EVENT_AP_STADISCONNECTED: return "AP_STADISCONNECTED";
    case SYSTEM_EVENT_AP_PROBEREQRECVED:return "AP_PROBEREQRECVED";
    case SYSTEM_EVENT_WIFI_READY: return "WIFI_READY";
    case SYSTEM_EVENT_SCAN_DONE: return "SCAN_DONE";
    case SYSTEM_EVENT_STA_START: return "STA_START";
    case SYSTEM_EVENT_STA_STOP: return "STA_STOP";
    case SYSTEM_EVENT_STA_LOST_IP: return "LOST_IP";
    case SYSTEM_EVENT_STA_WPS_ER_SUCCESS: return "STA_WPS_ER_SUCCESS";
    case SYSTEM_EVENT_STA_WPS_ER_FAILED: return "STA_WPS_ER_FAILED";
    case SYSTEM_EVENT_STA_WPS_ER_TIMEOUT: return "STA_WPS_ER_TIMEOUT";
    case SYSTEM_EVENT_STA_WPS_ER_PIN: return "STA_WPS_ER_PIN";
    case SYSTEM_EVENT_AP_START: return "AP_START";
    case SYSTEM_EVENT_AP_STOP: return "AP_STOP";
    case SYSTEM_EVENT_GOT_IP6: return "GOT_IP6";
    case SYSTEM_EVENT_ETH_START: return "ETH_START";
    case SYSTEM_EVENT_ETH_STOP: return "ETH_STOP";
    case SYSTEM_EVENT_ETH_CONNECTED: return "ETH_CONNECTED";
    case SYSTEM_EVENT_ETH_DISCONNECTED: return "ETH_DISCONNECTED";
    case SYSTEM_EVENT_ETH_GOT_IP: return "ETH_GOT_IP";
    case SYSTEM_EVENT_MAX: return "MAX";
    default: return "unknown event, see wifiEventToString";
  }
}
#endif

/**
 * convert WiFi error to a string value.
 */
static char *wifiErrorToString(esp_err_t err){
  jsDebug(DBG_INFO, "wifiErrorToString %d: %s \n", err,esp_err_to_name(err));
  return (char *) esp_err_to_name(err);
}

/**
 * Callback function that is invoked at the culmination of a scan.
 */
static void scanCB() {
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

  if (g_jsScanCallback == NULL) {
    return;
  }

  uint16_t apCount;
  ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&apCount));

  JsVar *jsAccessPointArray = jsvNewArray(NULL, 0);
  if (apCount > 0) {
    wifi_ap_record_t *list = (wifi_ap_record_t *)malloc(sizeof(wifi_ap_record_t) * apCount);
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&apCount, list));

    int i;
    for (i=0; i<apCount; i++) {
      JsVar *jsCurrentAccessPoint = jsvNewObject();
      jsvObjectSetIntChild(jsCurrentAccessPoint, "rssi", list[i].rssi);
      jsvObjectSetStringChild(jsCurrentAccessPoint, "authMode", authModeToString(list[i].authmode));

      // The SSID may **NOT** be NULL terminated ... so handle that.
      char temp[32 + 1];
      strncpy((char *)temp, list[i].ssid, 32);
      temp[32] = '\0';
      jsvObjectSetStringChild(jsCurrentAccessPoint, "ssid", temp);
      sprintf(temp, MACSTR, MAC2STR(list[i].bssid));
      jsvObjectSetStringChild(jsCurrentAccessPoint, "mac", temp);
      sprintf(temp, "%d", list[i].primary);
      jsvObjectSetStringChild(jsCurrentAccessPoint, "channel", temp);
      // Can't find a flag for this?  http://esp-idf.readthedocs.io/en/latest/api-reference/wifi/esp_wifi.html?highlight=wifi_ap_record_t
      //jsvObjectSetBoolChild(jsCurrentAccessPoint, "isHidden", list[i].ssid_hidden);
      // Add the new record to the array
      jsvArrayPush(jsAccessPointArray, jsCurrentAccessPoint);
      jsvUnLock(jsCurrentAccessPoint);
    } // End of loop over each access point
    free(list);
  } // Number of access points > 0

  // We have now completed the scan callback, so now we can invoke the JS callback.
  JsVar *params[] = {jsAccessPointArray};
  jsiQueueEvents(NULL, g_jsScanCallback, params, 1);

  jsvUnLock(jsAccessPointArray);
  jsvUnLock(g_jsScanCallback);
  g_jsScanCallback = NULL;
} // End of scanCB


/** Get the global object for the Wifi library/module, this is used in order to send the
 * "on event" callbacks to the handlers.
 */
static JsVar *getWifiModule() {
  JsVar *moduleName = jsvNewFromString("Wifi");
  JsVar *m = jswrap_require(moduleName);
  jsvUnLock(moduleName);
  return m;
} // End of getWifiModule

/**
 * Given an ESP32 WiFi event type, determine the corresponding
 * event handler name we should publish upon.  For example, if we
 * have an event of type "SYSTEM_EVENT_STA_CONNECTED" then we wish
 * to publish an event upon "#onassociated".  The implementation
 * here is a simple switch as at this time we don't want to assume
 * anything about the values of event types (i.e. whether they are small
 * and sequential).  If we could make assumptions about the event
 * types we may have been able to use a lookup array.
 *
 * The mappings are:
 * SYSTEM_EVENT_AP_PROBEREQRECVED   - #onprobe_recv
 * SYSTEM_EVENT_AP_STACONNECTED     - #onsta_joined
 * SYSTEM_EVENT_AP_STADISCONNECTED  - #onsta_left
 * SYSTEM_EVENT_STA_AUTHMODE_CHANGE - #onauth_change
 * SYSTEM_EVENT_STA_CONNECTED       - #onassociated
 * SYSTEM_EVENT_STA_DISCONNECTED    - #ondisconnected
 * SYSTEM_EVENT_STA_GOT_IP          - #onconnected
 *
 * See also:
 * * event_handler()
 *
 */

static int s_retry_num = 0;

#if ESP_IDF_VERSION_MAJOR>=5
static char *ipGetEvent(uint32_t event) {
  jsDebug(DBG_INFO,"ipGetEvent: Got event: %d\n", event);

  switch(event) {
    case IP_EVENT_STA_GOT_IP: jsDebug(DBG_INFO,"%d : #onconnected\n", event);
                              return "#onconnected";
  }
  //jsDebug(DBG_INFO, "Unhandled wifi event type: %d\n", event);
  jsDebug(DBG_INFO, "Unhandled ip event type: %d\n", event);
  return NULL;
}
#endif

#if ESP_IDF_VERSION_MAJOR>=5
const char *getEventToString(int e) {
  switch (e) {
    case WIFI_EVENT_WIFI_READY: return "#onwifi_ready";
    case WIFI_EVENT_SCAN_DONE: return "#onscan_done";
    case WIFI_EVENT_STA_START: return "#onsta_start";
    case WIFI_EVENT_STA_STOP: return "#onsta_stop";
    case WIFI_EVENT_STA_CONNECTED: return "#onassociated";
    case WIFI_EVENT_STA_DISCONNECTED: return "#ondisconnected";
    case WIFI_EVENT_STA_AUTHMODE_CHANGE: return "#onauth_change";
    case WIFI_EVENT_AP_START: return "#onap_start";
    case WIFI_EVENT_AP_STOP: return "#onap_stop";
    case WIFI_EVENT_AP_STACONNECTED: return "#onsta_joined";
    case WIFI_EVENT_AP_STADISCONNECTED: return "#onsta_left";
    case WIFI_EVENT_AP_PROBEREQRECVED: return "#onprobe_recv";
    default: return "unknown";
  }
}
#else
const char *getEventToString(int e) {
  switch (e) {
    // no handler name; treat as invalid / no script
    case SYSTEM_EVENT_WIFI_READY: return NULL;
    case SYSTEM_EVENT_SCAN_DONE: return NULL;
    case SYSTEM_EVENT_STA_START: return "#onsta_start";
    case SYSTEM_EVENT_STA_STOP: return NULL;
    case SYSTEM_EVENT_STA_CONNECTED: return "#onassociated";
    case SYSTEM_EVENT_STA_DISCONNECTED: return "#ondisconnected";
    case SYSTEM_EVENT_STA_GOT_IP: return "#onconnected";
    case SYSTEM_EVENT_STA_AUTHMODE_CHANGE: return "#onauth_change";
    case SYSTEM_EVENT_AP_START: return NULL;
    case SYSTEM_EVENT_AP_STOP: return NULL;
    case SYSTEM_EVENT_AP_STACONNECTED: return "#onsta_joined";
    case SYSTEM_EVENT_AP_STADISCONNECTED: return "#onsta_left";
    case SYSTEM_EVENT_AP_PROBEREQRECVED: return "#onprobe_recv";
    default: return "unknown";
  }
}
#endif

#if ESP_IDF_VERSION_MAJOR>=5
/**
 * Invoke the JavaScript callback to notify the program that an ESP32
 * WiFi event has occurred.
 */
static void sendIPEvent(
    uint32_t eventType, //!< The ESP32 WiFi event type.
    JsVar *jsDetails  //!< The JS object to be passed as a parameter to the callback.
) {

  JsVar *module = getWifiModule();
  if (!module) {
    jsDebug(DBG_INFO, "!module -> return\n");
    return; // out of memory?
  }

  // get event name as string and compose param list
  JsVar *params[1];
  params[0] = jsDetails;
  char *eventName = ipGetEvent(eventType);
  if (eventName == NULL) {
    jsDebug(DBG_INFO, "eventName == NULL -> return\n");
    return;
  }
  //jsDebug(DBG_INFO, "sendIPEvent %s\n", eventName);
  jsDebug(DBG_INFO, "sendIPEvent %s\n", eventName);
  jsiQueueObjectCallbacks(module, eventName, params, 1);
  jsvUnLock(module);
  return;
}
#endif

/**
 * Invoke the JavaScript callback to notify the program that an ESP8266
 * WiFi event has occurred.
 */
static void sendWifiEvent(
    uint32_t eventType, //!< The ESP32 WiFi event type.
    JsVar *jsDetails  //!< The JS object to be passed as a parameter to the callback.
) {
  JsVar *module = getWifiModule();
  if (!module) {
    return; // out of memory?
  }

  // get event name as string and compose param list
  JsVar *params[1];
  params[0] = jsDetails;
  const char *eventName = getEventToString(eventType);
  if (eventName == NULL) {
    return;
  }
  jsDebug(DBG_INFO, "sendWifiEvent %s\n", eventName);
  jsiQueueObjectCallbacks(module, eventName, params, 1);
  jsvUnLock(module);
  return;
}

/** Called when we can an AP or Station disconnect event. If
 * we don't seem to be connected to anything, stop wifi*/
void stopWifiIfIdle() {
  wifi_mode_t mode;
  esp_wifi_get_mode(&mode);
  if ((mode == WIFI_MODE_NULL ||
      (mode == WIFI_MODE_STA && !g_isStaConnected)) &&
      !g_jsScanCallback/*scanning*/) {
    jsDebug(DBG_INFO, "stopWifiIfIdle: calling esp_wifi_stop()\n");
    esp_wifi_stop();
  }
}

/**
 * Wifi event handler
 * Here we get invoked whenever a WiFi event is received from the ESP32 WiFi
 */
#if ESP_IDF_VERSION_MAJOR>=5
static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {

  //jsDebug(DBG_INFO, "Event: %s (%d)\n", event_base == WIFI_EVENT ? wifiEventToString(event_id) : "OTHER", event_id);
  jsDebug(DBG_INFO, "Event: %s (%d)\n", event_base == WIFI_EVENT ? wifiEventToString(event_id) : "OTHER", event_id);

  if (event_base == WIFI_EVENT && event_id >= WIFI_EVENT_MAX) {
    jsDebug(DBG_INFO, "Internal event ");
  }
  else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
    wifi_event_sta_disconnected_t* disconnected = (wifi_event_sta_disconnected_t*)event_data;
    if (--s_retry_num > 0) {
      esp_wifi_connect();
      jsDebug(DBG_INFO, "retry to AP connect\n");
      return;
    }
    g_isStaConnected = false;
    g_lastEventStaDisconnected.ssid_len = disconnected->ssid_len;
    memcpy(g_lastEventStaDisconnected.ssid, disconnected->ssid, 32);
    memcpy(g_lastEventStaDisconnected.bssid, disconnected->bssid, 6);
    g_lastEventStaDisconnected.reason = disconnected->reason;

    // JS callback logic unchanged
    if (jsvIsFunction(g_jsDisconnectCallback)) {
      jsiQueueEvents(NULL, g_jsDisconnectCallback, NULL, 0);
      jsvUnLock(g_jsDisconnectCallback);
      g_jsDisconnectCallback = NULL;
    }
    JsVar *jsDetails = jsvNewObject();
    char temp[33];
    memcpy(temp, disconnected->ssid, 32); temp[32] = '\0';
    jsvObjectSetChildAndUnLock(jsDetails, "ssid", jsvNewFromString(temp));
    sprintf(temp, MACSTR, MAC2STR(disconnected->bssid));
    jsvObjectSetChildAndUnLock(jsDetails, "mac", jsvNewFromString(temp));
    sprintf(temp, "%d", disconnected->reason);
    jsvObjectSetChildAndUnLock(jsDetails, "reason", jsvNewFromString(temp));
    jsvObjectSetChildAndUnLock(jsDetails, "msg", jsvNewFromString(wifiReasonToString(disconnected->reason)));
    sendWifiEvent(WIFI_EVENT_STA_DISCONNECTED, jsDetails);  // Update event_id if needed
  } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED) {
    wifi_event_sta_connected_t* connected = (wifi_event_sta_connected_t*)event_data;
    g_isStaConnected = true;
    g_lastEventStaConnected.ssid_len = connected->ssid_len;
    memcpy(g_lastEventStaConnected.ssid, connected->ssid, 32);
    memcpy(g_lastEventStaConnected.bssid, connected->bssid, 6);
    g_lastEventStaConnected.channel = connected->channel;
    g_lastEventStaConnected.authmode = connected->authmode;

    JsVar *jsDetails = jsvNewObject();
    char temp[33];
    memcpy(temp, connected->ssid, 32); temp[32] = '\0';
    jsvObjectSetChildAndUnLock(jsDetails, "ssid", jsvNewFromString(temp));
    sprintf(temp, MACSTR, MAC2STR(connected->bssid));
    jsvObjectSetChildAndUnLock(jsDetails, "mac", jsvNewFromString(temp));
    sprintf(temp, "%d", connected->channel);
    jsvObjectSetChildAndUnLock(jsDetails, "channel", jsvNewFromString(temp));
    sendWifiEvent(WIFI_EVENT_STA_CONNECTED, jsDetails);
  } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STACONNECTED) {
    wifi_event_ap_staconnected_t* sta_connected = (wifi_event_ap_staconnected_t*)event_data;
    JsVar *jsDetails = jsvNewObject();
    char temp[18];
    sprintf(temp, MACSTR, MAC2STR(sta_connected->mac));
    jsvObjectSetChildAndUnLock(jsDetails, "mac", jsvNewFromString(temp));
    sendWifiEvent(WIFI_EVENT_AP_STACONNECTED, jsDetails);
  } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STADISCONNECTED) {
    wifi_event_ap_stadisconnected_t* sta_disconnected = (wifi_event_ap_stadisconnected_t*)event_data;
    JsVar *jsDetails = jsvNewObject();
    char temp[18];
    sprintf(temp, MACSTR, MAC2STR(sta_disconnected->mac));
    jsvObjectSetChildAndUnLock(jsDetails, "mac", jsvNewFromString(temp));
    sendWifiEvent(WIFI_EVENT_AP_STADISCONNECTED, jsDetails);
  } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
    ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
    sendWifiCompletionCB(&g_jsGotIpCallback, NULL);
    JsVar *jsDetails = jsvNewObject();
    char temp[16];
    sprintf(temp, IPSTR, IP2STR(&event->ip_info.ip));
    jsvObjectSetChildAndUnLock(jsDetails, "ip", jsvNewFromString(temp));
    sprintf(temp, IPSTR, IP2STR(&event->ip_info.netmask));
    jsvObjectSetChildAndUnLock(jsDetails, "netmask", jsvNewFromString(temp));
    sprintf(temp, IPSTR, IP2STR(&event->ip_info.gw));
    jsvObjectSetChildAndUnLock(jsDetails, "gw", jsvNewFromString(temp));
    jsDebug(DBG_INFO, "Wifi: About to emit connect!\n");
    sendIPEvent(IP_EVENT_STA_GOT_IP, jsDetails);
    // mDNS unchanged
    const char *hostname;
    esp_err_t err = esp_netif_get_hostname(sta_netif, &hostname);
    if (err == ESP_OK && hostname && hostname[0]) startMDNS(hostname);
  } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_SCAN_DONE) {
    scanCB();
    sendWifiEvent(WIFI_EVENT_SCAN_DONE, NULL);
  }
}
#else
/**
 * Wifi event handler
 * Here we get invoked whenever a WiFi event is received from the ESP32 WiFi
 * subsystem.  The events include:
 * * SYSTEM_EVENT_STA_DISCONNECTED - As a station, we were disconnected.
 */
static esp_err_t event_handler(void *ctx, system_event_t *event)
{
  UNUSED(ctx);
  /*
   * SYSTEM_EVENT_STA_DISCONNECT
   * Structure contains:
   * * ssid
   * * ssid_len
   * * bssid
   * * reason
   */
  jsDebug(DBG_INFO, "Wifi: Event(%d):SYSTEM_EVENT_%s\n",event->event_id,wifiEventToString(event->event_id));

  if (event->event_id == SYSTEM_EVENT_STA_DISCONNECTED) {
    if (--s_retry_num > 0 ) {
      esp_wifi_connect();
      jsDebug(DBG_INFO,"retry to AP connect\n");
      return ESP_OK;
    }
    g_isStaConnected = false; // Flag as disconnected
    g_lastEventStaDisconnected = event->event_info.disconnected; // Save the last disconnected info

    if (jsvIsFunction(g_jsDisconnectCallback)) {
      jsiQueueEvents(NULL, g_jsDisconnectCallback, NULL, 0);
      jsvUnLock(g_jsDisconnectCallback);
      g_jsDisconnectCallback = NULL;
    }
    JsVar *jsDetails = jsvNewObject();

    char temp[33];
    memcpy(temp, event->event_info.disconnected.ssid, 32);
    temp[32] = '\0';
    jsvObjectSetStringChild(jsDetails, "ssid", temp);
    sprintf(temp, MACSTR, MAC2STR(event->event_info.disconnected.bssid));
    jsvObjectSetStringChild(jsDetails, "mac", temp);
    sprintf(temp, "%d", event->event_info.disconnected.reason);
    jsvObjectSetStringChild(jsDetails, "reason", temp);
    jsvObjectSetStringChild(jsDetails, "msg", wifiReasonToString(event->event_info.disconnected.reason));
    sendWifiEvent(event->event_id, jsDetails);

    if (g_jsGotIpCallback) {
      /* If we were connecting, g_jsGotIpCallback is set. If we fail (eg this event is fired) then we
      should call the callback with the first argument as the error */
      sendWifiCompletionCB(&g_jsGotIpCallback, wifiReasonToString(event->event_info.disconnected.reason));
    }

    stopWifiIfIdle();
    return ESP_OK;
  } // End of handle SYSTEM_EVENT_STA_DISCONNECTED

  /**
   * SYSTEM_EVENT_STA_CONNECTED
   * Structure contains:
   * * ssid
   * * ssid_len
   * * bssid
   * * channel
   * * authmode
   */
  if (event->event_id == SYSTEM_EVENT_STA_CONNECTED) {
    g_isStaConnected = true; // Flag us as connected.
    g_lastEventStaConnected = event->event_info.connected; // Save the last connected info

    // Publish the on("associated") event to any one who has registered
    // an interest.
    JsVar *jsDetails = jsvNewObject();

    char temp[33];
    memcpy(temp, event->event_info.connected.ssid, 32);
    temp[32] = '\0';
    jsvObjectSetStringChild(jsDetails, "ssid", temp);
    sprintf(temp, MACSTR, MAC2STR(event->event_info.connected.bssid));
    jsvObjectSetStringChild(jsDetails, "mac", temp);
    sprintf(temp, "%d", event->event_info.connected.channel);
    jsvObjectSetStringChild(jsDetails, "channel", temp);
    sendWifiEvent(event->event_id, jsDetails);
    return ESP_OK;
  } // End of handle SYSTEM_EVENT_STA_CONNECTED

  if (event->event_id == SYSTEM_EVENT_STA_START) {
    // Perform an esp_wifi_connect
    esp_err_t err = esp_wifi_connect();
    if (err != ESP_OK) {
      jsDebug(DBG_INFO, "Wifi: event_handler STA_START: esp_wifi_connect: %d(%s)\n", err,wifiErrorToString(err));
      return ESP_OK;
    }
    return ESP_OK;
  }

  /**
   * SYSTEM_EVENT_STA_GOT_IP
   * Structure contains:
   * * ipinfo.ip
   * * ipinfo.netmask
   * * ip_info.gw
   */
  if (event->event_id == SYSTEM_EVENT_STA_GOT_IP) {
    sendWifiCompletionCB(&g_jsGotIpCallback, NULL);
    JsVar *jsDetails = jsvNewObject();

    // 123456789012345_6
    // xxx.xxx.xxx.xxx\0
    char temp[16];
    sprintf(temp, IPSTR, IP2STR(&event->event_info.got_ip.ip_info.ip));
    jsvObjectSetStringChild(jsDetails, "ip", temp);
    sprintf(temp, IPSTR, IP2STR(&event->event_info.got_ip.ip_info.netmask));
    jsvObjectSetStringChild(jsDetails, "netmask", temp);
    sprintf(temp, IPSTR, IP2STR(&event->event_info.got_ip.ip_info.gw));
    jsvObjectSetStringChild(jsDetails, "gw", temp);
    jsDebug(DBG_INFO, "Wifi: About to emit connect!\n");
    sendWifiEvent(event->event_id, jsDetails);
    // start mDNS
    const char * hostname;
#if ESP_IDF_VERSION_MAJOR>=5
    esp_err_t err = esp_netif_get_hostname(sta_netif, &hostname);
#else
    esp_err_t err = tcpip_adapter_get_hostname(TCPIP_ADAPTER_IF_STA, &hostname);
#endif
    if (hostname && hostname[0] != 0) {
      startMDNS(hostname);
    }
    return ESP_OK;
  } // End of handle SYSTEM_EVENT_STA_GOT_IP

  /**
   * SYSTEM_EVENT_AP_STACONNECTED
   * Structure contains:
   * * mac
   * * aid
   */
  if (event->event_id == SYSTEM_EVENT_AP_STACONNECTED) {
    JsVar *jsDetails = jsvNewObject();
    // 12345678901234567_8
    // xx:xx:xx:xx:xx:xx\0
    char temp[18];
    sprintf(temp, MACSTR, MAC2STR(event->event_info.sta_connected.mac));
    jsvObjectSetStringChild(jsDetails, "mac", temp);
    sendWifiEvent(event->event_id, jsDetails);
    return ESP_OK;
  }

  /**
   * SYSTEM_EVENT_AP_STADISCONNECTED
   * Structure contains:
   * * mac
   * * aid
   */
  if (event->event_id == SYSTEM_EVENT_AP_STADISCONNECTED) {
    JsVar *jsDetails = jsvNewObject();
    // 12345678901234567_8
    // xx:xx:xx:xx:xx:xx\0
    char temp[18];
    sprintf(temp, MACSTR, MAC2STR(event->event_info.sta_disconnected.mac));
    jsvObjectSetStringChild(jsDetails, "mac", temp);
    sendWifiEvent(event->event_id, jsDetails);
    return ESP_OK;
  }

  /**
   * SYSTEM_EVENT_SCAN_DONE
   * Called when a previous network scan has completed.  Here we check to see if we have
   * a registered callback that is interested in being called when a scan has completed
   * and, if we do, we build the parameters for that callback and then invoke it.
   */
  if (event->event_id == SYSTEM_EVENT_SCAN_DONE) {
    scanCB();
    stopWifiIfIdle();
    return ESP_OK;
  }

  /**
   * SYSTEM_EVENT_AP_START
   * Called when we have started being an access point.
   */
  if (event->event_id == SYSTEM_EVENT_AP_START) {
    g_isAPStarted = true;
    sendWifiCompletionCB(&g_jsAPStartedCallback, NULL);
    return ESP_OK;
  }
  /**
   * SYSTEM_EVENT_AP_STOP
   * Called when we have stopped being an access point.
   */
  if (event->event_id == SYSTEM_EVENT_AP_STOP) {
    g_isAPStarted = false;
    stopWifiIfIdle();
  }

  jsDebug(DBG_INFO, "Wifi: event_handler -> NOT HANDLED EVENT: %d\n", event->event_id );
  return ESP_OK;
} // End of event_handler
#endif

/**
 * Initialize the one time ESP32 wifi components including the event
 * handler.
 */
void esp32_wifi_init() {
#if ESP_IDF_VERSION_MAJOR>=5
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  esp_netif_create_default_wifi_ap();
  esp_netif_create_default_wifi_sta();
#endif

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

#if ESP_IDF_VERSION_MAJOR>=5
  ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, &instance_wifi));
  ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, &instance_ip));
  //ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_LOST_IP, &wifi_event_handler, NULL, &instance_lost_ip));
  //ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_AP_STAIPASSIGNED, &wifi_event_handler, NULL, &instance_ap_ip));

#else
  tcpip_adapter_init();
  ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));
  ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
#endif

  // 3. CRITICAL: Set mode + start WiFi BEFORE handlers
  //ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
  //ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
  ESP_ERROR_CHECK(esp_wifi_start());

}

/**
 * Some of the WiFi functions have a completion callback which is of the form:
 * function(err) { ... }
 * These callbacks are called with an error string (if an error is encountered) or null if there is
 * no error.  Since this occurrence happens a number of times, this helper function takes as input
 * a pointer to a callback function and a parameter.
 */
static void sendWifiCompletionCB(
    JsVar **g_jsCallback, //!< Pointer to the global callback variable
    char *reason          //!< NULL if successful, error string otherwise
) {
  jsDebug(DBG_INFO, "sendWifiCompletionCB\n");
  // Check that we have a callback function.
  if (!jsvIsFunction(*g_jsCallback)) {
    return; // we have not got a function pointer: nothing to do
  }

  JsVar *params[1];
  params[0] = reason ? jsvNewFromString(reason) : jsvNewNull();
  jsiQueueEvents(NULL, *g_jsCallback, params, 1);
  jsvUnLock(params[0]);
  // unlock and delete the global callback
  jsvUnLock(*g_jsCallback);
  *g_jsCallback = NULL;
} // End of sendWifiCompletionCB

/*JSON{
  "type":"init",
  "generate":"jswrap_esp32_wifi_soft_init"
}
*/

/**
 * Perform a soft initialization of ESP32 networking.
 */
void jswrap_esp32_wifi_soft_init() {
  //jsDebug(DBG_INFO, "jswrap_esp32_wifi_soft_init()\n");
  JsNetwork net;
  networkCreate(&net, JSNETWORKTYPE_ESP32); // Set the network type to be ESP32
  networkState = NETWORKSTATE_ONLINE; // Set the global state of the networking to be online
}

void jswrap_wifi_disconnect(JsVar *jsCallback) {
  // We save the callback function so that it can subsequently invoked.  Then we execute the
  // ESP-IDF function to disconnect us from the access point.  The thinking is that will result
  // in a subsequent event which we will detect and use to call the callback.
  //
  esp_err_t err;
  // Free any existing callback, then register new callback
  if (g_jsDisconnectCallback != NULL) {
    jsvUnLock(g_jsDisconnectCallback);
  }
  g_jsDisconnectCallback = NULL;

  // Check that the callback is a good callback if supplied.
  if (jsCallback != NULL && !jsvIsUndefined(jsCallback) && !jsvIsFunction(jsCallback)) {
    EXPECT_CB_EXCEPTION(jsCallback);
    return;
  }

  // Save the callback for later execution.
  g_jsDisconnectCallback = jsvLockAgainSafe(jsCallback);

  // Call the ESP-IDF to disconnect us from the access point.
  jsDebug(DBG_INFO, "Disconnecting.....\n");

  // turn off auto-connect
#if !(ESP_IDF_VERSION_MAJOR>=4)
  esp_wifi_set_auto_connect(false);
#endif
  s_retry_num = 0; // flag so we don't attempt to reconnect
  err = esp_wifi_disconnect();
  if (err != ESP_OK) {
    jsDebug(DBG_INFO, "jswrap_wifi_disconnect: esp_wifi_disconnect rc=%d(%s)\n", err,wifiErrorToString(err));
  }
  if (jsvIsFunction(jsCallback)) {
    jsiQueueEvents(NULL, jsCallback, NULL, 0);
  }
} // End of jswrap_wifi_disconnect

void jswrap_wifi_stopAP(JsVar *jsCallback) {
  // handle the callback parameter
  if (jsCallback != NULL && !jsvIsUndefined(jsCallback) && !jsvIsFunction(jsCallback)) {
    EXPECT_CB_EXCEPTION(jsCallback);
    return;
  }

  // Change operating mode intelligently.  We want to remove us from being
  // an access point but if we are also a station, we want to preserve that.
  esp_err_t err;
  wifi_mode_t mode;
  err = esp_wifi_get_mode(&mode);
  switch(mode) {
    case WIFI_MODE_NULL:
    case WIFI_MODE_STA:
      break;
    case WIFI_MODE_AP:
      mode = WIFI_MODE_NULL;
      break;
    case WIFI_MODE_APSTA:
      mode = WIFI_MODE_STA;
      break;
    default:
      break;
  }
  err = esp_wifi_set_mode(mode);
  if (err != ESP_OK) {
    jsDebug(DBG_INFO, "jswrap_wifi_stopAP: esp_wifi_set_mode rc=%d(%s)\n", err,wifiErrorToString(err));
  }

  if (jsvIsFunction(jsCallback)) {
    jsiQueueEvents(NULL, jsCallback, NULL, 0);
  }
} // End of jswrap_wifi_stopAP

void jswrap_wifi_connect(
    JsVar *jsSsid,
    JsVar *jsOptions,
    JsVar *jsCallback
  ) {

  jsDebug(DBG_INFO, "jswrap_wifi_connect: entry\n");

  // Check that the ssid value isn't obviously in error.
  if (!jsvIsString(jsSsid)) {
    jsExceptionHere(JSET_ERROR, "No SSID provided");
    return;
  }

  // Create SSID string
  char ssid[33];
  jsvGetString(jsSsid, ssid, sizeof(ssid)-1);

  // Make sure jsOptions is NULL or an object
  if (jsOptions != NULL && !jsvIsObject(jsOptions)) {
    EXPECT_OPT_EXCEPTION(jsOptions);
    return;
  }

  // Check callback
  if (g_jsGotIpCallback != NULL) {
    jsvUnLock(g_jsGotIpCallback);
  }
  g_jsGotIpCallback = NULL;
  if (jsCallback != NULL && !jsvIsUndefined(jsCallback) && !jsvIsFunction(jsCallback)) {
    EXPECT_CB_EXCEPTION(jsCallback);
    return;
  }

  // Clear disconnect callback to prevent disconnection from disabling station mode
  if (g_jsDisconnectCallback != NULL) {
    jsvUnLock(g_jsDisconnectCallback);
  }
  g_jsDisconnectCallback = NULL;

  // Get the optional password
  char password[65];
  memset(password, 0, sizeof(password));
  if (jsOptions != NULL) {
    JsVar *jsPassword = jsvObjectGetChildIfExists(jsOptions, "password");
    if (jsPassword != NULL && !jsvIsString(jsPassword)) {
      jsExceptionHere(JSET_ERROR, "Expecting options.password to be a string but got %t", jsPassword);
      jsvUnLock(jsPassword);
      return;
    }
    if (jsPassword != NULL) {
      jsvGetString(jsPassword, password, sizeof(password)-1);
    } else {
      password[0] = '\0';
    }
    jsvUnLock(jsPassword);
  } // End of we had options
  jsDebug(DBG_INFO, "jswrap_wifi_connect: SSID '%s', password '%s', Callback done\n", ssid, password);

  // At this point, we have the ssid in "ssid" and the password in "password".
  // Perform an esp_wifi_set_mode
  wifi_mode_t mode;
  esp_err_t err;
  err = esp_wifi_get_mode(&mode);
  if (err != ESP_OK) {
    jsError( "jswrap_wifi_connect: esp_wifi_get_mode: %d(%s)", err,wifiErrorToString(err));
    return;
  }
  wifi_mode_t oldMode = mode;
  switch(oldMode) {
    case WIFI_MODE_NULL:
    case WIFI_MODE_STA:
      mode = WIFI_MODE_STA;
      break;
    case WIFI_MODE_APSTA:
    case WIFI_MODE_AP:
#if CONFIG_IDF_TARGET_ESP32C3
      mode = WIFI_MODE_STA; // C3 doesn't like AP and station at once
#else
      mode = WIFI_MODE_APSTA;
#endif
      break;
    default:
      jsError( "jswrap_wifi_connect: Unexpected mode type: %d", mode);
      break;
  }

  jsDebug(DBG_INFO, "jswrap_wifi_connect: mode %s -> %s\n", wifiModeToString(oldMode), wifiModeToString(mode));

  err = esp_wifi_set_mode(mode);
  if (err != ESP_OK) {
    jsError( "jswrap_wifi_connect: esp_wifi_set_mode: %d(%s), mode=%d", err, wifiErrorToString(err), mode);
    return;
  }
  jsDebug(DBG_INFO, "jswrap_wifi_connect: esp_wifi_set_mode done\n");

  // Perform a an esp_wifi_set_config
  wifi_config_t staConfig;
  memset(&staConfig, 0, sizeof(staConfig));
  memcpy(staConfig.sta.ssid, ssid, sizeof(staConfig.sta.ssid));
  memcpy(staConfig.sta.password, password, sizeof(staConfig.sta.password));
  staConfig.sta.bssid_set = false;
#if !(ESP_IDF_VERSION_MAJOR>=4)
  esp_wifi_set_auto_connect(true);
  jsDebug(DBG_INFO, "jswrap_wifi_connect: esp_wifi_set_autoconnect done\n");
#endif

  err = esp_wifi_set_config(ESP_IF_WIFI_STA,  &staConfig);
  if (err != ESP_OK) {
    jsError( "jswrap_wifi_connect: esp_wifi_set_config: %d(%s)", err,wifiErrorToString(err));
    return;
  }
  jsDebug(DBG_INFO, "jswrap_wifi_connect: esp_wifi_set_config done\n");

  // Attempt to reconnect 3 times, just in case
  s_retry_num = 3;
  /* If we were already connected in station mode, force a disconnect so
  we reconnect. s_retry_num meant we won't fail immediately on disconnect
  but will reconnect */
  if (g_isStaConnected) {
    jsDebug(DBG_INFO, "jswrap_wifi_connect: disconnecting so we can reconnect\n");
    esp_wifi_disconnect();
  } else { // not connected - call esp_wifi_start
    // Perform an esp_wifi_start
    jsDebug(DBG_INFO, "jswrap_wifi_connect: esp_wifi_start %s\n",ssid);
    err = esp_wifi_start();
    if (err != ESP_OK) {
      jsError( "jswrap_wifi_connect: esp_wifi_start: %d(%s)", err,wifiErrorToString(err));
      return;
    }
  }

  // Save the callback for later execution.
  g_jsGotIpCallback = jsvLockAgainSafe(jsCallback);
  // esp_wifi_connect gets called from the WIFI_EVENT_STA_START event - it errors if we call it here
}

void jswrap_wifi_scan(JsVar *jsCallback) {
  // If we have a saved scan callback function we must be scanning already
  if (g_jsScanCallback != NULL) {
    jsExceptionHere(JSET_ERROR, "A scan is already in progress");
    return;
  }

  // Check and save callback
  if (!jsvIsFunction(jsCallback)) {
    EXPECT_CB_EXCEPTION(jsCallback);
    return;
  }
  g_jsScanCallback = jsvLockAgainSafe(jsCallback);

  // We need to be in some kind of a station mode in order to perform a scan
  // Now we determine the mode we are currently in and set our new mode appropriately.
  // NULL -> STA
  // STA -> STA
  // AP -> APSTA
  // APSTA -> APSTA
  wifi_mode_t mode;
  esp_err_t err = esp_wifi_get_mode(&mode);
  if (err != ESP_OK) {
    jsError( "jswrap_wifi_scan: esp_wifi_get_mode: %d(%s)", err,wifiErrorToString(err));
    return;
  }

  switch(mode) {
    case WIFI_MODE_NULL:
      mode = WIFI_MODE_STA;
      break;
    case WIFI_MODE_STA:
      break;
    case WIFI_MODE_AP:
      mode = WIFI_MODE_APSTA;
      break;
    case WIFI_MODE_APSTA:
      break;
    default:
      jsError( "Unknown mode %d", mode);
      break;
  }

  err = esp_wifi_set_mode(mode);
  if (err != ESP_OK) {
    jsError( "jswrap_wifi_scan: esp_wifi_set_mode: %d(%s)", err,wifiErrorToString(err));
    return;
  }

  // Perform an esp_wifi_start
  err = esp_wifi_start();
  if (err != ESP_OK) {
    jsError( "jswrap_wifi_connect: esp_wifi_start: %d(%s)", err,wifiErrorToString(err));
    return;
  }

  wifi_scan_config_t scanConf = {
     .ssid = NULL,
     .bssid = NULL,
     .channel = 0,
     .show_hidden = true
  };
  esp_wifi_scan_start(&scanConf, false); // Don't block for scan.
  // When the scan completes, we will be notified by an arriving event that is handled
  // in the event handler.  The event handler will see that we have a callback function
  // registered and will invoke that callback at that time.
} // End of jswrap_wifi_scan

void jswrap_wifi_startAP(
    JsVar *jsSsid,     //!< The network SSID that we will use to listen as.
    JsVar *jsOptions,  //!< Configuration options.
    JsVar *jsCallback  //!< A callback to be invoked when completed.
  ) {

  // Check callback.  It is invalid if it is defined and not a function.
  if (jsCallback != NULL && !jsvIsUndefined(jsCallback) && !jsvIsFunction(jsCallback)) {
    EXPECT_CB_EXCEPTION(jsCallback);
    return;
  }

  // Validate that the SSID value is provided and is a string.
  if (!jsvIsString(jsSsid)) {
      jsExceptionHere(JSET_ERROR, "No SSID");
    return;
  }

  // Make sure jsOptions is NULL or an object
  if (jsOptions != NULL && !jsvIsNull(jsOptions) && !jsvIsObject(jsOptions)) {
    EXPECT_OPT_EXCEPTION(jsOptions);
    return;
  }

  // Check callback
  if (g_jsAPStartedCallback != NULL) {
    jsvUnLock(g_jsAPStartedCallback);
  }
  g_jsAPStartedCallback = NULL;
  if (jsCallback != NULL && !jsvIsUndefined(jsCallback) && !jsvIsFunction(jsCallback)) {
    EXPECT_CB_EXCEPTION(jsCallback);
    return;
  }

  wifi_ap_config_t apConfig;
  bzero(&apConfig, sizeof(apConfig));
  apConfig.ssid_hidden     = 0;
  apConfig.beacon_interval = 100;
  apConfig.channel         = 0;
  apConfig.authmode        = WIFI_AUTH_OPEN;
  apConfig.max_connection  = 4;
  apConfig.ssid_len        = (uint8_t)jsvGetString(jsSsid, (char *)apConfig.ssid, sizeof(apConfig.ssid));
  apConfig.authmode        = WIFI_AUTH_OPEN;
  strcpy(apConfig.password, "");

  if (jsvIsObject(jsOptions)) {
    // Handle channel
    JsVar *jsChan = jsvObjectGetChildIfExists(jsOptions, "channel");
    if (jsvIsInt(jsChan)) {
      int chan = jsvGetInteger(jsChan);
      if (chan >= 1 && chan <= 13) {
        apConfig.channel = (uint8_t)chan;
      }
    }
    jsvUnLock(jsChan);

    // Handle password
    JsVar *jsPassword = jsvObjectGetChildIfExists(jsOptions, "password");
    if (jsPassword != NULL) {
      if (!jsvIsString(jsPassword) || jsvGetStringLength(jsPassword) < 8) {
        jsExceptionHere(JSET_ERROR, "Password must be string of at least 8 characters");
        jsvUnLock(jsPassword);
        return;
      }
      jsvGetString(jsPassword, (char *)apConfig.password, sizeof(apConfig.password)-1);
    }

    // Handle the authMode
    JsVar *jsAuth = jsvObjectGetChildIfExists(jsOptions, "authMode");
    if (jsvIsString(jsAuth)) {
      if (jsvIsStringEqual(jsAuth, "open")) {
        apConfig.authmode = WIFI_AUTH_OPEN;
      } else if (jsvIsStringEqual(jsAuth, "wpa2")) {
        apConfig.authmode = WIFI_AUTH_WPA2_PSK;
      } else if (jsvIsStringEqual(jsAuth, "wpa")) {
        apConfig.authmode = WIFI_AUTH_WPA_PSK;
      } else if (jsvIsStringEqual(jsAuth, "wpa_wpa2")) {
        apConfig.authmode = WIFI_AUTH_WPA_WPA2_PSK;
      } else {
        jsvUnLock(jsAuth);
        jsExceptionHere(JSET_ERROR, "Unknown authMode value");
        return;
      }
    } else {
    // no explicit auth mode, set according to presence of password
      if (strlen(apConfig.password) == 0) {
        apConfig.authmode = WIFI_AUTH_OPEN;
      } else {
        apConfig.authmode = WIFI_AUTH_WPA2_PSK;
      }
    } // End of no explicit authMode

    jsvUnLock(jsAuth);

    // Make sure password and authMode match
    if (apConfig.authmode != WIFI_AUTH_OPEN && strlen(apConfig.password) == 0) {
      jsExceptionHere(JSET_ERROR, "Password not set but authMode not open");
      return;
    }

    // Make sure that if authmode is explicitly open then there is NO password supplied.
    if (apConfig.authmode == WIFI_AUTH_OPEN && strlen(apConfig.password) > 0) {
      jsExceptionHere(JSET_ERROR, "Auth mode set to open but password supplied");
      return;
    }
  } // End we have an options structure

  esp_err_t err;
  // set callback
  if (jsvIsFunction(jsCallback)) {
    g_jsAPStartedCallback = jsvLockAgainSafe(jsCallback);
  }

  wifi_mode_t mode;
  err = esp_wifi_get_mode(&mode);
#if CONFIG_IDF_TARGET_ESP32C3
  if (mode == WIFI_MODE_STA) mode = WIFI_MODE_NULL; // C3 doesn't like AP and station at once
#endif
  err = esp_wifi_set_mode( mode | WIFI_MODE_AP);
  if (err != ESP_OK) {
    jsError( "jswrap_wifi_startAP: esp_wifi_set_mode: %d(%s)", err,wifiErrorToString(err));
    return;
  }

  err = esp_wifi_set_config(WIFI_IF_AP, (wifi_config_t *)&apConfig);
  if (err != ESP_OK) {
    jsError( "jswrap_wifi_startAP: wifi_set_config: %d - ssid=%.*s, password=%s, authMode=%d, maxConnections=%d, beacon=%d, channel=%d",
      err, apConfig.ssid_len, apConfig.ssid, apConfig.password, apConfig.authmode, apConfig.max_connection, apConfig.beacon_interval, apConfig.channel);
    return;
  }

  // Perform an esp_wifi_start
  err = esp_wifi_start();
  if (err != ESP_OK) {
    jsError( "jswrap_wifi_startAP: esp_wifi_start: %d(%s)", err,wifiErrorToString(err));
    return;
  }
} // End of jswrap_wifi_startAP


JsVar *jswrap_wifi_getStatus(JsVar *jsCallback) {
  UNUSED(jsCallback);
  // We have to determine the following information:
  //
  // - [done] The status of the station interface
  // - [    ] The status of the access point interface
  // - [done] The current mode of operation
  // - [    ] The physical modulation
  // - [done] The power save type
  // - [    ] The save mode
  //
  // For the status of the station and access point interfaces, we don't know how to get those
  // but have asked here: http://esp32.com/viewtopic.php?f=13&t=330

  // Get the current mode of operation.
  wifi_mode_t mode;
  esp_wifi_get_mode(&mode);

  // Get the current power save type
  wifi_ps_type_t psType;
  esp_wifi_get_ps(&psType);
  char *psTypeStr;
  switch(psType) {
#if ESP_IDF_VERSION_MAJOR>=4
  case WIFI_PS_MIN_MODEM:
    psTypeStr = "min_modem";
    break;
  case WIFI_PS_MAX_MODEM:
    psTypeStr = "max_modem";
    break;
#else
  case WIFI_PS_MODEM:
    psTypeStr = "modem";
    break;
#endif
  case WIFI_PS_NONE:
    psTypeStr = "none";
    break;
  default:
    psTypeStr = "unknown";
    break;
  }

  JsVar *jsWiFiStatus = jsvNewObject();
  if (g_isStaConnected) {
    jsvObjectSetStringChild(jsWiFiStatus, "station", "connected");

    if (mode == WIFI_MODE_STA || mode == WIFI_MODE_APSTA) {
      wifi_ap_record_t ap_info;
      esp_wifi_sta_get_ap_info(&ap_info);
      char buf[35];

      // SSID of AP
      strncpy(buf, ap_info.ssid, sizeof(buf));
      buf[34] = 0;
      jsvObjectSetStringChild(jsWiFiStatus, "ssid", buf);

      // MAC address of AP
      sprintf(buf, MACSTR, MAC2STR(ap_info.bssid));
      buf[18] = 0;
      jsvObjectSetStringChild(jsWiFiStatus, "bssid", buf);

      // Channel of AP
      jsvObjectSetIntChild(jsWiFiStatus, "channel", ap_info.primary);

      // RSSI
      jsvObjectSetIntChild(jsWiFiStatus, "rssi", ap_info.rssi);

      // HT mode
      jsvObjectSetChildAndUnLock(jsWiFiStatus, "htMode",
        jsvNewFromString(htModeToString(ap_info.second)));

      // Auth mode
      jsvObjectSetChildAndUnLock(jsWiFiStatus, "authMode",
        jsvNewFromString(authModeToString(g_lastEventStaConnected.authmode)));

      /* Later version
       * // Pairwise cipher
       * jsvObjectSetChildAndUnLock(jsWiFiStatus, "pairwiseCipher",
       *   jsvNewFromString(cipherTypeToString(ap_info.pairwise_cipher)));
       *
       * // Group cipher
       * jsvObjectSetChildAndUnLock(jsWiFiStatus, "groupCipher",
       *   jsvNewFromString(cipherTypeToString(ap_info.group_cipher)));
       */
    }

  } else {
    jsvObjectSetChildAndUnLock(jsWiFiStatus, "station",
        jsvNewFromString(wifiReasonToString(g_lastEventStaDisconnected.reason)));
  }
  // if wifi hasn't started, return NULL
  if (!g_isAPStarted && !g_isStaConnected)
    mode = WIFI_MODE_NULL;
  jsvObjectSetStringChild(jsWiFiStatus, "mode", wifiModeToString(mode));
  jsvObjectSetStringChild(jsWiFiStatus, "powersave", psTypeStr);

  return jsWiFiStatus;
} // End of jswrap_wifi_getStatus


void jswrap_wifi_setConfig(JsVar *jsSettings) {
  // Make sure jsSetings an object
  if (!jsvIsObject(jsSettings)) {
    EXPECT_OPT_EXCEPTION(jsSettings);
    return;
  }

  // phy setting
  JsVar *jsPhy = jsvObjectGetChildIfExists(jsSettings, "phy");
  if (jsvIsString(jsPhy)) {
    if (jsvIsStringEqual(jsPhy, "11b")) {
      esp_wifi_set_protocol(WIFI_IF_AP,WIFI_PROTOCOL_11B);
    } else if (jsvIsStringEqual(jsPhy, "11g")) {
      esp_wifi_set_protocol(WIFI_IF_AP,WIFI_PROTOCOL_11B| WIFI_PROTOCOL_11G);
    } else if (jsvIsStringEqual(jsPhy, "11n")) {
      esp_wifi_set_protocol(WIFI_IF_AP,WIFI_PROTOCOL_11B| WIFI_PROTOCOL_11G|WIFI_PROTOCOL_11N);
    } else {
      jsvUnLock(jsPhy);
      jsExceptionHere(JSET_ERROR, "Unknown phy mode");
      return;
    }
  }
  if (jsPhy != NULL) jsvUnLock(jsPhy);

  // powersave setting
  // Call esp_wifi_set_ps(WIFI_PS_MIN_MODEM) to enable Modem-sleep minimum power save mode or esp_wifi_set_ps(WIFI_PS_MAX_MODEM)
  JsVar *jsPowerSave = jsvObjectGetChildIfExists(jsSettings, "powersave");
  if (jsvIsString(jsPowerSave)) {
    if (jsvIsStringEqual(jsPowerSave, "none")) {
      esp_wifi_set_ps(WIFI_PS_NONE);
#if !(ESP_IDF_VERSION_MAJOR>=4)
    } else if (jsvIsStringEqual(jsPowerSave, "ps-poll")) {
      esp_wifi_set_ps(WIFI_PS_MODEM);
#endif
    } else if (jsvIsStringEqual(jsPowerSave, "min")) {
      esp_wifi_set_ps(WIFI_PS_MIN_MODEM);
    } else if (jsvIsStringEqual(jsPowerSave, "max")) {
      esp_wifi_set_ps(WIFI_PS_MAX_MODEM);
    } else {
      jsvUnLock(jsPowerSave);
      jsExceptionHere(JSET_ERROR, "Unknown powersave mode");
      return;
    }
  }
  if (jsPowerSave != NULL) jsvUnLock(jsPowerSave);
}

JsVar *jswrap_wifi_getDetails(JsVar *jsCallback) {
  // Check callback
  if (jsCallback != NULL && !jsvIsNull(jsCallback) && !jsvIsFunction(jsCallback)) {
    EXPECT_CB_EXCEPTION(jsCallback);
    return NULL;
  }

  JsVar *jsDetails = jsvNewObject();
  if (g_isStaConnected == true) {
    wifi_sta_config_t config;
    esp_wifi_get_config(WIFI_IF_STA, (wifi_config_t *)&config);
    char buf[65];

    // ssid
    strncpy(buf, (char *)config.ssid, 32);
    buf[32] = 0;
    jsvObjectSetStringChild(jsDetails, "ssid", buf);

    // password
    strncpy(buf, (char *)config.password, 64);
    buf[64] = 0;
    jsvObjectSetStringChild(jsDetails, "password", (char *)config.password);

    // Status
    jsvObjectSetStringChild(jsDetails, "status", "connected");

    // Authmode
    jsvObjectSetChildAndUnLock(jsDetails, "authMode",
        jsvNewFromString(authModeToString(g_lastEventStaConnected.authmode)));
  } else {
    // Status
    jsvObjectSetChildAndUnLock(jsDetails, "status",
        jsvNewFromString(wifiReasonToString(g_lastEventStaDisconnected.reason)));
  }

  // Schedule callback if a function was provided
  if (jsvIsFunction(jsCallback)) {
    JsVar *params[1];
    params[0] = jsDetails;
    jsiQueueEvents(NULL, jsCallback, params, 1);
  }
  return jsDetails;
} // End of jswrap_wifi_getDetails


JsVar *jswrap_wifi_getAPDetails(JsVar *jsCallback) {
  // Check callback
  if (jsCallback != NULL && !jsvIsNull(jsCallback) && !jsvIsFunction(jsCallback)) {
    EXPECT_CB_EXCEPTION(jsCallback);
    return NULL;
  }

  JsVar *jsDetails = jsvNewObject();

  wifi_ap_config_t config;
  esp_wifi_get_config(WIFI_IF_AP, (wifi_config_t *)&config);
  jsvObjectSetStringChild(jsDetails, "authMode", authModeToString(config.authmode));
  jsvObjectSetBoolChild(jsDetails, "hidden", config.ssid_hidden);
  jsvObjectSetIntChild(jsDetails, "maxConn", config.max_connection);

  char buf[65];

  // ssid
  strncpy(buf, (char *)config.ssid, 32);
  buf[32] = 0;
  jsvObjectSetStringChild(jsDetails, "ssid", buf);

  // password
  strncpy(buf, (char *)config.password, 64);
  buf[64] = 0;
  jsvObjectSetStringChild(jsDetails, "password", (char *)config.password);

  // Schedule callback if a function was provided
  if (jsvIsFunction(jsCallback)) {
    JsVar *params[1];
    params[0] = jsDetails;
    jsiQueueEvents(NULL, jsCallback, params, 1);
  }
  return jsDetails;
} // End of jswrap_wifi_getAPDetails


#if DEBUG
static void dumpJsVarObject(JsVar *o) {
  JsvObjectIterator it;
  jsvObjectIteratorNew(&it, o);

  while (jsvObjectIteratorHasValue(&it)) {
    JsVar *key = jsvObjectIteratorGetKey(&it);
    JsVar *val = jsvObjectIteratorGetValue(&it);

    char keyBuf[32], valBuf[64];
    jsvGetString(key, keyBuf, sizeof(keyBuf));
    jsvGetString(val, valBuf, sizeof(valBuf));  // ← Converts ANY type to string

    jsDebug(DBG_INFO, "  %s='%s'\n", keyBuf, valBuf);

    jsvUnLock(key);
    jsvUnLock(val);
    jsvObjectIteratorNext(&it);
  }
  jsvObjectIteratorFree(&it);
}
#endif

void jswrap_wifi_save(JsVar *what) {
  jsDebug(DBG_INFO, "Wifi.save\n");
  JsVar *o = jsvNewObject();
  if (!o) return;

  if (jsvIsString(what) && jsvIsStringEqual(what, "clear")) {
    JsVar *name = jsvNewFromString(WIFI_CONFIG_STORAGE_NAME);
    jswrap_storage_erase(name);
    jsvUnLock(name);
    jsDebug(DBG_INFO, "Wifi.save(clear)\n");
    return;
  }

  // station stuff
  wifi_sta_config_t sta_config;
  esp_wifi_get_config(WIFI_IF_STA, (wifi_config_t *)&sta_config);
  jsvObjectSetStringChild(o, "ssid", (char *)sta_config.ssid);
  jsvObjectSetStringChild(o, "password", (char *)sta_config.password);

  wifi_mode_t wifi_mode;
  esp_wifi_get_mode(&wifi_mode);
  jsvObjectSetIntChild(o, "mode", wifi_mode);

  //jsvObjectSetIntChild(o, "phyMode", wifi_get_phy_mode());
  wifi_ps_type_t psType;
  esp_wifi_get_ps(&psType);
  jsvObjectSetIntChild(o, "sleepType", psType);

  wifi_ap_config_t ap_config;
  esp_wifi_get_config(WIFI_IF_AP, (wifi_config_t *)&ap_config);

  jsvObjectSetStringChild(o, "ssidAP", (char *)ap_config.ssid);
  jsvObjectSetStringChild(o, "passwordAP", (char *) ap_config.password);
  jsvObjectSetIntChild(o, "authmodeAP", ap_config.authmode);
  jsvObjectSetIntChild(o, "hiddenAP", ap_config.ssid_hidden);
  jsvObjectSetIntChild(o, "channelAP", ap_config.channel);

  const char * hostname = NULL;
#if ESP_IDF_VERSION_MAJOR>=5
  esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
  esp_err_t err = esp_netif_get_hostname(netif, &hostname);
  jsDebug(DBG_INFO,"esp_netif_get_hostname: %s\n", wifiErrorToString(err));
  jsDebug(DBG_INFO,"hostname: %s\n", hostname);
#else
  esp_err_t err = tcpip_adapter_get_hostname(TCPIP_ADAPTER_IF_STA, &hostname);
#endif
  if (hostname) jsvObjectSetStringChild(o, "hostname", (char *) hostname);
  // save object
#ifdef DEBUG
  dumpJsVarObject(o);
#endif
  JsVar *name = jsvNewFromString(WIFI_CONFIG_STORAGE_NAME);
  jswrap_storage_erase(name);
  jswrap_storage_write(name,o,0,0);
  jsvUnLock2(name,o);

  jsDebug(DBG_INFO, "Wifi.save: write completed\n");
}

void jswrap_wifi_restore(void) {
  jsDebug(DBG_INFO, "jswrap_wifi_restore\n");

#if CONFIG_IDF_TARGET_ESP32C3
  esp_wifi_set_max_tx_power(34); // 8.5dBm
#endif

  JsVar *name = jsvNewFromString(WIFI_CONFIG_STORAGE_NAME);
  JsVar *o = jswrap_storage_readJSON(name, true);
  if (!o) { // no data
    jsDebug(DBG_INFO, "jswrap_wifi_restore: No data - not starting WiFi\n");
    jsvUnLock2(name,o);
    return;
  }

  JsVar *v;
  wifi_mode_t savedMode = jsvObjectGetIntegerChild(o,"mode");
  esp_wifi_set_mode(savedMode);

  wifi_config_t config;  // Unified config for IDF 5.x
  bzero(&config, sizeof(config));

  esp_err_t err;
  if (savedMode & WIFI_MODE_AP) {
    config.ap.authmode = jsvObjectGetIntegerChild(o,"authmodeAP");
    config.ap.ssid_hidden = jsvObjectGetIntegerChild(o,"hiddenAP");

    v = jsvObjectGetChildIfExists(o,"ssidAP");
    config.ap.ssid_len = jsvGetString(v, (char *)config.ap.ssid, sizeof(config.ap.ssid));
    jsvUnLock(v);

    v = jsvObjectGetChildIfExists(o,"passwordAP");
    jsvGetString(v, (char *)config.ap.password, sizeof(config.ap.password));
    jsvUnLock(v);

    config.ap.channel = jsvObjectGetIntegerChild(o,"channelAP");

    config.ap.max_connection = 4;
    config.ap.beacon_interval = 100;
    err = esp_wifi_set_config(WIFI_IF_AP, &config);  // Fixed: apConfig -> config
    jsDebug(DBG_INFO, "jswrap_wifi_restore: AP=%s\n", config.ap.ssid);
  }

  if (savedMode & WIFI_MODE_STA) {
    v = jsvObjectGetChildIfExists(o,"ssid");
    jsvGetString(v, (char *)config.sta.ssid, sizeof(config.sta.ssid));
    jsvUnLock(v);

    v = jsvObjectGetChildIfExists(o,"password");
    jsvGetString(v, (char *)config.sta.password, sizeof(config.sta.password));
    jsvUnLock(v);

    err = esp_wifi_set_config(ESP_IF_WIFI_STA, &config);  // Fixed ESP_IF_WIFI_STA
    jsDebug(DBG_INFO, "Wifi.restore: STA=%s\n", config.sta.ssid);
  }

  err = esp_wifi_start();
  if (err != ESP_OK) {
    jsError("jswrap_wifi_restore: esp_wifi_start: %d(%s)", err - ESP_ERR_WIFI_BASE, wifiErrorToString(err));
    jsvUnLock2(name,o);
    return;
  }

  if (savedMode & WIFI_MODE_STA) {
    v = jsvObjectGetChildIfExists(o,"hostname");
    if (v) {
      char hostname[64];
      jsvGetString(v, hostname, sizeof(hostname));
      jsDebug(DBG_INFO, "Wifi.restore: hostname=%s\n", hostname);
#if ESP_IDF_VERSION_MAJOR >= 5
      esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
      if (netif) esp_netif_set_hostname(netif, hostname);
#else
      tcpip_adapter_set_hostname(TCPIP_ADAPTER_IF_STA, hostname);
#endif
    }
    jsvUnLock(v);
  }

  if ((savedMode == WIFI_MODE_STA) || (savedMode == WIFI_MODE_APSTA)) {
    err = esp_wifi_connect();
    if (err != ESP_OK) {
      jsError("jswrap_wifi_restore: esp_wifi_connect: %d(%s)", err - ESP_ERR_WIFI_BASE, wifiErrorToString(err));
      jsvUnLock2(name,o);
      return;
    }
  } else {
    jsDebug(DBG_INFO, "Wifi: Both STA AND APSTA are off\n");
  }

  jsvUnLock2(name,o);
}

/**
 * Get the ip info for the given interface.  The interfaces are:
 * * TCPIP_ADAPTER_IF_STA - Station
 * * TCPIP_ADAPTER_IF_AP - Access Point
 */
static JsVar *getIPInfo(JsVar *jsCallback, int interface) {
  if (jsCallback && !jsvIsNull(jsCallback) && !jsvIsFunction(jsCallback)) {
    EXPECT_CB_EXCEPTION(jsCallback);
    return NULL;
  }

  JsVar *jsIpInfo = jsvNewObject();
  if (!jsIpInfo) return NULL;  // Early out on OOM

#if ESP_IDF_VERSION_MAJOR >= 5
  esp_netif_t *netif = esp_netif_get_handle_from_ifkey(
    interface == TCPIP_ADAPTER_IF_STA ? "WIFI_STA_DEF" : "WIFI_AP_DEF"
  );
  esp_netif_ip_info_t ipInfo;
  esp_err_t err = (netif) ? esp_netif_get_ip_info(netif, &ipInfo) : ESP_FAIL;
#else
  tcpip_adapter_ip_info_t ipInfo;
  esp_err_t err = tcpip_adapter_get_ip_info(interface, &ipInfo);
#endif

  if (err == ESP_OK) {
    jsvObjectSetChildAndUnLock(jsIpInfo, "ip",
      networkGetAddressAsString((uint8_t*)&ipInfo.ip, 4, 10, '.'));
    jsvObjectSetChildAndUnLock(jsIpInfo, "netmask",
      networkGetAddressAsString((uint8_t*)&ipInfo.netmask, 4, 10, '.'));
    jsvObjectSetChildAndUnLock(jsIpInfo, "gw",
      networkGetAddressAsString((uint8_t*)&ipInfo.gw, 4, 10, '.'));
  }

  // MAC always succeeds
  uint8_t mac[6];
  wifi_interface_t wifif = interface == TCPIP_ADAPTER_IF_STA ? WIFI_IF_STA : WIFI_IF_AP;
  esp_wifi_get_mac(wifif, mac);
  char macStr[18];
  sprintf(macStr, MACSTR, MAC2STR(mac));
  jsvObjectSetStringChild(jsIpInfo, "mac", macStr);

  // Callback
  if (jsvIsFunction(jsCallback)) {
    JsVar *params[2] = {jsvNewWithFlags(JSV_NULL), jsIpInfo};
    jsiQueueEvents(NULL, jsCallback, params, 2);
    jsvUnLock(params[0]);
  }

  return jsIpInfo;

} // End of getIPInfo

JsVar *jswrap_wifi_getIP(JsVar *jsCallback) {
  JsVar *jsIpInfo = getIPInfo(jsCallback, TCPIP_ADAPTER_IF_STA);
  return jsIpInfo;
} // End of jswrap_wifi_getIP

JsVar *jswrap_wifi_getAPIP(JsVar *jsCallback) {
  JsVar *jsIpInfo = getIPInfo(jsCallback, TCPIP_ADAPTER_IF_AP);
  return jsIpInfo;
}

JsVar *jswrap_wifi_getHostname(JsVar *jsCallback) {
  const char *hostname = NULL;
#if ESP_IDF_VERSION_MAJOR >= 5
  esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");

  if (netif != NULL) {
    esp_err_t err = esp_netif_get_hostname(netif, &hostname);
    if (err != ESP_OK || hostname == NULL) {
      hostname = "";
    }
  } else {
    hostname = "";
  }
#else
  esp_err_t err = tcpip_adapter_get_hostname(TCPIP_ADAPTER_IF_STA, &hostname);
  if (err != ESP_OK || hostname == NULL) {
    hostname = "";
  }
#endif
  return jsvNewFromString(hostname ? hostname : "");
}

void jswrap_wifi_setHostname(JsVar *jsHostname, JsVar *jsCallback) {
  char hostname[256];
  jsvGetString(jsHostname, hostname, sizeof(hostname));
  jsDebug(DBG_INFO, "Wifi.setHostname: %s\n", hostname);

#if ESP_IDF_VERSION_MAJOR >= 5
  esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
  if (netif != NULL) {
    esp_err_t err = esp_netif_set_hostname(netif, hostname);
    if (err != ESP_OK) {
      jsExceptionHere(JSET_ERROR, "Failed to set hostname");
    }
  }
#else
  esp_err_t err = tcpip_adapter_set_hostname(TCPIP_ADAPTER_IF_STA, hostname);
  if (err != ESP_OK) {
    jsExceptionHere(JSET_ERROR, "Failed to set hostname");
  }
#endif

  // now update mDNS
  startMDNS(hostname);

  if (jsvIsFunction(jsCallback)) {
    jsiQueueEvents(0, jsCallback, 0, 0);
  }
}

static uint8_t seq_no;

esp_err_t pingResults(ping_target_id_t msgType, esp_ping_found * pingResp){
	//printf("AvgTime:%.1fmS Sent:%d Rec:%d Err:%d min(mS):%d max(mS):%d ",
  //(float)pf->total_time/pf->recv_count, pf->send_count, pf->recv_count, pf->err_count, pf->min_time, pf->max_time );
	//printf("Resp(mS):%d Timeouts:%d Total Time:%d\n",pf->resp_time, pf->timeout_count, pf->total_time);
  if (g_jsPingCallback != NULL) {
    JsVar *jsPingResponse = jsvNewObject();
    jsvObjectSetIntChild(jsPingResponse, "totalCount", pingResp->send_count);
    jsvObjectSetIntChild(jsPingResponse, "totalBytes", pingResp->total_bytes);
    jsvObjectSetIntChild(jsPingResponse, "totalTime", pingResp->total_time);
    jsvObjectSetIntChild(jsPingResponse, "respTime", pingResp->resp_time);
    // don't have a sequence
    jsvObjectSetIntChild(jsPingResponse, "seqNo", ++seq_no);
    jsvObjectSetIntChild(jsPingResponse, "timeoutCount", pingResp->timeout_count);
    jsvObjectSetIntChild(jsPingResponse, "bytes", pingResp->bytes);
    jsvObjectSetIntChild(jsPingResponse, "error", pingResp->err_count);
    JsVar *params[1];
    params[0] = jsPingResponse;
    jsiQueueEvents(NULL, g_jsPingCallback, params, 1);
    jsvUnLock(jsPingResponse);
  }
	return ESP_OK;
}

void jswrap_wifi_ping(
    JsVar *ipAddr,      //!< A string or integer representation of an IP address.
    JsVar *pingCallback //!< Optional callback function.
) {
  // If the parameter is a string, get the IP address from the string
  // representation.
  ip4_addr_t ip;
  if (jsvIsString(ipAddr)) {
    char ipString[20];
    jsvGetString(ipAddr, ipString, sizeof(ipString)-1);
    ip.addr = networkParseIPAddress(ipString);
    if (ip.addr == 0) {
        jsExceptionHere(JSET_ERROR, "Not a valid IP address");
      return;
    }
  } else
  // If the parameter is an integer, treat it as an IP address.
  if (jsvIsInt(ipAddr)) {
    ip.addr = jsvGetInteger(ipAddr);
  } else
  // The parameter was neither a string nor an IP address and hence we don't
  // know how to get the IP address of the partner to ping so throw an
  // exception.
  {
      jsExceptionHere(JSET_ERROR, "IP address must be string or integer");
    return;
  }

  if (jsvIsUndefined(pingCallback) || jsvIsNull(pingCallback)) {
    if (g_jsPingCallback != NULL) {
      jsvUnLock(g_jsPingCallback);
    }
    g_jsPingCallback = NULL;
  } else if (!jsvIsFunction(pingCallback)) {
      jsExceptionHere(JSET_ERROR, "Callback is not a function");
    return;
  } else {
    if (g_jsPingCallback != NULL) {
      jsvUnLock(g_jsPingCallback);
    }
    g_jsPingCallback = pingCallback;
    jsvLockAgainSafe(g_jsPingCallback);
  }

  // We now have an IP address to ping ... so ping.

  uint32_t ping_count = 5;  //how many pings per report
  uint32_t ping_timeout = 1000; //mS till we consider it timed out
  uint32_t ping_delay = 500; //mS between pings
  esp_ping_set_target(PING_TARGET_IP_ADDRESS_COUNT, &ping_count, sizeof(uint32_t));
  esp_ping_set_target(PING_TARGET_RCV_TIMEO, &ping_timeout, sizeof(uint32_t));
  esp_ping_set_target(PING_TARGET_DELAY_TIME, &ping_delay, sizeof(uint32_t));
  esp_ping_set_target(PING_TARGET_IP_ADDRESS, &ip.addr, sizeof(uint32_t));
  esp_ping_set_target(PING_TARGET_RES_FN, &pingResults, sizeof(pingResults));
  seq_no=0;
  #if ESP_IDF_VERSION_MAJOR < 5
  ping_init();
  #endif
}
void jswrap_wifi_setSNTP(JsVar *jsServer, JsVar *jsZone) {
  if (!jsvIsString(jsZone)) {
    jsExceptionHere(JSET_ERROR, "Zone is not a string");
    return;
  }

  if (!jsvIsString(jsServer)) {
    jsExceptionHere(JSET_ERROR, "Server is not a string");
    return;
  }
  char zone[64];
  jsvGetString(jsZone, zone, 64);

  char server[64];
  jsvGetString(jsServer, server, 64);

  setenv("TZ", zone, 1);
  tzset();

#if ESP_IDF_VERSION_MAJOR < 4
  // Do nothing - SNTP stops with WiFi disconnect
#elif ESP_IDF_VERSION_MAJOR >= 4
  if (esp_sntp_enabled()) {
    esp_sntp_stop();
  }
#endif

  sntp_setoperatingmode(SNTP_OPMODE_POLL);
  sntp_setservername(0, server);
  sntp_init();
  jsDebug(DBG_INFO, "SNTP: %s %s\n", server, zone);
}

/**
 * Handle a response from esp_gethostbyname.
 * Invoke the callback function to inform the caller that a hostname has been converted to
 * an IP address.  The callback function should take a parameter that is the IP address.
 */

static void dnsFoundCallback(
    const char *hostname,    //!< The hostname that was converted to an IP address.
    const ip_addr_t *ipAddr, //!< The ip address retrieved.  This may be 0.
    void *arg                //!< Parameter passed in from espconn_gethostbyname.
  ) {

  jsDebug(DBG_INFO, "Wifi.getHostByName CB - %s %x\n", hostname, ipAddr );
  if (g_jsHostByNameCallback != NULL) {
    JsVar *params[1];
    if (ipAddr && IP_IS_V4(ipAddr)) {
      ip4_addr_t *ip4 = ip_2_ip4(ipAddr);
      params[0] = networkGetAddressAsString((uint8_t *)&ip4->addr, 4, 10, '.');
    } else {
      params[0] = jsvNewNull();
    }
    jsiQueueEvents(NULL, g_jsHostByNameCallback, params, 1);
    jsvUnLock(params[0]);
    jsvUnLock(g_jsHostByNameCallback);
    g_jsHostByNameCallback = NULL;
  }
}

void jswrap_wifi_getHostByName(
    JsVar *jsHostname,
    JsVar *jsCallback
) {
  ip_addr_t ipAddr;
  char hostname[256];

  jsDebug(DBG_INFO, "Wifi.getHostByName\n");

  if (!jsvIsString(jsHostname)) {
    jsExceptionHere(JSET_ERROR, "Hostname parameter is not a string");
    return;
  }
  if (!jsvIsFunction(jsCallback)) {
    jsExceptionHere(JSET_ERROR, "Callback is not a function");
    return;
  }
  // Save the callback, unlocking an old one if needed.
  if (g_jsHostByNameCallback != NULL)
    jsvUnLock(g_jsHostByNameCallback);
  g_jsHostByNameCallback = jsCallback;
  jsvLockAgainSafe(g_jsHostByNameCallback);

  jsvGetString(jsHostname, hostname, sizeof(hostname));

#if ESP_IDF_VERSION_MAJOR >= 4
  // IDF 4/5: show current DNS server via ESP‑NETIF
    esp_netif_t *sta = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    esp_netif_dns_info_t dns_info;
    esp_err_t ret = esp_netif_get_dns_info(sta, ESP_NETIF_DNS_MAIN, &dns_info);
    if (ret == ESP_OK) {
      if (dns_info.ip.type == IPADDR_TYPE_V4) {
        uint32_t addr = dns_info.ip.u_addr.ip4.addr;
        jsDebug(DBG_INFO,"DNS server: %d.%d.%d.%d\n",
          (addr>>0)&0xff, (addr>>8)&0xff,
          (addr>>16)&0xff, (addr>>24)&0xff);
      }
    } else {
    jsDebug(DBG_INFO, "esp_netif_get_dns_info: %d\n", ret);
  }
#endif

  jsDebug(DBG_INFO, "Wifi.getHostByName: %s\n", hostname);
  esp_err_t err = dns_gethostbyname(hostname, &ipAddr, dnsFoundCallback, NULL);
  if (err) jsDebug(DBG_INFO,"err = %d (%s)\n", err, esp_err_to_name(err));
  if (err == ESP_OK) {
    jsDebug(DBG_INFO, "Already resolved\n");
    dnsFoundCallback(hostname, &ipAddr, NULL);
  } else if (err == ERR_INPROGRESS) {
    jsDebug(DBG_INFO,"DNS query in progress (ERR_INPROGRESS)\n");
  } else {
    jsDebug(DBG_INFO,"DNS lookup failed: %d from dns_gethostbyname\n", err);
    // no callback will come
    if (g_jsHostByNameCallback != NULL) {
      JsVar *params[1] = { jsvNewNull() };
      jsiQueueEvents(NULL, g_jsHostByNameCallback, params, 1);
      jsvUnLock(params[0]);
      jsvUnLock(g_jsHostByNameCallback);
      g_jsHostByNameCallback = NULL;
    }
  }
}

// worker for jswrap_wifi_setIP and jswrap_wifi_setAPIP
static void setIP(JsVar *jsSettings, JsVar *jsCallback, int interface) {
  char ipTmp[20];
  int len = 0;
  esp_err_t err;
#if ESP_IDF_VERSION_MAJOR>=5
  esp_netif_ip_info_t info;
  memset(&info, 0, sizeof(info));
#else
  tcpip_adapter_ip_info_t info;
  memset( &info, 0, sizeof(info) );
#endif

// first check parameter
  if (!jsvIsObject(jsSettings)) {
    EXPECT_OPT_EXCEPTION(jsSettings);
    return;
  }

// get,check and store ip
  JsVar *jsIP = jsvObjectGetChildIfExists(jsSettings, "ip");
  if (jsIP != NULL && !jsvIsString(jsIP)) {
      EXPECT_OPT_EXCEPTION(jsIP);
      jsvUnLock(jsIP);
      return;
  }
  jsvGetString(jsIP, ipTmp, sizeof(ipTmp)-1);
  info.ip.addr = networkParseIPAddress(ipTmp);
  if ( info.ip.addr  == 0) {
    jsExceptionHere(JSET_ERROR, "Not a valid IP address");
    jsvUnLock(jsIP);
    return;
  }
  jsvUnLock(jsIP);

// get, check and store gw
  JsVar *jsGW = jsvObjectGetChildIfExists(jsSettings, "gw");
  if (jsGW != NULL && !jsvIsString(jsGW)) {
      EXPECT_OPT_EXCEPTION(jsGW);
      jsvUnLock(jsGW);
      return ;
  }
  jsvGetString(jsGW, ipTmp, sizeof(ipTmp)-1);
  info.gw.addr = networkParseIPAddress(ipTmp);
  if (info.gw.addr == 0) {
    jsExceptionHere(JSET_ERROR, "Not a valid Gateway address");
    jsvUnLock(jsGW);
    return;
  }
  jsvUnLock(jsGW);

// netmask setting
  JsVar *jsNM = jsvObjectGetChildIfExists(jsSettings, "netmask");
  if (jsNM != NULL && !jsvIsString(jsNM)) {
      EXPECT_OPT_EXCEPTION(jsNM);
      jsvUnLock(jsNM);
      return;
  }
  jsvGetString(jsNM, ipTmp, sizeof(ipTmp)-1);
  info.netmask.addr = networkParseIPAddress(ipTmp);
  if (info.netmask.addr == 0) {
    jsExceptionHere(JSET_ERROR, "Not a valid Netmask");
    jsvUnLock(jsNM);
    return;
  }
  jsvUnLock(jsNM);
// set IP for station
  if (interface == WIFI_IF_STA) {
#if ESP_IDF_VERSION_MAJOR>=5
    esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    esp_netif_dhcpc_stop(netif);  // CLIENT DHCP for STA
    err = esp_netif_set_ip_info(netif, &info);
#else
    tcpip_adapter_dhcps_stop(TCPIP_ADAPTER_IF_STA);
    err = tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_STA, &info);
#endif
  }
// set IP for access point
  else {
#if ESP_IDF_VERSION_MAJOR>=5
    esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_AP_DEF");
    if (netif != NULL) {
      esp_netif_dhcps_stop(netif);
      err = esp_netif_set_ip_info(netif, &info);
      esp_netif_dhcps_start(netif);
    }
#else
    tcpip_adapter_dhcps_stop(TCPIP_ADAPTER_IF_AP);
    err = tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_AP, &info);
    tcpip_adapter_dhcps_start(TCPIP_ADAPTER_IF_AP);
#endif
  }
// Schedule callback
  if (jsvIsFunction(jsCallback)) {
    JsVar *params[1];
    params[0] = err ? jsvNewWithFlags(JSV_NULL) : jsvNewFromString("Failure");
    jsiQueueEvents(NULL, jsCallback, params, 1);
    jsvUnLock(params[0]);
  }
  else {
    jsExceptionHere(JSET_ERROR, "Callback is not a function");
  }
  return ;
};


void jswrap_wifi_setIP(JsVar *jsSettings, JsVar *jsCallback) {
  setIP(jsSettings, jsCallback, WIFI_IF_STA);
  return ;
}

void jswrap_wifi_setAPIP(JsVar *jsSettings, JsVar *jsCallback) {
  setIP(jsSettings, jsCallback, WIFI_IF_AP);
  return ;
}
