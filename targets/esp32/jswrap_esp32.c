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
#include "esp_ota_ops.h"

#include "driver/rtc_io.h"

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
  "name"     : "deepSleepExt0",
  "generate" : "jswrap_ESP32_deepSleep_ext0",
  "params"   : [
    ["pin", "pin", "Pin to trigger wakeup"],
    ["level", "int", "Logic level to trigger"]
  ]
}
Put device in deepsleep state until interrupted by pin "pin".
Eligible pin numbers are restricted to those [GPIOs designated
as RTC GPIOs](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/gpio.html#gpio-summary).
*/
void jswrap_ESP32_deepSleep_ext0(Pin pin, int level) {
  if (!rtc_gpio_is_valid_gpio(pin)) {
    jsExceptionHere(JSET_ERROR, "Invalid pin!");
    return;
  }
  esp_sleep_enable_ext0_wakeup(pin, level);
  esp_deep_sleep_start(); // This function does not return.
} // End of jswrap_ESP32_deepSleep_ext0

/*JSON{
  "type"     : "staticmethod",
  "class"    : "ESP32",
  "ifdef" : "ESP32",
  "name"     : "getWakeupCause",
  "generate" : "jswrap_ESP32_getWakeupCause",
  "return"   : ["int", "The cause of the ESP32's wakeup from sleep"]
}
Returns a variable identifying the cause of wakeup from deep sleep.
Possible causes include:

* `0: ESP_SLEEP_WAKEUP_UNDEFINED` - reset was not caused by exit from deep sleep
* `2: ESP_SLEEP_WAKEUP_EXT0` - Wakeup caused by external signal using RTC_IO
* `3: ESP_SLEEP_WAKEUP_EXT1` - Wakeup caused by external signal using RTC_CNTL
* `4: ESP_SLEEP_WAKEUP_TIMER` - Wakeup caused by timer
* `5: ESP_SLEEP_WAKEUP_TOUCHPAD` - Wakeup caused by touchpad
* `6: ESP_SLEEP_WAKEUP_ULP` - Wakeup caused by ULP program

*/
int jswrap_ESP32_getWakeupCause() {
  return esp_sleep_get_wakeup_cause();
} // End of jswrap_ESP32_getWakeupCause


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
 "ifdef"  : "BLUETOOTH"
}
*/
void jswrap_ESP32_setBLE_Debug(int level){
  ESP32_setBLE_Debug(level);
}

/*JSON{
 "type"  : "staticmethod",
 "class"  : "ESP32",
 "ifdef" : "ESP32",
 "name"    : "enableBLE",
 "generate"  : "jswrap_ESP32_enableBLE",
 "params"  : [
   ["enable", "bool", "switches Bluetooth on or off" ]
 ],
 "ifdef"  : "BLUETOOTH" 
}
Switches Bluetooth off/on, removes saved code from Flash, resets the board, and
on restart creates jsVars depending on available heap (actual additional 1800)
*/
void jswrap_ESP32_enableBLE(bool enable) { //may be later, we will support BLEenable(ALL/SERVER/CLIENT)
  ESP32_Set_NVS_Status(ESP_NETWORK_BLE,enable);
  jsfRemoveCodeFromFlash();
  esp_restart();
}
#endif
/*JSON{
 "type"  : "staticmethod",
 "class"  : "ESP32",
 "ifdef" : "ESP32",
 "name"    : "enableWifi",
 "generate"  : "jswrap_ESP32_enableWifi",
 "params"  : [
   ["enable", "bool", "switches Wifi on or off" ]
 ] 
}
Switches Wifi off/on, removes saved code from Flash, resets the board, and on
restart creates jsVars depending on available heap (actual additional 3900)
*/
void jswrap_ESP32_enableWifi(bool enable) { //may be later, we will support BLEenable(ALL/SERVER/CLIENT)
  ESP32_Set_NVS_Status(ESP_NETWORK_WIFI,enable);
  jsfRemoveCodeFromFlash();
  esp_restart();
}

/*JSON{
 "type" : "staticmethod",
 "class"  : "ESP32",
 "ifdef" : "ESP32",
 "name"   : "setOTAValid",
 "generate" : "jswrap_ESP32_setOTAValid",
 "params" : [
   ["isValid", "bool", "Set whether this app is valid or not. If `isValid==false` the device will reboot." ]
 ]
}
This function is useful for ESP32 [OTA Updates](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/ota.html)

Normally Espruino is uploaded to the `factory` partition so this isn't so useful,
but it is possible to upload Espruino to the `ota_0` partition (or ota_1 if a different table has been added).

If this is the case, you can use this function to mark the currently running version of Espruino as good or bad.
 * If set as valid, Espruino will continue running, and the fact that everything is ok is written to flash
 * If set as invalid (false) Espruino will mark itself as not working properly and will reboot. The ESP32 bootloader
 will then start and will load any other partition it can find that is marked as ok.
*/
void jswrap_ESP32_setOTAValid(bool isValid) {
  if (isValid) esp_ota_mark_app_valid_cancel_rollback();
  else esp_ota_mark_app_invalid_rollback_and_reboot();
}

