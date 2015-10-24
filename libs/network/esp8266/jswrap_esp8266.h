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

void   jswrap_ESP8266WiFi_beAccessPoint(JsVar *jsv_ssid, JsVar *jsv_password);
void   jswrap_ESP8266WiFi_connect(JsVar *jsv_ssid, JsVar *jsv_password, JsVar *gotIpCallback);
void   jswrap_ESP8266WiFi_disconnect();
void   jswrap_ESP8266WiFi_dumpSocket(JsVar *socketId);
void   jswrap_ESP8266WiFi_getAccessPoints(JsVar *callback);
JsVar *jswrap_ESP8266WiFi_getAddressAsString(JsVar *address);
JsVar *jswrap_ESP8266WiFi_getAutoConnect();
JsVar *jswrap_ESP8266WiFi_getConnectedStations();
JsVar *jswrap_ESP8266WiFi_getConnectStatus();
JsVar *jswrap_ESP8266WiFi_getDHCPHostname();
void   jswrap_ESP8266WiFi_getHostByName(JsVar *jsHostname, JsVar *jsCallback);
JsVar *jswrap_ESP8266WiFi_getIPInfo();
JsVar *jswrap_ESP8266WiFi_getRSSI();
JsVar *jswrap_ESP8266WiFi_getRstInfo();
JsVar *jswrap_ESP8266WiFi_getState();
JsVar *jswrap_ESP8266WiFi_getStationConfig();
void   jswrap_ESP8266WiFi_init();
void   jswrap_ESP8266WiFi_kill();
void   jswrap_ESP8266WiFi_logDebug(JsVar *jsDebug);
void   jswrap_ESP8266WiFi_mdnsInit();
void   jswrap_ESP8266WiFi_onWiFiEvent(JsVar *callback);
void   jswrap_ESP8266WiFi_ping(JsVar *ipAddr, JsVar *pingCallback);
void   jswrap_ESP8266WiFi_restart();
void   jswrap_ESP8266WiFi_setAutoConnect(JsVar *autoconnect);
void   jswrap_ESP8266WiFi_setDHCPHostname(JsVar *jsHostname);
JsVar *jswrap_ESP8266WiFi_socketConnect(JsVar *options, JsVar *callback);
void   jswrap_ESP8266WiFi_socketEnd(JsVar *socket, JsVar *data);
void   jswrap_ESP8266WiFi_stopAP();
void   jswrap_ESP8266WiFi_updateCPUFreq(JsVar *jsFreq);


void   jswrap_ESP8266_wifi_connect(JsVar *jsSsid, JsVar *jsPassword, JsVar *jsOptions, JsVar *jsCallback);
void   jswrap_ESP8266_wifi_createAP(JsVar *jsSsid, JsVar *jsPassword, JsVar *jsOptions, JsVar *jsCallback);
void   jswrap_ESP8266_wifi_disconnect();
JsVar *jswrap_ESP8266_wifi_getIP();
void   jswrap_ESP8266_wifi_scan(JsVar *jsCallback);

#endif /* LIBS_NETWORK_ESP8266_JSWRAP_ESP8266_H_ */
