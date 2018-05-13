/* Copyright (c) 2014 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

#include "jswrap_bluetooth.h"
#include "jsinteractive.h"
#include "jsdevices.h"
#include "jswrap_promise.h"
#include "jswrap_interactive.h"
#include "jswrap_string.h"
#include "jsnative.h"

#include "bluetooth_utils.h"
#include "bluetooth.h"

#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#ifdef NRF5X
#include "nrf5x_utils.h"
#include "nordic_common.h"
#include "nrf.h"
#include "ble_hci.h"
#include "ble_advdata.h"
#include "ble_conn_params.h"
#include "app_timer.h"
#include "ble_nus.h"
#include "app_util_platform.h"
#if NRF_SD_BLE_API_VERSION<5
#include "softdevice_handler.h"
#endif

#ifdef USE_NFC
#include "nfc_uri_msg.h"
#endif
#endif

#ifdef ESP32
#include "BLE/esp32_gap_func.h"
#include "BLE/esp32_gatts_func.h"
#include "BLE/esp32_gattc_func.h"
#define BLE_CONN_HANDLE_INVALID -1
#endif

// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------

JsVar *blePromise = 0;
JsVar *bleTaskInfo = 0;
BleTask bleTask = BLETASK_NONE;


bool bleInTask(BleTask task) {
  return bleTask==task;
}

BleTask bleGetCurrentTask() {
  return bleTask;
}

bool bleNewTask(BleTask task, JsVar *taskInfo) {
  if (bleTask) {
    jsExceptionHere(JSET_ERROR, "BLE task %d is already in progress", (int)bleTask);
    return false;
  }
/*  if (blePromise) {
    jsiConsolePrintf("Existing blePromise!\n");
    jsvTrace(blePromise,2);
  }
  if (bleTaskInfo) {
    jsiConsolePrintf("Existing bleTaskInfo!\n");
    jsvTrace(bleTaskInfo,2);
  }*/
  assert(!blePromise && !bleTaskInfo);
  blePromise = jspromise_create();
  bleTask = task;
  bleTaskInfo = jsvLockAgainSafe(taskInfo);
  return true;
}

void bleCompleteTask(BleTask task, bool ok, JsVar *data) {
  //jsiConsolePrintf(ok?"RES %d %v\n":"REJ %d %q\n", task, data);
  if (task != bleTask) {
    jsExceptionHere(JSET_INTERNALERROR, "BLE task completed that wasn't scheduled (%d/%d)", task, bleTask);
    return;
  }
  bleTask = BLETASK_NONE;
  if (blePromise) {
    if (ok) jspromise_resolve(blePromise, data);
    else jspromise_reject(blePromise, data);
    jsvUnLock(blePromise);
    blePromise = 0;
  }
  jsvUnLock(bleTaskInfo);
  bleTaskInfo = 0;
  jshHadEvent();
}

void bleCompleteTaskSuccess(BleTask task, JsVar *data) {
  bleCompleteTask(task, true, data);
}
void bleCompleteTaskSuccessAndUnLock(BleTask task, JsVar *data) {
  bleCompleteTask(task, true, data);
  jsvUnLock(data);
}
void bleCompleteTaskFail(BleTask task, JsVar *data) {
  bleCompleteTask(task, false, data);
}
void bleCompleteTaskFailAndUnLock(BleTask task, JsVar *data) {
  bleCompleteTask(task, false, data);
  jsvUnLock(data);
}
void bleSwitchTask(BleTask task) {
  bleTask = task;
}

// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
#ifdef NRF52
void bleSetActiveBluetoothGattServer(JsVar *var) {
  jsvObjectSetChild(execInfo.hiddenRoot, BLE_NAME_GATT_SERVER, var);
}

JsVar *bleGetActiveBluetoothGattServer() {
  return jsvObjectGetChild(execInfo.hiddenRoot, BLE_NAME_GATT_SERVER, 0);
}
#endif
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------

/*JSON{
  "type" : "init",
  "generate" : "jswrap_nrf_init"
}*/
void jswrap_nrf_init() {
  // Turn off sleeping if it was on before
  jsiStatus &= ~BLE_IS_SLEEPING;


  if (jsiStatus & JSIS_COMPLETELY_RESET) {
#ifdef USE_NFC
#ifdef PUCKJS
    // By default Puck.js's NFC will send you to the PuckJS website
    // address is included so Web Bluetooth can connect to the correct one
    JsVar *addr = jswrap_nrf_bluetooth_getAddress();
    JsVar *uri = jsvVarPrintf("https://puck-js.com/go?a=%v", addr);
    jsvUnLock(addr);
    jswrap_nrf_nfcURL(uri);
    jsvUnLock(uri);
#endif
#endif
  } else {
#ifdef USE_NFC
    // start NFC, if it had been set
    JsVar *flatStr = jsvObjectGetChild(execInfo.hiddenRoot, "NfcEnabled", 0);
    if (flatStr) {
      uint8_t *flatStrPtr = (uint8_t*)jsvGetFlatStringPointer(flatStr);
      if (flatStrPtr) jsble_nfc_start(flatStrPtr, jsvGetLength(flatStr));
      jsvUnLock(flatStr);
    }
#endif
  }
  // Set advertising interval back to default
  bleAdvertisingInterval = DEFAULT_ADVERTISING_INTERVAL;
  // Now set up whatever advertising we were doing before
  jswrap_nrf_reconfigure_softdevice();
}

/** Reconfigure the softdevice (on init or after restart) to have all the services/advertising we need */
void jswrap_nrf_reconfigure_softdevice() {
  // restart various
  JsVar *v,*o;
  v = jsvObjectGetChild(execInfo.root, BLE_SCAN_EVENT,0);
  if (v) jsble_set_scanning(true);
  jsvUnLock(v);
  v = jsvObjectGetChild(execInfo.root, BLE_RSSI_EVENT,0);
  if (v) jsble_set_rssi_scan(true);
  jsvUnLock(v);
  // advertising
  v = jsvObjectGetChild(execInfo.hiddenRoot, BLE_NAME_ADVERTISE_DATA, 0);
  o = jsvObjectGetChild(execInfo.hiddenRoot, BLE_NAME_ADVERTISE_OPTIONS, 0);
  if (v || o) jswrap_nrf_bluetooth_setAdvertising(v, o);
  jsvUnLock2(v,o);
  // services
  v = jsvObjectGetChild(execInfo.hiddenRoot, BLE_NAME_SERVICE_DATA, 0);
  if (v) jsble_set_services(v);
  jsvUnLock(v);
  // If we had scan response data set, update it
  JsVar *scanData = jsvObjectGetChild(execInfo.hiddenRoot, BLE_NAME_SCAN_RESPONSE_DATA, 0);
  if (scanData) jswrap_nrf_bluetooth_setScanResponse(scanData);
  jsvUnLock(scanData);
}

/*JSON{
  "type" : "idle",
  "generate" : "jswrap_nrf_idle"
}*/
bool jswrap_nrf_idle() {
  return false;
}

/*JSON{
  "type" : "kill",
  "generate" : "jswrap_nrf_kill"
}*/
void jswrap_nrf_kill() {
#ifdef USE_NFC
  // stop NFC emulation
  jsble_nfc_stop(); // not a problem to call this if NFC isn't started
#endif
  // stop any BLE tasks
  bleTask = BLETASK_NONE;
  if (blePromise) jsvUnLock(blePromise);
  blePromise = 0;
  if (bleTaskInfo) jsvUnLock(bleTaskInfo);
  bleTaskInfo = 0;
  // if we were scanning, make sure we stop
  jsble_set_scanning(false);
  jsble_set_rssi_scan(false);

#if CENTRAL_LINK_COUNT>0
  // if we were connected to something, disconnect
  if (jsble_has_central_connection()) {
    jsble_disconnect(m_central_conn_handle);
  }
#endif
}

void jswrap_nrf_dumpBluetoothInitialisation(vcbprintf_callback user_callback, void *user_data) {


  JsVar *v,*o;
  v = jsvObjectGetChild(execInfo.root, BLE_SCAN_EVENT,0);
  if (v) {
    user_callback("NRF.setScan(", user_data);
    jsiDumpJSON(user_callback, user_data, v, 0);
    user_callback(");\n", user_data);
  }
  jsvUnLock(v);
  v = jsvObjectGetChild(execInfo.root, BLE_RSSI_EVENT,0);
  if (v) {
    user_callback("NRF.setRSSIHandler(", user_data);
    jsiDumpJSON(user_callback, user_data, v, 0);
    user_callback(");\n", user_data);
  }
  jsvUnLock(v);
  // advertising
  v = jsvObjectGetChild(execInfo.hiddenRoot, BLE_NAME_ADVERTISE_DATA, 0);
  o = jsvObjectGetChild(execInfo.hiddenRoot, BLE_NAME_ADVERTISE_OPTIONS, 0);
  if (v || o)
    cbprintf(user_callback, user_data, "NRF.setAdvertising(%j, %j);\n",v,o);
  jsvUnLock2(v,o);
  // services
  v = jsvObjectGetChild(execInfo.hiddenRoot, BLE_NAME_SERVICE_DATA, 0);
  o = jsvObjectGetChild(execInfo.hiddenRoot, BLE_NAME_SERVICE_OPTIONS, 0);
  if (v || o)
    cbprintf(user_callback, user_data, "NRF.setServices(%j, %j);\n",v,o);
  jsvUnLock2(v,o);
}

// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------

/*JSON{
  "type" : "event",
  "class" : "NRF",
  "name" : "connect",
  "params" : [
    ["addr","JsVar","The address of the device that has connected"]
  ]
}
Called when a host device connects to Espruino. The first argument contains the address.
 */
/*JSON{
  "type" : "event",
  "class" : "NRF",
  "name" : "disconnect",
  "params" : [
    ["reason","int","The reason code reported back by the BLE stack - see Nordic's `ble_hci.h` file for more information"]
  ]
}
Called when a host device disconnects from Espruino.
 */


/*JSON{
  "type" : "event",
  "class" : "NRF",
  "name" : "servicesDiscover",
  "ifdef" : "NRF52,ESP32"
}
Called with discovered services when discovery is finished
 */
/*JSON{
  "type" : "event",
  "class" : "NRF",
  "name" : "characteristicsDiscover",
  "ifdef" : "NRF52,ESP32"
}
Called with discovered characteristics when discovery is finished
 */

/*JSON{
  "type" : "event",
  "class" : "NRF",
  "name" : "NFCon",
  "ifdef" : "NRF52"
}
Called when an NFC field is detected
 */
/*JSON{
  "type" : "event",
  "class" : "NRF",
  "name" : "NFCoff",
  "ifdef" : "NRF52"
}
Called when an NFC field is no longer detected
 */
/*JSON{
  "type" : "event",
  "class" : "NRF",
  "name" : "NFCrx",
  "params" : [
    ["arr","JsVar","An ArrayBuffer containign the received data"]
  ],
  "ifdef" : "NRF52"
}
When NFC is started with `NRF.nfcStart`, this is fired
when NFC data is received. It doesn't get called if
NFC is started with `NRF.nfcURL` or `NRF.nfcRaw`
 */
/*JSON{
  "type" : "event",
  "class" : "BluetoothDevice",
  "name" : "gattserverdisconnected",
  "params" : [
    ["reason","int","The reason code reported back by the BLE stack - see Nordic's `ble_hci.h` file for more information"]
  ],
  "ifdef" : "NRF52"
}
Called when the device gets disconnected.

To connect and then print `Disconnected` when the device is
disconnected, just do the following:

```
var gatt;
NRF.connect("aa:bb:cc:dd:ee:ff").then(function(gatt) {
  gatt.device.on('gattserverdisconnected', function(reason) {
    console.log("Disconnected ",reason);
  });
});
```
 */
/*JSON{
  "type" : "event",
  "class" : "BluetoothRemoteGATTCharacteristic",
  "name" : "characteristicvaluechanged",
  "ifdef" : "NRF52"
}
Called when a characteristic's value changes, *after* `BluetoothRemoteGATTCharacteristic.startNotifications` has been called.
See that for an example.

The first argument is of the form `{target : BluetoothRemoteGATTCharacteristic}`

`BluetoothRemoteGATTCharacteristic.value` will then contain the new value.
 */

/*JSON{
    "type": "class",
    "class" : "NRF"
}
The NRF class is for controlling functionality of the Nordic nRF51/nRF52 chips. Currently these only used in [Puck.js](http://puck-js.com) and the [BBC micro:bit](/MicroBit).

The main part of this is control of Bluetooth Low Energy - both searching for devices, and changing advertising data.
*/
/*JSON{
  "type" : "object",
  "name" : "Bluetooth",
  "instanceof" : "Serial",
  "ifdef" : "BLUETOOTH"
}
The Bluetooth Serial port - used when data is sent or received over Bluetooth Smart on nRF51/nRF52 chips.
 */

/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "disconnect",
    "generate" : "jswrap_nrf_bluetooth_disconnect"
}
If a device is connected to Espruino, disconnect from it.
*/
void jswrap_nrf_bluetooth_disconnect() {
  uint32_t err_code;
  if (jsble_has_simple_connection()) {
    err_code = jsble_disconnect(m_conn_handle);
    jsble_check_error(err_code);
  }
}

/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "sleep",
    "generate" : "jswrap_nrf_bluetooth_sleep"
}
Disable Bluetooth advertising and disconnect from any device that
connected to Puck.js as a peripheral (this won't affect any devices
that Puck.js initiated connections to).

This makes Puck.js undiscoverable, so it can't be connected to.

Use `NRF.wake()` to wake up and make Puck.js connectable again.
*/
void jswrap_nrf_bluetooth_sleep() {
  // set as sleeping
  bleStatus |= BLE_IS_SLEEPING;
  // stop advertising
  jsble_advertising_stop();
  // If connected, disconnect.
  // when we disconnect, we'll see BLE_IS_SLEEPING and won't advertise
  jswrap_nrf_bluetooth_disconnect();
}

/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "wake",
    "generate" : "jswrap_nrf_bluetooth_wake"
}
Enable Bluetooth advertising (this is enabled by default), which
allows other devices to discover and connect to Puck.js.

Use `NRF.sleep()` to disable advertising.
*/
void jswrap_nrf_bluetooth_wake() {
  bleStatus &= ~BLE_IS_SLEEPING;
  jsble_advertising_start();
}

/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "restart",
    "generate" : "jswrap_nrf_bluetooth_restart"
}
Restart the Bluetooth softdevice (if there is currently a BLE connection,
it will queue a restart to be done when the connection closes).

You shouldn't need to call this function in normal usage. However, Nordic's
BLE softdevice has some settings that cannot be reset. For example there
are only a certain number of unique UUIDs. Once these are all used the
only option is to restart the softdevice to clear them all out.
*/
void jswrap_nrf_bluetooth_restart() {
  if (jsble_has_connection()) {
    jsiConsolePrintf("BLE Connected, queueing BLE restart for later\n");
    bleStatus |= BLE_NEEDS_SOFTDEVICE_RESTART;
    return;
  } else {
    // Not connected, so we can restart now
    jsble_restart_softdevice();
    return;
  }
}

/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "getAddress",
    "generate" : "jswrap_nrf_bluetooth_getAddress",
    "return" : ["JsVar", "MAC address - a string of the form 'aa:bb:cc:dd:ee:ff'" ]
}
Get this device's Bluetooth MAC address.

For Puck.js, the last 5 characters of this (eg. `ee:ff`)
are used in the device's advertised Bluetooth name.
*/
JsVar *jswrap_nrf_bluetooth_getAddress() {
#ifdef NRF5X
  uint32_t addr0 =  NRF_FICR->DEVICEADDR[0];
  uint32_t addr1 =  NRF_FICR->DEVICEADDR[1];
#else
  uint32_t addr0 = 0xDEADDEAD;
  uint32_t addr1 = 0xDEAD;
#endif
  return jsvVarPrintf("%02x:%02x:%02x:%02x:%02x:%02x",
      ((addr1>>8 )&0xFF)|0xC0,
      ((addr1    )&0xFF),
      ((addr0>>24)&0xFF),
      ((addr0>>16)&0xFF),
      ((addr0>>8 )&0xFF),
      ((addr0    )&0xFF));
}

/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "getBattery",
    "generate" : "jswrap_nrf_bluetooth_getBattery",
    "return" : ["float", "Battery level in volts" ]
}
Get the battery level in volts (the voltage that the NRF chip is running off of).

This is the battery level of the device itself - it has nothing to with any
device that might be connected.
*/
JsVarFloat jswrap_nrf_bluetooth_getBattery() {
  return jshReadVRef();
}

/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "setAdvertising",
    "generate" : "jswrap_nrf_bluetooth_setAdvertising",
    "params" : [
      ["data","JsVar","The data to advertise as an object - see below for more info"],
      ["options","JsVar","An optional object of options"]
    ]
}
Change the data that Espruino advertises.

Data can be of the form `{ UUID : data_as_byte_array }`. The UUID should be
a [Bluetooth Service ID](https://developer.bluetooth.org/gatt/services/Pages/ServicesHome.aspx).

For example to return battery level at 95%, do:

```
NRF.setAdvertising({
  0x180F : [95]
});
```

Or you could report the current temperature:

```
setInterval(function() {
  NRF.setAdvertising({
    0x1809 : [Math.round(E.getTemperature())]
  });
}, 30000);
```

You can also supply the raw advertising data in an array. For example
to advertise as an Eddystone beacon:

```
NRF.setAdvertising([0x03,  // Length of Service List
  0x03,  // Param: Service List
  0xAA, 0xFE,  // Eddystone ID
  0x13,  // Length of Service Data
  0x16,  // Service Data
  0xAA, 0xFE, // Eddystone ID
  0x10,  // Frame type: URL
  0xF8, // Power
  0x03, // https://
  'g','o','o','.','g','l','/','B','3','J','0','O','c'],
    {interval:100});
```

(However for Eddystone we'd advise that you use the [Espruino Eddystone library](/Puck.js+Eddystone))

**Note:** When specifying data as an array, certain advertising options such as
`discoverable` and `showName` won't have any effect.

**Note:** The size of Bluetooth LE advertising packets is limited to 31 bytes. If
you want to advertise more data, consider using an array for `data` (See below), or
`NRF.setScanResponse`.

You can even specify an array of arrays or objects, in which case each advertising packet
will be used in turn - for instance to make your device advertise
both Eddystone and iBeacon:

```
NRF.setAdvertising([
  {0x180F : [Puck.getBatteryPercentage()]}, // normal advertising, with battery %
  require("ble_ibeacon").get(...), // iBeacon
  require("ble_eddystone").get(...), // eddystone
],{interval:500});
```

`options` is an object, which can contain:

```
{
  name: "Hello" // The name of the device
  showName: true/false // include full name, or nothing
  discoverable: true/false // general discoverable, or limited - default is limited
  connectable: true/false // whether device is connectable - default is true
  interval: 600 // Advertising interval in msec, between 20 and 10000
  manufacturer: 0x0590 // IF sending manufacturer data, this is the manufacturer ID
  manufacturerData: [...] // IF sending manufacturer data, this is an array of data
}
```

So for instance to set the name of Puck.js without advertising any
other data you can just use the command:

```
NRF.setAdvertising({},{name:"Hello"});
```

You can also specify 'manufacturer data', which is another form of advertising data.
We've registered the Manufacturer ID 0x0590 (as Pur3 Ltd) for use with *Official
Espruino devices* - use it to advertise whatever data you'd like, but we'd recommend
using JSON.
*/
void jswrap_nrf_bluetooth_setAdvertising(JsVar *data, JsVar *options) {
  uint32_t err_code;
  bool bleChanged = false;
  bool isAdvertising = bleStatus & BLE_IS_ADVERTISING;

  if (jsvIsObject(options)) {
    JsVar *v;

    v = jsvObjectGetChild(options, "interval", 0);
    if (v) {
      uint16_t new_advertising_interval = MSEC_TO_UNITS(jsvGetIntegerAndUnLock(v), UNIT_0_625_MS);
      if (new_advertising_interval<0x0020) new_advertising_interval=0x0020;
      if (new_advertising_interval>0x4000) new_advertising_interval=0x4000;
      if (new_advertising_interval != bleAdvertisingInterval) {
        bleAdvertisingInterval = new_advertising_interval;
        bleChanged = true;
      }
    }

    v = jsvObjectGetChild(options, "connectable", 0);
    if (v) {
      if (jsvGetBoolAndUnLock(v)) bleStatus &= ~BLE_IS_NOT_CONNECTABLE;
      else bleStatus |= BLE_IS_NOT_CONNECTABLE;
      bleChanged = true;
    }

    v = jsvObjectGetChild(options, "name", 0);
    if (v) {
      JSV_GET_AS_CHAR_ARRAY(namePtr, nameLen, v);
      if (namePtr) {
#ifdef NRF5X
        ble_gap_conn_sec_mode_t sec_mode;
        BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);
        err_code = sd_ble_gap_device_name_set(&sec_mode,
                                              (const uint8_t *)namePtr,
                                              nameLen);
//#else
//        err_code = 0xDEAD;
//        jsiConsolePrintf("FIXME\n");
#endif
#ifdef ESP32
		bluetooth_setDeviceName(v);
#endif
        jsble_check_error(err_code);
        bleChanged = true;
      }
      jsvUnLock(v);
    }
  } else if (!jsvIsUndefined(options)) {
    jsExceptionHere(JSET_TYPEERROR, "Expecting 'options' to be object or undefined, got %t", options);
    return;
  }

  JsVar *advArray = 0;
  JsVar *initialArray = 0;

  if (jsvIsObject(data) || jsvIsUndefined(data)) {
    // if it's an object, work out what the advertising data for it is
    advArray = jswrap_nrf_bluetooth_getAdvertisingData(data, options);
    // if undefined, make sure we *save* undefined
    if (jsvIsUndefined(data)) {
      initialArray = advArray;
      advArray = 0;
    }
  } else if (jsvIsArray(data)) {
    advArray = jsvLockAgain(data);
    // Check if it's nested arrays - if so we alternate between advertising types
    bleStatus &= ~(BLE_IS_ADVERTISING_MULTIPLE|BLE_ADVERTISING_MULTIPLE_MASK);
    // check for nested, and if so then preconvert the objects into arrays
    bool isNested = false;
    int elements = 0;
    JsvObjectIterator it;
    jsvObjectIteratorNew(&it, advArray);
    while (jsvObjectIteratorHasValue(&it)) {
      JsVar *v = jsvObjectIteratorGetValue(&it);
      if (jsvIsObject(v) || jsvIsUndefined(v)) {
        JsVar *newv = jswrap_nrf_bluetooth_getAdvertisingData(v, options);
        jsvObjectIteratorSetValue(&it, newv);
        jsvUnLock(newv);
        isNested = true;
      } else if (jsvIsArray(v) || jsvIsArrayBuffer(v)) {
        isNested = true;
      }
      elements++;
      jsvUnLock(v);
      jsvObjectIteratorNext(&it);
    }
    jsvObjectIteratorFree(&it);
    // it's nested - set multiple advertising mode
    if (isNested) {
      // nested - enable multiple advertising - start at index 0
      if (elements>1)
        bleStatus |= BLE_IS_ADVERTISING_MULTIPLE;
      // start with the first element
      initialArray = jsvGetArrayItem(advArray, 0);
    }
  } else if (jsvIsArrayBuffer(data)) {
    // it's just data - no multiple advertising
    advArray = jsvLockAgain(data);
    bleStatus &= ~(BLE_IS_ADVERTISING_MULTIPLE|BLE_ADVERTISING_MULTIPLE_MASK);
  }
  if (!initialArray) initialArray = jsvLockAgain(advArray);
  // failure check
  if (!(jsvIsArray(initialArray) || jsvIsArrayBuffer(initialArray))) {
    jsExceptionHere(JSET_TYPEERROR, "Expecting object, array or undefined, got %t", data);
    jsvUnLock2(advArray, initialArray);
    return;
  }
  JSV_GET_AS_CHAR_ARRAY(dPtr, dLen, initialArray);
  if (!dPtr) {
    jsvUnLock2(advArray, initialArray);
    jsExceptionHere(JSET_TYPEERROR, "Unable to convert data argument to an array");
    return;
  }
  // Save the current service data
  jsvObjectSetOrRemoveChild(execInfo.hiddenRoot, BLE_NAME_ADVERTISE_DATA, advArray);
  jsvObjectSetOrRemoveChild(execInfo.hiddenRoot, BLE_NAME_ADVERTISE_OPTIONS, options);
  jsvUnLock(advArray);
  // now actually update advertising
  if (bleChanged && isAdvertising)
    jsble_advertising_stop();
#ifdef NRF5X
  err_code = sd_ble_gap_adv_data_set((uint8_t *)dPtr, dLen, NULL, 0);
//#else
//  err_code = 0xDEAD;
//  jsiConsolePrintf("FIXME\n");
#endif
#ifdef ESP32
  err_code = bluetooth_gap_setAdvertizing(advArray);
#endif
  jsvUnLock(initialArray);
  jsble_check_error(err_code);
  if (bleChanged && isAdvertising)
    jsble_advertising_start();
}

/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "getAdvertisingData",
    "generate" : "jswrap_nrf_bluetooth_getAdvertisingData",
    "params" : [
      ["data","JsVar","The data to advertise as an object"],
      ["options","JsVar","An optional object of options"]
    ],
    "return" : ["JsVar", "An array containing the advertising data" ]
}
This is just like `NRF.setAdvertising`, except instead of advertising
the data, it returns the packet that would be advertised as an array.
*/
JsVar *jswrap_nrf_bluetooth_getAdvertisingData(JsVar *data, JsVar *options) {
  uint32_t err_code;
#ifdef ESP32
  JsVar *r;
  r = bluetooth_gap_getAdvertisingData(data,options);
  return r;
#endif
#ifdef NRF5X
  ble_advdata_t advdata;
  jsble_setup_advdata(&advdata);
#endif

  if (jsvIsObject(options)) {
    JsVar *v;
#ifdef NRF5X
    v = jsvObjectGetChild(options, "showName", 0);
    if (v) advdata.name_type = jsvGetBoolAndUnLock(v) ?
        BLE_ADVDATA_FULL_NAME :
        BLE_ADVDATA_NO_NAME;

    v = jsvObjectGetChild(options, "discoverable", 0);
    if (v) advdata.flags = jsvGetBoolAndUnLock(v) ?
        BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE :
        BLE_GAP_ADV_FLAGS_LE_ONLY_LIMITED_DISC_MODE;

    v = jsvObjectGetChild(options, "manufacturerData", 0);
    if (v) {
      JSV_GET_AS_CHAR_ARRAY(dPtr, dLen, v);
      if (dPtr && dLen) {
        advdata.p_manuf_specific_data = (ble_advdata_manuf_data_t*)alloca(sizeof(ble_advdata_manuf_data_t));
        advdata.p_manuf_specific_data->company_identifier = 0xFFFF; // pre-fill with test manufacturer data
        advdata.p_manuf_specific_data->data.size = dLen;
        advdata.p_manuf_specific_data->data.p_data = (uint8_t*)dPtr;
      }
      jsvUnLock(v);
    }
    v = jsvObjectGetChild(options, "manufacturer", 0);
    if (v) {
      if (advdata.p_manuf_specific_data)
        advdata.p_manuf_specific_data->company_identifier = jsvGetInteger(v);
      else
        jsExceptionHere(JSET_TYPEERROR, "'manufacturer' specified without 'manufacturerdata'");
      jsvUnLock(v);
    }
#endif
  } else if (!jsvIsUndefined(options)) {
    jsExceptionHere(JSET_TYPEERROR, "Expecting 'options' to be object or undefined, got %t", options);
    return 0;
  }

  if (jsvIsArray(data) || jsvIsArrayBuffer(data)) {
    return jsvLockAgain(data);
  } else if (jsvIsObject(data)) {
#ifdef NRF5X
    ble_advdata_service_data_t *service_data = (ble_advdata_service_data_t*)alloca(jsvGetChildren(data)*sizeof(ble_advdata_service_data_t));
#endif
    int n = 0;
    JsvObjectIterator it;
    jsvObjectIteratorNew(&it, data);
    while (jsvObjectIteratorHasValue(&it)) {
      JsVar *v = jsvObjectIteratorGetValue(&it);
      JSV_GET_AS_CHAR_ARRAY(dPtr, dLen, v);
      jsvUnLock(v);
#ifdef NRF5X
      service_data[n].service_uuid = jsvGetIntegerAndUnLock(jsvObjectIteratorGetKey(&it));
      service_data[n].data.size    = dLen;
      service_data[n].data.p_data  = (uint8_t*)dPtr;
#endif
      jsvObjectIteratorNext(&it);
      n++;
    }
    jsvObjectIteratorFree(&it);
#ifdef NRF5X
    advdata.service_data_count   = n;
    advdata.p_service_data_array = service_data;
#endif
  } else if (!jsvIsUndefined(data)) {
    jsExceptionHere(JSET_TYPEERROR, "Expecting object, array or undefined, got %t", data);
    return 0;
  }

  uint16_t  len_advdata = BLE_GAP_ADV_MAX_SIZE;
  uint8_t   encoded_advdata[BLE_GAP_ADV_MAX_SIZE];

#ifdef NRF5X
#if NRF_SD_BLE_API_VERSION<5
  err_code = adv_data_encode(&advdata, encoded_advdata, &len_advdata);
#else
  err_code = ble_advdata_encode(&advdata, encoded_advdata, &len_advdata);
#endif
#else
  err_code = 0xDEAD;
  jsiConsolePrintf("FIXME\n");
#endif
  if (jsble_check_error(err_code)) return 0;
  return jsvNewArrayBufferWithData(len_advdata, encoded_advdata);
}

/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "setScanResponse",
    "generate" : "jswrap_nrf_bluetooth_setScanResponse",
    "params" : [
      ["data","JsVar","The data to for the scan response"]
    ]
}

The raw scan response data should be supplied as an array. For example to return "Sample" for the device name:

```
NRF.setScanResponse([0x07,  // Length of Data
  0x09,  // Param: Complete Local Name
  'S', 'a', 'm', 'p', 'l', 'e']);
```

**Note:** `NRF.setServices(..., {advertise:[ ... ]})` writes advertised
services into the scan response - so you can't use both `advertise`
and `NRF.setServices` or one will overwrite the other.
*/
void jswrap_nrf_bluetooth_setScanResponse(JsVar *data) {
  uint32_t err_code;

  jsvObjectSetOrRemoveChild(execInfo.hiddenRoot, BLE_NAME_SCAN_RESPONSE_DATA, data);

  if (jsvIsArray(data) || jsvIsArrayBuffer(data)) {
    JSV_GET_AS_CHAR_ARRAY(dPtr, dLen, data);
    if (!dPtr) {
      jsExceptionHere(JSET_TYPEERROR, "Unable to convert data argument to an array");
      return;
    }
#ifdef NRF5X
    err_code = sd_ble_gap_adv_data_set(NULL, 0, (uint8_t *)dPtr, dLen);
#else
    err_code = 0xDEAD;
    jsiConsolePrintf("FIXME\n");
#endif
    jsble_check_error(err_code);
  } else if (!jsvIsUndefined(data)) {
    jsExceptionHere(JSET_TYPEERROR, "Expecting array-like object or undefined, got %t", data);
  }
}

/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "setServices",
    "generate" : "jswrap_nrf_bluetooth_setServices",
    "params" : [
      ["data","JsVar","The service (and characteristics) to advertise"],
      ["options","JsVar","Optional object containing options"]
    ]
}

Change the services and characteristics Espruino advertises.

If you want to **change** the value of a characteristic, you need
to use `NRF.updateServices()` instead

To expose some information on Characteristic `ABCD` on service `BCDE` you could do:

```
NRF.setServices({
  0xBCDE : {
    0xABCD : {
      value : "Hello",
      readable : true
    }
  }
});
```

Or to allow the 3 LEDs to be controlled by writing numbers 0 to 7 to a
characteristic, you can do the following. `evt.data` is an array of
bytes.

```
NRF.setServices({
  0xBCDE : {
    0xABCD : {
      writable : true,
      onWrite : function(evt) {
        digitalWrite([LED3,LED2,LED1], evt.data[0]);
      }
    }
  }
});
```

You can supply many different options:

```
NRF.setServices({
  0xBCDE : {
    0xABCD : {
      value : "Hello", // optional
      maxLen : 5, // optional (otherwise is length of initial value)
      broadcast : false, // optional, default is false
      readable : true,   // optional, default is false
      writable : true,   // optional, default is false
      notify : true,   // optional, default is false
      indicate : true,   // optional, default is false
      description: "My Characteristic",  // optional, default is null
      onWrite : function(evt) { // optional
        console.log("Got ", evt.data);
      }
    }
    // more characteristics allowed
  }
  // more services allowed
});
```

**Note:** UUIDs can be integers between `0` and `0xFFFF`, strings of
the form `"ABCD"`, or strings of the form `"ABCDABCD-ABCD-ABCD-ABCD-ABCDABCDABCD"`

`options` can be of the form:

```
NRF.setServices(undefined, {
  hid : new Uint8Array(...), // optional, default is undefined. Enable BLE HID support
  uart : true, // optional, default is true. Enable BLE UART support
  advertise: [ '180D' ] // optional, list of service UUIDs to advertise
});
```

To enable BLE HID, you must set `hid` to an array which is the BLE report
descriptor. The easiest way to do this is to use the `ble_hid_controls`
or `ble_hid_keyboard` modules.

**Note:** Just creating a service doesn't mean that the service will
be advertised. It will only be available after a device connects. To
advertise, specify the UUIDs you wish to advertise in the `advertise`
field of the second `options` argument. For example this will create
and advertise a heart rate service:

```
NRF.setServices({
  0x180D: { // heart_rate
    0x2A37: { // heart_rate_measurement
      notify: true,
      value : [0x06, heartrate],
    }
  }
}, { advertise: [ '180D' ] });
```

You may specify 128 bit UUIDs to advertise, however you may get a `DATA_SIZE`
exception because there is insufficient space in the Bluetooth LE advertising
packet for the 128 bit UART UUID as well as the UUID you specified. In this
case you can add `uart:false` after the `advertise` element to disable the
UART, however you then be unable to connect to Puck.js's console via Bluetooth.

If you absolutely require two or more 128 bit UUIDs then you will have to
specify your own raw advertising data packets with `NRF.setAdvertising`

*/
void jswrap_nrf_bluetooth_setServices(JsVar *data, JsVar *options) {
  if (!(jsvIsObject(data) || jsvIsUndefined(data))) {
    jsExceptionHere(JSET_TYPEERROR, "Expecting object or undefined, got %t", data);
    return;
  }

#if BLE_HIDS_ENABLED
  JsVar *use_hid = 0;
#endif
  bool use_uart = true;
  JsVar *advertise = 0;

  jsvConfigObject configs[] = {
#if BLE_HIDS_ENABLED
      {"hid", JSV_ARRAY, &use_hid},
#endif
      {"uart", JSV_BOOLEAN, &use_uart},
      {"advertise",  JSV_ARRAY, &advertise},
  };
  if (!jsvReadConfigObject(options, configs, sizeof(configs) / sizeof(jsvConfigObject))) {
    return;
  }

#if BLE_HIDS_ENABLED
  // Handle turning on/off of HID
  if (jsvIsIterable(use_hid)) {
    jsvObjectSetChild(execInfo.hiddenRoot, BLE_NAME_HID_DATA, use_hid);
    bleStatus |= BLE_NEEDS_SOFTDEVICE_RESTART;
  } else if (!use_hid) {
    jsvObjectRemoveChild(execInfo.hiddenRoot, BLE_NAME_HID_DATA);
    if (bleStatus & BLE_HID_INITED)
      bleStatus |= BLE_NEEDS_SOFTDEVICE_RESTART;
  } else {
    jsExceptionHere(JSET_TYPEERROR, "'hid' must be undefined, or an array");
  }
  jsvUnLock(use_hid);
#endif
  if (use_uart) {
    if (!(bleStatus & BLE_NUS_INITED))
      bleStatus |= BLE_NEEDS_SOFTDEVICE_RESTART;
    jsvObjectRemoveChild(execInfo.hiddenRoot, BLE_NAME_NUS);
  } else {
    if (bleStatus & BLE_NUS_INITED)
      bleStatus |= BLE_NEEDS_SOFTDEVICE_RESTART;
    jsvObjectSetChildAndUnLock(execInfo.hiddenRoot, BLE_NAME_NUS, jsvNewFromBool(false));
  }

  // Save the current service data and options
  jsvObjectSetOrRemoveChild(execInfo.hiddenRoot, BLE_NAME_SERVICE_DATA, data);
  jsvObjectSetOrRemoveChild(execInfo.hiddenRoot, BLE_NAME_SERVICE_OPTIONS, options);
  // Service UUIDs to advertise
  if (advertise) bleStatus|=BLE_NEEDS_SOFTDEVICE_RESTART;
  jsvObjectSetOrRemoveChild(execInfo.hiddenRoot, BLE_NAME_SERVICE_ADVERTISE, advertise);
  jsvUnLock(advertise);

  // work out whether to apply changes
  if (bleStatus & (BLE_SERVICES_WERE_SET|BLE_NEEDS_SOFTDEVICE_RESTART)) {
    jswrap_nrf_bluetooth_restart();
  } else {
    /* otherwise, we can set the services now, since we're only adding
     * and not changing anything we don't need a restart. */
    jsble_set_services(data);
  }
}

/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "updateServices",
    "generate" : "jswrap_nrf_bluetooth_updateServices",
    "params" : [
      ["data","JsVar","The service (and characteristics) to update"]
    ]
}

Update values for the services and characteristics Espruino advertises.
Only services and characteristics previously declared using `setServices` are affected.

To update the '0xABCD' characteristic in the '0xBCDE' service:

```
NRF.updateServices({
  0xBCDE : {
    0xABCD : {
      value : "World"
    }
  }
});
```

You can also use 128 bit UUIDs, for example `"b7920001-3c1b-4b40-869f-3c0db9be80c6"`.

To define a service and characteristic and then notify connected clients of a
change to it when a button is pressed:

```
NRF.setServices({
  0xBCDE : {
    0xABCD : {
      value : "Hello",
      maxLen : 20,
      notify: true
    }
  }
});
setWatch(function() {
  NRF.updateServices({
    0xBCDE : {
      0xABCD : {
        value : "World!",
        notify: true
      }
    }
  });
}, BTN, { repeat:true, edge:"rising", debounce: 50 });
```

This only works if the characteristic was created with `notify: true` using `setServices`,
otherwise the characteristic will be updated but no notification will be sent.

Also note that `maxLen` was specified. If it wasn't then the maximum length of
the characteristic would have been 5 - the length of `"Hello"`.

To indicate (i.e. notify with ACK) connected clients of a change to the '0xABCD' characteristic in the '0xBCDE' service:

```
NRF.updateServices({
  0xBCDE : {
    0xABCD : {
      value : "World",
      indicate: true
    }
  }
});
```

This only works if the characteristic was created with `indicate: true` using `setServices`,
otherwise the characteristic will be updated but no notification will be sent.

**Note:** See `NRF.setServices` for more information
*/
void jswrap_nrf_bluetooth_updateServices(JsVar *data) {
  uint32_t err_code;
  bool ok = true;

  if (jsvIsObject(data)) {
    JsvObjectIterator it;
    jsvObjectIteratorNew(&it, data);
    while (jsvObjectIteratorHasValue(&it)) {
      ble_uuid_t ble_uuid;
      memset(&ble_uuid, 0, sizeof(ble_uuid));

      const char *errorStr;
      if ((errorStr = bleVarToUUIDAndUnLock(&ble_uuid,
          jsvObjectIteratorGetKey(&it)))) {
        jsExceptionHere(JSET_ERROR, "Invalid Service UUID: %s", errorStr);
        break;
      }

      JsVar *serviceVar = jsvObjectIteratorGetValue(&it);
      JsvObjectIterator serviceit;
      jsvObjectIteratorNew(&serviceit, serviceVar);

      while (ok && jsvObjectIteratorHasValue(&serviceit)) {
        ble_uuid_t char_uuid;
        if ((errorStr = bleVarToUUIDAndUnLock(&char_uuid,
            jsvObjectIteratorGetKey(&serviceit)))) {
          jsExceptionHere(JSET_ERROR, "Invalid Characteristic UUID: %s",
              errorStr);
          break;
        }

        uint16_t char_handle = bleGetGATTHandle(char_uuid);
        if (char_handle != BLE_GATT_HANDLE_INVALID) {
          JsVar *charVar = jsvObjectIteratorGetValue(&serviceit);
          JsVar *charValue = jsvObjectGetChild(charVar, "value", 0);

          bool notification_requested = jsvGetBoolAndUnLock(jsvObjectGetChild(charVar, "notify", 0));
          bool indication_requested = jsvGetBoolAndUnLock(jsvObjectGetChild(charVar, "indicate", 0));

          if (charValue) {
            JSV_GET_AS_CHAR_ARRAY(vPtr, vLen, charValue);
            if (vPtr && vLen) {
#ifdef NRF5X
              ble_gatts_hvx_params_t hvx_params;
              ble_gatts_value_t gatts_value;

              // Update the value for subsequent reads even if no client is currently connected
              memset(&gatts_value, 0, sizeof(gatts_value));
              gatts_value.len = vLen;
              gatts_value.offset = 0;
              gatts_value.p_value = (uint8_t*)vPtr;
              err_code = sd_ble_gatts_value_set(m_conn_handle, char_handle, &gatts_value);
              if (jsble_check_error(err_code)) {
                ok = false;
              } if ((notification_requested || indication_requested) && jsble_has_simple_connection()) {
                // Notify/indicate connected clients if necessary
                memset(&hvx_params, 0, sizeof(hvx_params));
                uint16_t len = (uint16_t)vLen;
                hvx_params.handle = char_handle;
                hvx_params.type = indication_requested ? BLE_GATT_HVX_INDICATION : BLE_GATT_HVX_NOTIFICATION;
                hvx_params.offset = 0;
                hvx_params.p_len = &len;
                hvx_params.p_data = (uint8_t*)vPtr;

                err_code = sd_ble_gatts_hvx(m_conn_handle, &hvx_params);
                if ((err_code != NRF_SUCCESS)
                  && (err_code != NRF_ERROR_INVALID_STATE)
#if NRF_SD_BLE_API_VERSION<5
                  && (err_code != BLE_ERROR_NO_TX_PACKETS)
#else
                  && (err_code != NRF_ERROR_RESOURCES)
#endif
                  && (err_code != BLE_ERROR_GATTS_SYS_ATTR_MISSING)) {
                  if (jsble_check_error(err_code))
                    ok = false;
                }
              }
#endif
            }
          }
          jsvUnLock(charValue);
          jsvUnLock(charVar);
        } else {
          JsVar *str = bleUUIDToStr(char_uuid);
          jsExceptionHere(JSET_ERROR, "Unable to find service with UUID %v", str);
          jsvUnLock(str);
        }

        jsvObjectIteratorNext(&serviceit);
      }
      jsvObjectIteratorFree(&serviceit);
      jsvUnLock(serviceVar);

      jsvObjectIteratorNext(&it);
    }
    jsvObjectIteratorFree(&it);

  } else if (!jsvIsUndefined(data)) {
    jsExceptionHere(JSET_TYPEERROR, "Expecting object or undefined, got %t",
        data);
  }
}

/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "setScan",
    "generate" : "jswrap_nrf_bluetooth_setScan",
    "params" : [
      ["callback","JsVar","The callback to call with received advertising packets, or undefined to stop"]
    ]
}

Start/stop listening for BLE advertising packets within range. Returns a
`BluetoothDevice` for each advertsing packet. **This is not an active scan, so
Scan Response advertising data is not included**

```
// Start scanning
packets=10;
NRF.setScan(function(d) {
  packets--;
  if (packets<=0)
    NRF.setScan(); // stop scanning
  else
    console.log(d); // print packet info
});
```

Each `BluetoothDevice` will look a bit like:

```
BluetoothDevice {
  "id": "aa:bb:cc:dd:ee:ff", // address
  "rssi": -89,               // signal strength
  "services": [ "128bit-uuid", ... ],     // zero or more service UUIDs
  "data": new Uint8Array([ ... ]).buffer, // ArrayBuffer of returned data
  "serviceData" : { "0123" : [ 1 ] }, // if service data is in 'data', it's extracted here
  "manufacturer" : 0x1234, // if manufacturer data is in 'data', the 16 bit manufacturer ID is extracted here
  "manufacturerData" : [...], // if manufacturer data is in 'data', the data is extracted here
  "name": "DeviceName"       // the advertised device name
 }
```

**Note:** BLE advertising packets can arrive quickly - faster than you'll
be able to print them to the console. It's best only to print a few, or
to use a function like `NRF.findDevices(..)` which will collate a list
of available devices.

**Note:** Using setScan turns the radio's receive mode on constantly. This
can draw a *lot* of power (12mA or so), so you should use it sparingly or
you can run your battery down quickly.
*/
void jswrap_nrf_bluetooth_setScan_cb(JsVar *callback, JsVar *adv) {
  /* This is called when we get data - do some processing here in the main loop
  then call the callback with it (it avoids us doing more allocations than
  needed inside the IRQ) */
  if (!adv) return;
  // Create a proper BluetoothDevice object
  JsVar *device = jspNewObject(0, "BluetoothDevice");
  jsvObjectSetChildAndUnLock(device, "id", jsvObjectGetChild(adv, "id", 0));
  jsvObjectSetChildAndUnLock(device, "rssi", jsvObjectGetChild(adv, "rssi", 0));
  JsVar *services = jsvNewEmptyArray();
  JsVar *serviceData = jsvNewObject();
  JsVar *data = jsvObjectGetChild(adv, "data", 0);
  if (data) {
    jsvObjectSetChild(device, "data", data);
    JSV_GET_AS_CHAR_ARRAY(dPtr, dLen, data);
    if (dPtr && dLen) {
      if (services && serviceData) {
        uint32_t i = 0;
        while (i < dLen) {
          uint8_t field_length = dPtr[i];
          uint8_t field_type   = dPtr[i + 1];

          if (field_type == BLE_GAP_AD_TYPE_SHORT_LOCAL_NAME) { // 0x08 - Short Name
            jsvObjectSetChildAndUnLock(device, "shortName", jsvNewStringOfLength(field_length-1, (char*)&dPtr[i+2]));
          } else if (field_type == BLE_GAP_AD_TYPE_COMPLETE_LOCAL_NAME) { // 0x09 - Complete Name
            jsvObjectSetChildAndUnLock(device, "name", jsvNewStringOfLength(field_length-1, (char*)&dPtr[i+2]));
          } else if (field_type == BLE_GAP_AD_TYPE_16BIT_SERVICE_UUID_MORE_AVAILABLE || // 0x02, 0x03 - 16 bit UUID
                     field_type == BLE_GAP_AD_TYPE_16BIT_SERVICE_UUID_COMPLETE) {
            for (int svc_idx = 2; svc_idx < field_length + 1; svc_idx += 2) {
              JsVar *s = jsvVarPrintf("%04x", UNALIGNED_UINT16(&dPtr[i+svc_idx]));
              jsvArrayPushAndUnLock(services, s);
            }
          } else if (field_type == BLE_GAP_AD_TYPE_128BIT_SERVICE_UUID_MORE_AVAILABLE || // 0x06, 0x07 - 128 bit UUID
                     field_type == BLE_GAP_AD_TYPE_128BIT_SERVICE_UUID_COMPLETE) {
            JsVar *s = bleUUID128ToStr((uint8_t*)&dPtr[i+2]);
            jsvArrayPushAndUnLock(services, s);
          } else if (field_type == BLE_GAP_AD_TYPE_SERVICE_DATA) { // 0x16 - service data 16 bit UUID
            JsVar *childName = jsvAsArrayIndexAndUnLock(jsvVarPrintf("%04x", UNALIGNED_UINT16(&dPtr[i+2])));
            if (childName) {
              JsVar *child = jsvFindChildFromVar(serviceData, childName, true);
              JsVar *value = jsvNewArrayBufferWithData(field_length-3, (unsigned char*)&dPtr[i+4]);
              if (child && value) jsvSetValueOfName(child, value);
              jsvUnLock2(child, value);
            }
            jsvUnLock(childName);
          } else if (field_type == BLE_GAP_AD_TYPE_MANUFACTURER_SPECIFIC_DATA) {
            jsvObjectSetChildAndUnLock(device, "manufacturer",
                            jsvNewFromInteger((dPtr[i+3]<<8) | dPtr[i+2]));
            jsvObjectSetChildAndUnLock(device, "manufacturerData",
                jsvNewArrayBufferWithData(field_length-3, (unsigned char*)&dPtr[i+4]));
          } // or unknown...
          i += field_length + 1;
        }
      }
    }
  }
  if (jsvGetArrayLength(services))
    jsvObjectSetChild(device, "services", services);
  if (jsvGetLength(serviceData))
    jsvObjectSetChild(device, "serviceData", serviceData);
  jsvUnLock3(data, services, serviceData);
  jspExecuteFunction(callback, 0, 1, &device);
  jsvUnLock(device);
}

void jswrap_nrf_bluetooth_setScan(JsVar *callback) {
  // set the callback event variable
  if (!jsvIsFunction(callback)) callback=0;
  if (callback) {
    JsVar *fn = jsvNewNativeFunction((void (*)(void))jswrap_nrf_bluetooth_setScan_cb, JSWAT_THIS_ARG|(JSWAT_JSVAR<<JSWAT_BITS));
    if (fn) {
      jsvObjectSetChild(fn, JSPARSE_FUNCTION_THIS_NAME, callback);
      jsvObjectSetChild(execInfo.root, BLE_SCAN_EVENT, fn);
      jsvUnLock(fn);
    }
  } else
    jsvObjectRemoveChild(execInfo.root, BLE_SCAN_EVENT);
  // either start or stop scanning
  uint32_t err_code = jsble_set_scanning(callback != 0);
  jsble_check_error(err_code);
}


/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "findDevices",
    "generate" : "jswrap_nrf_bluetooth_findDevices",
    "params" : [
      ["callback","JsVar","The callback to call with received advertising packets, or undefined to stop"],
      ["time","JsVar","The time in milliseconds to scan for (defaults to 2000)"]
    ]
}
Utility function to return a list of BLE devices detected in range. Behind the scenes,
this uses `NRF.setScan(...)` and collates the results.

```
NRF.findDevices(function(devices) {
  console.log(devices);
}, 1000);
```

prints something like:

```
[
  BluetoothDevice {
    "id": "e7:e0:57:ad:36:a2 random",
    "rssi": -45,
    "services": [ "4567" ],
    "serviceData" : { "0123" : [ 1 ] },
    "manufacturerData" : [...],
    "data": new ArrayBuffer([ ... ]),
    "name": "Puck.js 36a2"
   },
  BluetoothDevice {
    "id": "c0:52:3f:50:42:c9 random",
    "rssi": -65,
    "data": new ArrayBuffer([ ... ]),
    "name": "Puck.js 8f57"
   }
 ]
```

For more information on the structure, see `NRF.setScan`.

You could then use [`BluetoothDevice.gatt.connect(...)`](/Reference#l_BluetoothRemoteGATTServer_connect) on
the device returned, to make a connection.

You can also use [`NRF.connect(...)`](/Reference#l_NRF_connect) on just the `id` string returned, which
may be useful if you always want to connect to a specific device.

**Note:** Using findDevices turns the radio's receive mode on for 2000ms (or however long you specify). This
can draw a *lot* of power (12mA or so), so you should use it sparingly or you can run your battery down quickly.

**Note:** The 'data' field contains the data of *the last packet received*. There may have been more
packets. To get data for each packet individually use `NRF.setScan` instead.
*/
void jswrap_nrf_bluetooth_findDevices_found_cb(JsVar *device) {
  JsVar *arr = jsvObjectGetChild(execInfo.hiddenRoot, "BLEADV", JSV_ARRAY);
  if (!arr) return;
  JsVar *deviceAddr = jsvObjectGetChild(device, "id", 0);
  JsVar *found = 0;
  JsvObjectIterator it;
  jsvObjectIteratorNew(&it, arr);
  while (!found && jsvObjectIteratorHasValue(&it)) {
    JsVar *obj = jsvObjectIteratorGetValue(&it);
    JsVar *addr = jsvObjectGetChild(obj, "id", 0);
    if (jsvCompareString(addr, deviceAddr, 0, 0, true) == 0)
      found = jsvLockAgain(obj);
    jsvUnLock2(addr, obj);
    jsvObjectIteratorNext(&it);
  }
  jsvObjectIteratorFree(&it);
  if (found) {
    JsvObjectIterator oit;
    jsvObjectIteratorNew(&oit, device);
    while (jsvObjectIteratorHasValue(&oit)) {
      JsVar *key = jsvObjectIteratorGetKey(&oit);
      JsVar *value = jsvSkipName(key);
      JsVar *existingKey = jsvFindChildFromVar(found, key, true);
      bool isServices = jsvIsStringEqual(key,"services");
      bool isServiceData = jsvIsStringEqual(key,"serviceData");
      if (isServices || isServiceData) {
        // for services or servicedata we append to the array/object
        JsVar *existingValue = jsvSkipName(existingKey);
        if (existingValue) {
          if (isServices) {
            jsvArrayPushAll(existingValue, value, true);
          } else {
            jsvObjectAppendAll(existingValue, value);
          }
          jsvUnLock(existingValue);
        } else // nothing already - just copy
          jsvSetValueOfName(existingKey, value);
      }
      jsvUnLock3(existingKey, key, value);
      jsvObjectIteratorNext(&oit);
    }
    jsvObjectIteratorFree(&oit);
  } else
    jsvArrayPush(arr, device);
  jsvUnLock3(found, deviceAddr, arr);
}
void jswrap_nrf_bluetooth_findDevices_timeout_cb() {
  jswrap_nrf_bluetooth_setScan(0);
  JsVar *arr = jsvObjectGetChild(execInfo.hiddenRoot, "BLEADV", JSV_ARRAY);
  JsVar *cb = jsvObjectGetChild(execInfo.hiddenRoot, "BLEADVCB", 0);
  jsvObjectRemoveChild(execInfo.hiddenRoot, "BLEADV");
  jsvObjectRemoveChild(execInfo.hiddenRoot, "BLEADVCB");
  if (arr && cb) {
    jsiQueueEvents(0, cb, &arr, 1);
  }
  jsvUnLock2(arr,cb);
}
void jswrap_nrf_bluetooth_findDevices(JsVar *callback, JsVar *timeout) {
  if (!jsvIsFunction(callback)) {
    jsExceptionHere(JSET_ERROR, "Expecting function for first argument, got %t", callback);
    return;
  }
  // utility fn that uses setScan
  JsVarFloat time = 2000;
  if (!jsvIsUndefined(timeout)) {
    time = jsvGetFloat(timeout);
    if (!jsvIsNumeric(timeout) || time < 10) {
      jsExceptionHere(JSET_ERROR, "Invalid timeout");
      return;
    }
  }
  jsvObjectSetChildAndUnLock(execInfo.hiddenRoot, "BLEADV", jsvNewEmptyArray());
  jsvObjectSetChild(execInfo.hiddenRoot, "BLEADVCB", callback);
  JsVar *fn;
  fn = jsvNewNativeFunction((void (*)(void))jswrap_nrf_bluetooth_findDevices_found_cb, JSWAT_VOID|(JSWAT_JSVAR<<JSWAT_BITS));
  if (fn) {
    jswrap_nrf_bluetooth_setScan(fn);
    jsvUnLock(fn);
  }
  fn = jsvNewNativeFunction((void (*)(void))jswrap_nrf_bluetooth_findDevices_timeout_cb, JSWAT_VOID);
  if (fn)
    jsvUnLock2(jswrap_interface_setTimeout(fn, time, 0), fn);
}

/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "setRSSIHandler",
    "generate" : "jswrap_nrf_bluetooth_setRSSIHandler",
    "params" : [
      ["callback","JsVar","The callback to call with the RSSI value, or undefined to stop"]
    ]
}

Start/stop listening for RSSI values on the currently active connection
(where This device is a peripheral and is being connected to by a 'central' device)

```
// Start scanning
NRF.setRSSIHandler(function(rssi) {
  console.log(rssi); // prints -85 (or similar)
});
// Stop Scanning
NRF.setRSSIHandler();
```

RSSI is the 'Received Signal Strength Indication' in dBm
*/
void jswrap_nrf_bluetooth_setRSSIHandler(JsVar *callback) {
  // set the callback event variable
  if (!jsvIsFunction(callback)) callback=0;
  jsvObjectSetChild(execInfo.root, BLE_RSSI_EVENT, callback);
  // either start or stop scanning
  uint32_t err_code = jsble_set_rssi_scan(callback != 0);
  jsble_check_error(err_code);
}




/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "setTxPower",
    "generate" : "jswrap_nrf_bluetooth_setTxPower",
    "params" : [
      ["power","int","Transmit power. Accepted values are -40, -30, -20, -16, -12, -8, -4, 0, and 4 dBm. Others will give an error code."]
    ]
}
Set the BLE radio transmit power. The default TX power is 0 dBm.
*/
void jswrap_nrf_bluetooth_setTxPower(JsVarInt pwr) {
  uint32_t              err_code;
#ifdef NRF5X
  err_code = sd_ble_gap_tx_power_set(pwr);
#else
  err_code = 0xDEAD;
  jsiConsolePrintf("FIXME\n");
#endif
  jsble_check_error(err_code);
}


/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "setLowPowerConnection",
    "generate" : "jswrap_nrf_bluetooth_setLowPowerConnection",
    "params" : [
      ["lowPower","bool","Whether the connection is low power or not"]
    ]
}

This sets the connection parameters - these affect the transfer speed and
power usage when the device is connected.

* When not low power, the connection interval is between 7.5 and 20ms
* When low power, the connection interval is between 500 and 1000ms

When low power connection is enabled, transfers of data over Bluetooth
will be very slow, however power usage while connected will be drastically
decreased.

This will only take effect after the connection is disconnected and
re-established.
*/
void jswrap_nrf_bluetooth_setLowPowerConnection(bool lowPower) {
  BLEFlags oldflags = jsvGetIntegerAndUnLock(jsvObjectGetChild(execInfo.hiddenRoot, BLE_NAME_FLAGS, 0));
  BLEFlags flags = oldflags;
  if (lowPower)
    flags |= BLE_FLAGS_LOW_POWER;
  else
    flags &= ~BLE_FLAGS_LOW_POWER;
  if (flags != oldflags) {
    jsvObjectSetChildAndUnLock(execInfo.hiddenRoot, BLE_NAME_FLAGS, jsvNewFromInteger(flags));
    jswrap_nrf_bluetooth_restart();
  }
}


/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "nfcURL",
    "ifdef" : "NRF52",
    "generate" : "jswrap_nrf_nfcURL",
    "params" : [
      ["url","JsVar","The URL string to expose on NFC, or `undefined` to disable NFC"]
    ]
}
Enables NFC and starts advertising the given URL. For example:

```
NRF.nfcURL("http://espruino.com");
```
*/
void jswrap_nrf_nfcURL(JsVar *url) {
#ifdef USE_NFC
  // Check for disabling NFC
  if (jsvIsUndefined(url)) {
    jsvObjectRemoveChild(execInfo.hiddenRoot, "NfcData");
    jswrap_nrf_nfcStop();
    return;
  }

  if (!jsvIsString(url)) {
    jsExceptionHere(JSET_TYPEERROR, "Expecting a String, got %t", url);
    return;
  }

  JSV_GET_AS_CHAR_ARRAY(urlPtr, urlLen, url);
  if (!urlPtr || !urlLen)
    return jsExceptionHere(JSET_ERROR, "Unable to get URL data");

  nfc_uri_id_t uriType = NFC_URI_NONE;
  if (memcmp(urlPtr, "http://", 7)==0) {
    urlPtr+=7;
    urlLen-=7;
    uriType = NFC_URI_HTTP;
  } else if (memcmp(urlPtr, "https://", 8)==0) {
    urlPtr+=8;
    urlLen-=8;
    uriType = NFC_URI_HTTPS;
  }

  /* Encode NDEF message into a flat string - we need this to store the
   * data so it hangs around. Avoid having a static var so we have RAM
   * available if not using NFC. NFC data is read by nfc_callback */
  JsVar *flatStr = jsvNewFlatStringOfLength(NDEF_FULL_URL_HEADER_LEN + urlLen + NDEF_TERM_TLV_LEN);
  if (!flatStr)
    return jsExceptionHere(JSET_ERROR, "Unable to create string with URI data in");
  jsvObjectSetChild(execInfo.hiddenRoot, "NfcData", flatStr);
  uint8_t *flatStrPtr = (uint8_t*)jsvGetFlatStringPointer(flatStr);
  jsvUnLock(flatStr);

  /* assemble NDEF Message */
  memcpy(flatStrPtr, NDEF_HEADER, NDEF_FULL_URL_HEADER_LEN); /* fill header */
  flatStrPtr[NDEF_IC_OFFSET] = uriType; /* set URI Identifier Code */
  memcpy(flatStrPtr+NDEF_FULL_URL_HEADER_LEN, urlPtr, urlLen); /* add payload */

  /* inject length fields into header */
  flatStrPtr[NDEF_MSG_LEN_OFFSET] = NDEF_RECORD_HEADER_LEN + urlLen;
  flatStrPtr[NDEF_PL_LEN_LSB_OFFSET] = NDEF_IC_LEN + urlLen;

  /* write terminator TLV block */
  flatStrPtr[NDEF_FULL_URL_HEADER_LEN + urlLen] = NDEF_TERM_TLV;

  /* start nfc peripheral */
  JsVar* uid = jswrap_nrf_nfcStart(NULL);

  /* inject UID/BCC */
  size_t len;
  char *uidPtr = jsvGetDataPointer(uid, &len);
  if(uidPtr) memcpy(flatStrPtr, uidPtr, TAG_HEADER_LEN);
  jsvUnLock(uid);
#endif
}

/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "nfcRaw",
    "ifdef" : "NRF52",
    "generate" : "jswrap_nrf_nfcRaw",
    "params" : [
      ["payload","JsVar","The NFC NDEF message to deliver to the reader"]
    ]
}
Enables NFC and starts advertising with Raw data. For example:

```
NRF.nfcRaw(new Uint8Array([193, 1, 0, 0, 0, 13, 85, 3, 101, 115, 112, 114, 117, 105, 110, 111, 46, 99, 111, 109]));
// same as NRF.nfcURL("http://espruino.com");
```
*/
void jswrap_nrf_nfcRaw(JsVar *payload) {
#ifdef USE_NFC
  // Check for disabling NFC
  if (jsvIsUndefined(payload)) {
    jsvObjectRemoveChild(execInfo.hiddenRoot, "NfcData");
    jswrap_nrf_nfcStop();
    return;
  }

  JSV_GET_AS_CHAR_ARRAY(dataPtr, dataLen, payload);
  if (!dataPtr || !dataLen)
    return jsExceptionHere(JSET_ERROR, "Unable to get NFC data");

  /* Create a flat string - we need this to store the NFC data so it hangs around.
   * Avoid having a static var so we have RAM available if not using NFC.
   * NFC data is read by nfc_callback in bluetooth.c */
  JsVar *flatStr = jsvNewFlatStringOfLength(NDEF_FULL_RAW_HEADER_LEN + dataLen + NDEF_TERM_TLV_LEN);
  if (!flatStr)
    return jsExceptionHere(JSET_ERROR, "Unable to create string with NFC data in");
  jsvObjectSetChild(execInfo.hiddenRoot, "NfcData", flatStr);
  uint8_t *flatStrPtr = (uint8_t*)jsvGetFlatStringPointer(flatStr);
  jsvUnLock(flatStr);

  /* assemble NDEF Message */
  memcpy(flatStrPtr, NDEF_HEADER, NDEF_FULL_RAW_HEADER_LEN); /* fill header */
  memcpy(flatStrPtr+NDEF_FULL_RAW_HEADER_LEN, dataPtr, dataLen); /* add payload */

  /* inject length fields into header */
  flatStrPtr[NDEF_MSG_LEN_OFFSET] = dataLen;

  /* write terminator TLV block */
  flatStrPtr[NDEF_FULL_RAW_HEADER_LEN + dataLen] = NDEF_TERM_TLV;

  /* start nfc peripheral */
  JsVar* uid = jswrap_nrf_nfcStart(NULL);

  /* inject UID/BCC */
  size_t len;
  char *uidPtr = jsvGetDataPointer(uid, &len);
  if(uidPtr) memcpy(flatStrPtr, uidPtr, TAG_HEADER_LEN);
  jsvUnLock(uid);
#endif
}


/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "nfcStart",
    "ifdef" : "NRF52",
    "generate" : "jswrap_nrf_nfcStart",
    "params" : [
      ["payload","JsVar","Optional 7 byte UID"]
    ],
    "return" : ["JsVar", "Internal tag memory (first 10 bytes of tag data)" ]
}
**Advanced NFC Functionality.** If you just want to advertise a URL, use `NRF.nfcURL` instead.

Enables NFC and starts advertising. `NFCrx` events will be
fired when data is received.

```
NRF.nfcStart();
```
*/
JsVar *jswrap_nrf_nfcStart(JsVar *payload) {
#ifdef USE_NFC
  /* Turn off NFC */
  jsble_nfc_stop();

  /* Create a flat string - we need this to store the NFC data so it hangs around.
   * Avoid having a static var so we have RAM available if not using NFC */
  JsVar *flatStr = 0;
  if (!jsvIsUndefined(payload)) {
    /* Custom UID */
    JSV_GET_AS_CHAR_ARRAY(dataPtr, dataLen, payload);
    if (!dataPtr || !dataLen) {
      jsExceptionHere(JSET_ERROR, "Unable to get NFC data");
      return 0;
    }
    flatStr = jsvNewFlatStringOfLength(dataLen);
    if (!flatStr) {
      jsExceptionHere(JSET_ERROR, "Unable to create string with NFC data in");
      return 0;
    }
    jsvObjectSetChild(execInfo.hiddenRoot, "NfcEnabled", flatStr);
    jsvUnLock(flatStr);
    uint8_t *flatStrPtr = (uint8_t*)jsvGetFlatStringPointer(flatStr);
    memcpy(flatStrPtr, dataPtr, dataLen);
  } else {
    /* Default UID */
    flatStr = jsvNewFlatStringOfLength(0);
    if (!flatStr) {
      jsExceptionHere(JSET_ERROR, "Unable to create string with NFC data in");
      return 0;
    }
    jsvObjectSetChild(execInfo.hiddenRoot, "NfcEnabled", flatStr);
    jsvUnLock(flatStr);
  }

  /* start nfc */
  uint8_t *flatStrPtr = (uint8_t*)jsvGetFlatStringPointer(flatStr);
  jsble_nfc_start(flatStrPtr, jsvGetLength(flatStr));

  /* return internal tag header */
  char *ptr = 0; size_t size = TAG_HEADER_LEN;
  JsVar *arr = jsvNewArrayBufferWithPtr(size, &ptr);
  if (ptr) jsble_nfc_get_internal((uint8_t *)ptr, &size);
  return arr;
#endif
}

/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "nfcStop",
    "ifdef" : "NRF52",
    "generate" : "jswrap_nrf_nfcStop",
    "params" : [ ]
}
**Advanced NFC Functionality.** If you just want to advertise a URL, use `NRF.nfcURL` instead.

Disables NFC.

```
NRF.nfcStop();
```
*/
void jswrap_nrf_nfcStop() {
#ifdef USE_NFC
  jsvObjectRemoveChild(execInfo.hiddenRoot, "NfcEnabled");
  jsble_nfc_stop();
#endif
}


/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "nfcSend",
    "ifdef" : "NRF52",
    "generate" : "jswrap_nrf_nfcSend",
    "params" : [
      ["payload","JsVar","Optional tx data"]
    ]
}
**Advanced NFC Functionality.** If you just want to advertise a URL, use `NRF.nfcURL` instead.

Acknowledges the last frame and optionally transmits a response.
If payload is an array, then a array.length byte nfc frame is sent.
If payload is a int, then a 4bit ACK/NACK is sent.
**Note:** ```nfcSend``` should always be called after an ```NFCrx``` event.

```
NRF.nfcSend(new Uint8Array([0x01, 0x02, ...]));
// or
NRF.nfcSend(0x0A);
// or
NRF.nfcSend();
```
*/
void jswrap_nrf_nfcSend(JsVar *payload) {
#ifdef USE_NFC
  /* Switch to RX */
  if (jsvIsUndefined(payload))
    return jsble_nfc_send_rsp(0, 0);

  /* Send 4 bit ACK/NACK */
  if (jsvIsInt(payload))
    return jsble_nfc_send_rsp(jsvGetInteger(payload), 4);

  /* Send n byte payload */
  JSV_GET_AS_CHAR_ARRAY(dataPtr, dataLen, payload);
  if (!dataPtr || !dataLen)
    return jsExceptionHere(JSET_ERROR, "Unable to get NFC data");

  jsble_nfc_send((uint8_t*)dataPtr, dataLen);
#endif
}

/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "sendHIDReport",
    "ifdef" : "NRF52",
    "generate" : "jswrap_nrf_sendHIDReport",
    "params" : [
      ["data","JsVar","Input report data as an array"],
      ["callback","JsVar","A callback function to be called when the data is sent"]
    ]
}
Send a USB HID report. HID must first be enabled with `NRF.setServices({}, {hid: hid_report})`
*/
void jswrap_nrf_sendHIDReport(JsVar *data, JsVar *callback) {
#if BLE_HIDS_ENABLED
  JSV_GET_AS_CHAR_ARRAY(vPtr, vLen, data)
  if (vPtr && vLen) {
    if (jsvIsFunction(callback))
      jsvObjectSetChild(execInfo.root, BLE_HID_SENT_EVENT, callback);
    jsble_send_hid_input_report((uint8_t*)vPtr, vLen);
  } else {
    jsExceptionHere(JSET_ERROR, "Expecting array, got %t", data);
  }
#endif
}


/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "requestDevice",
    "ifdef" : "NRF52,ESP32",
    "generate" : "jswrap_nrf_bluetooth_requestDevice",
    "params" : [
      ["options","JsVar","Options used to filter the device to use"]
    ],
    "return" : ["JsVar", "A Promise that is resolved (or rejected) when the connection is complete" ]
}
Search for available devices matching the given filters. Since we have no UI here,
Espruino will pick the FIRST device it finds, or it'll call `catch`.

The following filter types are implemented:

* `services` - list of services as strings (all of which must match). 128 bit services must be in the form '01230123-0123-0123-0123-012301230123'
* `name` - exact device name
* `namePrefix` - starting characters of device name

```
NRF.requestDevice({ filters: [{ namePrefix: 'Puck.js' }] }).then(function(device) { ... });
// or
NRF.requestDevice({ filters: [{ services: ['1823'] }] }).then(function(device) { ... });
```

You can also specify a timeout to wait for devices in milliseconds. The default is 2 seconds (2000):

```
NRF.requestDevice({ timeout:2000, filters: [ ... ] })
```

As a full example, to send data to another Puck.js to turn an LED on:

```
var gatt;
NRF.requestDevice({ filters: [{ namePrefix: 'Puck.js' }] }).then(function(device) {
  return device.gatt.connect();
}).then(function(g) {
  gatt = g;
  return gatt.getPrimaryService("6e400001-b5a3-f393-e0a9-e50e24dcca9e");
}).then(function(service) {
  return service.getCharacteristic("6e400002-b5a3-f393-e0a9-e50e24dcca9e");
}).then(function(characteristic) {
  return characteristic.writeValue("LED1.set()\n");
}).then(function() {
  gatt.disconnect();
  console.log("Done!");
});
```

Or slightly more concisely, using ES6 arrow functions:

```
var gatt;
NRF.requestDevice({ filters: [{ namePrefix: 'Puck.js' }]}).then(
  device => device.gatt.connect()).then(
  g => (gatt=g).getPrimaryService("6e400001-b5a3-f393-e0a9-e50e24dcca9e")).then(
  service => service.getCharacteristic("6e400002-b5a3-f393-e0a9-e50e24dcca9e")).then(
  characteristic => characteristic.writeValue("LED1.reset()\n")).then(
  () => { gatt.disconnect(); console.log("Done!"); } );
```

Note that you'll have to keep track of the `gatt` variable so that you can
disconnect the Bluetooth connection when you're done.
*/
#if CENTRAL_LINK_COUNT>0

JsVar *jswrap_nrf_bluetooth_requestDevice_filter_device(JsVar *filter, JsVar *device) {
  bool matches = true;
  JsVar *v;
  if ((v = jsvObjectGetChild(filter, "services", 0))) {
    // Find one service in the device's service
    JsVar *deviceServices = jsvObjectGetChild(device, "services", 0);
    JsvObjectIterator it;
    jsvObjectIteratorNew(&it, v);
    while (jsvObjectIteratorHasValue(&it)) {
      bool foundService = false;
      JsVar *uservice = jsvObjectIteratorGetValue(&it);
      ble_uuid_t userviceUuid;
      bleVarToUUIDAndUnLock(&userviceUuid, uservice);
      JsvObjectIterator dit;
      jsvObjectIteratorNew(&dit, deviceServices);
      while (jsvObjectIteratorHasValue(&dit)) {
        JsVar *deviceService = jsvObjectIteratorGetValue(&dit);
        ble_uuid_t deviceServiceUuid;
        bleVarToUUIDAndUnLock(&deviceServiceUuid, deviceService);
        if (bleUUIDEqual(userviceUuid, deviceServiceUuid))
          foundService = true;
        jsvObjectIteratorNext(&dit);
      }
      jsvObjectIteratorFree(&dit);
      if (!foundService) matches = false;
      jsvObjectIteratorNext(&it);
    }
    jsvObjectIteratorFree(&it);
    jsvUnLock2(v, deviceServices);
  }
  if ((v = jsvObjectGetChild(filter, "name", 0))) {
    // match name exactly
    JsVar *deviceName = jsvObjectGetChild(device, "name", 0);
    if (!jsvIsEqual(v, deviceName))
      matches = false;
    jsvUnLock2(v, deviceName);
  }
  if ((v = jsvObjectGetChild(filter, "namePrefix", 0))) {
    // match start of name
    JsVar *deviceName = jsvObjectGetChild(device, "name", 0);
    if (!jsvIsString(v) ||
        !jsvIsString(deviceName) ||
        jsvGetStringLength(v)>jsvGetStringLength(deviceName) ||
        jsvCompareString(v, deviceName,0,0,true)!=0)
      matches = false;
    jsvUnLock2(v, deviceName);
  }
  return matches ? jsvLockAgain(device) : 0;
}

JsVar *jswrap_nrf_bluetooth_requestDevice_filter_devices(JsVar *filter, JsVar *devices) {
  JsVar *foundDevice = 0;
  JsvObjectIterator dit;
  jsvObjectIteratorNew(&dit, devices);
  while (!foundDevice && jsvObjectIteratorHasValue(&dit)) {
    JsVar *device = jsvObjectIteratorGetValue(&dit);
    foundDevice = jswrap_nrf_bluetooth_requestDevice_filter_device(filter, device);
    jsvUnLock(device);
    jsvObjectIteratorNext(&dit);
  }
  jsvObjectIteratorFree(&dit);
  return foundDevice;
}

void jswrap_nrf_bluetooth_requestDevice_finish(JsVar *options, JsVar *devices) {
  if (!bleInTask(BLETASK_REQUEST_DEVICE))
    return;
  JsVar *foundDevice = 0;
  JsVar *filters = jsvObjectGetChild(options, "filters", 0);
  if (jsvIsArray(filters)) {
    JsvObjectIterator fit;
    jsvObjectIteratorNew(&fit, filters);
    while (!foundDevice && jsvObjectIteratorHasValue(&fit)) {
      JsVar *filter = jsvObjectIteratorGetValue(&fit);
      foundDevice = jswrap_nrf_bluetooth_requestDevice_filter_devices(filter, devices);
      jsvUnLock(filter);
      jsvObjectIteratorNext(&fit);
    }
    jsvObjectIteratorFree(&fit);
  } else {
    jsExceptionHere(JSET_TYPEERROR, "requestDevice expecting an array of filters, got %t", filters);
    bleCompleteTaskFail(BLETASK_REQUEST_DEVICE, 0);
    jsvUnLock(filters);
    return;
  }
  jsvUnLock(filters);
  if (foundDevice)
    bleCompleteTaskSuccessAndUnLock(BLETASK_REQUEST_DEVICE, foundDevice);
  else
    bleCompleteTaskFailAndUnLock(BLETASK_REQUEST_DEVICE, jsvNewFromString("No device found matching filters"));
}
#endif

JsVar *jswrap_nrf_bluetooth_requestDevice(JsVar *options) {
#if CENTRAL_LINK_COUNT>0
  if (!(jsvIsUndefined(options) || jsvIsObject(options))) {
    jsExceptionHere(JSET_TYPEERROR, "Expecting an object, for %t", options);
    return 0;
  }
  JsVar *timeout = jsvObjectGetChild(options, "timeout", 0);
  JsVar *promise = 0;

  if (bleNewTask(BLETASK_REQUEST_DEVICE, 0)) {
    JsVar *fn = jsvNewNativeFunction((void (*)(void))jswrap_nrf_bluetooth_requestDevice_finish, JSWAT_THIS_ARG|(JSWAT_JSVAR<<JSWAT_BITS));
    if (fn) {
      jsvObjectSetChild(fn, JSPARSE_FUNCTION_THIS_NAME, options);
      jswrap_nrf_bluetooth_findDevices(fn, timeout);
      jsvUnLock(fn);
    }
    promise = jsvLockAgainSafe(blePromise);
  }
  jsvUnLock(timeout);
  return promise;
#else
  jsExceptionHere(JSET_ERROR, "Unimplemented");
  return 0;
#endif
}


/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "connect",
    "ifdef" : "NRF52,ESP32",
    "generate" : "jswrap_nrf_bluetooth_connect",
    "params" : [
      ["mac","JsVar","The MAC address to connect to"]
    ],
    "return" : ["JsVar", "A Promise that is resolved (or rejected) when the connection is complete" ]
}
Connect to a BLE device by MAC address. Returns a promise,
the argument of which is the `BluetoothRemoteGATTServer` connection.

```
NRF.connect("aa:bb:cc:dd:ee").then(function(server) {
  // ...
});
```

You can use it as follows - this would connect to another Puck device and turn its LED on:

```
var gatt;
NRF.connect("aa:bb:cc:dd:ee").then(function(g) {
  gatt = g;
  return gatt.getPrimaryService("6e400001-b5a3-f393-e0a9-e50e24dcca9e");
}).then(function(service) {
  return service.getCharacteristic("6e400002-b5a3-f393-e0a9-e50e24dcca9e");
}).then(function(characteristic) {
  return characteristic.writeValue("LED1.set()\n");
}).then(function() {
  gatt.disconnect();
  console.log("Done!");
});
```
*/
JsVar *jswrap_nrf_bluetooth_connect(JsVar *mac) {
#if CENTRAL_LINK_COUNT>0
  JsVar *device = jspNewObject(0, "BluetoothDevice");
  if (!device) return 0;
  jsvObjectSetChild(device, "id", mac);
  JsVar *gatt = jswrap_BluetoothDevice_gatt(device);
  if (!gatt) return 0;
  return jswrap_nrf_BluetoothRemoteGATTServer_connect(gatt);
#else
  jsExceptionHere(JSET_ERROR, "Unimplemented");
  return 0;
#endif
}

/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "setWhitelist",
    "ifdef" : "NRF52",
    "generate" : "jswrap_nrf_setWhitelist",
    "params" : [
      ["whitelisting","bool","Are we using a whitelist? (default false)"]
    ]
}
If set to true, whenever a device bonds it will be added to the
whitelist.

When set to false, the whitelist is cleared and newly bonded
devices will not be added to the whitelist.

**Note:** This is remembered between `reset()`s but isn't
remembered after power-on (you'll have to add it to `onInit()`.
*/
void jswrap_nrf_setWhitelist(bool whitelist) {
#if PEER_MANAGER_ENABLED
  jsble_central_setWhitelist(whitelist);
#endif
}


/*JSON{
  "type" : "class",
  "class" : "BluetoothDevice",
  "ifdef" : "NRF52"
}
A Web Bluetooth-style device - you can request one using `NRF.requestDevice(address)`

For example:

```
var gatt;
NRF.requestDevice({ filters: [{ name: 'Puck.js abcd' }] }).then(function(device) {
  console.log("found device");
  return device.gatt.connect();
}).then(function(g) {
  gatt = g;
  console.log("connected");
  return gatt.startBonding();
}).then(function() {
  console.log("bonded", gatt.getSecurityStatus());
  gatt.disconnect();
}).catch(function(e) {
  console.log("ERROR",e);
});
```
*/
/*JSON{
    "type" : "property",
    "class" : "BluetoothDevice",
    "name" : "gatt",
    "ifdef" : "NRF52,ESP32",
    "generate" : "jswrap_BluetoothDevice_gatt",
    "return" : ["JsVar", "A `BluetoothRemoteGATTServer` for this device" ]
}
*/
JsVar *jswrap_BluetoothDevice_gatt(JsVar *parent) {
#if CENTRAL_LINK_COUNT>0
  JsVar *gatt = jsvObjectGetChild(parent, "gatt", 0);
  if (gatt) return gatt;

  gatt = jspNewObject(0, "BluetoothRemoteGATTServer");
  jsvObjectSetChild(parent, "gatt", gatt);
  jsvObjectSetChild(gatt, "device", parent);
  jsvObjectSetChildAndUnLock(gatt, "connected", jsvNewFromBool(false));
  return gatt;
#else
  jsExceptionHere(JSET_ERROR, "Unimplemented");
  return 0;
#endif
}

/*JSON{
    "type" : "method",
    "class" : "BluetoothRemoteGATTServer",
    "name" : "connect",
    "ifdef" : "NRF52,ESP32",
    "generate" : "jswrap_nrf_BluetoothRemoteGATTServer_connect",
    "return" : ["JsVar", "A Promise that is resolved (or rejected) when the connection is complete" ]
}
Connect to a BLE device - returns a promise,
the argument of which is the `BluetoothRemoteGATTServer` connection.

See [`NRF.requestDevice`](/Reference#l_NRF_requestDevice) for usage examples.
*/
#if CENTRAL_LINK_COUNT>0
static void _jswrap_nrf_bluetooth_central_connect(JsVar *addr) {
  // this function gets called on idle - just to make it less
  // likely we get connected while in the middle of executing stuff
  ble_gap_addr_t peer_addr;
  // this should be ok since we checked in jswrap_nrf_BluetoothRemoteGATTServer_connect
  if (!bleVarToAddr(addr, &peer_addr)) return;
  jsble_central_connect(peer_addr);
}
#endif

JsVar *jswrap_nrf_BluetoothRemoteGATTServer_connect(JsVar *parent) {
#if CENTRAL_LINK_COUNT>0

  JsVar *device = jsvObjectGetChild(parent, "device", 0);
  JsVar *addr = jsvObjectGetChild(device, "id", 0);
  // Convert mac address to something readable
  ble_gap_addr_t peer_addr;
  if (!bleVarToAddr(addr, &peer_addr)) {
    jsvUnLock2(device, addr);
    jsExceptionHere(JSET_TYPEERROR, "Expecting a device with a mac address of the form aa:bb:cc:dd:ee:ff");
    return 0;
  }
  jsvUnLock(device);

  JsVar *promise = 0;
  if (bleNewTask(BLETASK_CONNECT, parent/*BluetoothRemoteGATTServer*/)) {
    JsVar *fn = jsvNewNativeFunction((void (*)(void))_jswrap_nrf_bluetooth_central_connect, JSWAT_VOID|(JSWAT_JSVAR<<JSWAT_BITS));
    if (fn) {
      jsiQueueEvents(0, fn, &addr, 1);
      jsvUnLock(fn);
      promise = jsvLockAgainSafe(blePromise);
    }
  }
  jsvUnLock(addr);
  return promise;
#else
  jsExceptionHere(JSET_ERROR, "Unimplemented");
  return 0;
#endif
}

/*JSON{
  "type" : "class",
  "class" : "BluetoothRemoteGATTServer",
    "ifdef" : "NRF52,ESP32"
}
Web Bluetooth-style GATT server - get this using `NRF.connect(address)`
or `NRF.requestDevice(options)` and `response.gatt.connect`

https://webbluetoothcg.github.io/web-bluetooth/#bluetoothremotegattserver
*/
/*JSON{
    "type" : "method",
    "class" : "BluetoothRemoteGATTServer",
    "name" : "disconnect",
    "generate" : "jswrap_BluetoothRemoteGATTServer_disconnect",
    "ifdef" : "NRF52,ESP32"
}
Disconnect from a previously connected BLE device connected with
`NRF.connect` - this does not disconnect from something that has
connected to the Espruino.
*/
void jswrap_BluetoothRemoteGATTServer_disconnect(JsVar *parent) {
#if CENTRAL_LINK_COUNT>0
  uint32_t              err_code;

  if (m_central_conn_handle != BLE_CONN_HANDLE_INVALID) {
    // we have a connection, disconnect
    err_code = jsble_disconnect(m_central_conn_handle);
    jsble_check_error(err_code);
  } else {
    // no connection - try and cancel the connect attempt (assume we have one)
#ifdef NRF52
    err_code = sd_ble_gap_connect_cancel();
#endif
#ifdef ESP32
    jsWarn("connect cancel not implemented yet\n");
#endif
    // maybe we don't, in which case we don't care about the error code
  }
#else
  jsExceptionHere(JSET_ERROR, "Unimplemented");
#endif
}

/*JSON{
    "type" : "method",
    "class" : "BluetoothRemoteGATTServer",
    "name" : "startBonding",
    "ifdef" : "NRF52",
    "generate" : "jswrap_nrf_BluetoothRemoteGATTServer_startBonding",
    "params" : [
      ["forceRePair","bool","If the device is already bonded, re-pair it"]
    ],
    "return" : ["JsVar", "A Promise that is resolved (or rejected) when the bonding is complete" ]
}
Start negotiating bonding (secure communications) with the connected device,
and return a Promise that is completed on success or failure.

```
var gatt;
NRF.requestDevice({ filters: [{ name: 'Puck.js abcd' }] }).then(function(device) {
  console.log("found device");
  return device.gatt.connect();
}).then(function(g) {
  gatt = g;
  console.log("connected");
  return gatt.startBonding();
}).then(function() {
  console.log("bonded", gatt.getSecurityStatus());
  gatt.disconnect();
}).catch(function(e) {
  console.log("ERROR",e);
});
```

**This is not part of the Web Bluetooth Specification.** It has been added
specifically for Puck.js.
*/
JsVar *jswrap_nrf_BluetoothRemoteGATTServer_startBonding(JsVar *parent, bool forceRePair) {
#if CENTRAL_LINK_COUNT>0
  if (bleNewTask(BLETASK_BONDING, parent/*BluetoothRemoteGATTServer*/)) {
    JsVar *promise = jsvLockAgainSafe(blePromise);
    jsble_central_startBonding(forceRePair);
    return promise;
  }
  return 0;
#else
  jsExceptionHere(JSET_ERROR, "Unimplemented");
  return 0;
#endif
}


/*JSON{
    "type" : "method",
    "class" : "BluetoothRemoteGATTServer",
    "name" : "getSecurityStatus",
    "ifdef" : "NRF52",
    "generate" : "jswrap_nrf_BluetoothRemoteGATTServer_getSecurityStatus",
    "return" : ["JsVar", "An object" ]
}
Return an object with information about the security
state of the current connection:


```
{
  connected       // The connection is active (not disconnected).
  encrypted       // Communication on this link is encrypted.
  mitm_protected  // The encrypted communication is also protected against man-in-the-middle attacks.
  bonded          // The peer is bonded with us
}
```

See `BluetoothRemoteGATTServer.startBonding` for information about
negotiating a secure connection.

**This is not part of the Web Bluetooth Specification.** It has been added
specifically for Puck.js.
*/
JsVar *jswrap_nrf_BluetoothRemoteGATTServer_getSecurityStatus(JsVar *parent) {
#if CENTRAL_LINK_COUNT>0
  return jsble_central_getSecurityStatus();
#else
  jsExceptionHere(JSET_ERROR, "Unimplemented");
  return 0;
#endif
}

/*JSON{
  "type" : "method",
  "class" : "BluetoothRemoteGATTServer",
  "name" : "getPrimaryService",
  "generate" : "jswrap_BluetoothRemoteGATTServer_getPrimaryService",
  "params" : [ ["service","JsVar","The service UUID"] ],
  "return" : ["JsVar", "A Promise that is resolved (or rejected) when the primary service is found (the argument contains a `BluetoothRemoteGATTService`)" ],
  "ifdef" : "NRF52,ESP32"
}
See `NRF.connect` for usage examples.
*/
JsVar *jswrap_BluetoothRemoteGATTServer_getPrimaryService(JsVar *parent, JsVar *service) {
#if CENTRAL_LINK_COUNT>0
  const char *err;
  ble_uuid_t uuid;

  if (!bleNewTask(BLETASK_PRIMARYSERVICE, 0))
    return 0;

  err = bleVarToUUID(&uuid, service);
  if (err) {
    jsExceptionHere(JSET_ERROR, "%s", err);
    return 0;
  }

  JsVar *promise = jsvLockAgainSafe(blePromise);
  jsble_central_getPrimaryServices(uuid);
  return promise;
#else
  jsExceptionHere(JSET_ERROR, "Unimplemented");
  return 0;
#endif
}
/*JSON{
  "type" : "method",
  "class" : "BluetoothRemoteGATTServer",
  "name" : "getPrimaryServices",
  "generate" : "jswrap_BluetoothRemoteGATTServer_getPrimaryServices",
  "return" : ["JsVar", "A Promise that is resolved (or rejected) when the primary services are found (the argument contains an array of `BluetoothRemoteGATTService`)" ],
  "ifdef" : "NRF52,ESP32"
}
*/
JsVar *jswrap_BluetoothRemoteGATTServer_getPrimaryServices(JsVar *parent) {
#if CENTRAL_LINK_COUNT>0
  ble_uuid_t uuid;
  uuid.type = BLE_UUID_TYPE_UNKNOWN;

  if (!bleNewTask(BLETASK_PRIMARYSERVICE, 0))
    return 0;
  JsVar *promise = jsvLockAgainSafe(blePromise);
  jsble_central_getPrimaryServices(uuid);
  return promise;
#else
  jsExceptionHere(JSET_ERROR, "Unimplemented");
  return 0;
#endif
}

/*JSON{
  "type" : "method",
  "class" : "BluetoothRemoteGATTServer",
  "name" : "setRSSIHandler",
  "generate" : "jswrap_BluetoothRemoteGATTServer_setRSSIHandler",
  "params" : [
    ["callback","JsVar","The callback to call with the RSSI value, or undefined to stop"]
  ],
  "ifdef" : "NRF52,ESP32"
}

Start/stop listening for RSSI values on the active GATT connection

```
// Start listening for RSSI value updates
gattServer.setRSSIHandler(function(rssi) {
  console.log(rssi); // prints -85 (or similar)
});
// Stop listening
gattServer.setRSSIHandler();
```

RSSI is the 'Received Signal Strength Indication' in dBm

*/
void jswrap_BluetoothRemoteGATTServer_setRSSIHandler(JsVar *parent, JsVar *callback) {
#if CENTRAL_LINK_COUNT>0
  // set the callback event variable
  if (!jsvIsFunction(callback)) callback=0;
  jsvObjectSetChild(parent, BLE_RSSI_EVENT, callback);
  // either start or stop scanning
  uint32_t err_code = jsble_set_central_rssi_scan(callback != 0);
  jsble_check_error(err_code);
#else
  jsExceptionHere(JSET_ERROR, "Unimplemented");
  return;
#endif
}

/*JSON{
  "type" : "class",
  "class" : "BluetoothRemoteGATTService",
  "ifdef" : "NRF52,ESP32"
}
Web Bluetooth-style GATT service - get this using `BluetoothRemoteGATTServer.getPrimaryService(s)`

https://webbluetoothcg.github.io/web-bluetooth/#bluetoothremotegattservice
*/
/*JSON{
  "type" : "method",
  "class" : "BluetoothRemoteGATTService",
  "name" : "getCharacteristic",
  "generate" : "jswrap_BluetoothRemoteGATTService_getCharacteristic",
  "params" : [ ["characteristic","JsVar","The characteristic UUID"] ],
  "return" : ["JsVar", "A Promise that is resolved (or rejected) when the characteristic is found (the argument contains a `BluetoothRemoteGATTCharacteristic`)" ],
  "ifdef" : "NRF52,ESP32"
}
See `NRF.connect` for usage examples.
*/
JsVar *jswrap_BluetoothRemoteGATTService_getCharacteristic(JsVar *parent, JsVar *characteristic) {
#if CENTRAL_LINK_COUNT>0
  const char *err;
  ble_uuid_t uuid;

  if (!bleNewTask(BLETASK_CHARACTERISTIC, 0))
    return 0;

  err = bleVarToUUID(&uuid, characteristic);
  if (err) {
    jsExceptionHere(JSET_ERROR, "%s", err);
    return 0;
  }

  JsVar *promise = jsvLockAgainSafe(blePromise);
  jsble_central_getCharacteristics(parent, uuid);
  return promise;
#else
  jsExceptionHere(JSET_ERROR, "Unimplemented");
  return 0;
#endif
}
/*JSON{
  "type" : "method",
  "class" : "BluetoothRemoteGATTService",
  "name" : "getCharacteristics",
  "generate" : "jswrap_BluetoothRemoteGATTService_getCharacteristics",
  "return" : ["JsVar", "A Promise that is resolved (or rejected) when the characteristic is found (the argument contains an array of `BluetoothRemoteGATTCharacteristic`)" ],
  "ifdef" : "NRF52,ESP32"
}
*/
JsVar *jswrap_BluetoothRemoteGATTService_getCharacteristics(JsVar *parent) {
#if CENTRAL_LINK_COUNT>0
  ble_uuid_t uuid;
  uuid.type = BLE_UUID_TYPE_UNKNOWN;

  if (!bleNewTask(BLETASK_CHARACTERISTIC, 0))
    return 0;

  JsVar *promise = jsvLockAgainSafe(blePromise);
  jsble_central_getCharacteristics(parent, uuid);
  return promise;
#else
  jsExceptionHere(JSET_ERROR, "Unimplemented");
  return 0;
#endif
}

/*JSON{
  "type" : "class",
  "class" : "BluetoothRemoteGATTCharacteristic",
  "ifdef" : "NRF52,ESP32"
}
Web Bluetooth-style GATT characteristic - get this using `BluetoothRemoteGATTService.getCharacteristic(s)`

https://webbluetoothcg.github.io/web-bluetooth/#bluetoothremotegattcharacteristic
*/
/*JSON{
    "type" : "method",
    "class" : "BluetoothRemoteGATTCharacteristic",
    "name" : "writeValue",
    "generate" : "jswrap_nrf_BluetoothRemoteGATTCharacteristic_writeValue",
    "params" : [
      ["data","JsVar","The data to write"]
    ],
    "return" : ["JsVar", "A Promise that is resolved (or rejected) when the characteristic is written" ]
}

Write a characteristic's value

```
var device;
NRF.connect(device_address).then(function(d) {
  device = d;
  return d.getPrimaryService("service_uuid");
}).then(function(s) {
  console.log("Service ",s);
  return s.getCharacteristic("characteristic_uuid");
}).then(function(c) {
  return c.writeValue("Hello");
}).then(function(d) {
  device.disconnect();
}).catch(function() {
  console.log("Something's broken.");
});
```
*/
JsVar *jswrap_nrf_BluetoothRemoteGATTCharacteristic_writeValue(JsVar *characteristic, JsVar *data) {
#if CENTRAL_LINK_COUNT>0
  JSV_GET_AS_CHAR_ARRAY(dataPtr, dataLen, data);
  if (!dataPtr) return 0;

  if (!bleNewTask(BLETASK_CHARACTERISTIC_WRITE, 0))
    return 0;

  JsVar *promise = jsvLockAgainSafe(blePromise);
  jsble_central_characteristicWrite(characteristic, dataPtr, dataLen);
  return promise;
#else
  jsExceptionHere(JSET_ERROR, "Unimplemented");
  return 0;
#endif
}
/*JSON{
    "type" : "method",
    "class" : "BluetoothRemoteGATTCharacteristic",
    "name" : "readValue",
    "generate" : "jswrap_nrf_BluetoothRemoteGATTCharacteristic_readValue",
    "return" : ["JsVar", "A Promise that is resolved (or rejected) with a `DataView` when the characteristic is read" ],
    "ifdef" : "NRF52,ESP32"
}

Read a characteristic's value, return a promise containing a `DataView`

```
var device;
NRF.connect(device_address).then(function(d) {
  device = d;
  return d.getPrimaryService("service_uuid");
}).then(function(s) {
  console.log("Service ",s);
  return s.getCharacteristic("characteristic_uuid");
}).then(function(c) {
  return c.readValue();
}).then(function(d) {
  console.log("Got:", JSON.stringify(d.buffer));
  device.disconnect();
}).catch(function() {
  console.log("Something's broken.");
});
```
*/
JsVar *jswrap_nrf_BluetoothRemoteGATTCharacteristic_readValue(JsVar *characteristic) {
#if CENTRAL_LINK_COUNT>0
  if (!bleNewTask(BLETASK_CHARACTERISTIC_READ, characteristic))
    return 0;

  JsVar *promise = jsvLockAgainSafe(blePromise);
  jsble_central_characteristicRead(characteristic);
  return promise;
#else
  jsExceptionHere(JSET_ERROR, "Unimplemented");
  return 0;
#endif
}

/*JSON{
    "type" : "method",
    "class" : "BluetoothRemoteGATTCharacteristic",
    "name" : "startNotifications",
    "generate" : "jswrap_nrf_BluetoothRemoteGATTCharacteristic_startNotifications",
    "return" : ["JsVar", "A Promise that is resolved (or rejected) with data when notifications have been added" ],
    "ifdef" : "NRF52"
}
Starts notifications - whenever this characteristic's value changes, a `characteristicvaluechanged` event is fired.

```
var device;
NRF.connect(device_address).then(function(d) {
  device = d;
  return d.getPrimaryService("service_uuid");
}).then(function(s) {
  console.log("Service ",s);
  return s.getCharacteristic("characteristic_uuid");
}).then(function(c) {
  c.on('characteristicvaluechanged', function(event) {
    console.log("-> "+event.target.value);
  });
  return c.startNotifications();
}).then(function(d) {
  console.log("Waiting for notifications"));
}).catch(function() {
  console.log("Something's broken.");
});
```

For example, to listen to the output of another Puck.js's Nordic
Serial port service, you can use:

```
var gatt;
NRF.connect("pu:ck:js:ad:dr:es random").then(function(g) {
  gatt = g;
  return gatt.getPrimaryService("6e400001-b5a3-f393-e0a9-e50e24dcca9e");
}).then(function(service) {
  return service.getCharacteristic("6e400003-b5a3-f393-e0a9-e50e24dcca9e");
}).then(function(characteristic) {
  characteristic.on('characteristicvaluechanged', function(event) {
    console.log("RX: "+JSON.stringify(event.target.value.buffer));
  });
  return characteristic.startNotifications();
}).then(function() {
  console.log("Done!");
});
```
*/
JsVar *jswrap_nrf_BluetoothRemoteGATTCharacteristic_startNotifications(JsVar *characteristic) {
#if CENTRAL_LINK_COUNT>0
  
  // Set our characteristic's handle up in the list of handles to notify for
  // TODO: What happens when we close the connection and re-open another?
  uint16_t handle = (uint16_t)jsvGetIntegerAndUnLock(jsvObjectGetChild(characteristic, "handle_value", 0));
  JsVar *handles = jsvObjectGetChild(execInfo.hiddenRoot, "bleHdl", JSV_ARRAY);
  if (handles) {
    jsvSetArrayItem(handles, handle, characteristic);
    jsvUnLock(handles);
  }
  
  JsVar *promise;
  
  // Check for existing cccd_handle 
  uint16_t cccd = (uint16_t)jsvGetIntegerAndUnLock(jsvObjectGetChild(characteristic,"handle_cccd", 0));
  if ( !cccd ) {
    if (!bleNewTask(BLETASK_CHARACTERISTIC_DESC_AND_STARTNOTIFY, characteristic))
      return 0;
    promise = jsvLockAgainSafe(blePromise);
    jsble_central_characteristicDescDiscover(characteristic);
  }
  else {
    if (!bleNewTask(BLETASK_CHARACTERISTIC_NOTIFY, 0))
      return 0;
    promise = jsvLockAgainSafe(blePromise);    
    jsble_central_characteristicNotify(characteristic, true);
  }
  return promise;
#else
  jsExceptionHere(JSET_ERROR, "Unimplemented");
  return 0;
#endif
}

/*JSON{
    "type" : "method",
    "class" : "BluetoothRemoteGATTCharacteristic",
    "name" : "stopNotifications",
    "generate" : "jswrap_nrf_BluetoothRemoteGATTCharacteristic_stopNotifications",
    "return" : ["JsVar", "A Promise that is resolved (or rejected) with data when notifications have been removed" ],
    "ifdef" : "NRF52"
}
Stop notifications (that were requested with `BluetoothRemoteGATTCharacteristic.startNotifications`)
*/
JsVar *jswrap_nrf_BluetoothRemoteGATTCharacteristic_stopNotifications(JsVar *characteristic) {
#if CENTRAL_LINK_COUNT>0
  // Remove our characteristic handle from the list of handles to notify for
  uint16_t handle = (uint16_t)jsvGetIntegerAndUnLock(jsvObjectGetChild(characteristic, "handle_value", 0));
  JsVar *handles = jsvObjectGetChild(execInfo.hiddenRoot, "bleHdl", JSV_ARRAY);
  if (handles) {
    jsvSetArrayItem(handles, handle, 0);
    jsvUnLock(handles);
  }
  JsVar *promise = jsvLockAgainSafe(blePromise);
  jsble_central_characteristicNotify(characteristic, false);
  return promise;
#else
  jsExceptionHere(JSET_ERROR, "Unimplemented");
  return 0;
#endif
}
