void jswrap_ESP32_wifi_soft_init();
void jswrap_ESP32_wifi_disconnect(JsVar *jsCallback);
void jswrap_ESP32_wifi_stopAP(JsVar *jsCallback);
void jswrap_ESP32_wifi_connect(
    JsVar *jsSsid,
    JsVar *jsOptions,
    JsVar *jsCallback
  );
void jswrap_ESP32_wifi_scan(JsVar *jsCallback);
void jswrap_ESP32_wifi_startAP(
    JsVar *jsSsid,     //!< The network SSID that we will use to listen as.
    JsVar *jsOptions,  //!< Configuration options.
    JsVar *jsCallback  //!< A callback to be invoked when completed.
  );
JsVar *jswrap_ESP32_wifi_getStatus(JsVar *jsCallback);
void jswrap_ESP32_wifi_setConfig(JsVar *jsSettings);
JsVar *jswrap_ESP32_wifi_getDetails(JsVar *jsCallback);
JsVar *jswrap_ESP32_wifi_getAPDetails(JsVar *jsCallback);
void jswrap_ESP32_wifi_save(JsVar *what);
void jswrap_ESP32_wifi_restore(void);
JsVar *jswrap_ESP32_wifi_getIP(JsVar *jsCallback);
JsVar *jswrap_ESP32_wifi_getAPIP(JsVar *jsCallback);
void jswrap_ESP32_wifi_getHostByName(
    JsVar *jsHostname,
    JsVar *jsCallback
);
JsVar *jswrap_ESP32_wifi_getHostname(JsVar *jsCallback);
void jswrap_ESP32_wifi_setHostname(
    JsVar *jsHostname //!< The hostname to set for device.
);
void jswrap_ESP32_ping(
    JsVar *ipAddr,      //!< A string or integer representation of an IP address.
    JsVar *pingCallback //!< Optional callback function.
);

void esp32_wifi_init();
