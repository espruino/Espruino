// Includes from ESP-IDF
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "tcpip_adapter.h"

#include "jsinteractive.h"
#include "network.h"
#include "jswrap_modules.h"
#include "jswrap_esp32_network.h"


#include "jsutils.h"

#define UNUSED(x) (void)(x)

static void sendWifiCompletionCB(
  JsVar **g_jsCallback,  //!< Pointer to the global callback variable
  char  *reason          //!< NULL if successful, error string otherwise
);

// A callback function to be invoked on a disconnect response.
static JsVar *g_jsDisconnectCallback;

// A callback function to be invoked when we have an IP address.
static JsVar *g_jsGotIpCallback;

// A callback function to be invoked when we complete an access point scan.
static JsVar *g_jsScanCallback;

// A callback function to be invoked when we are being an access point.
static JsVar *g_jsAPStartedCallback;

// The last time we were connected as a station.
static system_event_sta_connected_t g_lastEventStaConnected;

// The last time we were disconnected as a station.
static system_event_sta_disconnected_t g_lastEventStaDisconnected;

// Are we connected as a station?
static bool g_isStaConnected = false;

#define EXPECT_CB_EXCEPTION(jsCB)   jsExceptionHere(JSET_ERROR, "Expecting callback function but got %v", jsCB)
#define EXPECT_OPT_EXCEPTION(jsOPT) jsExceptionHere(JSET_ERROR, "Expecting options object but got %t", jsOPT)

/*JSON{
  "type" : "event",
  "class" : "Wifi",
  "name" : "disconnected",
  "params" : [
    ["details","JsVar","An object with event details"]
  ]
}
The 'disconnected' event is called when an association with an access point has been lost.
The details include:

* ssid - The SSID of the access point from which the association was lost
* mac - The BSSID/mac address of the access point
* reason - The reason for the disconnection (string)

*/

/*JSON{
  "type" : "event",
  "class" : "Wifi",
  "name" : "associated",
  "params" : [
    ["details","JsVar","An object with event details"]
  ]
}
The 'connected' event is called when an association with an access point has succeeded, i.e., a
connection to the AP's network has been established. The details include:

* ssid - The SSID of the access point to which the association was established
* mac - The BSSID/mac address of the access point
* channel - The wifi channel used (an integer, typ 1..14)

*/

/*JSON{
  "type" : "event",
  "class" : "Wifi",
  "name" : "auth_change",
  "params" : [
    ["details","JsVar","An object with event details"]
  ]
}
The 'auth_change' event is called when the authentication mode with the associated access point changes.
The details include:

* oldMode - The old auth mode (string: open, wep, wpa, wpa2, wpa_wpa2)
* newMode - The new auth mode (string: open, wep, wpa, wpa2, wpa_wpa2)

*/

/*JSON{
  "type" : "event",
  "class" : "Wifi",
  "name" : "dhcp_timeout"
}
The 'dhcp_timeout' event is called when a DHCP request to the connected access point fails and
thus no IP address could be acquired (or renewed).
*/

/*JSON{
  "type" : "event",
  "class" : "Wifi",
  "name" : "connected",
  "params" : [
    ["details","JsVar","An object with event details"]
  ]
}
The 'connected' event is called when the connection with an access point is ready for traffic.
In the case of a dynamic IP address configuration this is when an IP address is obtained,
in the case of static IP address allocation this happens when an association is formed
(in that case the 'associated' and 'connected' events are fired in rapid succession).
The details include:

* ip - The IP address obtained as string
* netmask - The network's IP range mask as string
* gw - The network's default gateway as string

*/

/*JSON{
  "type" : "event",
  "class" : "Wifi",
  "name" : "sta_joined",
  "params" : [
    ["details","JsVar","An object with event details"]
  ]
}
The 'sta_joined' event is called when a station establishes an association
(i.e. connects) with the ESP32's access point.
The details include:

* mac - The MAC address of the station in string format (00:00:00:00:00:00)

*/

/*JSON{
  "type" : "event",
  "class" : "Wifi",
  "name" : "sta_left",
  "params" : [
    ["details","JsVar","An object with event details"]
  ]
}
The 'sta_left' event is called when a station disconnects from the ESP32's
access point (or its association times out?).
The details include:

* mac - The MAC address of the station in string format (00:00:00:00:00:00)

*/

/*JSON{
  "type" : "event",
  "class" : "Wifi",
  "name" : "probe_recv",
  "params" : [
    ["details","JsVar","An object with event details"]
  ]
}
The 'probe_recv' event is called when a probe request is received from some
station by the ESP32's access point. The details include:

* mac - The MAC address of the station in string format (00:00:00:00:00:00)
* rssi - The signal strength in dB of the probe request

*/


/**
 * Convert an wifi_auth_mode_t data type to a string value.
 */
static char *authModeToString(wifi_auth_mode_t authMode) {

  switch(authMode) {
  case WIFI_AUTH_OPEN:
    return "open";
  case WIFI_AUTH_WEP:
    return "wep";
  case WIFI_AUTH_WPA_PSK:
    return "wpa";
  case WIFI_AUTH_WPA2_PSK:
    return "wpa2";
  case WIFI_AUTH_WPA_WPA2_PSK:
    return "wpa_wpa2";
  }
  return "unknown";
} // End of authModeToString


/**
 * Convert a Wifi reason code to a string representation.
 */
static char *wifiReasonToString(uint8_t reason) {
  switch(reason) {
  case WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT:
    return "4WAY_HANDSHAKE_TIMEOUT";
  case WIFI_REASON_802_1X_AUTH_FAILED:
    return "802_1X_AUTH_FAILED";
  case WIFI_REASON_AKMP_INVALID:
    return "AKMP_INVALID";
  case WIFI_REASON_ASSOC_EXPIRE:
    return "ASSOC_EXPIRE";
  case WIFI_REASON_ASSOC_FAIL:
    return "ASSOC_FAIL";
  case WIFI_REASON_ASSOC_LEAVE:
    return "ASSOC_LEAVE";
  case WIFI_REASON_ASSOC_NOT_AUTHED:
    return "ASSOC_NOT_AUTHED";
  case WIFI_REASON_ASSOC_TOOMANY:
    return "ASSOC_TOOMANY";
  case WIFI_REASON_AUTH_EXPIRE:
    return "AUTH_EXPIRE";
  case WIFI_REASON_AUTH_FAIL:
    return "AUTH_FAIL";
  case WIFI_REASON_AUTH_LEAVE:
    return "AUTH_LEAVE";
  case WIFI_REASON_BEACON_TIMEOUT:
    return "BEACON_TIMEOUT";
  case WIFI_REASON_CIPHER_SUITE_REJECTED:
    return "CIPHER_SUITE_REJECTED";
  case WIFI_REASON_DISASSOC_PWRCAP_BAD:
    return "DISASSOC_PWRCAP_BAD";
  case WIFI_REASON_DISASSOC_SUPCHAN_BAD:
    return "DISASSOC_SUPCHAN_BAD";
  case WIFI_REASON_GROUP_CIPHER_INVALID:
    return "GROUP_CIPHER_INVALID";
  case WIFI_REASON_GROUP_KEY_UPDATE_TIMEOUT:
    return "GROUP_KEY_UPDATE_TIMEOUT";
  case WIFI_REASON_HANDSHAKE_TIMEOUT:
    return "HANDSHAKE_TIMEOUT";
  case WIFI_REASON_IE_INVALID:
    return "IE_INVALID";
  case WIFI_REASON_IE_IN_4WAY_DIFFERS:
    return "IE_IN_4WAY_DIFFERS";
  case WIFI_REASON_INVALID_RSN_IE_CAP:
    return "INVALID_RSN_IE_CAP";
  case WIFI_REASON_MIC_FAILURE:
    return "MIC_FAILURE";
  case WIFI_REASON_NOT_ASSOCED:
    return "NOT_ASSOCED";
  case WIFI_REASON_NOT_AUTHED:
    return "NOT_AUTHED";
  case WIFI_REASON_NO_AP_FOUND:
    return "NO_AP_FOUND";
  case WIFI_REASON_PAIRWISE_CIPHER_INVALID:
    return "PAIRWISE_CIPHER_INVALID";
  case WIFI_REASON_UNSPECIFIED:
    return "UNSPECIFIED";
  case WIFI_REASON_UNSUPP_RSN_IE_VERSION:
    return "REASON_UNSUPP_RSN_IE_VERSION";
  }
  jsWarn( "wifiReasonToString: Unknown reasonL %d", reason);
  return "Unknown reason";
} // End of wifiReasonToString

/**
 * Convery a wifi_mode_t data type to a string value.
 */
static char *wifiModeToString(wifi_mode_t mode) {
  switch(mode) {
  case WIFI_MODE_NULL:
    return "NULL";
  case WIFI_MODE_STA:
    return "STA";
  case WIFI_MODE_AP:
    return "AP";
  case WIFI_MODE_APSTA:
    return "APSTA";
  }
  return "UNKNOWN";
} // End of wifiModeToString


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
      jsvObjectSetChildAndUnLock(jsCurrentAccessPoint, "rssi", jsvNewFromInteger(list[i].rssi));
      jsvObjectSetChildAndUnLock(jsCurrentAccessPoint, "authMode", jsvNewFromString(authModeToString(list[i].authmode)));

      // The SSID may **NOT** be NULL terminated ... so handle that.
      char ssid[32 + 1];
      strncpy((char *)ssid, list[i].ssid, 32);
      ssid[32] = '\0';
      jsvObjectSetChildAndUnLock(jsCurrentAccessPoint, "ssid", jsvNewFromString(ssid));

          /*
          char macAddrString[6*3 + 1];
          os_sprintf(macAddrString, macFmt,
            bssInfo->bssid[0], bssInfo->bssid[1], bssInfo->bssid[2],
            bssInfo->bssid[3], bssInfo->bssid[4], bssInfo->bssid[5]);
          jsvObjectSetChildAndUnLock(jsCurrentAccessPoint, "mac", jsvNewFromString(macAddrString));
          */

      // Add the new record to the array
      jsvArrayPush(jsAccessPointArray, jsCurrentAccessPoint);
      jsvUnLock(jsCurrentAccessPoint);
    } // End of loop over each access point
    free(list);
  } // Number of access points > 0

  // We have now completed the scan callback, so now we can invoke the JS callback.
  JsVar *params[1];
  params[0] = jsAccessPointArray;
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
static char *wifiGetEvent(uint32_t event) {
  switch(event) {
  case SYSTEM_EVENT_AP_PROBEREQRECVED:
    return "#onprobe_recv";
  case SYSTEM_EVENT_AP_STACONNECTED:
    return "#onsta_joined";
  case SYSTEM_EVENT_AP_STADISCONNECTED:
    return "#onsta_left";
  case SYSTEM_EVENT_AP_START:
    break;
  case SYSTEM_EVENT_AP_STOP:
    break;
  case SYSTEM_EVENT_SCAN_DONE:
    break;
  case SYSTEM_EVENT_STA_AUTHMODE_CHANGE:
    return "#onauth_change";
  case SYSTEM_EVENT_STA_CONNECTED:
    return "#onassociated";
  case SYSTEM_EVENT_STA_DISCONNECTED:
    return "#ondisconnected";
  case SYSTEM_EVENT_STA_GOT_IP:
    return "#onconnected";
  case SYSTEM_EVENT_STA_START:
    break;
  case SYSTEM_EVENT_STA_STOP:
    break;
  case SYSTEM_EVENT_WIFI_READY:
    break;
  }
  jsWarn( "Unhandled wifi event type: %d", event);
  return NULL;
} // End of wifiGetEvent

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
  char *eventName = wifiGetEvent(eventType);
  if (eventName == NULL) {
    return;
  }

  jsiQueueObjectCallbacks(module, eventName, params, 1);
  jsvUnLock(module);
  return;
}

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
  if (event->event_id == SYSTEM_EVENT_STA_DISCONNECTED) {
    g_isStaConnected = false; // Flag us as disconnected
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
    jsvObjectSetChildAndUnLock(jsDetails, "ssid", jsvNewFromString(temp));
    sprintf(temp, MACSTR, MAC2STR(event->event_info.disconnected.bssid));
    jsvObjectSetChildAndUnLock(jsDetails, "mac", jsvNewFromString(temp));
    sprintf(temp, "%d", event->event_info.disconnected.reason);
    jsvObjectSetChildAndUnLock(jsDetails, "reason", jsvNewFromString(temp));
    sendWifiEvent(event->event_id, jsDetails);
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
    jsvObjectSetChildAndUnLock(jsDetails, "ssid", jsvNewFromString(temp));
    sprintf(temp, MACSTR, MAC2STR(event->event_info.connected.bssid));
    jsvObjectSetChildAndUnLock(jsDetails, "mac", jsvNewFromString(temp));
    sprintf(temp, "%d", event->event_info.connected.channel);
    jsvObjectSetChildAndUnLock(jsDetails, "channel", jsvNewFromString(temp));
    sendWifiEvent(event->event_id, jsDetails);
    return ESP_OK;
  } // End of handle SYSTEM_EVENT_STA_CONNECTED


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
    jsvObjectSetChildAndUnLock(jsDetails, "ip", jsvNewFromString(temp));
    sprintf(temp, IPSTR, IP2STR(&event->event_info.got_ip.ip_info.netmask));
    jsvObjectSetChildAndUnLock(jsDetails, "netmask", jsvNewFromString(temp));
    sprintf(temp, IPSTR, IP2STR(&event->event_info.got_ip.ip_info.gw));
    jsvObjectSetChildAndUnLock(jsDetails, "gw", jsvNewFromString(temp));

    sendWifiEvent(event->event_id, jsDetails);
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
    sprintf(temp, MACSTR, MAC2STR(&event->event_info.sta_connected.mac));
    jsvObjectSetChildAndUnLock(jsDetails, "mac", jsvNewFromString(temp));
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
    sprintf(temp, MACSTR, MAC2STR(&event->event_info.sta_disconnected.mac));
    jsvObjectSetChildAndUnLock(jsDetails, "mac", jsvNewFromString(temp));
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
    return ESP_OK;
  }

  /**
   * SYSTEM_EVENT_AP_START
   * Called when we have started being an access point.
   */
  if (event->event_id == SYSTEM_EVENT_AP_START) {
    sendWifiCompletionCB(&g_jsAPStartedCallback, NULL);
    return ESP_OK;
  }

  return ESP_OK;
} // End of event_handler


/**
 * Initialize the one time ESP32 wifi components including the event
 * handler.
 */
void esp32_wifi_init() {
  ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL));
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));
  ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_FLASH));
  
} // End of esp32_wifi_init


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
  // Check that we have a callback function.
  if (!jsvIsFunction(*g_jsCallback)){
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
   "type": "library",
   "class": "Wifi"
}
The wifi library is a generic cross-platform library to control the Wifi interface.  It supports functionality such as connecting to wifi networks, getting network information, starting and access point, etc.
**Currently this library is ESP8226 and ESP32 specific** and needs to be ported to other Espruino platforms.

To get started and connect to your local access point all you need is
```
var wifi = require("Wifi");
wifi.connect("my-ssid", {password:"my-pwd"}, function(ap){ console.log("connected:", ap); });
```
If you want the connection to happen automatically at boot, add `wifi.save();`.

*/

/**
 * Perform a soft initialization of ESP32 networking.
 */
void jswrap_ESP32_wifi_soft_init() {
  JsNetwork net;
  networkCreate(&net, JSNETWORKTYPE_ESP32); // Set the network type to be ESP32
  networkState = NETWORKSTATE_ONLINE; // Set the global state of the networking to be online
}


/*JSON{
  "type"     : "staticmethod",
  "class"    : "Wifi",
  "name"     : "disconnect",
  "generate" : "jswrap_ESP32_wifi_disconnect",
  "params"   : [
    ["callback", "JsVar", "An optional function to be called back on disconnection. The callback function receives no argument."]
  ]
}
Disconnect the wifi station from an access point and disable the station mode. It is OK
to call `disconnect` to turn off station mode even if no connection exists (for example,
connection attempts may be failing). Station mode can be re-enabled by calling `connect` or `scan`.
*/
void jswrap_ESP32_wifi_disconnect(JsVar *jsCallback) {
  // We save the callback function so that it can subsequently invoked.  Then we execute the
  // ESP-IDF function to disconnect us from the access point.  The thinking is that will result
  // in a subsequent event which we will detect and use to call the callback.
  //
  // Free any existing callback, then register new callback
  if (g_jsDisconnectCallback != NULL) jsvUnLock(g_jsDisconnectCallback);
  g_jsDisconnectCallback = NULL;

  // Check that the callback is a good callback if supplied.
  if (jsCallback != NULL && !jsvIsUndefined(jsCallback) && !jsvIsFunction(jsCallback)) {
    EXPECT_CB_EXCEPTION(jsCallback);
    return;
  }

  // Save the callback for later execution.
  g_jsDisconnectCallback = jsvLockAgainSafe(jsCallback);

  // Call the ESP-IDF to disconnect us from the access point.
  esp_wifi_disconnect();
} // End of jswrap_ESP32_wifi_disconnect


/*JSON{
  "type"     : "staticmethod",
  "class"    : "Wifi",
  "name"     : "stopAP",
  "generate" : "jswrap_ESP32_wifi_stopAP",
  "params"   : [
    ["callback", "JsVar", "An optional function to be called back on successful stop. The callback function receives no argument."]
  ]
}
Stop being an access point and disable the AP operation mode. Ap mode can be
re-enabled by calling `startAP`.
*/
void jswrap_ESP32_wifi_stopAP(JsVar *jsCallback) {
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
    jsWarn("jswrap_ESP32_wifi_stopAP: esp_wifi_set_mode rc=%d", err);
  }

  if (jsvIsFunction(jsCallback)) {
    jsiQueueEvents(NULL, jsCallback, NULL, 0);
  }
} // End of jswrap_ESP32_wifi_stopAP


/*JSON{
  "type"     : "staticmethod",
  "class"    : "Wifi",
  "name"     : "connect",
  "generate" : "jswrap_ESP32_wifi_connect",
  "params"   : [
    ["ssid", "JsVar", "The access point network id."],
    ["options", "JsVar", "Connection options (optional)."],
    ["callback", "JsVar", "A function to be called back on completion (optional)."]
  ]
}
Connect to an access point as a station. If there is an existing connection to an AP it is first disconnected if the SSID or password are different from those passed as parameters. Put differently, if the passed SSID and password are identical to the currently connected AP then nothing is changed.
When the connection attempt completes the callback function is invoked with one `err` parameter, which is NULL if there is no error and a string message if there is an error. If DHCP is enabled the callback occurs once an IP addres has been obtained, if a static IP is set the callback occurs once the AP's network has been joined.  The callback is also invoked if a connection already exists and does not need to be changed.
The `options` properties may contain:
* `password` - Password string to be used to access the network.
* `dnsServers` (array of String) - An array of up to two DNS servers in dotted decimal format string.

Notes
* the options should include the ability to set a static IP and associated netmask and gateway, this is a future enhancement.
* the only error reported in the callback is "Bad password", all other errors (such as access point not found or DHCP timeout) just cause connection retries. If the reporting of such temporary errors is desired, the caller must use its own timeout and the `getDetails().status` field.
* the `connect` call automatically enables station mode, it can be disabled again by calling `disconnect`.
*/
void jswrap_ESP32_wifi_connect(
    JsVar *jsSsid,
    JsVar *jsOptions,
    JsVar *jsCallback
  ) {

  // Check that the ssid value isn't obviously in error.
  if (!jsvIsString(jsSsid)) {
    jsExceptionHere(JSET_ERROR, "No SSID provided");
    return;
  }

  // Create SSID string
  char ssid[33];
  size_t len = jsvGetString(jsSsid, ssid, sizeof(ssid)-1);
  ssid[len]='\0';

  // Make sure jsOptions is NULL or an object
  if (jsOptions != NULL && !jsvIsObject(jsOptions)) {
    EXPECT_OPT_EXCEPTION(jsOptions);
    return;
  }

  // Check callback
  if (g_jsGotIpCallback != NULL) jsvUnLock(g_jsGotIpCallback);
  g_jsGotIpCallback = NULL;
  if (jsCallback != NULL && !jsvIsUndefined(jsCallback) && !jsvIsFunction(jsCallback)) {
    EXPECT_CB_EXCEPTION(jsCallback);
    return;
  }

  // Clear disconnect callback to prevent disconnection from disabling station mode
  if (g_jsDisconnectCallback != NULL) jsvUnLock(g_jsDisconnectCallback);
  g_jsDisconnectCallback = NULL;

  // Get the optional password
  char password[65];
  memset(password, 0, sizeof(password));
  if (jsOptions != NULL) {
    JsVar *jsPassword = jsvObjectGetChild(jsOptions, "password", 0);
    if (jsPassword != NULL && !jsvIsString(jsPassword)) {
      jsExceptionHere(JSET_ERROR, "Expecting options.password to be a string but got %t", jsPassword);
      jsvUnLock(jsPassword);
      return;
    }
    if (jsPassword != NULL) {
      size_t len = jsvGetString(jsPassword, password, sizeof(password)-1);
      password[len]='\0';
    } else {
      password[0] = '\0';
    }
    jsvUnLock(jsPassword);
  } // End of we had options

  // At this point, we have the ssid in "ssid" and the password in "password".
  // Perform an esp_wifi_set_mode
  wifi_mode_t mode;
  esp_err_t err;
  err = esp_wifi_get_mode(&mode);
  if (err != ESP_OK) {
    jsError( "jswrap_ESP32_wifi_connect: esp_wifi_get_mode: %d", err);
    return;
  }
  switch(mode) {
  case WIFI_MODE_NULL:
  case WIFI_MODE_STA:
    mode = WIFI_MODE_STA;
    break;
  case WIFI_MODE_APSTA:
  case WIFI_MODE_AP:
    mode = WIFI_MODE_APSTA;
    break;
  default:
    jsError( "jswrap_ESP32_wifi_connect: Unexpected mode type: %d", mode);
    break;
  }

  err = esp_wifi_set_mode(mode);
  if (err != ESP_OK) {
    jsError( "jswrap_ESP32_wifi_connect: esp_wifi_set_mode: %d, mode=%d", err, mode);
    return;
  }

  // Perform a an esp_wifi_set_config
  wifi_config_t staConfig;
  memcpy(staConfig.sta.ssid, ssid, sizeof(staConfig.sta.ssid));
  memcpy(staConfig.sta.password, password, sizeof(staConfig.sta.password));
  staConfig.sta.bssid_set = false;
  esp_wifi_set_auto_connect(false); // turn off default behaviour 
  err = esp_wifi_set_config(WIFI_IF_STA,  &staConfig);
  if (err != ESP_OK) {
    jsError( "jswrap_ESP32_wifi_connect: esp_wifi_set_config: %d", err);
    return;
  }

  // Perform an esp_wifi_start
  err = esp_wifi_start();
  if (err != ESP_OK) {
    jsError( "jswrap_ESP32_wifi_connect: esp_wifi_start: %d", err);
    return;
  }

  // Save the callback for later execution.
  g_jsGotIpCallback = jsvLockAgainSafe(jsCallback);

  // Perform an esp_wifi_connect
  err = esp_wifi_connect();
  if (err != ESP_OK) {
    jsError( "jswrap_ESP32_wifi_connect: esp_wifi_connect: %d", err);
    return;
  }
}


/*JSON{
  "type"     : "staticmethod",
  "class"    : "Wifi",
  "name"     : "scan",
  "generate" : "jswrap_ESP32_wifi_scan",
  "params"   : [
    ["callback", "JsVar", "A function to be called back on completion."]
  ]
}
Perform a scan for access points. This will enable the station mode if it is not currently
enabled. Once the scan is complete the callback function is called with an array of APs
found, each AP is an object with:
* `ssid`: SSID string.
* `mac`: access point MAC address in 00:00:00:00:00:00 format.
* `authMode`: `open`, `wep`, `wpa`, `wpa2`, or `wpa_wpa2`.
* `channel`: wifi channel 1..13.
* `hidden`: true if the SSID is hidden.
* `rssi`: signal strength in dB in the range -110..0.
For the ESP32, the return values are:
* `ssid`: SSID string.
* `authMode`
* `rssi`: Signal strength.
Notes:
* in order to perform the scan the station mode is turned on and remains on, use Wifi.disconnect()
* to turn it off again, if desired. Only one scan can be in progress at a time.
*/
void jswrap_ESP32_wifi_scan(JsVar *jsCallback) {
  // If we have a saved scan callback function we must be scanning already
  if (g_jsScanCallback != NULL) {
    jsExceptionHere(JSET_ERROR, "A scan is already in progress.");
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
    jsError( "jswrap_ESP32_wifi_scan: esp_wifi_get_mode: %d", err);
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
    jsError( "jswrap_ESP32_wifi_scan: esp_wifi_set_mode: %d", err);
    return;
  }

  // Perform an esp_wifi_start
  err = esp_wifi_start();
  if (err != ESP_OK) {
    jsError( "jswrap_ESP32_wifi_connect: esp_wifi_start: %d", err);
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
} // End of jswrap_ESP32_wifi_scan


/*JSON{
  "type"     : "staticmethod",
  "class"    : "Wifi",
  "name"     : "startAP",
  "generate" : "jswrap_ESP32_wifi_startAP",
  "params"   : [
    ["ssid", "JsVar", "The network id."],
    ["options", "JsVar", "Configuration options (optional)."],
    ["callback", "JsVar", "Optional function to be called when the AP is successfully started."]
  ]
}
Create a WiFi access point allowing stations to connect. If the password is NULL or an empty string the access point is open, otherwise it is encrypted.
The callback function is invoked once the access point is set-up and receives one `err` argument, which is NULL on success and contains an error message string otherwise.
The `options` object can contain the following properties.
* `authMode` - The authentication mode to use.  Can be one of "open", "wpa2", "wpa", "wpa_wpa2". The default is open (but open access points are not recommended).
* `password` - The password for connecting stations if authMode is not open.
* `channel` - The channel to be used for the access point in the range 1..13. If the device is also connected to an access point as a station then that access point determines the channel.
Notes:
* the options should include the ability to set the AP IP and associated netmask, this is a future enhancement.
* the `startAP` call automatically enables AP mode. It can be disabled again by calling `stopAP`.
*/
void jswrap_ESP32_wifi_startAP(
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
      jsExceptionHere(JSET_ERROR, "No SSID.");
    return;
  }

  // Make sure jsOptions is NULL or an object
  if (jsOptions != NULL && !jsvIsNull(jsOptions) && !jsvIsObject(jsOptions)) {
    EXPECT_OPT_EXCEPTION(jsOptions);
    return;
  }

  // Check callback
  if (g_jsAPStartedCallback != NULL) jsvUnLock(g_jsAPStartedCallback);
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
    JsVar *jsChan = jsvObjectGetChild(jsOptions, "channel", 0);
    if (jsvIsInt(jsChan)) {
      int chan = jsvGetInteger(jsChan);
      if (chan >= 1 && chan <= 13) {
        apConfig.channel = (uint8_t)chan;
      }
    }
    jsvUnLock(jsChan);

    // Handle password
    JsVar *jsPassword = jsvObjectGetChild(jsOptions, "password", 0);
    if (jsPassword != NULL) {
      if (!jsvIsString(jsPassword) || jsvGetStringLength(jsPassword) < 8) {
        jsExceptionHere(JSET_ERROR, "Password must be string of at least 8 characters");
        jsvUnLock(jsPassword);
        return;
      }
      size_t len = jsvGetString(jsPassword, (char *)apConfig.password, sizeof(apConfig.password)-1);
      apConfig.password[len] = '\0';
    }

    // Handle the authMode
    JsVar *jsAuth = jsvObjectGetChild(jsOptions, "authMode", 0);
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
        jsExceptionHere(JSET_ERROR, "Unknown authMode value.");
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
      jsExceptionHere(JSET_ERROR, "Password not set but authMode not open.");
      return;
    }

    // Make sure that if authmode is explicitly open then there is NO password supplied.
    if (apConfig.authmode == WIFI_AUTH_OPEN && strlen(apConfig.password) > 0) {
      jsExceptionHere(JSET_ERROR, "Auth mode set to open but password supplied.");
      return;
    }
  } // End we have an options structure

  // Set the mode to be accesss point
  // FIX ... we can't hard code this to be just an access point.
  esp_err_t err;

  // set callback
  if (jsvIsFunction(jsCallback)) {
    g_jsAPStartedCallback = jsvLockAgainSafe(jsCallback);
  }

  err = esp_wifi_set_mode(WIFI_MODE_AP);
  if (err != ESP_OK) {
    jsError( "jswrap_ESP32_wifi_startAP: esp_wifi_set_mode: %d", err);
    return;
  }

  err = esp_wifi_set_config(WIFI_IF_AP, (wifi_config_t *)&apConfig);
  if (err != ESP_OK) {
    jsError( "jswrap_ESP32_wifi_startAP: wifi_set_config: %d - ssid=%.*s, password=%s, authMode=%d, maxConnections=%d, beacon=%d, channel=%d",
      err, apConfig.ssid_len, apConfig.ssid, apConfig.password, apConfig.authmode, apConfig.max_connection, apConfig.beacon_interval, apConfig.channel);
    return;
  }

  // Perform an esp_wifi_start
  err = esp_wifi_start();
  if (err != ESP_OK) {
    jsError( "jswrap_ESP32_wifi_startAP: esp_wifi_start: %d", err);
    return;
  }
} // End of jswrap_ESP32_wifi_startAP


/*JSON{
  "type"     : "staticmethod",
  "class"    : "Wifi",
  "name"     : "getStatus",
  "generate" : "jswrap_ESP32_wifi_getStatus",
  "return"   : ["JsVar", "An object representing the current WiFi status, if available immediately."],
  "params"   : [
    ["callback", "JsVar", "An optional function to be called back with the current Wifi status, i.e. the same object as returned directly. The callback function is more portable than the direct return value."]
  ]
}
Retrieve the current overall WiFi configuration. This call provides general information that pertains to both station and access point modes. The getDetails and getAPDetails calls provide more in-depth information about the station and access point configurations, respectively. The status object has the following properties:
* `station` - Status of the wifi station: `off`, `connecting`, ...
* `ap` - Status of the wifi access point: `disabled`, `enabled`.
* `mode` - The current operation mode: `off`, `sta`, `ap`, `sta+ap`.
* `phy` - Modulation standard configured: `11b`, `11g`, `11n` (the esp8266 docs are not very clear, but it is assumed that 11n means b/g/n). This setting limits the modulations that the radio will use, it does not indicate the current modulation used with a specific access point.
* `powersave` - Power saving mode: `none` (radio is on all the time), `ps-poll` (radio is off between beacons as determined by the access point's DTIM setting). Note that in 'ap' and 'sta+ap' modes the radio is always on, i.e., no power saving is possible.
* `savedMode` - The saved operation mode which will be applied at boot time: `off`, `sta`, `ap`, `sta+ap`.
*/
JsVar *jswrap_ESP32_wifi_getStatus(JsVar *jsCallback) {
  UNUSED(jsCallback);
  // We have to determine the following information:
  //
  // - [    ] The status of the station interface
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

  char *modeStr;
  switch(mode) {
  case WIFI_MODE_NULL:
    modeStr = "off";
    break;
  case WIFI_MODE_AP:
    modeStr = "ap";
    break;
  case WIFI_MODE_STA:
    modeStr = "sta";
    break;
  case WIFI_MODE_APSTA:
    modeStr ="sta+ap";
    break;
  default:
    modeStr = "unknown";
    break;
  }

  // Get the current power save type
  wifi_ps_type_t psType;
  esp_wifi_get_ps(&psType);
  char *psTypeStr;
  switch(psType) {
  case WIFI_PS_MODEM:
    psTypeStr = "modem";
    break;
  case WIFI_PS_NONE:
    psTypeStr = "none";
    break;
  default:
    psTypeStr = "unknown";
    break;
  }

  JsVar *jsWiFiStatus = jsvNewObject();
  if (g_isStaConnected) {
    jsvObjectSetChildAndUnLock(jsWiFiStatus, "station", jsvNewFromString("connected"));
  } else {
    jsvObjectSetChildAndUnLock(jsWiFiStatus, "station",
        jsvNewFromString(wifiReasonToString(g_lastEventStaDisconnected.reason)));
  }
  jsvObjectSetChildAndUnLock(jsWiFiStatus, "mode", jsvNewFromString(modeStr));
  jsvObjectSetChildAndUnLock(jsWiFiStatus, "powersave", jsvNewFromString(psTypeStr));
  return jsWiFiStatus;
} // End of jswrap_ESP32_wifi_getStatus


/*JSON{
  "type"     : "staticmethod",
  "class"    : "Wifi",
  "name"     : "setConfig",
  "generate" : "jswrap_ESP32_wifi_setConfig",
  "params"   : [
    ["settings", "JsVar", "An object with the configuration settings to change."]
  ]
}
Sets a number of global wifi configuration settings. All parameters are optional and which are passed determines which settings are updated.
The settings available are:
* `phy` - Modulation standard to allow: `11b`, `11g`, `11n` (the esp32 docs are not very clear, but it is assumed that 11n means b/g/n).
* `powersave` - Power saving mode: `none` (radio is on all the time), `ps-poll` (radio is off between beacons as determined by the access point's DTIM setting). Note that in 'ap' and 'sta+ap' modes the radio is always on, i.e., no power saving is possible.
Note: esp32 SDK programmers may be missing an "opmode" option to set the sta/ap/sta+ap operation mode. Please use connect/scan/disconnect/startAP/stopAP, which all set the esp32 opmode indirectly.
*/
void jswrap_ESP32_wifi_setConfig(JsVar *jsSettings) {
  UNUSED(jsSettings);
  jsError( "jswrap_ESP32_wifi_setConfig - Not implemented");
} // End of jswrap_ESP32_wifi_setConfig


/*JSON{
  "type"     : "staticmethod",
  "class"    : "Wifi",
  "name"     : "getDetails",
  "generate" : "jswrap_ESP32_wifi_getDetails",
  "return"   : ["JsVar", "An object representing the wifi station details, if available immediately."],
  "params"   : [
    ["callback", "JsVar", "An optional function to be called back with the wifi details, i.e. the same object as returned directly. The callback function is more portable than the direct return value."]
  ]
}
Retrieve the wifi station configuration and status details. The details object has the following properties:
* `status` - Details about the wifi station connection, one of `off`, `connecting`, `wrong_password`, `no_ap_found`, `connect_fail`, or `connected`. The off, bad_password and connected states are stable, the other states are transient. The connecting state will either result in connected or one of the error states (bad_password, no_ap_found, connect_fail) and the no_ap_found and connect_fail states will result in a reconnection attempt after some interval.
* `rssi` - signal strength of the connected access point in dB, typically in the range -110 to 0, with anything greater than -30 being an excessively strong signal.
* `ssid` - SSID of the access point.
* `password` - the password used to connect to the access point.
* `authMode` - the authentication used: `open`, `wpa`, `wpa2`, `wpa_wpa2` (not currently supported).
* `savedSsid` - the SSID to connect to automatically at boot time, null if none.
*/
JsVar *jswrap_ESP32_wifi_getDetails(JsVar *jsCallback) {
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
    jsvObjectSetChildAndUnLock(jsDetails, "ssid", jsvNewFromString(buf));

    // password
    strncpy(buf, (char *)config.password, 64);
    buf[64] = 0;
    jsvObjectSetChildAndUnLock(jsDetails, "password", jsvNewFromString((char *)config.password));

    // Status
    jsvObjectSetChildAndUnLock(jsDetails, "status", jsvNewFromString("connected"));

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
} // End of jswrap_ESP32_wifi_getDetails


/*JSON{
  "type"     : "staticmethod",
  "class"    : "Wifi",
  "name"     : "getAPDetails",
  "generate" : "jswrap_ESP32_wifi_getAPDetails",
  "return"   : ["JsVar", "An object representing the current access point details, if available immediately."],
  "params"   : [
    ["callback", "JsVar", "An optional function to be called back with the current access point details, i.e. the same object as returned directly. The callback function is more portable than the direct return value."]
  ]
}
Retrieve the current access point configuration and status.  The details object has the following properties:
* `status` - Current access point status: `enabled` or `disabled`
* `stations` - an array of the stations connected to the access point.  This array may be empty.  Each entry in the array is an object describing the station which, at a minimum contains `ip` being the IP address of the station.
* `ssid` - SSID to broadcast.
* `password` - Password for authentication.
* `authMode` - the authentication required of stations: `open`, `wpa`, `wpa2`, `wpa_wpa2`.
* `hidden` - True if the SSID is hidden, false otherwise.
* `maxConn` - Max number of station connections supported.
* `savedSsid` - the SSID to broadcast automatically at boot time, null if the access point is to be disabled at boot.
*/
JsVar *jswrap_ESP32_wifi_getAPDetails(JsVar *jsCallback) {
  // Check callback
  if (jsCallback != NULL && !jsvIsNull(jsCallback) && !jsvIsFunction(jsCallback)) {
    EXPECT_CB_EXCEPTION(jsCallback);
    return NULL;
  }

  JsVar *jsDetails = jsvNewObject();

  wifi_ap_config_t config;
  esp_wifi_get_config(WIFI_IF_AP, (wifi_config_t *)&config);
  jsvObjectSetChildAndUnLock(jsDetails, "authMode", jsvNewFromString(authModeToString(config.authmode)));
  jsvObjectSetChildAndUnLock(jsDetails, "hidden",   jsvNewFromBool(config.ssid_hidden));
  jsvObjectSetChildAndUnLock(jsDetails, "maxConn",  jsvNewFromInteger(config.max_connection));

  char buf[65];

  // ssid
  strncpy(buf, (char *)config.ssid, 32);
  buf[32] = 0;
  jsvObjectSetChildAndUnLock(jsDetails, "ssid", jsvNewFromString(buf));

  // password
  strncpy(buf, (char *)config.password, 64);
  buf[64] = 0;
  jsvObjectSetChildAndUnLock(jsDetails, "password", jsvNewFromString((char *)config.password));

  // Schedule callback if a function was provided
  if (jsvIsFunction(jsCallback)) {
    JsVar *params[1];
    params[0] = jsDetails;
    jsiQueueEvents(NULL, jsCallback, params, 1);
  }
  return jsDetails;
} // End of jswrap_ESP32_wifi_getAPDetails


/*JSON{
  "type"     : "staticmethod",
  "class"    : "Wifi",
  "name"     : "save",
  "generate" : "jswrap_ESP32_wifi_save",
  "params"   : [
    ["what", "JsVar", "An optional parameter to specify what to save, on the esp8266 the two supported values are `clear` and `sta+ap`. The default is `sta+ap`"]
  ]
}
Save the current wifi configuration (station and access point) and automatically apply this configuration at boot time, unless `what=="clear"`, in which case the saved configuration is cleared such that wifi remains disabled at boot. The saved configuration includes:
* mode (off/sta/ap/sta+ap)
* SSIDs & passwords
* phy (11b/g/n)
* powersave setting
* DHCP hostname
*/
void jswrap_ESP32_wifi_save(JsVar *what) {
  if (jsvIsString(what) && jsvIsStringEqual(what, "clear")) {
	esp_wifi_set_auto_connect(false);
  } else {
	esp_wifi_set_auto_connect(true);
  } 
} // End of jswrap_ESP32_wifi_save


/*JSON{
  "type"     : "staticmethod",
  "class"    : "Wifi",
  "name"     : "restore",
  "generate" : "jswrap_ESP32_wifi_restore"
}
Restores the saved Wifi configuration. See `Wifi.save()`.
*/
void jswrap_ESP32_wifi_restore(void) {
  bool auto_connect;
  int err=esp_wifi_get_auto_connect(&auto_connect);
  
  if ( auto_connect ) {
    err = esp_wifi_start();
    if (err != ESP_OK) {
      jsError( "jswrap_ESP32_wifi_restore: esp_wifi_start: %d", err);
    }	
    
	wifi_mode_t mode;
    err = esp_wifi_get_mode(&mode);
    if ( (  mode == WIFI_MODE_STA ) || (  mode == WIFI_MODE_APSTA ) ) {
      // Perform an esp_wifi_start
      err = esp_wifi_connect();
      if (err != ESP_OK) {
        jsError( "jswrap_ESP32_wifi_restore: esp_wifi_connect: %d", err - ESP_ERR_WIFI_BASE);
        return;
	  }
    }
  } else {
    // No previous wifi.save()
  }

} // End of jswrap_ESP32_wifi_restore


/**
 * Get the ip info for the given interface.  The interfaces are:
 * * TCPIP_ADAPTER_IF_STA - Station
 * * TCPIP_ADAPTER_IF_AP - Access Point
 */
static JsVar *getIPInfo(JsVar *jsCallback, tcpip_adapter_if_t interface) {
  // Check callback
  if (jsCallback != NULL && !jsvIsNull(jsCallback) && !jsvIsFunction(jsCallback)) {
    EXPECT_CB_EXCEPTION(jsCallback);
    return NULL;
  }

  // first get IP address info, this may fail if we're not connected
  tcpip_adapter_ip_info_t ipInfo;
  esp_err_t err = tcpip_adapter_get_ip_info(interface, &ipInfo);
  JsVar *jsIpInfo = jsvNewObject();
  if (err == ESP_OK) {
    jsvObjectSetChildAndUnLock(jsIpInfo, "ip",
      networkGetAddressAsString((uint8_t *)&ipInfo.ip, 4, 10, '.'));
    jsvObjectSetChildAndUnLock(jsIpInfo, "netmask",
      networkGetAddressAsString((uint8_t *)&ipInfo.netmask, 4, 10, '.'));
    jsvObjectSetChildAndUnLock(jsIpInfo, "gw",
      networkGetAddressAsString((uint8_t *)&ipInfo.gw, 4, 10, '.'));
  }

  // now get MAC address (which always succeeds)
  uint8_t macAddr[6];
  esp_wifi_get_mac(interface==TCPIP_ADAPTER_IF_STA?WIFI_IF_STA:WIFI_IF_AP, macAddr);
  char macAddrString[6*3 + 1];
  sprintf(macAddrString, MACSTR, MAC2STR(macAddr));
  jsvObjectSetChildAndUnLock(jsIpInfo, "mac", jsvNewFromString(macAddrString));

  // Schedule callback if a function was provided
  if (jsvIsFunction(jsCallback)) {
    JsVar *params[1];
    params[0] = jsIpInfo;
    jsiQueueEvents(NULL, jsCallback, params, 1);
  }

  return jsIpInfo;
} // End of getIPInfo


/*JSON{
  "type"     : "staticmethod",
  "class"    : "Wifi",
  "name"     : "getIP",
  "generate" : "jswrap_ESP32_wifi_getIP",
  "return"   : ["JsVar", "An object representing the station IP information, if available immediately."],
  "params"   : [
    ["callback", "JsVar", "An optional function to be called back with the IP information, i.e. the same object as returned directly. The callback function is more portable than the direct return value."]
  ]
}
Return the station IP information in an object as follows:
* ip - IP address as string (e.g. "192.168.1.5")
* netmask - The interface netmask as string
* gw - The network gateway as string
* mac - The MAC address as string of the form 00:00:00:00:00:00
Note that the `ip`, `netmask`, and `gw` fields are omitted if no connection is established:
*/
JsVar *jswrap_ESP32_wifi_getIP(JsVar *jsCallback) {
  JsVar *jsIpInfo = getIPInfo(jsCallback, TCPIP_ADAPTER_IF_STA);
  return jsIpInfo;
} // End of jswrap_ESP32_wifi_getIP


/*JSON{
  "type"     : "staticmethod",
  "class"    : "Wifi",
  "name"     : "getAPIP",
  "generate" : "jswrap_ESP32_wifi_getAPIP",
  "return"   : ["JsVar", "An object representing the esp8266's Access Point IP information, if available immediately."],
  "params"   : [
    ["callback", "JsVar", "An optional function to be called back with the the IP information, i.e. the same object as returned directly. The callback function is more portable than the direct return value."]
  ]
}
Return the access point IP information in an object which contains:
* ip - IP address as string (typ "192.168.4.1")
* netmask - The interface netmask as string
* gw - The network gateway as string
* mac - The MAC address as string of the form 00:00:00:00:00:00
*/
JsVar *jswrap_ESP32_wifi_getAPIP(JsVar *jsCallback) {
  JsVar *jsIpInfo = getIPInfo(jsCallback, TCPIP_ADAPTER_IF_AP);
  return jsIpInfo;
}


/*JSON{
  "type"     : "staticmethod",
  "class"    : "Wifi",
  "name"     : "getHostByName",
  "generate" : "jswrap_ESP32_wifi_getHostByName",
    "params"   : [
    ["hostname", "JsVar", "The hostname to lookup."],
    ["callback", "JsVar", "The callback to invoke when the hostname is returned."]
  ]
}
Lookup the hostname and invoke a callback with the IP address as integer argument. If the lookup fails, the callback is invoked with a null argument.
**Note:** only a single hostname lookup can be made at a time, concurrent lookups are not supported.
*/
void jswrap_ESP32_wifi_getHostByName(
    JsVar *jsHostname,
    JsVar *jsCallback
) {
  UNUSED(jsHostname);
  UNUSED(jsCallback);
  jsError( "jswrap_ESP32_wifi_getHostByName - Not implemented - no api in esp-idf");
  // Could use net_esp32_gethostbyname in network_esp32.c
}



/*JSON{
  "type"     : "staticmethod",
  "class"    : "Wifi",
  "name"     : "getHostname",
  "generate" : "jswrap_ESP32_wifi_getHostname",
  "return"   : ["JsVar", "The currently configured hostname, if available immediately."],
  "params"   : [
    ["callback", "JsVar", "An optional function to be called back with the hostname, i.e. the same string as returned directly. The callback function is more portable than the direct return value."]
  ]
}
Returns the hostname announced to the DHCP server and broadcast via mDNS when connecting to an access point.
*/
JsVar *jswrap_ESP32_wifi_getHostname(JsVar *jsCallback) {
  UNUSED(jsCallback);
  jsError( "jswrap_ESP32_wifi_getHostname - Not implemented");
  return NULL;
}

/*JSON{
  "type"     : "staticmethod",
  "class"    : "Wifi",
  "name"     : "setHostname",
  "generate" : "jswrap_ESP32_wifi_setHostname",
  "params"   : [
    ["hostname", "JsVar", "The new hostname."]
  ]
}
Set the hostname. Depending on implemenation, the hostname is sent with every DHCP request and is broadcast via mDNS. The DHCP hostname may be visible in the access point and may be forwarded into DNS as hostname.local.
If a DHCP lease currently exists changing the hostname will cause a disconnect and reconnect in order to transmit the change to the DHCP server.
The mDNS announcement also includes an announcement for the "espruino" service.
*/
void jswrap_ESP32_wifi_setHostname(
    JsVar *jsHostname //!< The hostname to set for device.
) {
  UNUSED(jsHostname);
  jsError( "jswrap_ESP32_wifi_setHostname - Not implemented");
}


/*JSON{
  "type"     : "staticmethod",
  "class"    : "ESP32",
  "name"     : "ping",
  "generate" : "jswrap_ESP32_ping",
  "params"   : [
    ["ipAddr", "JsVar", "A string representation of an IP address."],
    ["pingCallback", "JsVar", "Optional callback function."]
  ]
}
Perform a network ping request. The parameter can be either a String or a numeric IP address.
**Note:** This function should probably be removed, or should it be part of the wifi library?
*/
void jswrap_ESP32_ping(
    JsVar *ipAddr,      //!< A string or integer representation of an IP address.
    JsVar *pingCallback //!< Optional callback function.
) {
  UNUSED(ipAddr);
  UNUSED(pingCallback);
  jsError( "jswrap_ESP32_ping - Not implemented");  
}
