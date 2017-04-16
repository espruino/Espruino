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
#include "app_update/include/esp_ota_ops.h"

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
  "name"     : "setBoot",
  "generate" : "jswrap_ESP32_setBoot",
 "params"   : [
    ["jsPartitionName", "JsVar", "Name of ota partition to boot into next boot"]
 ],
  "return"   : ["JsVar", "Change boot partition after ota update"]
}
*/
JsVar *jswrap_ESP32_setBoot(JsVar *jsPartitionName) {
  JsVar *esp32State = jsvNewObject();  
  esp_err_t err;
  char partitionNameStr[20];

  jsvGetString(jsPartitionName, partitionNameStr, sizeof(partitionNameStr));
  esp_partition_iterator_t it = esp_partition_find(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_ANY, partitionNameStr);
  if (it==0) {
    jsError("Couldn't find partition with name %s\n", partitionNameStr);
  }
  else {
    const esp_partition_t *p = esp_partition_get(it);
    err= ESP_OK; //esp_ota_set_boot_partition(p);
    if (err!=ESP_OK) {
      jsError("Couldn't set boot partition %d!\n",err);
    } else {
      jsvObjectSetChildAndUnLock(esp32State, "addr",     jsvNewFromInteger(p->address));
      jsvObjectSetChildAndUnLock(esp32State, "nextPartitionBoot", jsvNewFromString( p->label));
    }
  }
  esp_partition_iterator_release(it);  
  return esp32State;
} // End of jswrap_ESP32_setBoot
