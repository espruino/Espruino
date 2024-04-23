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
#include "ble_gap.h"
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
#include "nfc_ble_pair_msg.h"
#include "nfc_launchapp_msg.h"
#endif
#if ESPR_BLUETOOTH_ANCS
#include "bluetooth_ancs.h"
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

#if ESPR_NO_PROMISES!=1
JsVar *blePromise = 0;
#endif
JsVar *bleTaskInfo = 0;
JsVar *bleTaskInfo2 = 0;
BleTask bleTask = BLETASK_NONE;

/// Get the string value of the given task
const char *bleGetTaskString(BleTask task) {
#ifndef SAVE_ON_FLASH_EXTREME
  const char *str = BLETASK_STRINGS; // 0 separated, with two 0s at the end
  while (task && *str) {
    if (!str) return "?";
    str += strlen(str)+1;
    task--;
  }
  if (!*str) return "?";
  return str;
#else
  return "?";
#endif
}

bool bleInTask(BleTask task) {
  return bleTask==task;
}

BleTask bleGetCurrentTask() {
  return bleTask;
}

bool bleNewTask(BleTask task, JsVar *taskInfo) {
  if (bleTask) {
    jsExceptionHere(JSET_ERROR, "BLE task %s is already in progress", bleGetTaskString(bleTask));
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
#if ESPR_NO_PROMISES!=1
  assert(!blePromise && !bleTaskInfo && !bleTaskInfo2);
  blePromise = jspromise_create();
#endif
  bleTask = task;
  bleTaskInfo = jsvLockAgainSafe(taskInfo);
  bleTaskInfo2 = NULL;
  return true;
}

void bleCompleteTask(BleTask task, bool ok, JsVar *data) {
  //jsiConsolePrintf(ok?"RES %d %v\n":"REJ %d %q\n", task, data);
  if (task != bleTask) {
    jsExceptionHere(JSET_INTERNALERROR, "BLE task completed that wasn't scheduled (%s/%s)", bleGetTaskString(task), bleGetTaskString(bleTask));
    return;
  }
  bleTask = BLETASK_NONE;
#if ESPR_NO_PROMISES!=1
  if (blePromise) {
    if (ok) jspromise_resolve(blePromise, data);
    else jspromise_reject(blePromise, data);
    jsvUnLock(blePromise);
    blePromise = 0;
  }
#endif
  jsvUnLock(bleTaskInfo);
  bleTaskInfo = 0;
  jsvUnLock(bleTaskInfo2);
  bleTaskInfo2 = 0;
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
#if CENTRAL_LINK_COUNT>0
void bleSetActiveBluetoothGattServer(int idx, JsVar *var) {
  assert(idx >=0 && idx < CENTRAL_LINK_COUNT);
  if (idx<0) return;
  char name[BLE_NAME_GATT_SERVER_LEN] = BLE_NAME_GATT_SERVER;
  name[BLE_NAME_GATT_SERVER_LEN-2] = '0'+idx;
  jsvObjectSetChild(execInfo.hiddenRoot, name, var);
}

JsVar *bleGetActiveBluetoothGattServer(int idx) {
  assert(idx < CENTRAL_LINK_COUNT);
  if (idx<0) return 0;
  char name[BLE_NAME_GATT_SERVER_LEN] = BLE_NAME_GATT_SERVER;
  name[BLE_NAME_GATT_SERVER_LEN-2] = '0'+idx;
  return jsvObjectGetChildIfExists(execInfo.hiddenRoot, name);
}

uint16_t jswrap_ble_BluetoothRemoteGATTServer_getHandle(JsVar *parent) {
  JsVar *handle = jsvObjectGetChildIfExists(parent, "handle");
  if (!jsvIsInt(handle)) return BLE_CONN_HANDLE_INVALID;
  return jsvGetIntegerAndUnLock(handle);
}
uint16_t jswrap_ble_BluetoothDevice_getHandle(JsVar *parent) {
  JsVar *gatt = jswrap_BluetoothDevice_gatt(parent);
  uint16_t handle = BLE_CONN_HANDLE_INVALID;
  if (gatt) handle = jswrap_ble_BluetoothRemoteGATTServer_getHandle(gatt);
  return handle;
}
uint16_t jswrap_ble_BluetoothRemoteGATTService_getHandle(JsVar *parent) {
  JsVar *device = jsvObjectGetChildIfExists(parent, "device");
  uint16_t handle = BLE_CONN_HANDLE_INVALID;
  if (device) handle = jswrap_ble_BluetoothDevice_getHandle(device);
  return handle;
}
uint16_t jswrap_ble_BluetoothRemoteGATTCharacteristic_getHandle(JsVar *parent) {
  JsVar *service = jsvObjectGetChildIfExists(parent, "service");
  uint16_t handle = BLE_CONN_HANDLE_INVALID;
  if (service) handle = jswrap_ble_BluetoothRemoteGATTService_getHandle(service);
  return handle;
}
#endif
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------

/*JSON{
  "type" : "init",
  "generate" : "jswrap_ble_init"
}*/
void jswrap_ble_init() {
  // Turn off sleeping if it was on before
  jsiStatus &= ~BLE_IS_SLEEPING;


  if (jsiStatus & JSIS_COMPLETELY_RESET) {
#if defined(USE_NFC) && defined(NFC_DEFAULT_URL)
    // By default Puck.js's NFC will send you to the PuckJS website
    // address is included so Web Bluetooth can connect to the correct one
    JsVar *addr = jswrap_ble_getAddress();
    JsVar *uri = jsvVarPrintf(NFC_DEFAULT_URL"?a=%v", addr);
    jsvUnLock(addr);
    jswrap_nfc_URL(uri);
    jsvUnLock(uri);
#endif
  } else {
#ifdef USE_NFC
    // start NFC, if it had been set
    JsVar *flatStr = jsvObjectGetChildIfExists(execInfo.hiddenRoot, "NfcEnabled");
    if (flatStr) {
      uint8_t *flatStrPtr = (uint8_t*)jsvGetFlatStringPointer(flatStr);
      if (flatStrPtr) jsble_nfc_start(flatStrPtr, jsvGetLength(flatStr));
      jsvUnLock(flatStr);
    }
#endif
  }
  // Set advertising interval back to default
  bleAdvertisingInterval = MSEC_TO_UNITS(BLUETOOTH_ADVERTISING_INTERVAL, UNIT_0_625_MS);           /**< The advertising interval (in units of 0.625 ms). */
  // Now set up whatever advertising we were doing before
  jswrap_ble_reconfigure_softdevice();
}

/** Reconfigure the softdevice (on init or after restart) to have all the services/advertising we need */
void jswrap_ble_reconfigure_softdevice() {
  JsVar *v,*o;
  // restart various
  v = jsvObjectGetChildIfExists(execInfo.root, BLE_SCAN_EVENT);
  if (v) jsble_set_scanning(true, NULL);
  jsvUnLock(v);
  v = jsvObjectGetChildIfExists(execInfo.root, BLE_RSSI_EVENT);
  if (v) jsble_set_rssi_scan(true);
  jsvUnLock(v);
  // advertising
  v = jsvObjectGetChildIfExists(execInfo.hiddenRoot, BLE_NAME_ADVERTISE_DATA);
  o = jsvObjectGetChildIfExists(execInfo.hiddenRoot, BLE_NAME_ADVERTISE_OPTIONS);
  jswrap_ble_setAdvertising(v, o);
  jsvUnLock2(v,o);
  // services
  v = jsvObjectGetChildIfExists(execInfo.hiddenRoot, BLE_NAME_SERVICE_DATA);
  jsble_set_services(v);
  jsvUnLock(v);
  // If we had scan response data set, update it
  JsVar *scanData = jsvObjectGetChildIfExists(execInfo.hiddenRoot, BLE_NAME_SCAN_RESPONSE_DATA);
  if (scanData) jswrap_ble_setScanResponse(scanData);
  jsvUnLock(scanData);
  // Set up security related stuff
  jsble_update_security();
}

/*JSON{
  "type" : "idle",
  "generate" : "jswrap_ble_idle"
}*/
bool jswrap_ble_idle() {
  return false;
}

/*JSON{
  "type" : "kill",
  "generate" : "jswrap_ble_kill"
}*/
void jswrap_ble_kill() {
#ifdef USE_NFC
  // stop NFC emulation
  jsble_nfc_stop(); // not a problem to call this if NFC isn't started
#endif
  // stop any BLE tasks
  bleTask = BLETASK_NONE;
#if ESPR_NO_PROMISES!=1
  if (blePromise) jsvUnLock(blePromise);
  blePromise = 0;
#endif
  if (bleTaskInfo) jsvUnLock(bleTaskInfo);
  bleTaskInfo = 0;
  if (bleTaskInfo2) jsvUnLock(bleTaskInfo2);
  bleTaskInfo2 = 0;
  // if we were scanning, make sure we stop
  jsble_set_scanning(false, NULL);
  jsble_set_rssi_scan(false);

#if CENTRAL_LINK_COUNT>0
  // if we were connected to something, disconnect
  for (int i=0;i<CENTRAL_LINK_COUNT;i++)
    if (m_central_conn_handles[i] != BLE_CONN_HANDLE_INVALID)
      jsble_disconnect(m_central_conn_handles[i]);
#endif
}

void jswrap_ble_dumpBluetoothInitialisation(vcbprintf_callback user_callback, void *user_data) {


  JsVar *v,*o;
  v = jsvObjectGetChildIfExists(execInfo.root, BLE_SCAN_EVENT);
  if (v) {
    user_callback("NRF.setScan(", user_data);
    jsiDumpJSON(user_callback, user_data, v, 0);
    user_callback(");\n", user_data);
  }
  jsvUnLock(v);
  v = jsvObjectGetChildIfExists(execInfo.root, BLE_RSSI_EVENT);
  if (v) {
    user_callback("NRF.setRSSIHandler(", user_data);
    jsiDumpJSON(user_callback, user_data, v, 0);
    user_callback(");\n", user_data);
  }
  jsvUnLock(v);
  // advertising
  v = jsvObjectGetChildIfExists(execInfo.hiddenRoot, BLE_NAME_ADVERTISE_DATA);
  o = jsvObjectGetChildIfExists(execInfo.hiddenRoot, BLE_NAME_ADVERTISE_OPTIONS);
  if (v || o)
    cbprintf(user_callback, user_data, "NRF.setAdvertising(%j, %j);\n",v,o);
  jsvUnLock2(v,o);
  // services
  v = jsvObjectGetChildIfExists(execInfo.hiddenRoot, BLE_NAME_SERVICE_DATA);
  o = jsvObjectGetChildIfExists(execInfo.hiddenRoot, BLE_NAME_SERVICE_OPTIONS);
  if (v || o)
    cbprintf(user_callback, user_data, "NRF.setServices(%j, %j);\n",v,o);
  jsvUnLock2(v,o);
  // security
  v = jsvObjectGetChildIfExists(execInfo.hiddenRoot, BLE_NAME_SECURITY);
  if (v)
    cbprintf(user_callback, user_data, "NRF.setSecurity(%j);\n",v);
  jsvUnLock(v);
  // mac address
  v = jsvObjectGetChildIfExists(execInfo.hiddenRoot, BLE_NAME_MAC_ADDRESS);
  if (v)
    cbprintf(user_callback, user_data, "NRF.setAddress(%j);\n",v);
  jsvUnLock(v);
}

// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------

/*JSON{
    "type" : "class",
    "class" : "NRF"
}
The NRF class is for controlling functionality of the Nordic nRF51/nRF52 chips.

Most functionality is related to Bluetooth Low Energy, however there are also
some functions related to NFC that apply to NRF52-based devices.

*/

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
Called when a host device connects to Espruino. The first argument contains the
address.
 */
/*JSON{
  "type" : "event",
  "class" : "NRF",
  "name" : "disconnect",
  "params" : [
    ["reason","int","The reason code reported back by the BLE stack - see Nordic's [`ble_hci.h` file](https://github.com/espruino/Espruino/blob/master/targetlibs/nrf5x_12/components/softdevice/s132/headers/ble_hci.h#L71) for more information"]
  ]
}
Called when a host device disconnects from Espruino.

The most common reason is:
* 19 - `REMOTE_USER_TERMINATED_CONNECTION`
* 22 - `LOCAL_HOST_TERMINATED_CONNECTION`
 */
/*JSON{
  "type" : "event",
  "class" : "NRF",
  "name" : "error",
  "#if" : "defined(NRF52_SERIES)",
  "params" : [
    ["msg","JsVar","The error string"]
  ]
}
Called when the Nordic Bluetooth stack (softdevice) generates an error. In pretty
much all cases an Exception will also have been thrown.
*/
/*JSON{
  "type" : "event",
  "class" : "NRF",
  "name" : "passkey",
  "ifdef" : "NRF52_SERIES",
  "params" : [
    ["passkey","JsVar","A 6 character numeric String to be displayed"]
  ]
}
(Added in 2v19) Called when a central device connects to Espruino, pairs, and sends a passkey that Espruino should display.

For this to be used, you'll have to specify that your device has a display using `NRF.setSecurity({mitm:1, display:1});`

For instance:

```
NRF.setSecurity({mitm:1, display:1});
NRF.on("passkey", key => print("Enter PIN: ",passkey));
```

It is also possible to specify a static passkey with `NRF.setSecurity({passkey:"123456", mitm:1, display:1});`
in which case no `passkey` event handler is needed (this method works on Espruino 2v02 and later)

**Note:** A similar event, [`BluetoothDevice.on("passkey", ...)`](http://www.espruino.com/Reference#l_BluetoothDevice_passkey) is available
for when Espruino is connecting *to* another device (central mode).
*/
/*JSON{
  "type" : "event",
  "class" : "NRF",
  "name" : "security",
  "params" : [
    ["status","JsVar","An object containing `{auth_status,bonded,lv4,kdist_own,kdist_peer}"]
  ]
}
Contains updates on the security of the current Bluetooth link.

See Nordic's `ble_gap_evt_auth_status_t` structure for more information.
*/
/*JSON{
  "type" : "event",
  "class" : "NRF",
  "name" : "advertising",
  "#if" : "defined(NRF52_SERIES)",
  "params" : [
    ["isAdvertising","bool","Whether we are advertising or not"]
  ]
}
Called when Bluetooth advertising starts or stops on Espruino
*/
/*JSON{
  "type" : "event",
  "class" : "NRF",
  "name" : "bond",
  "#if" : "defined(NRF52_SERIES)",
  "params" : [
    ["status","JsVar","One of `'request'/'start'/'success'/'fail'`"]
  ]
}
Called during the bonding process to update on status

`status` is one of:

* `"request"` - Bonding has been requested in code via `NRF.startBonding`
* `"start"` - The bonding procedure has started
* `"success"` - The bonding procedure has succeeded (`NRF.startBonding`'s promise resolves)
* `"fail"` - The bonding procedure has failed (`NRF.startBonding`'s promise rejects)
 */
/*JSON{
  "type" : "event",
  "class" : "NRF",
  "name" : "HID",
  "#if" : "defined(NRF52_SERIES)"
}
Called with a single byte value when Espruino is set up as a HID device and the
computer it is connected to sends a HID report back to Espruino. This is usually
used for handling indications such as the Caps Lock LED.
 */

/*JSON{
  "type" : "event",
  "class" : "NRF",
  "name" : "servicesDiscover",
  "#if" : "defined(NRF52_SERIES) || defined(ESP32)"
}
Called with discovered services when discovery is finished
 */
/*JSON{
  "type" : "event",
  "class" : "NRF",
  "name" : "characteristicsDiscover",
  "#if" : "defined(NRF52_SERIES) || defined(ESP32)"
}
Called with discovered characteristics when discovery is finished
 */


/*JSON{
  "type" : "event",
  "class" : "NRF",
  "name" : "NFCon",
  "#if" : "defined(NRF52_SERIES) && defined(USE_NFC)"
}
Called when an NFC field is detected
 */
/*JSON{
  "type" : "event",
  "class" : "NRF",
  "name" : "NFCoff",
  "#if" : "defined(NRF52_SERIES) && defined(USE_NFC)"
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
  "#if" : "defined(NRF52_SERIES) && defined(USE_NFC)"
}
When NFC is started with `NRF.nfcStart`, this is fired when NFC data is
received. It doesn't get called if NFC is started with `NRF.nfcURL` or
`NRF.nfcRaw`
 */
/*JSON{
  "type" : "event",
  "class" : "BluetoothDevice",
  "name" : "gattserverdisconnected",
  "params" : [
    ["reason","int","The reason code reported back by the BLE stack - see Nordic's `ble_hci.h` file for more information"]
  ],
  "ifdef" : "NRF52_SERIES"
}
Called when the device gets disconnected.

To connect and then print `Disconnected` when the device is disconnected, just
do the following:

```
var gatt;
NRF.connect("aa:bb:cc:dd:ee:ff").then(function(gatt) {
  gatt.device.on('gattserverdisconnected', function(reason) {
    console.log("Disconnected ",reason);
  });
});
```

Or:

```
var gatt;
NRF.requestDevice(...).then(function(device) {
  device.on('gattserverdisconnected', function(reason) {
    console.log("Disconnected ",reason);
  });
});
```
 */
/*JSON{
  "type" : "event",
  "class" : "BluetoothRemoteGATTCharacteristic",
  "name" : "characteristicvaluechanged",
  "ifdef" : "BLUETOOTH"
}
Called when a characteristic's value changes, *after*
`BluetoothRemoteGATTCharacteristic.startNotifications` has been called.

```
  ...
  return service.getCharacteristic("characteristic_uuid");
}).then(function(c) {
  c.on('characteristicvaluechanged', function(event) {
    console.log("-> "+event.target.value);
  });
  return c.startNotifications();
}).then(...
```

The first argument is of the form `{target :
BluetoothRemoteGATTCharacteristic}`, and
`BluetoothRemoteGATTCharacteristic.value` will then contain the new value (as a
DataView).
 */

/*JSON{
  "type" : "object",
  "name" : "Bluetooth",
  "instanceof" : "Serial",
  "ifdef" : "BLUETOOTH"
}
The Bluetooth Serial port - used when data is sent or received over Bluetooth
Smart on nRF51/nRF52 chips.
 */

/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "disconnect",
    "generate" : "jswrap_ble_disconnect"
}
If a device is connected to Espruino, disconnect from it.
*/
void jswrap_ble_disconnect() {
  uint32_t err_code;
  if (jsble_has_peripheral_connection()) {
    err_code = jsble_disconnect(m_peripheral_conn_handle);
    jsble_check_error(err_code);
  }
}

/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "sleep",
    "generate" : "jswrap_ble_sleep"
}
Disable Bluetooth advertising and disconnect from any device that connected to
Puck.js as a peripheral (this won't affect any devices that Puck.js initiated
connections to).

This makes Puck.js undiscoverable, so it can't be connected to.

Use `NRF.wake()` to wake up and make Puck.js connectable again.
*/
void jswrap_ble_sleep() {
  // set as sleeping
  bleStatus |= BLE_IS_SLEEPING;
  // stop advertising
  jsble_advertising_stop();
  // If connected, disconnect.
  // when we disconnect, we'll see BLE_IS_SLEEPING and won't advertise
  jswrap_ble_disconnect();
}

/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "wake",
    "generate" : "jswrap_ble_wake"
}
Enable Bluetooth advertising (this is enabled by default), which allows other
devices to discover and connect to Puck.js.

Use `NRF.sleep()` to disable advertising.
*/
void jswrap_ble_wake() {
  bleStatus &= ~BLE_IS_SLEEPING;
  jsble_check_error(jsble_advertising_start());
}

/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "restart",
    "generate" : "jswrap_ble_restart",
    "params" : [
      ["callback","JsVar","[optional] A function to be called while the softdevice is uninitialised. Use with caution - accessing console/bluetooth will almost certainly result in a crash."]
    ]
}
Restart the Bluetooth softdevice (if there is currently a BLE connection, it
will queue a restart to be done when the connection closes).

You shouldn't need to call this function in normal usage. However, Nordic's BLE
softdevice has some settings that cannot be reset. For example there are only a
certain number of unique UUIDs. Once these are all used the only option is to
restart the softdevice to clear them all out.
*/
void jswrap_ble_restart(JsVar *callback) {
  if (jsble_has_connection()) {
    jsiConsolePrintf("BLE Connected, queueing BLE restart for later\n");
    bleStatus |= BLE_NEEDS_SOFTDEVICE_RESTART;
  } else {
    // Not connected, so we can restart now
    jsble_restart_softdevice(jsvIsFunction(callback)?callback:NULL);
  }
}

/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "eraseBonds",
    "#if" : "defined(NRF52_SERIES)",
    "generate" : "jswrap_ble_eraseBonds",
    "params" : [
      ["callback","JsVar","[optional] A function to be called while the softdevice is uninitialised. Use with caution - accessing console/bluetooth will almost certainly result in a crash."]
    ]
}
Delete all data stored for all peers (bonding data used for secure connections). This cannot be done
while a connection is active, so if there is a connection it will be postponed until everything is disconnected
(which can be done by calling `NRF.disconnect()` and waiting).

Booting your device while holding all buttons down together should also have the same effect.
*/
void jswrap_ble_eraseBonds() {
#if PEER_MANAGER_ENABLED
  if (jsble_has_connection()) {
    jsExceptionHere(JSET_ERROR, "BLE Connected, can't erase bonds.");
  } else {
    jsble_central_eraseBonds();
  }
#endif
}

/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "getAddress",
    "generate" : "jswrap_ble_getAddress",
    "return" : ["JsVar", "MAC address - a string of the form 'aa:bb:cc:dd:ee:ff'" ]
}
Get this device's default Bluetooth MAC address.

For Puck.js, the last 5 characters of this (e.g. `ee:ff`) are used in the
device's advertised Bluetooth name.
*/
JsVar *jswrap_ble_getAddress() {
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
    "name" : "setAddress",
    "#if" : "defined(NRF52_SERIES)",
    "generate" : "jswrap_ble_setAddress",
    "params" : [
      ["addr","JsVar","The address to use (as a string)"]
    ]
}
Set this device's default Bluetooth MAC address:

```
NRF.setAddress("ff:ee:dd:cc:bb:aa random");
```

Addresses take the form:

* `"ff:ee:dd:cc:bb:aa"` or `"ff:ee:dd:cc:bb:aa public"` for a public address
* `"ff:ee:dd:cc:bb:aa random"` for a random static address (the default for
  Espruino)

This may throw a `INVALID_BLE_ADDR` error if the upper two bits of the address
don't match the address type.

To change the address, Espruino must restart the softdevice. It will only do so
when it is disconnected from other devices.
*/
void jswrap_ble_setAddress(JsVar *address) {
#ifdef NRF52_SERIES
  ble_gap_addr_t p_addr;
  if (!bleVarToAddr(address, &p_addr)) {
    jsExceptionHere(JSET_ERROR, "Expecting mac address of the form aa:bb:cc:dd:ee:ff");
    return;
  }
  jsvObjectSetChild(execInfo.hiddenRoot, BLE_NAME_MAC_ADDRESS, address);
  jswrap_ble_restart(NULL);
#else
  jsExceptionHere(JSET_ERROR, "Not implemented");
#endif
}

/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "resolveAddress",
    "#if" : "defined(NRF52_SERIES)",
    "generate" : "jswrap_ble_resolveAddress",
    "params" : [
      ["options" ,"JsVar", "The address that should be resolved."]
    ],
    "return": ["JsVar", "The resolved address, or `undefined` if it couldn't be resolved."]
}
Try to resolve a **bonded** peer's address from a random private resolvable address. If the peer
is not bonded, there will be no IRK and `undefined` will be returned.

A bunch of devices, especially smartphones, implement address randomisation and periodically change
their bluetooth address to prevent being tracked.

If such a device uses a "random private resolvable address", that address is generated
with the help of an identity resolving key (IRK) that is exchanged during bonding.

If we know the IRK of a device, we can check if an address was potentially generated by that device.

The following will check an address against the IRKs of all bonded devices,
and return the actual address of a bonded device if the given address was likely generated using that device's IRK:

```
NRF.on('connect',addr=> {
  // addr could be "aa:bb:cc:dd:ee:ff private-resolvable"
  if (addr.endsWith("private-resolvable")) {
    let resolved = NRF.resolveAddress(addr);
    // resolved is "aa:bb:cc:dd:ee:ff public"
    if (resolved) addr = resolved;
  }
  console.log("Device connected: ", addr);
})
```

You can get the current connection's address using `NRF.getSecurityStatus().connected_addr`,
so can for instance do `NRF.resolveAddress(NRF.getSecurityStatus().connected_addr)`.
*/
JsVar *jswrap_ble_resolveAddress(JsVar *address) {
#if defined(NRF52_SERIES) && PEER_MANAGER_ENABLED==1
  return jsble_resolveAddress(address);
#else
  jsExceptionHere(JSET_ERROR, "Not implemented");
  return 0;
#endif
}

/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "getBattery",
    "generate" : "jswrap_ble_getBattery",
    "return" : ["float", "Battery level in volts" ]
}
Get the battery level in volts (the voltage that the NRF chip is running off
of).

This is the battery level of the device itself - it has nothing to with any
device that might be connected.
*/
JsVarFloat jswrap_ble_getBattery() {
  return jshReadVRef();
}

/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "setAdvertising",
    "generate" : "jswrap_ble_setAdvertising",
    "params" : [
      ["data","JsVar","The service data to advertise as an object - see below for more info"],
      ["options","JsVar","[optional] Object of options"]
    ]
}
Change the data that Espruino advertises.

Data can be of the form `{ UUID : data_as_byte_array }`. The UUID should be a
[Bluetooth Service
ID](https://developer.bluetooth.org/gatt/services/Pages/ServicesHome.aspx).

For example to return battery level at 95%, do:

```
NRF.setAdvertising({
  0x180F : [95] // Service data 0x180F = 95
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

If you specify a value for the object key, Service Data is advertised. However
if you specify `undefined`, the Service UUID is advertised:

```
NRF.setAdvertising({
  0x180D : undefined // Advertise service UUID 0x180D (HRM)
});
```

Service UUIDs can also be supplied in the second argument of `NRF.setServices`,
but those go in the scan response packet.

You can also supply the raw advertising data in an array. For example to
advertise as an Eddystone beacon:

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

(However for Eddystone we'd advise that you use the [Espruino Eddystone
library](/Puck.js+Eddystone))

**Note:** When specifying data as an array, certain advertising options such as
`discoverable` and `showName` won't have any effect.

**Note:** The size of Bluetooth LE advertising packets is limited to 31 bytes.
If you want to advertise more data, consider using an array for `data` (See
below), or `NRF.setScanResponse`.

You can even specify an array of arrays or objects, in which case each
advertising packet will be used in turn - for instance to make your device
advertise battery level and its name as well as both Eddystone and iBeacon :

```
NRF.setAdvertising([
  {0x180F : [E.getBattery()]}, // normal advertising, with battery %
  require("ble_ibeacon").get(...), // iBeacon
  require("ble_eddystone").get(...), // eddystone
], {interval:300});
```

`options` is an object, which can contain:

```
{
  name: "Hello"              // The name of the device
  showName: true/false       // include full name, or nothing
  discoverable: true/false   // general discoverable, or limited - default is limited
  connectable: true/false    // whether device is connectable - default is true
  scannable : true/false     // whether device can be scanned for scan response packets - default is true
  whenConnected : true/false // keep advertising when connected (nRF52 only)
                             // switches to advertising as non-connectable when it is connected
  interval: 600              // Advertising interval in msec, between 20 and 10000 (default is 375ms)
  manufacturer: 0x0590       // IF sending manufacturer data, this is the manufacturer ID
  manufacturerData: [...]    // IF sending manufacturer data, this is an array of data
  phy: "1mbps/2mbps/coded"   // (NRF52833/NRF52840 only) use the long-range coded phy for transmission (1mbps default)
}
```

Setting `connectable` and `scannable` to false gives the lowest power
consumption as the BLE radio doesn't have to listen after sending advertising.

**NOTE:** Non-`connectable` advertising can't have an advertising interval less
than 100ms according to the BLE spec.

So for instance to set the name of Puck.js without advertising any other data
you can just use the command:

```
NRF.setAdvertising({},{name:"Hello"});
```

You can also specify 'manufacturer data', which is another form of advertising
data. We've registered the Manufacturer ID 0x0590 (as Pur3 Ltd) for use with
*Official Espruino devices* - use it to advertise whatever data you'd like, but
we'd recommend using JSON.

For example by not advertising a device name you can send up to 24 bytes of JSON
on Espruino's manufacturer ID:

```
var data = {a:1,b:2};
NRF.setAdvertising({},{
  showName:false,
  manufacturer:0x0590,
  manufacturerData:JSON.stringify(data)
});
```

If you're using [EspruinoHub](https://github.com/espruino/EspruinoHub) then it
will automatically decode this into the following MQTT topics:

* `/ble/advertise/ma:c_:_a:dd:re:ss/espruino` -> `{"a":10,"b":15}`
* `/ble/advertise/ma:c_:_a:dd:re:ss/a` -> `1`
* `/ble/advertise/ma:c_:_a:dd:re:ss/b` -> `2`

Note that **you only have 24 characters available for JSON**, so try to use the
shortest field names possible and avoid floating point values that can be very
long when converted to a String.
*/
void jswrap_ble_setAdvertising(JsVar *data, JsVar *options) {
  uint32_t err_code = 0;
  bool isAdvertising = bleStatus & BLE_IS_ADVERTISING;

  if (jsvIsObject(options)) {
    JsVar *v;

    v = jsvObjectGetChildIfExists(options, "interval");
    if (v) {
      uint16_t new_advertising_interval = MSEC_TO_UNITS(jsvGetIntegerAndUnLock(v), UNIT_0_625_MS);
      if (new_advertising_interval<0x0020) new_advertising_interval=0x0020;
      if (new_advertising_interval>0x4000) new_advertising_interval=0x4000;
      if (new_advertising_interval != bleAdvertisingInterval) {
        bleAdvertisingInterval = new_advertising_interval;
      }
    }

    v = jsvObjectGetChildIfExists(options, "connectable");
    if (v) {
      if (jsvGetBoolAndUnLock(v)) bleStatus &= ~BLE_IS_NOT_CONNECTABLE;
      else bleStatus |= BLE_IS_NOT_CONNECTABLE;
    }
    v = jsvObjectGetChildIfExists(options, "scannable");
    if (v) {
      if (jsvGetBoolAndUnLock(v)) bleStatus &= ~BLE_IS_NOT_SCANNABLE;
      else bleStatus |= BLE_IS_NOT_SCANNABLE;
    }
#ifndef SAVE_ON_FLASH
    v = jsvObjectGetChildIfExists(options, "whenConnected");
    if (v) {
      if (jsvGetBoolAndUnLock(v)) {
        bleStatus |= BLE_ADVERTISE_WHEN_CONNECTED;
        if (jsble_has_peripheral_connection()) // if we're connected now, start advertising
          isAdvertising = true;
      } else bleStatus &= ~BLE_ADVERTISE_WHEN_CONNECTED;
    }
#endif

    v = jsvObjectGetChildIfExists(options, "name");
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
      }
      jsvUnLock(v);
    }
  } else if (!jsvIsUndefined(options)) {
    jsExceptionHere(JSET_TYPEERROR, "Expecting Object or undefined, got %t", options);
    return;
  }

  JsVar *advArray = 0;

  if (jsvIsObject(data) || jsvIsUndefined(data)) {
    // if it's an object, work out what the advertising data for it is
    // We still call this even for undefined as it does set some global parameters too unfortunately
    advArray = jswrap_ble_getAdvertisingData(data, options);
    // if undefined, make sure we *save* undefined
    if (jsvIsUndefined(data)) {
      jsvUnLock(advArray);
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
        JsVar *newv = jswrap_ble_getAdvertisingData(v, options);
        jsvObjectIteratorSetValue(&it, newv);
        jsvUnLock(newv);
        isNested = true;
      } else if (jsvIsArray(v) || jsvIsArrayBuffer(v)) {
        isNested = true;
        if (jsvIsArray(v)) {
          /* don't store sparse arrays for advertising data. It's inefficient but also
          in SWI1_IRQHandler they need decoding which is slow *and* will cause jsvNew... to be
          called, which may interfere with what happens in the main thread (eg. GC).
          Instead convert them to ArrayBuffers */
          uint8_t advdata[BLE_GAP_ADV_MAX_SIZE];
          unsigned int advdatalen = jsvIterateCallbackToBytes(v, advdata, BLE_GAP_ADV_MAX_SIZE);
          JsVar *newv = jsvNewArrayBufferWithData(advdatalen, advdata);
          jsvObjectIteratorSetValue(&it, newv);
          jsvUnLock(newv);
        }
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
    }
  } else if (jsvIsArrayBuffer(data)) {
    // it's just data - no multiple advertising
    advArray = jsvLockAgain(data);
    bleStatus &= ~(BLE_IS_ADVERTISING_MULTIPLE|BLE_ADVERTISING_MULTIPLE_MASK);
  } else
    jsExceptionHere(JSET_TYPEERROR, "Expecting Array or Object, got %t", data);
  // Save the current service data
  jsvObjectSetOrRemoveChild(execInfo.hiddenRoot, BLE_NAME_ADVERTISE_DATA, advArray);
  jsvObjectSetOrRemoveChild(execInfo.hiddenRoot, BLE_NAME_ADVERTISE_OPTIONS, options);
  jsvUnLock(advArray);
  // now actually update advertising
  if (isAdvertising)
    jsble_advertising_stop();
#ifdef ESP32
  err_code = bluetooth_gap_setAdvertising(advArray);
#endif
  jsble_check_error(err_code);
  if (isAdvertising)
    jsble_check_error(jsble_advertising_start()); // sets up advertising data again
}

/// Used by bluetooth.c internally when it needs to set up advertising at first
JsVar *jswrap_ble_getCurrentAdvertisingData() {
  // This is safe if JS not initialised, jsvObjectGetChild returns 0
  JsVar *adv = jsvObjectGetChildIfExists(execInfo.hiddenRoot, BLE_NAME_ADVERTISE_DATA);
  // we may not even have started the JS interpreter yet!
  if (!adv && execInfo.root) adv = jswrap_ble_getAdvertisingData(NULL, NULL); // use the defaults
  else {
    if (bleStatus&BLE_IS_ADVERTISING_MULTIPLE) {
      int idx = (bleStatus&BLE_ADVERTISING_MULTIPLE_MASK)>>BLE_ADVERTISING_MULTIPLE_SHIFT;
      JsVar *v = jsvGetArrayItem(adv, idx);
      jsvUnLock(adv);
      adv = v;
    }
  }
  return adv;
}

/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "getAdvertisingData",
    "generate" : "jswrap_ble_getAdvertisingData",
    "params" : [
      ["data","JsVar","The data to advertise as an object"],
      ["options","JsVar","[optional] An object of options"]
    ],
    "return" : ["JsVar", "An array containing the advertising data" ]
}
This is just like `NRF.setAdvertising`, except instead of advertising the data,
it returns the packet that would be advertised as an array.
*/
JsVar *jswrap_ble_getAdvertisingData(JsVar *data, JsVar *options) {
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
    v = jsvObjectGetChildIfExists(options, "showName");
    if (v) advdata.name_type = jsvGetBoolAndUnLock(v) ?
        BLE_ADVDATA_FULL_NAME :
        BLE_ADVDATA_NO_NAME;

    v = jsvObjectGetChildIfExists(options, "discoverable");
    if (v) advdata.flags = jsvGetBoolAndUnLock(v) ?
        BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE :
        BLE_GAP_ADV_FLAGS_LE_ONLY_LIMITED_DISC_MODE;

    v = jsvObjectGetChildIfExists(options, "manufacturerData");
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
    v = jsvObjectGetChildIfExists(options, "manufacturer");
    if (v) {
      if (advdata.p_manuf_specific_data)
        advdata.p_manuf_specific_data->company_identifier = jsvGetInteger(v);
      else
        jsExceptionHere(JSET_TYPEERROR, "'manufacturer' specified without 'manufacturerdata'");
      jsvUnLock(v);
    }
#endif
  } else if (!jsvIsUndefined(options)) {
    jsExceptionHere(JSET_TYPEERROR, "Expecting Object or undefined, got %t", options);
    return 0;
  }

  if (jsvIsArray(data) || jsvIsArrayBuffer(data)) {
    return jsvLockAgain(data);
  } else if (jsvIsObject(data)) {
#ifdef NRF5X
    // we may not use all of service_data/adv_uuids - but allocate the max we can
    int maxServices = jsvGetChildren(data);
    ble_advdata_service_data_t *service_data = (ble_advdata_service_data_t*)alloca(maxServices*sizeof(ble_advdata_service_data_t));
    int service_data_cnt = 0;
    ble_uuid_t *adv_uuid = (ble_uuid_t*)alloca(maxServices*sizeof(ble_uuid_t));
    int adv_uuid_cnt = 0;
    if (maxServices && (!service_data || !adv_uuid))
      return 0; // allocation error
#endif
    JsvObjectIterator it;
    jsvObjectIteratorNew(&it, data);
    while (jsvObjectIteratorHasValue(&it)) {
      JsVar *v = jsvObjectIteratorGetValue(&it);
      JSV_GET_AS_CHAR_ARRAY(dPtr, dLen, v);
      const char *errorStr;
      ble_uuid_t ble_uuid;
      if ((errorStr=bleVarToUUIDAndUnLock(&ble_uuid, jsvObjectIteratorGetKey(&it)))) {
        jsExceptionHere(JSET_ERROR, "Invalid Service UUID: %s", errorStr);
        break;
      }
#ifdef NRF5X
      if (jsvIsUndefined(v)) {
        adv_uuid[adv_uuid_cnt]  = ble_uuid;
        adv_uuid_cnt++;
      } else {
        service_data[service_data_cnt].service_uuid = ble_uuid.uuid;
        service_data[service_data_cnt].data.size    = dLen;
        service_data[service_data_cnt].data.p_data  = (uint8_t*)dPtr;
        service_data_cnt++;
      }
#endif
      jsvUnLock(v);
      jsvObjectIteratorNext(&it);
    }
    jsvObjectIteratorFree(&it);
#ifdef NRF5X
    advdata.service_data_count   = service_data_cnt;
    advdata.p_service_data_array = service_data;
    advdata.uuids_complete.uuid_cnt = adv_uuid_cnt;
    advdata.uuids_complete.p_uuids  = adv_uuid;
#endif
  } else if (!jsvIsUndefined(data)) {
    jsExceptionHere(JSET_TYPEERROR, "Expecting Object, Array or undefined, got %t", data);
    return 0;
  }

#if ESPR_BLUETOOTH_ANCS
  if (bleStatus & BLE_ANCS_AMS_OR_CTS_INITED) {
    static ble_uuid_t m_adv_uuids[1]; /**< Universally unique service identifiers. */
    ble_ancs_get_adv_uuid(m_adv_uuids);
    advdata.uuids_solicited.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
    advdata.uuids_solicited.p_uuids  = m_adv_uuids;
  }
#endif

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
  if (err_code && !execInfo.hiddenRoot) return 0; // don't error if JS not initialised
  if (jsble_check_error(err_code)) return 0;
  return jsvNewArrayBufferWithData(len_advdata, encoded_advdata);
}

/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "setScanResponse",
    "generate" : "jswrap_ble_setScanResponse",
    "params" : [
      ["data","JsVar","The data to for the scan response"]
    ]
}

The raw scan response data should be supplied as an array. For example to return
"Sample" for the device name:

```
NRF.setScanResponse([0x07,  // Length of Data
  0x09,  // Param: Complete Local Name
  'S', 'a', 'm', 'p', 'l', 'e']);
```

**Note:** `NRF.setServices(..., {advertise:[ ... ]})` writes advertised services
into the scan response - so you can't use both `advertise` and `NRF.setServices`
or one will overwrite the other.
*/
void jswrap_ble_setScanResponse(JsVar *data) {
  uint32_t err_code = 0;


  if (jsvIsUndefined(data)) {
    jsvObjectRemoveChild(execInfo.hiddenRoot, BLE_NAME_SCAN_RESPONSE_DATA);
  } else if (jsvIsArray(data) || jsvIsArrayBuffer(data)) {
    JSV_GET_AS_CHAR_ARRAY(respPtr, respLen, data);
    if (!respPtr) {
      jsExceptionHere(JSET_TYPEERROR, "Unable to convert data argument to an array");
      return;
    }
    // only set data if we managed to decode it ok
    jsvObjectSetOrRemoveChild(execInfo.hiddenRoot, BLE_NAME_SCAN_RESPONSE_DATA, data);

  err_code=jsble_advertising_update_scanresponse((char *)respPtr, respLen);
    jsble_check_error(err_code);
  } else {
    jsExceptionHere(JSET_TYPEERROR, "Expecting array-like object or undefined, got %t", data);
  }
}

// TODO TypeScript improve
/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "setServices",
    "generate" : "jswrap_ble_setServices",
    "params" : [
      ["data","JsVar","The service (and characteristics) to advertise"],
      ["options","JsVar","[optional] Object containing options"]
    ],
    "typescript" : "setServices(data: { [key: number]: { [key: number]: { value?: string, maxLen?: number, broadcast?: boolean, readable?: boolean, writable?: boolean, notify?: boolean, indicate?: boolean, description?: string, security?: { read?: { encrypted?: boolean, mitm?: boolean, lesc?: boolean, signed?: boolean }, write?: { encrypted?: boolean, mitm?: boolean, lesc?: boolean, signed?: boolean } }, onWrite?: (evt: { data: ArrayBuffer }) => void } } }, options?: any): void;"
}

Change the services and characteristics Espruino advertises.

If you want to **change** the value of a characteristic, you need to use
`NRF.updateServices()` instead

To expose some information on Characteristic `ABCD` on service `BCDE` you could
do:

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
characteristic, you can do the following. `evt.data` is an ArrayBuffer.

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
      description: "My Characteristic",  // optional, default is null,
      security: { // optional - see NRF.setSecurity
        read: { // optional
          encrypted: false, // optional, default is false
          mitm: false, // optional, default is false
          lesc: false, // optional, default is false
          signed: false // optional, default is false
        },
        write: { // optional
          encrypted: true, // optional, default is false
          mitm: false, // optional, default is false
          lesc: false, // optional, default is false
          signed: false // optional, default is false
        }
      },
      onWrite : function(evt) { // optional
        console.log("Got ", evt.data); // an ArrayBuffer
      },
      onWriteDesc : function(evt) { // optional - called when the 'cccd' descriptor is written
        // for example this is called when notifications are requested by the client:
        console.log("Notifications enabled = ", evt.data[0]&1);
      }
    }
    // more characteristics allowed
  }
  // more services allowed
});
```

**Note:** UUIDs can be integers between `0` and `0xFFFF`, strings of the form
`"ABCD"`, or strings of the form `"ABCDABCD-ABCD-ABCD-ABCD-ABCDABCDABCD"`

`options` can be of the form:

```
NRF.setServices(undefined, {
  hid : new Uint8Array(...), // optional, default is undefined. Enable BLE HID support
  uart : true, // optional, default is true. Enable BLE UART support
  advertise: [ '180D' ] // optional, list of service UUIDs to advertise
  ancs : true, // optional, Bangle.js-only, enable Apple ANCS support for notifications (see `NRF.ancs*`)
  ams : true // optional, Bangle.js-only, enable Apple AMS support for media control (see `NRF.ams*`)
  cts : true // optional, Bangle.js-only, enable Apple Current Time Service support (see `NRF.ctsGetTime`)
});
```

To enable BLE HID, you must set `hid` to an array which is the BLE report
descriptor. The easiest way to do this is to use the `ble_hid_controls` or
`ble_hid_keyboard` modules.

**Note:** Just creating a service doesn't mean that the service will be
advertised. It will only be available after a device connects. To advertise,
specify the UUIDs you wish to advertise in the `advertise` field of the second
`options` argument. For example this will create and advertise a heart rate
service:

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
packet for the 128 bit UART UUID as well as the UUID you specified. In this case
you can add `uart:false` after the `advertise` element to disable the UART,
however you then be unable to connect to Puck.js's console via Bluetooth.

If you absolutely require two or more 128 bit UUIDs then you will have to
specify your own raw advertising data packets with `NRF.setAdvertising`

**Note:** The services on Espruino can only be modified when there is no device
connected to it as it requires a restart of the Bluetooth stack. **iOS devices
will 'cache' the list of services** so apps like NRF Connect may incorrectly
display the old services even after you have modified them. To fix this, disable
and re-enable Bluetooth on your iOS device, or use an Android device to run NRF
Connect.

**Note:** Not all combinations of security configuration values are valid, the
valid combinations are: encrypted, encrypted + mitm, lesc, signed, signed +
mitm. See `NRF.setSecurity` for more information.
*/
void jswrap_ble_setServices(JsVar *data, JsVar *options) {
  if (!(jsvIsObject(data) || jsvIsUndefined(data))) {
    jsExceptionHere(JSET_TYPEERROR, "Expecting Object or undefined, got %t", data);
    return;
  }

#if BLE_HIDS_ENABLED
  JsVar *use_hid = 0;
#endif
  bool use_uart = true;
#if ESPR_BLUETOOTH_ANCS
  bool use_ancs = false;
  bool use_ams = false;
  bool use_cts = false;
#endif
  JsVar *advertise = 0;

  jsvConfigObject configs[] = {
#if BLE_HIDS_ENABLED
      {"hid", JSV_ARRAY, &use_hid},
#endif
      {"uart", JSV_BOOLEAN, &use_uart}, // sets BLE_NAME_NUS
#if ESPR_BLUETOOTH_ANCS
      {"ancs", JSV_BOOLEAN, &use_ancs},
      {"ams", JSV_BOOLEAN, &use_ams},
      {"cts", JSV_BOOLEAN, &use_cts},
#endif
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
#if ESPR_BLUETOOTH_ANCS
  if (use_ancs) {
    if (!(bleStatus & BLE_ANCS_INITED))
      bleStatus |= BLE_NEEDS_SOFTDEVICE_RESTART;
    jsvObjectSetChildAndUnLock(execInfo.hiddenRoot, BLE_NAME_ANCS, jsvNewFromBool(true));
  } else {
    if (bleStatus & BLE_ANCS_INITED)
      bleStatus |= BLE_NEEDS_SOFTDEVICE_RESTART;
    jsvObjectRemoveChild(execInfo.hiddenRoot, BLE_NAME_ANCS);
  }
  if (use_ams) {
    if (!(bleStatus & BLE_AMS_INITED))
      bleStatus |= BLE_NEEDS_SOFTDEVICE_RESTART;
    jsvObjectSetChildAndUnLock(execInfo.hiddenRoot, BLE_NAME_AMS, jsvNewFromBool(true));
  } else {
    if (bleStatus & BLE_AMS_INITED)
      bleStatus |= BLE_NEEDS_SOFTDEVICE_RESTART;
    jsvObjectRemoveChild(execInfo.hiddenRoot, BLE_NAME_AMS);
  }
  if (use_cts) {
    if (!(bleStatus & BLE_CTS_INITED))
      bleStatus |= BLE_NEEDS_SOFTDEVICE_RESTART;
    jsvObjectSetChildAndUnLock(execInfo.hiddenRoot, BLE_NAME_CTS, jsvNewFromBool(true));
  } else {
    if (bleStatus & BLE_CTS_INITED)
      bleStatus |= BLE_NEEDS_SOFTDEVICE_RESTART;
    jsvObjectRemoveChild(execInfo.hiddenRoot, BLE_NAME_CTS);
  }
#endif

  // Save the current service data and options
  jsvObjectSetOrRemoveChild(execInfo.hiddenRoot, BLE_NAME_SERVICE_DATA, data);
  jsvObjectSetOrRemoveChild(execInfo.hiddenRoot, BLE_NAME_SERVICE_OPTIONS, options);
  // Service UUIDs to advertise
  if (advertise) bleStatus|=BLE_NEEDS_SOFTDEVICE_RESTART;
  jsvObjectSetOrRemoveChild(execInfo.hiddenRoot, BLE_NAME_SERVICE_ADVERTISE, advertise);
  jsvUnLock(advertise);

  // work out whether to apply changes
  if (bleStatus & (BLE_SERVICES_WERE_SET|BLE_NEEDS_SOFTDEVICE_RESTART)) {
    jswrap_ble_restart(NULL);
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
    "generate" : "jswrap_ble_updateServices",
    "params" : [
      ["data","JsVar","The service (and characteristics) to update"]
    ]
}

Update values for the services and characteristics Espruino advertises. Only
services and characteristics previously declared using `NRF.setServices` are
affected.

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

You can also use 128 bit UUIDs, for example
`"b7920001-3c1b-4b40-869f-3c0db9be80c6"`.

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

This only works if the characteristic was created with `notify: true` using
`NRF.setServices`, otherwise the characteristic will be updated but no
notification will be sent.

Also note that `maxLen` was specified. If it wasn't then the maximum length of
the characteristic would have been 5 - the length of `"Hello"`.

To indicate (i.e. notify with ACK) connected clients of a change to the '0xABCD'
characteristic in the '0xBCDE' service:

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

This only works if the characteristic was created with `indicate: true` using
`NRF.setServices`, otherwise the characteristic will be updated but no
notification will be sent.

**Note:** See `NRF.setServices` for more information
*/
void jswrap_ble_updateServices(JsVar *data) {
  uint32_t err_code;
  bool ok = true;

  if (bleStatus & BLE_NEEDS_SOFTDEVICE_RESTART) {
    jsExceptionHere(JSET_ERROR, "Can't update services until BLE restart");
    /* TODO: We could conceivably update hiddenRoot->BLE_NAME_SERVICE_DATA so that
    when the softdevice restarts it contains the updated data, but this seems like
    overkill and potentially could cause nasty hidden bugs. */
    return;
  }

#ifdef NRF5X
  jsble_peripheral_activity(); // flag that we've been busy
#endif

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
          JsVar *charValue = jsvObjectGetChildIfExists(charVar, "value");

          bool notification_requested = jsvObjectGetBoolChild(charVar, "notify");
          bool indication_requested = jsvObjectGetBoolChild(charVar, "indicate");

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
              err_code = sd_ble_gatts_value_set(m_peripheral_conn_handle, char_handle, &gatts_value);
              if (jsble_check_error(err_code)) {
                ok = false;
              } if ((notification_requested || indication_requested) && jsble_has_peripheral_connection()) {
                // Notify/indicate connected clients if necessary
                memset(&hvx_params, 0, sizeof(hvx_params));
                uint16_t len = (uint16_t)vLen;
                hvx_params.handle = char_handle;
                hvx_params.type = indication_requested ? BLE_GATT_HVX_INDICATION : BLE_GATT_HVX_NOTIFICATION;
                hvx_params.offset = 0;
                hvx_params.p_len = &len;
                hvx_params.p_data = (uint8_t*)vPtr;

                err_code = sd_ble_gatts_hvx(m_peripheral_conn_handle, &hvx_params);
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
#ifdef ESP32
              gatts_update_service(char_handle, vPtr, vLen, notification_requested, indication_requested);
#endif
            }
          }
          jsvUnLock2(charValue, charVar);
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
    jsExceptionHere(JSET_TYPEERROR, "Expecting Object or undefined, got %t", data);
  }
}


/// Filter device based on a list of filters (like .requestDevice. Return true if it matches ANY of the filters
bool jswrap_ble_filter_device(JsVar *filters, JsVar *device) {
  bool matches = false;
  JsvObjectIterator fit;
  jsvObjectIteratorNew(&fit, filters);
  while (!matches && jsvObjectIteratorHasValue(&fit)) {
    JsVar *filter = jsvObjectIteratorGetValue(&fit);
    matches = true;
    JsVar *v;
    if ((v = jsvObjectGetChildIfExists(filter, "services"))) {
      // Find one service in the device's service
      JsVar *deviceServices = jsvObjectGetChildIfExists(device, "services");
      JsvObjectIterator it;
      jsvObjectIteratorNew(&it, v);
      while (jsvObjectIteratorHasValue(&it)) {
        bool foundService = false;
        if (deviceServices) {
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
        }
        if (!foundService) matches = false;
        jsvObjectIteratorNext(&it);
      }
      jsvObjectIteratorFree(&it);
      jsvUnLock2(v, deviceServices);
    }
    if ((v = jsvObjectGetChildIfExists(filter, "name"))) {
      // match name exactly
      JsVar *deviceName = jsvObjectGetChildIfExists(device, "name");
      if (!jsvIsEqual(v, deviceName))
        matches = false;
      jsvUnLock2(v, deviceName);
    }
    if ((v = jsvObjectGetChildIfExists(filter, "namePrefix"))) {
      // match start of name
      JsVar *deviceName = jsvObjectGetChildIfExists(device, "name");
      if (!jsvIsString(v) ||
          !jsvIsString(deviceName) ||
          jsvGetStringLength(v)>jsvGetStringLength(deviceName) ||
          jsvCompareString(v, deviceName,0,0,true)!=0)
        matches = false;
      jsvUnLock2(v, deviceName);
    }
    // Non-standard 'id' element
    if ((v = jsvObjectGetChildIfExists(filter, "id"))) {
      JsVar *w = jsvObjectGetChildIfExists(device, "id");
      if (!jsvIsBasicVarEqual(v,w))
        matches = false;
      jsvUnLock2(v,w);
    }
    // match service data
    if ((v = jsvObjectGetChildIfExists(filter, "serviceData"))) {
      if (jsvIsObject(v)) {
        JsvObjectIterator it;
        jsvObjectIteratorNew(&it,v);
        while (jsvObjectIteratorHasValue(&it)) {
          JsVar *childName = jsvObjectIteratorGetKey(&it);
          JsVar *serviceData = jsvObjectGetChildIfExists(device, "serviceData");
          if (!serviceData) matches = false;
          else {
            JsVar *child = jsvFindChildFromVar(serviceData, childName, false);
            if (!child) matches = false;
            jsvUnLock(child);
          }
          jsvUnLock2(childName, serviceData);
          jsvObjectIteratorNext(&it);
        }
        jsvObjectIteratorFree(&it);
      }
      jsvUnLock(v);
    }
    // match manufacturer data
    if ((v = jsvObjectGetChildIfExists(filter, "manufacturerData"))) {
      if (jsvIsObject(v)) {
        JsvObjectIterator it;
        jsvObjectIteratorNew(&it,v);
        while (jsvObjectIteratorHasValue(&it)) {
          JsVar* manfacturera = jsvObjectIteratorGetKey(&it);
          JsVar* manfacturerb = jsvObjectGetChildIfExists(device, "manufacturer");
          if (!jsvIsBasicVarEqual(manfacturera, manfacturerb))
            matches = false;
          jsvUnLock2(manfacturera, manfacturerb);
          jsvObjectIteratorNext(&it);
        }
        jsvObjectIteratorFree(&it);
      }
      jsvUnLock(v);
    }
    // check if all ok
    jsvUnLock(filter);
    jsvObjectIteratorNext(&fit);
  }
  jsvObjectIteratorFree(&fit);
  return matches;
}

/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "setScan",
    "generate" : "jswrap_ble_setScan",
    "params" : [
      ["callback","JsVar","The callback to call with received advertising packets, or undefined to stop"],
      ["options","JsVar","[optional] An object `{filters: ...}` (as would be passed to `NRF.requestDevice`) to filter devices by"]
    ]
}

Start/stop listening for BLE advertising packets within range. Returns a
`BluetoothDevice` for each advertising packet. **By default this is not an active
scan, so Scan Response advertising data is not included (see below)**

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
  "manufacturerData" : new Uint8Array([...]).buffer, // if manufacturer data is in 'data', the data is extracted here as an ArrayBuffer
  "name": "DeviceName"       // the advertised device name
 }
```

You can also supply a set of filters (as described in `NRF.requestDevice`) as a
second argument, which will allow you to filter the devices you get a callback
for. This helps to cut down on the time spent processing JavaScript code in
areas with a lot of Bluetooth advertisements. For example to find only devices
with the manufacturer data `0x0590` (Espruino's ID) you could do:

```
NRF.setScan(function(d) {
  console.log(d.manufacturerData);
}, { filters: [{ manufacturerData:{0x0590:{}} }] });
```

You can also specify `active:true` in the second argument to perform active
scanning (this requests scan response packets) from any devices it finds.

**Note:** Using a filter in `setScan` filters each advertising packet
individually. As a result, if you filter based on a service UUID and a device
advertises with multiple packets (or a scan response when `active:true`) only
the packets matching the filter are returned. To aggregate multiple packets you
can use `NRF.findDevices`.

**Note:** BLE advertising packets can arrive quickly - faster than you'll be
able to print them to the console. It's best only to print a few, or to use a
function like `NRF.findDevices(..)` which will collate a list of available
devices.

**Note:** Using setScan turns the radio's receive mode on constantly. This can
draw a *lot* of power (12mA or so), so you should use it sparingly or you can
run your battery down quickly.
*/
void jswrap_ble_setScan_cb(JsVar *callback, JsVar *filters, JsVar *adv) {
  /* This is called when we get data - do some processing here in the main loop
  then call the callback with it (it avoids us doing more allocations than
  needed inside the IRQ) */
  if (!adv) return;
  // Create a proper BluetoothDevice object
  JsVar *device = jspNewObject(0, "BluetoothDevice");
  JsVar *deviceAddr =  jsvObjectGetChildIfExists(adv, "id");
  jsvObjectSetChild(device, "id", deviceAddr);
  jsvObjectSetChildAndUnLock(device, "rssi", jsvObjectGetChildIfExists(adv, "rssi"));
  JsVar *services = jsvNewEmptyArray();
  JsVar *serviceData = jsvNewObject();
  JsVar *data = jsvObjectGetChildIfExists(adv, "data");
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
          } else if (field_type == BLE_GAP_AD_TYPE_SERVICE_DATA || // 0x16 - service data 16 bit UUID
                     field_type == BLE_GAP_AD_TYPE_SERVICE_DATA_128BIT_UUID) {
            bool is128bit = field_type == BLE_GAP_AD_TYPE_SERVICE_DATA_128BIT_UUID;
            int dataOffset;
            JsVar *childName;
            if (is128bit) {
              childName = bleUUID128ToStr((uint8_t*)&dPtr[i+2]);
              dataOffset = 2+16;
            } else {
              childName = jsvAsArrayIndexAndUnLock(jsvVarPrintf("%04x", UNALIGNED_UINT16(&dPtr[i+2])));
              dataOffset = 2+2;
            }
            if (childName) {
              JsVar *child = jsvFindChildFromVar(serviceData, childName, true);
              JsVar *value = jsvNewArrayBufferWithData(field_length+1-dataOffset, (unsigned char*)&dPtr[i+dataOffset]);
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

  bool deviceMatchedFilters = !filters || jswrap_ble_filter_device(filters, device);

  /* If BLEADV exists then we're using 'NRF.findDevices'
  https://github.com/espruino/Espruino/issues/2178 means that if we have
  a device that advertises its name in a scan response (or other advertising packet)
  BUT we are filtering by a service in the main advertisement, we want to pass this
  advertisement through *if any packet from this device previously matched the filters*
  even if this one doesn't. */
  if (!deviceMatchedFilters) {
    JsVar *arr = jsvObjectGetChild(execInfo.hiddenRoot, "BLEADV", JSV_ARRAY);
    if (arr) {
      JsvObjectIterator it;
      jsvObjectIteratorNew(&it, arr);
      while (!deviceMatchedFilters && jsvObjectIteratorHasValue(&it)) {
        JsVar *obj = jsvObjectIteratorGetValue(&it);
        JsVar *addr = jsvObjectGetChildIfExists(obj, "id");
        if (jsvCompareString(addr, deviceAddr, 0, 0, true) == 0)
          deviceMatchedFilters = true; // we have already matched - so match this one
        jsvUnLock2(addr, obj);
        jsvObjectIteratorNext(&it);
      }
      jsvObjectIteratorFree(&it);
    }
    jsvUnLock(arr);
  }

  if (deviceMatchedFilters)
    jspExecuteFunction(callback, 0, 1, &device);
  jsvUnLock2(deviceAddr, device);
}

void jswrap_ble_setScan(JsVar *callback, JsVar *options) {
  JsVar *filters = 0;
  if (jsvIsObject(options)) {
    filters = jsvObjectGetChildIfExists(options, "filters");
    if (filters && !jsvIsArray(filters)) {
      jsvUnLock(filters);
      jsExceptionHere(JSET_TYPEERROR, "requestDevice expecting an array of filters, got %t", filters);
      return;
    }
  } else if (options)
    jsExceptionHere(JSET_TYPEERROR, "Expecting Object, got %t", options);
  // set the callback event variable
  if (!jsvIsFunction(callback)) callback=0;
  if (callback) {
    JsVar *fn = jsvNewNativeFunction((void (*)(void))jswrap_ble_setScan_cb, JSWAT_THIS_ARG|(JSWAT_JSVAR<<JSWAT_BITS)|(JSWAT_JSVAR<<(JSWAT_BITS*2)));
    if (fn) {
      jsvAddFunctionParameter(fn, 0, filters); // bind param 1
      jsvObjectSetChild(fn, JSPARSE_FUNCTION_THIS_NAME, callback); // bind 'this'
      jsvObjectSetChild(execInfo.root, BLE_SCAN_EVENT, fn);
      jsvUnLock(fn);
    }
  } else {
    jsvObjectRemoveChild(execInfo.root, BLE_SCAN_EVENT);
  }
  // either start or stop scanning
  uint32_t err_code = jsble_set_scanning(callback != 0, options);
  jsble_check_error(err_code);
  jsvUnLock(filters);
}

/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "filterDevices",
    "generate" : "jswrap_ble_filterDevices",
    "params" : [
      ["devices","JsVar","An array of `BluetoothDevice` objects, from `NRF.findDevices` or similar"],
      ["filters","JsVar","A list of filters (as would be passed to `NRF.requestDevice`) to filter devices by"]
    ],
    "return" : ["JsVar","An array of `BluetoothDevice` objects that match the given filters"]
}
This function can be used to quickly filter through Bluetooth devices.

For instance if you wish to scan for multiple different types of device at the
same time then you could use `NRF.findDevices` with all the filters you're
interested in. When scanning is finished you can then use `NRF.filterDevices` to
pick out just the devices of interest.

```
// the two types of device we're interested in
var filter1 = [{serviceData:{"fe95":{}}}];
var filter2 = [{namePrefix:"Pixl.js"}];
// the following filter will return both types of device
var allFilters = filter1.concat(filter2);
// now scan for both types of device, and filter them out afterwards
NRF.findDevices(function(devices) {
  var devices1 = NRF.filterDevices(devices, filter1);
  var devices2 = NRF.filterDevices(devices, filter2);
  // ...
}, {filters : allFilters});
```

*/
JsVar *jswrap_ble_filterDevices(JsVar *devices, JsVar *filters) {
  if (!jsvIsArray(devices) || !jsvIsArray(filters)) {
    jsExceptionHere(JSET_TYPEERROR, "Expecting both arguments to be arrays");
    return 0;
  }
  JsVar *result = jsvNewEmptyArray();
  if (!result) return 0;
  JsvObjectIterator it;
  jsvObjectIteratorNew(&it, devices);
  while (jsvObjectIteratorHasValue(&it)) {
    JsVar *device = jsvObjectIteratorGetValue(&it);
    if (jswrap_ble_filter_device(filters, device))
      jsvArrayPush(result, device);
    jsvUnLock(device);
    jsvObjectIteratorNext(&it);
  }
  jsvObjectIteratorFree(&it);
  return result;
}

/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "findDevices",
    "generate" : "jswrap_ble_findDevices",
    "params" : [
      ["callback","JsVar","The callback to call with received advertising packets (as `BluetoothDevice`), or undefined to stop"],
      ["options","JsVar","[optional] A time in milliseconds to scan for (defaults to 2000), Or an optional object `{filters: ..., timeout : ..., active: bool}` (as would be passed to `NRF.requestDevice`) to filter devices by"]
    ],
    "typescript" : "findDevices(callback: (devices: BluetoothDevice[]) => void, options?: number | { filters?: NRFFilters[], timeout?: number, active?: boolean }): void;"
}
Utility function to return a list of BLE devices detected in range. Behind the
scenes, this uses `NRF.setScan(...)` and collates the results.

```
NRF.findDevices(function(devices) {
  console.log(devices);
}, 1000);
```

prints something like:

```
[
  BluetoothDevice {
    "id" : "e7:e0:57:ad:36:a2 random",
    "rssi": -45,
    "services": [ "4567" ],
    "serviceData" : { "0123" : [ 1 ] },
    "manufacturer" : 1424,
    "manufacturerData" : new Uint8Array([ ... ]).buffer,
    "data": new ArrayBuffer([ ... ]).buffer,
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

For more information on the structure returned, see `NRF.setScan`.

If you want to scan only for specific devices you can replace the timeout with
an object of the form `{filters: ..., timeout : ..., active: bool}` using the
filters described in `NRF.requestDevice`. For example to search for devices with
Espruino's `manufacturerData`:

```
NRF.findDevices(function(devices) {
  ...
}, {timeout : 2000, filters : [{ manufacturerData:{0x0590:{}} }] });
```

You could then use
[`BluetoothDevice.gatt.connect(...)`](/Reference#l_BluetoothRemoteGATTServer_connect)
on the device returned to make a connection.

You can also use [`NRF.connect(...)`](/Reference#l_NRF_connect) on just the `id`
string returned, which may be useful if you always want to connect to a specific
device.

**Note:** Using findDevices turns the radio's receive mode on for 2000ms (or
however long you specify). This can draw a *lot* of power (12mA or so), so you
should use it sparingly or you can run your battery down quickly.

**Note:** The 'data' field contains the data of *the last packet received*.
There may have been more packets. To get data for each packet individually use
`NRF.setScan` instead.
*/
void jswrap_ble_findDevices_found_cb(JsVar *device) {
  JsVar *arr = jsvObjectGetChild(execInfo.hiddenRoot, "BLEADV", JSV_ARRAY);
  if (!arr) return;
  JsVar *deviceAddr = jsvObjectGetChildIfExists(device, "id");
  JsVar *found = 0;
  JsvObjectIterator it;
  jsvObjectIteratorNew(&it, arr);
  while (!found && jsvObjectIteratorHasValue(&it)) {
    JsVar *obj = jsvObjectIteratorGetValue(&it);
    JsVar *addr = jsvObjectGetChildIfExists(obj, "id");
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
      JsVar *value = jsvObjectIteratorGetValue(&oit);
      JsVar *existingKey = jsvFindChildFromVar(found, key, true);
      bool isServices = jsvIsStringEqual(key,"services");
      bool isServiceData = jsvIsStringEqual(key,"serviceData");
      JsVar *existingValue = 0;
      if (isServices || isServiceData) {
        // for services or servicedata we append to the array/object
        existingValue = jsvSkipName(existingKey);
        if (existingValue) {
          if (isServices) {
            jsvArrayPushAll(existingValue, value, true);
          } else {
            jsvObjectAppendAll(existingValue, value);
          }
          jsvUnLock(existingValue);
        }
      }
      if (!existingValue) // nothing already - just copy
        jsvSetValueOfName(existingKey, value);
      jsvUnLock3(existingKey, key, value);
      jsvObjectIteratorNext(&oit);
    }
    jsvObjectIteratorFree(&oit);
  } else
    jsvArrayPush(arr, device);
  jsvUnLock3(found, deviceAddr, arr);
}
void jswrap_ble_findDevices_timeout_cb() {
  jswrap_ble_setScan(0,0);
  JsVar *arr = jsvObjectGetChild(execInfo.hiddenRoot, "BLEADV", JSV_ARRAY);
  JsVar *cb = jsvObjectGetChildIfExists(execInfo.hiddenRoot, "BLEADVCB");
  jsvObjectRemoveChild(execInfo.hiddenRoot, "BLEADV");
  jsvObjectRemoveChild(execInfo.hiddenRoot, "BLEADVCB");
  if (arr && cb) {
    jsiQueueEvents(0, cb, &arr, 1);
  }
  jsvUnLock2(arr,cb);
}
void jswrap_ble_findDevices(JsVar *callback, JsVar *options) {
  JsVarFloat time = 2000;
  if (!jsvIsFunction(callback)) {
    jsExceptionHere(JSET_ERROR, "Expecting function for first argument, got %t", callback);
    return;
  }
  if (jsvIsNumeric(options)) {
    time = jsvGetFloat(options);
    options = 0;
  } else if (jsvIsObject(options)) {
    JsVar *v = jsvObjectGetChildIfExists(options,"timeout");
    if (v) time = jsvGetFloatAndUnLock(v);
  } else if (options) {
    jsExceptionHere(JSET_ERROR, "Expecting Number or object, got %t", options);
    return;
  }
  if (isnan(time) || time < 10) {
    jsExceptionHere(JSET_ERROR, "Invalid timeout");
    return;
  }

  jsvObjectSetChildAndUnLock(execInfo.hiddenRoot, "BLEADV", jsvNewEmptyArray());
  jsvObjectSetChild(execInfo.hiddenRoot, "BLEADVCB", callback);
  JsVar *fn;
  fn = jsvNewNativeFunction((void (*)(void))jswrap_ble_findDevices_found_cb, JSWAT_VOID|(JSWAT_JSVAR<<JSWAT_BITS));
  if (fn) {
    jswrap_ble_setScan(fn, options);
    jsvUnLock(fn);
  }
  jsvUnLock(jsiSetTimeout(jswrap_ble_findDevices_timeout_cb, time));
}

/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "setRSSIHandler",
    "generate" : "jswrap_ble_setRSSIHandler",
    "params" : [
      ["callback","JsVar","The callback to call with the RSSI value, or undefined to stop"]
    ]
}

Start/stop listening for RSSI values on the currently active connection (where
This device is a peripheral and is being connected to by a 'central' device)

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
void jswrap_ble_setRSSIHandler(JsVar *callback) {
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
    "generate" : "jswrap_ble_setTxPower",
    "params" : [
      ["power","int","Transmit power. Accepted values are -40(nRF52 only), -30(nRF51 only), -20, -16, -12, -8, -4, 0, and 4 dBm. On nRF52840 (eg Bangle.js 2) 5/6/7/8 dBm are available too. Others will give an error code."]
    ]
}
Set the BLE radio transmit power. The default TX power is 0 dBm, and
*/
void jswrap_ble_setTxPower(JsVarInt pwr) {
  jsble_set_tx_power(pwr);
}


/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "setLowPowerConnection",
    "generate" : "jswrap_ble_setLowPowerConnection",
    "params" : [
      ["lowPower","bool","Whether the connection is low power or not"]
    ]
}

**THIS IS DEPRECATED** - please use `NRF.setConnectionInterval` for peripheral
and `NRF.connect(addr, options)`/`BluetoothRemoteGATTServer.connect(options)`
for central connections.

This sets the connection parameters - these affect the transfer speed and power
usage when the device is connected.

* When not low power, the connection interval is between 7.5 and 20ms
* When low power, the connection interval is between 500 and 1000ms

When low power connection is enabled, transfers of data over Bluetooth will be
very slow, however power usage while connected will be drastically decreased.

This will only take effect after the connection is disconnected and
re-established.
*/
void jswrap_ble_setLowPowerConnection(bool lowPower) {
  BLEFlags oldflags = jsvGetIntegerAndUnLock(jsvObjectGetChildIfExists(execInfo.hiddenRoot, BLE_NAME_FLAGS));
  BLEFlags flags = oldflags;
  if (lowPower)
    flags |= BLE_FLAGS_LOW_POWER;
  else
    flags &= ~BLE_FLAGS_LOW_POWER;
  if (flags != oldflags) {
    jsvObjectSetChildAndUnLock(execInfo.hiddenRoot, BLE_NAME_FLAGS, jsvNewFromInteger(flags));
    jswrap_ble_restart(NULL);
  }
}

#ifdef USE_NFC
static void nfc_raw_data_start(uint8_t *dataPtr, size_t dataLen){
  /* Create a flat string - we need this to store the NFC data so it hangs around.
   * Avoid having a static var so we have RAM available if not using NFC.
   * NFC data is read by nfc_callback in bluetooth.c */
  bool isLongMsg = dataLen > 254;
  size_t nfcDataLen = dataLen + NDEF_TERM_TLV_LEN + (isLongMsg ? NDEF_HEADER_LEN_LONG : NDEF_HEADER_LEN_SHORT);


  JsVar *flatStr = jsvNewFlatStringOfLength(nfcDataLen);
  if (!flatStr)
    return jsExceptionHere(JSET_ERROR, "Unable to create string with NFC data in");
  jsvObjectSetChild(execInfo.hiddenRoot, "NfcData", flatStr);
  uint8_t *flatStrPtr = (uint8_t*)jsvGetFlatStringPointer(flatStr);
  jsvUnLock(flatStr);

/* assemble NDEF Message */
  memcpy(flatStrPtr, NDEF_HEADER, NDEF_HEADER_LEN_LONG); /* fill header */
  /* inject tag2 message length into header */
  if (isLongMsg){
    flatStrPtr[NDEF_HEADER_MSG_LEN_OFFSET] = 255;
    flatStrPtr[NDEF_HEADER_MSG_LEN_OFFSET+1] = dataLen >> 8;
    flatStrPtr[NDEF_HEADER_MSG_LEN_OFFSET+2] = dataLen & 255;
  } else {
    flatStrPtr[NDEF_HEADER_MSG_LEN_OFFSET] = dataLen;
  }

  /* add NDEF message record header after NDEF header */
  uint8_t *ndefMsgPtr = flatStrPtr + (isLongMsg ? NDEF_HEADER_LEN_LONG : NDEF_HEADER_LEN_SHORT);
  memcpy(ndefMsgPtr, dataPtr, dataLen); /* add payload */


  /* write terminator TLV block */
  flatStrPtr[nfcDataLen - NDEF_TERM_TLV_LEN] = NDEF_TERM_TLV;

  /* start nfc peripheral */
  JsVar* uid = jswrap_nfc_start(NULL);

  /* inject UID/BCC */
  size_t len;
  char *uidPtr = jsvGetDataPointer(uid, &len);
  if(uidPtr) memcpy(flatStrPtr, uidPtr, TAG_HEADER_LEN);
  jsvUnLock(uid);
}
#endif

/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "nfcURL",
    "#if" : "defined(NRF52_SERIES) && defined(USE_NFC)",
    "generate" : "jswrap_nfc_URL",
    "params" : [
      ["url","JsVar","The URL string to expose on NFC, or `undefined` to disable NFC"]
    ]
}
Enables NFC and starts advertising the given URL. For example:

```
NRF.nfcURL("http://espruino.com");
```
*/
void jswrap_nfc_URL(JsVar *url) {
#ifdef USE_NFC
  // Check for disabling NFC
  if (jsvIsUndefined(url)) {
    jsvObjectRemoveChild(execInfo.hiddenRoot, "NfcData");
    jswrap_nfc_stop();
    return;
  }

  if (!jsvIsString(url)) {
    jsExceptionHere(JSET_TYPEERROR, "Expecting String, got %t", url);
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

  uint16_t msgLen = NDEF_URL_RECORD_HEADER_LEN + urlLen;
  if (msgLen>NDEF_TAG2_VALUE_MAXLEN)
    return jsExceptionHere(JSET_ERROR, "URL too long");

  uint8_t *ndefMsgPtr = alloca(urlLen+NDEF_URL_RECORD_HEADER_LEN);
  memcpy(ndefMsgPtr, NDEF_URL_RECORD_HEADER, NDEF_URL_RECORD_HEADER_LEN); /* fill header */

  ndefMsgPtr[NDEF_MSG_IC_OFFSET] = uriType; /* set URI Identifier Code */
  memcpy(ndefMsgPtr+NDEF_URL_RECORD_HEADER_LEN, urlPtr, urlLen); /* add payload */

  /* put 16 bit (big endian) payload length into record header */
  ndefMsgPtr[NDEF_MSG_PL_LEN_MSB_OFFSET] = (NDEF_IC_LEN + urlLen)>>8;
  ndefMsgPtr[NDEF_MSG_PL_LEN_MSB_OFFSET+1] = (NDEF_IC_LEN + urlLen)&255;

  nfc_raw_data_start(ndefMsgPtr, urlLen+NDEF_URL_RECORD_HEADER_LEN);
#endif
}

/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "nfcPair",
    "#if" : "defined(NRF52_SERIES) && defined(USE_NFC)",
    "generate" : "jswrap_nfc_pair",
    "params" : [
      ["key","JsVar","16 byte out of band key"]
    ]
}
Enables NFC and with an out of band 16 byte pairing key.

For example the following will enable out of band pairing on BLE such that the
device will pair when you tap the phone against it:

```
var bleKey = [0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x99, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0x00];
NRF.on('security',s=>print("security",JSON.stringify(s)));
NRF.nfcPair(bleKey);
NRF.setSecurity({oob:bleKey, mitm:true});
```
*/
void jswrap_nfc_pair(JsVar *key) {
#ifdef USE_NFC
  // Check for disabling NFC
  if (jsvIsUndefined(key)) {
    jsvObjectRemoveChild(execInfo.hiddenRoot, "NfcData");
    jswrap_nfc_stop();
    return;
  }

  JSV_GET_AS_CHAR_ARRAY(keyPtr, keyLen, key);
  if (!keyPtr || keyLen!=BLE_GAP_SEC_KEY_LEN)
    return jsExceptionHere(JSET_ERROR, "Unable to get key data or key isn't 16 bytes long");

  /* assemble NDEF Message */
  /* Encode BLE pairing message into the buffer. */
  uint8_t buf[256];
  uint32_t ndef_msg_len = sizeof(buf);
  uint32_t err_code = nfc_ble_pair_default_msg_encode(NFC_BLE_PAIR_MSG_FULL,
                                             (ble_advdata_tk_value_t *)keyPtr,
                                             NULL,
                                             buf,
                                             &ndef_msg_len);
  if (jsble_check_error(err_code)) return;

  nfc_raw_data_start(buf, ndef_msg_len);

#endif
}

/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "nfcAndroidApp",
    "#if" : "defined(NRF52_SERIES) && defined(USE_NFC)",
    "generate" : "jswrap_nfc_androidApp",
    "params" : [
      ["app","JsVar","The unique identifier of the given Android App"]
    ]
}
Enables NFC with a record that will launch the given android app.

For example:

```
NRF.nfcAndroidApp("no.nordicsemi.android.nrftoolbox")
```
*/
void jswrap_nfc_androidApp(JsVar *appName) {
#ifdef USE_NFC
  // Check for disabling NFC
  if (jsvIsUndefined(appName)) {
    jsvObjectRemoveChild(execInfo.hiddenRoot, "NfcData");
    jswrap_nfc_stop();
    return;
  }

  JSV_GET_AS_CHAR_ARRAY(appNamePtr, appNameLen, appName);
  if (!appNamePtr || !appNameLen)
    return jsExceptionHere(JSET_ERROR, "Unable to get app name");

  /* assemble NDEF Message */
  /* Encode BLE pairing message into the buffer. */
  uint8_t buf[512];
  uint32_t ndef_msg_len = sizeof(buf);
  /* Encode launchapp message into the buffer. */
  uint32_t err_code = nfc_launchapp_msg_encode((uint8_t*)appNamePtr,
                                      appNameLen,
                                      0,
                                      0,
                                      buf,
                                      &ndef_msg_len);
  if (jsble_check_error(err_code)) return;

  nfc_raw_data_start(buf, ndef_msg_len);
#endif
}

/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "nfcRaw",
    "#if" : "defined(NRF52_SERIES) && defined(USE_NFC)",
    "generate" : "jswrap_nfc_raw",
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
void jswrap_nfc_raw(JsVar *payload) {
#ifdef USE_NFC
  // Check for disabling NFC
  if (jsvIsUndefined(payload)) {
    jsvObjectRemoveChild(execInfo.hiddenRoot, "NfcData");
    jswrap_nfc_stop();
    return;
  }

  JSV_GET_AS_CHAR_ARRAY(dataPtr, dataLen, payload);
  if (!dataPtr || !dataLen || dataLen>NDEF_TAG2_VALUE_MAXLEN)
    return jsExceptionHere(JSET_ERROR, "Unable to get NFC data");

  nfc_raw_data_start((uint8_t*)dataPtr, dataLen);
#endif
}

/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "nfcStart",
    "#if" : "defined(NRF52_SERIES) && defined(USE_NFC)",
    "generate" : "jswrap_nfc_start",
    "params" : [
      ["payload","JsVar","Optional 7 byte UID"]
    ],
    "return" : ["JsVar", "Internal tag memory (first 10 bytes of tag data)" ]
}
**Advanced NFC Functionality.** If you just want to advertise a URL, use
`NRF.nfcURL` instead.

Enables NFC and starts advertising. `NFCrx` events will be fired when data is
received.

```
NRF.nfcStart();
```
*/
JsVar *jswrap_nfc_start(JsVar *payload) {
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
#else
  return 0;
#endif
}

/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "nfcStop",
    "#if" : "defined(NRF52_SERIES) && defined(USE_NFC)",
    "generate" : "jswrap_nfc_stop",
    "params" : [ ]
}
**Advanced NFC Functionality.** If you just want to advertise a URL, use
`NRF.nfcURL` instead.

Disables NFC.

```
NRF.nfcStop();
```
*/
void jswrap_nfc_stop() {
#ifdef USE_NFC
  jsvObjectRemoveChild(execInfo.hiddenRoot, "NfcEnabled");
  jsble_nfc_stop();
#endif
}


/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "nfcSend",
    "#if" : "defined(NRF52_SERIES) && defined(USE_NFC)",
    "generate" : "jswrap_nfc_send",
    "params" : [
      ["payload","JsVar","Optional tx data"]
    ]
}
**Advanced NFC Functionality.** If you just want to advertise a URL, use
`NRF.nfcURL` instead.

Acknowledges the last frame and optionally transmits a response. If payload is
an array, then a array.length byte nfc frame is sent. If payload is a int, then
a 4bit ACK/NACK is sent. **Note:** ```nfcSend``` should always be called after
an ```NFCrx``` event.

```
NRF.nfcSend(new Uint8Array([0x01, 0x02, ...]));
// or
NRF.nfcSend(0x0A);
// or
NRF.nfcSend();
```
*/
void jswrap_nfc_send(JsVar *payload) {
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
    "ifdef" : "NRF52_SERIES",
    "generate" : "jswrap_ble_sendHIDReport",
    "params" : [
      ["data","JsVar","Input report data as an array"],
      ["callback","JsVar","A callback function to be called when the data is sent"]
    ],
    "typescript" : "sendHIDReport(data: number[], callback?: () => void): void"
}
Send a USB HID report. HID must first be enabled with `NRF.setServices({}, {hid:
hid_report})`
*/
void jswrap_ble_sendHIDReport(JsVar *data, JsVar *callback) {
#if BLE_HIDS_ENABLED
  JSV_GET_AS_CHAR_ARRAY(vPtr, vLen, data)
  if (vPtr && vLen) {
    if (jsvIsFunction(callback))
      jsvObjectSetChild(execInfo.root, BLE_HID_SENT_EVENT, callback);
    jsble_send_hid_input_report((uint8_t*)vPtr, vLen);
  } else {
    jsExceptionHere(JSET_ERROR, "Expecting Array, got %t", data);
  }
#endif
}


/*JSON{
  "type" : "event",
  "class" : "E",
  "name" : "ANCS",
  "params" : [["info","JsVar","An object (see below)"]],
  "ifdef" : "BANGLEJS"
}
Called when a notification arrives on an Apple iOS device Bangle.js is connected
to


```
{
event:"add",
uid:42,
category:4,
categoryCnt:42,
silent:true,
important:false,
preExisting:true,
positive:false,
negative:true
}
```

You can then get more information with `NRF.ancsGetNotificationInfo`, for instance:

```
E.on('ANCS', event => {
  NRF.ancsGetNotificationInfo( event.uid ).then(a=>print("Notify",E.toJS(a)));
});
```
*/

/*JSON{
  "type" : "event",
  "class" : "E",
  "name" : "AMS",
  "params" : [["info","JsVar","An object (see below)"]],
  "ifdef" : "BANGLEJS"
}
Called when a media event arrives on an Apple iOS device Bangle.js is connected
to


```
{
id : "artist"/"album"/"title"/"duration",
value : "Some text",
truncated : bool // the 'value' was too big to be sent completely
}
```

*/

/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "ancsIsActive",
    "ifdef" : "NRF52_SERIES",
    "generate" : "jswrap_ble_ancsIsActive",
    "params" : [ ],
    "return" : ["bool", "True if Apple Notification Center Service (ANCS) has been initialised and is active" ]
}
Check if Apple Notification Center Service (ANCS) is currently active on the BLE
connection
*/
bool jswrap_ble_ancsIsActive() {
#if ESPR_BLUETOOTH_ANCS
  return ((bleStatus & BLE_ANCS_INITED) && ble_ancs_is_active());
#else
  return false;
#endif
}

/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "ancsAction",
    "ifdef" : "NRF52_SERIES",
    "generate" : "jswrap_ble_ancsAction",
    "params" : [
      ["uid","int","The UID of the notification to respond to"],
      ["positive","bool","`true` for positive action, `false` for negative"]
    ]
}
Send an ANCS action for a specific Notification UID. Corresponds to
posaction/negaction in the 'ANCS' event that was received
*/
void jswrap_ble_ancsAction(int uid, bool isPositive) {
#if ESPR_BLUETOOTH_ANCS
  if (!(bleStatus & BLE_ANCS_INITED) || !ble_ancs_is_active()) {
    jsExceptionHere(JSET_ERROR, "ANCS not active");
    return;
  }
  ble_ancs_action(uid, isPositive);
#endif
}

/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "ancsGetNotificationInfo",
    "ifdef" : "NRF52_SERIES",
    "generate" : "jswrap_ble_ancsGetNotificationInfo",
    "params" : [
      ["uid","int","The UID of the notification to get information for"]
    ],
    "return" : ["JsVar", "A `Promise` that is resolved (or rejected) when the connection is complete" ],
    "return_object" : "Promise"
}
Get ANCS info for a notification event received via `E.ANCS`, e.g.:

```
E.on('ANCS', event => {
  NRF.ancsGetNotificationInfo( event.uid ).then(a=>print("Notify",E.toJS(a)));
});
```

Returns:

```
{
  "uid" : integer,
  "appId": string,
  "title": string,
  "subtitle": string,
  "message": string,
  "messageSize": string,
  "date": string,
  "posAction": string,
  "negAction": string
}
```

*/
JsVar *jswrap_ble_ancsGetNotificationInfo(JsVarInt uid) {
  JsVar *promise = 0;
#if ESPR_BLUETOOTH_ANCS
  if (!(bleStatus & BLE_ANCS_INITED) || !ble_ancs_is_active()) {
    jsExceptionHere(JSET_ERROR, "ANCS not active");
    return 0;
  }
  if (ble_ancs_request_notif(uid)) { // if fails, it'll create an exception
    if (bleNewTask(BLETASK_ANCS_NOTIF_ATTR, 0)) {
      promise = jsvLockAgainSafe(blePromise);
    }
  }
#endif
  return promise;
}

/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "ancsGetAppInfo",
    "ifdef" : "NRF52_SERIES",
    "generate" : "jswrap_ble_ancsGetAppInfo",
    "params" : [
      ["id","JsVar","The app ID to get information for"]
    ],
    "return" : ["JsVar", "A `Promise` that is resolved (or rejected) when the connection is complete" ],
    "return_object" : "Promise"
}
Get ANCS info for an app (app id is available via `NRF.ancsGetNotificationInfo`)

Promise returns:

```
{
  "uid" : int,
  "appId" : string,
  "title" : string,
  "subtitle" : string,
  "message" : string,
  "messageSize" : string,
  "date" : string,
  "posAction" : string,
  "negAction" : string,
  "name" : string,
}
```
*/
JsVar *jswrap_ble_ancsGetAppInfo(JsVar *appId) {
  JsVar *promise = 0;
#if ESPR_BLUETOOTH_ANCS
  if (!(bleStatus & BLE_ANCS_INITED) || !ble_ancs_is_active()) {
    jsExceptionHere(JSET_ERROR, "ANCS not active");
    return 0;
  }
  char appIdStr[48];
  jsvGetString(appId, appIdStr, sizeof(appIdStr));
  if (ble_ancs_request_app(appIdStr, strlen(appIdStr))) { // if fails, it'll create an exception
    if (bleNewTask(BLETASK_ANCS_APP_ATTR, appId)) {
      promise = jsvLockAgainSafe(blePromise);
    }
  }
#endif
  return promise;
}

/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "amsIsActive",
    "ifdef" : "NRF52_SERIES",
    "generate" : "jswrap_ble_amsIsActive",
    "params" : [ ],
    "return" : ["bool", "True if Apple Media Service (AMS) has been initialised and is active" ]
}
Check if Apple Media Service (AMS) is currently active on the BLE connection
*/
bool jswrap_ble_amsIsActive() {
#if ESPR_BLUETOOTH_ANCS
  return ((bleStatus & BLE_AMS_INITED) && ble_ams_is_active());
#else
  return false;
#endif
}

/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "amsGetPlayerInfo",
    "ifdef" : "NRF52_SERIES",
    "generate" : "jswrap_ble_amsGetPlayerInfo",
    "params" : [
      ["id","JsVar","Either 'name', 'playbackinfo' or 'volume'"]
    ],
    "return" : ["JsVar", "A `Promise` that is resolved (or rejected) when the connection is complete" ],
    "return_object" : "Promise"
}
Get Apple Media Service (AMS) info for the current media player. "playbackinfo"
returns a concatenation of three comma-separated values:

- PlaybackState: a string that represents the integer value of the playback
  state:
    - PlaybackStatePaused = 0
    - PlaybackStatePlaying = 1
    - PlaybackStateRewinding = 2
    - PlaybackStateFastForwarding = 3
- PlaybackRate: a string that represents the floating point value of the
  playback rate.
- ElapsedTime: a string that represents the floating point value of the elapsed
  time of the current track, in seconds

*/
JsVar *jswrap_ble_amsGetPlayerInfo(JsVar *id) {
  JsVar *promise = 0;
#if ESPR_BLUETOOTH_ANCS
  if (!(bleStatus & BLE_AMS_INITED) || !ble_ams_is_active()) {
    jsExceptionHere(JSET_ERROR, "AMS not active");
    return 0;
  }
  ble_ams_c_player_attribute_id_val_t cmd;
  if (jsvIsStringEqual(id,"name")) cmd=BLE_AMS_PLAYER_ATTRIBUTE_ID_NAME;
  else if (jsvIsStringEqual(id,"playbackinfo")) cmd=BLE_AMS_PLAYER_ATTRIBUTE_ID_PLAYBACK_INFO;
  else if (jsvIsStringEqual(id,"volume")) cmd=BLE_AMS_PLAYER_ATTRIBUTE_ID_VOLUME;
  else {
    jsExceptionHere(JSET_ERROR, "Unknown id %q", id);
    return promise;
  }
  if (ble_ams_request_player_info(cmd)) { // if fails, it'll create an exception
    if (bleNewTask(BLETASK_AMS_ATTR, 0)) {
      promise = jsvLockAgainSafe(blePromise);
    }
  }
#endif
  return promise;
}

/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "amsGetTrackInfo",
    "ifdef" : "NRF52_SERIES",
    "generate" : "jswrap_ble_amsGetTrackInfo",
    "params" : [
      ["id","JsVar","Either 'artist', 'album', 'title' or 'duration'"]
    ],
    "return" : ["JsVar", "A `Promise` that is resolved (or rejected) when the connection is complete" ],
    "return_object" : "Promise"
}
Get Apple Media Service (AMS) info for the currently-playing track
*/
JsVar *jswrap_ble_amsGetTrackInfo(JsVar *id) {
  JsVar *promise = 0;
#if ESPR_BLUETOOTH_ANCS
  if (!(bleStatus & BLE_AMS_INITED) || !ble_ams_is_active()) {
    jsExceptionHere(JSET_ERROR, "AMS not active");
    return 0;
  }
  ble_ams_c_track_attribute_id_val_t cmd;
  if (jsvIsStringEqual(id,"artist")) cmd=BLE_AMS_TRACK_ATTRIBUTE_ID_ARTIST;
  else if (jsvIsStringEqual(id,"album")) cmd=BLE_AMS_TRACK_ATTRIBUTE_ID_ALBUM;
  else if (jsvIsStringEqual(id,"title")) cmd=BLE_AMS_TRACK_ATTRIBUTE_ID_TITLE;
  else if (jsvIsStringEqual(id,"duration")) cmd=BLE_AMS_TRACK_ATTRIBUTE_ID_DURATION;
  else {
    jsExceptionHere(JSET_ERROR, "Unknown id %q", id);
    return promise;
  }
  if (ble_ams_request_track_info(cmd)) { // if fails, it'll create an exception
    if (bleNewTask(BLETASK_AMS_ATTR, 0)) {
      promise = jsvLockAgainSafe(blePromise);
    }
  }
#endif
  return promise;
}

/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "amsCommand",
    "ifdef" : "NRF52_SERIES",
    "generate" : "jswrap_ble_amsCommand",
    "params" : [
      ["id","JsVar","For example, 'play', 'pause', 'volup' or 'voldown'"]
    ]
}
Send an AMS command to an Apple Media Service device to control music playback

Command is one of play, pause, playpause, next, prev, volup, voldown, repeat,
shuffle, skipforward, skipback, like, dislike, bookmark
*/
void jswrap_ble_amsCommand(JsVar *id) {
#if ESPR_BLUETOOTH_ANCS
  if (!(bleStatus & BLE_AMS_INITED) || !ble_ams_is_active()) {
    jsExceptionHere(JSET_ERROR, "AMS not active");
    return;
  }
  ble_ams_c_remote_control_id_val_t cmd;
  if (jsvIsStringEqual(id,"play")) cmd=BLE_AMS_REMOTE_COMMAND_ID_PLAY;
  else if (jsvIsStringEqual(id,"pause")) cmd=BLE_AMS_REMOTE_COMMAND_ID_PAUSE;
  else if (jsvIsStringEqual(id,"playpause")) cmd=BLE_AMS_REMOTE_COMMAND_ID_TOGGLE_PLAY_PAUSE;
  else if (jsvIsStringEqual(id,"next")) cmd=BLE_AMS_REMOTE_COMMAND_ID_NEXT_TRACK;
  else if (jsvIsStringEqual(id,"prev")) cmd=BLE_AMS_REMOTE_COMMAND_ID_PREVIOUS_TRACK;
  else if (jsvIsStringEqual(id,"volup")) cmd=BLE_AMS_REMOTE_COMMAND_ID_VOLUME_UP;
  else if (jsvIsStringEqual(id,"voldown")) cmd=BLE_AMS_REMOTE_COMMAND_ID_VOLUME_DOWN;
  else if (jsvIsStringEqual(id,"repeat")) cmd=BLE_AMS_REMOTE_COMMAND_ID_ADVANCE_REPEAT_MODE;
  else if (jsvIsStringEqual(id,"shuffle")) cmd=BLE_AMS_REMOTE_COMMAND_ID_ADVANCE_SHUFFLE_MODE;
  else if (jsvIsStringEqual(id,"skipforward")) cmd=BLE_AMS_REMOTE_COMMAND_ID_SKIP_FORWARD;
  else if (jsvIsStringEqual(id,"skipback")) cmd=BLE_AMS_REMOTE_COMMAND_ID_SKIP_BACKWARD;
  else if (jsvIsStringEqual(id,"like")) cmd=BLE_AMS_REMOTE_COMMAND_ID_LIKE_TRACK;
  else if (jsvIsStringEqual(id,"dislike")) cmd=BLE_AMS_REMOTE_COMMAND_ID_DISLIKE_TRACK;
  else if (jsvIsStringEqual(id,"bookmark")) cmd=BLE_AMS_REMOTE_COMMAND_ID_BOOKMARK_TRACK;
  else {
    jsExceptionHere(JSET_ERROR, "Unknown command %q", id);
    return;
  }
  ble_ams_command(cmd);
#endif
}

/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "ctsIsActive",
    "ifdef" : "NRF52_SERIES",
    "generate" : "jswrap_ble_ctsIsActive",
    "params" : [ ],
    "return" : ["bool", "True if Apple Current Time Service (CTS) has been initialised and is active" ]
}
Check if Apple Current Time Service (CTS) is currently active on the BLE connection
*/
bool jswrap_ble_ctsIsActive() {
#if ESPR_BLUETOOTH_ANCS
  return ((bleStatus & BLE_CTS_INITED) && ble_cts_is_active());
#else
  return false;
#endif
}

/*JSON{
  "type" : "event",
  "class" : "NRF",
  "name" : "CTS",
  "params" : [["info","JsVar","An object (see below)"]],
  "ifdef" : "BANGLEJS"
}
Returns time information from the Current Time Service
(if requested with `NRF.ctsGetTime` and is activated by calling `NRF.setServices(..., {..., cts:true})`)

```
{
  date : // Date object with the current date
  day :  // if known, 0=sun,1=mon (matches JS `Date`)
  reason : [ // reason for the date change
      "external", // External time change
      "manual",   // Manual update
      "timezone", // Timezone changed
      "DST",      // Daylight savings
    ]
  timezone // if LTI characteristic exists, this is the timezone
  dst      // if LTI characteristic exists, this is the dst adjustment
}
```

For instance this can be used as follows to update Espruino's time:

```
E.on('CTS',e=>{
  setTime(e.date.getTime()/1000);
});
NRF.ctsGetTime(); // also returns a promise with CTS info
```
*/

/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "ctsGetTime",
    "ifdef" : "NRF52_SERIES",
    "generate" : "jswrap_ble_ctsGetTime",
    "return" : ["JsVar", "A `Promise` that is resolved (or rejected) when time is received" ],
    "return_object" : "Promise"
}
Read the time from CTS - creates an `NRF.on('CTS', ...)` event as well

```
NRF.ctsGetTime(); // also returns a promise
```
*/
JsVar *jswrap_ble_ctsGetTime() {
  JsVar *promise = 0;
#if ESPR_BLUETOOTH_ANCS
  if (!(bleStatus & BLE_CTS_INITED) || !ble_cts_is_active()) {
    jsExceptionHere(JSET_ERROR, "CTS not active");
    return 0;
  }
  if (ble_cts_read_time()) { // if fails, it'll create an exception
    if (bleNewTask(BLETASK_CTS_GET_TIME, 0)) {
      promise = jsvLockAgainSafe(blePromise);
    }
  }
#endif
  return promise;
}


/*TYPESCRIPT
type NRFFilters = {
  services?: string[];
  name?: string;
  namePrefix?: string;
  id?: string;
  serviceData?: object;
  manufacturerData?: object;
};
*/

/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "requestDevice",
    "#if" : "defined(NRF52_SERIES) || defined(ESP32)",
    "generate" : "jswrap_ble_requestDevice",
    "params" : [
      ["options","JsVar","Options used to filter the device to use"]
    ],
    "typescript" : "requestDevice(options?: { filters?: NRFFilters[], timeout?: number, active?: boolean, phy?: string, extended?: boolean }): Promise<any>;",
    "return" : ["JsVar", "A `Promise` that is resolved (or rejected) when the connection is complete" ],
    "return_object" : "Promise"
}
Search for available devices matching the given filters. Since we have no UI
here, Espruino will pick the FIRST device it finds, or it'll call `catch`.

`options` can have the following fields:

* `filters` - a list of filters that a device must match before it is returned
  (see below)
* `timeout` - the maximum time to scan for in milliseconds (scanning stops when
a match is found. e.g. `NRF.requestDevice({ timeout:2000, filters: [ ... ] })`
* `active` - whether to perform active scanning (requesting 'scan response'
packets from any devices that are found). e.g. `NRF.requestDevice({ active:true,
filters: [ ... ] })`
* `phy` - (NRF52833/NRF52840 only) the type of Bluetooth signals to scan for (can
  be `"1mbps/coded/both/2mbps"`)
  * `1mbps` (default) - standard Bluetooth LE advertising
  * `coded` - long range
  * `both` - standard and long range
  * `2mbps` - high speed 2mbps (not working)
* `extended` - (NRF52833/NRF52840 only) support receiving extended-length advertising
  packets (default=true if phy isn't `"1mbps"`)
* `extended` - (NRF52833/NRF52840 only) support receiving extended-length advertising
  packets (default=true if phy isn't `"1mbps"`)
* `window` - (2v22+) how long we scan for in milliseconds (default 100ms)
* `interval` - (2v22+) how often we scan in milliseconds (default 100ms) - `window=interval=100`(default) is all the time. When
scanning on both `1mbps` and `coded`, `interval` needs to be twice `window`.

**NOTE:** `timeout` and `active` are not part of the Web Bluetooth standard.

The following filter types are implemented:

* `services` - list of services as strings (all of which must match). 128 bit
  services must be in the form '01230123-0123-0123-0123-012301230123'
* `name` - exact device name
* `namePrefix` - starting characters of device name
* `id` - exact device address (`id:"e9:53:86:09:89:99 random"`) (this is
  Espruino-specific, and is not part of the Web Bluetooth spec)
* `serviceData` - an object containing service characteristics which must all
  match (`serviceData:{"1809":{}}`). Matching of actual service data is not
  supported yet.
* `manufacturerData` - an object containing manufacturer UUIDs which must all
  match (`manufacturerData:{0x0590:{}}`). Matching of actual manufacturer data
  is not supported yet.

```
NRF.requestDevice({ filters: [{ namePrefix: 'Puck.js' }] }).then(function(device) { ... });
// or
NRF.requestDevice({ filters: [{ services: ['1823'] }] }).then(function(device) { ... });
// or
NRF.requestDevice({ filters: [{ manufacturerData:{0x0590:{}} }] }).then(function(device) { ... });
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

Note that you have to keep track of the `gatt` variable so that you can
disconnect the Bluetooth connection when you're done.

**Note:** Using a filter in `NRF.requestDevice` filters each advertising packet
individually. As soon as a matching advertisement is received,
`NRF.requestDevice` resolves the promise and stops scanning. This means that if
you filter based on a service UUID and a device advertises with multiple packets
(or a scan response when `active:true`) only the packet matching the filter is
returned - you may not get the device's name is that was in a separate packet.
To aggregate multiple packets you can use `NRF.findDevices`.
*/
#if CENTRAL_LINK_COUNT>0
/// Called when we timeout waiting for a device
void jswrap_ble_requestDevice_finish() {
  if (!bleInTask(BLETASK_REQUEST_DEVICE))
    return;
  jswrap_ble_setScan(0,0);  // stop scanning
  bleCompleteTaskFailAndUnLock(BLETASK_REQUEST_DEVICE, jsvNewFromString("No device found matching filters"));
}

/// Called when a device is found
void jswrap_ble_requestDevice_scan(JsVar *device) {
  if (!bleInTask(BLETASK_REQUEST_DEVICE))
    return;
  // We know the device matches because setScan would have checked for us
  jswrap_ble_setScan(0,0); // stop scanning
  JsVar *argArr = jsvNewArray(&bleTaskInfo, 1);
  jswrap_interface_clearTimeout(argArr /*the timeout*/); // cancel the timeout
  jsvUnLock(argArr);
  bleCompleteTaskSuccess(BLETASK_REQUEST_DEVICE, device);
}
#endif

JsVar *jswrap_ble_requestDevice(JsVar *options) {
#if CENTRAL_LINK_COUNT>0
  if (!(jsvIsUndefined(options) || jsvIsObject(options))) {
    jsExceptionHere(JSET_TYPEERROR, "Expecting Object, got %t", options);
    return 0;
  }
  JsVar *filters = jsvObjectGetChildIfExists(options, "filters");
  if (!jsvIsArray(filters)) {
    jsvUnLock(filters);
    jsExceptionHere(JSET_TYPEERROR, "requestDevice expecting an array of filters, got %t", filters);
    return 0;
  }
  jsvUnLock(filters);

  JsVarFloat timeout = jsvObjectGetFloatChild(options, "timeout");
  if (isnan(timeout) || timeout<=0) timeout = 2000;

  JsVar *promise = 0;

  // Set a timeout for when we finish if we didn't find anything
  JsVar *timeoutIndex = jsiSetTimeout(jswrap_ble_requestDevice_finish, timeout);
  // Now create a promise, and pass in the timeout index so we can cancel the timeout if we find something
  if (bleNewTask(BLETASK_REQUEST_DEVICE, timeoutIndex)) {
    // Start scanning
    JsVar *fn = jsvNewNativeFunction((void (*)(void))jswrap_ble_requestDevice_scan, (JSWAT_JSVAR<<JSWAT_BITS));
    if (fn) {
      jswrap_ble_setScan(fn, options);
      jsvUnLock(fn);
    }
    promise = jsvLockAgainSafe(blePromise);
  }
  jsvUnLock(timeoutIndex);
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
    "#if" : "defined(NRF52_SERIES) || defined(ESP32)",
    "generate" : "jswrap_ble_connect",
    "params" : [
      ["mac","JsVar","The MAC address to connect to"],
      ["options","JsVar","(Espruino-specific) An object of connection options (see `BluetoothRemoteGATTServer.connect` for full details)"]
    ],
    "return" : ["JsVar", "A `Promise` that is resolved (or rejected) when the connection is complete" ],
    "return_object" : "Promise"
}
Connect to a BLE device by MAC address. Returns a promise, the argument of which
is the `BluetoothRemoteGATTServer` connection.

```
NRF.connect("aa:bb:cc:dd:ee").then(function(server) {
  // ...
});
```

This has the same effect as calling `BluetoothDevice.gatt.connect` on a
`BluetoothDevice` requested using `NRF.requestDevice`. It just allows you to
specify the address directly (without having to scan).

You can use it as follows - this would connect to another Puck device and turn
its LED on:

```
var gatt;
NRF.connect("aa:bb:cc:dd:ee random").then(function(g) {
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

**Note:** Espruino Bluetooth devices use a type of BLE address known as 'random
static', which is different to a 'public' address. To connect to an Espruino
device you'll need to use an address string of the form `"aa:bb:cc:dd:ee
random"` rather than just `"aa:bb:cc:dd:ee"`. If you scan for devices with
`NRF.findDevices`/`NRF.setScan` then addresses are already reported in the
correct format.
*/
JsVar *jswrap_ble_connect(JsVar *mac, JsVar *options) {
#if CENTRAL_LINK_COUNT>0
  JsVar *device = jspNewObject(0, "BluetoothDevice");
  if (!device) return 0;
  jsvObjectSetChild(device, "id", mac);
  JsVar *gatt = jswrap_BluetoothDevice_gatt(device);
  jsvUnLock(device);
  if (!gatt) return 0;
  JsVar *promise = jswrap_ble_BluetoothRemoteGATTServer_connect(gatt, options);
  jsvUnLock(gatt);
  return promise;
#else
  jsExceptionHere(JSET_ERROR, "Unimplemented");
  return 0;
#endif
}

/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "setWhitelist",
    "ifdef" : "NRF52_SERIES",
    "generate" : "jswrap_ble_setWhitelist",
    "params" : [
      ["whitelisting","bool","Are we using a whitelist? (default false)"]
    ]
}
If set to true, whenever a device bonds it will be added to the whitelist.

When set to false, the whitelist is cleared and newly bonded devices will not be
added to the whitelist.

**Note:** This is remembered between `reset()`s but isn't remembered after
power-on (you'll have to add it to `onInit()`.
*/
void jswrap_ble_setWhitelist(bool whitelist) {
#if PEER_MANAGER_ENABLED
  jsble_central_setWhitelist(whitelist);
#endif
}

/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "setConnectionInterval",
    "ifdef" : "NRF52_SERIES",
    "generate" : "jswrap_ble_setConnectionInterval",
    "params" : [
      ["interval","JsVar","The connection interval to use (see below)"]
    ]
}
When connected, Bluetooth LE devices communicate at a set interval. Lowering the
interval (e.g. more packets/second) means a lower delay when sending data, higher
bandwidth, but also more power consumption.

By default, when connected as a peripheral Espruino automatically adjusts the
connection interval. When connected it's as fast as possible (7.5ms) but when
idle for over a minute it drops to 200ms. On continued activity (>1 BLE
operation) the interval is raised to 7.5ms again.

The options for `interval` are:

* `undefined` / `"auto"` : (default) automatically adjust connection interval
* `100` : set min and max connection interval to the same number (between 7.5ms
  and 4000ms)
* `{minInterval:20, maxInterval:100}` : set min and max connection interval as a
  range

This configuration is not remembered during a `save()` - you will have to re-set
it via `onInit`.

**Note:** If connecting to another device (as Central), you can use an extra
argument to `NRF.connect` or `BluetoothRemoteGATTServer.connect` to specify a
connection interval.

**Note:** This overwrites any changes imposed by the deprecated
`NRF.setLowPowerConnection`
*/
void jswrap_ble_setConnectionInterval(JsVar *interval) {
#if NRF52_SERIES
  if (jsvIsUndefined(interval) || jsvIsStringEqual(interval,"auto")) {
    // allow automatic interval setting
    bleStatus &= ~BLE_DISABLE_DYNAMIC_INTERVAL;
  } else if (jsvIsNumeric(interval)) {
    // disable auto interval
    bleStatus |= BLE_DISABLE_DYNAMIC_INTERVAL;
    JsVarFloat f = jsvGetFloat(interval);
    jsble_check_error(jsble_set_periph_connection_interval(f,f));
  } else if (jsvIsObject(interval)) {
    // disable auto interval
    bleStatus |= BLE_DISABLE_DYNAMIC_INTERVAL;
    JsVarFloat min = jsvObjectGetFloatChild(interval,"minInterval");
    JsVarFloat max = jsvObjectGetFloatChild(interval,"maxInterval");
    jsble_check_error(jsble_set_periph_connection_interval(min, max));
  }
#endif
}

/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "setSecurity",
    "ifdef" : "NRF52_SERIES",
    "generate" : "jswrap_ble_setSecurity",
    "params" : [
      ["options","JsVar","An object containing security-related options (see below)"]
    ]
}
Sets the security options used when connecting/pairing. This applies to both
central *and* peripheral mode.

```
NRF.setSecurity({
  display : bool  // default false, can this device display a passkey on a screen/etc?
                  // - sent via the `BluetoothDevice.passkey` event
  keyboard : bool // default false, can this device enter a passkey
                  // - request sent via the `BluetoothDevice.passkeyRequest` event
  pair : bool // default true, allow other devices to pair with this device
  bond : bool // default true, Perform bonding
              // This stores info from pairing in flash and allows reconnecting without having to pair each time
  mitm : bool // default false, Man In The Middle protection
  lesc : bool // default false, LE Secure Connections
  passkey : // default "", or a 6 digit passkey to use (display must be true for this)
  oob : [0..15] // if specified, Out Of Band pairing is enabled and
                // the 16 byte pairing code supplied here is used
  encryptUart : bool // default false (unless oob or passkey specified)
                     // This sets the BLE UART service such that it
                     // is encrypted and can only be used from a paired connection
});
```

**NOTE:** Some combinations of arguments will cause an error. For example
supplying a passkey without `display:1` is not allowed. If `display:1` is set
you do not require a physical display, the user just needs to know the passkey
you supplied.

For instance, to require pairing and to specify a passkey, use:

```
NRF.setSecurity({passkey:"123456", mitm:1, display:1});
```

Or to require pairing and to display a PIN that the connecting device
provides, use:

```
NRF.setSecurity({mitm:1, display:1});
NRF.on("passkey", key => print("Enter PIN: ", key));
```

However, while most devices will request a passkey for pairing at this point it
is still possible for a device to connect without requiring one (e.g. using the
'NRF Connect' app).

To force a passkey you need to protect each characteristic you define with
`NRF.setSecurity`. For instance the following code will *require* that the
passkey `123456` is entered before the characteristic
`9d020002-bf5f-1d1a-b52a-fe52091d5b12` can be read.

```
NRF.setSecurity({passkey:"123456", mitm:1, display:1});
NRF.setServices({
  "9d020001-bf5f-1d1a-b52a-fe52091d5b12" : {
    "9d020002-bf5f-1d1a-b52a-fe52091d5b12" : {
      // readable always
      value : "Not Secret"
    },
    "9d020003-bf5f-1d1a-b52a-fe52091d5b12" : {
      // readable only once bonded
      value : "Secret",
      readable : true,
      security: {
        read: {
          mitm: true,
          encrypted: true
        }
      }
    },
    "9d020004-bf5f-1d1a-b52a-fe52091d5b12" : {
      // readable always
      // writable only once bonded
      value : "Readable",
      readable : true,
      writable : true,
      onWrite : function(evt) {
        console.log("Wrote ", evt.data);
      },
      security: {
        write: {
          mitm: true,
          encrypted: true
        }
      }
    }
  }
});
```

**Note:** If `passkey` or `oob` is specified, the Nordic UART service (if
enabled) will automatically be set to require encryption, but otherwise it is
open.
*/
void jswrap_ble_setSecurity(JsVar *options) {
  if (!jsvIsObject(options) && !jsvIsUndefined(options))
    jsExceptionHere(JSET_TYPEERROR, "Expecting Object or undefined, got %t", options);
  else {
    jsvObjectSetOrRemoveChild(execInfo.hiddenRoot, BLE_NAME_SECURITY, options);
    jsble_update_security();
    // If we need UART to be encrypted, we need to trigger a restart
    if (bleStatus & BLE_NEEDS_SOFTDEVICE_RESTART)
      jswrap_ble_restart(NULL);
  }
}

/*TYPESCRIPT
type NRFSecurityStatus = {
  advertising: boolean,
} & (
  {
    connected: true,
    encrypted: boolean,
    mitm_protected: boolean,
    bonded: boolean,
    connected_addr?: string,
  } | {
    connected: false,
    encrypted: false,
    mitm_protected: false,
    bonded: false,
  }
);
*/

/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "getSecurityStatus",
    "ifdef" : "NRF52_SERIES",
    "generate" : "jswrap_ble_getSecurityStatus",
    "return" : ["JsVar", "An object" ],
    "return_object" : "NRFSecurityStatus"
}
Return an object with information about the security state of the current
peripheral connection:

```
{
  connected       // The connection is active (not disconnected).
  encrypted       // Communication on this link is encrypted.
  mitm_protected  // The encrypted communication is also protected against man-in-the-middle attacks.
  bonded          // The peer is bonded with us
  advertising     // Are we currently advertising?
  connected_addr  // If connected=true, the MAC address of the currently connected device
}
```

If there is no active connection, `{connected:false}` will be returned.

See `NRF.setSecurity` for information about negotiating a secure connection.
*/
JsVar *jswrap_ble_getSecurityStatus(JsVar *parent) {
  return jsble_get_security_status(m_peripheral_conn_handle);
}

/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "startBonding",
    "ifdef" : "NRF52_SERIES",
    "generate" : "jswrap_ble_startBonding",
    "params" : [
      ["forceRepair","bool","True if we should force repairing even if there is already valid pairing info"]
    ],
    "return" : ["JsVar", "A promise" ]
}
*/
JsVar *jswrap_ble_startBonding(bool forceRePair) {
#if PEER_MANAGER_ENABLED
  if (bleNewTask(BLETASK_BONDING, NULL)) {
    JsVar *promise = jsvLockAgainSafe(blePromise);
    jsble_startBonding(forceRePair);
    return promise;
  }
#endif
  return 0;
}

/*JSON{
  "type" : "class",
  "class" : "BluetoothDevice",
  "ifdef" : "NRF52_SERIES"
}
A Web Bluetooth-style device - you can request one using
`NRF.requestDevice(address)`

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
    "#if" : "defined(NRF52_SERIES) || defined(ESP32)",
    "generate" : "jswrap_BluetoothDevice_gatt",
    "return" : ["JsVar", "A `BluetoothRemoteGATTServer` for this device" ]
}
*/
JsVar *jswrap_BluetoothDevice_gatt(JsVar *parent) {
#if CENTRAL_LINK_COUNT>0
  JsVar *gatt = jsvObjectGetChildIfExists(parent, "gatt");
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
    "type" : "property",
    "class" : "BluetoothDevice",
    "name" : "rssi",
    "ifdef" : "NRF52_SERIES",
    "generate" : false,
    "return" : ["bool", "The last received RSSI (signal strength) for this device" ]
}
*//*Documentation only*/
/*JSON{
    "type" : "event",
    "class" : "BluetoothDevice",
    "name" : "passkey",
    "ifdef" : "NRF52_SERIES",
    "params" : [
      ["passkey","JsVar","A 6 character numeric String to be displayed"]
    ]
}
Called when the device pairs and sends a passkey that Espruino should display.

For this to be used, you'll have to specify that there's a display using
`NRF.setSecurity`

**This is not part of the Web Bluetooth Specification.** It has been added
specifically for Espruino.
*/
/*JSON{
    "type" : "event",
    "class" : "BluetoothDevice",
    "name" : "passkeyRequest",
    "ifdef" : "NRF52_SERIES"
}
Called when the device pairs, displays a passkey, and wants Espruino to tell it
what the passkey was.

Respond with `BluetoothDevice.sendPasskey("123456")` with a 6 character string
containing only `0..9`.

For this to be used, you'll have to specify that there's a keyboard using
`NRF.setSecurity`

**This is not part of the Web Bluetooth Specification.** It has been added
specifically for Espruino.
*/
/*JSON{
    "type" : "method",
    "class" : "BluetoothDevice",
    "name" : "sendPasskey",
    "ifdef" : "NRF52_SERIES",
    "generate" : "jswrap_ble_BluetoothDevice_sendPasskey",
    "params" : [
      ["passkey","JsVar","A 6 character numeric String to be returned to the device"]
    ]
}
To be used as a response when the event `BluetoothDevice.sendPasskey` has been
received.

**This is not part of the Web Bluetooth Specification.** It has been added
specifically for Espruino.
*/
#if NRF52_SERIES
void jswrap_ble_BluetoothDevice_sendPasskey(JsVar *parent, JsVar *passkeyVar) {
#if CENTRAL_LINK_COUNT>0
  char passkey[BLE_GAP_PASSKEY_LEN+1];
  memset(passkey, 0, sizeof(passkey));
  jsvGetStringChars(passkeyVar,0,passkey, sizeof(passkey));
  uint32_t err_code = jsble_central_send_passkey(jswrap_ble_BluetoothDevice_getHandle(parent), passkey);
  jsble_check_error(err_code);
#endif
}
#endif

/*JSON{
    "type" : "method",
    "class" : "BluetoothRemoteGATTServer",
    "name" : "connect",
    "#if" : "defined(NRF52_SERIES) || defined(ESP32)",
    "generate" : "jswrap_ble_BluetoothRemoteGATTServer_connect",
    "params" : [
      ["options","JsVar","[optional] (Espruino-specific) An object of connection options (see below)"]
    ],
    "return" : ["JsVar", "A `Promise` that is resolved (or rejected) when the connection is complete" ],
    "return_object" : "Promise"
}
Connect to a BLE device - returns a promise, the argument of which is the
`BluetoothRemoteGATTServer` connection.

See [`NRF.requestDevice`](/Reference#l_NRF_requestDevice) for usage examples.

`options` is an optional object containing:

```
{
   minInterval // min connection interval in milliseconds, 7.5 ms to 4 s
   maxInterval // max connection interval in milliseconds, 7.5 ms to 4 s
}
```

By default the interval is 20-200ms (or 500-1000ms if
`NRF.setLowPowerConnection(true)` was called. During connection Espruino
negotiates with the other device to find a common interval that can be used.

For instance calling:

```
NRF.requestDevice({ filters: [{ namePrefix: 'Pixl.js' }] }).then(function(device) {
  return device.gatt.connect({minInterval:7.5, maxInterval:7.5});
}).then(function(g) {
```

will force the connection to use the fastest connection interval possible (as
long as the device at the other end supports it).

**Note:** The Web Bluetooth spec states that if a device hasn't advertised its
name, when connected to a device the central (in this case Espruino) should
automatically retrieve the name from the corresponding characteristic (`0x2a00`
on service `0x1800`). Espruino does not automatically do this.

*/
#if CENTRAL_LINK_COUNT>0
static void _jswrap_ble_central_connect(JsVar *addr, JsVar *options) {
  // this function gets called on idle - just to make it less
  // likely we get connected while in the middle of executing stuff
  ble_gap_addr_t peer_addr;
  // this should be ok since we checked in jswrap_ble_BluetoothRemoteGATTServer_connect
  if (!bleVarToAddr(addr, &peer_addr)) return;
  jsble_central_connect(peer_addr, options);
}
#endif

JsVar *jswrap_ble_BluetoothRemoteGATTServer_connect(JsVar *parent, JsVar *options) {
#if CENTRAL_LINK_COUNT>0

  JsVar *device = jsvObjectGetChildIfExists(parent, "device");
  JsVar *addr = jsvObjectGetChildIfExists(device, "id");
  // Convert mac address to something readable
  ble_gap_addr_t peer_addr;
  if (!bleVarToAddr(addr, &peer_addr)) {
    jsvUnLock2(device, addr);
    jsExceptionHere(JSET_TYPEERROR, "Expecting device with a mac address of the form aa:bb:cc:dd:ee:ff");
    return 0;
  }
  jsvUnLock(device);

  // we're already connected - just return a resolved promise
  if (jsvObjectGetBoolChild(parent,"connected")) {
    return jswrap_promise_resolve(parent);
  }

  JsVar *promise = 0;
  if (bleNewTask(BLETASK_CONNECT, parent/*BluetoothRemoteGATTServer*/)) {
    JsVar *fn = jsvNewNativeFunction((void (*)(void))_jswrap_ble_central_connect, JSWAT_VOID|(JSWAT_JSVAR<<JSWAT_BITS)|(JSWAT_JSVAR<<(2*JSWAT_BITS)));
    if (fn) {
      JsVar *args[] = {addr, options};
      jsiQueueEvents(0, fn, args, 2);
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
    "#if" : "defined(NRF52_SERIES) || defined(ESP32)"
}
Web Bluetooth-style GATT server - get this using `NRF.connect(address)` or
`NRF.requestDevice(options)` and `response.gatt.connect`

https://webbluetoothcg.github.io/web-bluetooth/#bluetoothremotegattserver
*/
/*JSON{
    "type" : "property",
    "class" : "BluetoothRemoteGATTServer",
    "name" : "connected",
    "generate" : false,
    "return" : ["bool", "Whether the device is connected or not" ]
}
*//*Documentation only*/
/*JSON{
    "type" : "property",
    "class" : "BluetoothRemoteGATTServer",
    "name" : "handle",
    "generate" : false,
    "return" : ["int", "The handle to this device (if it is currently connected) - the handle is an internal value used by the Bluetooth Stack" ]
}
*//*Documentation only*/
/*JSON{
    "type" : "method",
    "class" : "BluetoothRemoteGATTServer",
    "name" : "disconnect",
    "generate" : "jswrap_BluetoothRemoteGATTServer_disconnect",
    "return" : ["JsVar", "A `Promise` that is resolved (or rejected) when the disconnection is complete (non-standard)" ],
    "return_object" : "Promise",
    "#if" : "defined(NRF52_SERIES) || defined(ESP32)"
}
Disconnect from a previously connected BLE device connected with
`BluetoothRemoteGATTServer.connect` - this does not disconnect from something
that has connected to the Espruino.

**Note:** While `.disconnect` is standard Web Bluetooth, in the spec it returns
undefined not a `Promise` for implementation reasons. In Espruino we return a
`Promise` to make it easier to detect when Espruino is free to connect to
something else.
*/
JsVar *jswrap_BluetoothRemoteGATTServer_disconnect(JsVar *parent) {
#if CENTRAL_LINK_COUNT>0
  uint32_t              err_code;

  uint16_t central_conn_handle = jswrap_ble_BluetoothRemoteGATTServer_getHandle(parent);
  if (central_conn_handle != BLE_CONN_HANDLE_INVALID) {
    // we have a connection, disconnect
    JsVar *promise = 0;
    if (bleNewTask(BLETASK_DISCONNECT, parent/*BluetoothRemoteGATTServer*/))
      promise = jsvLockAgainSafe(blePromise);
    err_code = jsble_disconnect(central_conn_handle);
    jsble_check_error(err_code);
    return promise;
  } else {
    // no connection - try and cancel the connect attempt (assume we have one)
#ifdef NRF52_SERIES
    if (bleInTask(BLETASK_CONNECT)) {
      bleCompleteTaskFailAndUnLock(BLETASK_CONNECT, jsvNewFromString("Connection cancelled"));
      err_code = sd_ble_gap_connect_cancel();
    } else {
      jsExceptionHere(JSET_ERROR, "Not connected");
    }
#endif
#ifdef ESP32
    jsWarn("connect cancel not implemented yet\n");
#endif
    // maybe we don't, in which case we don't care about the error code
    return jswrap_promise_resolve(parent);
  }
#else
  jsExceptionHere(JSET_ERROR, "Unimplemented");
#endif
  return 0;
}

/*JSON{
    "type" : "method",
    "class" : "BluetoothRemoteGATTServer",
    "name" : "startBonding",
    "ifdef" : "NRF52_SERIES",
    "generate" : "jswrap_ble_BluetoothRemoteGATTServer_startBonding",
    "params" : [
      ["forceRePair","bool","If the device is already bonded, re-pair it"]
    ],
    "return" : ["JsVar", "A `Promise` that is resolved (or rejected) when the bonding is complete" ],
    "return_object" : "Promise"
}
Start negotiating bonding (secure communications) with the connected device, and
return a Promise that is completed on success or failure.

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
specifically for Espruino.
*/
JsVar *jswrap_ble_BluetoothRemoteGATTServer_startBonding(JsVar *parent, bool forceRePair) {
#if CENTRAL_LINK_COUNT>0
  if (bleNewTask(BLETASK_BONDING, parent/*BluetoothRemoteGATTServer*/)) {
    JsVar *promise = jsvLockAgainSafe(blePromise);
    jsble_central_startBonding(jswrap_ble_BluetoothRemoteGATTServer_getHandle(parent), forceRePair);
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
    "ifdef" : "NRF52_SERIES",
    "generate" : "jswrap_ble_BluetoothRemoteGATTServer_getSecurityStatus",
    "return" : ["JsVar", "An object" ],
    "return_object" : "NRFSecurityStatus"
}
Return an object with information about the security state of the current
connection:


```
{
  connected       // The connection is active (not disconnected).
  encrypted       // Communication on this link is encrypted.
  mitm_protected  // The encrypted communication is also protected against man-in-the-middle attacks.
  bonded          // The peer is bonded with us
}
```

See `BluetoothRemoteGATTServer.startBonding` for information about negotiating a
secure connection.

**This is not part of the Web Bluetooth Specification.** It has been added
specifically for Puck.js.
*/
JsVar *jswrap_ble_BluetoothRemoteGATTServer_getSecurityStatus(JsVar *parent) {
#if CENTRAL_LINK_COUNT>0
  return jsble_get_security_status(jswrap_ble_BluetoothRemoteGATTServer_getHandle(parent));
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
  "return" : ["JsVar", "A `Promise` that is resolved (or rejected) when the primary service is found (the argument contains a `BluetoothRemoteGATTService`)" ],
    "return_object" : "Promise",
  "#if" : "defined(NRF52_SERIES) || defined(ESP32)"
}
See `NRF.connect` for usage examples.
*/
JsVar *jswrap_BluetoothRemoteGATTServer_getPrimaryService(JsVar *parent, JsVar *service) {
#if CENTRAL_LINK_COUNT>0
  const char *err;
  ble_uuid_t uuid;

  JsVar *device = jsvObjectGetChildIfExists(parent, "device");
  bool ok = bleNewTask(BLETASK_PRIMARYSERVICE, device/*BluetoothDevice*/);
  jsvUnLock(device);
  if (!ok) {
    return 0;
  }

  err = bleVarToUUID(&uuid, service);
  if (err) {
    jsExceptionHere(JSET_ERROR, "%s", err);
    return 0;
  }

  JsVar *promise = jsvLockAgainSafe(blePromise);
  jsble_central_getPrimaryServices(jswrap_ble_BluetoothRemoteGATTServer_getHandle(parent), uuid);
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
  "return" : ["JsVar", "A `Promise` that is resolved (or rejected) when the primary services are found (the argument contains an array of `BluetoothRemoteGATTService`)" ],
    "return_object" : "Promise",
  "#if" : "defined(NRF52_SERIES) || defined(ESP32)"
}
*/
JsVar *jswrap_BluetoothRemoteGATTServer_getPrimaryServices(JsVar *parent) {
#if CENTRAL_LINK_COUNT>0
  ble_uuid_t uuid;
  uuid.type = BLE_UUID_TYPE_UNKNOWN;

  JsVar *device = jsvObjectGetChildIfExists(parent, "device");
  bool ok = bleNewTask(BLETASK_PRIMARYSERVICE, device/*BluetoothDevice*/);
  jsvUnLock(device);
  if (!ok)
    return 0;

  JsVar *promise = jsvLockAgainSafe(blePromise);
  jsble_central_getPrimaryServices(jswrap_ble_BluetoothRemoteGATTServer_getHandle(parent), uuid);
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
  "#if" : "defined(NRF52_SERIES) || defined(ESP32)"
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
  uint32_t err_code = jsble_set_central_rssi_scan(jswrap_ble_BluetoothRemoteGATTServer_getHandle(parent), callback != 0);
  jsble_check_error(err_code);
#else
  jsExceptionHere(JSET_ERROR, "Unimplemented");
  return;
#endif
}

/*JSON{
  "type" : "class",
  "class" : "BluetoothRemoteGATTService",
  "#if" : "defined(NRF52_SERIES) || defined(ESP32)"
}
Web Bluetooth-style GATT service - get this using
`BluetoothRemoteGATTServer.getPrimaryService(s)`

https://webbluetoothcg.github.io/web-bluetooth/#bluetoothremotegattservice
*/
/*JSON{
    "type" : "property",
    "class" : "BluetoothRemoteGATTService",
    "name" : "device",
    "ifdef" : "NRF52_SERIES",
    "generate" : false,
    "return" : ["JsVar", "The `BluetoothDevice` this Service came from" ]
}
*//*Documentation only*/
/*JSON{
  "type" : "method",
  "class" : "BluetoothRemoteGATTService",
  "name" : "getCharacteristic",
  "generate" : "jswrap_BluetoothRemoteGATTService_getCharacteristic",
  "params" : [ ["characteristic","JsVar","The characteristic UUID"] ],
  "return" : ["JsVar", "A `Promise` that is resolved (or rejected) when the characteristic is found (the argument contains a `BluetoothRemoteGATTCharacteristic`)" ],
  "return_object" : "Promise",
  "#if" : "defined(NRF52_SERIES) || defined(ESP32)"
}
See `NRF.connect` for usage examples.
*/
JsVar *jswrap_BluetoothRemoteGATTService_getCharacteristic(JsVar *parent, JsVar *characteristic) {
#if CENTRAL_LINK_COUNT>0
  const char *err;
  ble_uuid_t uuid;

  if (!bleNewTask(BLETASK_CHARACTERISTIC, parent/*BluetoothRemoteGATTService*/))
    return 0;

  err = bleVarToUUID(&uuid, characteristic);
  if (err) {
    jsExceptionHere(JSET_ERROR, "%s", err);
    return 0;
  }

  JsVar *promise = jsvLockAgainSafe(blePromise);
  jsble_central_getCharacteristics(jswrap_ble_BluetoothRemoteGATTService_getHandle(parent), parent, uuid);
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
  "return" : ["JsVar", "A `Promise` that is resolved (or rejected) when the characteristic is found (the argument contains an array of `BluetoothRemoteGATTCharacteristic`)" ],
  "return_object" : "Promise",
  "#if" : "defined(NRF52_SERIES) || defined(ESP32)"
}
*/
JsVar *jswrap_BluetoothRemoteGATTService_getCharacteristics(JsVar *parent) {
#if CENTRAL_LINK_COUNT>0
  ble_uuid_t uuid;
  uuid.type = BLE_UUID_TYPE_UNKNOWN;

  if (!bleNewTask(BLETASK_CHARACTERISTIC, parent/*BluetoothRemoteGATTService*/))
    return 0;

  JsVar *promise = jsvLockAgainSafe(blePromise);
  jsble_central_getCharacteristics(jswrap_ble_BluetoothRemoteGATTService_getHandle(parent), parent, uuid);
  return promise;
#else
  jsExceptionHere(JSET_ERROR, "Unimplemented");
  return 0;
#endif
}

/*JSON{
  "type" : "class",
  "class" : "BluetoothRemoteGATTCharacteristic",
  "#if" : "defined(NRF52_SERIES) || defined(ESP32)"
}
Web Bluetooth-style GATT characteristic - get this using
`BluetoothRemoteGATTService.getCharacteristic(s)`

https://webbluetoothcg.github.io/web-bluetooth/#bluetoothremotegattcharacteristic
*/
/*JSON{
    "type" : "property",
    "class" : "BluetoothRemoteGATTCharacteristic",
    "name" : "service",
    "ifdef" : "NRF52_SERIES",
    "generate" : false,
    "return" : ["JsVar", "The `BluetoothRemoteGATTService` this Service came from" ]
}
*//*Documentation only*/
/*JSON{
    "type" : "method",
    "class" : "BluetoothRemoteGATTCharacteristic",
    "name" : "writeValue",
    "generate" : "jswrap_ble_BluetoothRemoteGATTCharacteristic_writeValue",
    "params" : [
      ["data","JsVar","The data to write"]
    ],
    "return" : ["JsVar", "A `Promise` that is resolved (or rejected) when the characteristic is written" ],
    "return_object" : "Promise",
    "#if" : "defined(NRF52_SERIES) || defined(ESP32)"
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
JsVar *jswrap_ble_BluetoothRemoteGATTCharacteristic_writeValue(JsVar *characteristic, JsVar *data) {
#if CENTRAL_LINK_COUNT>0
  JSV_GET_AS_CHAR_ARRAY(dataPtr, dataLen, data);
  if (!dataPtr) return 0;

  if (!bleNewTask(BLETASK_CHARACTERISTIC_WRITE, 0))
    return 0;

  JsVar *promise = jsvLockAgainSafe(blePromise);
  jsble_central_characteristicWrite(jswrap_ble_BluetoothRemoteGATTCharacteristic_getHandle(characteristic), characteristic, dataPtr, dataLen);
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
    "generate" : "jswrap_ble_BluetoothRemoteGATTCharacteristic_readValue",
    "return" : ["JsVar", "A `Promise` that is resolved (or rejected) with a `DataView` when the characteristic is read" ],
    "return_object" : "Promise",
    "#if" : "defined(NRF52_SERIES) || defined(ESP32)"
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
JsVar *jswrap_ble_BluetoothRemoteGATTCharacteristic_readValue(JsVar *characteristic) {
#if CENTRAL_LINK_COUNT>0
  if (!bleNewTask(BLETASK_CHARACTERISTIC_READ, characteristic/*BluetoothRemoteGATTCharacteristic*/))
    return 0;

  JsVar *promise = jsvLockAgainSafe(blePromise);
  jsble_central_characteristicRead(jswrap_ble_BluetoothRemoteGATTCharacteristic_getHandle(characteristic), characteristic);
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
    "generate" : "jswrap_ble_BluetoothRemoteGATTCharacteristic_startNotifications",
    "return" : ["JsVar", "A `Promise` that is resolved (or rejected) with data when notifications have been added" ],
    "return_object" : "Promise",
    "ifdef" : "BLUETOOTH"
}
Starts notifications - whenever this characteristic's value changes, a
`characteristicvaluechanged` event is fired and `characteristic.value` will then
contain the new value as a `DataView`.

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
    console.log("-> ",event.target.value); // this is a DataView
  });
  return c.startNotifications();
}).then(function(d) {
  console.log("Waiting for notifications");
}).catch(function() {
  console.log("Something's broken.");
});
```

For example, to listen to the output of another Puck.js's Nordic Serial port
service, you can use:

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
JsVar *jswrap_ble_BluetoothRemoteGATTCharacteristic_startNotifications(JsVar *characteristic) {
#if CENTRAL_LINK_COUNT>0
  uint16_t central_conn_handle = jswrap_ble_BluetoothRemoteGATTCharacteristic_getHandle(characteristic);

  // Set our characteristic's handle up in the list of handles to notify for
  // TODO: What happens when we close the connection and re-open another?
  uint16_t handle = (uint16_t)jsvObjectGetIntegerChild(characteristic, "handle_value");
  JsVar *handles = jsvObjectGetChild(execInfo.hiddenRoot, "bleHdl", JSV_ARRAY);
  if (handles) {
    jsvSetArrayItem(handles, handle, characteristic);
    jsvUnLock(handles);
  }

  JsVar *promise;
#ifndef ESP32
  // Check for existing cccd_handle
  JsVar *cccdVar = jsvObjectGetChildIfExists(characteristic,"handle_cccd");
  if ( !cccdVar ) { // if it doesn't exist, try and find it
    if (!bleNewTask(BLETASK_CHARACTERISTIC_DESC_AND_STARTNOTIFY, characteristic/*BluetoothRemoteGATTCharacteristic*/))
      return 0;
    promise = jsvLockAgainSafe(blePromise);
    jsble_central_characteristicDescDiscover(central_conn_handle, characteristic);
  } else {
    jsvUnLock(cccdVar);
#else
  {
#endif
    if (!bleNewTask(BLETASK_CHARACTERISTIC_NOTIFY, characteristic/*BluetoothRemoteGATTCharacteristic*/))
      return 0;
    promise = jsvLockAgainSafe(blePromise);
    jsble_central_characteristicNotify(central_conn_handle, characteristic, true);
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
    "generate" : "jswrap_ble_BluetoothRemoteGATTCharacteristic_stopNotifications",
    "return" : ["JsVar", "A `Promise` that is resolved (or rejected) with data when notifications have been removed" ],
    "return_object" : "Promise",
    "ifdef" : "NRF52_SERIES"
}
Stop notifications (that were requested with
`BluetoothRemoteGATTCharacteristic.startNotifications`)
*/
JsVar *jswrap_ble_BluetoothRemoteGATTCharacteristic_stopNotifications(JsVar *characteristic) {
#if CENTRAL_LINK_COUNT>0
  uint16_t central_conn_handle = jswrap_ble_BluetoothRemoteGATTCharacteristic_getHandle(characteristic);

  // Remove our characteristic handle from the list of handles to notify for
  uint16_t handle = (uint16_t)jsvObjectGetIntegerChild(characteristic, "handle_value");
  JsVar *handles = jsvObjectGetChild(execInfo.hiddenRoot, "bleHdl", JSV_ARRAY);
  if (handles) {
    jsvSetArrayItem(handles, handle, 0);
    jsvUnLock(handles);
  }
  JsVar *promise = jsvLockAgainSafe(blePromise);
  jsble_central_characteristicNotify(central_conn_handle, characteristic, false);
  return promise;
#else
  jsExceptionHere(JSET_ERROR, "Unimplemented");
  return 0;
#endif
}
