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
  jsble_reset();
}

// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------

/*JSON{
  "type" : "event",
  "class" : "NRF",
  "name" : "connect",
  "#ifdef" : "NRF52"
}
Called when Espruino *connects to another device* - not when a a device
connects to Espruino.
 */
/*JSON{
  "type" : "event",
  "class" : "NRF",
  "name" : "disconnect",
  "#ifdef" : "NRF52"
}
Called when Espruino *disconnects from another device* - not when a a device
disconnects from Espruino.
 */

/*JSON{
  "type" : "event",
  "class" : "NRF",
  "name" : "servicesDiscover",
  "#ifdef" : "NRF52"
}
Called with discovered services when discovery is finished
 */
/*JSON{
  "type" : "event",
  "class" : "NRF",
  "name" : "characteristicsDiscover",
  "#ifdef" : "NRF52"
}
Called with discovered characteristics when discovery is finished
 */

/*JSON{
  "type" : "event",
  "class" : "NRF",
  "name" : "NFCon",
  "#ifdef" : "NRF52"
}
Called when an NFC field is detected
 */
/*JSON{
  "type" : "event",
  "class" : "NRF",
  "name" : "NFCoff",
  "#ifdef" : "NRF52"
}
Called when an NFC field is no longer detected
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
  "#ifdef" : "BLUETOOTH"
}
The Bluetooth Serial port - used when data is sent or received over Bluetooth Smart on nRF51/nRF52 chips.
 */

/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "sleep",
    "generate" : "jswrap_nrf_bluetooth_sleep"
}
Disable Bluetooth communications
*/
void jswrap_nrf_bluetooth_sleep(void) {
  uint32_t err_code;

  // If connected, disconnect.
  if (jsble_has_simple_connection()) {
      err_code = sd_ble_gap_disconnect(m_conn_handle,  BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
      if (err_code)
          jsExceptionHere(JSET_ERROR, "Got BLE error code %d", err_code);
  }

  // Stop advertising
  if (bleStatus & BLE_IS_ADVERTISING)
    jsble_advertising_stop();
}

/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "wake",
    "generate" : "jswrap_nrf_bluetooth_wake"
}
Enable Bluetooth communications (they are enabled by default)
*/
void jswrap_nrf_bluetooth_wake(void) {
  jsble_advertising_start();
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
JsVarFloat jswrap_nrf_bluetooth_getBattery(void) {
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
        if (err_code)
          jsExceptionHere(JSET_ERROR, "Got BLE error code %d", err_code);
        bleChanged = true;
      }
      jsvUnLock(v);
    }
  } else if (!jsvIsUndefined(options)) {
    jsExceptionHere(JSET_TYPEERROR, "Expecting 'options' to be object or undefined, got %t", options);
    return;
  }

  if (jsvIsArray(data)) {
    // raw data...
    JSV_GET_AS_CHAR_ARRAY(dPtr, dLen, data);
    if (!dPtr) {
      jsExceptionHere(JSET_TYPEERROR, "Unable to convert data argument to an array");
      return;
    }

    if (bleChanged && isAdvertising)
      jsble_advertising_stop();
    err_code = sd_ble_gap_adv_data_set((uint8_t *)dPtr, dLen, NULL, 0);
    if (err_code)
       jsExceptionHere(JSET_ERROR, "Got BLE error code %d", err_code);
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
  if (err_code)
    jsExceptionHere(JSET_ERROR, "Got BLE error code %d", err_code);
  if (bleChanged && isAdvertising)
    jsble_advertising_start();
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
the form `"0xABCD"`, or strings of the form `""ABCDABCD-ABCD-ABCD-ABCD-ABCDABCDABCD""`

**Note:** Currently, services/characteristics can't be removed once added.
As a result, calling setServices multiple times will cause characteristics
to either be updated (value only) or ignored.

`options` can be of the form:

```
NRF.setServices(undefined, {
  hid : new Uint8Array(...), // optional, default is undefined. Enable BLE HID support
  uart : true, // optional, default is true. Enable BLE UART support
});
```

To enable BLE HID, you must set `hid` to an array which is the BLE report
descriptor. The easiest way to do this is to use the `ble_hid_controls`
or `ble_hid_keyboard` modules.

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

  jsvConfigObject configs[] = {
#if BLE_HIDS_ENABLED
      {"hid", JSV_ARRAY, &use_hid},
#endif
      {"uart", JSV_BOOLEAN, &use_uart}
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
    jsvObjectSetChild(execInfo.hiddenRoot, BLE_NAME_HID_DATA, 0);
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
    jsvObjectSetChild(execInfo.hiddenRoot, BLE_NAME_NUS, 0);
  } else {
    if (bleStatus & BLE_NUS_INITED)
      bleStatus |= BLE_NEEDS_SOFTDEVICE_RESTART;
    jsvObjectSetChildAndUnLock(execInfo.hiddenRoot, BLE_NAME_NUS, jsvNewFromBool(false));
  }

  // Save the current service data
  jsvObjectSetChild(execInfo.hiddenRoot, BLE_NAME_SERVICE_DATA, data);

  // work out whether to apply changes
  if (bleStatus & (BLE_SERVICES_WERE_SET|BLE_NEEDS_SOFTDEVICE_RESTART)) {
    if (jsble_has_connection()) {
      // Defer setting services until we have no active connection
      jsiConsolePrintf("BLE Connected, so queueing service update for later\n");
      bleStatus |= BLE_NEEDS_SOFTDEVICE_RESTART;
      return;
    } else {
      // Not connected, but we can update services now
      jsble_restart_softdevice();
      return;
    }
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

      while (jsvObjectIteratorHasValue(&serviceit)) {
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
              if (err_code != NRF_SUCCESS) {
                APP_ERROR_CHECK(err_code);
              }

              // Notify/indicate connected clients if necessary
              if ((notification_requested || indication_requested) && jsble_has_simple_connection()) {
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
                  APP_ERROR_CHECK(err_code);
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

Start/stop listening for BLE advertising packets within range.

```
// Start scanning
NRF.setScan(function(d) {
  console.log(JSON.stringify(d,null,2));
});
// prints {"rssi":-72, "addr":"##:##:##:##:##:##", "data":new ArrayBuffer([2,1,6,...])}

// Stop Scanning
NRF.setScan(false);
```
*/
void jswrap_nrf_bluetooth_setScan(JsVar *callback) {
  // set the callback event variable
  if (!jsvIsFunction(callback)) callback=0;
  jsvObjectSetChild(execInfo.root, BLE_SCAN_EVENT, callback);
  // either start or stop scanning
  uint32_t err_code = jsble_set_scanning(callback != 0);
  if (err_code)
    jsExceptionHere(JSET_ERROR, "Got BLE error code %d", err_code);
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

RSSI is the 'Received Signal Strength Indication' in dBm

```
// Start scanning
NRF.setRSSIHandler(function(rssi) {
  console.log(rssi);
});
// prints -85 (or similar)

// Stop Scanning
NRF.setRSSIHandler();
```
*/
void jswrap_nrf_bluetooth_setRSSIHandler(JsVar *callback) {
  // set the callback event variable
  if (!jsvIsFunction(callback)) callback=0;
  jsvObjectSetChild(execInfo.root, BLE_RSSI_EVENT, callback);
  // either start or stop scanning
  uint32_t err_code = jsble_set_rssi_scan(callback != 0);
  if (err_code)
    jsExceptionHere(JSET_ERROR, "Got BLE error code %d", err_code);
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
  if (err_code)
    jsExceptionHere(JSET_ERROR, "Got BLE error code %d", err_code);
}

/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "connect",
    "generate" : "jswrap_nrf_bluetooth_connect",
    "params" : [
      ["mac","JsVar","The MAC address to connect to"]
    ]
}
Connect to a BLE device by MAC address

**Note:** This is only available on some devices
*/
void jswrap_nrf_bluetooth_connect(JsVar *mac) {
#if CENTRAL_LINK_COUNT>0
  // Convert mac address to something readable
  ble_gap_addr_t peer_addr;
  if (!bleVarToAddr(mac, &peer_addr)) {
    jsExceptionHere(JSET_TYPEERROR, "Expecting a mac address of the form aa:bb:cc:dd:ee:ff");
    return;
  }

  uint32_t              err_code;
  ble_gap_scan_params_t     m_scan_param;
  m_scan_param.active       = 0;            // Active scanning set.
  m_scan_param.interval     = SCAN_INTERVAL;// Scan interval.
  m_scan_param.window       = SCAN_WINDOW;  // Scan window.
  m_scan_param.timeout      = 0x0000;       // No timeout.

  ble_gap_conn_params_t   gap_conn_params;
  gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
  gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
  gap_conn_params.slave_latency     = SLAVE_LATENCY;
  gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;

  err_code = sd_ble_gap_connect(&peer_addr, &m_scan_param, &gap_conn_params);
  if (err_code)
    jsExceptionHere(JSET_ERROR, "Got BLE error code %d", err_code);
#else
  jsExceptionHere(JSET_ERROR, "Unimplemented");
#endif
}

/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "disconnect",
    "generate" : "jswrap_nrf_bluetooth_disconnect"
}
Disconnect from a previously connected BLE device connected with
`NRF.connect` - this does not disconnect from something that has
connected to the Espruino.

**Note:** This is only available on some devices
*/
void jswrap_nrf_bluetooth_disconnect() {
#if CENTRAL_LINK_COUNT>0
  uint32_t              err_code;

  if (m_central_conn_handle != BLE_CONN_HANDLE_INVALID) {
    // we have a connection, disconnect
    err_code = sd_ble_gap_disconnect(m_central_conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
  } else {
    // no connection - try and cancel the connect attempt (assume we have one)
    err_code = sd_ble_gap_connect_cancel();
  }
  if (err_code) {
    jsExceptionHere(JSET_ERROR, "Got BLE error code %d", err_code);
  }
#else
  jsExceptionHere(JSET_ERROR, "Unimplemented");
#endif
}

/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "discoverServices",
    "generate" : "jswrap_nrf_bluetooth_discoverServices"
}
**Note:** This is only available on some devices
*/
void jswrap_nrf_bluetooth_discoverServices() {
#if CENTRAL_LINK_COUNT>0
  if (m_central_conn_handle == BLE_CONN_HANDLE_INVALID)
    return jsExceptionHere(JSET_ERROR, "Not Connected");

  uint32_t              err_code;
  err_code = sd_ble_gattc_primary_services_discover(m_central_conn_handle, 1 /* start handle */, NULL);
  if (err_code)
    jsExceptionHere(JSET_ERROR, "Got BLE error code %d", err_code);

#else
  jsExceptionHere(JSET_ERROR, "Unimplemented");
#endif
}

/*JSON{
    "type": "class",
    "class" : "BLEService"
}
**Note:** This is only available on some devices
*/

/*JSON{
    "type" : "method",
    "class" : "BLEService",
    "name" : "discoverCharacteristics",
    "generate" : "jswrap_nrf_bleservice_discoverCharacteristics"
}
**Note:** This is only available on some devices
*/
void jswrap_nrf_bleservice_discoverCharacteristics(JsVar *service) {
#if CENTRAL_LINK_COUNT>0
  if (m_central_conn_handle == BLE_CONN_HANDLE_INVALID)
    return jsExceptionHere(JSET_ERROR, "Not Connected");

  ble_gattc_handle_range_t range;
  range.start_handle = jsvGetIntegerAndUnLock(jsvObjectGetChild(service, "start_handle", 0));
  range.end_handle = jsvGetIntegerAndUnLock(jsvObjectGetChild(service, "end_handle", 0));

  uint32_t              err_code;
  err_code = sd_ble_gattc_characteristics_discover(m_central_conn_handle, &range);
  if (err_code)
    jsExceptionHere(JSET_ERROR, "Got BLE error code %d", err_code);
#else
  jsExceptionHere(JSET_ERROR, "Unimplemented");
#endif
}



/*JSON{
    "type": "class",
    "class" : "BLECharacteristic"
}
**Note:** This is only available on some devices
*/

/*JSON{
    "type" : "method",
    "class" : "BLECharacteristic",
    "name" : "write",
    "generate" : "jswrap_nrf_blecharacteristic_write",
    "params" : [
      ["data","JsVar","The data to write"]
    ]
}
**Note:** This is only available on some devices
*/
void jswrap_nrf_blecharacteristic_write(JsVar *characteristic, JsVar *data) {
#if CENTRAL_LINK_COUNT>0
  if (m_central_conn_handle == BLE_CONN_HANDLE_INVALID)
    return jsExceptionHere(JSET_ERROR, "Not Connected");

  JSV_GET_AS_CHAR_ARRAY(dataPtr, dataLen, data);
  if (!dataPtr) return;

  const ble_gattc_write_params_t write_params = {
      .write_op = BLE_GATT_OP_WRITE_CMD,
      .flags    = BLE_GATT_EXEC_WRITE_FLAG_PREPARED_WRITE,
      .handle   = jsvGetIntegerAndUnLock(jsvObjectGetChild(characteristic, "handle_value", 0)),
      .offset   = 0,
      .len      = dataLen,
      .p_value  = (uint8_t*)dataPtr
  };

  uint32_t              err_code;
  err_code = sd_ble_gattc_write(m_central_conn_handle, &write_params);
  if (err_code)
    jsExceptionHere(JSET_ERROR, "Got BLE error code %d", err_code);
#else
  jsExceptionHere(JSET_ERROR, "Unimplemented");
#endif
}

/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "nfcURL",
    "#ifdef" : "NRF52",
    "generate" : "jswrap_nrf_nfcURL",
    "params" : [
      ["url","JsVar","The URL string to expose on NFC, or `undefined` to disable NFC"]
    ]
}
Enables NFC and starts advertising the given URL. For example:

```
NRF.nrfURL("http://espruino.com");
```

**Note:** This is only available on nRF52-based devices
*/
void jswrap_nrf_nfcURL(JsVar *url) {
#ifdef USE_NFC
  // Check for disabling NFC
  if (jsvIsUndefined(url)) {
    jsvObjectSetChild(execInfo.hiddenRoot, "NFC", 0);
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

  uint8_t msg_buf[256];
  uint32_t len = sizeof(msg_buf);
  /* Encode URI message into buffer */
  err_code = nfc_uri_msg_encode( NFC_URI_NONE, // TODO: could auto-prepend http/etc.
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
    "name" : "sendHIDReport",
    "#ifdef" : "NRF52",
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


/* ---------------------------------------------------------------------
 *                                                               TESTING
 * ---------------------------------------------------------------------

 // Scanning, getting a service, characteristic, and then writing it

NRF.setScan(function(d) {
  console.log(JSON.stringify(d,null,2));
});

NRF.setScan(false);

NRF.on('connect', function() { print("CONNECTED"); });
NRF.connect("f0:de:1d:13:9f:48")


NRF.on('servicesDiscover', function(services) {
  print("services: "+JSON.stringify(services,null,2));

  NRF.on('characteristicsDiscover', function(c) {
    print("characteristics: "+JSON.stringify(c,null,2));
    chars = c;
    chars[0].write(255)
  });
  services[services.length-1].discoverCharacteristics();
});
NRF.discoverServices();

chars[0].write(0)


// ------------------------------ on BLE server (microbit) - allow display of data
NRF.setServices({
  0xBCDE : {
    0xABCD : {
      value : "0", // optional
      maxLen : 1, // optional (otherwise is length of initial value)
      broadcast : false, // optional, default is false
      readable : true,   // optional, default is false
      writable : true,   // optional, default is false
      onWrite : function(evt) { // optional
        show(evt.data);
      }
    }
    // more characteristics allowed
  }
  // more services allowed
});

 */
