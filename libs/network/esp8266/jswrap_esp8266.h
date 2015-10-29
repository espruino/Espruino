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
 * Contains ESP8266 board specific function definitions.
 * ----------------------------------------------------------------------------
 */

#ifndef LIBS_NETWORK_ESP8266_JSWRAP_ESP8266_H_
#define LIBS_NETWORK_ESP8266_JSWRAP_ESP8266_H_
#include "jsvar.h"

// Deprecated
void   jswrap_ESP8266WiFi_beAccessPoint(JsVar *jsv_ssid, JsVar *jsv_password);
// Deprecated
void   jswrap_ESP8266WiFi_connect(JsVar *jsv_ssid, JsVar *jsv_password, JsVar *gotIpCallback);
// Deprecated
void   jswrap_ESP8266WiFi_disconnect();
void   jswrap_ESP8266WiFi_getAccessPoints(JsVar *callback);
// Deprecated
JsVar *jswrap_ESP8266WiFi_getAddressAsString(JsVar *address);
JsVar *jswrap_ESP8266WiFi_getAutoConnect();
JsVar *jswrap_ESP8266WiFi_getConnectedStations();
JsVar *jswrap_ESP8266WiFi_getConnectStatus();
JsVar *jswrap_ESP8266WiFi_getDHCPHostname();
// Deprecated
JsVar *jswrap_ESP8266WiFi_getIPInfo();
JsVar *jswrap_ESP8266WiFi_getRSSI();
JsVar *jswrap_ESP8266WiFi_getStationConfig();
void   jswrap_ESP8266WiFi_init();
void   jswrap_ESP8266WiFi_kill();
void   jswrap_ESP8266WiFi_mdnsInit();
void   jswrap_ESP8266WiFi_onWiFiEvent(JsVar *callback);
void   jswrap_ESP8266WiFi_setAutoConnect(JsVar *autoconnect);
void   jswrap_ESP8266WiFi_setDHCPHostname(JsVar *jsHostname);
JsVar *jswrap_ESP8266WiFi_socketConnect(JsVar *options, JsVar *callback);
void   jswrap_ESP8266WiFi_socketEnd(JsVar *socket, JsVar *data);
// Deprecated
void   jswrap_ESP8266WiFi_stopAP();

void   jswrap_ESP8266_wifi_connect(JsVar *jsSsid, JsVar *jsPassword, JsVar *jsOptions, JsVar *jsCallback);
void   jswrap_ESP8266_wifi_createAP(JsVar *jsSsid, JsVar *jsPassword, JsVar *jsOptions, JsVar *jsCallback);
void   jswrap_ESP8266_wifi_disconnect(JsVar *jsOptions, JsVar *jsCallback);
JsVar *jswrap_ESP8266_wifi_getIP();
void   jswrap_ESP8266_wifi_scan(JsVar *jsCallback);
void   jswrap_ESP8266_wifi_stopAP(JsVar *jsOptions, JsVar *jsCallback);

void   jswrap_ESP8266_dumpAllSocketData();
void   jswrap_ESP8266_dumpSocket(JsVar *jsSocketId);
JsVar *jswrap_ESP8266_getAddressAsString(JsVar *jsAddress);
void   jswrap_ESP8266_getHostByName(JsVar *jsHostname, JsVar *jsCallback);
JsVar *jswrap_ESP8266_getRstInfo();
JsVar *jswrap_ESP8266_getState();
void   jswrap_ESP8266_logDebug(JsVar *jsDebug);
void   jswrap_ESP8266_ping(JsVar *jsIpAddr, JsVar *jsPingCallback);
void   jswrap_ESP8266_restart();
void   jswrap_ESP8266_updateCPUFreq(JsVar *jsFreq);

void   jswrap_ESP8266_init();

#endif /* LIBS_NETWORK_ESP8266_JSWRAP_ESP8266_H_ */
