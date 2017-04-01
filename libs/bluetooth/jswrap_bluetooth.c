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

#include "nrf5x_utils.h"
#include "bluetooth.h"
#include "bluetooth_utils.h"

#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "nordic_common.h"
#include "nrf.h"
#include "ble_hci.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "softdevice_handler.h"
#include "app_timer.h"
#include "ble_nus.h"
#include "app_util_platform.h"

#ifdef USE_NFC
#include "nfc_uri_msg.h"
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
    jsExceptionHere(JSET_ERROR, "BLE task is already in progress");
    return false;
  }
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

#ifdef USE_NFC
  if (jsiStatus & JSIS_COMPLETELY_RESET) {
#ifdef PUCKJS
    // By default Puck.js's NFC will send you to the PuckJS website
    // address is included so Web Bluetooth can connect to the correct one
    JsVar *addr = jswrap_nrf_bluetooth_getAddress();
    JsVar *uri = jsvVarPrintf("https://puck-js.com/go?a=%v", addr);
    jsvUnLock(addr);
    jswrap_nrf_nfcURL(uri);
    jsvUnLock(uri);
#endif
  } else {
    // start NFC, if it had been set
    JsVar *flatStr = jsvObjectGetChild(execInfo.hiddenRoot, "NFC", 0);
    if (flatStr) {
      uint8_t *flatStrPtr = (uint8_t*)jsvGetFlatStringPointer(flatStr);
      if (flatStrPtr) jsble_nfc_start(flatStrPtr, jsvGetLength(flatStr));
      jsvUnLock(flatStr);
    }
  }
#endif
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

  jsble_reset();
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
  "ifdef" : "NRF52"
}
Called with discovered services when discovery is finished
 */
/*JSON{
  "type" : "event",
  "class" : "NRF",
  "name" : "characteristicsDiscover",
  "ifdef" : "NRF52"
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
var t = getTime();
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
    err_code = sd_ble_gap_disconnect(m_conn_handle,  BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
    jsble_check_error(err_code);
  }
}

/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "sleep",
    "generate" : "jswrap_nrf_bluetooth_sleep"
}
Disable Bluetooth communications
*/
void jswrap_nrf_bluetooth_sleep() {
  uint32_t err_code;

  // If connected, disconnect.
  if (jsble_has_simple_connection()) {
    err_code = sd_ble_gap_disconnect(m_conn_handle,  BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
    jsble_check_error(err_code);
  }

  // Stop advertising
  if (bleStatus & BLE_IS_ADVERTISING)
    jsble_advertising_stop();
  bleStatus |= BLE_IS_SLEEPING;
}

/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "wake",
    "generate" : "jswrap_nrf_bluetooth_wake"
}
Enable Bluetooth communications (they are enabled by default)
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
Get this device's Bluetooth MAC address
*/
JsVar *jswrap_nrf_bluetooth_getAddress() {
  uint32_t addr0 =  NRF_FICR->DEVICEADDR[0];
  uint32_t addr1 =  NRF_FICR->DEVICEADDR[1];
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
Get the battery level in volts
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

Data can be of the form `{ UUID : data_as_byte_array }`. The UUID should be a [Bluetooth Service ID](https://developer.bluetooth.org/gatt/services/Pages/ServicesHome.aspx).

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

**Note:** Currently only standardised bluetooth UUIDs are allowed (see the
list above).

You can also supply the raw advertising data in an array. For example to advertise as an Eddystone beacon:

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

You can even specify an array of arrays, in which case each advertising packet
will be iterated over in turn - for instance to make your device advertise
both Eddystone and iBeacon:

```
NRF.setAdvertising([
[0x03,0x03,0xAA,0xFE,0x13,0x16,0xAA,0xFE,0x10,0xF8,0x03,'g','o','o','.','g','l','/','C','H','o','J','H','0'],
[....],
[....],
],{interval:500});
```

`options` is an object, which can contain:

```
{
  name: "Hello" // The name of the device
  showName: true/false // include full name, or nothing
  discoverable: true/false // general discoverable, or limited - default is limited
  interval: 600 // Advertising interval in msec, between 20 and 10000
}
```
*/
void jswrap_nrf_bluetooth_setAdvertising(JsVar *data, JsVar *options) {
  uint32_t err_code;
  ble_advdata_t advdata;
  jsble_setup_advdata(&advdata);
  bool bleChanged = false;
  bool isAdvertising = bleStatus & BLE_IS_ADVERTISING;

  // Save the current service data
  jsvObjectSetOrRemoveChild(execInfo.hiddenRoot, BLE_NAME_ADVERTISE_DATA, data);
  jsvObjectSetOrRemoveChild(execInfo.hiddenRoot, BLE_NAME_ADVERTISE_OPTIONS, options);

  if (jsvIsObject(options)) {
    JsVar *v;
    v = jsvObjectGetChild(options, "showName", 0);
    if (v) advdata.name_type = jsvGetBoolAndUnLock(v) ?
        BLE_ADVDATA_FULL_NAME :
        BLE_ADVDATA_NO_NAME;

    v = jsvObjectGetChild(options, "discoverable", 0);
    if (v) advdata.flags = jsvGetBoolAndUnLock(v) ?
        BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE :
        BLE_GAP_ADV_FLAGS_LE_ONLY_LIMITED_DISC_MODE;

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

    v = jsvObjectGetChild(options, "name", 0);
    if (v) {
      JSV_GET_AS_CHAR_ARRAY(namePtr, nameLen, v);
      if (namePtr) {
        ble_gap_conn_sec_mode_t sec_mode;
        BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);
        err_code = sd_ble_gap_device_name_set(&sec_mode,
                                              (const uint8_t *)namePtr,
                                              nameLen);
        jsble_check_error(err_code);
        bleChanged = true;
      }
      jsvUnLock(v);
    }
  } else if (!jsvIsUndefined(options)) {
    jsExceptionHere(JSET_TYPEERROR, "Expecting 'options' to be object or undefined, got %t", options);
    return;
  }

  if (jsvIsArray(data) || jsvIsArrayBuffer(data)) {
    // raw data...
    // Check if it's nested arrays - if so we alternate between advertising types
    bleStatus &= ~(BLE_IS_ADVERTISING_MULTIPLE|BLE_ADVERTISING_MULTIPLE_MASK);
    if (jsvIsArray(data)) {
      JsVar *item = jsvGetArrayItem(data, 0);
      if (jsvIsArray(item) || jsvIsArrayBuffer(item)) {
        // nested - enable multiple advertising - start at index 0
        bleStatus |= BLE_IS_ADVERTISING_MULTIPLE;
        // start with the first element
        jsvUnLock(data);
        data = item;
        item = 0;
      } else
        jsvUnLock(item);
    }

    JSV_GET_AS_CHAR_ARRAY(dPtr, dLen, data);
    if (!dPtr) {
      jsExceptionHere(JSET_TYPEERROR, "Unable to convert data argument to an array");
      return;
    }

    if (bleChanged && isAdvertising)
      jsble_advertising_stop();
    err_code = sd_ble_gap_adv_data_set((uint8_t *)dPtr, dLen, NULL, 0);
    jsble_check_error(err_code);
    if (bleChanged && isAdvertising)
      jsble_advertising_start();
    return; // we're done here now
  } else if (jsvIsObject(data)) {
    ble_advdata_service_data_t *service_data = (ble_advdata_service_data_t*)alloca(jsvGetChildren(data)*sizeof(ble_advdata_service_data_t));
    int n = 0;
    JsvObjectIterator it;
    jsvObjectIteratorNew(&it, data);
    while (jsvObjectIteratorHasValue(&it)) {
      service_data[n].service_uuid = jsvGetIntegerAndUnLock(jsvObjectIteratorGetKey(&it));
      JsVar *v = jsvObjectIteratorGetValue(&it);
      JSV_GET_AS_CHAR_ARRAY(dPtr, dLen, v);
      jsvUnLock(v);
      service_data[n].data.size    = dLen;
      service_data[n].data.p_data  = (uint8_t*)dPtr;
      jsvObjectIteratorNext(&it);
      n++;
    }
    jsvObjectIteratorFree(&it);

    advdata.service_data_count   = n;
    advdata.p_service_data_array = service_data;
  } else if (!jsvIsUndefined(data)) {
    jsExceptionHere(JSET_TYPEERROR, "Expecting object or undefined, got %t", data);
    return;
  }

  if (bleChanged && isAdvertising)
    jsble_advertising_stop();
  err_code = ble_advdata_set(&advdata, NULL);
  jsble_check_error(err_code);
  if (bleChanged && isAdvertising)
    jsble_advertising_start();
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

    err_code = sd_ble_gap_adv_data_set(NULL, 0, (uint8_t *)dPtr, dLen);
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

  // Save the current service data
  jsvObjectSetOrRemoveChild(execInfo.hiddenRoot, BLE_NAME_SERVICE_DATA, data);
  // Service UUIDs to advertise
  if (advertise) bleStatus|=BLE_NEEDS_SOFTDEVICE_RESTART;
  jsvObjectSetOrRemoveChild(execInfo.hiddenRoot, BLE_NAME_SERVICE_ADVERTISE, advertise);
  jsvUnLock(advertise);

  // work out whether to apply changes
  if (bleStatus & (BLE_SERVICES_WERE_SET|BLE_NEEDS_SOFTDEVICE_RESTART)) {
    jswrap_nrf_bluetooth_restart();
  }
  /* otherwise, we can set the services now, since we're only adding
   * and not changing anything we don't need a restart. */
  jsble_set_services(data);
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

To notify connected clients of a change to the '0xABCD' characteristic in the '0xBCDE' service:
```
NRF.updateServices({
  0xBCDE : {
    0xABCD : {
      value : "World",
      notify: true
    }
  }
});
```
This only works if the characteristic was created with `notify: true` using `setServices`,
otherwise the characteristic will be updated but no notification will be sent.

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

**Note:** See `setServices` for more information
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
                  && (err_code != BLE_ERROR_NO_TX_PACKETS)
                  && (err_code != BLE_ERROR_GATTS_SYS_ATTR_MISSING)) {
                  if (jsble_check_error(err_code))
                    ok = false;
                }
              }
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
`BluetoothDevice` for each advertsing packet

```
// Start scanning
packets=10;
NRF.setScan(function(d) {
  packets--;
  console.log(d); // print packet info
  if (packets<=0)
    NRF.setScan(); // stop scanning
});
```

**Note:** BLE advertising packets can arrive quickly - faster than you'll
be able to print them to the console. It's best only to print a few, or
to use a function like `NRF.findDevices(..)` which will collate a list
of available devices.
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
  JsVar *services = jsvObjectSetChild(device, "services", jsvNewEmptyArray());
  JsVar *data = jsvObjectGetChild(adv, "data", 0);
  if (data) {
    jsvObjectSetChild(device, "data", data);
    JSV_GET_AS_CHAR_ARRAY(dPtr, dLen, data);
    if (dPtr && dLen) {
      if (services) {
        uint32_t i = 0;
        while (i < dLen) {
          uint8_t field_length = dPtr[i];
          uint8_t field_type   = dPtr[i + 1];

          if (field_type == BLE_GAP_AD_TYPE_COMPLETE_LOCAL_NAME) {
            JsVar *s = jsvNewFromEmptyString();
            if (s) {
              jsvAppendStringBuf(s, (char*)&dPtr[i+2], field_length-1);
              jsvObjectSetChildAndUnLock(device, "name", s);
            }
          } else if (field_type == BLE_GAP_AD_TYPE_16BIT_SERVICE_UUID_MORE_AVAILABLE ||
                     field_type == BLE_GAP_AD_TYPE_16BIT_SERVICE_UUID_COMPLETE) {
            for (int svc_idx = 2; svc_idx < field_length + 1; svc_idx += 2) {
              JsVar *s = jsvVarPrintf("%04x", UNALIGNED_UINT16(&dPtr[i+svc_idx]));
              jsvArrayPushAndUnLock(services, s);
            }
          } else if (field_type == BLE_GAP_AD_TYPE_128BIT_SERVICE_UUID_MORE_AVAILABLE ||
                     field_type == BLE_GAP_AD_TYPE_128BIT_SERVICE_UUID_COMPLETE) {
            JsVar *s = bleUUID128ToStr((uint8_t*)&dPtr[i+2]);
            jsvArrayPushAndUnLock(services, s);
          } // or unknown...
          i += field_length + 1;
        }
      }
    }
  }
  jsvUnLock2(data, services);
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
Utility function to return a list of BLE devices detected in range.

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
    "services": [  ],
    "data": new ArrayBuffer([ ... ]),
    "name": "Puck.js 36a2"
   },
  BluetoothDevice {
    "id": "c0:52:3f:50:42:c9 random",
    "rssi": -65,
    "services": [  ],
    "data": new ArrayBuffer([ ... ]),
    "name": "Puck.js 8f57"
   }
 ]
```

You could then use [`BluetoothDevice.gatt.connect(...)`](/Reference#l_BluetoothRemoteGATTServer_connect) on
the device returned, to make a connection.

You can also use [`NRF.connect(...)`](/Reference#l_NRF_connect) on just the `id` string returned, which
may be useful if you always want to connect to a specific device.
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
      found = jsvObjectIteratorGetKey(&it);
    jsvUnLock2(addr, obj);
    jsvObjectIteratorNext(&it);
  }
  jsvObjectIteratorFree(&it);
  if (found) {
    // TODO: merge information?
    jsvSetValueOfName(found, device);
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
  err_code = sd_ble_gap_tx_power_set(pwr);
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

**Note:** This is only available on nRF52-based devices
*/
void jswrap_nrf_nfcURL(JsVar *url) {
#ifdef USE_NFC
  // Check for disabling NFC
  if (jsvIsUndefined(url)) {
    jsvObjectRemoveChild(execInfo.hiddenRoot, "NFC");
    jsble_nfc_stop();
    return;
  }

  if (!jsvIsString(url)) {
    jsExceptionHere(JSET_TYPEERROR, "Expecting a String, got %t", url);
    return;
  }

  uint32_t err_code;
  /* Turn off NFC */
  jsble_nfc_stop();
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

  uint8_t msg_buf[256];
  uint32_t len = sizeof(msg_buf);
  /* Encode URI message into buffer */
  err_code = nfc_uri_msg_encode( uriType, // TODO: could auto-prepend http/etc.
                                 (uint8_t *)urlPtr,
                                 urlLen,
                                 msg_buf,
                                 &len);
  if (err_code)
    return jsExceptionHere(JSET_ERROR, "nfc_uri_msg_encode: NFC error code %d", err_code);

  /* Create a flat string - we need this to store the URI data so it hangs around.
   * Avoid having a static var so we have RAM available if not using NFC */
  JsVar *flatStr = jsvNewFlatStringOfLength(len);
  if (!flatStr)
    return jsExceptionHere(JSET_ERROR, "Unable to create string with URI data in");
  jsvObjectSetChild(execInfo.hiddenRoot, "NFC", flatStr);
  uint8_t *flatStrPtr = (uint8_t*)jsvGetFlatStringPointer(flatStr);
  jsvUnLock(flatStr);
  memcpy(flatStrPtr, msg_buf, len);

  // start nfc properly
  jsble_nfc_start(flatStrPtr, len);
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

**Note:** This is only available on nRF52-based devices
*/
void jswrap_nrf_nfcRaw(JsVar *payload) {
#ifdef USE_NFC
  // Check for disabling NFC
  if (jsvIsUndefined(payload)) {
    jsvObjectRemoveChild(execInfo.hiddenRoot, "NFC");
    jsble_nfc_stop();
    return;
  }

  /* Turn off NFC */
  jsble_nfc_stop();

  JSV_GET_AS_CHAR_ARRAY(dataPtr, dataLen, payload);
  if (!dataPtr || !dataLen)
    return jsExceptionHere(JSET_ERROR, "Unable to get NFC data");

  /* Create a flat string - we need this to store the NFC data so it hangs around.
   * Avoid having a static var so we have RAM available if not using NFC */
  JsVar *flatStr = jsvNewFlatStringOfLength(dataLen);
  if (!flatStr)
    return jsExceptionHere(JSET_ERROR, "Unable to create string with NFC data in");
  jsvObjectSetChild(execInfo.hiddenRoot, "NFC", flatStr);
  uint8_t *flatStrPtr = (uint8_t*)jsvGetFlatStringPointer(flatStr);
  jsvUnLock(flatStr);
  memcpy(flatStrPtr, dataPtr, dataLen);

  // start nfc properly
  jsble_nfc_start(flatStrPtr, dataLen);
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
    "ifdef" : "NRF52",
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
  characteristic.writeValue("LED1.set()\n");
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

**Note:** This is only available on some devices
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
    "ifdef" : "NRF52",
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
  characteristic.writeValue("LED1.set()\n");
}).then(function() {
  gatt.disconnect();
  console.log("Done!");
});
```

**Note:** This is only available on some devices
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
  "type" : "class",
  "class" : "BluetoothDevice",
  "ifdef" : "NRF52"
}
Web Bluetooth-style device - get this using `NRF.requestDevice(address)`
*/
/*JSON{
    "type" : "property",
    "class" : "BluetoothDevice",
    "name" : "gatt",
    "ifdef" : "NRF52",
    "generate" : "jswrap_BluetoothDevice_gatt",
    "return" : ["JsVar", "A `BluetoothRemoteGATTServer` for this device" ]
}

**Note:** This is only available on some devices
*/
JsVar *jswrap_BluetoothDevice_gatt(JsVar *parent) {
  JsVar *gatt = jsvObjectGetChild(parent, "gatt", 0);
  if (gatt) return gatt;

  gatt = jspNewObject(0, "BluetoothRemoteGATTServer");
  jsvObjectSetChild(parent, "gatt", gatt);
  jsvObjectSetChild(gatt, "device", parent);
  jsvObjectSetChildAndUnLock(gatt, "connected", jsvNewFromBool(false));
  return gatt;
}

/*JSON{
    "type" : "method",
    "class" : "BluetoothRemoteGATTServer",
    "name" : "connect",
    "ifdef" : "NRF52",
    "generate" : "jswrap_nrf_BluetoothRemoteGATTServer_connect",
    "return" : ["JsVar", "A Promise that is resolved (or rejected) when the connection is complete" ]
}
Connect to a BLE device - returns a promise,
the argument of which is the `BluetoothRemoteGATTServer` connection.

See [`NRF.requestDevice`](/Reference#l_NRF_requestDevice) for usage examples.

**Note:** This is only available on some devices
*/
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
  jsvUnLock2(device, addr);

  if (bleNewTask(BLETASK_CONNECT, parent/*BluetoothRemoteGATTServer*/)) {
    JsVar *promise = jsvLockAgainSafe(blePromise);
    jsble_central_connect(peer_addr);
    return promise;
  }
  return 0;
#else
  jsExceptionHere(JSET_ERROR, "Unimplemented");
  return 0;
#endif
}

/*JSON{
  "type" : "class",
  "class" : "BluetoothRemoteGATTServer",
  "ifdef" : "NRF52"
}
Web Bluetooth-style GATT server - get this using `NRF.connect(address)`
or `NRF.requestDevice(options)` then `response.gatt.connect`

https://webbluetoothcg.github.io/web-bluetooth/#bluetoothremotegattserver
*/
/*JSON{
    "type" : "method",
    "class" : "BluetoothRemoteGATTServer",
    "name" : "disconnect",
    "generate" : "jswrap_BluetoothRemoteGATTServer_disconnect",
    "ifdef" : "NRF52"
}
Disconnect from a previously connected BLE device connected with
`NRF.connect` - this does not disconnect from something that has
connected to the Espruino.

**Note:** This is only available on some devices
*/
void jswrap_BluetoothRemoteGATTServer_disconnect(JsVar *parent) {
#if CENTRAL_LINK_COUNT>0
  uint32_t              err_code;

  if (m_central_conn_handle != BLE_CONN_HANDLE_INVALID) {
    // we have a connection, disconnect
    err_code = sd_ble_gap_disconnect(m_central_conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
    jsble_check_error(err_code);
  } else {
    // no connection - try and cancel the connect attempt (assume we have one)
    err_code = sd_ble_gap_connect_cancel();
    // maybe we don't, in which case we don't care about the error code
  }
#else
  jsExceptionHere(JSET_ERROR, "Unimplemented");
#endif
}

/*JSON{
  "type" : "method",
  "class" : "BluetoothRemoteGATTServer",
  "name" : "getPrimaryService",
  "generate" : "jswrap_BluetoothRemoteGATTServer_getPrimaryService",
  "params" : [ ["service","JsVar","The service UUID"] ],
  "return" : ["JsVar", "A Promise that is resolved (or rejected) when the primary service is found" ],
  "ifdef" : "NRF52"
}
**Note:** This is only available on some devices
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
  "return" : ["JsVar", "A Promise that is resolved (or rejected) when the primary services are found" ],
  "ifdef" : "NRF52"
}
**Note:** This is only available on some devices
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
  "type" : "class",
  "class" : "BluetoothRemoteGATTService",
  "ifdef" : "NRF52"
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
  "return" : ["JsVar", "A Promise that is resolved (or rejected) when the characteristic is found" ],
  "ifdef" : "NRF52"
}
**Note:** This is only available on some devices
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
  "return" : ["JsVar", "A Promise that is resolved (or rejected) when the characteristic is found" ],
  "ifdef" : "NRF52"
}
**Note:** This is only available on some devices
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
  "ifdef" : "NRF52"
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

**Note:** This is only available on some devices
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
    "return" : ["JsVar", "A Promise that is resolved (or rejected) with a DataView when the characteristic is read" ]
}

Read a characteristic's value, return a promise containing a DataView

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

**Note:** This is only available on some devices
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
    "return" : ["JsVar", "A Promise that is resolved (or rejected) with data when notifications have been added" ]
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

**Note:** This is only available on some devices
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
    "return" : ["JsVar", "A Promise that is resolved (or rejected) with data when notifications have been removed" ]
}
**Note:** This is only available on some devices
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
