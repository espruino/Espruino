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

#include "esp_system.h"
#include "esp_log.h"
#include "app_update/include/esp_ota_ops.h"

static char *tag = "jswrap_esp32";

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
  ESP_LOGD(tag, ">> jswrap_ESP32_reboot");
  esp_restart(); // Call the ESP-IDF to restart the ESP32.
  ESP_LOGD(tag, "<< jswrap_ESP32_reboot");
} // End of jswrap_ESP32_reboot


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
  esp_partition_t * partition=esp_ota_get_boot_partition();
  jsvObjectSetChildAndUnLock(esp32State, "addr",     jsvNewFromInteger(partition->address));
  jsvObjectSetChildAndUnLock(esp32State, "partitionBoot", jsvNewFromString( partition->label));
  //jsvObjectSetChildAndUnLock(esp32State, "partitionRunning",   jsvNewFromString( esp_ota_get_running_partition()->label));
  //jsvObjectSetChildAndUnLock(esp32State, "partitionNext",   jsvNewFromString( esp_ota_get_next_update_partition(NULL)->label));
  return esp32State;
} // End of jswrap_ESP32_getState


/*JSON{
  "type"     : "staticmethod",
  "class"    : "ESP32",
  "name"     : "setLogLevel",
  "generate" : "jswrap_ESP32_setLogLevel",
  "params"   : [
   ["tag", "JsVar", "The tag to set logging."],
   ["logLevel", "JsVar", "The log level to set."]
   ]
}
Set the logLevel for the corresponding debug tag.  If tag is `*` then we reset all
tags to this logLevel.  The logLevel may be one of:
* verbose
* debug
* info
* warn
* error
* none
*/
/**
 * The ESP-IDF provides a logging/debug mechanism where logging statements can be inserted
 * into the code.  At run time, the logging levels can be adjusted dynamically through
 * a call to esp_log_level_set.  This allows us to selectively switch on or off
 * distinct log levels.  Imagine a situation where you have no logging (normal status)
 * and something isn't working as desired.  Now what you can do is switch on all logging
 * or a subset of logging through this JavaScript API.
 */
void jswrap_ESP32_setLogLevel(JsVar *jsTagToSet, JsVar *jsLogLevel) {
  char tagToSetStr[20];
  esp_log_level_t level;

  ESP_LOGD(tag, ">> jswrap_ESP32_setLogLevel");
  // TODO: Add guards for invalid parameters.
  jsvGetString(jsTagToSet, tagToSetStr, sizeof(tagToSetStr));

  // Examine the level string and see what kind of level it is.
  if (jsvIsStringEqual(jsLogLevel, "verbose")) {
    level = ESP_LOG_VERBOSE;
  } else if (jsvIsStringEqual(jsLogLevel, "debug")) {
    level = ESP_LOG_DEBUG;
  } else if (jsvIsStringEqual(jsLogLevel, "info")) {
    level = ESP_LOG_INFO;
  } else if (jsvIsStringEqual(jsLogLevel, "warn")) {
    level = ESP_LOG_WARN;
  } else if (jsvIsStringEqual(jsLogLevel, "error")) {
    level = ESP_LOG_ERROR;
  } else if (jsvIsStringEqual(jsLogLevel, "none")) {
    level = ESP_LOG_NONE;
  } else {
    ESP_LOGW(tag, "<< jswrap_ESP32_setLogLevel - Unknown log level");
    return;
  }
  esp_log_level_set(tagToSetStr, level); // Call the ESP-IDF to set the log level for the given tag.
  ESP_LOGD(tag, "<< jswrap_ESP32_setLogLevel");
  return;
} // End of jswrap_ESP32_setLogLevel
