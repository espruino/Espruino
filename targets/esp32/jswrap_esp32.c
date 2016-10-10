#include "jswrap_esp32.h"

#include "esp_system.h"

/*JSON{
 "type"     : "staticmethod",
 "class"    : "ESP32",
 "name"     : "neopixelWrite",
 "generate" : "jswrap_ESP32_neopixelWrite",
 "params"   : [
   ["pin", "pin", "Pin for output signal."],
   ["arrayOfData", "JsVar", "Array of LED data."]
 ]
}*/
void jswrap_ESP32_neopixelWrite(Pin pin, JsVar *jsArrayOfData) {
  return;
}


/*JSON{
  "type"     : "staticmethod",
  "class"    : "ESP32",
  "name"     : "getState",
  "generate" : "jswrap_ESP32_getState",
  "return"   : ["JsVar", "The state of the ESP32"]
}
Returns an object that contains details about the state of the ESP32 with the following fields:

* `sdkVersion`   - Version of the SDK.
* `cpuFrequency` - CPU operating frequency in Mhz.
* `freeHeap`     - Amount of free heap in bytes.
* `maxCon`       - Maximum number of concurrent connections.
* `flashMap`     - Configured flash size&map: '512KB:256/256' .. '4MB:512/512'
* `flashKB`      - Configured flash size in KB as integer
* `flashChip`    - Type of flash chip as string with manufacturer & chip, ex: '0xEF 0x4016`

*/
JsVar *jswrap_ESP32_getState() {
  // Create a new variable and populate it with the properties of the ESP32 that we
  // wish to return.
  JsVar *esp32State = jsvNewObject();
  jsvObjectSetChildAndUnLock(esp32State, "sdkVersion",   jsvNewFromString(system_get_sdk_version()));
  //jsvObjectSetChildAndUnLock(esp32State, "cpuFrequency", jsvNewFromInteger(system_get_cpu_freq()));
  jsvObjectSetChildAndUnLock(esp32State, "freeHeap",     jsvNewFromInteger(system_get_free_heap_size()));
  return esp32State;
}
