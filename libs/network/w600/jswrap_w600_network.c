#include "wm_type_def.h"
#include "wm_wifi.h"
#include "wm_params.h"
#include "lwip/ip_addr.h"
#include "lwip/netif.h"
#include "wm_netif.h"
#include "wm_efuse.h"

#include "jswrap_w600_network.h"
#include "jswrap_modules.h"

#include "network.h"
#include "jsinteractive.h"


#define EXPECT_CB_EXCEPTION(jsCB) jsExceptionHere(JSET_ERROR, "Expecting callback function but got %v", jsCB)
#define EXPECT_OPT_EXCEPTION(jsOPT) jsExceptionHere(JSET_ERROR, "Expecting options object but got %t", jsOPT)

typedef struct{
  uint8_t mac[ETH_ALEN];
  bool    used;
}wifi_softap_client_t;

static char macFmt[] = "%02x:%02x:%02x:%02x:%02x:%02x";
static char *wifi_auths[]={"open","wep40","wep104","wpa_tkip","wpa_ccmp","wpa2_tkip","wpa2_ccmp","wpa_auto","wpa2_auto"};
static char *wifi_mode[] = { "off", "sta", "adhoc","sta+adhoc","ap", "sta+ap","ap+adhoc","sta+ap+adhoc" };
static char *wifi_conn[]={"disconnected","scanning","connecting","connected"};
// A callback function to be invoked when we have an IP address.
static JsVar *g_jsGotIpCallback;

// A callback function to be invoked on a disconnect response.
static JsVar *g_jsDisconnectCallback;

// A callback function to be invoked when we find a new access point.
static JsVar *g_jsScanCallback;

// Flag to tell the wifi event handler that it should turn STA off on disconnect
static bool g_disconnecting;

// Flag to tell the wifi event handler to ignore the next disconnect event because
// we're provoking it in order to connect to something different
static bool g_skipDisconnect;

static const char *wifi_events[] = {"#ondisconnected", "#onconnected", "#onsta_joined", "#onsta_left" };
static char wifi_event_buff[sizeof("#ondisconnected")+1]; // length of longest string
static char *wifi_get_event(uint8_t event) {
  strncpy(wifi_event_buff, wifi_events[event], sizeof(wifi_event_buff));
  wifi_event_buff[sizeof(wifi_event_buff)-1] = 0;
  return wifi_event_buff;
}

static void send_wifi_completion(
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

static void send_wifi_event(
    uint8_t eventType, //!< The W600 WiFi event type.
    JsVar *jsDetails  //!< The JS object to be passed as a parameter to the callback.
 ){

  JsVar *moduleName = jsvNewFromString("Wifi");
  JsVar *module = jswrap_require(moduleName);
  jsvUnLock(moduleName);

  // get event name as string and compose param list
  JsVar *params[1];
  params[0] = jsDetails;
  char *eventName = wifi_get_event(eventType);
  jsiQueueObjectCallbacks(module, eventName, params, 1);
  jsvUnLock(module);
}

static wifi_softap_client_t softap_clients[ETH_ALEN];
static wifi_softap_client_t* find_client(uint8_t *mac){
  if(mac==0){
    for(int i=0;i<ETH_ALEN;i++){
      if(!softap_clients[i].used){
        return &softap_clients[i];
      }
    }
    return 0;
  }else{
    for(int i=0;i<ETH_ALEN;i++){
      if(memcmp(mac,softap_clients[i].mac,ETH_ALEN)==0){
        return &softap_clients[i];
      }
    }
    return 0;
  }
}
/**
 * W600 WiFi Event handler.
 * This function is called by the W600
 * environment when significant events happen related to the WiFi environment.
 * The event handler is registered with a call to tls_netif_add_status_event()
 * that is provided by the W600 SDK.
 */
void wifi_status_change_callback(uint8_t status){
  JsVar *jsDetails = jsvNewObject();

  switch(status){
    case NETIF_IP_NET_UP:{
      if (jsvIsFunction(g_jsGotIpCallback)) {
        send_wifi_completion(&g_jsGotIpCallback, NULL);
      }
      send_wifi_event(1, jsDetails);
    }break;
    case NETIF_WIFI_JOIN_FAILED:{
      if (jsvIsFunction(g_jsGotIpCallback)) {
        send_wifi_completion(&g_jsGotIpCallback,"failed");
      }
    }break;
    case NETIF_WIFI_DISCONNECTED:{
      if (g_disconnecting) {

        uint8_t opmode=0;
        tls_param_get(TLS_PARAM_ID_WPROTOCOL,&opmode,false);
        opmode&=~TLS_PARAM_IEEE80211_INFRA;
        tls_param_set(TLS_PARAM_ID_WPROTOCOL,&opmode,false);
        
        g_disconnecting = false;
        if (jsvIsFunction(g_jsDisconnectCallback)) {
          jsiQueueEvents(NULL, g_jsDisconnectCallback, NULL, 0);
          jsvUnLock(g_jsDisconnectCallback);
          g_jsDisconnectCallback = NULL;
        }
      }
      send_wifi_event(0, jsDetails);
    }break;
  }

  jsvUnLock(jsDetails);
}

void wifi_softap_client_callback(uint8_t *mac,enum tls_wifi_client_event_type status){
  wifi_softap_client_t *client=find_client(mac);

  JsVar *jsDetails = jsvNewObject();

  char macAddrString[6*3 + 1];
  sprintf(macAddrString, macFmt, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  jsvObjectSetChildAndUnLock(jsDetails, "mac", jsvNewFromString(macAddrString));

  if(status==WM_WIFI_CLIENT_EVENT_ONLINE&&client){
    client->used=true;
    memcpy(client->mac,mac,ETH_ALEN);
    send_wifi_event(2, jsDetails);
  }else if(status==WM_WIFI_CLIENT_EVENT_OFFLINE&&client){
    client->used=false;
    send_wifi_event(3, jsDetails);
  }

  jsvUnLock(jsDetails);
}

void wifi_scan_result_callback(){
  /**
   * Create a JsVar that is an array of JS objects where each JS object represents a
   * retrieved access point set of information.   The structure of a record will be:
   * o authMode
   * o rssi
   * o channel
   * o ssid
   * When the array has been built, invoke the callback function passing in the array
   * of records.
   */

  if(g_jsScanCallback==NULL){
    return;
  }
  
  int buflen=2048;
  char *buffer=tls_mem_alloc(buflen);
  
  if(buffer==NULL){
    return;
  }
  memset(buffer,0,buflen);

  if(tls_wifi_get_scan_rslt((uint8_t *)buffer,buflen)){
    tls_mem_free(buffer);
    return;
  }

  struct tls_scan_bss_t *scan_res=(struct tls_scan_bss_t *)buffer;
  struct tls_bss_info_t *bss_info=scan_res->bss;

  JsVar *jsAccessPointArray = jsvNewArray(NULL, 0);
  int8_t _rssi;

  for(uint32_t i=0;i<scan_res->count;i++) {
    // Add a new object to the JS array that will be passed as a parameter to
    // the callback.
    // Create, populate and add a child ...
    JsVar *jsCurrentAccessPoint = jsvNewObject();

    _rssi=-(0x100-bss_info->rssi);

    jsvObjectSetChildAndUnLock(jsCurrentAccessPoint, "rssi", jsvNewFromInteger(_rssi));
    jsvObjectSetChildAndUnLock(jsCurrentAccessPoint, "channel", jsvNewFromInteger(bss_info->channel));
    jsvObjectSetChildAndUnLock(jsCurrentAccessPoint, "authMode", jsvNewFromString(wifi_auths[bss_info->privacy]));

    // The SSID may **NOT** be NULL terminated ... so handle that.
    char ssid[sizeof(bss_info->ssid) + 1];
    strncpy(ssid, (char *)bss_info->ssid, sizeof(bss_info->ssid));
    ssid[sizeof(ssid)-1] = '\0';
    jsvObjectSetChildAndUnLock(jsCurrentAccessPoint, "ssid", jsvNewFromString(ssid));

    char macAddrString[6*3 + 1];
    sprintf(macAddrString, macFmt,
      bss_info->bssid[0], bss_info->bssid[1], bss_info->bssid[2],
      bss_info->bssid[3], bss_info->bssid[4], bss_info->bssid[5]);
    jsvObjectSetChildAndUnLock(jsCurrentAccessPoint, "mac", jsvNewFromString(macAddrString));

    // Add the new record to the array
    jsvArrayPush(jsAccessPointArray, jsCurrentAccessPoint);
    jsvUnLock(jsCurrentAccessPoint);

    bss_info++;
  }

  tls_mem_free(buffer);

  // We have now completed the scan callback, so now we can invoke the JS callback.
  JsVar *params[1];
  params[0] = jsAccessPointArray;
  jsiQueueEvents(NULL, g_jsScanCallback, params, 1);

  jsvUnLock(jsAccessPointArray);
  jsvUnLock(g_jsScanCallback);
  g_jsScanCallback = NULL;
}

/**
 * Get the ip info for the given interface.  The interfaces are:
 * * 0 - Station
 * * 1 - Access Point
 */
JsVar *get_ip_info(JsVar *jsCallback, int interface) {

  struct netif *ethif= tls_get_netif();
  if(interface==1){
    ethif=ethif->next;
  }

  // first get IP address info, this may fail if we're not connected
  JsVar *jsIpInfo = jsvNewObject();
  jsvObjectSetChildAndUnLock(jsIpInfo, "ip",networkGetAddressAsString((uint8_t *)&ethif->ip_addr.addr, 4, 10, '.'));
  jsvObjectSetChildAndUnLock(jsIpInfo, "netmask",networkGetAddressAsString((uint8_t *)&ethif->netmask.addr, 4, 10, '.'));
  jsvObjectSetChildAndUnLock(jsIpInfo, "gw",networkGetAddressAsString((uint8_t *)&ethif->gw.addr, 4, 10, '.'));

  // now get MAC address (which always succeeds)
  uint8_t macAddr[6];
  tls_get_mac_addr(macAddr);
  char macAddrString[6*3 + 1];
  sprintf(macAddrString, macFmt,
          macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]);
  jsvObjectSetChildAndUnLock(jsIpInfo, "mac", jsvNewFromString(macAddrString));

  // Schedule callback if a function was provided
  if (jsvIsFunction(jsCallback)) {
    JsVar *params[2];
    params[0] = jsvNewWithFlags(JSV_NULL);
    params[1] = jsIpInfo;
    jsiQueueEvents(NULL, jsCallback, params, 2);
    jsvUnLock(params[0]);
  }

  return jsIpInfo;
}

void set_ip_info(JsVar *jsCallback,JsVar *jsSettings, int interface){
  
  char ipTmp[20];
  bool rc = false;
  
  ip_addr_t ipaddr,netmask,gateway;
  memset(&ipaddr, 0, sizeof(ip_addr_t));
  memset(&netmask, 0, sizeof(ip_addr_t));
  memset(&gateway, 0, sizeof(ip_addr_t));

  // first check parameter 
  if (!jsvIsObject(jsSettings)) {
    EXPECT_OPT_EXCEPTION(jsSettings);
    return;
  }

  // get,check and store ip
  JsVar *jsIP = jsvObjectGetChild(jsSettings, "ip", 0);
  if (jsIP != NULL && !jsvIsString(jsIP)) {
      EXPECT_OPT_EXCEPTION(jsIP);
      jsvUnLock(jsIP);
      return; 
  }
  jsvGetString(jsIP, ipTmp, sizeof(ipTmp)-1);
  ipaddr.addr = networkParseIPAddress(ipTmp); 
  if ( ipaddr.addr  == 0) {
    jsExceptionHere(JSET_ERROR, "Not a valid IP address.");
    jsvUnLock(jsIP);
    return;
  }
  jsvUnLock(jsIP);

  // get, check and store gw
  JsVar *jsGW = jsvObjectGetChild(jsSettings, "gw", 0);
  if (jsGW != NULL && !jsvIsString(jsGW)) {
      EXPECT_OPT_EXCEPTION(jsGW);
      jsvUnLock(jsGW);
      return ;
  }
  jsvGetString(jsGW, ipTmp, sizeof(ipTmp)-1);
  gateway.addr = networkParseIPAddress(ipTmp);
  if (gateway.addr == 0) {
    jsExceptionHere(JSET_ERROR, "Not a valid Gateway address.");
    jsvUnLock(jsGW);
    return;
  }
  jsvUnLock(jsGW);

  // netmask setting
  JsVar *jsNM = jsvObjectGetChild(jsSettings, "netmask", 0);
  if (jsNM != NULL && !jsvIsString(jsNM)) {
      EXPECT_OPT_EXCEPTION(jsNM);
      jsvUnLock(jsNM);
      return;
  }  
  jsvGetString(jsNM, ipTmp, sizeof(ipTmp)-1);
  netmask.addr = networkParseIPAddress(ipTmp); 
  if (netmask.addr == 0) {
    jsExceptionHere(JSET_ERROR, "Not a valid Netmask.");
    jsvUnLock(jsNM);    
    return;
  }
  jsvUnLock(jsNM);

  // set IP for access point
  if (interface == 1 ) {
    tls_dhcps_stop();
    rc=tls_netif2_set_addr(&ipaddr,&netmask,&gateway);
    tls_dhcps_start();
  }
  // set IP for station
  else {
    tls_dhcp_stop();
    rc=tls_netif_set_addr(&ipaddr,&netmask,&gateway);
  }

  // Schedule callback
  if (jsvIsFunction(jsCallback)) {
    JsVar *params[1];
    params[0] = rc ? jsvNewWithFlags(JSV_NULL) : jsvNewFromString("Failure");
    jsiQueueEvents(NULL, jsCallback, params, 1); 
    jsvUnLock(params[0]);
  }
  else {
    jsExceptionHere(JSET_ERROR, "Callback is not a function.");
  }
  
  return ;
}

void jswrap_wifi_connect(JsVar *jsSsid, JsVar *jsOptions, JsVar *jsCallback){
  // Notes:
  // The callback function is saved in the file local variable called g_jsGotIpCallback.  The
  // callback will be made when the WiFi callback found in the function called wifiEventHandler.

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
  g_disconnecting = false; // we're gonna be connecting...

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
      len = jsvGetString(jsPassword, password, sizeof(password)-1);
      password[len]='\0';
    } else {
      password[0] = '\0';
    }
    jsvUnLock(jsPassword);
  }

  

  enum tls_wifi_states wifiConnectStatus = tls_wifi_get_state();
  
  struct tls_param_ssid original_ssid;
  struct tls_param_original_key original_pwd;
  memset(&original_ssid,0,sizeof(struct tls_param_ssid));
  memset(&original_pwd,0,sizeof(struct tls_param_original_key));

  tls_param_get(TLS_PARAM_ID_ORIGIN_SSID,&original_ssid,false);
  tls_param_get(TLS_PARAM_ID_ORIGIN_KEY,&original_ssid,false);


  if (wifiConnectStatus ==  WM_WIFI_JOINED&&
      strncmp((char *)original_ssid.ssid, (char *)ssid, 32) == 0 &&
      strncmp((char *)original_pwd.psk, (char *)password, 64) == 0) {
    // we're already happily connected to the target AP, thus we don't need to do anything
    if (jsvIsFunction(jsCallback)) {
      JsVar *params[1];
      params[0] = jsvNewNull();
      jsiQueueEvents(NULL, jsCallback, params, 1);  // TODO: fix callback params and unlock...
      jsvUnLock(params[0]);
    }
    return;
  } else {
    // we're not happily connected to the right AP, so disconnect to start over
    tls_wifi_disconnect();
    // we skip the disconnect event unless we're connected (then it's legit) and unless
    // we're idle/off (then there is no disconnect event to start with)
    g_skipDisconnect = wifiConnectStatus != WM_WIFI_JOINED;

    uint8_t opmode=0;
    tls_param_get(TLS_PARAM_ID_WPROTOCOL,&opmode,false);
    opmode|=TLS_PARAM_IEEE80211_INFRA;
    tls_param_set(TLS_PARAM_ID_WPROTOCOL,&opmode,false);

  }

  // set callback
  if (jsvIsFunction(jsCallback)) g_jsGotIpCallback = jsvLockAgainSafe(jsCallback);

  // Do we have a child property called dnsServers?
  JsVar *jsDNSServers = jsvObjectGetChild(jsOptions, "dnsServers", 0);
  if (jsvIsArray(jsDNSServers) != false) {
    JsVarInt numDNSServers = jsvGetArrayLength(jsDNSServers);
    ip_addr_t dnsAddresses[2];
    if (numDNSServers > 0) {
      // One server
      JsVar *jsCurrentDNSServer = jsvGetArrayItem(jsDNSServers, 0);
      char buffer[50];
      size_t size = jsvGetString(jsCurrentDNSServer, buffer, sizeof(buffer)-1);
      buffer[size] = '\0';
      jsvUnLock(jsCurrentDNSServer);
      dnsAddresses[0].addr = networkParseIPAddress(buffer);
    }
    if (numDNSServers > 1) {
      // Two servers
      JsVar *jsCurrentDNSServer = jsvGetArrayItem(jsDNSServers, 1);
      char buffer[50];
      size_t size = jsvGetString(jsCurrentDNSServer, buffer, sizeof(buffer)-1);
      buffer[size] = '\0';
      jsvUnLock(jsCurrentDNSServer);
      dnsAddresses[1].addr = networkParseIPAddress(buffer);
    }

    for(int i=0;i<numDNSServers;i++){
      tls_netif_dns_setserver(i,&dnsAddresses[i]);
    }

  }
  jsvUnLock(jsDNSServers);


  // Perform the network level connection.
  tls_wifi_connect((uint8_t *)ssid,strlen(ssid),(uint8_t *)password,strlen(password));
}

void jswrap_wifi_disconnect(JsVar *jsCallback){

  // Free any existing callback, then register new callback
  if (g_jsDisconnectCallback != NULL) jsvUnLock(g_jsDisconnectCallback);
  g_jsDisconnectCallback = NULL;
  if (jsCallback != NULL && !jsvIsUndefined(jsCallback) && !jsvIsFunction(jsCallback)) {
    EXPECT_CB_EXCEPTION(jsCallback);
    return;
  }
  g_jsDisconnectCallback = jsvLockAgainSafe(jsCallback);

  enum tls_wifi_states wifiConnectStatus = tls_wifi_get_state();

  // Do the disconnect, we ignore errors 'cause we don't care if we're not currently connected
  tls_wifi_disconnect();

  if (wifiConnectStatus == WM_WIFI_JOINED) {
    // If we're connected we let the event handler turn off wifi so we can cleanly disconnect
    // The event handler will also make the callback
    g_disconnecting = true;
  } else {
    // We're not really connected, so we might as well make the callback right here
    uint8_t opmode=0;
    tls_param_get(TLS_PARAM_ID_WPROTOCOL,&opmode,false);
    opmode&=TLS_PARAM_IEEE80211_SOFTAP;
    tls_param_set(TLS_PARAM_ID_WPROTOCOL,&opmode,false);

    g_disconnecting = false;
    if (jsvIsFunction(jsCallback)) {
      jsiQueueEvents(NULL, jsCallback, NULL, 0);
    }
  }
}

void jswrap_wifi_startAP(JsVar *jsSsid, JsVar *jsOptions, JsVar *jsCallback){

  // Check callback
  if (jsCallback != NULL && !jsvIsUndefined(jsCallback) && !jsvIsFunction(jsCallback)) {
    EXPECT_CB_EXCEPTION(jsCallback);
    return;
  }

  // Validate that the SSID is provided and is a string.
  if (!jsvIsString(jsSsid)) {
      jsExceptionHere(JSET_ERROR, "No SSID.");
    return;
  }

  // Make sure jsOptions is NULL or an object
  if (jsOptions != NULL && !jsvIsNull(jsOptions) && !jsvIsObject(jsOptions)) {
    EXPECT_OPT_EXCEPTION(jsOptions);
    return;
  }

  memset(softap_clients,0,sizeof(wifi_softap_client_t)*ETH_ALEN);

  struct tls_softap_info_t ap_info;
  memset(&ap_info, 0, sizeof(struct tls_softap_info_t));

  ap_info.encrypt=IEEE80211_ENCRYT_NONE;
  jsvGetString(jsSsid,(char *)ap_info.ssid,33);


  // Handle any options that may have been supplied.
  if (jsvIsObject(jsOptions)) {

    // Handle channel
    JsVar *jsChan = jsvObjectGetChild(jsOptions, "channel", 0);
    if (jsvIsInt(jsChan)) {
      int chan = jsvGetInteger(jsChan);
      if (chan >= 1 && chan <= 13) ap_info.channel = chan;
    }
    jsvUnLock(jsChan);

    // Handle password
    JsVar *jsPassword = jsvObjectGetChild(jsOptions, "password", 0);
    if (jsPassword != NULL) {
      // handle password:null
      if (jsvGetStringLength(jsPassword) != 0) {
        if (!jsvIsString(jsPassword) || jsvGetStringLength(jsPassword) < 8) {
          jsExceptionHere(JSET_ERROR, "Password must be string of at least 8 characters");
          jsvUnLock(jsPassword);
          return;
        }

        ap_info.keyinfo.key_len=jsvGetString(jsPassword, (char *)ap_info.keyinfo.key, 64);
        ap_info.keyinfo.format=1;
      }
    }
    jsvUnLock(jsPassword);

    // Handle "authMode" processing.  Here we check that "authMode", if supplied, is
    // one of the allowed values and set the softApConfig object property appropriately.
    JsVar *jsAuth = jsvObjectGetChild(jsOptions, "authMode", 0);
    if (jsvIsString(jsAuth)) {
      if (jsvIsStringEqual(jsAuth, "open")) {
        ap_info.encrypt=IEEE80211_ENCRYT_NONE;
      } else if (jsvIsStringEqual(jsAuth, "wpa2")) {
        ap_info.encrypt=IEEE80211_ENCRYT_CCMP_WPA2;
      } else if (jsvIsStringEqual(jsAuth, "wpa")) {
        ap_info.encrypt = IEEE80211_ENCRYT_CCMP_WPA;
      } else {
        jsvUnLock(jsAuth);
        jsExceptionHere(JSET_ERROR, "Unknown authMode value.");
        return;
      }
    } else {
      // no explicit auth mode, set according to presence of password
      ap_info.encrypt = ap_info.keyinfo.key[0] == 0 ? IEEE80211_ENCRYT_NONE : IEEE80211_ENCRYT_CCMP_WPA2;
    }
    jsvUnLock(jsAuth);

    // Make sure password and authmode match
    if (ap_info.encrypt != IEEE80211_ENCRYT_NONE && ap_info.keyinfo.key[0] == 0) {
      jsExceptionHere(JSET_ERROR, "Password not set but authMode not open.");
      return;
    }
    if (ap_info.encrypt == IEEE80211_ENCRYT_NONE && ap_info.keyinfo.key[0] != 0) {
      jsExceptionHere(JSET_ERROR, "Auth mode set to open but password supplied.");
      return;
    }
  }

  // Define that we are in Soft AP mode including station mode if required.
  uint8_t opmode=0;
  tls_param_get(TLS_PARAM_ID_WPROTOCOL,&opmode,false);
  opmode|=TLS_PARAM_IEEE80211_SOFTAP;
  tls_param_set(TLS_PARAM_ID_WPROTOCOL,&opmode,false);

  struct tls_ip_info_t ip_info;
  memset(&ip_info,0,sizeof(struct tls_ip_info_t));
  ip_info.ip_addr[0]=192;
  ip_info.ip_addr[1]=168;
  ip_info.ip_addr[2]=8;
  ip_info.ip_addr[3]=1;
  ip_info.netmask[0]=255;
  ip_info.netmask[1]=255;
  ip_info.netmask[2]=255;
  ip_info.netmask[3]=0;
  memcpy(ip_info.dnsname,"local.beanjs",12);


  // Set the WiFi configuration.
  bool ok= tls_wifi_softap_create(&ap_info,&ip_info)==WM_SUCCESS;

  // Is this still true:
  // We should really check that becoming an access point works, however as of SDK 1.4, we
  // are finding that if we are currently connected to an access point and we switch to being
  // an access point, it works ... but returns 1 indicating an error.
  //if (!rc) DBG("Error %d returned from wifi_softap_set_config, probably ignore...\n", rc);

  if (jsCallback != NULL) {
    // Set the return error as a function of the return code returned from the call to
    // the W600 API to create the AP
    JsVar *params[1];
    params[0] = ok ? jsvNewNull() : jsvNewFromString("Error from wifi_softap_set_config");
    jsiQueueEvents(NULL, jsCallback, params, 1);
    jsvUnLock(params[0]);
  }
}

void jswrap_wifi_stopAP(JsVar *jsCallback){

  // handle the callback parameter
  if (jsCallback != NULL && !jsvIsUndefined(jsCallback) && !jsvIsFunction(jsCallback)) {
    EXPECT_CB_EXCEPTION(jsCallback);
    return;
  }

  // Change operating mode
  uint8_t opmode=0;
  tls_param_get(TLS_PARAM_ID_WPROTOCOL,&opmode,false);
  opmode&=~TLS_PARAM_IEEE80211_SOFTAP;
  tls_param_set(TLS_PARAM_ID_WPROTOCOL,&opmode,false);

  tls_wifi_softap_destroy();

  if (jsvIsFunction(jsCallback)) {
    jsiQueueEvents(NULL, jsCallback, NULL, 0);
  }

}

void jswrap_wifi_scan(JsVar *jsCallback){

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
  g_disconnecting = false; // we don't want that to interfere

  // Ask the W600 to perform a network scan after first entering
  // station mode.  The network scan will eventually result in a callback
  // being executed (scanCB) which will contain the results.
  uint8_t opmode=0;
  tls_param_get(TLS_PARAM_ID_WPROTOCOL,&opmode,false);
  opmode|=TLS_PARAM_IEEE80211_INFRA;
  tls_param_set(TLS_PARAM_ID_WPROTOCOL,&opmode,false);

  // Request a scan of the network calling "scanCB" on completion
  tls_wifi_scan();
}

JsVar *jswrap_wifi_getStatus(JsVar *jsCallback){

  // Check callback
  if (jsCallback != NULL && !jsvIsNull(jsCallback) && !jsvIsFunction(jsCallback)) {
    EXPECT_CB_EXCEPTION(jsCallback);
    return NULL;
  }

  uint8_t opMode=0;
  tls_param_get(TLS_PARAM_ID_WPROTOCOL,&opMode,false);
  enum tls_wifi_states wifiConnectStatus = tls_wifi_get_state();

  JsVar *jsWiFiStatus = jsvNewObject();

  jsvObjectSetChildAndUnLock(jsWiFiStatus, "mode", jsvNewFromString(wifi_mode[opMode]));
  jsvObjectSetChildAndUnLock(jsWiFiStatus, "station", jsvNewFromString((opMode&TLS_PARAM_IEEE80211_INFRA) ? wifi_conn[wifiConnectStatus] : "off"));
  jsvObjectSetChildAndUnLock(jsWiFiStatus, "ap", jsvNewFromString((opMode & TLS_PARAM_IEEE80211_SOFTAP) ? "enabled" : "disabled"));

  // Schedule callback if a function was provided
  if (jsvIsFunction(jsCallback)) {
    JsVar *params[1];
    params[0] = jsWiFiStatus;
    jsiQueueEvents(NULL, jsCallback, params, 1);
  }

  return jsWiFiStatus;

}

JsVar *jswrap_wifi_getDetails(JsVar *jsCallback){

  // Check callback
  if (jsCallback != NULL && !jsvIsNull(jsCallback) && !jsvIsFunction(jsCallback)) {
    EXPECT_CB_EXCEPTION(jsCallback);
    return NULL;
  }

  enum tls_wifi_states wifiConnectStatus = tls_wifi_get_state();

  JsVar *jsDetails = jsvNewObject();

  jsvObjectSetChildAndUnLock(jsDetails, "status", jsvNewFromString(wifi_conn[wifiConnectStatus]));

  struct tls_param_ssid original_ssid;
  struct tls_param_original_key original_pwd;
  memset(&original_ssid,0,sizeof(struct tls_param_ssid));
  memset(&original_pwd,0,sizeof(struct tls_param_original_key));

  tls_param_get(TLS_PARAM_ID_ORIGIN_SSID,&original_ssid,false);
  tls_param_get(TLS_PARAM_ID_ORIGIN_KEY,&original_ssid,false);

  char buf[65];
  // ssid
  strncpy(buf, (char *)original_ssid.ssid, 32);
  buf[32] = 0;
  jsvObjectSetChildAndUnLock(jsDetails, "ssid", jsvNewFromString(buf));
  // password
  strncpy(buf, (char *)original_pwd.psk, 64);
  buf[64] = 0;
  jsvObjectSetChildAndUnLock(jsDetails, "password", jsvNewFromString(buf));

  // Schedule callback if a function was provided
  if (jsvIsFunction(jsCallback)) {
    JsVar *params[1];
    params[0] = jsDetails;
    jsiQueueEvents(NULL, jsCallback, params, 1);
  }

  return jsDetails;
}

JsVar *jswrap_wifi_getAPDetails(JsVar *jsCallback){

  // Check callback
  if (jsCallback != NULL && !jsvIsNull(jsCallback) && !jsvIsFunction(jsCallback)) {
    EXPECT_CB_EXCEPTION(jsCallback);
    return NULL;
  }

  uint8_t opMode=0;
  tls_param_get(TLS_PARAM_ID_WPROTOCOL,&opMode,false);

  JsVar *jsDetails = jsvNewObject();

  jsvObjectSetChildAndUnLock(jsDetails, "status",jsvNewFromString(opMode & TLS_PARAM_IEEE80211_SOFTAP ? "enabled" : "disabled"));

  if(opMode & TLS_PARAM_IEEE80211_SOFTAP){
    uint8_t authMode=0;
    tls_param_get(TLS_PARAM_ID_SOFTAP_ENCRY,&authMode,false);
    jsvObjectSetChildAndUnLock(jsDetails, "authMode", jsvNewFromString(wifi_auths[authMode]));
    
    char buf[65];
    // ssid
    struct tls_param_ssid tls_ssid;
    tls_param_get(TLS_PARAM_ID_SOFTAP_SSID,&tls_ssid,false);
    strncpy(buf, (char *)tls_ssid.ssid, 32);
    buf[32] = 0;
    jsvObjectSetChildAndUnLock(jsDetails, "ssid", jsvNewFromString(buf));
    // password
    struct tls_param_key tls_key;
    tls_param_get(TLS_PARAM_ID_SOFTAP_KEY,&tls_key,false);
    strncpy(buf, (char *)tls_key.psk, 64);
    buf[64] = 0;
    jsvObjectSetChildAndUnLock(jsDetails, "password", jsvNewFromString(buf));


    JsVar *jsArray = jsvNewArray(NULL, 0);
    char macAddrString[6*3 + 1];

    for (uint32_t i = 0; i < ETH_ALEN; i++){
      wifi_softap_client_t *client=&softap_clients[i];
      if(client->used){
        JsVar *jsSta = jsvNewObject();
        jsvObjectSetChildAndUnLock(jsSta, "ip",networkGetAddressAsString((uint8_t *)&tls_dhcps_getip(client->mac)->addr, 4, 10, '.'));
        sprintf(macAddrString, macFmt,
          client->mac[0], client->mac[1], client->mac[2],
          client->mac[3], client->mac[4], client->mac[5]);
        jsvObjectSetChildAndUnLock(jsSta, "mac", jsvNewFromString(macAddrString));
        jsvArrayPush(jsArray, jsSta);
        jsvUnLock(jsSta);
      }
    }

    jsvObjectSetChildAndUnLock(jsDetails, "stations", jsArray);
  }

  // Schedule callback if a function was provided
  if (jsvIsFunction(jsCallback)) {
    JsVar *params[1];
    params[0] = jsDetails;
    jsiQueueEvents(NULL, jsCallback, params, 1);
  }

  return jsDetails;
}

JsVar *jswrap_wifi_getIP(JsVar *jsCallback){
  return get_ip_info(jsCallback,0);
}

JsVar *jswrap_wifi_getAPIP(JsVar *jsCallback){
 return get_ip_info(jsCallback,1);
}

void   jswrap_wifi_setIP(JsVar *jsSettings, JsVar *jsCallback){
  set_ip_info(jsCallback,jsSettings,0);
}

void   jswrap_wifi_setAPIP(JsVar *jsSettings, JsVar *jsCallback){
  set_ip_info(jsCallback,jsSettings,1);
}



void jswrap_wifi_init(){

  jswrap_wifi_reset();

  uint8_t auto_connect=WIFI_AUTO_CNT_ON;
  tls_wifi_auto_connect_flag(WIFI_AUTO_CNT_FLAG_SET,&auto_connect);
  tls_netif_add_status_event(wifi_status_change_callback);
  tls_wifi_softap_client_event_register(wifi_softap_client_callback);
  tls_wifi_scan_result_cb_register(wifi_scan_result_callback);
}

void jswrap_wifi_reset(){
  g_jsGotIpCallback = NULL;
  g_jsScanCallback = NULL;
  g_jsDisconnectCallback = NULL;
  g_disconnecting = false;
}

/*JSON{
  "type":"init",
  "generate":"jswrap_wifi_soft_init"
}*/
void jswrap_wifi_soft_init(){
  JsNetwork net;
  networkCreate(&net, JSNETWORKTYPE_W600);
  networkState = NETWORKSTATE_ONLINE;
}


