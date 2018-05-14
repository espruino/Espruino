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

#include "esp_system.h"
#include "esp_sleep.h"

#ifdef BLUETOOTH
#include "BLE/esp32_bluetooth_utils.h"
#endif

#include "jsutils.h"
#include "jsinteractive.h"
#include "jsparse.h"

/*JSON{
 "type"     : "staticmethod",
 "class"    : "ESP32",
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
  "name"     : "reboot",
  "generate" : "jswrap_ESP32_reboot"
}
Perform a hardware reset/reboot of the ESP32.
*/
void jswrap_ESP32_reboot() {
  esp_restart(); // Call the ESP-IDF to restart the ESP32.
} // End of jswrap_ESP32_reboot


/*JSON{
  "type"     : "staticmethod",
  "class"    : "ESP32",
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
  "name"     : "getState",
  "generate" : "jswrap_ESP32_getState",
  "return"   : ["JsVar", "The state of the ESP32"]
}
Returns an object that contains details about the state of the ESP32 with the following fields:

* `sdkVersion`   - Version of the SDK.
* `freeHeap`     - Amount of free heap in bytes.

*/
JsVar *jswrap_ESP32_getState() {
  // Create a new variable and populate it with the properties of the ESP32 that we
  // wish to return.
  JsVar *esp32State = jsvNewObject();
  jsvObjectSetChildAndUnLock(esp32State, "sdkVersion",   jsvNewFromString(esp_get_idf_version()));
  jsvObjectSetChildAndUnLock(esp32State, "freeHeap",     jsvNewFromInteger(esp_get_free_heap_size()));
  return esp32State;
} // End of jswrap_ESP32_getState

#ifdef BLUETOOTH
/*JSON{
 "type"     : "staticmethod",
 "class"    : "ESP32",
 "name"     : "setBLE_Debug",
 "generate" : "jswrap_ESP32_setBLE_Debug",
 "params"   : [
   ["level", "int", "which events should be shown (GATTS, GATTC, GAP)"]
 ],
 "ifdef"	: "BLUETOOTH"
}
*/
void jswrap_ESP32_setBLE_Debug(int level){
	ESP32_setBLE_Debug(level);
}
#endif
