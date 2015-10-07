/*
 * jswrap_ESP8266WiFi.h
 *
 *  Created on: Aug 26, 2015
 *      Author: kolban
 */

#ifndef LIBS_NETWORK_ESP8266_JSWRAP_ESP8266_H_
#define LIBS_NETWORK_ESP8266_JSWRAP_ESP8266_H_
#include "jsvar.h"

void   jswrap_ESP8266WiFi_connect(JsVar *jsv_ssid, JsVar *jsv_password, JsVar *gotIpCallback);
void   jswrap_ESP8266WiFi_getAccessPoints(JsVar *callback);
void   jswrap_ESP8266WiFi_disconnect();
void   jswrap_ESP8266WiFi_restart();
JsVar *jswrap_ESP8266WiFi_getRstInfo();
JsVar *jswrap_ESP8266WiFi_getIPInfo();
void   jswrap_ESP8266WiFi_setAutoConnect(JsVar *autoconnect);
JsVar *jswrap_ESP8266WiFi_getAutoConnect();
JsVar *jswrap_ESP8266WiFi_getStationConfig();
void   jswrap_ESP8266WiFi_onWiFiEvent(JsVar *callback);
JsVar *jswrap_ESP8266WiFi_getAddressAsString(JsVar *address);
void   jswrap_ESP8266WiFi_init();
JsVar *jswrap_ESP8266WiFi_getConnectStatus();
JsVar *jswrap_ESP8266WiFi_socketConnect(JsVar *options, JsVar *callback);
void   jswrap_ESP8266WiFi_socketEnd(JsVar *socket, JsVar *data);
void   jswrap_ESP8266WiFi_ping(JsVar *ipAddr, JsVar *pingCallback);
void   jswrap_ESP8266WiFi_beAccessPoint(JsVar *jsv_ssid, JsVar *jsv_password);
JsVar *jswrap_ESP8266WiFi_getConnectedStations();
JsVar *jswrap_ESP8266WiFi_getRSSI();
JsVar *jswrap_ESP8266WiFi_getState();
void   jswrap_ESP8266WiFi_dumpSocket(JsVar *socketId);
void   jswrap_ESP8266WiFi_updateCPUFreq(JsVar *jsFreq);

#endif /* LIBS_NETWORK_ESP8266_JSWRAP_ESP8266_H_ */
