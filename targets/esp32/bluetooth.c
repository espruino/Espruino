/**
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2013 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * Utilities for converting Nordic datastructures to Espruino and vice versa
 * ----------------------------------------------------------------------------
 */
#include <stdio.h>

#include "bt.h"
#include "esp_bt_main.h"
#include "esp_gatt_common_api.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"

#include "jswrap_bluetooth.h"
#include "bluetooth.h"
#include "bluetooth_utils.h"
#include "jsutils.h"
#include "jsparse.h"
#include "jsinteractive.h"

#include "BLE/esp32_gap_func.h"
#include "BLE/esp32_gatts_func.h"
#include "BLE/esp32_gattc_func.h"
#include "BLE/esp32_bluetooth_utils.h"
#include "jshardwareESP32.h"
 
volatile BLEStatus bleStatus;
ble_uuid_t bleUUIDFilter;
uint16_t bleAdvertisingInterval;           /**< The advertising interval (in units of 0.625 ms). */
volatile uint16_t m_peripheral_conn_handle;    /**< Handle of the current connection. */
volatile uint16_t m_central_conn_handles[1]; /**< Handle of central mode connection */

/** Initialise the BLE stack */
void jsble_init(){
	esp_err_t ret;
	if(ESP32_Get_NVS_Status(ESP_NETWORK_BLE)){
		ret = esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT);
		if(ret) {
			jsExceptionHere(JSET_ERROR,"mem release failed:%x\n",ret);
			return;
		}
	
		if(initController()) return;
		if(initBluedroid()) return;
		if(registerCallbacks()) return;
		setMtu();
		gap_init_security();
	}
	else{
		ret = esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT); 
		jsWarn("Bluetooth is disabled per ESP32.enableBLE(false)\n");
	}
}
/** Completely deinitialise the BLE stack. Return true on success */
bool jsble_kill(){
	jsWarn("kill not implemented yet\n");
  return true;
}

/// Executes a pending BLE event - returns the number of events Handled
int jsble_exec_pending(IOEvent *event) {
  int eventsHandled = 1;
  // if we got event data, unpack it first into a buffer
#if NRF_BLE_MAX_MTU_SIZE>64
  unsigned char buffer[NRF_BLE_MAX_MTU_SIZE];
#else
  unsigned char buffer[64];
#endif
  assert(sizeof(buffer) >= sizeof(ble_gap_evt_adv_report_t));
  assert(sizeof(buffer) >= NRF_BLE_MAX_MTU_SIZE);
  size_t bufferLen = 0;
  while (IOEVENTFLAGS_GETTYPE(event->flags) == EV_BLUETOOTH_PENDING_DATA) {
    int i, chars = IOEVENTFLAGS_GETCHARS(event->flags);
    for (i=0;i<chars;i++) {
      assert(bufferLen < sizeof(buffer));
      if (bufferLen < sizeof(buffer))
        buffer[bufferLen++] = event->data.chars[i];
    }

    jshPopIOEvent(event);
    eventsHandled++;
  }
  assert(IOEVENTFLAGS_GETTYPE(event->flags) == EV_BLUETOOTH_PENDING);

  // Now handle the actual event
  BLEPending blep = (BLEPending)event->data.time;
  uint16_t data = (uint16_t)(event->data.time>>8);
  switch (blep) {
   case BLEP_NONE: break;
   /*case BLEP_ERROR: {
     JsVar *v = jsble_get_error_string(data);
     jsWarn("SD %v (:%d)",v, *(uint32_t*)buffer);
     jsvUnLock(v);
     break;
   }
   case BLEP_CONNECTED: {
     assert(bufferLen == sizeof(ble_gap_addr_t));
     ble_gap_addr_t *peer_addr = (ble_gap_addr_t*)buffer;
     bleQueueEventAndUnLock(JS_EVENT_PREFIX"connect", bleAddrToStr(*peer_addr));
     jshHadEvent();
     break;
   }
   case BLEP_DISCONNECTED: {
     JsVar *reason = jsvNewFromInteger(data);
     bleQueueEventAndUnLock(JS_EVENT_PREFIX"disconnect", reason);
     break;
   }*/
   case BLEP_TASK_DISCOVER_SERVICE: { /* buffer = esp_ble_gattc_cb_param_t, bleTaskInfo = BluetoothDevice, bleTaskInfo2 = an array of BluetoothRemoteGATTService, or 0 */
     if (!bleInTask(BLETASK_PRIMARYSERVICE)) {
       jsExceptionHere(JSET_INTERNALERROR,"Wrong task: %d vs %d", bleGetCurrentTask(), BLETASK_PRIMARYSERVICE);
       break;
     }
     esp_ble_gattc_cb_param_t *p_data = (esp_ble_gattc_cb_param_t *)buffer;
     esp_gatt_srvc_id_t *srvc_id = (esp_gatt_srvc_id_t *)&p_data->search_res.srvc_id;

     if (!bleTaskInfo2) bleTaskInfo2 = jsvNewEmptyArray();
     if (!bleTaskInfo2) break;
     JsVar *o = jspNewObject(0, "BluetoothRemoteGATTService");
     if (o) {
       ble_uuid_t ble_uuid;
       espbtuuid_TO_bleuuid(srvc_id->id.uuid, &ble_uuid);
       jsvObjectSetChild(o,"device", bleTaskInfo);
       jsvObjectSetChildAndUnLock(o,"uuid", bleUUIDToStr(ble_uuid));
       jsvObjectSetChildAndUnLock(o,"isPrimary", jsvNewFromBool(true));
       jsvObjectSetChildAndUnLock(o,"start_handle", jsvNewFromInteger(p_data->search_res.start_handle));
       jsvObjectSetChildAndUnLock(o,"end_handle", jsvNewFromInteger(p_data->search_res.end_handle));
       jsvArrayPushAndUnLock(bleTaskInfo2, o);
     }
     break;
   }
   case BLEP_TASK_DISCOVER_SERVICE_COMPLETE: { /* bleTaskInfo = BluetoothDevice, bleTaskInfo2 = an array of BluetoothRemoteGATTService */
     // When done, send the result to the handler
     if (bleTaskInfo2 && bleUUIDFilter.type != BLE_UUID_TYPE_UNKNOWN) {
       // single item because filtering
       JsVar *t = jsvSkipNameAndUnLock(jsvArrayPopFirst(bleTaskInfo2));
       jsvUnLock(bleTaskInfo2);
       bleTaskInfo2 = t;
     }
     if (bleTaskInfo) bleCompleteTaskSuccess(BLETASK_PRIMARYSERVICE, bleTaskInfo2);
     else bleCompleteTaskFailAndUnLock(BLETASK_PRIMARYSERVICE, jsvNewFromString("No Services found"));
     break;
   }
   case BLEP_TASK_CENTRAL_CONNECTED: {
     int centralIdx = 0; // FIXME: only one central right now
     bleSetActiveBluetoothGattServer(centralIdx, bleTaskInfo); /* bleTaskInfo = instance of BluetoothRemoteGATTServer */
     jsvObjectSetChildAndUnLock(bleTaskInfo, "connected", jsvNewFromBool(true));
     jsvObjectSetChildAndUnLock(bleTaskInfo, "handle", jsvNewFromInteger(0));
     bleCompleteTaskSuccess(BLETASK_CONNECT, bleTaskInfo);
     break;
   }
   case BLEP_TASK_CHARACTERISTIC_READ: {
     JsVar *d = jsvNewDataViewWithData(bufferLen, buffer);
     jsvObjectSetChild(bleTaskInfo, "value", d); // set this.value
     bleCompleteTaskSuccessAndUnLock(BLETASK_CHARACTERISTIC_READ, d);
     break;
   }
   case BLEP_TASK_CHARACTERISTIC_WRITE: {
     bleCompleteTaskSuccess(BLETASK_CHARACTERISTIC_WRITE, 0);
     break;
   }
   case BLEP_TASK_CHARACTERISTIC_NOTIFY: {
     bleCompleteTaskSuccess(BLETASK_CHARACTERISTIC_NOTIFY, 0);
     break;
   }
   case BLEP_CENTRAL_DISCONNECTED: { // reason as data low byte
     int centralIdx = 0; // FIXME: only one central right now
     if (bleInTask(BLETASK_DISCONNECT))
       bleCompleteTaskSuccess(BLETASK_DISCONNECT, bleTaskInfo);
     JsVar *gattServer = bleGetActiveBluetoothGattServer(centralIdx);
     if (gattServer) {
       JsVar *bluetoothDevice = jsvObjectGetChild(gattServer, "device", 0);
       jsvObjectSetChildAndUnLock(gattServer, "connected", jsvNewFromBool(false));
       jsvObjectRemoveChild(gattServer, "handle");
       if (bluetoothDevice) {
         // HCI error code, see BLE_HCI_STATUS_CODES in ble_hci.h
         JsVar *reason = jsvNewFromInteger(data & 255);
         jsiQueueObjectCallbacks(bluetoothDevice, JS_EVENT_PREFIX"gattserverdisconnected", &reason, 1);
         jsvUnLock(reason);
         jshHadEvent();
       }
       jsvUnLock2(gattServer, bluetoothDevice);
     }
     bleSetActiveBluetoothGattServer(centralIdx, 0);
     break;
   }

   default:
     jsWarn("jsble_exec_pending: Unknown enum type %d",(int)blep);
  }
  return eventsHandled;
}

void jsble_restart_softdevice(JsVar *jsFunction){
	bleStatus &= ~(BLE_NEEDS_SOFTDEVICE_RESTART | BLE_SERVICES_WERE_SET);
	if (bleStatus & BLE_IS_SCANNING) {
		bluetooth_gap_setScan(false);
	}
	if (jsvIsFunction(jsFunction))
	  jspExecuteFunction(jsFunction,NULL,0,NULL);
	jswrap_ble_reconfigure_softdevice();
}

uint32_t jsble_advertising_start(){
	esp_err_t status;
	if (bleStatus & BLE_IS_ADVERTISING) return;
	status = bluetooth_gap_startAdvertizing(true);
	return status;
}
void jsble_advertising_stop(){
	esp_err_t status;
	status = bluetooth_gap_startAdvertizing(false);
	if(status){
	   jsExceptionHere(JSET_ERROR,"error in stop advertising:0X%x",status);
	}
}
/** Is BLE connected to any device at all? */
bool jsble_has_connection(){
#if CENTRAL_LINK_COUNT>0
  return (m_central_conn_handles[0] != BLE_GATT_HANDLE_INVALID) ||
         (m_peripheral_conn_handle != BLE_GATT_HANDLE_INVALID);
#else
  return m_peripheral_conn_handle != BLE_GATT_HANDLE_INVALID;
#endif
}

/** Is BLE connected to a central device at all? */
bool jsble_has_central_connection(){
#if CENTRAL_LINK_COUNT>0
  return (m_central_conn_handles[0] != BLE_GATT_HANDLE_INVALID);
#else
  return false;
#endif
}

/** Is BLE connected to a server device at all (eg, the simple, 'slave' mode)? */
bool jsble_has_peripheral_connection(){
  return (m_peripheral_conn_handle != BLE_GATT_HANDLE_INVALID);
}

/// Checks for error and reports an exception if there was one. Return true on error
bool jsble_check_error_line(uint32_t err_code, int lineNumber) {
  if (err_code != ESP_OK) {
    const char *n = esp_err_to_name(err_code);
    jsExceptionHere(JSET_ERROR, "BLE: %s (:%d)", n?n:"?", lineNumber);
    return true;
  }
	NOT_USED(err_code);
	NOT_USED(lineNumber);
	return false;
}
/// Scanning for advertisign packets
uint32_t jsble_set_scanning(bool enabled, JsVar *options){
  bool activeScan = false;
  if (enabled && jsvIsObject(options)) {
    activeScan = jsvGetBoolAndUnLock(jsvObjectGetChild(options, "active", 0));
    if (activeScan) {
      jsWarn("active scan not implemented\n");
    }
  }
	bluetooth_gap_setScan(enabled);
	return 0;
}

/// returning RSSI values for current connection
uint32_t jsble_set_rssi_scan(bool enabled){
	jsWarn("set rssi scan not implemeted yet\n");
	NOT_USED(enabled);
	return 0;
}

/** Actually set the services defined in the 'data' object. Note: we can
 * only do this *once* - so to change it we must reset the softdevice and
 * then call this again */
void jsble_set_services(JsVar *data){
	gatts_set_services(data);
}

/// Disconnect from the given connection
uint32_t jsble_disconnect(uint16_t conn_handle){
	return gattc_disconnect(conn_handle);
	return 0;
}

/// For BLE HID, send an input report to the receiver. Must be <= HID_KEYS_MAX_LEN
void jsble_send_hid_input_report(uint8_t *data, int length){
	jsWarn("send hid input report not implemented yet\n");
	NOT_USED(data);
	NOT_USED(length);
}

/// Connect to the given peer address. When done call bleCompleteTask
void jsble_central_connect(ble_gap_addr_t peer_addr, JsVar *options){
  // Ignore options for now
	gattc_connect(peer_addr, options);
}
/// Get primary services. Filter by UUID unless UUID is invalid, in which case return all. When done call bleCompleteTask
void jsble_central_getPrimaryServices(uint16_t central_conn_handle, ble_uuid_t uuid){
  NOT_USED(central_conn_handle);
  bleUUIDFilter = uuid;
	gattc_searchService(uuid);
}
/// Get characteristics. Filter by UUID unless UUID is invalid, in which case return all. When done call bleCompleteTask
void jsble_central_getCharacteristics(uint16_t central_conn_handle, JsVar *service, ble_uuid_t uuid){
  NOT_USED(central_conn_handle);
	gattc_getCharacteristics(service, uuid);
}
// Write data to the given characteristic. When done call bleCompleteTask
void jsble_central_characteristicWrite(uint16_t central_conn_handle, JsVar *characteristic, char *dataPtr, size_t dataLen){
	uint16_t handle = jsvGetIntegerAndUnLock(jsvObjectGetChild(characteristic, "handle_value", 0));
	gattc_writeValue(handle, dataPtr, dataLen);
}
// Read data from the given characteristic. When done call bleCompleteTask
void jsble_central_characteristicRead(uint16_t central_conn_handle, JsVar *characteristic){
	uint16_t handle = jsvGetIntegerAndUnLock(jsvObjectGetChild(characteristic, "handle_value", 0));
	gattc_readValue(handle);
}
// Discover descriptors of characteristic
void jsble_central_characteristicDescDiscover(uint16_t central_conn_handle, JsVar *characteristic){
	jsWarn("Central characteristicDescDiscover not implemented yet\n");
	NOT_USED(characteristic);
}
// Set whether to notify on the given characteristic. When done call bleCompleteTask
void jsble_central_characteristicNotify(uint16_t central_conn_handle, JsVar *characteristic, bool enable){
  uint16_t handle = jsvGetIntegerAndUnLock(jsvObjectGetChild(characteristic, "handle_value", 0));

  // see https://github.com/espressif/esp-idf/blob/master/examples/bluetooth/bluedroid/ble/gatt_client/tutorial/Gatt_Client_Example_Walkthrough.md#registering-for-notifications
}
/// Start bonding on the current central connection
void jsble_central_startBonding(uint16_t central_conn_handle, bool forceRePair){
	jsWarn("central start bonding not implemented yet\n");
	NOT_USED(forceRePair);
}
/// RSSI monitoring in central mode
uint32_t jsble_set_central_rssi_scan(uint16_t central_conn_handle, bool enabled){
	jsWarn("central set rssi scan not implemented yet\n");
	return 0;
}
// Set whether or not the whitelist is enabled
void jsble_central_setWhitelist(uint16_t central_conn_handle, bool whitelist){
	jsWarn("central set Whitelist not implemented yet\n");
}

void jsble_update_security() {
}

/// Return an object showing the security status of the given connection
JsVar *jsble_get_security_status(uint16_t conn_handle) {
  return 0;
}

/// Set the transmit power of the current (and future) connections
void jsble_set_tx_power(int8_t pwr) {
  jsWarn("jsble_set_tx_power not implemented yet\n");
}

uint32_t jsble_central_send_passkey(uint16_t central_conn_handle, char *passkey) {
  jsWarn("central set Whitelist not implemented yet\n");
  return 0;
}
