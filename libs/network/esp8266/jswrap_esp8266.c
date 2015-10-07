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
static void scanCB(void *arg, STATUS status);
static void wifiEventHandler(System_Event_t *event);
static void ipAddrToString(struct ip_addr addr, char *string);
static char *nullTerminateString(char *target, char *source, int sourceLength);
static void setupJsNetwork();
static void pingRecvCB();
static char *wifiConnectStatusToString(uint8 status);

static JsVar *jsScanCallback;
static JsVar *jsWiFiEventCallback;

// A callback function to be invoked when we have an IP address.
static JsVar *jsGotIpCallback;

static JsVar *jsPingCallback;

// Global data structure for ping request
static struct ping_option pingOpt;

// Reasons for which a connection failed
uint8_t wifiReason = 0;
static char *wifiReasons[] = {
  "", "unspecified", "auth_expire", "auth_leave", "assoc_expire", "assoc_toomany", "not_authed",
  "not_assoced", "assoc_leave", "assoc_not_authed", "disassoc_pwrcap_bad", "disassoc_supchan_bad",
  "ie_invalid", "mic_failure", "4way_handshake_timeout", "group_key_update_timeout",
  "ie_in_4way_differs", "group_cipher_invalid", "pairwise_cipher_invalid", "akmp_invalid",
  "unsupp_rsn_ie_version", "invalid_rsn_ie_cap", "802_1x_auth_failed", "cipher_suite_rejected",
  "beacon_timeout", "no_ap_found" };

static char *wifiGetReason(void) {
  if (wifiReason <= 24) return wifiReasons[wifiReason];
  if (wifiReason >= 200 && wifiReason <= 201) return wifiReasons[wifiReason-200+24];
  return wifiReasons[1];
}

static char *wifiMode[] = { 0, "STA", "AP", "AP+STA" };
static char *wifiPhy[]  = { 0, "11b", "11g", "11n" };

// Let's define the JavaScript class that will contain our `world()` method. We'll call it `Hello`
/*JSON{
  "type"  : "class",
  "class" : "ESP8266WiFi"
}*/


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
 *
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

  // Set the global which is the gotIP callback
  if (jsGotIpCallback != NULL) {
    jsvUnLock(jsGotIpCallback);
    jsGotIpCallback = NULL;
  }

  // What does this do?
  if (gotIpCallback != NULL) {
    jsGotIpCallback = jsvLockAgainSafe(gotIpCallback);
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
    ["jsv_ssid","JsVar","The network SSID"],
    ["jsv_password","JsVar","The password to allow stations to connect to the access point"]
  ]
}*/
void jswrap_ESP8266WiFi_beAccessPoint(
    JsVar *jsv_ssid,    //!< The network identity that the access point will advertize itself as.
    JsVar *jsv_password //!< The password a station will need to connect to the access point.
  ) {
  // Validate that the SSID and password are somewhat useful.
  if (jsv_ssid == NULL || !jsvIsString(jsv_ssid)) {
      jsExceptionHere(JSET_ERROR, "No SSID.");
    return;
  }
  if (jsv_password == NULL || !jsvIsString(jsv_password)) {
      jsExceptionHere(JSET_ERROR, "No password.");
    return;
  }

  // Create strings from the JsVars for the ESP8266 API calls.
  char ssid[33];
  int len = jsvGetString(jsv_ssid, ssid, sizeof(ssid)-1);
  ssid[len]='\0';
  char password[65];
  len = jsvGetString(jsv_password, password, sizeof(password)-1);
  password[len]='\0';

  // Define that we are in Soft AP mode.
  os_printf("Wifi: switching to soft-AP\n");
  wifi_set_opmode_current(SOFTAP_MODE);

  // Build our SoftAP configuration details
  struct softap_config softApConfig;
  memset(&softApConfig, 0, sizeof(softApConfig));

  os_strcpy((char *)softApConfig.ssid, ssid);
  os_strcpy((char *)softApConfig.password, password);
  softApConfig.ssid_len = 0; // Null terminated SSID
  softApConfig.authmode = AUTH_WPA2_PSK;
  softApConfig.ssid_hidden = 0; // Not hidden.
  softApConfig.max_connection = 4; // Maximum number of connections.

  // Set the WiFi configuration.
  int rc = wifi_softap_set_config_current(&softApConfig);
  if (rc != 1) {
      jsExceptionHere(JSET_ERROR, "Error setting ESP8266 softap config.");
  }
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
}*/
void jswrap_ESP8266WiFi_getAccessPoints(
    JsVar *callback //!< Function to call back when access points retrieved.
  ) {
  os_printf("> ESP8266WiFi_getAccessPoints\n");
  if (callback == NULL || !jsvIsFunction(callback)) {
      jsExceptionHere(JSET_ERROR, "No callback.");
    return;
  }

  // Save the callback for the scan in the global variable called jsScanCallback.
  jsScanCallback = jsvLockAgainSafe(callback);

  // Ask the ESP8266 to perform a network scan after first entering
  // station mode.  The network scan will eventually result in a callback
  // being executed (scanCB) which will contain the results.

  // Ensure we are in station mode
  wifi_set_opmode_current(STATION_MODE);

  // Request a scan of the network calling "scanCB" on completion
  wifi_station_scan(NULL, scanCB);

  os_printf("< ESP8266WiFi_getAccessPoints\n");
}


/**
 * Disconnect the station from the access point.
 */
/*JSON{
  "type"     : "staticmethod",
  "class"    : "ESP8266WiFi",
  "name"     : "disconnect",
  "generate" : "jswrap_ESP8266WiFi_disconnect"
}*/
void jswrap_ESP8266WiFi_disconnect() {
  wifi_station_disconnect();
}


/*JSON{
  "type"     : "staticmethod",
  "class"    : "ESP8266WiFi",
  "name"     : "restart",
  "generate" : "jswrap_ESP8266WiFi_restart"
}*/
void jswrap_ESP8266WiFi_restart() {
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
    if (jsWiFiEventCallback != NULL) {
      jsvUnLock(jsWiFiEventCallback);
    }
    jsWiFiEventCallback = NULL;
    return;
  }

  if (!jsvIsFunction(callback)) {
      jsExceptionHere(JSET_ERROR, "No callback.");
    return;
  }

  // We are about to save a new global WiFi even callback handler.  If we have previously
  // had one, we need to unlock it so that we don't leak memory.
  if (jsWiFiEventCallback != NULL) {
    jsvUnLock(jsWiFiEventCallback);
  }

  // Save the global WiFi event callback handler.
  jsWiFiEventCallback = jsvLockAgainSafe(callback);
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

  uart_rx_intr_disable(0);
  wifi_station_set_auto_connect(newValue);
  uart_rx_intr_disable(1);
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
  "class"    : "ESP8266WiFi",
  "name"     : "getRstInfo",
  "generate" : "jswrap_ESP8266WiFi_getRstInfo",
  "return"   : ["JsVar","A Restart Object"],
  "return_object" : "Restart"
}*/
JsVar *jswrap_ESP8266WiFi_getRstInfo() {
  struct rst_info* info = system_get_rst_info();
  JsVar *restartInfo = jspNewObject(NULL, "Restart");
  jsvUnLock(jsvObjectSetChild(restartInfo, "reason",   jsvNewFromInteger(info->reason)));
  jsvUnLock(jsvObjectSetChild(restartInfo, "exccause", jsvNewFromInteger(info->exccause)));
  jsvUnLock(jsvObjectSetChild(restartInfo, "epc1",     jsvNewFromInteger(info->epc1)));
  jsvUnLock(jsvObjectSetChild(restartInfo, "epc2",     jsvNewFromInteger(info->epc2)));
  jsvUnLock(jsvObjectSetChild(restartInfo, "epc3",     jsvNewFromInteger(info->epc3)));
  jsvUnLock(jsvObjectSetChild(restartInfo, "excvaddr", jsvNewFromInteger(info->excvaddr)));
  jsvUnLock(jsvObjectSetChild(restartInfo, "depc",     jsvNewFromInteger(info->depc)));
  return restartInfo;
}

/*JSON{
  "type"     : "staticmethod",
  "class"    : "ESP8266WiFi",
  "name"     : "updateCPUFreq",
  "generate" : "jswrap_ESP8266WiFi_updateCPUFreq",
  "params"   : [
    ["freq","JsVar","Desired frequency - either 80 or 160."]
  ]
}
 * Update the operating frequency of the ESP8266 processor.
 */
void jswrap_ESP8266WiFi_updateCPUFreq(
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
  "class"    : "ESP8266WiFi",
  "name"     : "getState",
  "generate" : "jswrap_ESP8266WiFi_getState",
  "return"   : ["JsVar","The state of the ESP8266"],
  "return_object" : "ESP8266State"
}*/
JsVar *jswrap_ESP8266WiFi_getState() {
  // Create a new variable and populate it with the properties of the ESP8266 that we
  // wish to return.
  JsVar *esp8266State = jspNewObject(NULL, "ESP8266State");
  jsvUnLock(jsvObjectSetChild(esp8266State, "sdkVersion",   jsvNewFromString(system_get_sdk_version())));
  jsvUnLock(jsvObjectSetChild(esp8266State, "cpuFrequency", jsvNewFromInteger(system_get_cpu_freq())));
  jsvUnLock(jsvObjectSetChild(esp8266State, "freeHeap",     jsvNewFromInteger(system_get_free_heap_size())));
  jsvUnLock(jsvObjectSetChild(esp8266State, "maxCon",       jsvNewFromInteger(espconn_tcp_get_max_con())));
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
}*/
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
}*/
JsVar *jswrap_ESP8266WiFi_getIPInfo() {
  struct ip_info info;
  wifi_get_ip_info(0, &info);

  JsVar *ipInfo = jspNewObject(NULL, "Restart");
  jsvUnLock(jsvObjectSetChild(ipInfo, "ip", jsvNewFromInteger(info.ip.addr)));
  jsvUnLock(jsvObjectSetChild(ipInfo, "netmask", jsvNewFromInteger(info.netmask.addr)));
  jsvUnLock(jsvObjectSetChild(ipInfo, "gw", jsvNewFromInteger(info.gw.addr)));
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
  jsvUnLock(jsvObjectSetChild(jsConfig, "ssid", jsvNewFromString((char *)config.ssid)));
  //char password[65];
  //nullTerminateString(password, (char *)config.password, 64);
  jsvUnLock(jsvObjectSetChild(jsConfig, "password", jsvNewFromString((char *)config.password)));
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
      jsvUnLock(jsvObjectSetChild(jsStation, "ip", jsvNewFromInteger(stationInfo->ip.addr)));
      jsvArrayPush(jsArray, jsStation);
      stationInfo = STAILQ_NEXT(stationInfo, next);
    }
    wifi_softap_free_station_info();
  }
  return jsArray;
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
  jsvUnLock(jsvObjectSetChild(var, "status", jsStatus));

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
  jsvUnLock(jsvObjectSetChild(var, "statusMsg", jsStatusMsg));
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
  "class"    : "ESP8266WiFi",
  "name"     : "ping",
  "generate" : "jswrap_ESP8266WiFi_ping",
  "params"   : [
    ["ipAddr","JsVar","A string or integer representation of an IP address."],
    ["pingCallback", "JsVar", "Optional callback function."]
  ]
}*/
void jswrap_ESP8266WiFi_ping(
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
    if (jsPingCallback != NULL) {
      jsvUnLock(jsPingCallback);
    }
    jsPingCallback = NULL;
  } else if (!jsvIsFunction(pingCallback)) {
      jsExceptionHere(JSET_ERROR, "Callback is not a function.");
    return;
  } else {
    if (jsPingCallback != NULL) {
      jsvUnLock(jsPingCallback);
    }
    jsPingCallback = pingCallback;
    jsvLockAgainSafe(jsPingCallback);
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
  "class"    : "ESP8266WiFi",
  "name"     : "dumpSocket",
  "generate" : "jswrap_ESP8266WiFi_dumpSocket",
  "params"   : [
    ["socketId","JsVar","The socket to be dumped."]
  ]
}*/

void jswrap_ESP8266WiFi_dumpSocket(
    JsVar *socketId //!< The socket to be dumped.
  ) {
  esp8266_dumpSocket(jsvGetInteger(socketId)-1);
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
  if (jsPingCallback != NULL) {
    JsVar *jsPingResponse = jspNewObject(NULL, "PingResponse");
    jsvUnLock(jsvObjectSetChild(jsPingResponse, "totalCount",   jsvNewFromInteger(pingResp->total_count)));
    jsvUnLock(jsvObjectSetChild(jsPingResponse, "totalBytes",   jsvNewFromInteger(pingResp->total_bytes)));
    jsvUnLock(jsvObjectSetChild(jsPingResponse, "totalTime",    jsvNewFromInteger(pingResp->total_time)));
    jsvUnLock(jsvObjectSetChild(jsPingResponse, "respTime",     jsvNewFromInteger(pingResp->resp_time)));
    jsvUnLock(jsvObjectSetChild(jsPingResponse, "seqNo",        jsvNewFromInteger(pingResp->seqno)));
    jsvUnLock(jsvObjectSetChild(jsPingResponse, "timeoutCount", jsvNewFromInteger(pingResp->timeout_count)));
    jsvUnLock(jsvObjectSetChild(jsPingResponse, "bytes",        jsvNewFromInteger(pingResp->bytes)));
    jsvUnLock(jsvObjectSetChild(jsPingResponse, "error",        jsvNewFromInteger(pingResp->ping_err)));
    JsVar *params[1];
    params[0] = jsPingResponse;
    jsiQueueEvents(NULL, jsPingCallback, params, 1);
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
    // sint8 rssi – The received signal strength indication
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
    jsvUnLock(jsvObjectSetChild(currentAccessPoint, "rssi", jsvNewFromInteger(bssInfo->rssi)));
    jsvUnLock(jsvObjectSetChild(currentAccessPoint, "channel", jsvNewFromInteger(bssInfo->channel)));
    jsvUnLock(jsvObjectSetChild(currentAccessPoint, "authMode", jsvNewFromInteger(bssInfo->authmode)));
    jsvUnLock(jsvObjectSetChild(currentAccessPoint, "isHidden", jsvNewFromBool(bssInfo->is_hidden)));
    // The SSID may **NOT** be NULL terminated ... so handle that.
    char ssid[sizeof(bssInfo->ssid) + 1];
    os_strncpy((char *)ssid, (char *)bssInfo->ssid, sizeof(bssInfo->ssid));
    ssid[sizeof(ssid)-1] = '\0';
    jsvUnLock(jsvObjectSetChild(currentAccessPoint, "ssid", jsvNewFromString(ssid)));

    // Add the new record to the array
    jsvArrayPush(accessPointArray, currentAccessPoint);

    os_printf(" - ssid: %s\n", bssInfo->ssid);
    bssInfo = STAILQ_NEXT(bssInfo, next);
  }

  // We have now completed the scan callback, so now we can invoke the JS callback.
  JsVar *params[1];
  params[0] = accessPointArray;
  jsiQueueEvents(NULL, jsScanCallback, params, 1);
  jsvUnLock(jsScanCallback);
  os_printf("<< scanCB\n");
}


/**
 * Invoke the JavaScript callback to notify the program that an ESP8266
 * WiFi event has occurred.
 */
static void sendWifiEvent(uint32 eventType, JsVar *details) {
  // We need to check that we actually have an event callback handler because
  // it might have been disabled/removed.
  if (jsWiFiEventCallback != NULL) {
    // Build a callback event.
    JsVar *params[2];
    params[0] = jsvNewFromInteger(eventType);
    params[1] = details;
    jsiQueueEvents(NULL, jsWiFiEventCallback, params, 2);
  }

  if (jsGotIpCallback != NULL && eventType == EVENT_STAMODE_GOT_IP) {
    JsVar *params[2];
    params[0] = jsvNewFromInteger(eventType);
    params[1] = details;
    jsiQueueEvents(NULL, jsGotIpCallback, params, 2);
    // Once we have registered the callback, we can unlock and release
    // the variable as we are only calling it once.
    //jsvUnLock(jsGotIpCallback);
    //jsGotIpCallback = NULL;
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
      evt->event_info.disconnected.ssid, wifiGetReason(), evt->event_info.disconnected.reason);
    JsVar *details = jspNewObject(NULL, "EventDetails");
    jsvObjectSetChild(details, "reason", jsvNewFromInteger(evt->event_info.disconnected.reason));
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
