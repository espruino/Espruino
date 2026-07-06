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

#ifdef ESPR_USE_USB_SERIAL_JTAG
#include "hal/usb_serial_jtag_ll.h"
#include "driver/usb_serial_jtag.h"
#include "rtosutil.h"
#endif

#ifdef BLUETOOTH
#include "BLE/esp32_bluetooth_utils.h"
#endif
#include "jshardwareESP32.h"
#include "jshardwareSpi.h"

#ifndef ESP_IDF_VERSION_MAJOR
#define ESP_IDF_VERSION_MAJOR 3
#endif

#ifndef ESP_ERR_SLEEP_REJECT
#define ESP_ERR_SLEEP_REJECT ESP_ERR_INVALID_STATE
#endif
#ifndef ESP_ERR_SLEEP_TOO_SHORT_SLEEP_DURATION
#define ESP_ERR_SLEEP_TOO_SHORT_SLEEP_DURATION ESP_ERR_INVALID_ARG
#endif

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

static bool esp32IsValidDeepSleepWakeupPin(Pin pin) {
  gpio_num_t gpio = pinToESP32Pin(pin);
  if (gpio < 0 || gpio >= GPIO_NUM_MAX) return false;
#if ESP_IDF_VERSION_MAJOR >= 4
  return esp_sleep_is_valid_wakeup_gpio(gpio);
#else
  switch (gpio) {
    case GPIO_NUM_0: case GPIO_NUM_2: case GPIO_NUM_4:
    case GPIO_NUM_12: case GPIO_NUM_13: case GPIO_NUM_14: case GPIO_NUM_15:
    case GPIO_NUM_25: case GPIO_NUM_26: case GPIO_NUM_27:
    case GPIO_NUM_32: case GPIO_NUM_33: case GPIO_NUM_34: case GPIO_NUM_35:
    case GPIO_NUM_36: case GPIO_NUM_37: case GPIO_NUM_39:
      return true;
    default:
      return false;
  }
#endif
}

static uint64_t esp32DeepSleepGpioMask(Pin pin) {
  return 1ULL << pinToESP32Pin(pin);
}


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
On ESP32-C3, valid wakeup pins are GPIO 0–5 (e.g. `D3` for the power button).
On other ESP32 variants, pins must be [RTC GPIOs](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/gpio.html#gpio-summary).
*/
void jswrap_ESP32_deepSleep_ext0(Pin pin, int level) {
  if (!esp32IsValidDeepSleepWakeupPin(pin)) {
    jsExceptionHere(JSET_ERROR, "Invalid pin");
    return;
  }
#ifdef CONFIG_IDF_TARGET_ESP32C3
  esp_deep_sleep_enable_gpio_wakeup(esp32DeepSleepGpioMask(pin), level);
#else
  esp_sleep_enable_ext0_wakeup(pinToESP32Pin(pin), level);
#endif
  esp_deep_sleep_start(); // This function does not return.
} // End of jswrap_ESP32_deepSleep_ext0

/*JSON{
  "type"     : "staticmethod",
  "class"    : "ESP32",
  "ifdef" : "ESP32",
  "name"     : "deepSleepExt1",
  "generate" : "jswrap_ESP32_deepSleep_ext1",
  "params"   : [
    ["pinVar", "JsVar", "Array of Pins to trigger wakeup"],
    ["mode", "int", "Trigger mode"]
  ]
}
Put device in deepsleep state until interrupted by pins in the "pinVar" array.
The trigger "mode" determines the pin state which will wake up the device.
Valid modes are:

* `0: ESP_EXT1_WAKEUP_ALL_LOW` - all nominated pins must be set LOW to trigger wakeup
* `1: ESP_EXT1_WAKEUP_ANY_HIGH` - any of nominated pins set HIGH will trigger wakeup

On ESP32-C3, valid wakeup pins are GPIO 0–5.
On other ESP32 variants, pins must be [RTC GPIOs](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/gpio.html#gpio-summary).
*/
void jswrap_ESP32_deepSleep_ext1(JsVar *pinVar, JsVarInt mode) {
  uint64_t pinSum = 0;
  if (jsvIsArray(pinVar)) {
    JsvIterator it;
    jsvIteratorNew(&it, pinVar, JSIF_DEFINED_ARRAY_ElEMENTS);
    while (jsvIteratorHasElement(&it)) {
      Pin pin = jshGetPinFromVarAndUnLock(jsvIteratorGetValue(&it));
      if (!esp32IsValidDeepSleepWakeupPin(pin)) {
        jsvIteratorFree(&it);
        jsExceptionHere(JSET_ERROR, "Invalid pin");
        return;
      }
      pinSum |= esp32DeepSleepGpioMask(pin);

      jsvIteratorNext(&it);
    }
    jsvIteratorFree(&it);
  } else {
    // We really expected an array of pins but
    // handle case of a single pin anyway
    Pin pin = jshGetPinFromVar(pinVar);
    if (!esp32IsValidDeepSleepWakeupPin(pin)) {
      jsExceptionHere(JSET_ERROR, "Invalid pin");
      return;
    }
    pinSum = esp32DeepSleepGpioMask(pin);
  }

  if ((mode < 0) || (mode > 1)) {
    jsExceptionHere(JSET_ERROR, "Invalid mode (%d)!", mode);
    return;
  }

#ifdef CONFIG_IDF_TARGET_ESP32C3
  esp_deep_sleep_enable_gpio_wakeup(pinSum, mode);
#else
  esp_sleep_enable_ext1_wakeup(pinSum, mode);
#endif
  esp_deep_sleep_start(); // This function does not return.
} // End of jswrap_ESP32_deepSleep_ext1

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
  "ifdef"    : "ESP32",
  "name"     : "lightSleep",
  "generate" : "jswrap_ESP32_lightSleep",
  "params"   : [ ["ms", "int", "Sleep time in milliseconds"] ]
}
Enter light sleep for approximately `ms` milliseconds, then resume execution
here. Unlike deep sleep, RAM and JS state are preserved.

On boards with USB Serial/JTAG console, the USB link drops during sleep;
reconnect the cable or reopen the serial port. Avoid tools that reset the
chip on connect (DTR toggle) if you need to verify RAM was preserved.

Stopping WiFi/BLE with `ESP32.enableWifi(false)` and `ESP32.enableBLE(false)`
before sleeping reduces power draw significantly.
*/
void jswrap_ESP32_lightSleep(JsVarInt ms) {
  if (ms <= 0) return;

#ifdef ESPR_USE_USB_SERIAL_JTAG
  // ConsoleTask (priority 20) outruns the Espruino task and keeps calling
  // usb_serial_jtag_read_bytes while USJ pads are gated in light sleep,
  // which can fault and reset the chip. Pause it for the sleep window.
  int consoleIdx = task_indexByName("ConsoleTask");
  if (consoleIdx >= 0) task_Suspend(consoleIdx);
  vTaskDelay(1); // let ConsoleTask finish its current read slice
  usb_serial_jtag_ll_txfifo_flush();
  usb_serial_jtag_driver_uninstall();
#endif

  esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
  esp_err_t err = esp_sleep_enable_timer_wakeup((uint64_t)ms * 1000ULL);
  if (err != ESP_OK) {
#ifdef ESPR_USE_USB_SERIAL_JTAG
    usb_serial_jtag_driver_config_t usb_cfg = {.tx_buffer_size = 128, .rx_buffer_size = 128};
    usb_serial_jtag_driver_install(&usb_cfg);
    if (consoleIdx >= 0) task_Resume(consoleIdx);
#endif
    jsExceptionHere(JSET_ERROR, "lightSleep: %d", err);
    return;
  }

  err = esp_light_sleep_start();

  /* jsiIdle advances timers using (now - jsiLastIdleTime). While we blocked
   * here the clock jumped but jsiLastIdleTime did not — the next idle pass
   * would subtract the full sleep from every setInterval/setTimeout, often
   * re-firing them in a tight loop until the watchdog resets. */
  jsiLastIdleTime = jshGetSystemTime();

#ifdef ESPR_USE_USB_SERIAL_JTAG
  usb_serial_jtag_driver_config_t usb_cfg = {.tx_buffer_size = 128, .rx_buffer_size = 128};
  usb_serial_jtag_driver_install(&usb_cfg);
  if (consoleIdx >= 0) task_Resume(consoleIdx);
#endif

  if (err != ESP_OK &&
      err != ESP_ERR_SLEEP_TOO_SHORT_SLEEP_DURATION &&
      err != ESP_ERR_SLEEP_REJECT) {
    jsExceptionHere(JSET_ERROR, "lightSleep: %d", err);
  }
}


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
  jsvObjectSetStringChild(esp32State, "sdkVersion", (esp_get_idf_version));
  jsvObjectSetIntChild(esp32State, "freeHeap", esp_get_free_heap_size());
  jsvObjectSetBoolChild(esp32State, "BLE", ESP32_Get_NVS_Status(ESP_NETWORK_BLE));
  jsvObjectSetBoolChild(esp32State, "Wifi", ESP32_Get_NVS_Status(ESP_NETWORK_WIFI));
  jsvObjectSetIntChild(esp32State, "minHeap", heap_caps_get_minimum_free_size(MALLOC_CAP_8BIT));
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

