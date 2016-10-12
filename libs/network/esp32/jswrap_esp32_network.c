// Includes from ESP-IDF
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "tcpip_adapter.h"


#include "jsinteractive.h"
#include "network.h"
#include "jswrap_esp32_network.h"

static void sendWifiCompletionCB(
  JsVar **g_jsCallback,  //!< Pointer to the global callback variable
  char  *reason          //!< NULL if successful, error string otherwise
);

// Tag for ESP-IDF logging
static char *tag = "jswrap_esp32_network";

// A callback function to be invoked on a disconnect response.
static JsVar *g_jsDisconnectCallback;

// A callback function to be invoked when we have an IP address.
static JsVar *g_jsGotIpCallback;

// Format of a MAC address.
static char macFmt[] = "%02x:%02x:%02x:%02x:%02x:%02x";

/**
 * Wifi event handler
 * Here we get invoked whenever a WiFi event is received from the ESP32 WiFi
 * subsystem.  The events include:
 * * SYSTEM_EVENT_STA_DISCONNECTED - As a station, we were disconnected.
 */
static esp_err_t event_handler(void *ctx, system_event_t *event)
{
  ESP_LOGD(tag, ">> event_handler");
  if (event->event_id == SYSTEM_EVENT_STA_DISCONNECTED) {
    if (jsvIsFunction(g_jsDisconnectCallback)) {
      jsiQueueEvents(NULL, g_jsDisconnectCallback, NULL, 0);
      jsvUnLock(g_jsDisconnectCallback);
      g_jsDisconnectCallback = NULL;
    }
    ESP_LOGD(tag, "<< event_handler - STA DISCONNECTED");
    return ESP_OK;
  } // End of handle SYSTEM_EVENT_STA_DISCONNECTED


  if (event->event_id == SYSTEM_EVENT_STA_GOT_IP) {
    sendWifiCompletionCB(&g_jsGotIpCallback, NULL);
    ESP_LOGD(tag, "<< event_handler - STA GOT IP");
    return ESP_OK;
  } // End of handle SYSTEM_EVENT_STA_GOT_IP

  ESP_LOGD(tag, "<< event_handler");
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
  ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
} // End of esp32_wifi_init


static void sendWifiCompletionCB(
    JsVar **g_jsCallback, //!< Pointer to the global callback variable
    char *reason          //!< NULL if successful, error string otherwise
) {
  if (!jsvIsFunction(*g_jsCallback)) return; // we ain't got a function pointer: nothing to do

  JsVar *params[1];
  params[0] = reason ? jsvNewFromString(reason) : jsvNewNull();
  jsiQueueEvents(NULL, *g_jsCallback, params, 1);
  jsvUnLock(params[0]);
  // unlock and delete the global callback
  jsvUnLock(*g_jsCallback);
  *g_jsCallback = NULL;
}

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
	ESP_LOGD(tag, ">> jswrap_ESP32_wifi_soft_init");
	JsNetwork net;
	networkCreate(&net, JSNETWORKTYPE_ESP32); // Set the network type to be ESP32
  networkState = NETWORKSTATE_ONLINE; // Set the global state of the networking to be online
	ESP_LOGD(tag, "<< jswrap_ESP32_wifi_soft_init");	
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
  ESP_LOGD(tag, ">> jswrap_ESP32_wifi_disconnect");

  // Free any existing callback, then register new callback
  if (g_jsDisconnectCallback != NULL) jsvUnLock(g_jsDisconnectCallback);
  g_jsDisconnectCallback = NULL;

  // Check that the callback is a good callback if supplied.
  if (jsCallback != NULL && !jsvIsUndefined(jsCallback) && !jsvIsFunction(jsCallback)) {
    //EXPECT_CB_EXCEPTION(jsCallback);
    return;
  }

  // Save the callback for later execution.
  g_jsDisconnectCallback = jsvLockAgainSafe(jsCallback);

  // Call the ESP-IDF to disconnect us from the access point.
  esp_wifi_disconnect();

  ESP_LOGD(tag, "<< jswrap_ESP32_wifi_disconnect");
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
  ESP_LOGD(tag, ">> jswrap_ESP32_wifi_stopAP");
  ESP_LOGD(tag, "Not implemented");
  ESP_LOGD(tag, "<< jswrap_ESP32_wifi_stopAP");
}


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
  ESP_LOGD(tag, ">> jswrap_ESP32_wifi_connect");

  // Check that the ssid value isn't obviously in error.
  if (!jsvIsString(jsSsid)) {
    jsExceptionHere(JSET_ERROR, "No SSID provided");
    return;
  }

  // Create SSID string
  char ssid[33];
  int len = jsvGetString(jsSsid, ssid, sizeof(ssid)-1);
  ssid[len]='\0';

  // Make sure jsOptions is NULL or an object
  if (jsOptions != NULL && !jsvIsObject(jsOptions)) {
    //EXPECT_OPT_EXCEPTION(jsOptions);
    return;
  }

  // Check callback
  if (g_jsGotIpCallback != NULL) jsvUnLock(g_jsGotIpCallback);
  g_jsGotIpCallback = NULL;
  if (jsCallback != NULL && !jsvIsUndefined(jsCallback) && !jsvIsFunction(jsCallback)) {
    //EXPECT_CB_EXCEPTION(jsCallback);
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
      int len = jsvGetString(jsPassword, password, sizeof(password)-1);
      password[len]='\0';
    } else {
      password[0] = '\0';
    }
    jsvUnLock(jsPassword);
  } // End of we had options

  // At this point, we have the ssid in "ssid" and the password in "password".
  // Perform an esp_wifi_set_mode
  esp_err_t err = esp_wifi_set_mode(WIFI_MODE_STA);
  if (err != ESP_OK) {
    ESP_LOGE(tag, "jswrap_ESP32_wifi_connect: esp_wifi_set_mode: %d", err);
    return;
  }

  // Perform a an esp_wifi_set_config
  wifi_config_t staConfig;
  memcpy(staConfig.sta.ssid, ssid, sizeof(staConfig.sta.ssid));
  memcpy(staConfig.sta.password, password, sizeof(staConfig.sta.password));
  staConfig.sta.bssid_set = false;
  err = esp_wifi_set_config(WIFI_IF_STA,  &staConfig);
  if (err != ESP_OK) {
    ESP_LOGE(tag, "jswrap_ESP32_wifi_connect: esp_wifi_set_config: %d", err);
    return;
  }

  // Perform an esp_wifi_start
  err = esp_wifi_start();
  if (err != ESP_OK) {
    ESP_LOGE(tag, "jswrap_ESP32_wifi_connect: esp_wifi_start: %d", err);
    return;
  }

  // Save the callback for later execution.
  g_jsGotIpCallback = jsvLockAgainSafe(jsCallback);

  // Perform an esp_wifi_connect
  err = esp_wifi_connect();
  if (err != ESP_OK) {
    ESP_LOGE(tag, "jswrap_ESP32_wifi_connect: esp_wifi_connect: %d", err);
    return;
  }

  ESP_LOGD(tag, "<< jswrap_ESP32_wifi_connect");
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
Perform a scan for access points. This will enable the station mode if it is not currently enabled. Once the scan is complete the callback function is called with an array of APs found, each AP is an object with:
* `ssid`: SSID string.
* `mac`: access point MAC address in 00:00:00:00:00:00 format.
* `authMode`: `open`, `wep`, `wpa`, `wpa2`, or `wpa_wpa2`.
* `channel`: wifi channel 1..13.
* `hidden`: true if the SSID is hidden.
* `rssi`: signal strength in dB in the range -110..0.
Notes:
* in order to perform the scan the station mode is turned on and remains on, use Wifi.disconnect() to turn it off again, if desired.
* only one scan can be in progress at a time.
*/
void jswrap_ESP32_wifi_scan(JsVar *jsCallback) {
  ESP_LOGD(tag, ">> jswrap_ESP32_wifi_scan");
  ESP_LOGD(tag, "Not implemented");
  ESP_LOGD(tag, "<< jswrap_ESP32_wifi_scan");
}

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
  ESP_LOGD(tag, ">> jswrap_ESP32_wifi_startAP");
  ESP_LOGD(tag, "Not implemented");
  ESP_LOGD(tag, "<< jswrap_ESP32_wifi_startAP");
}

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
  ESP_LOGD(tag, ">> jswrap_ESP32_wifi_getStatus");
  ESP_LOGD(tag, "Not implemented");
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
  case WIFI_PS_LIGHT:
    psTypeStr = "light";
    break;
  case WIFI_PS_MAC:
    psTypeStr = "mac";
    break;
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
  jsvObjectSetChildAndUnLock(jsWiFiStatus, "mode", jsvNewFromString(modeStr));
  jsvObjectSetChildAndUnLock(jsWiFiStatus, "powersave", jsvNewFromString(psTypeStr));
  ESP_LOGD(tag, "<< jswrap_ESP32_wifi_getStatus");
  return jsWiFiStatus;
}

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
  ESP_LOGD(tag, ">> jswrap_ESP32_wifi_setConfig");
  ESP_LOGD(tag, "Not implemented");
  ESP_LOGD(tag, "<< jswrap_ESP32_wifi_setConfig");
}

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
  ESP_LOGD(tag, ">> jswrap_ESP32_wifi_getDetails");
  ESP_LOGD(tag, "Not implemented");
  ESP_LOGD(tag, "<< jswrap_ESP32_wifi_getDetails");
  return NULL;
}

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
  ESP_LOGD(tag, ">> jswrap_ESP32_wifi_getAPDetails");
  ESP_LOGD(tag, "Not implemented");
  ESP_LOGD(tag, "<< jswrap_ESP32_wifi_getAPDetails");
  return NULL;
}


/*JSON{
  "type"     : "staticmethod",
  "class"    : "Wifi",
  "name"     : "save",
  "generate" : "jswrap_ESP32_wifi_save",
  "params"   : [
    ["what", "JsVar", "An optional parameter to specify what to save, on the esp8266 the two supported values are `clear` and `sta+ap`. The default is `sta+ap`"]
  ]
}
Save the current wifi configuration (station and access point) to flash and automatically apply this configuration at boot time, unless `what=="clear"`, in which case the saved configuration is cleared such that wifi remains disabled at boot. The saved configuration includes:
* mode (off/sta/ap/sta+ap)
* SSIDs & passwords
* phy (11b/g/n)
* powersave setting
* DHCP hostname
*/
void jswrap_ESP32_wifi_save(JsVar *what) {
  ESP_LOGD(tag, ">> jswrap_ESP32_wifi_save");
  ESP_LOGD(tag, "Not implemented");
  ESP_LOGD(tag, "<< jswrap_ESP32_wifi_save");
}

/*JSON{
  "type"     : "staticmethod",
  "class"    : "Wifi",
  "name"     : "restore",
  "generate" : "jswrap_ESP32_wifi_restore"
}
Restores the saved Wifi configuration from flash. See `Wifi.save()`.
*/
void jswrap_ESP32_wifi_restore(void) {
  ESP_LOGD(tag, ">> jswrap_ESP32_wifi_restore");
  ESP_LOGD(tag, "Not implemented");
  ESP_LOGD(tag, "<< jswrap_ESP32_wifi_restore");
}
/**
 * Get the ip info for the given interface.  The interfaces are:
 * * TCPIP_ADAPTER_IF_STA - Station
 * * TCPIP_ADAPTER_IF_AP - Access Point
 */
static JsVar *getIPInfo(JsVar *jsCallback, tcpip_adapter_if_t interface) {
  // Check callback
  if (jsCallback != NULL && !jsvIsNull(jsCallback) && !jsvIsFunction(jsCallback)) {
    //EXPECT_CB_EXCEPTION(jsCallback);
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
  sprintf(macAddrString, macFmt,
    macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]);
  jsvObjectSetChildAndUnLock(jsIpInfo, "mac", jsvNewFromString(macAddrString));

  // Schedule callback if a function was provided
  if (jsvIsFunction(jsCallback)) {
    JsVar *params[1];
    params[0] = jsIpInfo;
    jsiQueueEvents(NULL, jsCallback, params, 1);
  }

  return jsIpInfo;
}

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
  ESP_LOGD(tag, ">> jswrap_ESP32_wifi_getIP");
  JsVar *jsIpInfo = getIPInfo(jsCallback, TCPIP_ADAPTER_IF_STA);
  ESP_LOGD(tag, "<< jswrap_ESP32_wifi_getIP");
  return jsIpInfo;
}

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
  ESP_LOGD(tag, ">> jswrap_ESP32_wifi_getAPIP");
  ESP_LOGD(tag, "Not implemented");
  ESP_LOGD(tag, "<< jswrap_ESP32_wifi_getAPIP");
  return NULL;
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
  ESP_LOGD(tag, ">> jswrap_ESP32_wifi_getHostByName");
  ESP_LOGD(tag, "Not implemented");
  ESP_LOGD(tag, "<< jswrap_ESP32_wifi_getHostByName");
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
  ESP_LOGD(tag, ">> jswrap_ESP32_wifi_getHostname");
  ESP_LOGD(tag, "Not implemented");
  ESP_LOGD(tag, "<< jswrap_ESP32_wifi_getHostname");
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
  ESP_LOGD(tag, ">> jswrap_ESP32_wifi_setHostname");
  ESP_LOGD(tag, "Not implemented");
  ESP_LOGD(tag, "<< jswrap_ESP32_wifi_setHostname");
}


/*JSON{
  "type"     : "staticmethod",
  "class"    : "ESP8266",
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
  ESP_LOGD(tag, ">> jswrap_ESP32_ping");
  ESP_LOGD(tag, "Not implemented");
  ESP_LOGD(tag, "<< jswrap_ESP32_ping");
}
