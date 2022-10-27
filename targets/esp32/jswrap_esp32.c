/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2013 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * ESP32 specific exposed components.
 * ----------------------------------------------------------------------------
 */
#include <stdio.h>

#include "jswrap_esp32.h"
#include "jshardwareAnalog.h"
#include "jsutils.h"
#include "jsinteractive.h"
#include "jsparse.h"
#include "jsflash.h"

#include "esp_system.h"
#include "esp_sleep.h"
#include "esp_heap_caps.h"

#ifdef BLUETOOTH
#include "BLE/esp32_bluetooth_utils.h"
#endif
#include "jshardwareESP32.h"

#include "jsutils.h"
#include "jsinteractive.h"
#include "jsparse.h"

/*JSON{
  "type": "class",
  "class" : "ESP32",
  "ifdef" : "ESP32"
}
Class containing utility functions for the
[ESP32](http://www.espruino.com/ESP32)
*/

/*JSON{
 "type"     : "staticmethod",
 "class"    : "ESP32",
 "ifdef" : "ESP32",
 "name"     : "setAtten",
 "generate" : "jswrap_ESP32_setAtten",
 "params"   : [
   ["pin", "pin", "Pin for Analog read"],
   ["atten", "int", "Attenuate factor"]
 ]
}*/
void jswrap_ESP32_setAtten(Pin pin,int atten){
  printf("Atten:%d\n",atten);
  rangeADC(pin, atten);
}

/*JSON{
  "type"     : "staticmethod",
  "class"    : "ESP32",
  "ifdef" : "ESP32",
  "name"     : "reboot",
  "generate" : "jswrap_ESP32_reboot"
}
Perform a hardware reset/reboot of the ESP32.
*/
void jswrap_ESP32_reboot() {
  jshReboot();
} // End of jswrap_ESP32_reboot


/*JSON{
  "type"     : "staticmethod",
  "class"    : "ESP32",
  "ifdef" : "ESP32",
  "name"     : "deepSleep",
  "generate" : "jswrap_ESP32_deepSleep",
  "params"   : [ ["us", "int", "Sleeptime in us"] ]
}
Put device in deepsleep state for "us" microseconds.
*/
void jswrap_ESP32_deepSleep(int us) {
  esp_sleep_enable_timer_wakeup((uint64_t)(us));
  esp_deep_sleep_start(); // This function does not return.
} // End of jswrap_ESP32_deepSleep


/*JSON{
  "type"     : "staticmethod",
  "class"    : "ESP32",
  "ifdef" : "ESP32",
  "name"     : "getState",
  "generate" : "jswrap_ESP32_getState",
  "return"   : ["JsVar", "The state of the ESP32"]
}
Returns an object that contains details about the state of the ESP32 with the
following fields:

* `sdkVersion` - Version of the SDK.
* `freeHeap` - Amount of free heap in bytes.
* `BLE` - Status of BLE, enabled if true.
* `Wifi` - Status of Wifi, enabled if true.
* `minHeap` - Minimum heap, calculated by heap_caps_get_minimum_free_size

*/
JsVar *jswrap_ESP32_getState() {
  // Create a new variable and populate it with the properties of the ESP32 that we
  // wish to return.
  JsVar *esp32State = jsvNewObject();
  jsvObjectSetChildAndUnLock(esp32State, "sdkVersion",   jsvNewFromString(esp_get_idf_version()));
  jsvObjectSetChildAndUnLock(esp32State, "freeHeap",     jsvNewFromInteger(esp_get_free_heap_size()));
  jsvObjectSetChildAndUnLock(esp32State, "BLE",          jsvNewFromBool(ESP32_Get_NVS_Status(ESP_NETWORK_BLE)));
  jsvObjectSetChildAndUnLock(esp32State, "Wifi",         jsvNewFromBool(ESP32_Get_NVS_Status(ESP_NETWORK_WIFI)));  
  jsvObjectSetChildAndUnLock(esp32State, "minHeap",      jsvNewFromInteger(heap_caps_get_minimum_free_size(MALLOC_CAP_8BIT)));
  return esp32State;
} // End of jswrap_ESP32_getState

#ifdef BLUETOOTH
/*JSON{
 "type"     : "staticmethod",
 "class"    : "ESP32",
 "ifdef" : "ESP32",
 "name"     : "setBLE_Debug",
 "generate" : "jswrap_ESP32_setBLE_Debug",
 "params"   : [
   ["level", "int", "which events should be shown (GAP=1, GATTS=2, GATTC=4). Use 255 for everything"]
 ],
 "ifdef"	: "BLUETOOTH"
}
*/
void jswrap_ESP32_setBLE_Debug(int level){
	ESP32_setBLE_Debug(level);
}

/*JSON{
 "type"	: "staticmethod",
 "class"	: "ESP32",
 "ifdef" : "ESP32",
 "name"		: "enableBLE",
 "generate"	: "jswrap_ESP32_enableBLE",
 "params"	: [
   ["enable", "bool", "switches Bluetooth on or off" ]
 ],
 "ifdef"	: "BLUETOOTH" 
}
Switches Bluetooth off/on, removes saved code from Flash, resets the board, and
on restart creates jsVars depending on available heap (actual additional 1800)
*/
void jswrap_ESP32_enableBLE(bool enable){ //may be later, we will support BLEenable(ALL/SERVER/CLIENT)
  ESP32_Set_NVS_Status(ESP_NETWORK_BLE,enable);
  jsfRemoveCodeFromFlash();
  esp_restart();
}
#endif
/*JSON{
 "type"	: "staticmethod",
 "class"	: "ESP32",
 "ifdef" : "ESP32",
 "name"		: "enableWifi",
 "generate"	: "jswrap_ESP32_enableWifi",
 "params"	: [
   ["enable", "bool", "switches Wifi on or off" ]
 ] 
}
Switches Wifi off/on, removes saved code from Flash, resets the board, and on
restart creates jsVars depending on available heap (actual additional 3900)
*/
void jswrap_ESP32_enableWifi(bool enable){ //may be later, we will support BLEenable(ALL/SERVER/CLIENT)
  ESP32_Set_NVS_Status(ESP_NETWORK_WIFI,enable);
  jsfRemoveCodeFromFlash();
  esp_restart();
}
